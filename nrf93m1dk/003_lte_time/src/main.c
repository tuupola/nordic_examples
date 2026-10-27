#include <date_time.h>
#include <time.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define L4_EVENT_MASK \
    (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED | NET_EVENT_DNS_SERVER_ADD)
#define L4_CONNECTED BIT(0)
#define L4_DNS_ADDED BIT(1)
#define LTE_CONNECT_TIMEOUT_S 120
#define DNS_TIMEOUT_S 10
#define LTE_DISCONNECT_TIMEOUT_S 10

static const struct device *modem = DEVICE_DT_GET(DT_ALIAS(modem));
static struct net_if *ppp_iface;
static K_EVENT_DEFINE(l4_event);

/* https://docs.zephyrproject.org/latest/doxygen/html/net__event_8h.html */
static void l4_event_handler(
    uint64_t event, struct net_if *iface, void *info, size_t info_length, void *user_data
) {
    ARG_UNUSED(info);
    ARG_UNUSED(info_length);
    ARG_UNUSED(user_data);

    if (iface != ppp_iface) {
        return;
    }

    switch (event) {
        case NET_EVENT_L4_CONNECTED:
            k_event_post(&l4_event, L4_CONNECTED);
            break;
        case NET_EVENT_DNS_SERVER_ADD:
            k_event_post(&l4_event, L4_DNS_ADDED);
            break;
        case NET_EVENT_L4_DISCONNECTED:
            k_event_set(&l4_event, 0);
            break;
        default:
            break;
    }
}

NET_MGMT_REGISTER_EVENT_HANDLER(l4_events, L4_EVENT_MASK, l4_event_handler, NULL);

static int modem_connect(void) {
    uint32_t events;
    int rc;

    if (!device_is_ready(modem)) {
        LOG_ERR("Modem device not ready");
        return -ENODEV;
    }

    ppp_iface = net_if_get_first_by_type(&NET_L2_GET_NAME(PPP));
    if (ppp_iface == NULL) {
        LOG_ERR("PPP interface not found");
        return -ENODEV;
    }

    rc = net_if_up(ppp_iface);
    if (rc < 0 && rc != -EALREADY) {
        LOG_ERR("Interface up failed: %d", rc);
        return rc;
    }

    LOG_INF("Waiting for LTE.");
    events =
        k_event_wait(&l4_event, L4_CONNECTED, false, K_SECONDS(LTE_CONNECT_TIMEOUT_S));
    if ((events & L4_CONNECTED) == 0) {
        LOG_ERR("LTE connect timed out");
        return -ETIMEDOUT;
    }

    events = k_event_wait(&l4_event, L4_DNS_ADDED, false, K_SECONDS(DNS_TIMEOUT_S));
    if ((events & L4_DNS_ADDED) == 0) {
        LOG_ERR("DNS server was not added");
        return -ETIMEDOUT;
    }

    return 0;
}

/* Auto-update only hooks lte_lc. NTP needs an explicit request. */
static int time_update(void) {
    int rc;

    rc = date_time_update_async(NULL);
    if (rc < 0) {
        LOG_ERR("Date time update failed: %d", rc);
    }

    return rc;
}

static int wait_disconnected(void) {
    int64_t deadline;

    deadline = k_uptime_get() + (LTE_DISCONNECT_TIMEOUT_S * MSEC_PER_SEC);
    while (k_uptime_get() < deadline) {
        if ((k_event_test(&l4_event, L4_CONNECTED) & L4_CONNECTED) == 0) {
            return 0;
        }
        k_sleep(K_MSEC(100));
    }

    return -ETIMEDOUT;
}

static void log_utc(void) {
    int rc;
    int64_t ts;
    time_t time_sec;
    struct tm *tm;

    rc = date_time_now(&ts);
    if (rc < 0) {
        LOG_WRN("Time not available: %d", rc);
        return;
    }

    time_sec = ts / 1000;
    tm = gmtime(&time_sec);
    if (tm == NULL) {
        LOG_ERR("gmtime failed");
        return;
    }

    LOG_INF(
        "UTC: %04d-%02d-%02d %02d:%02d:%02d",
        tm->tm_year + 1900,
        tm->tm_mon + 1,
        tm->tm_mday,
        tm->tm_hour,
        tm->tm_min,
        tm->tm_sec
    );
}

int main(void) {
    int rc;
    int count = 0;

    LOG_INF("Connecting to LTE network.");
    rc = modem_connect();
    if (rc < 0) {
        return rc;
    }
    LOG_INF("Connected to LTE network.");

    rc = time_update();
    if (rc < 0) {
        return rc;
    }

    while (1) {
        if ((k_event_test(&l4_event, L4_CONNECTED) & L4_CONNECTED) == 0) {
            LOG_WRN("LTE disconnected, reconnecting...");
            rc = modem_connect();
            if (rc < 0) {
                LOG_ERR("Reconnect failed: %d", rc);
                k_sleep(K_SECONDS(10));
                continue;
            }

            rc = time_update();
            if (rc < 0) {
                k_sleep(K_SECONDS(10));
                continue;
            }
        }

        log_utc();
        count++;
        if (count == 30) {
            LOG_INF("Forcing LTE disconnect for testing.");
            rc = net_if_down(ppp_iface);
            if (rc < 0 && rc != -EALREADY) {
                LOG_ERR("Interface down failed: %d", rc);
            } else {
                rc = wait_disconnected();
                if (rc < 0) {
                    LOG_ERR("LTE disconnect timed out");
                }
            }
        }

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
