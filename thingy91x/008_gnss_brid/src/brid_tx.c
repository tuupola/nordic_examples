/*
 * UART transport for sending RID advertising payloads to the nRF5340.
 *
 * Frame format:
 *   [0xAA] [0x55] [cmd] [len] [payload...]
 *
 * Legacy payload (27 bytes):
 *   [app_code=0x0D] [counter] [25 bytes RID message]
 */

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <rid/transport.h>

#include "brid_tx.h"

LOG_MODULE_REGISTER(brid_tx, LOG_LEVEL_INF);

#define SYNC_0 0xAA /* 0b10101010 */
#define SYNC_1 0x55 /* 0b01010101 */

#define CMD_LEGACY_ADV_UPDATE 0x01

#define RID_MESSAGE_SIZE 25

static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

static void uart_write_buffer(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}

int brid_tx_init(void) {
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART1 device not ready");
        return -ENODEV;
    }
    LOG_INF("UART1 transport ready.");
    return 0;
}

int brid_tx_send_legacy(const void *msg, uint8_t counter) {
    /* Payload: app_code (1) + counter (1) + message (25) = 27 bytes. */
    const uint8_t payload_len = 1 + 1 + RID_MESSAGE_SIZE;
    uint8_t header[4] = {
        SYNC_0,
        SYNC_1,
        CMD_LEGACY_ADV_UPDATE,
        payload_len,
    };
    uint8_t payload_header[2] = {
        RID_TRANSPORT_BLUETOOTH_APP_CODE, /* 0x0D */
        counter,
    };

    uart_write_buffer(header, sizeof(header));
    uart_write_buffer(payload_header, sizeof(payload_header));
    uart_write_buffer((const uint8_t *)msg, RID_MESSAGE_SIZE);

    return 0;
}
