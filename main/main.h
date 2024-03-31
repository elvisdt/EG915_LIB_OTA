
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


int validarIP(const char* ip);

char* m_get_esp_rest_reason();

int split_and_check_IP(char* cadena, char* ip);

/************************************************
 * JASON PARSER
*************************************************/

int modem_info_to_json(const modem_gsm_t, char* buffer);


#endif /*_MAIN_H_*/