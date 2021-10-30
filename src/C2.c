#include "C2.h"
#include "C1.h"
#include <stdint.h>
#include <stddef.h>

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

QueueHandle_t queueC1C2, queueC3C2;

void C2_task_in(void *param);
void C2_task_out(void *param);

void C2_init(void)
{
    BaseType_t res;

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        C2_task_in,                   // Function that implements the task.
        (const char *)"C2_task_in",   // Text name for the task.
        configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
        0,                            // Parameter passed into the task.
        tskIDLE_PRIORITY + 1,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        C2_task_out,                  // Function that implements the task.
        (const char *)"C2_task_out",  // Text name for the task.
        configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
        0,                            // Parameter passed into the task.
        tskIDLE_PRIORITY + 1,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);

    // Crear cola para impresion de puntaje
    queueC1C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, FRAME_MAX_LENGTH*sizeof(uint8_t));
    configASSERT(queueC1C2 != NULL);
    queueC3C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(uint8_t));
    configASSERT(queueC3C2 != NULL);
}

void C2_task_in(void *param)
{
    uint8_t datos[FRAME_MAX_LENGTH];
    while (TRUE)
    {
        xQueueReceive(queueC1C2, &datos, portMAX_DELAY); // Esperamos el caracter
        
        printf("Recibimos <<%s>> por UART\r\n", datos);
        
        // Tengo un paquete abierto

            //Si

                // Me llega un ( --> Reinicio el contador
                // Me llega un ) --> mando por la cola C2C3

            //No
                // Me llega un ( --> Pido 209 Bytes
        
        
        
        // Me llega un (
            // flag captura de paquete inciada -> Reset del indice
            // sino flag captura de paquete inciada -> Malloc de 209 bytes.
        // Me llega un )y tengo la flag de captura -> mando por la cola C2C3

        
        
        
        //printf("Recibimos <<%c>> por UART\r\n", datos);
        //Si llega -> validar
        //
    }
}

void C2_task_out(void *param)
{
    uint8_t datos;
    while (TRUE)
    {
        xQueueReceive(queueC3C2, &datos, portMAX_DELAY); // Esperamos el caracter
        //Eseperar respuesta de C3
        // Empaquetar con (, ), ID y CRC
        //Mandarselo a la UART
        //
    }
}
