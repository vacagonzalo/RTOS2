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

typedef enum
{
    NONE,
    SPACE,
    MAYUS,
    UNDER
} flagType_t;

extern msg_t msg[UARTS_TO_USE];

errorType_t digestor(queueRecievedFrame_t dato);

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
    errorType_t errorType = NO_ERROR;

    while (TRUE)
    {
        xQueueReceive(msg[index].queueC2C3, &datosC2C3, portMAX_DELAY); // Esperamos el caracter
        // taskENTER_CRITICAL();
        // printf("C2In to C3: CD=");
        // for (uint8_t i = OFFSET_ID; i < datosC2C3.length - DISCART_FRAME; i++)
        // {
        //     printf("%c", datosC2C3.ptr[i]);
        // }
        // printf("\r\n");
        // taskEXIT_CRITICAL();

        // //Dummy Process
        // for (uint8_t i = OFFSET_ID; i < datosC2C3.length - DISCART_FRAME; i++)
        // {
        //     datosC2C3.ptr[i]++;
        // }
        datosC3C2.ptr = datosC2C3.ptr;
        errorType = digestor(datosC2C3);
        /*

        ERROR_INVALID_DATA,
        ERROR_INVALID_OPCODE,
        ERROR_SYSTEM,
        NO_ERROR*/
        /* (SSSSEnnCC) */
        switch (errorType)
        {
            case NO_ERROR:
            {
                // Envio a C2 via queueC3C2
                datosC3C2.length = datosC2C3.length - DISCART_FRAME;                
                xQueueSend(msg[index].queueC3C2, &datosC3C2, portMAX_DELAY);
                break;
            }
            case ERROR_INVALID_DATA:
            {
                datosC3C2.length = (OFFSET_ID + COM_DATA_ERROR);
                memcpy(datosC3C2.ptr + OFFSET_ID, "E00", COM_DATA_ERROR);
                xQueueSend(msg[index].queueC3C2, &datosC3C2, portMAX_DELAY);
                break;
            }
            case ERROR_INVALID_OPCODE:
            {
                datosC3C2.length = (OFFSET_ID + COM_DATA_ERROR);
                memcpy(datosC3C2.ptr + OFFSET_ID, "E01", COM_DATA_ERROR);
                xQueueSend(msg[index].queueC3C2, &datosC3C2, portMAX_DELAY);
                break;
            }
            case ERROR_SYSTEM:
            {
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

errorType_t digestor(queueRecievedFrame_t dato)
{
    /* Chequear data finaliza con '_' o con ' ' */
    if (dato.ptr[dato.length - DISCART_FRAME - 1] == ' ' || dato.ptr[dato.length - DISCART_FRAME - 1] == '_')
    {
        return ERROR_INVALID_DATA;
    }

    flagType_t flagType = NONE;
    uint8_t words = 1;

    for (uint32_t i = OFFSET_ID + 1; i < dato.length - DISCART_FRAME; ++i)
    {
        switch (flagType)
        {
        case NONE:
            if (dato.ptr[i] == ' ')
            {
                flagType = SPACE;
            }
            else if (dato.ptr[i] == '_')
            {
                flagType = UNDER;
            }
            else if (dato.ptr[i] >= 'A' && dato.ptr[i] <= 'Z')
            {
                flagType = MAYUS;
            }
            break;
        case SPACE:
            if ((dato.ptr[i] == '_') || (dato.ptr[i] >= 'A' && dato.ptr[i] <= 'Z'))
            {
                return ERROR_INVALID_DATA;
            }
            else if (dato.ptr[i] == ' ')
            {
                words++;
            }
            break;
        case UNDER:
            if ((dato.ptr[i] == ' ') || (dato.ptr[i] >= 'A' && dato.ptr[i] <= 'Z'))
            {
                return ERROR_INVALID_DATA;
            }
            else if (dato.ptr[i] == '_')
            {
                words++;
            }
            break;
        case MAYUS:
            if ((dato.ptr[i] == '_') || (dato.ptr[i] == ' '))
            {
                return ERROR_INVALID_DATA;
            }
            else if (dato.ptr[i] >= 'A' && dato.ptr[i] <= 'Z')
            {
                words++;
            }
            break;
        default:
            break;
        }

        if (dato.ptr[i] == ' ')
        {
            if (dato.ptr[i - 1] == ' ')
            {
                return ERROR_INVALID_DATA; // doble espacio
            }
        }
        else if (dato.ptr[i] == '_')
        {
            if (dato.ptr[i - 1] == '_')
            {
                return ERROR_INVALID_DATA; // doble guion bajo
            }
        }

        // Chequeo de cantidad de palabras en el frame
        if (words > 15)
        {
            return ERROR_INVALID_DATA;
        }
    }
    /* Chequear comando invalido */
    if (dato.ptr[OFFSET_ID] != 'S' && dato.ptr[OFFSET_ID] != 'C' && dato.ptr[OFFSET_ID] != 'P')
    {
        return ERROR_INVALID_OPCODE;
    }

    return NO_ERROR;
}