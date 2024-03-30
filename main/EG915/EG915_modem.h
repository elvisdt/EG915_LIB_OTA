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
#include "esp_log.h"


#define BUFFER_SIZE         (2048)
#define BUF_SIZE_MODEM      (2048)
#define RD_BUF_SIZE         (BUF_SIZE_MODEM)


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


//---------------------------------------//
extern QueueHandle_t uart_modem_queue; 
extern uint8_t rx_modem_ready;
extern int rxBytesModem;
extern uint8_t * p_RxModem;

extern EG915_gpio_t modem_gpio;
extern EG915_uart_t modem_uart;

//---------------------------------------//
typedef struct EG915_info{
	char firmware[25];
	char imei[25];
	char iccid[25];
}EG915_info_t;

// Config input/ouput
void Modem_config_gpio();
void Modem_config_uart();
void Modem_config();

// Encendemos el Modem
int Modem_turn_ON();

// Apagamos el modem
int Modem_turn_OFF();

int Modem_turn_OFF_command();

// Reseteamos el Modem
void Modem_reset(); // No implement

int Modem_check_AT();
int Modem_check_uart();

// Configuramos los estados iniciales del Modem
int Modem_begin_commands();

// actualizamoz la hora
int Modem_process_up_time(char* response);
int Modem_update_time(uint8_t max_int);
// Funcion con formato para envio de comandos AT
int sendAT(char *Mensaje, char *ok, char *error, uint32_t timeout, char *respuesta);
int readAT(char *ok, char *error, uint32_t timeout, char *response);


//=======================================================//
// Obtenemos la fecha string
char*  Modem_get_date();

// Actualizamos la hora interna del ESP
time_t Modem_get_date_epoch();

// obtener el im
int Modem_get_IMEI(char* imei);
int Modem_get_signal();

int Modem_get_ICCID(char* iccid);
int Modem_get_firmware(char* firmare);
int Modem_get_dev_info(EG915_info_t* dev_modem);

//=======================================================//
// UDP CONFIGURATION AN CONNECTION
uint8_t Modem_TCP_UDP_open();
uint8_t Modem_TCP_UDP_send(char *msg, uint8_t len);
void Modem_TCP_UDP_close();

uint8_t TCP_open(char *IP, char *PORT);
uint8_t TCP_send(char *msg, uint8_t len);
void TCP_close();
uint8_t OTA(uint8_t *buff, uint8_t *inicio, uint8_t *fin, uint32_t len);


/************************************************************
 * MQTT FUNCTIONS
************************************************************/
/**
 * @brief Establece una conexión MQTT para un cliente específico.
 *
 * Esta función configura los parámetros necesarios para la conexión MQTT y
 * envía un comando AT al módem para abrir la conexión MQTT con el servidor
 * especificado por el indice del cleinte('idx), la dirección IP ('MQTT_IP')
 * y el puerto ('MQTT_PORT').
 *
 * @param idx      Índice del cliente MQTT (0-5).
 * @param MQTT_IP  Dirección IP del servidor MQTT.
 * @param MQTT_PORT Puerto del servidor MQTT.
 * @return Estado de apertura MQTT:
 *          0: La conexión MQTT se abrió correctamente.
 *          1: Error de parametros.
 *          2: Error Cliente Ocupado.
 *          3: Error de activación de PDP. 
 *          4: Error del nombre del dominio o IP.
 *          5: Error de conxion de red.
 *          -1: Error de aprtura de red.
 *          -2: Error de respuesta del modem. 
 */
int  Modem_Mqtt_Open(int idx, const char* MQTT_IP, const char* MQTT_PORT);

/**
 * @brief Verifica si MQTT está abierto o no para un cliente específico.
 *
 * Esta función envía un comando AT al módem para verificar si MQTT está 
 * abierto para el cliente identificado por el índice 'idx'.
 *
 * @param idx      Índice del cliente MQTT.
 * @param MQTT_IP  Dirección IP del servidor MQTT.
 * @param MQTT_PORT Puerto del servidor MQTT.
 * @return Estado de apertura MQTT:
 *         1: MQTT está abierto.
 *         0: MQTT no está no esta abierto.
 *         Valor negativo en caso de error.
 */
int Modem_CheckMqtt_Open(int idx, char* MQTT_IP, char* MQTT_PORT);

/**
 * @brief Establece una conexión MQTT para un cliente específico.
 *
 * Esta función envía un comando AT al módem para establecer una conexión MQTT
 * para el cliente identificado por el índice `idx` y el `clientID` proporcionado.
 *
 * @param idx      Índice del cliente MQTT.
 * @param clientID Identificador del cliente (generalmente una cadena única).
 * @return Estado de la conexión MQTT:
 *         0: Conexión aceptada.
 *         1: Versión de protocolo no aceptable.
 *         2: Servidor no disponible.
 *         3: Usuario o contraseña incorrectos.
 *         4: No autorizado.
 *         Valor negativo en caso de error.
 */
int Modem_Mqtt_Conn(int idx, const char* clientID);

/**
 * @brief Verifica el estado de conexión MQTT para un cliente específico.
 *
 * Esta función envía un comando AT al módem para obtener información sobre
 * el estado de conexión MQTT del cliente identificado por el índice `idx`.
 *
 * @param idx Índice del cliente MQTT.
 * @return Estado de conexión MQTT:
 *         1: Inicializando.
 *         2: Conectando.
 *         3: Conectado.
 *         4: Desconectando.
 *         Otro valor negativo en caso de error.
 */
int Modem_CheckMqtt_Conn(int idx);

int Modem_Mqtt_Disconnect(int idx);
int  Modem_Mqtt_Close(int idx);

int Modem_CheckMqtt_state(int idx);

// Publicamos la data deseada en el broker
int  Modem_PubMqtt_data(uint8_t * data,char * topic,int data_len, int id , int retain);

/**
 * @brief Suscribe un cliente MQTT a un tema específico.
 *
 * Esta función envía un comando AT al módem para suscribir al cliente MQTT
 * identificado por el índice `idx` al tema especificado por `topic_name`.
 *
 * @param idx        Índice del cliente MQTT.
 * @param topic_name Nombre del tema al que se suscribirá el cliente.
 * @return Resultado de la suscripción:
 *         0: Suscripción exitosa (OK).
 *         1: Retransmisión del paquete.
 *         2: Fallo en la suscripción.
 *         Otro valor negativo en caso de error.
 *
 * @note Esta función utiliza comandos AT específicos del módem y está diseñada
 * para trabajar con hardware y configuraciones específicas. Asegúrate de adaptar
 * los parámetros y los mensajes de registro según tus necesidades.
 */
int Modem_SubMqtt(int idx, char* topic_name);

int Modem_UnsubMqtt(int idx, char* topic_name);

int Modem_MqttCheck_SubData(int idx, uint8_t status_buff[5]);

int  Modem_sub_topic_json(int ID, char* topic_name, char* response);
                                                                                   


//=======================================================//
int Modem_readSMS(char* mensaje, char *numero);
int Modem_sendSMS(char* mensaje, char *numero);
int Modem_delete_SMS();


/*-------------------------------------*/
int remove_word_from_string(char *input_string, const char *target);
#endif /*_EG915U_Modem_H_*/
