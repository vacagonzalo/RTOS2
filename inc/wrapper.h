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
#include "timers.h"

/*=====[Definitions of public data types]====================================*/

typedef enum
{
    ISR_IDLE,
    ISR_ACQUIRING
} ISR_states_t;

typedef struct
{
    uint32_t index;
    uint8_t length;
    uint8_t *ptr;
} queueRecievedFrame_t;

typedef struct
{
    ISR_states_t state;
    queueRecievedFrame_t data;
    TimerHandle_t timeOut;
} ISR_FSM_t;

typedef struct
{
    uartMap_t uart;
    uint32_t baud;
    uint32_t index;
    QueueHandle_t queueISRC3;
    QueueHandle_t queueC3C2;
    SemaphoreHandle_t semphrC2ISR;
    QMPool poolMem; //memory pool (contienen la informacion que necesita la biblioteca qmpool.h)
    ISR_FSM_t fsm;
} config_t;

typedef enum
{
    ERROR_INVALID_DATA,
    ERROR_INVALID_OPCODE,
    ERROR_SYSTEM,
    NO_ERROR
} errorType_t;

/*=====[Prototypes (declarations) of public functions]=======================*/

void queue_init(config_t *config);
void ISR_init(config_t *config);
void C2_init(config_t *config);
void C3_init(config_t *config);
void initWrapper(config_t *config);

#endif /* __WRAPPER_H__ */