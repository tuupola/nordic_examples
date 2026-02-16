#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

LOG_MODULE_REGISTER(lte_ping, LOG_LEVEL_INF);

#define PING_TARGET "1.1.1.1"
#define PING_TIMEOUT_SEC 5
#define PING_INTERVAL_SEC 1
#define PING_PAYLOAD_SIZE 48

/* ICMP echo types */
#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0

struct icmp_echo_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};

static uint16_t icmp_checksum(const uint8_t *data, size_t len)
{
    uint32_t sum = 0;
    const uint16_t *ptr = (const uint16_t *)data;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(const uint8_t *)ptr;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return (uint16_t)~sum;
}

static int ping(int fd, struct sockaddr_in *address, uint16_t sequence)
{
    uint8_t buffer[sizeof(struct icmp_echo_header) + PING_PAYLOAD_SIZE];
    struct icmp_echo_header *header = (struct icmp_echo_header *)buffer;
    int64_t start;
    int rc;

    /* Build ICMP echo request */
    memset(buffer, 0, sizeof(buffer));
    header->type = ICMP_ECHO_REQUEST;
    header->code = 0;
    header->id = htons(1);
    header->sequence = htons(sequence);

    /* Fill payload with pattern */
    for (int i = 0; i < PING_PAYLOAD_SIZE; i++) {
        buffer[sizeof(struct icmp_echo_header) + i] = (uint8_t)(i & 0xFF);
    }

    header->checksum = icmp_checksum(buffer, sizeof(buffer));

    start = k_uptime_get();

    rc = zsock_sendto(fd, buffer, sizeof(buffer), 0,
                      (struct sockaddr *)address, sizeof(*address));
    if (rc < 0) {
        LOG_ERR("sendto failed: %d", errno);
        return -errno;
    }

    /* Wait for reply */
    rc = zsock_recv(fd, buffer, sizeof(buffer), 0);
    if (rc < 0) {
        if (errno == EAGAIN) {
            LOG_WRN("Ping timeout");
            return -ETIMEDOUT;
        }
        LOG_ERR("recv failed: %d", errno);
        return -errno;
    }

    int64_t rtt = k_uptime_get() - start;

    header = (struct icmp_echo_header *)buffer;
    if (header->type == ICMP_ECHO_REPLY) {
        LOG_INF("%d bytes from %s: seq=%d time=%lld ms",
                rc, PING_TARGET, sequence, rtt);
    } else {
        LOG_WRN("Unexpected ICMP type: %d", header->type);
    }

    return 0;
}

int main(void)
{
    int rc;

    LOG_INF("Initializing the modem firmware.");
    rc = nrf_modem_lib_init();
    if (rc < 0) {
        LOG_ERR("Modem init failed: %d", rc);
        return rc;
    }

    /* This is blocking and can take up to 30 seconds. */
    LOG_INF("Connecting to LTE network.");
    rc = lte_lc_connect();
    if (rc < 0) {
        LOG_ERR("LTE connect failed: %d", rc);
        return rc;
    }
    LOG_INF("Connected to LTE network.");

    /* Set up destination address */
    struct sockaddr_in address;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    rc = zsock_inet_pton(AF_INET, PING_TARGET, &address.sin_addr);
    if (rc != 1) {
        LOG_ERR("Invalid address: %s", PING_TARGET);
        return -EINVAL;
    }

    /* Open ICMP socket */
    int fd = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

    if (fd < 0) {
        LOG_ERR("socket failed: %d", errno);
        return -errno;
    }

    /* Set receive timeout */
    struct zsock_timeval timeout = {
        .tv_sec = PING_TIMEOUT_SEC,
        .tv_usec = 0,
    };

    rc = zsock_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                          &timeout, sizeof(timeout));
    if (rc < 0) {
        LOG_ERR("setsockopt failed: %d", errno);
        zsock_close(fd);
        return -errno;
    }

    LOG_INF("Pinging %s...", PING_TARGET);

    uint16_t sequence = 0;

    while (1) {
        ping(fd, &address, ++sequence);
        k_sleep(K_SECONDS(PING_INTERVAL_SEC));
    }

    zsock_close(fd);
    return 0;
}
