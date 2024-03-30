#ifndef OTA_INC_OTA_HEADERS_H_
#define OTA_INC_OTA_HEADERS_H_

#include "ota_global.h"

#include <stdint.h>


#define   OTA_SOF  0xAA    // Start of Frame
#define   OTA_EOF  0xBB    // End of Frame
#define   OTA_ACK  0x00    // ACK
#define   OTA_NACK 0x01    // NACK


#define   OTA_DATA_MAX_SIZE ( 512 )  //Maximum data Size
#define   OTA_DATA_OVERHEAD (    9 )  //data overhead
#define   OTA_PACKET_MAX_SIZE (   OTA_DATA_MAX_SIZE +   OTA_DATA_OVERHEAD )

/*
 * Exception codes
 */
typedef enum
{
    OTA_EX_OK       = 0,    // Success
    OTA_EX_ERR      = 1,    // Failure
}  OTA_EX_;

/*
 * OTA process state
 */
typedef enum
{
    OTA_STATE_IDLE    = 0,
    OTA_STATE_START   = 1,
    OTA_STATE_HEADER  = 2,
    OTA_STATE_DATA    = 3,
    OTA_STATE_END     = 4,
}  OTA_STATE_;

/*
 * Packet type
 */
typedef enum
{
    OTA_PACKET_TYPE_CMD       = 0,    // Command
    OTA_PACKET_TYPE_DATA      = 1,    // Data
    OTA_PACKET_TYPE_HEADER    = 2,    // Header
    OTA_PACKET_TYPE_RESPONSE  = 3,    // Response
}  OTA_PACKET_TYPE_;

/*
 * OTA Commands
 */
typedef enum
{
    OTA_CMD_START = 0,    // OTA Start command
    OTA_CMD_END   = 1,    // OTA End command
    OTA_CMD_ABORT = 2,    // OTA Abort command
}  OTA_CMD_;

/*
 * OTA meta info
 */
typedef struct
{
  uint32_t package_size;
  uint32_t package_crc;
  uint32_t reserved1;
  uint32_t reserved2;
  uint32_t filename[30];
}__attribute__((packed)) meta_info;

/*
 * OTA Command format
 *
 * ________________________________________
 * |     | Packet |     |     |     |     |
 * | SOF | Type   | Len | CMD | CRC | EOF |
 * |_____|________|_____|_____|_____|_____|
 *   1B      1B     2B    1B     4B    1B
 */
typedef struct
{
  uint8_t   sof;
  uint8_t   packet_type;
  uint16_t  data_len;
  uint8_t   cmd;
  uint32_t  crc;
  uint8_t   eof;
}__attribute__((packed))   OTA_COMMAND_;

/*
 * OTA Header format
 *
 * __________________________________________
 * |     | Packet |     | Header |     |     |
 * | SOF | Type   | Len |  Data  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B     16B     4B    1B
 */
typedef struct
{
  uint8_t     sof;
  uint8_t     packet_type;
  uint16_t    data_len;
  meta_info   meta_data;
  uint32_t    crc;
  uint8_t     eof;
}__attribute__((packed))   OTA_HEADER_;

/*
 * OTA Data format
 *
 * __________________________________________
 * |     | Packet |     |        |     |     |
 * | SOF | Type   | Len |  Data  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B    nBytes   4B    1B
 */
typedef struct
{
  uint8_t     sof;
  uint8_t     packet_type;
  uint16_t    data_len;
  uint8_t     *data;
}__attribute__((packed))   OTA_DATA_;

/*
 * OTA Response format
 *
 * __________________________________________
 * |     | Packet |     |        |     |     |
 * | SOF | Type   | Len | Status | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B      1B     4B    1B
 */
typedef struct
{
  uint8_t   sof;
  uint8_t   packet_type;
  uint16_t  data_len;
  uint8_t   status;
  uint32_t  crc;
  uint8_t   eof;
}__attribute__((packed))   OTA_RESP_;



typedef struct{
	OTA_STATE_ ota_state;
	uint16_t packet_number; //
	uint32_t total_packets; //
	uint32_t total_size; //bytes
	uint32_t received_size;
	uint32_t crc16;
	char filename[30];
}ota_t;

typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;


// Estado del proceso OTA
extern ota_t ota_d;

void func(void);

#endif /* OTA_INC_OTA_HEADERS_H_ */
