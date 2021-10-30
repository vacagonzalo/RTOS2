/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __C2_H__
#define __C2_H__

/*=====[Inclusions of public function dependencies]==========================*/

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C"
{
#endif
    //#include <inttypes.h>
    /*=====[Definition macros of public constants]===============================*/
    #define RECIEVED_FRAME_QUEUE_SIZE 5
    /*=====[Public function-like macros]=========================================*/

    /*=====[Definitions of public data types]====================================*/
    /*=====[Prototypes (declarations) of public functions]=======================*/

    void C2_init(void);

    /*=====[Prototypes (declarations) of public interrupt functions]=============*/

    /*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __C2_H__ */
