#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "ctype.h"

#include "modem_aux.h"


// Función para liberar la memoria de la estructura
void free_data(data_sms_strt_t* ds) {
    for (int i = 0; i < ds->lines; i++) {
        free(ds->data[i]);
        ds->data[i]=NULL;
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


int find_phone_and_extract(const char* input_string, char* phone) {
    // Busca el parámetro en el string
    char* found = strstr(input_string, ",");
    if (found != NULL) {
        // Encuentra la posición de la primera coma después del parámetro
        char* first_comma = found;
        // Encuentra la posición de la segunda coma después del parámetro
        char* second_comma = strchr(first_comma + 1, ',');
        if (second_comma != NULL) {
            // Extrae el valor entre las comas
            int value_length = second_comma - first_comma - 1;
            strncpy(phone, first_comma + 1, value_length);
            phone[value_length] = '\0';
            return 1;
        }
    }
    return 0;
}



int remove_word_from_string(char *input_string, const char *target) {
    char *found = strstr(input_string, target);
    if (found) {
        // Calcula la longitud del string antes de la palabra a eliminar
       /// size_t length_before_target = found - input_string;

        // Copia el string sin la palabra a eliminar
        memmove(found, found + strlen(target), strlen(found + strlen(target)) + 1);
        return 1; // Éxito
    } else {
        return 0; // La palabra no se encontró
    }
}
