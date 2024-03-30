#pragma once

#include "esp_ota_ops.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"


/****************************************************
 * DEFINES
*****************************************************/
#define LOG_TAG_GATT_SVR "gatt_svr"
#define REBOOT_DEEP_SLEEP_TIMEOUT   500
#define GATT_DEVICE_INFO_UUID       0x180A
#define GATT_MANUFACTURER_NAME_UUID 0x2A29
#define GATT_MODEL_NUMBER_UUID      0x2A24

/*--> EXTERNAL VARIALBLE <--*/
 extern  bool ota_updating;

/****************************************************
 * ESTRUCUTURES
*****************************************************/
typedef enum {
  SVR_CHR_OTA_CONTROL_NOP,
  SVR_CHR_OTA_CONTROL_REQUEST,
  SVR_CHR_OTA_CONTROL_REQUEST_ACK,
  SVR_CHR_OTA_CONTROL_REQUEST_NAK,
  SVR_CHR_OTA_CONTROL_DONE,
  SVR_CHR_OTA_CONTROL_DONE_ACK,
  SVR_CHR_OTA_CONTROL_DONE_NAK,
} svr_chr_ota_control_val_t;

// service: OTA Service
// f505f04b-2066-5069-8775-830fcfc57339
static const ble_uuid128_t gatt_svr_svc_ota_uuid =
    BLE_UUID128_INIT(0x39, 0x73, 0xc5, 0xcf, 0x0f, 0x83, 0x75, 0x87, 0x69, 0x50,
                     0x66, 0x20, 0x4b, 0xf0, 0x05, 0xf5);

// characteristic: OTA Control
// 834bb43d-8419-5109-b6a4-a0da03786bc6
static const ble_uuid128_t gatt_svr_chr_ota_control_uuid =
    BLE_UUID128_INIT(0xc6, 0x6b, 0x78, 0x03, 0xda, 0xa0, 0xa4, 0xb6, 0x09, 0x51,
                     0x19, 0x84, 0x3d, 0xb4, 0x4b, 0x83);

// characteristic: OTA Data
// bdda975f-9e48-5c04-b67e-f017f019b150
static const ble_uuid128_t gatt_svr_chr_ota_data_uuid =
    BLE_UUID128_INIT(0x50, 0xb1, 0x19, 0xf0, 0x17, 0xf0, 0x7e, 0xb6, 0x04, 0x5c,
                     0x48, 0x9e, 0x5f, 0x97, 0xda, 0xbd);


void gatt_svr_init();