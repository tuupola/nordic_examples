#define MAVLINK_USE_MESSAGE_INFO

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>

#include <common/mavlink.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define EXAMPLE_SYS_ID 1
#define EXAMPLE_COMP_ID MAV_COMP_ID_ONBOARD_COMPUTER /* 191 */

/* DMA buffers for the UART driver */
#define RX_BUF_SIZE 256
#define RX_TIMEOUT_US 100000

/* Used for decoupling ISR from the main thread */
#define RING_BUF_SIZE 1024
RING_BUF_DECLARE(rx_ring, RING_BUF_SIZE);

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

/* DMA buffers for the UART driver */
static uint8_t tx_buf[MAVLINK_MAX_PACKET_LEN];
static uint8_t rx_buf[2][RX_BUF_SIZE];
static uint8_t buf_idx;

static mavlink_message_t message;
static mavlink_status_t status;

static void process_mavlink(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (mavlink_parse_char(MAVLINK_COMM_0, data[i], &message, &status)) {
            const mavlink_message_info_t *info = mavlink_get_message_info(&message);
            if (info) {
                LOG_INF(
                    "%s id=%d from sys=%d comp=%d",
                    info->name,
                    message.msgid,
                    message.sysid,
                    message.compid
                );
            } else {
                LOG_WRN("Unknown message ID %d", message.msgid);
            }
        }
    }
}

static void uart_callback(
    const struct device *dev, struct uart_event *event, void *user_data
) {
    switch (event->type) {
        case UART_RX_RDY:
            ring_buf_put(
                &rx_ring, event->data.rx.buf + event->data.rx.offset, event->data.rx.len
            );
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

        case UART_TX_DONE:
            break;

        case UART_TX_ABORTED:
            LOG_WRN("TX aborted");
            break;

        default:
            break;
    }
}

static int send_heartbeat(void) {
    mavlink_message_t msg;
    uint16_t len;

    mavlink_msg_heartbeat_pack(
        EXAMPLE_SYS_ID,
        EXAMPLE_COMP_ID,
        &msg,
        MAV_TYPE_ONBOARD_CONTROLLER,
        MAV_AUTOPILOT_INVALID,
        0, /* base_mode */
        0, /* custom_mode */
        MAV_STATE_ACTIVE
    );

    len = mavlink_msg_to_send_buffer(tx_buf, &msg);
    LOG_INF("HEARTBEAT bytes=%u sys=%d comp=%d", len, EXAMPLE_SYS_ID, EXAMPLE_COMP_ID);
    return uart_tx(uart_dev, tx_buf, len, SYS_FOREVER_US);
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

    LOG_INF("UART1 TX=P0.18, RX=P0.19 at 460800 baud, heartbeat 1 Hz");

    while (1) {
        static uint8_t tmp[256];
        uint32_t len = ring_buf_get(&rx_ring, tmp, sizeof(tmp));
        if (len > 0) {
            process_mavlink(tmp, len);
        }

        uint32_t used = ring_buf_size_get(&rx_ring);
        uint32_t free = ring_buf_space_get(&rx_ring);
        LOG_INF("RX ring: %u/%u bytes used", used, RING_BUF_SIZE);

        if (used > RING_BUF_SIZE * 3 / 4) {
            LOG_WRN("RX buffer 75%% full");
        }

        rc = send_heartbeat();
        if (rc < 0) {
            LOG_WRN("Heartbeat TX failed: %d", rc);
        }
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
