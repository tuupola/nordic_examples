#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>

#include "lwgps/lwgps.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* DMA buffers for the UART driver */
#define RX_BUF_SIZE 2048
#define RX_TIMEOUT_US 10000

/* Used for decoupling ISR from the main thread */
#define RING_BUF_SIZE 2048
RING_BUF_DECLARE(rx_ring, RING_BUF_SIZE);

static K_SEM_DEFINE(rx_sem, 0, 1);

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

/* DMA buffers for the UART driver */
//static uint8_t tx_buf[512];
static uint8_t rx_buf[2][RX_BUF_SIZE];
static uint8_t buf_idx;
lwgps_t gps;

static void process_gnss(const uint8_t *data, size_t len) {
    LOG_HEXDUMP_DBG(data, len, "GNSS");

    lwgps_process(&gps, data, len);
    LOG_INF(
        "lat=%.6f lon=%.6f fix=%d sats=%d",
        gps.latitude,
        gps.longitude,
        gps.fix,
        gps.sats_in_use
    );
}

static void
uart_callback(const struct device *dev, struct uart_event *event, void *user_data) {
    uint32_t written;

    switch (event->type) {
        case UART_RX_RDY:
            /* Move GNSS parsing out of ISR context */
            written = ring_buf_put(
                &rx_ring, event->data.rx.buf + event->data.rx.offset, event->data.rx.len
            );
            /* Warn if ringbuffer is full. */
            if (written < event->data.rx.len) {
                LOG_WRN(
                    "Ringbuffer overflow, dropped %u bytes", event->data.rx.len - written
                );
            }
            /* Release semaphosre so main() can continue. */
            k_sem_give(&rx_sem);
            break;

        case UART_RX_BUF_REQUEST:
            LOG_DBG("UART_RX_BUF_REQUEST");
            buf_idx = (buf_idx + 1) % 2;
            uart_rx_buf_rsp(dev, rx_buf[buf_idx], RX_BUF_SIZE);
            break;

        case UART_RX_DISABLED:
            /* Re-enable after timeout or error. */
            LOG_WRN("UART_RX_DISABLED, re-enabling");
            uart_rx_enable(dev, rx_buf[0], RX_BUF_SIZE, RX_TIMEOUT_US);
            buf_idx = 0;
            break;

        case UART_TX_DONE:
            LOG_DBG("UART_TX_DONE");
            break;

        case UART_TX_ABORTED:
            LOG_WRN("UART_TX_ABORTED");
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

    lwgps_init(&gps);

    LOG_INF("UART1 TX=P0.18, RX=P0.19 at 115200 baud");

    /* Main loop processes GNSS messages from ring buffer  */
    while (1) {
        static uint8_t tmp[RX_BUF_SIZE];
        /* Wait for UART_RX_RDY to give semaphore. */
        k_sem_take(&rx_sem, K_FOREVER);
        uint32_t len = ring_buf_get(&rx_ring, tmp, sizeof(tmp));
        if (len > 0) {
            LOG_INF("ring_buf_get() %u bytes", len);
            process_gnss(tmp, len);
        }
    }
    return 0;
}
