#include <zephyr/device.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define GNSS_DEV DEVICE_DT_GET(DT_NODELABEL(gnss))

static void gnss_data_cb(const struct device *dev, const struct gnss_data *data) {
    ARG_UNUSED(dev);

    LOG_INF(
        "lat=%lld lon=%lld fix=%d sats=%u",
        data->nav_data.latitude,
        data->nav_data.longitude,
        data->info.fix_status,
        data->info.satellites_cnt
    );
}

GNSS_DATA_CALLBACK_DEFINE(GNSS_DEV, gnss_data_cb);

int main(void) {
    if (!device_is_ready(GNSS_DEV)) {
        LOG_ERR("%s not ready", GNSS_DEV->name);
        return -ENODEV;
    }

    LOG_INF("GNSS ready");
    return 0;
}
