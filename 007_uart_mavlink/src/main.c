#define MAVLINK_USE_MESSAGE_INFO

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <common/mavlink.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define RX_BUF_SIZE 256
#define RX_TIMEOUT_US 100000 /* 100 ms idle timeout */

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

static uint8_t rx_buf[2][RX_BUF_SIZE];
static uint8_t buf_idx;

static mavlink_message_t mavlink_msg;
static mavlink_status_t mavlink_status;

static void process_mavlink(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (mavlink_parse_char(MAVLINK_COMM_0, data[i], &mavlink_msg, &mavlink_status)) {
            const mavlink_message_info_t *info =
                mavlink_get_message_info(&mavlink_msg);
            if (info) {
                LOG_INF("%s (ID %d) from sys=%d comp=%d",
                    info->name, mavlink_msg.msgid, mavlink_msg.sysid, mavlink_msg.compid
                );
            } else {
                LOG_WRN("Unknown message ID %d", mavlink_msg.msgid);
            }
        }
    }
}

static void uart_callback(const struct device *dev, struct uart_event *event, void *user_data) {
    switch (event->type) {
        case UART_RX_RDY:
            process_mavlink(event->data.rx.buf + event->data.rx.offset, event->data.rx.len);
            break;

        case UART_RX_BUF_REQUEST:
            buf_idx = (buf_idx + 1) % 2;
            uart_rx_buf_rsp(dev, rx_buf[buf_idx], RX_BUF_SIZE);
            break;

        case UART_RX_DISABLED:
            /* Re-enable after timeout or error. */
            LOG_WRN("RX disabled, re-enabling");
            uart_rx_enable(dev, rx_buf[0], RX_BUF_SIZE, RX_TIMEOUT_US);
            buf_idx = 0;
            break;

        default:
            break;
    }
}

int main(void) {
    int rc;

    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART1 device not ready");
        return -ENODEV;
    }

    rc = uart_callback_set(uart_dev, uart_callback, NULL);
    if (rc < 0) {
        LOG_ERR("Failed to set UART callback: %d", rc);
        return rc;
    }

    buf_idx = 0;
    rc = uart_rx_enable(uart_dev, rx_buf[0], RX_BUF_SIZE, RX_TIMEOUT_US);
    if (rc < 0) {
        LOG_ERR("Failed to enable UART RX: %d", rc);
        return rc;
    }

    LOG_INF("Listening on UART1 (P0.18 RX / P0.19 TX) at 460800 baud");

    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}
