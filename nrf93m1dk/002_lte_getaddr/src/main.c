#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/socket.h>

LOG_MODULE_REGISTER(lte_getaddr, LOG_LEVEL_INF);

#define L4_EVENT_MASK \
    (NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED | NET_EVENT_DNS_SERVER_ADD)
#define L4_CONNECTED BIT(0)
#define L4_DNS_ADDED BIT(1)
#define LTE_CONNECT_TIMEOUT_S 120
#define DNS_TIMEOUT_S 10

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

int main(void) {
    int rc;
    struct zsock_addrinfo *result;
    struct zsock_addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    char ip_str[NET_IPV4_ADDR_LEN];
    struct sockaddr_in *sockaddr;

    LOG_INF("Connecting to LTE network.");
    rc = modem_connect();
    if (rc < 0) {
        return rc;
    }
    LOG_INF("Connected to LTE network.");

    LOG_INF("Resolving www.appelsiini.net.");
    rc = zsock_getaddrinfo("www.appelsiini.net", NULL, &hints, &result);
    if (rc != 0) {
        LOG_ERR("getaddrinfo failed: %d", rc);
        return rc;
    }

    sockaddr = (struct sockaddr_in *)result->ai_addr;
    net_addr_ntop(AF_INET, &sockaddr->sin_addr, ip_str, sizeof(ip_str));
    LOG_INF("IP: %s", ip_str);

    zsock_freeaddrinfo(result);

    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}
