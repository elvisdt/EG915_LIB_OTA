#ifndef _ERROS_MODEM_H_
#define _ERROS_MODEM_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


/*--->> error code to modem <<---*/
// Config
#define MD_CFG_SUCCESS      0x00
#define MD_CFG_FAIL         (MD_CFG_SUCCESS - 0x01)

//AT Answer
#define MD_AT_OK            MD_CFG_SUCCESS 
#define MD_AT_ERROR         (MD_AT_OK - 0x01)
#define MD_AT_TIMEOUT       (MD_AT_OK - 0x02)


        
//MQTT
#define MD_BASE_MQTT            0x1000
#define MD_MQTT_ERR_GEN        (0x1111)

#define MD_MQTT_IS_OPEN         MD_CFG_SUCCESS
#define MD_MQTT_NOT_OPEN        MD_MQTT_ERR_GEN

#define MD_MQTT_OPEN_OK         MD_CFG_SUCCESS
#define MD_MQTT_OPEN_FAIL_NET   (MD_BASE_MQTT + 0x11)
#define MD_MQTT_OPEN_FAIL_PRM   (MD_BASE_MQTT + 0x12)
#define MD_MQTT_OPEN_FAIL_IDX   (MD_BASE_MQTT + 0x13)
#define MD_MQTT_OPEN_FAIL_PDP   (MD_BASE_MQTT + 0x14)
#define MD_MQTT_OPEN_FAIL_DOM   (MD_BASE_MQTT + 0x15)
#define MD_MQTT_OPEN_FAIL_CNN   (MD_BASE_MQTT + 0x16)


#define MD_MQTT_CONN_OK          MD_CFG_SUCCESS
#define MD_MQTT_CONN_ERROR       MD_MQTT_ERR_GEN
#define MD_MQTT_CONN_FAIL_VER    (MD_BASE_MQTT + 0x21)
#define MD_MQTT_CONN_FAIL_SRV    (MD_BASE_MQTT + 0x22)
#define MD_MQTT_CONN_FAIL_UPS    (MD_BASE_MQTT + 0x23)
#define MD_MQTT_CONN_FAIL_AUT    (MD_BASE_MQTT + 0x24)

#define MD_MQTT_CONN_INIT        (MD_BASE_MQTT + 0x25) // El cliente MQTT está inicializando la conexión.
#define MD_MQTT_CONN_CONNECT     (MD_BASE_MQTT + 0x26) // El cliente MQTT está intentando conectar.
#define MD_MQTT_CONN_DISCONNECT  (MD_BASE_MQTT + 0x27) // El cliente MQTT está desconectando.


#define MD_MQTT_PUB_OK           MD_CFG_SUCCESS
#define MD_MQTT_PUB_FAIL         MD_MQTT_ERR_GEN


#define MD_MQTT_SUB_OK           MD_CFG_SUCCESS
#define MD_MQTT_SUB_FAIL         MD_MQTT_ERR_GEN
#define MD_MQTT_SUB_RETP         (MD_BASE_MQTT  + 0x51)
#define MD_MQTT_SUB_UNKNOWN      (MD_BASE_MQTT  + 0x52)


#define MD_MQTT_RECV_OK           (MD_CFG_SUCCESS)
#define MD_MQTT_RECV_FAIL         (MD_MQTT_ERR_GEN)
#define MD_MQTT_RECV_BUFF_DATA    (MD_BASE_MQTT + 0x61)
#define MD_MQTT_RECV_BUFF_CLEAN   (MD_BASE_MQTT + 0x62)
#define MD_MQTT_RECV_UNKOWN       (MD_BASE_MQTT + 0x63)



// SMS
#define MD_BASE_SMS                 0x2000

#define MD_SMS_READ_FOUND          (MD_BASE_SMS  + 0x01)
#define MD_SMS_READ_NO_FOUND       (MD_SMS_READ_FOUND + 0x01)
#define MD_SMS_READ_UNKOWN         (MD_SMS_READ_FOUND + 0x02)

#define MD_SMS_SEND_OK              (MD_BASE_SMS  + 0x10)
#define MD_SMS_SEND_FAIL            (MD_SMS_SEND_OK + 0x01)
#define MD_SMS_SEND_NO_ANS          (MD_SMS_SEND_OK + 0x02)



#define MD_BASE_TCP         0x2100
#define MD_TCP_ERR_GEN      (MD_BASE_TCP - 0x01)

#define MD_TCP_OPEN_OK      (MD_CFG_SUCCESS)
#define MD_TCP_OPEN_FAIL    (MD_TCP_ERR_GEN)

#define MD_TCP_CLOSE_OK     (MD_CFG_SUCCESS)
#define MD_TCP_CLOSE_FAIL   (MD_TCP_ERR_GEN)

#define MD_TCP_SEND_OK      (MD_CFG_SUCCESS)
#define MD_TCP_SEND_FAIL    (MD_TCP_ERR_GEN)


#endif /*_ERROS_MODEM_H_*/