#ifndef _MODEM_AUX_H_
#define _MODEM_AUX_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Estructura para almacenar los datos
typedef struct {
    int lines;
    char** data; 
} data_sms_strt_t;


// Función para liberar la memoria de la estructura
void free_data(data_sms_strt_t* ds);

void add_line(data_sms_strt_t* ds, const char* line);

int str_to_data_sms(const char* input_string, data_sms_strt_t* data);


#endif