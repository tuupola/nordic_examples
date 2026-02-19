#include <date_time.h>
#include <errno.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <time.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lte_time, LOG_LEVEL_INF);

static K_SEM_DEFINE(lte_connected_sem, 0, 1);
static volatile bool lte_connected;

static void lte_handler(const struct lte_lc_evt *const evt) {
    switch (evt->type) {
        case LTE_LC_EVT_NW_REG_STATUS:
            switch (evt->nw_reg_status) {
                case LTE_LC_NW_REG_REGISTERED_HOME:
                case LTE_LC_NW_REG_REGISTERED_ROAMING:
                    LOG_INF("LTE network registered.");
                    lte_connected = true;
                    k_sem_give(&lte_connected_sem);
                    break;
                case LTE_LC_NW_REG_SEARCHING:
                    LOG_INF("Searching for LTE network...");
                    lte_connected = false;
                    break;
                case LTE_LC_NW_REG_NOT_REGISTERED:
                case LTE_LC_NW_REG_REGISTRATION_DENIED:
                case LTE_LC_NW_REG_UNKNOWN:
                    LOG_WRN("LTE disconnected (status: %d).", evt->nw_reg_status);
                    lte_connected = false;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static int lte_connect(void) {
    int rc;

    LOG_INF("Connecting to LTE network.");
    rc = lte_lc_connect_async(lte_handler);
    if (rc < 0) {
        LOG_ERR("LTE connect async failed: %d", rc);
        return rc;
    }

    /* Block until the handler signals registration. */
    k_sem_take(&lte_connected_sem, K_FOREVER);
    LOG_INF("Connected to LTE network.");
    return 0;
}

int main(void) {
    int rc;
    int64_t ts;
    time_t time_sec;
    struct tm *tm;

    LOG_INF("Initializing the modem firmware.");
    rc = nrf_modem_lib_init();
    if (rc < 0) {
        LOG_ERR("Modem init failed: %d", rc);
        return rc;
    }

    rc = lte_connect();
    if (rc < 0) {
        return rc;
    }

    while (1) {
        if (!lte_connected) {
            LOG_WRN("LTE disconnected, reconnecting...");
            rc = lte_connect();
            if (rc < 0) {
                LOG_ERR("Reconnect failed: %d", rc);
                k_sleep(K_SECONDS(10));
                continue;
            }
        }

        /* This gets time from the LTE modem. */
        rc = date_time_now(&ts);
        if (rc == 0) {
            time_sec = ts / 1000;
            tm = gmtime(&time_sec);
            LOG_INF("UTC: %04d-%02d-%02d %02d:%02d:%02d",
                tm->tm_year + 1900, tm->tm_mon + 1,
                tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec
            );
        } else {
            LOG_WRN("Time not available: %d", rc);
        }
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
