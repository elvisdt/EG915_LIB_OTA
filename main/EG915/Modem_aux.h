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

/**
 * @brief Busca y extrae un número de teléfono de una cadena de entrada.
 *
 * @param input_string Cadena de entrada que contiene el número de teléfono y otros datos.
 * @param phone Puntero al buffer donde se almacenará el número de teléfono extraído.
 *
 * @return
 *     - ESP_OK si se encuentra y extrae correctamente el número de teléfono.
 *     - ESP_FAIL si no se puede encontrar el número de teléfono en la cadena de entrada o si hay algún error.
 */
int find_phone_and_extract(const char* input_string, char* phone);


/**
 * Elimina todas las ocurrencias de una palabra específica de una cadena de caracteres.
 * 
 * @param input_string La cadena de caracteres en la que se buscará y eliminará la palabra.
 * @param target La palabra que se eliminará de la cadena de caracteres.
 * @return 0 si la palabra se eliminó correctamente, -1 si la palabra no se encontró en la cadena.
 */
int remove_word_from_string(char *input_string, const char *target);

#endif /* _MODEM_AUX_H_ */