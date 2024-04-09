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

void remove_char(char *cadena, char caracter) {
    char *src, *dst;
    src = dst = cadena;
    while (*src) {
        if (*src != caracter) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
    return;
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


int find_phone_and_extract(char* input_string, char* phone) {
    // Busca el parámetro en el string
    char *token;
    // Utiliza strtok para dividir la cadena en tokens usando la coma como separador

    token = strtok(input_string, ",");
    while (token != NULL) {
        // Imprime cada token (parte entre las comas)
        // printf("Parte: %s\n",  token);
        if(strstr(token,"\"+")){
            strcpy(phone,token);
            remove_char(phone, '\"');
            printf("found token: %s\r\n",phone);
            return 0;//OK
        }
        token = strtok(NULL, ","); // Siguiente token
    }
    return -1; // FAIL
}




int remove_word_from_string(char *input_string, const char *target) {
    char *found = strstr(input_string, target);
    if (found) {
        // Copia el string sin la palabra a eliminar
        memmove(found, found + strlen(target), strlen(found + strlen(target)) + 1);
        return 0; // Éxito
    } else {
        return -1; // La palabra no se encontró
    }
}

void extraer_ultimos(const char *cadena_entrada, size_t longitud, char *ultimos_datos) {
    size_t longitud_entrada = strlen(cadena_entrada);
    
    // Si la longitud de la cadena de entrada es menor o igual a la longitud deseada,
    // simplemente copiamos toda la cadena de entrada
    if (longitud_entrada <= longitud) {
        strcpy(ultimos_datos, cadena_entrada);
    } else {
        // Calculamos el índice de inicio de los últimos datos
        size_t inicio = longitud_entrada - longitud;
        
        // Copiamos los últimos datos al buffer de salida
        strcpy(ultimos_datos, cadena_entrada + inicio);
    }
}