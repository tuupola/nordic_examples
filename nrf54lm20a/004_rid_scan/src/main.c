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

    /* If all above is tru save Remote ID bytes to buffer. */
    *buffer = data->data + BLUETOOTH_HEADER_SIZE;
    return false;
}

static void
callback(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
    const uint8_t *buffer = NULL;
    const rid_message_t *message;
    rid_protocol_version_t version;
    rid_message_type_t message_type;
    struct net_buf_simple_state state;

    net_buf_simple_save(ad, &state);
    bt_data_parse(ad, filter_rid, &buffer);
    net_buf_simple_restore(ad, &state);

    if (buffer == NULL) {
        return;
    }

    message = (const rid_message_t *)buffer;
    version = rid_message_get_protocol_version(message);
    message_type = rid_message_get_type(message);

    LOG_INF("%s rssi=%d type=%u len=%u", bt_addr_le_str(addr), rssi, type, ad->len);
    //LOG_HEXDUMP_INF(ad->data, ad->len, "ad");
    LOG_INF("protocol_version=%u message_type=%u", version, message_type);
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
