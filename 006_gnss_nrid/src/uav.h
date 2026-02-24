#ifndef UAV_H
#define UAV_H

#include <nrf_modem_gnss.h>
#include <rid/rid.h>

void uav_basic_id_get(void);
void uav_location_update(const struct nrf_modem_gnss_pvt_data_frame *pvt);
void uav_system_update(const struct nrf_modem_gnss_pvt_data_frame *pvt);

#endif /* UAV_H */
