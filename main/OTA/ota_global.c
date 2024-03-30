#include "ota_global.h"

void debug_ota(char const *Buff, ...){
  if( !DEBUG_OTA_ON){
    return ;
  }
  static const char *CHECK_TAG = "debug_ota";
  esp_log_level_set(CHECK_TAG, ESP_LOG_INFO);

  char buff[500];
  va_list args;
  va_start(args,Buff);
  vsprintf(buff,(char *)Buff,args);
  ESP_LOGI(CHECK_TAG,"%s",buff);
  //DEBUG_OTA_SERIAL.print(buff);
  return;
}
