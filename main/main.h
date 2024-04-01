
#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <string.h>

#include "EG915/EG915_modem.h"

/************************************************
 * DEFINES
*************************************************/

/************************************************
 * STRUCTURES
*************************************************/

typedef struct modem_gsm{
    EG915_info_t info;  
    char         code[10];
	int          signal;
	time_t       time;
}modem_gsm_t;


typedef struct {
    char mac[20];
    char name[10];
    float tem_max;
    float tem_min;
}cfg_ble_t;



int validarIP(const char* ip);

char* m_get_esp_rest_reason();

int split_and_check_IP(char* cadena, char* ip);

/************************************************
 * JASON PARSER
*************************************************/

int js_modem_to_str(const modem_gsm_t, char* buffer);

int js_str_to_ble(const char *json_string, cfg_ble_t *ble_config);

#endif /*_MAIN_H_*/