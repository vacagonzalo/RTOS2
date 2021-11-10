#include "msg.h"

msg_t msg;
void queue_init(void)
{
    // Crear cola para compartir los frames entre capas
    msg.queueC1C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(msg.queueC1C2 != NULL);
    msg.queueC2C3 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(msg.queueC2C3 != NULL);
    msg.queueC3C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(msg.queueC3C2 != NULL);
}
