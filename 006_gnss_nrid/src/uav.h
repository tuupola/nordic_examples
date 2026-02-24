#ifndef UAV_H
#define UAV_H

#include <nrf_modem_gnss.h>
#include <rid/rid.h>

void uav_basic_id_update(rid_basic_id_t *message);
void uav_location_update(rid_location_t *location, const struct nrf_modem_gnss_pvt_data_frame *pvt);
void uav_system_update(rid_system_t *system, const struct nrf_modem_gnss_pvt_data_frame *pvt);

#endif /* UAV_H */
