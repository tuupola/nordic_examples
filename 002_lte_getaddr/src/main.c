#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

LOG_MODULE_REGISTER(lte_getaddr, LOG_LEVEL_INF);

int main(void)
{
    int rc;
    struct zsock_addrinfo *result;
    struct zsock_addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    char ip_str[NET_IPV4_ADDR_LEN];

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

    LOG_INF("Resolving www.appelsiini.net.");
    rc = zsock_getaddrinfo("www.appelsiini.net", NULL, &hints, &result);
    if (rc != 0) {
        LOG_ERR("getaddrinfo failed: %d", rc);
        return rc;
    }

    struct sockaddr_in *sockaddr = (struct sockaddr_in *)result->ai_addr;
    net_addr_ntop(AF_INET, &sockaddr->sin_addr, ip_str, sizeof(ip_str));
    LOG_INF("IP: %s", ip_str);

    zsock_freeaddrinfo(result);

    while (1) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
