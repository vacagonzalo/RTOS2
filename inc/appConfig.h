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
#define FRAME_MAX_LENGTH 209
#define DEFAULT_BAUD_RATE 115200
#define UARTS_TO_USE 1

    /*=====[Public function-like macros]=========================================*/

    /*=====[Definitions of public data types]====================================*/

    /*=====[Prototypes (declarations) of public functions]=======================*/

    /*=====[Prototypes (declarations) of public interrupt functions]=============*/

    /*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __APPCONFIG_H__ */