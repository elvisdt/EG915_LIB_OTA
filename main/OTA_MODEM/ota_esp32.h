

#ifndef OTA_INC_OTA_STM32F4_H_
#define OTA_INC_OTA_STM32F4_H_

#include <stdbool.h>
#include <stdio.h>
#include "ota_headers.h"

#include "esp_ota_ops.h"
#include <esp_task_wdt.h>


/*
  manejae el estado con la variable ota y guarda el .bin en el esp32
  retorna HAL_OK o HAL_ERROR
*/

HAL_StatusTypeDef write_data_to_flash_app( uint8_t *data,
		   uint16_t data_len,
		   bool is_first_block,
		   ota_t *ota
		  );

#endif /* OTA_INC_OTA_STM32F4_H_ */
