#include "EG915_modem.h"
#include "credentials.h"
#include "Modem_aux.h"

#include <time.h>
#include <sys/time.h>
#include <esp_task_wdt.h>
#include <esp_ota_ops.h>

/**********************************************
 * DEFINES
***************************************************/

#define WAIT_MS(x)		vTaskDelay(pdMS_TO_TICKS(x))
#define WAIT_S(x)		vTaskDelay(pdMS_TO_TICKS(x*1e3))

#define DEBUG_MODEM		0 // 0 -> NO DEBUG, 1-> DEBUG

// Definimos las funciones en la libreria
const char * TAG = "EG915";


/**********************************************
 * VARIABLES
***************************************************/
esp_ota_handle_t otaHandlerXXX = 0;


uint8_t Rx_data[BUFFER_SIZE]; //--
char *buff_reciv = (char *)&Rx_data[0];
char buff_send[BUFFER_SIZE];


// buffer para almacenar respuesta M95 despues de publicar en MQTT
uint64_t 	actual_time_M95;
uint64_t 	idle_time_m95;


void Modem_config_gpio(){
	ESP_LOGI(TAG, "--> GPIO CONFIG <--");

	ESP_ERROR_CHECK(gpio_reset_pin(modem_gpio.gpio_pwrkey));
	ESP_ERROR_CHECK(gpio_reset_pin(modem_gpio.gpio_status));
	
	gpio_reset_pin(modem_gpio.gpio_reset);
	gpio_set_direction(modem_gpio.gpio_reset, GPIO_MODE_OUTPUT);
    
	ESP_ERROR_CHECK(gpio_set_direction(modem_gpio.gpio_pwrkey, GPIO_MODE_OUTPUT));
	/// SP_ERROR_CHECK(gpio_pulldown_en(modem_gpio.gpio_status));
	ESP_ERROR_CHECK(gpio_set_direction(modem_gpio.gpio_status, GPIO_MODE_INPUT));
	WAIT_MS(10);
}

void Modem_config_uart(){
	ESP_LOGI(TAG, "--> UART CONFIG <--");
	uart_config_t uart_config_modem = {
		.baud_rate 	= modem_uart.baud_rate,
		.data_bits 	= UART_DATA_8_BITS,
		.parity 	= UART_PARITY_DISABLE,
		.stop_bits 	= UART_STOP_BITS_1,
		.flow_ctrl 	= UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};

	// instalar el controlador UART
	esp_err_t ret = uart_driver_install(modem_uart.uart_num,
										BUF_SIZE_MODEM*2,
										BUF_SIZE_MODEM*2,
										configMAX_PRIORITIES-1,
										&uart_modem_queue, 0);

	if (ret == ESP_FAIL) {
		// Si la instalación falla, se desinstala y vuelve a instalar
		ESP_ERROR_CHECK(uart_driver_delete(modem_uart.uart_num));
		ESP_ERROR_CHECK(uart_driver_install(modem_uart.uart_num,
											BUF_SIZE_MODEM*2,
											BUF_SIZE_MODEM*2,
											configMAX_PRIORITIES-1,
											&uart_modem_queue, 1));
	}

	// Configure UART parameters
	ESP_ERROR_CHECK(uart_param_config(modem_uart.uart_num, &uart_config_modem));

	// Set UART pins as per KConfig settings
	ESP_ERROR_CHECK(uart_set_pin(modem_uart.uart_num,
								modem_uart.gpio_uart_tx,
								modem_uart.gpio_uart_rx,
								UART_PIN_NO_CHANGE,
								UART_PIN_NO_CHANGE));

	
	// Modem start: OFF
	uart_flush(modem_uart.uart_num);
	return;
}

void Modem_config(){
	// Llama a las funciones de configuración
	ESP_LOGI(TAG, "--> CONFIG MODEM <--");
    Modem_config_gpio();
    Modem_config_uart();

    // Modem start: OFF
    gpio_set_level(modem_gpio.gpio_pwrkey,0);
    WAIT_MS(50);
	return;
}


int Modem_turn_ON(){
	ESP_LOGI(TAG,"=> TURN ON MODULE");
	int ret = 0;

	//--- CHECK UART-----//
	WAIT_MS(2000);
	// if (gpio_get_level(modem_gpio.gpio_status)){
	// WAIT_MS(2000);	//

	ret = Modem_check_AT();
	if (ret == 1) return ret;
	
	gpio_set_level(modem_gpio.gpio_pwrkey,0);
	WAIT_MS(500);
	gpio_set_level(modem_gpio.gpio_pwrkey,1);
	WAIT_MS(2000);	// >= 2s
	gpio_set_level(modem_gpio.gpio_pwrkey,0);
	WAIT_MS(10000);
	
	//--- CHECK UART-----//
	if (gpio_get_level(modem_gpio.gpio_status)){	//
		ret = Modem_check_uart();
	}
	WAIT_MS(2000);
	return ret; // 0: tun OFF, 1:turn ON
}

int Modem_check_AT(){
	int ret = 0;
	ESP_LOGI(TAG, "=> CHECK COMMAND AT");
	uart_flush(modem_uart.uart_num);

	WAIT_MS(500);
	ret = sendAT("AT\r\n","OK\r\n","ERROR\r\n",2000,buff_reciv);
	WAIT_MS(100);
	return ret;
}


int Modem_turn_OFF(){
	ESP_LOGI(TAG, "=> Turn off Module by PWRKEY");
	int status = gpio_get_level(modem_gpio.gpio_status);
	if (status!=0){
		gpio_set_level(modem_gpio.gpio_pwrkey,0);
		WAIT_MS(100);
		gpio_set_level(modem_gpio.gpio_pwrkey,1);
		WAIT_MS(3100);  // >3s //
		gpio_set_level(modem_gpio.gpio_pwrkey,0);
		WAIT_MS(1000);
	}
	status = gpio_get_level(modem_gpio.gpio_status);
	return status;	 // 0: tun OFF, 1:turn ON
}

int Modem_turn_OFF_command(){
	int ret = sendAT("AT+QPOWD=1\r\n","OK\r\n","FAILED\r\n",1000,buff_reciv);
	if( ret != 1){
		return sendAT("AT+QPOWD=0\r\n","OK\r\n","FAILED\r\n",1000,buff_reciv);
	}
	return ret;
}


void Modem_reset(){
	ESP_LOGI(TAG,"=> RESET MODEM");
	gpio_set_level(modem_gpio.gpio_reset,0);
	WAIT_MS(100);
	gpio_set_level(modem_gpio.gpio_reset,1);
	WAIT_MS(110);
	gpio_set_level(modem_gpio.gpio_reset,0);
	WAIT_MS(2000);
}

int Modem_check_uart(){
	// Enviamos el commando AT
	char* com = "AT\r\n";

	// Comando para iniciar comunicacion
	rx_modem_ready = 0;
	uart_write_bytes(modem_uart.uart_num, (uint8_t *)com, strlen(com));
	
	// Intentos
	int a = 2;
	ESP_LOGI("MODEM","Esperando respuesta ...");

	//rx_modem_ready = 0;
	while(a > 0){
		if( ( readAT("OK","ERROR", 1500, buff_reciv) == 1 ) ){
			printf("  Modem is active\n");
			return 1;	// OK
		}
		WAIT_MS(300);
		a--;
	}
	return 0; // fail
}




int Modem_begin_commands(){
	ESP_LOGI(TAG, "=> CONFIG FIRST AT COMMANDS");
	uart_flush(modem_uart.uart_num);

    int ret = sendAT("AT\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);	
	WAIT_MS(200);
	if (ret !=1) return ret;

	// restablecer parametros de fabrica
	sendAT("AT&F\r\n","OK\r\n","ERROR\r\n",1000,buff_reciv);		
	WAIT_MS(300);

	// Disable local echo mode
	sendAT("ATE0\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);		
	WAIT_MS(500);

	// Disable "result code" prefix
	sendAT("AT+CMEE=0\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);	
	WAIT_MS(500);

	// Indicate if a password is required
	sendAT("AT+CPIN?\r\n","OK","ERROR\r\n", 5000,buff_reciv);
	WAIT_MS(5000);
	
	// Signal Quality - .+CSQ: 6,0  muy bajo
	sendAT("AT+CSQ\r\n","+CSQ:","ERROR\r\n", 400,buff_reciv);
	WAIT_MS(200);

	//--- Operator Selection ---//
	ret=sendAT("AT+COPS=0\r\n", "OK","ERROR\r\n",180000,buff_reciv);
	WAIT_MS(200);
	// if (bandera !=1) return bandera;

	sendAT("AT+COPS?\r\n", "OK\r\n","ERROR\r\n",50000,buff_reciv);
	WAIT_MS(200);


	// APN CONFIG
    snprintf(buff_send, sizeof(buff_send), "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", APN);
	sendAT(buff_send,"OK","ERROR\r\n", 500,buff_reciv);
	WAIT_MS(200);
	
	// activate PDP
	sendAT("AT+CGACT?\r\n", "OK","ERROR",100000,buff_reciv);
	WAIT_MS(100);

	sendAT("AT+CGACT=1,1\r\n", "OK","ERROR",100000,buff_reciv);
	WAIT_MS(100);
	
	sendAT("AT+CGPADDR=1\r\n", "OK","ERROR",500,buff_reciv);
	WAIT_MS(100);


	// Enable Network register
	sendAT("AT+CGREG=1\r\n","OK\r\n","ERROR\r\n", 400, buff_reciv);
	WAIT_MS(200);

	// chek Network Registration Status	
	ret=sendAT("AT+CGREG?\r\n","1,","ERROR\r\n", 400, buff_reciv);
	WAIT_MS(200);

	sendAT("AT+CSMS?\r\n","OK\r\n","ERROR\r\n", 400, buff_reciv);
	WAIT_MS(200);

	// config sms
	sendAT("AT+CSMS=1\r\n","OK\r\n","ERROR\r\n", 400, buff_reciv);
	WAIT_MS(200);

	return ret;
}

// Función para procesar la respuesta y actualizar la hora
int Modem_sync_time(char* response) {
    char* ptr = strstr(response, "+QNTP: 0");
    if (ptr) {
        char* start = strchr(ptr, '\"');
        if (start) {
            start++;
            char* end = strchr(start, '\"');
            if (end) {
                *end = '\0'; *end = '\"';
            
                struct tm tm;
                struct timeval stv;
                memset(&tm, 0, sizeof(struct tm));
                strptime(start, "%Y/%m/%d,%H:%M:%S", &tm);

                time_t epoch_time = mktime(&tm);
                stv.tv_sec = epoch_time ;
                stv.tv_usec = 0;
                settimeofday(&stv, NULL);
            
                return 1;	// UPDATE OK
            }
        }
    }
    return 0; // FAIL UPDATE
}

// Función principal para actualizar la hora
int Modem_update_time(uint8_t max_int){
    int ret=0;
    char* servidor="time.google.com"; // Servidor NTP inicial
	for (size_t i = 0; i < max_int; i++){
		sprintf(buff_send, "AT+QNTP=1,\"%s\",123,1\r\n", servidor);
		WAIT_MS(100);
		if (sendAT(buff_send, "+QNTP:", "ERROR", 70000, buff_reciv)== 1) {
			ret = Modem_sync_time(buff_reciv);
			if (ret == 1) return ret;//OK UPDATE
			
			if (strcmp(servidor, "time.google.com") == 0) {
                servidor = "pool.ntp.org";
            }else {
                servidor = "time.google.com";
           	}
        }
	}
    return 0; // FAIL UPDATE
}



int sendAT(char *command, char *ok, char *error, uint32_t timeout, char *response){
	memset(response, '\0',strlen(response));
	uint8_t send_resultado = 0;

	actual_time_M95 = esp_timer_get_time();
	idle_time_m95 = (uint64_t)(timeout*1000);

	#if DEBUG_MODEM
		ESP_LOGI("SEND_AT","%s",command);
	#endif

	rx_modem_ready = 0;
	uart_write_bytes(modem_uart.uart_num, (uint8_t *)command, strlen(command));

	while( ( esp_timer_get_time() - actual_time_M95 ) < idle_time_m95 ){
		if( rx_modem_ready == 0 ){
			vTaskDelay( 10 / portTICK_PERIOD_MS );
			continue;
		}
		if ( strstr((char *)p_RxModem,ok) != NULL ){
			send_resultado = 1;     
			break;
		}
		else if( strstr((char *)p_RxModem,error) != NULL ){
			send_resultado = 2;
			break;
		}
		rx_modem_ready = 0;
	}


    if( send_resultado == 0 ){
		ESP_LOGE("RECIB_AT","TIMEOUT");
        return 0;
    }

	strcpy(response,(char*)p_RxModem);
	#if DEBUG_MODEM
		if(send_resultado == 1){
			ESP_LOGW("RECIB_AT","%s",response);
		}
		else {
			ESP_LOGE("RECIB_AT","%s",response);
		}
	#endif
	
	return send_resultado;
}

int readAT(char *ok, char *error, uint32_t timeout, char *response){
	//memset(response, '\0',strlen(response));
	int correcto = 0;
	bool _timeout = true;

	actual_time_M95 = esp_timer_get_time();
	idle_time_m95 = (uint64_t)(timeout*1000);

	rx_modem_ready = 0;

	while((esp_timer_get_time() - actual_time_M95) < idle_time_m95){
		if(rx_modem_ready == 0){
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			continue;
		}
		if (strstr((char *)p_RxModem,ok) != NULL){
			correcto = 1;
			_timeout = false;
			break;
		}
		else if(strstr((char *)p_RxModem,error)!= NULL){
			correcto = 2;
			_timeout = false;
			break;
		}
		vTaskDelay( 5 / portTICK_PERIOD_MS );
		rx_modem_ready = 0;
	}	

    if(_timeout){
		ESP_LOGE(TAG, "Modem not responded");
        return 0;
    }

	//memcpy(response,p_RxModem,rxBytesModem);
	strcpy(response,(char*)p_RxModem);
	#if DEBUG_MODEM
		ESP_LOGI(TAG, "-> %s",response);
	#endif
	return correcto;
}
/*------------------------------------------*/
int Modem_get_IMEI(char* imei){
	// buff_reciv -> Variable donde se almacena la respuesta del M95
	ESP_LOGI(TAG, "=> READ IMEI");

	int state_imei = sendAT("AT+QGSN\r\n","+QGSN","ERROR",2000,buff_reciv);
	WAIT_MS(200);

	if (state_imei ==1){
		char* inicio = strstr(buff_reciv, "+QGSN: ");
		if (inicio != NULL) {
			sscanf(inicio, "+QGSN: %15s", imei);
			return 1; // IMEI encontrado
		}
	}
	return 0; // IMEI no encontrado
}


int Modem_get_ICCID(char* iccid){
	// buff_reciv -> Variable donde se almacena la respuesta del M95
	ESP_LOGI(TAG, "=> READ ICCID");
	memset(iccid,'\0',strlen(iccid));
	int state_imei = sendAT("AT+QCCID\r\n","OK\r\n","ERROR\r\n",500,buff_reciv);
	WAIT_MS(200);

	if (state_imei ==1){
		char* inicio = strstr(buff_reciv, "+QCCID: ");
		if (inicio != NULL) {
			int ret = sscanf(inicio, "+QCCID: %20s", iccid);
			if (ret ==1){
				return 1; // ICCID encontrado
			}
		}
	}
	return 0; // ICCD no encontrado
}

int Modem_get_firmware(char* firmare){
	ESP_LOGI(TAG, "=> READ FIRMWARE");
	int ret = sendAT("AT+GMR=?\r\n","OK","ERROR",500,buff_reciv);
	if (ret!=1) return 0;
	WAIT_MS(200);   
	ret = sendAT("AT+GMR\r\n","OK\r\n","ERROR",500,buff_reciv);
	WAIT_MS(50);
	
	if(ret==1){
		char* inicio = strstr(buff_reciv, "EG915");
		if (inicio != NULL) {
			int ret = sscanf(inicio, "%20s", firmare);
			if (ret ==1){
				return 1; // firmware encontrado
			}
		}
	}
	
	return ret;
}

int Modem_get_dev_info(EG915_info_t* dev_modem){
	int ret = 0;
	ret = Modem_get_ICCID(dev_modem->iccid);
	if (ret!=1){
		return 0;
	}

	ret = Modem_get_IMEI(dev_modem->imei);
	if (ret!=1){
		return 0;
	} 

	ret = Modem_get_firmware(dev_modem->firmware);
	if (ret!=1){
		return 0;
	}
	
	return 1;
}
/*-------------------------------*/
char*  Modem_get_date(){
	int a = sendAT("AT+CCLK?\r\n","+CCLK: ","ERROR",1000,buff_reciv);
	if( a != 1){
		printf("  hora no conseguida\n\r");	
		return NULL;		
	}
	char lim[3] = "\"";
	char * p_clock = strtok(buff_reciv,lim);
	while(p_clock != NULL){
		if(strstr(p_clock,"/")!= NULL){
			break;
		}
		p_clock = strtok(NULL,lim);
		// printf("%s\n",p_clock);
	}
	return (char*) p_clock;
}

time_t Modem_get_date_epoch() {

	char *date_str =  Modem_get_date();
	if (date_str == NULL) {
		return 0;	
    } 

	struct tm timeinfo = { 0 };
    int year, month, day, hour, minute, second;

    // Parsea la fecha y hora de la cadena de texto
    sscanf(date_str, "%2d/%2d/%2d,%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);

    timeinfo.tm_year = year + 100; 	// Los años se cuentan desde 1900
    timeinfo.tm_mon = month - 1; 	// Los meses se cuentan desde 0
    timeinfo.tm_mday = day;		 
    timeinfo.tm_hour = hour;	// La diferencia de Horas de formato UTC global
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    // Convierte struct tm a tiempo epoch
    time_t epoch_time = mktime(&timeinfo);
	
	return epoch_time;
}

/** 
    @brief  Obtenemos la calidad de señal
 */
int Modem_get_signal(){
	ESP_LOGI(TAG,"--> MODEM GET SIGNAL <--");
	char *linea;
	int valor1, valor2;
	int a = sendAT("AT+CSQ\r\n","+CSQ","ERROR",500,buff_reciv);
	if (a==1){
		linea = strtok(buff_reciv, "\n");
		while (linea != NULL) {
			// Intenta extraer los valores de la línea
			if (sscanf(linea, "+CSQ: %d,%d", &valor1, &valor2) == 2) {
				return valor1;
			}
			// Obtiene la siguiente línea
			linea = strtok(NULL, "\n");
		}
	}
	return 99;
}


int Modem_Mqtt_Open(int idx, const char* MQTT_IP, const char* MQTT_PORT){
	int ret=-1;
	char respuesta_esperada[20]={0};

	// config Params to MQTT
	sprintf(buff_send,"AT+QMTCFG=\"pdpcid\",%d\r\n",idx);
	sendAT(buff_send,"OK\r\n","ERROR\r\n",500,buff_reciv);
	WAIT_MS(50);
	
	sprintf(buff_send,"AT+QMTCFG=\"version\",%d,3\r\n",idx);
	sendAT(buff_send,"OK\r\n","ERROR\r\n",500,buff_reciv);
	WAIT_MS(50);

	sprintf(buff_send,"AT+QMTCFG=\"recv/mode\",%d,0,1\r\n",idx);
	sendAT(buff_send,"OK","ERROR\r\n",500,buff_reciv);
	WAIT_MS(50);

	sprintf(buff_send,"AT+QMTCFG=\"timeout\",%d,10,3,0\r\n",idx);
	sendAT(buff_send,"OK","ERROR\r\n",500,buff_reciv);
	WAIT_MS(50);

	sendAT("AT+QMTOPEN?\r\n","OK","ERROR\r\n",1000,buff_reciv);
	WAIT_MS(500);

	sprintf(buff_send,"AT+QMTOPEN=%d,\"%s\",%s\r\n",idx, MQTT_IP, MQTT_PORT);
	sprintf(respuesta_esperada,"+QMTOPEN: %d,",idx);
	ret = sendAT(buff_send,respuesta_esperada,"ERROR\r\n",100000,buff_reciv);
	WAIT_MS(200);
	if (ret!=1){
		ret = -2; // ERROR DE RESPUESTA 
		return ret;
	}
	char *p = strchr(buff_reciv, ',') + 1;
	if (p !=NULL){
		ret = atoi(p);
		switch (ret){
			case -1:
				ESP_LOGI(TAG,"Failed to open network");
				break;
			case 0:
				ESP_LOGI(TAG,"Network opened successfully");
				break;
			case 1:
				ESP_LOGI(TAG, "Wrong parameter");
				break;
			case 2:
				ESP_LOGI(TAG, "identifier is occupied");
				break;
			case 3:
				ESP_LOGI(TAG, "Failed to activate PDP");
				break;
			case 4:
				ESP_LOGI(TAG, "Failed to parse domain name");
				break;
			case 5:
				ESP_LOGI(TAG, "Network connection error");
				break;
			default:
				break;
		}
	}else{
		ret=-3;	// NOT FOUND ','
	}
	return ret;
}

int Modem_CheckMqtt_Open(int idx, char* MQTT_IP, char* MQTT_PORT){
	char res_esperada[50]={0};
	sprintf(res_esperada,"+QMTOPEN: %d,",idx);
	int ret = sendAT("AT+QMTOPEN?\r\n","OK\r\n","ERROR\r\n",10000,buff_reciv);
	WAIT_MS(200);
	if (ret==1){
		if (strstr(buff_reciv,res_esperada)!=NULL){
			ret=1; // Mqtt is Openning;
		}else{
			ret=0;	// Mqtt is not Openning
		}
	}else{
		ret = -1; // ERROR
	}
	return ret;
}




int Modem_Mqtt_Conn(int idx, const char* clientID){
	ESP_LOGI(TAG,"--> MQTT CONNECTION <--");

	int ret=0;
	char res_esperada[20]={0};
	sprintf(res_esperada,"+QMTCONN: %d,",idx);
	sprintf(buff_send,"AT+QMTCONN=%d,\"%s\"\r\n",idx, clientID);
	ret = sendAT(buff_send, res_esperada,"ERROR\r\n",10000,buff_reciv);

	if (ret==1){
		char *p = strchr(buff_reciv, ',') + 1;
		if (p!=NULL){
			ret = atoi(p);
			switch (ret){
				case 0:
					ESP_LOGI(TAG, "Connection Accepted");
					break;
				case 1:
					ESP_LOGW(TAG, "Unacceptable Protocol Version");
					break;
				case 2:
					ESP_LOGW(TAG, "Server Unavailable");
					break;
				case 3:
					ESP_LOGW(TAG, "Bad Username or Password");
					break;
				case 4:
					ESP_LOGW(TAG, " Not Authorized");
					break;
				default:
					break;
			}
		}else{
			ret = -2;
		}
	}else{
		ret=-1;
	}
	WAIT_MS(100);
	return ret;
}

int  Modem_CheckMqtt_Conn(int idx){
	ESP_LOGI(TAG,"--> MQTT CHECK CONNECTION <--");
	char res_esperada[30]={0};
	sprintf(res_esperada,"+QMTCONN: %d,",idx);
	
	int ret = sendAT("AT+QMTCONN?\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);
	WAIT_MS(100);
	if (ret==1){
		if (strstr(buff_reciv,res_esperada)!=NULL){
			char *p = strchr(buff_reciv, ',') + 1;
			if (p!=NULL){
				ret = atoi(p);
				switch (ret){
					case 1:
						ESP_LOGI(TAG, "is initializing");
						break;
					case 2:
						ESP_LOGI(TAG, "is connecting");
						break;
					case 3:
						ESP_LOGI(TAG, "is connected");
						break;
					case 4:
						ESP_LOGI(TAG, "is disconnecting");
						break;
					default:
						break;
				}
			}
		}else{
			ret=-2;
		}
	}else{
		ret =-1;
	}
	return ret;
}


int  Modem_Mqtt_Close(int idx){
	int ret;
	sprintf(buff_send,"AT+QMTCLOSE=%d\r\n",idx);
	ret = sendAT(buff_send,"+QMTCLOSE:","ERROR",30000,buff_reciv);
	WAIT_MS(100);
	return ret;
}

int Modem_Mqtt_Disconnect(int idx){
	int ret;
	sprintf(buff_send,"AT+QMTDISC=%d\r\n",idx);
	ret = sendAT(buff_send,"+QMTDISC:","ERROR\r\n", 30000,buff_reciv);
	WAIT_MS(100);
	return ret;
}


int  Modem_PubMqtt_data(uint8_t * data,char * topic,int data_len, int id , int retain){
	int k = 0;
	sprintf(buff_send,"AT+QMTPUBEX=%d,1,0,%d,\"%s\",%d\r\n",id,retain,topic,data_len);
	//printf("Pub_msg=\n%s\n",buff_send);
	sendAT(buff_send,">","ERROR",1000,buff_reciv);
	vTaskDelay(50);
	uart_write_bytes(modem_uart.uart_num,data,data_len);
	k = readAT("+QMT","ERROR",15000,buff_reciv);
	WAIT_MS(500);
	
	//remove_spaces(buff_reciv);
	printf("recib: %s \n", buff_reciv);
	if(k != 1) return 0;

	return 1;
}

int Modem_sub_topic_json(int ID, char* topic_name, char* response){
    sprintf(buff_send,"AT+QMTSUB=%d,1,\"%s\",0\r\n",ID,topic_name);
    int success = 0;    
    if(sendAT(buff_send,"+QMTRECV:","ERROR\r\n",20000,buff_reciv) == 1){
        success = 1;
    }

    if(success == 0){
        ESP_LOGE("MQTT Subs","No se recibio respuesta del topico:\n%s\n",topic_name);
        return 0;
    }
	
	char *start;
	start = strchr(buff_reciv, '{');

	// Si se encontró la llave
	if(start != NULL){

		char *data = strdup(start);
		strcpy(response,data);

		free(data);
		return 1; // RESPUESTA OK
	}
    return -1;
}

int Modem_unsub_topic(int ID, char* topic_name){
	sprintf(buff_send,"AT+QMTUNS=%d,1,\"%s\"\r\n",ID, topic_name);
	int a = sendAT(buff_send,"+QMTUNS:","ERROR",12000,buff_reciv);
	printf("->%s\n",buff_reciv);
	WAIT_MS(100);

	return a;
}

/***************************************************************
 * SMS type
**************************************************************/
/*-----------------------*/



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

/*------------------------------------------------------------*/

int Modem_readSMS(char* mensaje, char *numero){
	int ret = 0;
	ret = sendAT("AT+CMGF=1\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);
	vTaskDelay(pdMS_TO_TICKS(100));
	
	ret = sendAT("AT+CMGL=?\r\n","OK","ERROR",1000,buff_reciv);
	vTaskDelay(pdMS_TO_TICKS(100));
	if (ret !=1){
		return 0;
	}
	
	ret = sendAT("AT+CMGL=\"REC UNREAD\"\r\n","+CMGL: ","ERROR",1000,buff_reciv);
	// ret = sendAT("AT+CMGR=1\r\n","+CMGR: ","ERROR",1000,buff_reciv);
	int n_args=0;
	if(ret==1){
		// printf("%s\n",buff_reciv);
		data_sms_strt_t result_data;
		ret = str_to_data_sms(buff_reciv,&result_data);
		for (int i = 0; i < result_data.lines; i++) {
			printf("Línea %d: %s\n", i + 1, result_data.data[i]);
			if (i==0){
				ret = find_phone_and_extract(result_data.data[i], numero);
				if (ret ==1){
					#if DEBUG_MODEM
					ESP_LOGW(TAG,"phone:%s",numero);
					#endif
					n_args++;
				}
			}else if(i==1){	
				strcpy(mensaje,result_data.data[i]);
				#if DEBUG_MODEM
				ESP_LOGW(TAG,"msg:%s",mensaje);
				#endif
				n_args++;
			}
		}
		// Libera la memoria
		free_data(&result_data);
		Modem_delete_SMS();
	
	}
	
	vTaskDelay(pdMS_TO_TICKS(200));
	return n_args;
}


int Modem_sendSMS(char* mensaje, char *numero){
	int ret = 0;
	memset(buff_send, '\0',strlen(buff_send));
	sprintf(buff_send,"AT+CMGS=\"%s\"\r\n",numero);
	sendAT(buff_send,">", "ERROR\r\n", 5000,buff_reciv);

	memset(buff_send, '\0', strlen(buff_send)); 
	sprintf(buff_send, "%s%c", mensaje,26); // sms + 'Ctrl+Z'
	uart_flush(modem_uart.uart_num);
	uart_write_bytes(modem_uart.uart_num, buff_send, strlen(buff_send));
	uart_wait_tx_done(modem_uart.uart_num, pdMS_TO_TICKS(100000));
	WAIT_MS(100);

	ret = readAT("+CMGS", "ERROR", 10000, buff_reciv);
	return ret;
}


int Modem_delete_SMS(){
	int ret = 0;
	ret = sendAT("AT+CMGF=1\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);
	vTaskDelay(pdMS_TO_TICKS(200));
	ret = sendAT("AT+CMGD=1,4\r\n","OK\r\n","ERROR\r\n",5000,buff_reciv);
	return ret;
}


/*----------------------------------------------------*/
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


/************************************
 * TCP OPEN
****************************************/


uint8_t TCP_open(char *IP, char *PORT){
    uint8_t temporal=0;

	// AT+QICSGP=3,1,"movistar.pe","","",1
	sprintf(buff_send,"AT+QICSGP=3,1,\"%s,\"\",\"\",1\r\n",APN);
    sendAT(buff_send,"OK","ERROR",10000,buff_reciv);
	WAIT_MS(100);

    // Activate a PDP Context
    sendAT("AT+QIACT=1\r\n","OK","ERROR",150000,buff_reciv);
	WAIT_MS(100);

	 sendAT("AT+QIACT?\r\n","OK","ERROR",10000,buff_reciv);

    //IMPORTANTE: LEER ESTE COMANDO

    //char qiopen[40];
    sprintf(buff_send,"AT+QIOPEN=1,0,\"TCP\",\"%s\",%s,0,0\r\n",IP,PORT);
    temporal= sendAT(buff_send,"+QIOPEN: 0,0","ERROR",150000,buff_reciv);
    return temporal; //1 == ok 1 != error
}


uint8_t TCP_send(char *msg, uint8_t len){
    //sendAT(m95,"ATE1\r\n","OK\r\n","ERROR\r\n",5000,  m95->buff_reciv);//DES Activa modo ECO.
    uint8_t temporal = 0;
	memset(buff_send,'\0',strlen(buff_send));
	//sprintf(buff_send,"AT+QISEND=0\r\n",len);
    temporal = sendAT("AT+QISEND=0\r\n",">","ERROR\r\n",25500,buff_reciv);
    if(temporal != 1){
        return 0;
    }

    // uart_write_bytes(modem_uart.uart_num,(void *)msg,len);
	uart_write_bytes(modem_uart.uart_num,(void *)msg,len);
    temporal = sendAT("\x1A","SEND OK\r\n","ERROR\r\n",25500,buff_reciv);

	// temporal = sendAT(msg,"SEND OK\r\n","ERROR\r\n",25500,buff_reciv);

    if(temporal != 1){
        return 0;
    }
	ESP_LOGI("TCP","SEND OK CORRECTO\r\n");
    memset(buff_reciv, 0 , 50);
    return 1;
}

void TCP_close(){
    sendAT("AT+QICLOSE=0\r\n","OK\r\n","ERROR\r\n",25500,buff_reciv);
	WAIT_MS(100);
    if(Modem_check_AT()!=1){
        if(Modem_check_AT()!=1){
			ESP_LOGE(TAG," FAIL CLOSE TCP");
            // reiniciar();
        }
    }
	return;
}


uint8_t OTA(uint8_t *buff, uint8_t *inicio, uint8_t *fin, uint32_t len){
    const char *TAG = "OTA";
    if (*inicio) { //If it's the first packet of OTA since bootup, begin OTA
        ESP_LOGI(TAG,"BeginOTA");
        //Serial.println("BeginOTA");
        const esp_task_wdt_config_t config_wd = {
                .timeout_ms = 20,
                .idle_core_mask = 0,
                .trigger_panic = false,
            };
        esp_task_wdt_init(&config_wd);
        esp_ota_begin(esp_ota_get_next_update_partition(NULL), OTA_SIZE_UNKNOWN, &otaHandlerXXX);
        *inicio = 0;
    }
    if (len > 0)
    {
        esp_ota_write(otaHandlerXXX,(const void *)buff, len);

        if (len != 512 || *fin ==1)
        {
            esp_ota_end(otaHandlerXXX);
            const esp_task_wdt_config_t config_wd = {
                .timeout_ms = 5,
                .idle_core_mask = 0,
                .trigger_panic = false,
            };
            esp_task_wdt_init(&config_wd);
            ESP_LOGI(TAG,"EndOTA");
            //Serial.println("EndOTA");
            if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                esp_restart();
            }
            else {
                ESP_LOGI(TAG,"Upload Error");
                //Serial.println("Upload Error");
            }
        }

    }

    //uint8_t txData[5] = {1, 2, 3, 4, 5};
    //delay(1000);
    return 1;
}