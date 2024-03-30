#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "ctype.h"

#include "Modem_aux.h"


// Función para liberar la memoria de la estructura
void free_data(data_sms_strt_t* ds) {
    for (int i = 0; i < ds->lines; i++) {
        free(ds->data[i]);
    }
    free(ds->data);
}

// Función para agregar una línea a la estructura
void add_line(data_sms_strt_t* ds, const char* line) {

	if (strspn(line, " \r\n") != strlen(line)) {
        ds->lines++;
        ds->data = realloc(ds->data, ds->lines * sizeof(char*));
        ds->data[ds->lines - 1] = strdup(line);
    }
}



int str_to_data_sms(const char* input_string, data_sms_strt_t* data) {
    data->lines = 0;
    data->data = NULL;

    // Separa el string por saltos de línea
    char* token = strtok((char*)input_string, "\n");

    // Agrega cada línea a la estructura de datos
    while (token != NULL) {
        add_line(data, token); 
        token = strtok(NULL, "\n");
    }
    return 0;
}



void remove_spaces(char* str) {
    char* i = str;
    char* j = str;
    while(*j != 0) {
        *i = *j++;
        if(*i != ' ' && *i != '\n' && *i != '\r')
            i++;
    }
    *i = 0;
}

void remove_newlines(char* str) {
    char* p;
    while ((p = strchr(str, '\n')) != NULL) {
        *p = ' ';  // Reemplaza el salto de línea con un espacio
    }
    while ((p = strchr(str, '\r')) != NULL) {
        memmove(p, p + 1, strlen(p));
    }
}


void str_to_lowercase(char *str) {
    for(int i = 0; str[i]; i++){
        str[i] = tolower((unsigned char)str[i]);
    }
}


void str_to_uppercase(char *str) {
    for(int i = 0; str[i]; i++){
        str[i] = toupper((unsigned char)str[i]);
    }
}