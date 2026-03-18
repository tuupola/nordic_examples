#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(segger_rtt, LOG_LEVEL_INF);

int main(void) {
    while (1) {
        LOG_INF("Running on %s (%s)", CONFIG_BOARD, CONFIG_SOC);
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
