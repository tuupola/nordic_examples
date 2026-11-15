#include <zephyr/device.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <rid/location.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define GNSS_DEV DEVICE_DT_GET(DT_NODELABEL(gnss))

static int location_from_gnss(rid_location_t *location, const struct gnss_data *data) {
    int rc;

    rc = rid_location_init(location);
    if (rc < 0) {
        return rc;
    }

    /* If we have no fix set location invalid. */
    if (data->info.fix_status == GNSS_FIX_STATUS_NO_FIX) {
        return rid_location_set_coordinates(
            location, RID_LATITUDE_INVALID, RID_LONGITUDE_INVALID
        );
    }

    rc = rid_location_set_coordinates(
        location,
        (double)data->nav_data.latitude / 1000000000.0,
        (double)data->nav_data.longitude / 1000000000.0
    );
    if (rc < 0) {
        return rc;
    }

    rc = rid_location_set_geodetic_altitude(
        location, (float)(data->nav_data.altitude + data->info.geoid_separation) / 1000.0f
    );
    if (rc < 0) {
        return rc;
    }

    rc = rid_location_set_speed(location, (float)data->nav_data.speed / 1000.0f);
    if (rc < 0) {
        return rc;
    }

    if (data->nav_data.speed == 0) {
        rc = rid_location_set_track_direction(location, RID_TRACK_DIRECTION_UNKNOWN);
    } else {
        uint16_t degrees;

        degrees = (uint16_t)((data->nav_data.bearing / 1000U) % 360U);
        rc = rid_location_set_track_direction(location, degrees);
    }
    if (rc < 0) {
        return rc;
    }

    /* If we have GNSS date. */
    if (data->utc.month_day != 0) {
        uint32_t deciseconds;

        deciseconds = (uint32_t)data->utc.minute * 600U;
        deciseconds += (uint32_t)data->utc.millisecond / 100U;
        if (deciseconds <= RID_TIMESTAMP_MAX) {
            rc = rid_location_set_timestamp(location, (uint16_t)deciseconds);
            if (rc < 0) {
                return rc;
            }
        }
    }

    return RID_SUCCESS;
}

static void gnss_data_cb(const struct device *dev, const struct gnss_data *data) {
    int rc;
    static char json[512];
    rid_location_t location;

    ARG_UNUSED(dev);

    rc = location_from_gnss(&location, data);
    if (rc < 0) {
        LOG_ERR("location encode failed: %s", rid_error_to_string(rc));
        return;
    }

    LOG_HEXDUMP_DBG(&location, sizeof(location), "location");

    rc = rid_location_to_json(&location, json, sizeof(json), NULL);
    if (rc < 0) {
        LOG_ERR("location json failed: %s", rid_error_to_string(rc));
        return;
    }

    LOG_INF("%s", json);
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
