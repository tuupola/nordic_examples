#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/byteorder.h>

#include <rid/message.h>
#include <rid/message_pack.h>
#include <rid/transport.h>
#include <rid/version.h>

LOG_MODULE_REGISTER(rid_scan, LOG_LEVEL_INF);

#define BLUETOOTH_HEADER_SIZE 4 /* uuid + app code + counter */
#define BLUETOOTH_MIN_DATA_LEN (RID_MESSAGE_SIZE + BLUETOOTH_HEADER_SIZE)

static bool filter_rid(struct bt_data *data, void *user_data) {
    const uint8_t **buffer = user_data;

    /* Service data with a 16-bit UUID. */
    if (data->type != BT_DATA_SVC_DATA16) {
        return true;
    }

    /* Must be atleast 29 bytes. */
    if (data->data_len < BLUETOOTH_MIN_DATA_LEN) {
        return true;
    }

    /* Check for 0xFFFA. */
    if (sys_get_le16(data->data) != RID_TRANSPORT_BLUETOOTH_SERVICE_UUID) {
        return true;
    }

    /* Check for 0x0D. */
    if (data->data[RID_TRANSPORT_BLUETOOTH_SERVICE_UUID_SIZE] !=
        RID_TRANSPORT_BLUETOOTH_APP_CODE) {
        return true;
    }

    /* If all above is true save Remote ID bytes to buffer. */
    *buffer = data->data + BLUETOOTH_HEADER_SIZE;
    return false;
}

static void
callback(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
    int rc;
    static char json[2048];
    const uint8_t *buffer = NULL;
    const rid_message_t *message;
    struct net_buf_simple_state state;

    net_buf_simple_save(ad, &state);
    bt_data_parse(ad, filter_rid, &buffer);
    net_buf_simple_restore(ad, &state);

    /* Buffer has data only if we got a Remote ID. */
    if (buffer == NULL) {
        return;
    }

    message = (const rid_message_t *)buffer;

    LOG_DBG("%s rssi=%d type=%u len=%u", bt_addr_le_str(addr), rssi, type, ad->len);
    LOG_HEXDUMP_DBG(ad->data, ad->len, "ad");

    rc = rid_message_to_json(message, json, sizeof(json), NULL);
    if (rc < 0) {
        LOG_ERR("JSON encode failed: %s", rid_error_to_string(rc));
        return;
    }

    /* Add the mac address to the JSON output and dump. */
    printf("{\"mac\": \"%s\", %s\n", bt_addr_str(&addr->a), json + 1);
}

int main(void) {
    int rc;

    /* Give console some time to catch up. */
    k_msleep(1000);

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
