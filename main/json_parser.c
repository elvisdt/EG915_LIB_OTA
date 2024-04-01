
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON.h"
#include "main.h"

//-------------------------------------//


int modem_info_to_json(const modem_gsm_t modem, char* buffer){
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        printf("Error al crear el objeto JSON\n");
        return 0;
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

    return 1;
}

void parse_json_example(const char *json_string) {
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON: %s\n", error_ptr);
        }
        return;
    }

    // Obtener el valor del campo "value"
    cJSON *value_item = cJSON_GetObjectItem(root, "value");
    if (value_item != NULL) {
        int value = value_item->valueint;
        printf("Valor: %d\n", value);
    }

    // Liberar la memoria
    cJSON_Delete(root);
}
