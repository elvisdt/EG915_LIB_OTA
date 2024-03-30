set(srcs "main.c"
         "ota_modem.c")

set(ota_srcs  
    "OTA/crc.c"
    "OTA/ota_control.c"
    "OTA/ota_esp32.c"
    "OTA/ota_global.c"
    "OTA/ota_headers.c")

set(EG915_srcs  
    "EG915/EG915_modem.c"
    "EG915/Modem_aux.c")

idf_component_register(
    SRCS ${srcs} ${ota_srcs} ${EG915_srcs}
    INCLUDE_DIRS "." "OTA" "EG915"
)

