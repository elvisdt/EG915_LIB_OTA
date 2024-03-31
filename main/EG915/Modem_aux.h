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

void remove_spaces(char* str);

void remove_newlines(char* str);

void str_to_lowercase(char *str);

void str_to_uppercase(char *str);

int find_phone_and_extract(const char* input_string, char* phone);

/*-------------------------------------*/
int remove_word_from_string(char *input_string, const char *target);

#endif /* _MODEM_AUX_H_ */