	/*
 * ota_usb.h
 *
 *  Created on: 3 set. 2022
 *      Author: Franco
 */

#ifndef _OTA_INC_OTA_USB_H_
#define _OTA_INC_OTA_USB_H_


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
OTA_EX_ Ota_UartControl_Modem(void);

/*
  Lee los mensajes OTA
*/
uint16_t Ota_UartRead_Modem( uint8_t *data, uint16_t max_len);

OTA_EX_ Ota_UartSend_Resp_Modem(uint8_t ans);


#endif /* OTA_INC_OTA_USB_H_ */
