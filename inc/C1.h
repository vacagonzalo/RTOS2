/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __C1_H__
#define __C1_H__

/*=====[Inclusions of public function dependencies]==========================*/

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C"
{
#endif
#include "appConfig.h"
#include "qmpool.h"
#include <inttypes.h>
/*=====[Definition macros of public constants]===============================*/
#define RECIEVED_CHAR_QUEUE_SIZE 10

    /*=====[Public function-like macros]=========================================*/

    /*=====[Definitions of public data types]====================================*/

    typedef enum
    {
        C1_IDLE,
        C1_ACQUIRING
    } C1_states_t;

    typedef struct
    {
        C1_states_t state;
        uint8_t uart_index;
        uint8_t countChars;
        uint8_t pktRecieved[FRAME_MAX_LENGTH + 1];
        QueueHandle_t queueRecievedChar;
    } C1_FSM_t;

    typedef struct
    {
        t_UART_config config;
        C1_FSM_t fsm;

        QMPool Pool_memoria;
        char * Pool_puntero;
    } C1_t;

    /*=====[Prototypes (declarations) of public functions]=======================*/

    void C1_init(t_UART_config *config);

    /*=====[Prototypes (declarations) of public interrupt functions]=============*/

    /*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __C1_H__ */
