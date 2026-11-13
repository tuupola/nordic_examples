#ifndef UAV_H
#define UAV_H

#include <nrf_modem_gnss.h>
#include <rid/rid.h>

void uav_location_update(const struct nrf_modem_gnss_pvt_data_frame *pvt);
void uav_system_update(const struct nrf_modem_gnss_pvt_data_frame *pvt);

void uav_basic_id_get(rid_basic_id_t *msg);
void uav_location_get(rid_location_t *msg);
void uav_system_get(rid_system_t *msg);
void uav_message_pack_get(rid_message_pack_t *pack);

#endif /* UAV_H */
