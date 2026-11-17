#include <string.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <rid/basic_id.h>
#include <rid/location.h>
#include <rid/message_pack.h>
#include <rid/operator_id.h>
#include <rid/transport.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define GNSS_DEV DEVICE_DT_GET(DT_NODELABEL(gnss))

static rid_message_pack_t message_pack;

/* Everything before message is the header. */
#define SVC_DATA_HEADER_SIZE RID_TRANSPORT_BT_MESSAGE_OFFSET

/* Reserve space for the header and max size message pack. */
static uint8_t svc_data[SVC_DATA_HEADER_SIZE + RID_MESSAGE_PACK_MAX_SIZE] = {
    RID_TRANSPORT_BT_SERVICE_UUID_LO,
    RID_TRANSPORT_BT_SERVICE_UUID_HI,
    RID_TRANSPORT_BT_APP_CODE,
    0x00,
};

/* Put header only svc data to the ad array. */
static struct bt_data ad[] = {
    BT_DATA(BT_DATA_SVC_DATA16, svc_data, SVC_DATA_HEADER_SIZE),
};

static struct bt_le_ext_adv *adv;

static int update_location(const struct gnss_data *data) {
    int rc;
    uint8_t index;
    rid_location_t location;

    /* Find the index of the location message. We assume it exists. */
    rc = rid_message_pack_find_message_index_by_type(
        &message_pack, RID_MESSAGE_TYPE_LOCATION, 0, &index
    );
    if (rc < 0) {
        return rc;
    }

    /* Make a copy we can work on. */
    rc = rid_message_pack_copy_message_at(&message_pack, index, &location);
    if (rc < 0) {
        return rc;
    }

    /* If we have no fix set location invalid. */
    if (data->info.fix_status == GNSS_FIX_STATUS_NO_FIX) {
        rc = rid_location_set_coordinates(
            &location, RID_LATITUDE_INVALID, RID_LONGITUDE_INVALID
        );
        if (rc < 0) {
            return rc;
        }

        /* Replace the location message in the pack. */
        return rid_message_pack_set_message_at(&message_pack, index, &location);
    }

    rc = rid_location_set_coordinates(
        &location,
        (double)data->nav_data.latitude / 1000000000.0,
        (double)data->nav_data.longitude / 1000000000.0
    );
    if (rc < 0) {
        return rc;
    }

    rc = rid_location_set_geodetic_altitude(
        &location,
        (float)(data->nav_data.altitude + data->info.geoid_separation) / 1000.0f
    );
    if (rc < 0) {
        return rc;
    }

    rc = rid_location_set_speed(&location, (float)data->nav_data.speed / 1000.0f);
    if (rc < 0) {
        return rc;
    }

    if (data->nav_data.speed == 0) {
        rc = rid_location_set_track_direction(&location, RID_TRACK_DIRECTION_UNKNOWN);
    } else {
        uint16_t degrees;

        degrees = (uint16_t)((data->nav_data.bearing / 1000U) % 360U);
        rc = rid_location_set_track_direction(&location, degrees);
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
            rc = rid_location_set_timestamp(&location, (uint16_t)deciseconds);
            if (rc < 0) {
                return rc;
            }
        }
    }

    /* Replace the location message in the pack. */
    return rid_message_pack_set_message_at(&message_pack, index, &location);
}

static int update_advertising(void) {
    int rc;
    size_t pack_size;

    if (adv == NULL) {
        return 0;
    }

    /* Copy message pack data into service data. */
    pack_size = rid_message_pack_size(&message_pack);
    memcpy(&svc_data[RID_TRANSPORT_BT_MESSAGE_OFFSET], &message_pack, pack_size);
    ad[0].data_len = SVC_DATA_HEADER_SIZE + pack_size;
    svc_data[RID_TRANSPORT_BT_COUNTER_OFFSET]++;

    /* Tell bluetooth to start using the new data. */
    rc = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
    if (rc < 0) {
        LOG_ERR("bt_le_ext_adv_set_data() failed: %d", rc);
        return rc;
    }

    return 0;
}

static void gnss_data_cb(const struct device *dev, const struct gnss_data *data) {
    int rc;
    static char json[1024];

    ARG_UNUSED(dev);

    /* Update the location message in the message pack. */
    rc = update_location(data);
    if (rc < 0) {
        LOG_ERR("Location update failed: %s", rid_error_to_string(rc));
        return;
    }

    /* Tell bluetooth to use the updated message pack. */
    rc = update_advertising();
    if (rc < 0) {
        return;
    }

    LOG_HEXDUMP_DBG(&message_pack, rid_message_pack_size(&message_pack), "message_pack");

    rid_message_pack_to_json(&message_pack, json, sizeof(json), NULL);
    LOG_INF("%s", json);
}

GNSS_DATA_CALLBACK_DEFINE(GNSS_DEV, gnss_data_cb);

static int start_advertising(void) {
    int rc;
    const struct bt_le_adv_param *param = BT_LE_ADV_PARAM(
        BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_CODED,
        BT_GAP_ADV_SLOW_INT_MIN,
        BT_GAP_ADV_SLOW_INT_MAX,
        NULL
    );

    rc = bt_le_ext_adv_create(param, NULL, &adv);
    if (rc < 0) {
        LOG_ERR("bt_le_ext_adv_create() failed: %d", rc);
        return rc;
    }

    rc = update_advertising();
    if (rc < 0) {
        return rc;
    }

    rc = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
    if (rc < 0) {
        LOG_ERR("bt_le_ext_adv_start(): %d", rc);
        return rc;
    }

    return 0;
}

int main(void) {
    int rc;
    rid_basic_id_t basic_id;
    rid_operator_id_t operator_id;
    rid_location_t location;

    rid_message_pack_init(&message_pack);

    rid_basic_id_init(&basic_id);
    rid_basic_id_set_type(&basic_id, RID_ID_TYPE_SERIAL_NUMBER);
    rid_basic_id_set_ua_type(&basic_id, RID_UA_TYPE_HELICOPTER_OR_MULTIROTOR);
    rid_basic_id_set_uas_id(&basic_id, "DEADBEEFDEADBEEF", strlen("DEADBEEFDEADBEEF"));
    rid_message_pack_add_message(&message_pack, &basic_id);

    rid_location_init(&location);
    rid_message_pack_add_message(&message_pack, &location);

    rid_operator_id_init(&operator_id);
    rid_operator_id_set_type(&operator_id, RID_OPERATOR_ID_TYPE_DEFAULT);
    rid_operator_id_set(&operator_id, "FIN87astrdge12k8");
    rid_message_pack_add_message(&message_pack, &operator_id);

    if (!device_is_ready(GNSS_DEV)) {
        LOG_ERR("%s not ready", GNSS_DEV->name);
        return -ENODEV;
    }
    LOG_INF("GNSS ready");

    rc = bt_enable(NULL);
    if (rc < 0) {
        LOG_ERR("bt_enable() failed: %d", rc);
        return rc;
    }
    LOG_INF("Bluetooth initialized.");

    rc = start_advertising();
    if (rc < 0) {
        return rc;
    }
    LOG_INF("Extended advertising started");

    return 0;
}
