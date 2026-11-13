#ifndef BRID_TX_H
#define BRID_TX_H

#include <stdint.h>

/**
 * Initialize the UART transport to the nRF5340 BLE radio.
 *
 * @return 0 on success, negative errno on failure.
 */
int brid_tx_init(void);

/**
 * Send a single 25-byte RID message as a BLE Legacy advertising
 * update over the UART link.
 *
 * @param msg Pointer to a 25-byte RID message.
 * @param counter Incrementing message counter.
 * @return 0 on success, negative errno on failure.
 */
int brid_tx_send_legacy(const void *msg, uint8_t counter);

#endif /* BRID_TX_H */
