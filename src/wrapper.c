#include "wrapper.h"
#include "C2.h"
#include "C2_ISR.h"
#include "C3.h"
#include "msg.h"

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
    QMPool_init(&(config->Pool_memoria), Pool_puntero, POOL_SIZE * sizeof(uint8_t), PACKET_SIZE); //Tamanio del segmento de memoria reservado
    
}
