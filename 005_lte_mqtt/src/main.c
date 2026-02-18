#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

LOG_MODULE_REGISTER(lte_mqtt, LOG_LEVEL_INF);

int main(void)
{
    int rc;
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
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
