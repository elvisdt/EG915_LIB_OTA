#ifndef OTA_INC_OTA_GLOBAL_H_
#define OTA_INC_OTA_GLOBAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "esp_log.h"

//#define DEBUG_OTA_SERIAL Serial
#define DEBUG_OTA_ON  true
void debug_ota(char const *Buff, ...);

#endif /* OTA_INC_OTA_CONFIG_H_ */
