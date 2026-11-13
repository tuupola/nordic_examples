#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nRF5340, LOG_LEVEL_INF);

#define SYNC_0 0xAA /* 0b10101010 */
#define SYNC_1 0x55 /* 0b01010101 */

#define CMD_LEGACY_ADV_UPDATE 0x01

#define RID_UUID_LO 0xFA
#define RID_UUID_HI 0xFF

#define MAX_PAYLOAD 255

static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

/* Service data buffer: 2-byte UUID prefix + payload from nRF9151. */
static uint8_t svc_data[2 + MAX_PAYLOAD];

/* Advertising data, updated before each call to bt_le_adv_update_data(). */
static struct bt_data ad[] = {
    {.type = BT_DATA_SVC_DATA16, .data_len = 2, .data = svc_data},
};

static uint8_t uart_read_byte(void) {
    uint8_t c;

    while (uart_poll_in(uart_dev, &c) != 0) {
        k_yield();
    }
    return c;
}

static void uart_read_buffer(uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = uart_read_byte();
    }
}

int main(void) {
    int rc;

    /* Delay startup so we have time to see the log messages. */
    k_sleep(K_SECONDS(2));

    LOG_INF("Bluetooth starting.");

    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART1 device not ready");
        return -ENODEV;
    }
    LOG_INF("UART1 ready.");

    rc = bt_enable(NULL);
    if (rc < 0) {
        LOG_ERR("Bluetooth init failed: %d", rc);
        return rc;
    }
    LOG_INF("Bluetooth initialized.");

    /* Prepopulate the UUID prefix, this never changes. */
    svc_data[0] = RID_UUID_LO;
    svc_data[1] = RID_UUID_HI;

    /* Non-connectable undirected advertising. */
    struct bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_USE_IDENTITY,
        BT_GAP_ADV_FAST_INT_MIN_2,
        BT_GAP_ADV_FAST_INT_MAX_2,
        NULL
    );

    rc = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (rc < 0) {
        LOG_ERR("Advertising start failed: %d", rc);
        return rc;
    }
    LOG_INF("Advertising started. Waiting for UART frames.");

    while (1) {
        uint8_t cmd;
        uint8_t len;
        uint8_t payload[MAX_PAYLOAD];

        /* Wait for sync word. */
        if (uart_read_byte() != SYNC_0) {
            continue;
        }
        if (uart_read_byte() != SYNC_1) {
            continue;
        }

        /* Read command and payload length. */
        cmd = uart_read_byte();
        len = uart_read_byte();

        /* Ignore too big payloads. */
        if (len > MAX_PAYLOAD) {
            LOG_WRN("Payload too large: %u", len);
            for (uint8_t i = 0; i < len; i++) {
                uart_read_byte();
            }
            continue;
        }

        /* Read payload into a buffer. */
        uart_read_buffer(payload, len);

        switch (cmd) {
            case CMD_LEGACY_ADV_UPDATE:
                LOG_HEXDUMP_INF(payload, len, "CMD_LEGACY_ADV_UPDATE");
                memcpy(&svc_data[2], payload, len);
                ad[0].data_len = 2 + len;

                rc = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
                if (rc < 0) {
                    LOG_WRN("Adv update failed: %d", rc);
                }
                break;
            default:
                LOG_WRN("Unknown command: 0x%02x", cmd);
                break;
        }
    }
    return 0;
}
