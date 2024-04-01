#ifndef _EG915U_MODEM_H_
#define _EG915U_MODEM_H_

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <esp_task_wdt.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_sleep.h"
#include "esp_timer.h"


#include "errors_modem.h"
/*****************************************************
 * DEFINES
******************************************************/

#define BUFFER_SIZE         (2048)
#define BUF_SIZE_MODEM      (2048)
#define RD_BUF_SIZE         (BUF_SIZE_MODEM)



/*****************************************************
 * STRUCTURES
******************************************************/
/**
 * @brief Estructura para configurar los parámetros de UART del módem EG915U.
 */
typedef struct EG915_uart{
    gpio_num_t      gpio_uart_tx; /*!< Pin de transmisión UART */
    gpio_num_t      gpio_uart_rx; /*!< Pin de recepción UART */
    uart_port_t     uart_num;     /*!< Número de puerto UART */
    int             baud_rate;    /*!< Velocidad en baudios de UART */
} EG915_uart_t;

/**
 * @brief Estructura para configurar los pines GPIO del módem EG915U.
 */
typedef struct EG915_gpio{
    gpio_num_t gpio_status ; /*!< Pin de estado */
    gpio_num_t gpio_pwrkey ; /*!< Pin de encendido/apagado (PWRKEY) */
    gpio_num_t gpio_reset ; /*!< Pin de encendido/apagado (PWRKEY) */
} EG915_gpio_t;


/**
 * @brief Estructura para los datos del EG915
 */
typedef struct EG915_info{
	char firmware[25];
	char imei[25];
	char iccid[25];
}EG915_info_t;



/*****************************************************
 * VARIABLES
******************************************************/

/*---> EXTERNAL VARIABLES <---*/
extern QueueHandle_t uart_modem_queue; 
extern uint8_t rx_modem_ready;
extern int rxBytesModem;
extern uint8_t * p_RxModem;

extern EG915_gpio_t modem_gpio;
extern EG915_uart_t modem_uart;



/*****************************************************
 * FUNCTIONS
******************************************************/

/**
 * @brief Configura los pines GPIO para el módem.
 */
void Modem_config_gpio();


/**
 * @brief Configura la interfaz UART para el módem.
 */
void Modem_config_uart();


/**
 * @brief Configura la interfaz UART para el módem.
 */
void Modem_config();


/**
 * @brief Envía un comando AT al módem y espera una respuesta.
 *
 * Esta función envía un comando AT al módem y espera una respuesta específica.
 * El tiempo de espera está limitado por el parámetro 'timeout'.
 *
 * @param command Comando AT a enviar.
 * @param ok Respuesta esperada que indica éxito.
 * @param error Respuesta esperada que indica un error.
 * @param timeout Tiempo de espera en milisegundos.
 * @param response Puntero al búfer para almacenar la respuesta del módem.
 * @return Código de resultado (MD_AT_OK, MD_AT_ERROR o MD_AT_TIMEOUT).
 */
int sendAT(char *Mensaje, char *ok, char *error, uint32_t timeout, char *respuesta);


/**
 * @brief Lee una respuesta del módem después de enviar un comando AT.
 *
 * Esta función envía un comando AT al módem y espera una respuesta específica.
 * El tiempo de espera está limitado por el parámetro 'timeout'.
 *
 * @param ok Respuesta esperada que indica éxito.
 * @param error Respuesta esperada que indica un error.
 * @param timeout Tiempo de espera en milisegundos.
 * @param response Puntero al búfer para almacenar la respuesta del módem.
 * @return Código de resultado (MD_AT_OK, MD_AT_ERROR o MD_AT_TIMEOUT).
 */
int readAT(char *ok, char *error, uint32_t timeout, char *response);


/********************************************************************
 * 
********************************************************************/


/**
 * @brief Enciende el módem y realiza comprobaciones para asegurarse de que esté encendido correctamente.
 *
 * Esta función enciende el módem y realiza una serie de comprobaciones para asegurarse de que está encendido correctamente.
 *
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si el módem se encendió correctamente y todas las comprobaciones fueron exitosas.
 *             - MD_CFG_FAIL si hubo un error en las comprobaciones AT.
 */
int Modem_turn_ON();


/**
 * @brief Apaga el módem mediante la señal PWRKEY.
 *
 * Esta función apaga el módem pulsando la señal PWRKEY durante un tiempo específico.
 *
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si el módem se apagó correctamente.
 *             - MD_CFG_FAIL si el módem no se apagó correctamente.
 */
int Modem_turn_OFF();


/**
 * @brief Envía un comando AT para apagar el módem.
 *
 * Esta función envía un comando AT al módem para apagarlo. Primero intenta apagar el módem con
 * la opción de apagado forzado, y si eso falla, intenta apagarlo sin forzar.
 *
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_AT_OK si el módem se apagó correctamente.
 *             - MD_AT_ERROR si hubo un error al intentar apagar el módem.
 *             - MD_AT_TIMEOUT si se alcanzó el tiempo de espera sin recibir respuesta del módem.
 */
int Modem_turn_OFF_command();


/**
 * @brief Reinicia el módem.
 *
 * Esta función reinicia el módem realizando un ciclo de reset.
 */
void Modem_reset(); // No validate implement complet


/**
 * @brief Comprueba la comunicación con el módem enviando el comando AT.
 *
 * Esta función envía el comando AT al módem para comprobar la comunicación. Espera una respuesta
 * "OK" del módem dentro de un tiempo de espera especificado.
 *
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_AT_OK si se recibió una respuesta "OK" del módem.
 *             - MD_AT_ERROR si se recibió una respuesta "ERROR" del módem.
 *             - MD_AT_TIMEOUT si se alcanzó el tiempo de espera sin recibir respuesta del módem.
 */
int Modem_check_AT();


/**
 * @brief Verifica la comunicación UART con el módem enviando el comando AT.
 *
 * Esta función verifica la comunicación UART con el módem enviando el comando AT y esperando una respuesta.
 *
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se recibió una respuesta "OK" del módem.
 *             - MD_CFG_FAIL si se recibió una respuesta "ERROR" del módem o si no se recibió ninguna respuesta dentro del tiempo de espera.
 */
int Modem_check_uart();


/**
 * @brief Configura los comandos AT iniciales para iniciar la comunicación con el módem.
 *
 * Esta función envía una serie de comandos AT al módem para configurar su funcionamiento inicial.
 * Incluye comandos para restablecer los parámetros de fábrica, desactivar el modo de eco local,
 * configurar el prefijo del código de resultado, verificar si se requiere una contraseña, 
 * configurar el APN, activar el PDP, habilitar el registro de red, y configurar los mensajes SMS.
 *
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_AT_OK si todos los comandos AT fueron ejecutados correctamente.
 *             - MD_AT_ERROR si hubo un error al ejecutar alguno de los comandos AT.
 *             - MD_AT_TIMEOUT si se alcanzó el tiempo de espera sin recibir respuesta del módem.
 */
int Modem_begin_commands();


/**
 * @brief Sincroniza el tiempo del sistema con la respuesta del módem.
 *
 * Esta función analiza la respuesta del módem en busca de una cadena de tiempo y sincroniza el
 * tiempo del sistema con esta cadena.
 *
 * @param response La respuesta del módem que contiene la información de tiempo.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se sincronizó correctamente el tiempo del sistema.
 *             - MD_CFG_FAIL si no se pudo sincronizar el tiempo del sistema.
 */
int Modem_sync_time(char* response);


/**
 * @brief Actualiza el tiempo del sistema utilizando el servidor NTP a través del módem.
 *
 * Esta función intenta actualizar el tiempo del sistema utilizando el protocolo de tiempo de red
 * (NTP) a través del módem. Intenta con el servidor NTP "time.google.com" y "pool.ntp.org"
 * alternativamente hasta que se actualice correctamente el tiempo del sistema o se alcance el
 * número máximo de intentos.
 *
 * @param max_int El número máximo de intentos para actualizar el tiempo del sistema.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se actualizó correctamente el tiempo del sistema.
 *             - MD_CFG_FAIL si no se pudo actualizar el tiempo del sistema después de todos los intentos.
 */
int Modem_update_time(uint8_t max_int);


/**
 * @brief Obtiene la fecha del módem.
 *
 * Esta función envía un comando AT al módem para obtener la fecha y hora actuales. Analiza la respuesta
 * del módem para extraer la fecha y hora y devuelve un puntero a la cadena que contiene la fecha.
 * Si la operación falla, devuelve NULL.
 *
 * @return char* Un puntero a la cadena que contiene la fecha en formato de texto (si se obtuvo correctamente).
 *              NULL si no se pudo obtener la fecha del módem.
 */
char*  Modem_get_date();


/**
 * @brief Obtiene la fecha del módem en formato epoch.
 *
 * Esta función obtiene la fecha y hora del módem en formato de cadena de texto, la parsea y la
 * convierte en un tiempo epoch (el número de segundos transcurridos desde el 1 de enero de 1970).
 *
 * @return time_t El tiempo epoch que representa la fecha y hora obtenida del módem.
 *               -1 si no se pudo obtener la fecha del módem.
 */
time_t Modem_get_date_epoch();


/**
 * @brief Obtiene el número IMEI del módem.
 *
 * Esta función envía un comando AT al módem para obtener el número IMEI. Analiza la respuesta
 * del módem para extraer el número IMEI y lo almacena en la variable proporcionada.
 *
 * @param imei La variable donde se almacenará el número IMEI.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se encontró y se almacenó correctamente el número IMEI.
 *             - MD_CFG_FAIL si no se pudo encontrar el número IMEI en la respuesta del módem.
 */
int Modem_get_IMEI(char* imei);


/**
 * @brief Obtiene el número ICCID del módem.
 *
 * Esta función envía un comando AT al módem para obtener el número ICCID. Analiza la respuesta
 * del módem para extraer el número ICCID y lo almacena en la variable proporcionada.
 *
 * @param iccid La variable donde se almacenará el número ICCID.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se encontró y se almacenó correctamente el número ICCID.
 *             - MD_CFG_FAIL si no se pudo encontrar el número ICCID en la respuesta del módem.
 */
int Modem_get_ICCID(char* iccid);


/**
 * @brief Obtiene la versión del firmware del módem.
 *
 * Esta función envía un comando AT al módem para obtener la versión del firmware. Analiza la
 * respuesta del módem para extraer la versión del firmware y la almacena en la variable proporcionada.
 *
 * @param firmware La variable donde se almacenará la versión del firmware.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se encontró y se almacenó correctamente la versión del firmware.
 *             - MD_CFG_FAIL si no se pudo encontrar la versión del firmware en la respuesta del módem.
 */
int Modem_get_firmware(char* firmare);


/**
 * @brief Obtiene información del dispositivo del módem.
 *
 * Esta función obtiene información del dispositivo del módem, incluyendo el número ICCID, el número IMEI y la versión del firmware.
 *
 * @param dev_modem Un puntero a una estructura 'EG915_info_t' donde se almacenará la información del dispositivo.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_CFG_SUCCESS si se obtuvo correctamente toda la información del dispositivo.
 *             - MD_CFG_FAIL si no se pudo obtener alguna parte de la información del dispositivo.
 */
int Modem_get_dev_info(EG915_info_t* dev_modem);


/**
 * @brief Obtiene la intensidad de la señal del módem.
 *
 * Esta función envía un comando AT al módem para obtener la intensidad de la señal. Analiza la respuesta
 * del módem para extraer la intensidad de la señal y la devuelve como un valor entero.
 *
 * @return int La intensidad de la señal del módem, en unidades específicas del módem.
 *             99 si no se pudo obtener la intensidad de la señal.
 */
int Modem_get_signal();
//=======================================================//
// UDP CONFIGURATION AN CONNECTION
uint8_t Modem_TCP_UDP_open();                     // no implement
uint8_t Modem_TCP_UDP_send(char *msg, uint8_t len);
void Modem_TCP_UDP_close();

/**
 * @brief Abre una conexión TCP con el servidor especificado.
 *
 * Esta función configura y abre una conexión TCP con el servidor utilizando el módulo GSM.
 * Primero configura el perfil de APN, activa un contexto PDP y luego abre la conexión TCP con el servidor
 * en la dirección IP y puerto especificados.
 *
 * @param[in] ip_tcp Dirección IP del servidor al que se va a conectar.
 * @param[in] port_tcp Puerto del servidor al que se va a conectar.
 *
 * @return Estado de la operación:
 *     - MD_TCP_OPEN_OK si la conexión TCP se abrió correctamente.
 *     - MD_TCP_OPEN_FAIL si ocurrió un error al intentar abrir la conexión TCP.
 */
int TCP_open(char *ip_tcp, char *port_tcp);


/**
 * @brief Envía datos a través de una conexión TCP utilizando el módulo GSM.
 *
 * Esta función envía un mensaje a través de la conexión TCP abierta previamente con el servidor.
 *
 * @param[in] msg Puntero al buffer que contiene los datos a enviar.
 * @param[in] len Longitud de los datos a enviar en bytes.
 *
 * @return Estado de la operación:
 *     - MD_TCP_SEND_OK si los datos se enviaron correctamente a través de la conexión TCP.
 *     - MD_TCP_SEND_FAIL si ocurrió un error durante el envío de los datos.
 */

int TCP_send(char *msg, uint8_t len);

/**
 * @brief Cierra la conexión TCP actual.
 *
 * Esta función cierra la conexión TCP actual utilizando el módulo GSM.
 *
 * @return Estado de la operación:
 *     - MD_TCP_CLOSE_OK si la conexión TCP se cerró correctamente.
 *     - MD_TCP_CLOSE_FAIL si ocurrió un error al intentar cerrar la conexión TCP.
 */
int TCP_close();

uint8_t OTA(uint8_t *buff, uint8_t *inicio, uint8_t *fin, uint32_t len);


/************************************************************
 * MQTT FUNCTIONS
************************************************************/
/**
 * @brief Abre una conexión MQTT utilizando el módem.
 *
 * Esta función configura los parámetros necesarios para establecer una conexión MQTT utilizando el módem.
 * Luego envía un comando AT al módem para abrir la conexión MQTT con el servidor especificado.
 *
 * @param idx El índice del perfil PDP a utilizar.
 * @param MQTT_IP La dirección IP del servidor MQTT.
 * @param MQTT_PORT El puerto del servidor MQTT.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_MQTT_OPEN_OK si se abrió la conexión MQTT exitosamente.
 *             - MD_MQTT_OPEN_FAIL_NET si falló al abrir la conexión por problemas de red.
 *             - MD_MQTT_OPEN_FAIL_PRM si los parámetros son incorrectos.
 *             - MD_MQTT_OPEN_FAIL_IDX si el identificador está ocupado.
 *             - MD_MQTT_OPEN_FAIL_PDP si falló al activar el perfil PDP.
 *             - MD_MQTT_OPEN_FAIL_DOM si falló al parsear el nombre de dominio.
 *             - MD_MQTT_OPEN_FAIL_CNN si hay un error de conexión de red.
 *             - MD_CFG_FAIL si ocurrió un error de respuesta no reconocido.
 */
int  Modem_Mqtt_Open(int idx, const char* MQTT_IP, const char* MQTT_PORT);

/**
 * @brief Verifica el estado de la conexión MQTT utilizando el módem.
 *
 * Esta función envía un comando AT al módem para verificar si la conexión MQTT está abierta.
 * Analiza la respuesta del módem para determinar si la conexión MQTT está abierta o no.
 *
 * @param idx El índice del perfil PDP a verificar.
 * @param MQTT_IP La dirección IP del servidor MQTT.
 * @param MQTT_PORT El puerto del servidor MQTT.
 * @return int Un código de retorno que indica el estado de la conexión MQTT:
 *             - MD_MQTT_IS_OPEN si la conexión MQTT está abierta.
 *             - MD_MQTT_NOT_OPEN si la conexión MQTT no está abierta.
 *             - MD_CFG_FAIL si ocurrió un error durante la verificación.
 */
int Modem_Mqtt_CheckOpen(int idx, char* MQTT_IP, char* MQTT_PORT);

/**
 * @brief Establece una conexión MQTT con el servidor utilizando el módem.
 *
 * Esta función envía un comando AT al módem para establecer una conexión MQTT con el servidor utilizando
 * el cliente MQTT especificado. Analiza la respuesta del módem para determinar si la conexión se estableció
 * correctamente o si ocurrió algún error.
 *
 * @param idx El índice del perfil PDP a utilizar.
 * @param clientID El ID del cliente MQTT.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_MQTT_CONN_OK si la conexión MQTT se estableció correctamente.
 *             - MD_MQTT_CONN_FAIL_VER si la versión del protocolo es inaceptable.
 *             - MD_MQTT_CONN_FAIL_SRV si el servidor no está disponible.
 *             - MD_MQTT_CONN_FAIL_UPS si el nombre de usuario o la contraseña son incorrectos.
 *             - MD_MQTT_CONN_FAIL_AUT si el cliente no está autorizado.
 *             - MD_CFG_FAIL si ocurrió un error de respuesta no reconocido.
 */
int Modem_Mqtt_Conn(int idx, const char* clientID);

/**
 * @brief Verifica el estado de la conexión MQTT.
 *
 * Esta función verifica el estado de la conexión MQTT con el índice especificado.
 *
 * @param idx Índice de la conexión MQTT.
 * @return Estado de la conexión MQTT:
 *     - MD_MQTT_CONN_INIT si la conexión se está inicializando.
 *     - MD_MQTT_CONN_CONNECT si la conexión está en proceso de conexión.
 *     - MD_MQTT_CONN_OK si la conexión MQTT está establecida correctamente.
 *     - MD_MQTT_CONN_DISCONNECT si la conexión MQTT está siendo desconectada.
 *     - MD_MQTT_CONN_ERROR si ocurrió un error al verificar el estado de la conexión.
 */
int Modem_Mqtt_CheckConn(int idx);


/**
 * @brief Desconecta la conexión MQTT utilizando el módem.
 *
 * Esta función envía un comando AT al módem para desconectar la conexión MQTT especificada por el índice.
 * Espera la respuesta del módem para determinar si se desconectó correctamente.
 *
 * @param idx El índice de la conexión MQTT a desconectar.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_AT_OK si la conexión MQTT se desconectó correctamente.
 *             - MD_AT_ERROR si ocurrió un error durante la desconexión de la conexión MQTT.
 *             - MD_AT_TIMEOUT si se alcanzó el tiempo de espera sin respuesta del módem.
 */
int Modem_Mqtt_Disconnect(int idx);


/**
 * @brief Cierra la conexión MQTT utilizando el módem.
 *
 * Esta función envía un comando AT al módem para cerrar la conexión MQTT especificada por el índice.
 * Espera la respuesta del módem para determinar si se cerró la conexión correctamente.
 *
 * @param idx El índice de la conexión MQTT a cerrar.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_AT_OK si la conexión MQTT se cerró correctamente.
 *             - MD_AT_ERROR si ocurrió un error durante el cierre de la conexión MQTT.
 *             - MD_AT_TIMEOUT si se alcanzó el tiempo de espera sin respuesta del módem.
 */
int  Modem_Mqtt_Close(int idx);

int Modem_Mqtt_Checkstate(int idx); // NO IMPLEMENT


/**
 * @brief Publica datos en un tema MQTT utilizando el módem.
 *
 * Esta función envía un mensaje MQTT con los datos especificados al tema MQTT especificado.
 * Espera la confirmación del módem para determinar si la publicación se realizó correctamente.
 *
 * @param data Los datos que se van a publicar en el tema MQTT.
 * @param topic El nombre del tema MQTT en el que se publicarán los datos.
 * @param data_len La longitud de los datos a publicar.
 * @param id El identificador de la publicación MQTT.
 * @param retain Un indicador que indica si los mensajes deben ser retenidos por el servidor MQTT.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_MQTT_PUB_OK si la publicación de datos en el tema MQTT fue exitosa.
 *             - MD_MQTT_PUB_FAIL si ocurrió un error durante la publicación de datos en el tema MQTT.
 *             - MD_CFG_FAIL si ocurrió un error durante la ejecución del comando AT.
 */
int Modem_Mqtt_Pub(char* data, char* topic, int data_len, int id , int retain);
	
/**
 * @brief Suscribe un cliente MQTT a un tema específico utilizando el módem.
 *
 * Esta función envía un comando AT al módem para suscribir un cliente MQTT al tema especificado.
 * Espera la respuesta del módem para determinar si la suscripción se realizó correctamente.
 *
 * @param idx El índice de la conexión MQTT a la que se suscribirá el cliente.
 * @param topic_name El nombre del tema MQTT al que se suscribirá el cliente.
 * @return int Un código de retorno que indica el resultado de la operación:
 *             - MD_MQTT_SUB_OK si la suscripción al tema MQTT fue exitosa.
 *             - MD_MQTT_SUB_RETP si el tema MQTT ya había sido suscrito previamente.
 *             - MD_MQTT_SUB_FAIL si ocurrió un error durante la suscripción al tema MQTT.
 *             - MD_MQTT_SUB_UNKNOWN si se recibió una respuesta desconocida del módem.
 *             - MD_CFG_FAIL si ocurrió un error durante la ejecución del comando AT.
 */
int Modem_Mqtt_Sub(int idx, char* topic_name);

/**
 * @brief Cancela la suscripción a un tema MQTT en un cliente MQTT específico.
 *
 * Esta función envía un comando AT para cancelar la suscripción a un tema MQTT en un cliente MQTT específico.
 *
 * @param idx El índice del cliente MQTT.
 * @param topic_name El nombre del tema MQTT al que se cancelará la suscripción.
 * @return int Un código que indica el resultado de la operación:
 *             - MD_AT_OK si la operación se realiza con éxito.
 *             - MD_AT_ERROR si hay un error al ejecutar el comando AT.
 *             - MD_AT_TIMEOUT si se alcanza el tiempo de espera sin recibir respuesta.
 */
int Modem_Mqtt_Unsub(int idx, char* topic_name);


/**
 * @brief Verifica si hay datos en el búfer de recepción de los temas MQTT suscritos.
 *
 * Esta función verifica si hay datos en el búfer de recepción de los temas MQTT a los que se ha suscrito el cliente MQTT.
 *
 * @param idx El índice del cliente MQTT.
 * @param status_buff Un arreglo de 5 elementos donde se almacenará el estado de los búferes de recepción.
 *                    Cada elemento indicará si hay datos en el búfer correspondiente (1) o no (0).
 * @return int Un código que indica el resultado de la operación:
 *             - MD_MQTT_RECV_BUFF_DATA si hay datos en al menos uno de los búferes de recepción.
 *             - MD_MQTT_RECV_BUFF_CLEAN si todos los búferes de recepción están vacíos.
 *             - MD_MQTT_RECV_UNKOWN si ocurre un error desconocido al leer el estado del búfer de recepción.
 *             - MD_CFG_FAIL si hay un error al enviar el comando AT para verificar el búfer de recepción.
 */
int Modem_Mqtt_Check_Buff(int idx, uint8_t status_buff[5]);


/**
 * Lee los datos recibidos en un tópico MQTT desde el búfer del módem.
 * 
 * @param idx El índice del tópico MQTT del que se leerán los datos.
 * @param num_mem El número de mensajes en memoria que se leerán.
 * @param response El buffer donde se almacenarán los datos leídos.
 * @return MD_MQTT_READ_OK si se leyeron los datos correctamente, MD_MQTT_READ_FAIL si ocurrió un error al leer, MD_MQTT_READ_NO_FOUND si no se encontraron datos.
 */
int Modem_Mqtt_Read_data(int idx, int num_mem, char* response);


int Modem_sub_topic_json(int ID, char* topic_name, char* response);
                                                                                   


//=======================================================//
/**
 * @brief Lee un mensaje SMS no leído del módem y extrae el número de teléfono y el mensaje.
 *
 * Esta función cambia al modo de texto, lista los mensajes SMS no leídos, procesa la respuesta para obtener
 * los datos del SMS y extrae el número de teléfono y el mensaje del primer SMS no leído encontrado.
 *
 * @param mensaje Puntero al búfer donde se almacenará el mensaje SMS leído.
 * @param numero Puntero al búfer donde se almacenará el número de teléfono del remitente del SMS.
 *
 * @return MD_SMS_READ_FOUND si se encuentra y lee correctamente un mensaje SMS no leído.
 *         MD_SMS_READ_NO_FOUND si no se encuentra ningún mensaje SMS no leído.
 *         MD_SMS_READ_UNKOWN si ocurre un error desconocido al procesar los mensajes SMS.
 *         MD_CFG_FAIL si hay un error al enviar los comandos AT al módem.
 */
int Modem_SMS_Read(char* mensaje, char *numero);


/**
 * @brief Envía un mensaje de texto (SMS) a un número de teléfono específico.
 *
 * @param[in] mensaje Puntero al mensaje de texto que se va a enviar.
 * @param[in] numero Número de teléfono al que se enviará el mensaje.
 *
 * @return Estado de la operación:
 *     - MD_SMS_SEND_OK si el mensaje se envió con éxito.
 *     - MD_SMS_SEND_FAIL si ocurrió un error al enviar el mensaje.
 *     - MD_CFG_FAIL si ocurrió un error de configuración o comunicación con el módem.
 */

int Modem_SMS_Send(char* mensaje, char *numero);

/**
 * @brief Elimina todos los mensajes de texto (SMS) almacenados en la tarjeta SIM.
 *
 * Esta función establece el modo de texto para los mensajes SMS y elimina todos los mensajes almacenados
 * en la tarjeta SIM que estén marcados como leídos.
 *
 * @return Estado de la operación:
 *     - MD_AT_OK si los mensajes se eliminaron correctamente.
 *     - MD_AT_ERROR si ocurrió un error durante la eliminación de los mensajes.
 *     - MD_AT_TIMEOUT si se agotó el tiempo de espera durante la comunicación con el módem.
 */
int Modem_SMS_delete();



#endif /*_EG915U_Modem_H_*/
