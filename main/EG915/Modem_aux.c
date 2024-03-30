#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

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
