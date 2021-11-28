#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#include "sapi.h"

typedef struct
{
    uartMap_t uart;
    uint32_t baud;
    uint32_t index;
} config_t;

void initWrapper(config_t *config);

#endif /* __WRAPPER_H__ */