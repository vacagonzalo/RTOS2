/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __APPCONFIG_H__
#define __APPCONFIG_H__

#include "sapi.h"

/*=====[Inclusions of public function dependencies]==========================*/

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C"
{
#endif
#include <inttypes.h>

    /*=====[Definition macros of public constants]===============================*/

#define FRAME_MAX_LENGTH 175
#define DEFAULT_BAUD_RATE 115200
#define UARTS_TO_USE 1
#define PACKET_SIZE FRAME_MAX_LENGTH + 1         // Tamanio del paquete
#define POOL_TOTAL_BLOCKS 28                     // Cuantos paquetes
#define POOL_SIZE POOL_TOTAL_BLOCKS *PACKET_SIZE //Tamanio total del pool
#define TIMEOUT_PERIOD 4
#define TIMEOUT_PERIOD_TICKS pdMS_TO_TICKS(TIMEOUT_PERIOD)

#define OFFSET_CRC 2
#define OFFSET_ID 5
#define CMD_BYTE 5
#define FIRST_DATA_BYTE 6
#define START_DATA 7
#define DISCART_FRAME 3
#define FRAME_ID_LENGTH 4
#define FRAME_CRCEOF_LENGTH 3
#define FRAME_CDATA_DISCART_LENGTH 8
#define OFFSET_SOF 1
#define COM_DATA_ERROR 3
#define MAX_NUMBER_OF_WORDS 15
#define MAX_NUMBER_OF_LETTERS 10

#define PROCESS_FRAME_QUEUE_SIZE (POOL_TOTAL_BLOCKS-3)/2
#define TRANSMIT_FRAME_QUEUE_SIZE (POOL_TOTAL_BLOCKS-3)/2

    /*=====[Definitions of public data types]====================================*/

    typedef struct
    {
        uartMap_t uartName;
        uint32_t baudRate;
    } t_UART_config;

    /*=====[Prototypes (declarations) of public functions]=======================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __APPCONFIG_H__ */