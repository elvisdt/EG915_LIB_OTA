	/*
 * ota_usb.h
 *
 *  Created on: 3 set. 2022
 *      Author: Franco
 */

#ifndef OTA_INC_OTA_USB_H_
#define OTA_INC_OTA_USB_H_


#include <stdbool.h>
#include <stdio.h>
#include "esp_timer.h"

#include "ota_headers.h"
#include "ota_control.h"
#include "EG915_modem.h"


/*
  Punto de acceso al proceso OTA desde el programa principal.
  * bucle que lee espera cada mensaje ota.
*/
OTA_EX_ ota_uartControl_M95(void);

/*
  Lee los mensajes OTA
*/
uint16_t ota_uart_read_M95( uint8_t *data, uint16_t max_len);

OTA_EX_ ota_uart_send_resp_M95(uint8_t ans);


#endif /* OTA_INC_OTA_USB_H_ */
