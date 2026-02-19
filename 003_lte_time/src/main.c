#include <date_time.h>
#include <errno.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <time.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lte_time, LOG_LEVEL_INF);

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

    /* This is blocking and can take up to 30 seconds. */
    LOG_INF("Connecting to LTE network.");
    rc = lte_lc_connect();
    if (rc < 0) {
        LOG_ERR("LTE connect failed: %d", rc);
        return rc;
    }
    LOG_INF("Connected to LTE network.");

    while (1) {
        /* This gets time from the LTE modem. */
        rc = date_time_now(&ts);
        if (rc == 0) {
            time_sec = ts / 1000;
            tm = gmtime(&time_sec);
            LOG_INF("UTC: %04d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        } else {
            LOG_WRN("Time not available: %d", rc);
        }
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
