#ifndef _CREDENTIALS_H_
#define _CREDENTIALS_H_

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


#define UDP_CONNECTION
// #define TCP_CONNECTION
// #define MQTT_CONNECTION


#define APN     "movistar.pe"//"smglobal.entel.pe" //


// #define APN    "sim2m"

/**
 * OTA: defined OTA parameters
*/
#define ip_OTA      "18.229.227.108"
#define port_OTA    "65431"  


/**
 * MQTT: defined MQTT parameters
*/
#define ip_MQTT     "161.97.102.234" //"3.129.163.139" //"34.176.125.182"//
#define port_MQTT   "1883"



#endif /*_CREDENTIALS_H_*/