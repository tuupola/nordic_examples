#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_gnss.h>

LOG_MODULE_REGISTER(gnss, LOG_LEVEL_INF);

int main(void)
{
    int rc;

    LOG_INF("Initializing the modem firmware.");
    rc = nrf_modem_lib_init();
    if (rc < 0) {
        LOG_ERR("Modem init failed: %d", rc);
        return rc;
    }

    rc = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_GNSS);
    if (rc < 0) {
        LOG_ERR("Failed to activate GNSS: %d", rc);
        return rc;
    }

    rc = nrf_modem_gnss_start();
    if (rc < 0) {
        LOG_ERR("Failed to start GNSS: %d", rc);
        return rc;
    }
    LOG_INF("GNSS started.");

    while (1) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
