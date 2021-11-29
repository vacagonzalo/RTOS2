#include "wrapper.h"
#include "appConfig.h"

void initWrapper(config_t *config)
{
    C2_init(config);
    ISR_init(config);
    C3_init(config);
    queue_init(config);
    //	Reservo memoria para el memory pool
    void *Pool_puntero = pvPortMalloc(POOL_SIZE * sizeof(char));
    configASSERT(Pool_puntero != NULL);
    //	Creo el pool de memoria que va a usarse para la transmision
    QMPool_init(&(config->poolMem), Pool_puntero, POOL_SIZE * sizeof(uint8_t), PACKET_SIZE); //Tamanio del segmento de memoria reservado
}

void queue_init(config_t *config)
{
    // Crear cola para compartir los frames entre capas
    config->queueISRC2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, (PACKET_SIZE) * sizeof(uint8_t));
    configASSERT(config->queueISRC2 != NULL);
    config->queueC2C3 = xQueueCreate(PROCESS_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(config->queueC2C3 != NULL);
    config->queueC3C2 = xQueueCreate(TRANSMIT_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(config->queueC3C2 != NULL);
    // Creo semaforo para enviar el msj por ISR
    config->semphrC2ISR = xSemaphoreCreateBinary();
    configASSERT(config->semphrC2ISR != NULL);
}
