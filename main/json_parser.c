
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON.h"
#include "main.h"

//-------------------------------------//


int js_modem_to_str(const modem_gsm_t modem, char* buffer){
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        printf("Error al crear el objeto JSON\n");
        return -1;
    }
    cJSON_AddStringToObject(root, "iccid",   modem.info.iccid);
    cJSON_AddStringToObject(root, "code",    modem.code);
    cJSON_AddNumberToObject(root, "signal",  modem.signal);
    cJSON_AddNumberToObject(root, "time",    modem.time);
    
    char *json = cJSON_PrintUnformatted(root);
    sprintf(buffer,"%s\r\n",json);
    cJSON_Delete(root);
    
    free(json);
    json=NULL;

    return 0;
}



int js_str_to_ble(const char *json_string, cfg_ble_t *ble_config) {
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON: %s\n", error_ptr);
        }
        return -1;
    }

    cJSON *ble_item = cJSON_GetObjectItem(root, "ble");
    if (ble_item != NULL) {
        cJSON *mac_item = cJSON_GetObjectItem(ble_item, "mac");
        cJSON *name_item = cJSON_GetObjectItem(ble_item, "name");
        cJSON *tmax_item = cJSON_GetObjectItem(ble_item, "Tmax");
        cJSON *tmin_item = cJSON_GetObjectItem(ble_item, "Tmin");
        
        if (mac_item && name_item && tmax_item && tmin_item) {
            strncpy(ble_config->mac, mac_item->valuestring, sizeof(ble_config->mac));
            strncpy(ble_config->name, name_item->valuestring, sizeof(ble_config->name));
            ble_config->tem_max = (float)tmax_item->valuedouble;
            ble_config->tem_min = (float)tmin_item->valuedouble;
            cJSON_Delete(root);
            return 0; // Se encontraron todos los campos
        }
    }

    cJSON_Delete(root);
    return -1; // No se encontraron todos los campos
}
