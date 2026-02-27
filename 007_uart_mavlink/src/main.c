#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define RX_BUF_SIZE 256
#define RX_TIMEOUT_US 100000 /* 100 ms idle timeout */

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

static uint8_t rx_buf[2][RX_BUF_SIZE];
static uint8_t buf_idx;

static void uart_callback(const struct device *dev, struct uart_event *event, void *user_data) {
    switch (event->type) {
        case UART_RX_RDY:
            LOG_HEXDUMP_INF(event->data.rx.buf + event->data.rx.offset, event->data.rx.len, "RX");
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
