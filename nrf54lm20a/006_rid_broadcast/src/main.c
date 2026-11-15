#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <rid/basic_id.h>
#include <rid/location.h>
#include <rid/message_pack.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define GNSS_DEV DEVICE_DT_GET(DT_NODELABEL(gnss))

static rid_message_pack_t message_pack;
static rid_location_t location;

static int update_location(rid_location_t *location, const struct gnss_data *data) {
    int rc;

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
    uint8_t index;
    static char json[1024];

    ARG_UNUSED(dev);

    /* Update the location message with data from GNSS. */
    rc = update_location(&location, data);
    if (rc < 0) {
        LOG_ERR("Location update failed: %s", rid_error_to_string(rc));
        return;
    }

    /* Replace the location message in the pack. */
    rid_message_pack_find_message_index_by_type(
        &message_pack, RID_MESSAGE_TYPE_LOCATION, 0, &index
    );
    rid_message_pack_set_message_at(&message_pack, index, &location);

    LOG_HEXDUMP_DBG(&message_pack, rid_message_pack_size(&message_pack), "message_pack");

    rid_message_pack_to_json(&message_pack, json, sizeof(json), NULL);
    LOG_INF("%s", json);
}

GNSS_DATA_CALLBACK_DEFINE(GNSS_DEV, gnss_data_cb);

int main(void) {
    rid_basic_id_t basic_id;

    rid_message_pack_init(&message_pack);

    rid_basic_id_init(&basic_id);
    rid_basic_id_set_type(&basic_id, RID_ID_TYPE_SERIAL_NUMBER);
    rid_basic_id_set_ua_type(&basic_id, RID_UA_TYPE_HELICOPTER_OR_MULTIROTOR);
    rid_basic_id_set_uas_id(&basic_id, "1ABCD2345EF678XYZ", strlen("1ABCD2345EF678XYZ"));
    rid_message_pack_add_message(&message_pack, &basic_id);

    rid_location_init(&location);
    rid_message_pack_add_message(&message_pack, &location);

    if (!device_is_ready(GNSS_DEV)) {
        LOG_ERR("%s not ready", GNSS_DEV->name);
        return -ENODEV;
    }

    LOG_INF("GNSS ready");
    return 0;
}
