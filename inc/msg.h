/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __MSG_H__
#define __MSG_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "appConfig.h"
/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

    /*=====[Definition macros of public constants]===============================*/
#define RECIEVED_FRAME_QUEUE_SIZE 6
#define PROCESS_FRAME_QUEUE_SIZE 6

    /*=====[Public function-like macros]=========================================*/

    /*=====[Definitions of public data types]====================================*/

    typedef struct
    {
        uint32_t index;
        uint8_t length;
        uint8_t *ptr;
    } queueRecievedFrame_t;

    typedef struct
    {
        QueueHandle_t queueISRC2;
        QueueHandle_t queueC2C3;
        QueueHandle_t queueC3C2;
        SemaphoreHandle_t semphrC2ISR;
    } msg_t;

    typedef enum
    {
        ERROR_INVALID_DATA,
        ERROR_INVALID_OPCODE,
        ERROR_SYSTEM,
        NO_ERROR
    } errorType_t;

    /*=====[Prototypes (declarations) of public functions]=======================*/

    void queue_init(void);

    /*=====[Prototypes (declarations) of public interrupt functions]=============*/

    /*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __MSG_H__ */