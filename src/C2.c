/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

#include "C2.h"
#include "C1.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#define FRAME_ID_LENGTH 4
#define FRAME_CDATA_DISCART_LENGTH 8

QueueHandle_t queueC1C2, queueC2C3, queueC2InOut, queueC3C2;

void C2_task_in(void *param);
void C2_task_out(void *param);

void C2_init(void)
{
    BaseType_t res;

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        C2_task_in,                   // Function that implements the task.
        (const char *)"C2_task_in",   // Text name for the task.
        configMINIMAL_STACK_SIZE * 4, // Stack size in words, not bytes.
        0,                            // Parameter passed into the task.
        tskIDLE_PRIORITY + 1,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        C2_task_out,                  // Function that implements the task.
        (const char *)"C2_task_out",  // Text name for the task.
        configMINIMAL_STACK_SIZE * 4, // Stack size in words, not bytes.
        0,                            // Parameter passed into the task.
        tskIDLE_PRIORITY + 1,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);

    // Crear cola para impresion de puntaje
    queueC1C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(queueC1C2 != NULL);
    queueC2C3 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(queueC2C3 != NULL);
    queueC2InOut = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(queueC2InOut != NULL);
    queueC3C2 = xQueueCreate(RECIEVED_FRAME_QUEUE_SIZE, sizeof(queueRecievedFrame_t));
    configASSERT(queueC3C2 != NULL);
}

void C2_task_in(void *param)
{
    queueRecievedFrame_t datosC1C2, datosC2C3, datosC2InOut;

    datosC2InOut.length = FRAME_ID_LENGTH;
    datosC2InOut.ptr = pvPortMalloc(FRAME_ID_LENGTH * sizeof(uint8_t));
    configASSERT(datosC2InOut.ptr != NULL);

    while (TRUE)
    {
        xQueueReceive(queueC1C2, &datosC1C2, portMAX_DELAY); // Esperamos el caracter
        taskENTER_CRITICAL();
        printf("C1 to C2: ");
        for (uint8_t i = 0; i < datosC1C2.length; i++)
        {
            printf("%c", datosC1C2.ptr[i]);
        }
        printf(" UART=%d\r\n", datosC1C2.index);
        taskEXIT_CRITICAL();

        // Parseo de ID y envio a C2_task_out via queueC2InOut
        datosC2InOut.index = datosC1C2.index;
        memcpy(datosC2InOut.ptr, datosC1C2.ptr + 1, datosC2InOut.length);
        xQueueSend(queueC2InOut, &datosC2InOut, portMAX_DELAY);

        // Parseo de C+Data y envio a C3 via queueC2C3
        datosC2C3.index = datosC1C2.index;
        datosC2C3.length = datosC1C2.length;
        datosC2C3.ptr = datosC1C2.ptr; 
        xQueueSend(queueC2C3, &datosC2C3, portMAX_DELAY);
    }
}

void C2_task_out(void *param)
{
    queueRecievedFrame_t datosID, datosC3C2;
    while (TRUE)
    {
        xQueueReceive(queueC2InOut, &datosID, portMAX_DELAY); // Esperamos el ID
        taskENTER_CRITICAL();
        printf("C2In to C2Out: ID=");
        for (uint8_t i = 0; i < datosID.length; i++)
        {
            printf("%c", datosID.ptr[i]);
        }
        printf(" UART=%d\r\n", datosID.index);
        taskEXIT_CRITICAL();

        xQueueReceive(queueC3C2, &datosC3C2, portMAX_DELAY); // Esperamos el DATO

        taskENTER_CRITICAL();
        printf("C3 to C2Out: CD=");
        for (uint8_t i = 0; i < datosC3C2.length; i++)
        {
            printf("%c", datosC3C2.ptr[i]);
        }
        printf(" UART=%d\r\n", datosC3C2.index);
        taskEXIT_CRITICAL();

        // Libera memoria
        vPortFree(datosC3C2.ptr);
    }
}
