#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <rid/version.h>

LOG_MODULE_REGISTER(ble_scan, LOG_LEVEL_INF);

static void
callback(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
    LOG_INF("%s rssi=%d type=%u len=%u", bt_addr_le_str(addr), rssi, type, ad->len);
    LOG_HEXDUMP_INF(ad->data, ad->len, "ad");
}

int main(void) {
    int rc;

    LOG_INF("librid %s", rid_version_to_string());
    LOG_INF("board=%s soc=%s", CONFIG_BOARD, CONFIG_SOC);

    rc = bt_enable(NULL);
    if (rc < 0) {
        LOG_ERR("Bluetooth init failed: %d", rc);
        return rc;
    }

    rc = bt_le_scan_start(BT_LE_SCAN_CODED_PASSIVE, callback);
    if (rc < 0) {
        LOG_ERR("Bluetooth LE scan start failed: %d", rc);
        return rc;
    }

    LOG_INF("Bluetooth LE scan started...");
    return 0;
}
