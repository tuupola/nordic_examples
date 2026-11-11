#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <hal/nrf_ficr.h>

LOG_MODULE_REGISTER(board_info, LOG_LEVEL_INF);

static void log_chip_info(void) {
    uint32_t hwid, part, variant;

    part = NRF_FICR->INFO.PART;
    variant = NRF_FICR->INFO.VARIANT;
    hwid = NRF_FICR->INFO.CONFIGID & FICR_INFO_CONFIGID_HWID_Msk;

    LOG_INF("board=%s soc=%s", CONFIG_BOARD, CONFIG_SOC);
    LOG_INF(
        "part=0x%08x variant=%c%c%c%c",
        part,
        (variant >> 24) & 0xff,
        (variant >> 16) & 0xff,
        (variant >> 8) & 0xff,
        variant & 0xff
    );
    LOG_INF(
        "package=0x%08x ram=%u kB rram=%u kB",
        NRF_FICR->INFO.PACKAGE,
        NRF_FICR->INFO.RAM,
        NRF_FICR->INFO.RRAM
    );
    LOG_INF(
        "deviceid=%08x%08x hwid=0x%04x",
        NRF_FICR->INFO.DEVICEID[1],
        NRF_FICR->INFO.DEVICEID[0],
        hwid
    );
}

int main(void) {
    while (1) {
        log_chip_info();
        /* k_ prefix denotes kernel */
        k_sleep(K_MSEC(1000));
    }
    return 0;
}
