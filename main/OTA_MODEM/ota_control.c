/*
 * ota_control.c
 *
 *  Created on: 3 set. 2022
 *      Author: Franco
 */
#include "ota_control.h"

ota_t ota_d;

OTA_EX_ ota_init( void ){
  ota_d.ota_state = OTA_STATE_START;
  ota_d.packet_number = 0; //
  ota_d.total_packets = 0; //
  ota_d.total_size = 0; //bytes
  ota_d.received_size = 0;
  memset(ota_d.filename, 0, 30);
  return OTA_EX_OK;
}

OTA_EX_ ota_validate(uint8_t *data, uint16_t data_len){

  // CRC Check
  OTA_EX_ ret  = OTA_EX_OK;

  uint16_t data_chunk_size = data[2] + (data[3]<<8);

  uint16_t crc_calculado= crcr16dnp(&data[4], data_chunk_size , 0xFFFF);
  uint16_t crc_offset = data_chunk_size+4;
  uint16_t crc =(uint16_t)( data[crc_offset] + (data[crc_offset + 1]<<8) + (data[crc_offset+2]<<16) + (data[crc_offset+3] << 24));
  if( crc_calculado != crc){
    debug_ota("ota_control> Error CRC\r\n");
    ret = OTA_EX_ERR;
  }

  return ret;
}


OTA_EX_ ota_process(uint8_t *data, uint16_t data_len){
  OTA_EX_ ret = OTA_EX_ERR;

  do{
    if( ( data == NULL ) || ( data_len == 0u) )
    {
      break;
    }

    // Ota Abort Check
    OTA_COMMAND_ *cmd = (OTA_COMMAND_*)data;
    if( cmd->packet_type == OTA_PACKET_TYPE_CMD )
    {
      if( cmd->cmd == OTA_CMD_ABORT )
      {

        debug_ota("ota_control> OTA Abort");
        break;
      }
    }

    switch( ota_d.ota_state )
    {
      case OTA_STATE_IDLE:
      {
        printf("ota_control> OTA_STATE_IDLE...\r\n");
        ret = OTA_EX_OK;
      }
      break;

      case OTA_STATE_START:
      {
        OTA_COMMAND_ *cmd = (OTA_COMMAND_*)data;

        if( cmd->packet_type == OTA_PACKET_TYPE_CMD )
        {
          if( cmd->cmd == OTA_CMD_START )
          {
            debug_ota("ota_control> Received OTA START Command\r\n");
            ota_d.ota_state = OTA_STATE_HEADER;
            ret = OTA_EX_OK;
          }
        }
       }
       break;

       case OTA_STATE_HEADER:
        {
        OTA_HEADER_ * header = (OTA_HEADER_*)data;
        if( header->packet_type == OTA_PACKET_TYPE_HEADER )
        {
          ota_d.total_size = header->meta_data.package_size;
          ota_d.crc16    = header->meta_data.package_crc;
          //meta_info  * temp  = &(header->meta_data);
          char * name_temp = (char *)&(header->meta_data.filename[0]);
          //sprintf(ota_d.filename,"%s", header->meta_data.filename);
          //sprintf(ota_d.filename,"%s",name_temp);
          memcpy(ota_d.filename,name_temp,30);
          debug_ota("ota_control> Received OTA Header. filename %s .FW Size = %d\r\n", ota_d.filename,ota_d.total_size);
          ota_d.ota_state = OTA_STATE_DATA;
          ret = OTA_EX_OK;
        }
        }
        break;

        case OTA_STATE_DATA:
        {
        OTA_DATA_         *data_chunk    = (OTA_DATA_*)data;
        uint16_t          data_len = data_chunk->data_len;
        HAL_StatusTypeDef ex;

        if( data_chunk->packet_type == OTA_PACKET_TYPE_DATA )
        {
          /* write the chunk to the Flash (App location) */
          ex = write_data_to_flash_app( (uint8_t *)&data_chunk[0], data_len, ( ota_d.received_size == 0), &ota_d );

          if( ex == HAL_OK )
          {
          debug_ota("ota_control> [%lu/%lu]\r\n", ota_d.received_size, ota_d.total_size);
          if( ota_d.received_size >= ota_d.total_size)
          {
            //received the full data. So, move to end
            ota_d.ota_state = OTA_STATE_END;
          }
          ret = OTA_EX_OK;
          }
        }
        }
        break;

        case OTA_STATE_END:
        {

        OTA_COMMAND_ *cmd = (OTA_COMMAND_*)data;

        if( cmd->packet_type == OTA_PACKET_TYPE_CMD )
        {
          if( cmd->cmd == OTA_CMD_END )
          {
          debug_ota("ota_control> Received OTA END Command\r\n");



          ota_d.ota_state = OTA_STATE_IDLE;
          ret = OTA_EX_OK;
          }
        }
        }
        break;

        default:
        {

        debug_ota("ota_control> Error default state\r\n");
        ret = OTA_EX_ERR;
        }
        break;
      };
  }while(false);
   return ret;
}

OTA_EX_ ota_flash(uint8_t *data, uint16_t data_len){

    OTA_EX_ ret  = OTA_EX_OK;

    do{
      ret = ota_validate(data, data_len);
      if(ret != OTA_EX_OK){
        debug_ota("ota_control>  Validate Error\r\n");
        break;
      }
      ret = ota_process( data, data_len );
      if(ret != OTA_EX_OK){
        debug_ota("ota_control>  Process Error\r\n");
        break;
      }
    }while(false);

    return ret;
}




