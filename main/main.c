#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "esp_system.h"
#include <sdkconfig.h>
#include <sys/time.h>
#include <time.h>
#include <esp_timer.h>
#include <esp_log.h>

#include <nvs.h>
#include <nvs_flash.h>
#include <esp_spiffs.h>
#include <dirent.h>

#include "credentials.h"
#include "EG915_modem.h"
#include "ota_m95.h"


#include "crc.h"
#include "ota_control.h"
#include "ota_esp32.h"
#include "ota_headers.h"


#include <cJSON.h>

/***********************************************
 * DEFINES
************************************************/

#define TAG          "MAIN"
#define MAX_ATTEMPS     3

#define WAIT_MS(x)		vTaskDelay(pdMS_TO_TICKS(x))
#define WAIT_S(x)		vTaskDelay(pdMS_TO_TICKS(x*1e3))

/***********************************************
 * STRUCTURES
************************************************/

struct modem_gsm{
    EG915_info_t info;  
    char         code[10];
	int          signal;
	time_t       time;
};

/***********************************************
 * VARIABLES
************************************************/

/*---> External variables <--*/
QueueHandle_t uart_modem_queue;
uint8_t rx_modem_ready;
int rxBytesModem;
uint8_t *p_RxModem;
int end_task_uart_m95;


/*---> Task Handle <---*/
TaskHandle_t MAIN_task_handle    	=   NULL;
TaskHandle_t UART_task_handle    	=   NULL;
TaskHandle_t BLE_Task_handle        =   NULL;
TaskHandle_t MODEM_event_handle     =   NULL;


/*---> Data OTA Output <---*/
cJSON *doc;
char * output;


/*---> Aux Mememory <---*/
uint8_t aux_buff_mem[BUF_SIZE_MODEM];
char* buff_aux=(char*)aux_buff_mem;


/*---> data structure for modem <---*/
static struct modem_gsm data_modem={0};
static int ret_update_time =0;


/*---> gpio and uart config <---*/
EG915_gpio_t modem_gpio;
EG915_uart_t modem_uart;


/*---> OTA <--*/
char watchdog_en=1;
uint32_t current_time=0;


/*--> MQTT CHARS <---*/
int mqtt_idx = 0; 		// 0->5

/***********************************************
 * VARIABLES
************************************************/

void main_cfg_parms(){

    // ---> placa negra GPS <--- //
	modem_gpio.gpio_reset  = GPIO_NUM_35;
    modem_gpio.gpio_pwrkey = GPIO_NUM_48;
    modem_gpio.gpio_status = GPIO_NUM_42;

    modem_uart.uart_num     = UART_NUM_2;
    modem_uart.baud_rate    = 115200;
    modem_uart.gpio_uart_rx = GPIO_NUM_36;
    modem_uart.gpio_uart_tx = GPIO_NUM_37;
    
    //------placa morada cartavio-------------//
    /*
	modem_gpio.gpio_reset   = GPIO_NUM_1;
    modem_gpio.gpio_pwrkey  = GPIO_NUM_2;
    modem_gpio.gpio_status  = GPIO_NUM_40;

    modem_uart.uart_num     = UART_NUM_2;
    modem_uart.baud_rate    = 115200;
    modem_uart.gpio_uart_rx = GPIO_NUM_41;
    modem_uart.gpio_uart_tx = GPIO_NUM_42;
    */
};

int Init_config_modem(){
    ESP_LOGW(TAG, "--> INIT CONFIG MODEM <--");
    int state = 0;
    for (size_t i = 0; i < MAX_ATTEMPS; i++){
        state  = Modem_turn_ON();
        if (state ==0){
            Modem_turn_OFF();
            WAIT_S(3);
        }else{
            break;
        }
    }
    
    if(state != 1) return state;

    for (size_t i = 0; i < MAX_ATTEMPS; i++){
        WAIT_S(2);
        state  = Modem_begin_commands();
        if (state ==1){
            break;
        }
    }
	return state;
}

int Active_modem(){
	int status = 0;
	status = Init_config_modem();
	if (status){
        int status = Modem_get_dev_info(&data_modem.info);
        if(status !=1 ){
            return 0;   // FAIL
        }
    }
	return status;
}

void OTA_check(void){
  char buffer[700] ="";
  static const char *TAG_OTA = "OTA_task";
  esp_log_level_set(TAG_OTA, ESP_LOG_INFO);

  printf("OTA:Revisando conexion...\r\n");
  do{
	  if(!TCP_open(ip_OTA, port_OTA)){
		ESP_LOGI(TAG_OTA,"No se conecto al servidor");
		TCP_close();
		printf("OTA:Desconectado\r\n");
		break;
	  }

	  printf("OTA:Solicitando actualizacion...\r\n");
	  if(TCP_send(output, strlen(output))){                           // 1. Se envia la info del dispositivo
		printf("OTA:Esperando respuesta...\r\n");
		int ret_ota = readAT("}\r\n", "-8/()/(\r\n",10000,buffer);   // 2. Se recibe la 1ra respuesta con ota True si tiene un ota pendiente... (el servidor lo envia justo despues de recibir la info)(}\r\n para saber cuando llego la respuesta)
        if(ret_ota ==0){
            printf("OTA: no answer\r\n");
            break;
        }

        debug_ota("main> repta %s\r\n", buffer);
		if(strstr(buffer,"\"ota\": \"true\"") != 0x00){
			ESP_LOGI(TAG_OTA,"Iniciando OTA");
			printf("WTD desactivado\r\n");
			watchdog_en=0;
			if(ota_uartControl_M95() == OTA_EX_OK){
			  debug_ota("main> OTA m95 Correcto...\r\n");
			  esp_restart();
			}else{
			  debug_ota("main> OTA m95 Error...\r\n");
			}
			current_time = pdTICKS_TO_MS(xTaskGetTickCount())/1000;
			watchdog_en=1;
			printf("Watchdog reactivado\r\n");
		}
		printf("OTA:No hubo respuesta\r\n");
	  }
	}
	while(false);
    
    return;
}

/*---> TASK MODEM UART<---*/
static void Modem_rx_task(void *pvParameters){
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    p_RxModem = dtmp;
    for(;;) {
        if(xQueueReceive(uart_modem_queue, (void * )&event, portMAX_DELAY)) {
        	bzero(dtmp, RD_BUF_SIZE);
            if(event.type == UART_DATA) {
				rxBytesModem=event.size;
				uart_read_bytes(modem_uart.uart_num, dtmp, event.size, portMAX_DELAY*2);
				p_RxModem=dtmp;
				rx_modem_ready=1;
            }
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}


int CheckRecMqtt(void){

    ESP_LOGI("MAIN-MQTT","<< Revisando conexion >>");

    static int num_max_check = 0;
    if (num_max_check >(MAX_ATTEMPS+2)){
        ESP_LOGE("MAIN-MQTT", "RESTART ESP32");
        esp_restart();
    }

    
	int ret_conn, ret_open;
    char IP_MQTT_AUX[20]={0};
    char PORT_MQTT_AUX[10]={0};

    ret_conn = Modem_CheckMqtt_Conn(mqtt_idx);
    printf("ret_conn: %d, num_max: %d\r\n", ret_conn, num_max_check);

    if (ret_conn<0){
        ret_open=Modem_CheckMqtt_Open(mqtt_idx,IP_MQTT_AUX,PORT_MQTT_AUX);
        if (ret_open<0){
            num_max_check ++;
            CheckRecMqtt(); // CHECK AGAIN 
        }else if (ret_open==0){
             num_max_check ++;
            ret_open=Modem_Mqtt_Open(mqtt_idx,ip_MQTT,port_MQTT);
            if (ret_open == 0){
                Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
            }else if (ret_open>0){
                WAIT_S(2);
                Modem_Mqtt_Disconnect(mqtt_idx);
                Modem_Mqtt_Close(mqtt_idx);
                ret_open=Modem_Mqtt_Open(mqtt_idx,ip_MQTT,port_MQTT);
            }
            printf("ret_open: %d\r\n",ret_open);
            CheckRecMqtt(); // CHECK AGAIN 
        }else if (ret_open==1){
            num_max_check ++;
            Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
            CheckRecMqtt(); // CHECK AGAIN
        }else{
            CheckRecMqtt(); // CHECK AGAIN
        }
    }else if (ret_conn== 1){
        WAIT_S(2);
        Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
        CheckRecMqtt();  // CHECK AGAIN
    }else if (ret_conn==2){
        WAIT_S(2);
        CheckRecMqtt();  // CHECK AGAIN
    }else if (ret_conn == 3){
        // CONNECT IS SUCCESFULL
        num_max_check = 0;
        return 1;
    }else if (ret_conn == 4){
        Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
        CheckRecMqtt();  // CHECK AGAIN
    }
	return 0;
}

void app_main(void){
    ESP_LOGI(TAG, "--->> INIT PROJECT <<---");
	int ret_main = 0;

    main_cfg_parms();
    Modem_config();
    xTaskCreate(Modem_rx_task, "M95_rx_task", 1024*4, NULL, configMAX_PRIORITIES -1,&UART_task_handle);    // active service

    ret_main = Active_modem();
    if(ret_main != 1) esp_restart();
	ESP_LOGI(TAG,"-->> END CONFIG <<--\n");

    ret_update_time=Modem_update_time(3);
    ESP_LOGI(TAG, "RET update time: %d",ret_update_time);
    time(&data_modem.time);
    
    data_modem.signal = Modem_get_signal();
    strcpy(data_modem.code,PROJECT_VER);

    // char* date=epoch_to_string(data_modem.time);
	ESP_LOGI(TAG,"IMEI: %s", data_modem.info.imei);
    ESP_LOGI(TAG,"ICID: %s", data_modem.info.iccid);
    ESP_LOGI(TAG,"FIRMWARE: %s", data_modem.info.firmware);
    ESP_LOGI(TAG,"UNIX: %lld",data_modem.time);
    ESP_LOGW(TAG,"CODE: %s", data_modem.code);
	ESP_LOGW(TAG,"SIGNAL: %d", data_modem.signal);
    
    doc = cJSON_CreateObject();
    cJSON_AddItemToObject(doc,"imei",cJSON_CreateString(data_modem.info.imei));
    cJSON_AddItemToObject(doc,"project",cJSON_CreateString(PROJECT_NAME));
    cJSON_AddItemToObject(doc,"ota",cJSON_CreateString("true"));
    cJSON_AddItemToObject(doc,"cmd",cJSON_CreateString("false"));
    cJSON_AddItemToObject(doc,"sw",cJSON_CreateString("1.1"));
    cJSON_AddItemToObject(doc,"hw",cJSON_CreateString("1.1"));
    cJSON_AddItemToObject(doc,"otaV",cJSON_CreateString("1.0"));
    output = cJSON_PrintUnformatted(doc);

    ESP_LOGI(TAG,"Mensaje OTA:");
    printf(output);
    printf("\r\n");

    current_time = pdTICKS_TO_MS(xTaskGetTickCount())/1000;
    // OTA_check();
    WAIT_S(2);
    // Registra la hora de inicio
    int64_t start_time, end_time, elapsed_time;
    
    for (size_t i = 0; i < 10; i++){
        start_time = esp_timer_get_time();
        CheckRecMqtt();
        end_time = esp_timer_get_time();
        elapsed_time = end_time - start_time;
        ESP_LOGI("TIMER", "Tiempo de ejecución: %lld microsegundos", elapsed_time);
        WAIT_S(60);
    }
}

