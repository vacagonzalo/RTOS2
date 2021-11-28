#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "appConfig.h"
#include "qmpool.h"

typedef struct
{
    uartMap_t uart;
    uint32_t baud;
    uint32_t index;
    QueueHandle_t queueISRC2;
    QueueHandle_t queueC2C3;
    QueueHandle_t queueC3C2;
    SemaphoreHandle_t semphrC2ISR;
    QMPool Pool_memoria; //memory pool (contienen la informacion que necesita la biblioteca qmpool.h)
} config_t;

void initWrapper(config_t *config);

#endif /* __WRAPPER_H__ */