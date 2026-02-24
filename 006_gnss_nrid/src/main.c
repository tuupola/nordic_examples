#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_gnss.h>
#include <rid/rid.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "uav.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static struct nrf_modem_gnss_pvt_data_frame pvt_data;
static rid_location_t location;

static void gnss_event_handler(int event) {
    switch (event) {
        case NRF_MODEM_GNSS_EVT_PVT:
            LOG_INF("NRF_MODEM_GNSS_EVT_PVT");
            break;
        case NRF_MODEM_GNSS_EVT_FIX:
            LOG_INF("NRF_MODEM_GNSS_EVT_FIX");
            break;
        case NRF_MODEM_GNSS_EVT_NMEA:
            LOG_INF("NRF_MODEM_GNSS_EVT_NMEA");
            break;
        case NRF_MODEM_GNSS_EVT_AGNSS_REQ:
            LOG_INF("NRF_MODEM_GNSS_EVT_AGNSS_REQ");
            break;
        case NRF_MODEM_GNSS_EVT_BLOCKED:
            LOG_INF("NRF_MODEM_GNSS_EVT_BLOCKED");
            break;
        case NRF_MODEM_GNSS_EVT_UNBLOCKED:
            LOG_INF("NRF_MODEM_GNSS_EVT_UNBLOCKED");
            break;
        case NRF_MODEM_GNSS_EVT_PERIODIC_WAKEUP:
            LOG_INF("NRF_MODEM_GNSS_EVT_PERIODIC_WAKEUP");
            break;
        case NRF_MODEM_GNSS_EVT_SLEEP_AFTER_TIMEOUT:
            LOG_INF("NRF_MODEM_GNSS_EVT_SLEEP_AFTER_TIMEOUT");
            break;
        case NRF_MODEM_GNSS_EVT_SLEEP_AFTER_FIX:
            LOG_INF("NRF_MODEM_GNSS_EVT_SLEEP_AFTER_FIX");
            break;
        default:
            LOG_WRN("Unknown GNSS event: %d", event);
            break;
    }
}

int main(void) {
    int rc;

    LOG_INF("librid %s", rid_version_to_string());

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

    rc = nrf_modem_gnss_event_handler_set(gnss_event_handler);
    if (rc < 0) {
        LOG_ERR("Failed to set GNSS event handler: %d", rc);
        return rc;
    }

    /* Continous 1Hz navigation mode */
    rc = nrf_modem_gnss_fix_interval_set(1);
    if (rc < 0) {
        LOG_ERR("Failed to set fix interval: %d", rc);
        return rc;
    }

    rc = nrf_modem_gnss_start();
    if (rc < 0) {
        LOG_ERR("Failed to start GNSS: %d", rc);
        return rc;
    }
    LOG_INF("GNSS started.");

    while (1) {
        int tracked = 0;
        int used = 0;

        rc = nrf_modem_gnss_read(&pvt_data, sizeof(pvt_data), NRF_MODEM_GNSS_DATA_PVT);

        if (rc == 0 && (pvt_data.flags & NRF_MODEM_GNSS_PVT_FLAG_FIX_VALID)) {
            LOG_INF("  lat=%.06f lon=%.06f", pvt_data.latitude, pvt_data.longitude);

            uav_location_update(&location, &pvt_data);
        }

        for (int i = 0; i < NRF_MODEM_GNSS_MAX_SATELLITES; i++) {
            /* No data in this slot */
            if (pvt_data.sv[i].sv == 0) {
                break;
            }
            const char *used_str = "";

            /* We have a satellite */
            tracked++;
            if (pvt_data.sv[i].flags & NRF_MODEM_GNSS_SV_FLAG_USED_IN_FIX) {
                /* This satellite is used in fix */
                used++;
                used_str = " *";
            }
            LOG_INF(
                "  SV %3d: cn0=%.01f el=%d az=%d%s",
                pvt_data.sv[i].sv,
                pvt_data.sv[i].cn0 / 10.0,
                pvt_data.sv[i].elevation,
                pvt_data.sv[i].azimuth,
                used_str
            );
        }
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
