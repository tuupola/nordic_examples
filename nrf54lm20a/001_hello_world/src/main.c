#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hello_world, LOG_LEVEL_INF);

int main(void) {
    LOG_INF("board=%s soc=%s", CONFIG_BOARD, CONFIG_SOC);

    while (1) {
        LOG_INF("Hello world!");
        /* k_ prefix denotes kernel */
        k_sleep(K_MSEC(1000));
    }
    return 0;
}
