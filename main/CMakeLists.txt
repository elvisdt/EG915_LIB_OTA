set(srcs "main.c"
         "ota_modem.c"
         "json_parser.c")

set(ota_modem_srcs  
    "OTA_MODEM/crc.c"
    "OTA_MODEM/ota_control.c"
    "OTA_MODEM/ota_esp32.c"
    "OTA_MODEM/ota_global.c"
    "OTA_MODEM/ota_headers.c")

set(ota_ble_srcs  
    "OTA_BLE/gap.c"
    "OTA_BLE/gatt_svr.c")

set(EG915_srcs  
    "EG915/EG915_modem.c"
    "EG915/modem_aux.c")


idf_component_register(
    SRCS ${srcs} ${ota_modem_srcs} ${EG915_srcs} ${ota_ble_srcs}
    INCLUDE_DIRS "." "OTA_MODEM" "EG915" "OTA_BLE"
)

