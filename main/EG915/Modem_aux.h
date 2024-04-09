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

/**
 * Elimina todas las ocurrencias de un carácter específico de una cadena de texto.
 *
 * Esta función recorre la cadena de texto especificada y elimina todas las ocurrencias
 * del carácter especificado, dejando la cadena modificada sin ese carácter.
 *
 * @param cadena La cadena de texto de la cual se eliminarán los caracteres.
 * @param caracter El carácter que se desea eliminar de la cadena.
 */
void remove_char(char *cadena, char caracter);

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
 *     - 0 si se encuentra y extrae correctamente el número de teléfono.
 *     - -1 si no se puede encontrar el número de teléfono en la cadena de entrada o si hay algún error.
 */
int find_phone_and_extract(char* input_string, char* phone);


/**
 * Elimina todas las ocurrencias de una palabra específica de una cadena de caracteres.
 * 
 * @param input_string La cadena de caracteres en la que se buscará y eliminará la palabra.
 * @param target La palabra que se eliminará de la cadena de caracteres.
 * @return 0 si la palabra se eliminó correctamente, -1 si la palabra no se encontró en la cadena.
 */
int remove_word_from_string(char *input_string, const char *target);


/**
 * @brief Extrae los últimos datos de una cadena de entrada y los almacena en otra cadena.
 *
 * Esta función toma una cadena de entrada y extrae los últimos datos de acuerdo con la longitud especificada.
 * Si la longitud de la cadena de entrada es menor o igual a la longitud deseada, copia toda la cadena de entrada.
 * De lo contrario, copia los últimos datos desde el índice de inicio correcto.
 *
 * @param cadena_entrada La cadena de entrada de la cual se extraerán los últimos datos.
 * @param longitud La longitud de los últimos datos que se desean extraer.
 * @param ultimos_datos La cadena de salida donde se almacenarán los últimos datos extraídos.
 */
void extraer_ultimos(const char *cadena_entrada, size_t longitud, char *ultimos_datos);

#endif /* _MODEM_AUX_H_ */