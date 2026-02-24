#include <zephyr/logging/log.h>

#include "uav.h"

LOG_MODULE_REGISTER(uav, LOG_LEVEL_INF);

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
