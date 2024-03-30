/*
 * ota_control.h
 *
 *  Created on: 3 set. 2022
 *      Author: Franco
 */

#ifndef OTA_INC_OTA_CONTROL_H_
#define OTA_INC_OTA_CONTROL_H_


#include "ota_headers.h"
#include "ota_esp32.h"

#include <stdio.h>
#include "crc.h"


OTA_EX_ ota_init( void );

/*
  Llama a validate y process
  retorna OK o ERROR
*/
OTA_EX_ ota_flash(uint8_t *data, uint16_t data_len);

/*
  Valida el tipo de trama segun el stado del proceso OTA
  retorna OK o ERROR
*/
OTA_EX_ ota_process(uint8_t *data, uint16_t data_len);
/*
  Valida el CRC
*/
OTA_EX_ ota_validate(uint8_t *data, uint16_t data_len);






#endif /* OTA_INC_OTA_CONTROL_H_ */
