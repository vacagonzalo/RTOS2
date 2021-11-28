/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

#include "C2.h"
#include "msg.h"
#include "appConfig.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "qmpool.h"
#include "semphr.h"
#include "crc8.h"

extern msg_t msg[UARTS_TO_USE];
extern QMPool Pool_memoria;
extern uint8_t *pDataToSend;
extern t_UART_config uart_configs[];

extern void uartUsbSendCallback(void *param);
void int2ascii(uint8_t *p, uint8_t crc);
void C2_task_in(void *param);
void C2_task_out(void *param);

void C2_init(void)
{
    BaseType_t res;

    for (uint32_t i = 0; i < UARTS_TO_USE; ++i)
    {

        // Create a task in freeRTOS with dynamic memory
        res = xTaskCreate(
            C2_task_in,                   // Function that implements the task.
            (const char *)"C2_task_in",   // Text name for the task.
            configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
            (void *)i,                    // Parameter passed into the task.
            tskIDLE_PRIORITY + 3,         // Priority at which the task is created.
            0                             // Pointer to the task created in the system
        );
        configASSERT(res == pdPASS);

        // Create a task in freeRTOS with dynamic memory
        res = xTaskCreate(
            C2_task_out,                  // Function that implements the task.
            (const char *)"C2_task_out",  // Text name for the task.
            configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
            (void *)i,                    // Parameter passed into the task.
            tskIDLE_PRIORITY + 4,         // Priority at which the task is created.
            0                             // Pointer to the task created in the system
        );
        configASSERT(res == pdPASS);
    }
}

void C2_task_in(void *param)
{
    uint32_t index = (uint32_t)param;
    queueRecievedFrame_t datosC2C3;

    while (TRUE)
    {
        // Pedido de memoria al Pool
        datosC2C3.ptr = QMPool_get(&Pool_memoria, 0); //pido un bloque del pool
        configASSERT(datosC2C3.ptr != NULL);          //<-- Gestion de errores

        xQueueReceive(msg[index].queueISRC2, datosC2C3.ptr, portMAX_DELAY); // Esperamos el frame
        datosC2C3.length = (uint8_t)datosC2C3.ptr[FRAME_MAX_LENGTH];
        xQueueSend(msg[index].queueC2C3, &datosC2C3, portMAX_DELAY); // Manda a C3 a procesar
    }
}

void C2_task_out(void *param)
{
    uint32_t index = (uint32_t)param;
    queueRecievedFrame_t datosC3C2;
    uint8_t crc_eof[FRAME_CRCEOF_LENGTH];
    crc_eof[2] = ')';
    crc_eof[3] = '\0'; // TODO Borrar.
    while (TRUE)
    {
        xQueueReceive(msg[index].queueC3C2, &datosC3C2, portMAX_DELAY); // Esperamos el DATO
        // calculo de CRC a enviar
        uint8_t crcCalc = crc8_calc(crc8_init(), datosC3C2.ptr + OFFSET_SOF, datosC3C2.length - OFFSET_SOF);
        int2ascii(crc_eof, crcCalc);
        // CRC y EOF
        for (uint8_t i = 0; i < FRAME_CRCEOF_LENGTH; i++)
        {
            datosC3C2.ptr[datosC3C2.length + i] = crc_eof[i];
        }

        pDataToSend = datosC3C2.ptr;
        uartCallbackSet(uart_configs[index].uartName, UART_TRANSMITER_FREE, uartUsbSendCallback, (void *)index);
        while (pDataToSend < (datosC3C2.ptr + datosC3C2.length + DISCART_FRAME))
        {
            uartSetPendingInterrupt(uart_configs[index].uartName);
            // Espera semaforo para terminar de enviar el mensaje por ISR
            if (xSemaphoreTake(msg[index].semphrC2ISR, 0) == pdTRUE)
            {
                pDataToSend++;
            }
        }
        uartCallbackClr(uart_configs[index].uartName, UART_TRANSMITER_FREE);

        // Libero el bloque de memoria que ya fue trasmitido
        QMPool_put(&Pool_memoria, datosC3C2.ptr);
        datosC3C2.ptr = NULL;
    }
}

void int2ascii(uint8_t *p, uint8_t crc)
{
    uint8_t msn, lsn;

    msn = crc / 16;
    lsn = crc - msn * 16;

    if (msn < 10)
    {
        p[0] = msn + '0';
    }
    else
    {
        p[0] = msn - 10 + 'A';
    }

    if (lsn < 10)
    {
        p[1] = lsn + '0';
    }
    else
    {
        p[1] = lsn - 10 + 'A';
    }
}
