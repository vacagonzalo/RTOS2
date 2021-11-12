#include "msg.h"
#include "appConfig.h"
msg_t msg[UARTS_TO_USE];
void queue_init(void)
{
    for (uint32_t i = 0; i < UARTS_TO_USE; ++i)
    {
        // Crear cola para compartir los frames entre capas
        msg[i].queueC1C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, (PACKET_SIZE) * sizeof(uint8_t));
        configASSERT(msg[i].queueC1C2 != NULL);
        msg[i].queueC2C3 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
        configASSERT(msg[i].queueC2C3 != NULL);
        msg[i].queueC3C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
        configASSERT(msg[i].queueC3C2 != NULL);
        // Creo semaforo para enviar el msj por ISR
        msg[i].semphrC2ISR = xSemaphoreCreateBinary();
        configASSERT(msg[i].semphrC2ISR != NULL);
    }
}
