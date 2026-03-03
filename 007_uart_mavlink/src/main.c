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
#define RING_BUF_SIZE 512
RING_BUF_DECLARE(rx_ring, RING_BUF_SIZE);

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

/* DMA buffers for the UART driver */
static uint8_t tx_buf[MAVLINK_MAX_PACKET_LEN];
static uint8_t rx_buf[2][RX_BUF_SIZE];
static uint8_t buf_idx;

static mavlink_message_t message;
static mavlink_status_t status;
static bool toggle;

static void process_mavlink(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (mavlink_parse_char(MAVLINK_COMM_0, data[i], &message, &status)) {
            const mavlink_message_info_t *info = mavlink_get_message_info(&message);
            if (info) {
                LOG_INF(
                    "RX: %s id=%d from sys=%d comp=%d",
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
            /* Move MAVLink parsing out of ISR context */
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

    uint16_t len = mavlink_msg_to_send_buffer(tx_buf, &msg);
    LOG_INF("TX: HEARTBEAT id=0 sys=%d comp=%d", EXAMPLE_SYS_ID, EXAMPLE_COMP_ID);
    return uart_tx(uart_dev, tx_buf, len, SYS_FOREVER_US);
}

static int send_radio_status(void) {
    mavlink_message_t msg;
    uint8_t txbuf_free = (ring_buf_space_get(&rx_ring) * 100) / RING_BUF_SIZE;

    mavlink_msg_radio_status_pack(
        EXAMPLE_SYS_ID,
        EXAMPLE_COMP_ID,
        &msg,
        255,
        255,
        txbuf_free,
        255,
        255,
        status.packet_rx_drop_count,
        0
    );

    uint16_t len = mavlink_msg_to_send_buffer(tx_buf, &msg);
    if (txbuf_free < 25) {
        LOG_WRN(
            "TX: RADIO_STATUS id=109 sys=%d comp=%d txbuf=%d rxerrors=%d",
            EXAMPLE_SYS_ID,
            EXAMPLE_COMP_ID,
            txbuf_free,
            status.packet_rx_drop_count
        );
    } else {
        LOG_INF(
            "TX: RADIO_STATUS id=109 sys=%d comp=%d txbuf=%d rxerrors=%d",
            EXAMPLE_SYS_ID,
            EXAMPLE_COMP_ID,
            txbuf_free,
            status.packet_rx_drop_count
        );
    }
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

    LOG_INF("UART1 TX=P0.18, RX=P0.19 at 460800 baud, 2 Hz loop");

    while (1) {
        static uint8_t tmp[256];
        uint32_t len = ring_buf_get(&rx_ring, tmp, sizeof(tmp));
        if (len > 0) {
            process_mavlink(tmp, len);
        }

        /* Stupid toggling for demonstration purposes only */
        if (toggle) {
            rc = send_heartbeat();
            if (rc < 0) {
                LOG_WRN("Heartbeat TX failed: %d", rc);
            }
        } else {
            rc = send_radio_status();
            if (rc < 0) {
                LOG_WRN("Radio status TX failed: %d", rc);
            }
        }
        toggle = !toggle;

        k_sleep(K_MSEC(500));
    }
    return 0;
}
