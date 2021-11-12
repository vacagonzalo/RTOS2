/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

#include "C3.h"
#include "msg.h"
#include "appConfig.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#define OFFSET_ID 5
#define DISCART_FRAME 3

extern msg_t msg[UARTS_TO_USE];

void C3_task(void *param);

void C3_init(void)
{
    BaseType_t res;

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        C3_task,                      // Function that implements the task.
        (const char *)"C3_task",      // Text name for the task.
        configMINIMAL_STACK_SIZE * 4, // Stack size in words, not bytes.
        0,                            // Parameter passed into the task.
        tskIDLE_PRIORITY + 1,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);
}

void C3_task(void *param)
{
    uint32_t index = (uint32_t)param;
    queueRecievedFrame_t datosC2C3, datosC3C2;

    while (TRUE)
    {
        xQueueReceive(msg[index].queueC2C3, &datosC2C3, portMAX_DELAY); // Esperamos el caracter
        taskENTER_CRITICAL();
        printf("C2In to C3: CD=");
        for (uint8_t i = OFFSET_ID; i < datosC2C3.length - DISCART_FRAME; i++)
        {
            printf("%c", datosC2C3.ptr[i]);
        }
        printf("\r\n");
        //printf(" UART=%d\r\n", datosC2C3.index);
        taskEXIT_CRITICAL();

        // Dummy Process
        for (uint8_t i = OFFSET_ID; i < datosC2C3.length - DISCART_FRAME; i++)
        {
            datosC2C3.ptr[i]++;
        }

        // envio a C2 via queueC3C2
        //datosC3C2.index = datosC2C3.index;
        datosC3C2.length = datosC2C3.length-DISCART_FRAME;
        datosC3C2.ptr = datosC2C3.ptr;
        xQueueSend(msg[index].queueC3C2, &datosC3C2, portMAX_DELAY);
    }
}
