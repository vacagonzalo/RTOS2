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
#include <inttypes.h>
/*=====[Definition macros of public constants]===============================*/
#define FRAME_MAX_LENGTH 209
#define RECIEVED_CHAR_QUEUE_SIZE 10

    /*=====[Public function-like macros]=========================================*/

    /*=====[Definitions of public data types]====================================*/

    typedef struct
    {
        uint32_t index;
        uint8_t length;
        uint8_t *ptr;
    } queueRecievedFrame_t;

    /*=====[Prototypes (declarations) of public functions]=======================*/

    void C1_init(uint8_t count);
    void C1_task(void *param);

    /*=====[Prototypes (declarations) of public interrupt functions]=============*/

    /*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __C1_H__ */
