#include <zephyr/logging/log.h>
#include <zephyr/sys/timeutil.h>

#include "uav.h"

LOG_MODULE_REGISTER(uav, LOG_LEVEL_INF);

int uav_basic_id_update(rid_basic_id_t *message) {
    rid_basic_id_init(message);
    rid_basic_id_set_type(message, RID_ID_TYPE_SERIAL_NUMBER);
    rid_basic_id_set_ua_type(message, RID_UA_TYPE_HELICOPTER_OR_MULTIROTOR);
    rid_basic_id_set_uas_id(message, "1ABCD2345EF678XYZ");

    LOG_HEXDUMP_INF(message, sizeof(*message), "Basic ID");

    return 0;
}

int uav_location_update(rid_location_t *location, const struct nrf_modem_gnss_pvt_data_frame *pvt) {
    /* Create a location Remote ID message. */
    rid_location_init(location);
    rid_location_set_operational_status(location, RID_OPERATIONAL_STATUS_AIRBORNE);
    rid_location_set_height_type(location, RID_HEIGHT_TYPE_AGL);
    rid_location_set_latitude(location, pvt->latitude);
    rid_location_set_longitude(location, pvt->longitude);
    rid_location_set_geodetic_altitude(location, pvt->altitude);
    rid_location_set_speed(location, pvt->speed);
    rid_location_set_vertical_speed(location, pvt->vertical_speed);
    rid_location_set_track_direction(location, (uint16_t)pvt->heading);

    LOG_HEXDUMP_INF(location, sizeof(*location), "Location");

    return 0;
}

int uav_system_update(rid_system_t *system, const struct nrf_modem_gnss_pvt_data_frame *pvt) {
    struct tm tm = {
        .tm_year = pvt->datetime.year - 1900,
        .tm_mon = pvt->datetime.month - 1,
        .tm_mday = pvt->datetime.day,
        .tm_hour = pvt->datetime.hour,
        .tm_min = pvt->datetime.minute,
        .tm_sec = pvt->datetime.seconds,
    };

    rid_system_init(system);
    rid_system_set_operator_location_type(
        system, RID_OPERATOR_LOCATION_TYPE_TAKEOFF
    );
    rid_system_set_classification_type(
        system, RID_CLASSIFICATION_TYPE_UNDECLARED
    );
    rid_system_set_operator_latitude(system, pvt->latitude);
    rid_system_set_operator_longitude(system, pvt->longitude);
    rid_system_set_area_count(system, 1);
    rid_system_set_area_radius(system, 0);
    rid_system_set_area_ceiling(system, pvt->altitude + 50.0f);
    rid_system_set_area_floor(system, 0.0f);
    int64_t unixtime = timeutil_timegm(&tm);
    rid_system_set_unixtime(system, (uint32_t)unixtime);

    LOG_HEXDUMP_INF(system, sizeof(*system), "System");

    return 0;
}
