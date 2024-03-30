/*
 * ota_usb.c
 *
 *  Created on: 3 set. 2022
 *      Author: Franco
 */

#include "ota_m95.h"

#define DEBUG_PRINT_DATA 0 /*0 DISABLE, 1: ENABLE*/

static uint8_t Rx_Buffer[ OTA_PACKET_MAX_SIZE ];

OTA_EX_ ota_uartControl_M95(void){
  uint16_t  ret = OTA_EX_OK;
  uint16_t len;
  ota_init();
  debug_ota("Init");
  do{
      len = ota_uart_read_M95( Rx_Buffer, OTA_PACKET_MAX_SIZE );
      if( len != 0u ){
        ret = ota_flash( Rx_Buffer, len );
      }else{
        ret = OTA_EX_ERR;
      }

      if( ret != OTA_EX_OK ){
        debug_ota("ota_m95> ERROR, Sending NACK\r\n");
        ota_uart_send_resp_M95( OTA_NACK);
        break;
      }
      else{
        debug_ota("ota_m95> OK, Sending ACK\r\n");
        ota_uart_send_resp_M95( OTA_ACK);
      }

  }while( ota_d.ota_state != OTA_STATE_IDLE);
  return (OTA_EX_)ret;
}



uint16_t ota_uart_read_M95( uint8_t *data, uint16_t max_len){
  //int16_t  ret;
  uint16_t index   =  0u;
  //uint16_t data_len;

  uint64_t timeout = esp_timer_get_time();
  uint32_t idle_timeout = esp_timer_get_time();
  bool SOF = false;
  size_t len_data = 0;
  rx_modem_ready = 0;
  uint8_t * pointer = p_RxModem;
  while(true){
    // timeout general
    if( (esp_timer_get_time()-timeout)/1000 > 25000){
      debug_ota("ota_m95> ERROR, timeout index=%d\r\n", index);
      return 0;
      break;
    }
    if((rx_modem_ready == 1)&(rxBytesModem != 0)){
      debug_ota("data recibida : %d bytes",rxBytesModem);
      len_data = rxBytesModem;
    }

    if(len_data != 0){
      #if DEBUG_PRINT_DATA
        printf("data:\r\n");
      #endif
    
      for (size_t i = 0; i < len_data; i++)
      {
        if(index >= OTA_PACKET_MAX_SIZE+2){
          debug_ota("ota_m95> ERROR, ota packet max size\r\n");
          return 0;
        }
        data[index] = *pointer;
        pointer++;
        #if DEBUG_PRINT_DATA
          printf("0x%x",data[index]);
        #endif
        if(data[index] == 0xAA){
          debug_ota("SOF");
          SOF = true;
        }
        if(SOF){
          index = index+1;
        }
      }
      pointer = p_RxModem;
      len_data = 0;
      #if DEBUG_PRINT_DATA
         printf("\r\n****\r\n");
      #endif
     
      idle_timeout = esp_timer_get_time();
      //data[index] = m95->M95Serial->read();
      rx_modem_ready = 0;
    }

    // idle timeout
    if ( (index > 3) && ((esp_timer_get_time() - idle_timeout)/1000 > 500) ){
      debug_ota("ota_m95> OK, data recibida -> index %d\r\n", index);
      break;
    }
    vTaskDelay(1);

  }
  return index;
}


OTA_EX_ ota_uart_send_resp_M95(uint8_t ans){
  OTA_RESP_ rsp =
  {
    .sof         = OTA_SOF,
      .packet_type = OTA_PACKET_TYPE_RESPONSE,
      .data_len    = 1u,
      .status      = ans,
      .crc         = 0u,                //TODO: Add CRC
      .eof         = OTA_EOF
  };

  //send response
  //HAL_UART_Transmit(&huart6, (uint8_t *)&rsp, sizeof(OTA_RESP_), HAL_MAX_DELAY);
  TCP_send((char *)&rsp, sizeof(OTA_RESP_));

  return OTA_EX_OK;
}
