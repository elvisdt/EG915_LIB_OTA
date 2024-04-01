#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_event.h>

#include <esp_system.h>
#include <sdkconfig.h>
#include <sys/time.h>
#include <time.h>
#include <esp_timer.h>
#include <esp_log.h>
#include "esp_ota_ops.h"
#include <nvs.h>
#include <nvs_flash.h>

#include <cJSON.h>

/* main libraries */
#include "credentials.h"
#include "EG915_modem.h"
#include "ota_modem.h"
#include "main.h"


/* ota modem librarias */
#include "crc.h"
#include "ota_control.h"
#include "ota_esp32.h"
#include "ota_headers.h"

/* ota ble librarias */
#include "gap.h"
#include "gatt_svr.h"


/***********************************************
 * DEFINES
************************************************/

#define TAG          "MAIN"
#define MAX_ATTEMPS     3

#define WAIT_MS(x)		vTaskDelay(pdMS_TO_TICKS(x))
#define WAIT_S(x)		vTaskDelay(pdMS_TO_TICKS(x*1e3))

#define MASTER_TOPIC_MQTT   "OTA"

/***********************************************
 * STRUCTURES
************************************************/



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
TaskHandle_t UART_task_handle    	=   NULL;
TaskHandle_t MAIN_task_handle    	=   NULL;
TaskHandle_t BLE_Task_handle        =   NULL;


/*---> Data OTA Output <---*/
cJSON *doc;
char * output;


/*---> Aux Mememory <---*/
uint8_t aux_buff_mem[BUF_SIZE_MODEM];
char* buff_aux=(char*)aux_buff_mem;


/*---> data structure for modem <---*/
static modem_gsm_t data_modem={0};
static int ret_update_time =0;


/*---> gpio and uart config <---*/
EG915_gpio_t modem_gpio;
EG915_uart_t modem_uart;


/*---> OTA <--*/
uint8_t watchdog_en=1;
uint32_t current_time=0;


/*--> MQTT CHARS <---*/
int mqtt_idx = 0; 		// 0->5


/*--> ble ota varibles<--*/
bool ble_stopped = false; // Indicador de si el BLE está detenido


/*--> global main variables <--*/
uint8_t delay_info = 1;
uint32_t Info_time=0;
uint32_t MQTT_read_time=0;
uint32_t OTA_md_time=1;

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

/**
 * Inicializa la configuración del módem, encendiéndolo y ejecutando los comandos de inicio.
 * 
 * @return MD_CFG_SUCCESS si la configuración se inicializa correctamente, MD_CFG_FAIL en caso contrario.
 */
int Init_config_modem(){
    ESP_LOGW(TAG, "--> INIT CONFIG MODEM <--");
    int state = 0;
    for (size_t i = 0; i < MAX_ATTEMPS; i++){
        state  = Modem_turn_ON();
        if (state !=MD_CFG_SUCCESS){
            Modem_turn_OFF();
            WAIT_S(3);
        }else{
            break;
        }
    }

    if(state != MD_CFG_SUCCESS) return state;

    for (size_t i = 0; i < MAX_ATTEMPS; i++){
        WAIT_S(2);
        state  = Modem_begin_commands();
        if (state ==MD_AT_OK){
            return MD_CFG_SUCCESS;
        }
    }
	return MD_CFG_FAIL;
}

/**
 * Activa el módem realizando la inicialización de la configuración y obteniendo la información del dispositivo.
 * 
 * @return MD_CFG_SUCCESS si el módem se activa correctamente, MD_CFG_FAIL en caso contrario.
 */
int Active_modem(){
	int status = Init_config_modem();
	if (status == MD_CFG_SUCCESS){
        int status = Modem_get_dev_info(&data_modem.info);
        if(status ==MD_CFG_SUCCESS){
            return MD_CFG_SUCCESS;   // FAIL
        }
    }
	return MD_CFG_FAIL;
}

/**
 * @brief Checks for OTA (Over-The-Air) updates.
 *
 * This function establishes a connection to an OTA server, sends device information,
 * and waits for a response. If an OTA update is pending (indicated by the response),
 * it initiates the OTA process.
 *
 * @note The specific details such as 'ip_OTA', 'port_OTA', and custom functions are
 * application-specific and should be defined elsewhere in your code.
 */
void OTA_Modem_Check(void){

    static const char *TAG_OTA = "OTA_MD";
    esp_log_level_set(TAG_OTA, ESP_LOG_INFO);

    ESP_LOGI(TAG_OTA,"==MODEM OTA CHECK ==");
    char buffer[500] ="";
    do{
        if(TCP_open(ip_OTA, port_OTA)!=MD_TCP_OPEN_OK){
            ESP_LOGW(TAG_OTA,"Not connect to the Server");
            TCP_close();
            break;
        }
        ESP_LOGI(TAG_OTA,"Requesting update...");
        if(TCP_send(output, strlen(output))==MD_TCP_SEND_OK){                           // 1. Se envia la info del dispositivo
            ESP_LOGI(TAG_OTA,"Waiting for response...");
            readAT("}\r\n", "-8/()/(\r\n",10000,buffer);   // 2. Se recibe la 1ra respuesta con ota True si tiene un ota pendiente... (el servidor lo envia justo despues de recibir la info)(}\r\n para saber cuando llego la respuesta)
            debug_ota("main> repta %s\r\n", buffer);
            if(strstr(buffer,"\"ota\": \"true\"") != 0x00){
                ESP_LOGI(TAG_OTA,"Start OTA download");
                ESP_LOGW(TAG_OTA,"WDT deactivate");
                watchdog_en=0;
                if(Ota_UartControl_Modem() == OTA_EX_OK){
                    ESP_LOGI(TAG_OTA,"OTA UPDATE SUCCESFULL, RESTART");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    esp_restart();
                }else{
                    ESP_LOGW(TAG_OTA,"FAIL OTA UPDATE");
                }
                current_time = pdTICKS_TO_MS(xTaskGetTickCount())/1000;
                watchdog_en=1;
                ESP_LOGW(TAG_OTA,"WDT reactivate");
            }
            int ret_tcp=TCP_close(); // close tcp
            printf("ret : 0x%x\r\n",ret_tcp);
        }
    }while(false);
    return;
}

/***********************************************
 * MODEM UART TASK
************************************************/

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
    ESP_LOGI("MAIN-MQTT","--> Revisando conexion <--");
    static int num_max_check = 0;
    static int ret_conn=0;
    static int ret_open=0;

    if (num_max_check >=(MAX_ATTEMPS+2)){
        ESP_LOGE("MAIN-MQTT", "RESTART ESP32");
        esp_restart();
    }
    printf("num_max: %d\r\n", num_max_check);

    ret_conn = Modem_Mqtt_CheckConn(mqtt_idx);
    printf("ret_conn: 0x%X\r\n", ret_conn);
    if (ret_conn == MD_MQTT_CONN_ERROR){
        ret_open=Modem_Mqtt_CheckOpen(mqtt_idx,ip_MQTT, port_MQTT);
        printf("ret_open: 0x%X\r\n",ret_open);
        if (ret_open == MD_CFG_FAIL){
            num_max_check ++;
        }else if (ret_open==MD_MQTT_IS_OPEN){
            //Conectamos son nuesto indice e imei
            Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
        }else if (ret_open==MD_MQTT_NOT_OPEN){
            num_max_check ++;
            // Configurar y Abrir  MQTT
            ret_open=Modem_Mqtt_Open(mqtt_idx,ip_MQTT,port_MQTT);
            if (ret_open == MD_MQTT_OPEN_OK){
                // Abrir comunicacion  y suscripcion
                Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
            }else{
                WAIT_S(2);
                // desconectar y cerrar en caso exista alguna comunicacion
                Modem_Mqtt_Disconnect(mqtt_idx);
                Modem_Mqtt_Close(mqtt_idx);
            }
        }
        CheckRecMqtt(); // Volvemos a verificar la conexion
    }else if (ret_conn==MD_MQTT_CONN_INIT || ret_conn == MD_MQTT_CONN_DISCONNECT){
        WAIT_S(1);
        Modem_Mqtt_Conn(mqtt_idx, data_modem.info.imei);
        CheckRecMqtt();  // CHECK AGAIN
    }else if (ret_conn==MD_MQTT_CONN_CONNECT){
        WAIT_S(1);
        CheckRecMqtt();  // CHECK AGAIN
    }else if (ret_conn==MD_MQTT_CONN_OK){
        num_max_check = 0;
        ESP_LOGI("MAIN-MQTT","CONNECT SUCCESFULL");
        // printf("TOPIC SUB: \'%s\'\r\n",topic_sub);
        // printf("ret_sub: 0x%X\r\n",state_mqtt_sub);
    }
	return ret_conn;
}

/************************************************
 * BLE CONTROLLERS (FUNCS AND TASK)
*************************************************/

void stop_ble() {
  ESP_LOGI(TAG, " BLE STOPED");
    if (!ota_updating) {
        nimble_port_stop();
        ble_stopped = true;
    }
}

// Función para reiniciar el BLE
void restart_ble() {
    ESP_LOGI(TAG, " BLE RESTART");
    
    if (ble_stopped) {
        nimble_port_init();
        ble_hs_cfg.sync_cb = sync_cb;
        ble_hs_cfg.reset_cb = reset_cb;
        gatt_svr_init();
        ble_svc_gap_device_name_set(device_name);
        nimble_port_freertos_init(host_task);
        ble_stopped = false;
    }
}

// Tarea para controlar el ciclo de detención y reinicio del BLE
static void ble_control_task(void *pvParameter) {
    ble_stopped = true;
    restart_ble();
    while (1) {
        // Reiniciar ble por 5 min
        // restart_ble();

        // Detener ble por 1 minuto
        // stop_ble(); 
        printf("HOLA MUNDO \n");
        vTaskDelay(60*1000 / portTICK_PERIOD_MS);
    }
}

bool run_diagnostics() {
    // Verificar si se realizó una actualización OTA correctamente
    // Si no se detectó una actualización OTA, simplemente retornar verdadero
    return true;
}

/**********************************************
 * MAIN TASK CONTROL AND FUNCS
***********************************************/
void Info_Send(void){
    ESP_LOGI("MQTT-INFO","<-- Send device info -->");


	static char topic[60]="";
	sprintf(topic,"%s/%s/INFO",MASTER_TOPIC_MQTT,data_modem.info.imei);

	time(&data_modem.time);
	data_modem.signal = Modem_get_signal();
	
	modem_info_to_json(data_modem, buff_aux);

	int ret_check =  CheckRecMqtt();
    ESP_LOGI("MQTT-INFO","ret-conn: 0x%X",ret_check);
	if(ret_check ==MD_MQTT_CONN_OK){
		ret_check = Modem_Mqtt_Pub(buff_aux,topic,strlen(buff_aux),mqtt_idx, 0);
        ESP_LOGI("MQTT-INFO","ret-pubb: 0x%X",ret_check);
        WAIT_S(1);
	}
	// Modem_Mqtt_Disconnect(mqtt_idx);
    return;
}

/**
 * Lee los datos de un mensaje MQTT y los procesa.
 */
void MQTT_Read(void){
    ESP_LOGI("MQTT-READ","<-- READ MQTT DATA -->");

    int state_mqtt_sub=-0x01;
    static char topic_sub[60]={0};
    sprintf(topic_sub,"%s/%s/CONFIG",MASTER_TOPIC_MQTT, data_modem.info.imei);

    state_mqtt_sub = Modem_sub_topic_json(mqtt_idx, topic_sub, buff_aux);
    printf("sub mqtt= 0x%X\r\n",state_mqtt_sub);
    if (state_mqtt_sub == MD_CFG_SUCCESS) {
        printf("DATA: %s\r\n",buff_aux);
        parse_json_example(buff_aux);
    }
    
    Modem_Mqtt_Unsub(mqtt_idx, topic_sub);
    /*
    uint8_t mem_mqtt[5]={0};
	int ret_check =  CheckRecMqtt();


    // ESP_LOGI("MQTT-READ","ret-conn: 0x%X",ret_check);
	if(ret_check ==MD_MQTT_CONN_OK){
        // sub topic
        state_mqtt_sub = Modem_Mqtt_Sub(mqtt_idx,topic_sub);
        printf("sub mqtt= 0x%X\r\n",state_mqtt_sub);
        WAIT_S(2);
        ret_check = Modem_Mqtt_Check_Buff(mqtt_idx, mem_mqtt);
        printf("buff mem= 0x%X\r\n",ret_check);
        if (ret_check == MD_MQTT_RECV_BUFF_DATA){
            size_t num_elementos = sizeof(mem_mqtt) / sizeof(mem_mqtt[0]);
            for (size_t i = 0; i < num_elementos; i++){
                if (mem_mqtt[i]==1){
                    ret_check=Modem_Mqtt_Read_data(mqtt_idx, i,buff_aux );
                    printf("red mem= 0x%X, num mem %d\r\n",ret_check,i);
                    if (ret_check == MD_SMS_READ_FOUND){
                        ESP_LOGW("MQTT-READ","DATA: %s",buff_aux);
                        Modem_Mqtt_Pub(buff_aux,"HOLA/DATA",strlen(buff_aux),mqtt_idx,0);
                    }
                }
            WAIT_S(1);
            }
        }
        
        // UnSub
        Modem_Mqtt_Unsub(mqtt_idx, topic_sub);
        
    }
    */

    return;
}

/*
    current_time = pdTICKS_TO_MS(xTaskGetTickCount())/1000;
    // OTA_Modem_Check();
    WAIT_S(2);
    // Registra la hora de inicio
    int64_t start_time, end_time, elapsed_time;
    start_time = esp_timer_get_time();
    CheckRecMqtt();
    end_time = esp_timer_get_time();
    elapsed_time = end_time - start_time;
    ESP_LOGI("TIMER", "Tiempo de ejecución: %lld microsegundos", elapsed_time);
    WAIT_S(2);

     int ret_sub;
    ret_sub= Modem_SubMqtt(mqtt_idx, "TEST/ID");
    printf("ret_sub: %d\r\n",ret_sub);
*/

static void Main_Task(void* pvParameters){
    WAIT_S(3);
	for(;;){
        
		current_time=pdTICKS_TO_MS(xTaskGetTickCount())/1000;
		if (current_time%30==0){
			printf("Tiempo: %lu\r\n",current_time);
		}

        // SEND INFO DATA
		if ((pdTICKS_TO_MS(xTaskGetTickCount())/1000) >= Info_time){
			current_time=pdTICKS_TO_MS(xTaskGetTickCount())/1000;
			Info_time+= delay_info*60;// cada 1 min
			if(ret_update_time!=MD_CFG_SUCCESS){
			    ret_update_time=Modem_update_time(1);
			}
            Info_Send();
            WAIT_S(1);
		}

        // SEND CHECK READ DATA
        if ((pdTICKS_TO_MS(xTaskGetTickCount())/1000) >= MQTT_read_time){
			current_time=pdTICKS_TO_MS(xTaskGetTickCount())/1000;
			MQTT_read_time+= 10;// cada 20 seg
            MQTT_Read();
            WAIT_S(1);
		}
        if ((pdTICKS_TO_MS(xTaskGetTickCount())/1000) >= OTA_md_time){
			current_time=pdTICKS_TO_MS(xTaskGetTickCount())/1000;
			OTA_md_time += 60;
			OTA_Modem_Check();
			printf("Siguiente ciclo en 60 segundos\r\n");
			printf("OTA CHECK tomo %lu segundos\r\n",(pdTICKS_TO_MS(xTaskGetTickCount())/1000-current_time));
			vTaskDelay(100);
		}

        if(Modem_check_AT()!=MD_AT_OK){
			WAIT_S(2);
            if(Modem_check_AT()!=MD_AT_OK){
                Modem_turn_OFF();
                // check if modem turn ON
                if(Active_modem()!= MD_CFG_SUCCESS) esp_restart();
                ret_update_time=Modem_update_time(1);
			}
		}
		WAIT_S(1);
	}
	vTaskDelete(NULL);
}



void app_main(void){
    ESP_LOGW(TAG, "--->> INIT PROJECT <<---");
	int ret_main = 0;

    const esp_partition_t *partition = esp_ota_get_running_partition();
    switch (partition->address) {
        case 0x00010000:
            ESP_LOGI(TAG, "Running partition: factory");
            break;
        case 0x00110000:
            ESP_LOGI(TAG, "Running partition: ota_0");
            break;
        case 0x00210000:
            ESP_LOGI(TAG, "Running partition: ota_1");
            break;
        default:
            ESP_LOGE(TAG, "Running partition: unknown");
        break;
    }

    // check if an OTA has been done, if so run diagnostics
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGI(TAG, "An OTA update has been detected.");
            if (run_diagnostics()) {
                ESP_LOGI(TAG,"Diagnostics completed successfully! Continuing execution.");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                ESP_LOGE(TAG,"Diagnostics failed! Start rollback to the previous version.");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }

    /*--- Initialize NVS ---*/
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Init Modem params
    main_cfg_parms();
    Modem_config();

    // INIT OTA BLE 
    xTaskCreate(Modem_rx_task, "M95_rx_task", 1024*4, NULL, configMAX_PRIORITIES -1,&UART_task_handle);    // active service
    xTaskCreate(ble_control_task, "BLE_Ctrl_Task", 1024*4, NULL, configMAX_PRIORITIES-2,&BLE_Task_handle);

    ret_main = Active_modem();
    if(ret_main != MD_CFG_SUCCESS) esp_restart();

	ESP_LOGW(TAG,"-->> END CONFIG <<--\n");

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
    ESP_LOGI(TAG,"CODE: %s", data_modem.code);
	ESP_LOGI(TAG,"SIGNAL: %d", data_modem.signal);
    
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

	OTA_md_time     = pdTICKS_TO_MS(xTaskGetTickCount())/1000 + 60;
    MQTT_read_time  = pdTICKS_TO_MS(xTaskGetTickCount())/1000 + 15;
	Info_time       = pdTICKS_TO_MS(xTaskGetTickCount())/1000 + 10;
	current_time    = pdTICKS_TO_MS(xTaskGetTickCount())/1000;
   

    xTaskCreate(Main_Task,"Main_Task",1024*8,NULL,10,&MAIN_task_handle);
}

