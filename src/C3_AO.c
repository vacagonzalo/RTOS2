/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "wrapper.h"
#include "appConfig.h"
#include "AO.h"

typedef enum
{
    NONE,
    SPACE,
    MAYUS,
    UNDER
} flagType_t;

QueueHandle_t response_queue;

errorType_t digestor(queueRecievedFrame_t *dato);
void snake_packet(activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar);
void camel_packet(activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar);
void pascal_packet(activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar);
void wrong_cmd (activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar);
void C2ToOA_task(void *param);
void OAToC2_task(void *param);

bool_t C3_init(config_t *config)
{

    BaseType_t res;

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        C2ToOA_task,                  // Function that implements the task.
        (const char *)"C2ToOA_task",  // Text name for the task.
        configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
        (void *)config,               // Parameter passed into the task.
        tskIDLE_PRIORITY + 3,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);

    // Create a task in freeRTOS with dynamic memory
    res = xTaskCreate(
        OAToC2_task,                  // Function that implements the task.
        (const char *)"OAToC2_task",  // Text name for the task.
        configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
        (void *)config,               // Parameter passed into the task.
        tskIDLE_PRIORITY + 3,         // Priority at which the task is created.
        0                             // Pointer to the task created in the system
    );
    configASSERT(res == pdPASS);

    response_queue = xQueueCreate(N_QUEUE_AO, sizeof(queueRecievedFrame_t));
    configASSERT(response_queue != NULL);
}

void C2ToOA_task(void *param)
{
    config_t *config = (config_t *)param;

    queueRecievedFrame_t dato;

    char evento;

    activeObject_t snakeCase;
    snakeCase.itIsAlive = FALSE;

    activeObject_t camelCase;
    camelCase.itIsAlive = FALSE;

    activeObject_t pascalCase;
    pascalCase.itIsAlive = FALSE;

    activeObject_t wrongCmd;
    wrongCmd.itIsAlive = FALSE;

    while (TRUE)
    {

        xQueueReceive(config->queueISRC3, &dato, portMAX_DELAY); //espero a que venga un bloque por la cola

        if (dato.ptr != NULL) //si recibo null es porque ocurrio un error en la comunicacion
        {
            
            if (dato.ptr[CMD_BYTE] == 'S') /* EVENT deberia ser S, C o P */
            {
                if (snakeCase.itIsAlive == FALSE)
                {
                    // Se crea el objeto activo, con el comando correspondiente y tarea asociada.
                    activeObjectOperationCreate(&snakeCase, (callBackActObj_t) snake_packet, activeObjectTask, response_queue);
                }

                // Y enviamos el dato a la cola para procesar.
                activeObjectEnqueue(&snakeCase, &dato);
            }
            else if (dato.ptr[CMD_BYTE] == 'C')
            {
                if (camelCase.itIsAlive == FALSE)
                {
                    // Se crea el objeto activo, con el comando correspondiente y tarea asociada.
                    activeObjectOperationCreate(&camelCase, (callBackActObj_t) camel_packet, activeObjectTask, response_queue);
                }

                // Y enviamos el dato a la cola para procesar.
                activeObjectEnqueue(&camelCase, &dato);
            }
            else if (dato.ptr[CMD_BYTE] == 'P') /* EVENT deberia ser S, C o P */
            {
                if (pascalCase.itIsAlive == FALSE)
                {
                    // Se crea el objeto activo, con el comando correspondiente y tarea asociada.
                    activeObjectOperationCreate(&pascalCase, (callBackActObj_t) pascal_packet, activeObjectTask, response_queue);
                }

                // Y enviamos el dato a la cola para procesar.
                activeObjectEnqueue(&pascalCase, &dato);
            }
            else
            {
            	if (wrongCmd.itIsAlive == FALSE)
				{
					// Se crea el objeto activo, con el comando correspondiente y tarea asociada.
					activeObjectOperationCreate(&wrongCmd, (callBackActObj_t) wrong_cmd, activeObjectTask, response_queue);
				}

				// Y enviamos el dato a la cola para procesar.
				activeObjectEnqueue(&wrongCmd, &dato);
            }
        }
    }
}

void OAToC2_task(void *param)
{
    config_t *config = (config_t *)param;

    queueRecievedFrame_t * datosC3C2;

    while (TRUE)
    {
        if (xQueueReceive(response_queue, &datosC3C2, portMAX_DELAY))
        {
            // Envio a C2 via queueC3C2
            xQueueSend(config->queueC3C2, datosC3C2, portMAX_DELAY);
        }
    }
}

void wrong_cmd (activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar)
{
    queueRecievedFrame_t *msgProcess = (queueRecievedFrame_t *)mensaje_a_procesar;

        msgProcess->length = (OFFSET_ID + COM_DATA_ERROR + FRAME_CRCEOF_LENGTH);
        memcpy(msgProcess->ptr + OFFSET_ID, "E01", COM_DATA_ERROR);

    xQueueSend(response_queue, &msgProcess, 0);
}

void snake_packet(activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar)
{
    queueRecievedFrame_t *msgProcess = (queueRecievedFrame_t *)mensaje_a_procesar;
    errorType_t errorType = NO_ERROR;
    uint32_t i = FIRST_DATA_BYTE;

    errorType = digestor(msgProcess);

    switch (errorType)
    {
    case NO_ERROR:
    {
        // Pasar a Snake es lo mismo desde pascal o camel!

        while (i < msgProcess->length - DISCART_FRAME)
        {
            if (msgProcess->ptr[i] >= 'A' && msgProcess->ptr[i] <= 'Z')
            {
                msgProcess->ptr[i] += 32;
                if (i != FIRST_DATA_BYTE)
                {
                    msgProcess->length++;
                    memmove(msgProcess->ptr + i + 1, msgProcess->ptr + i, msgProcess->length - i);
                    msgProcess->ptr[i] = '_';
                    i++;
                }
            }
            i++;
        }
        break;
    }
    case ERROR_INVALID_DATA:
    {
        msgProcess->length = (OFFSET_ID + COM_DATA_ERROR + FRAME_CRCEOF_LENGTH);
        memcpy(msgProcess->ptr + OFFSET_ID, "E00", COM_DATA_ERROR);
        break;
    }
    default:
        break;
    }

    xQueueSend(response_queue, &msgProcess, 0);
}

void camel_packet(activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar)
{
    queueRecievedFrame_t *msgProcess = (queueRecievedFrame_t *)mensaje_a_procesar;
    errorType_t errorType = NO_ERROR;
    uint32_t i = FIRST_DATA_BYTE;

    errorType = digestor(msgProcess);

    switch (errorType)
    {
    case NO_ERROR:
    {
        uint32_t i = FIRST_DATA_BYTE;
        while (i < msgProcess->length - DISCART_FRAME)
        {
            if (msgProcess->ptr[i] == '_')
            {
                msgProcess->ptr[i + 1] -= 32;
                memmove(msgProcess->ptr + i, msgProcess->ptr + i + 1, msgProcess->length - i);
                msgProcess->length--;
            }
            else if (msgProcess->ptr[i] >= 'A' && msgProcess->ptr[i] <= 'Z')
            {
                if (i == FIRST_DATA_BYTE)
                {
                    msgProcess->ptr[i] += 32;
                }
                i = msgProcess->length;
            }
            i++;
        }
        break;
    }
    case ERROR_INVALID_DATA:
    {
    	msgProcess->length = (OFFSET_ID + COM_DATA_ERROR + FRAME_CRCEOF_LENGTH);
        memcpy(msgProcess->ptr + OFFSET_ID, "E00", COM_DATA_ERROR);
        break;
    }
    default:
        break;
    }
    xQueueSend(response_queue, &msgProcess, 0);
}

void pascal_packet(activeObject_t *caller_ao, queueRecievedFrame_t *mensaje_a_procesar)
{
    queueRecievedFrame_t *msgProcess = (queueRecievedFrame_t *)mensaje_a_procesar;
    errorType_t errorType = NO_ERROR;
    uint32_t i = FIRST_DATA_BYTE;

    errorType = digestor(msgProcess);

    switch (errorType)
    {
    case NO_ERROR:
        while (i < msgProcess->length - DISCART_FRAME)
        {
            if (msgProcess->ptr[i] == '_')
            {
                msgProcess->ptr[i + 1] -= 32;
                memmove(msgProcess->ptr + i, msgProcess->ptr + i + 1, msgProcess->length - i);
                msgProcess->length--;
            }
            else if (msgProcess->ptr[i] >= 'A' && msgProcess->ptr[i] <= 'Z')
            {
                i = msgProcess->length;
            }
            else
            {
                if (i == FIRST_DATA_BYTE)
                {
                    msgProcess->ptr[i] -= 32;
                }
            }
            i++;
        }
        break;
    case ERROR_INVALID_DATA:
    {
    	msgProcess->length = (OFFSET_ID + COM_DATA_ERROR + FRAME_CRCEOF_LENGTH);
    	memcpy(msgProcess->ptr + OFFSET_ID, "E00", COM_DATA_ERROR);
        break;
    }
    default:
        break;
    }
    xQueueSend(response_queue, &msgProcess, 0);
}

errorType_t digestor(queueRecievedFrame_t *dato)
{
    /* Chequear data finaliza con '_' o con ' ' */
    if (dato->ptr[dato->length - DISCART_FRAME - 1] == ' ' || dato->ptr[dato->length - DISCART_FRAME - 1] == '_')
    {
        return ERROR_INVALID_DATA;
    }

    /* Chequear comando invalido */
    if (dato->ptr[CMD_BYTE] != 'S' && dato->ptr[CMD_BYTE] != 'C' && dato->ptr[CMD_BYTE] != 'P')
    {
        return ERROR_INVALID_OPCODE;
    }

    // Cheque de dato invalido en el primer caracter
    if ((dato->ptr[FIRST_DATA_BYTE] == '_') || (dato->ptr[FIRST_DATA_BYTE] == ' '))
    {
        return ERROR_INVALID_DATA;
    }

    flagType_t flagType = NONE;
    uint8_t words = 1;
    uint32_t contCharsMax = 0;

    for (uint32_t i = FIRST_DATA_BYTE; i < dato->length - DISCART_FRAME; ++i)
    {
        contCharsMax++;
        switch (flagType)
        {
        case NONE:
            if (dato->ptr[i] == ' ')
            {
                flagType = SPACE;
                dato->ptr[i] = '_'; // Convierte separacion con espacio en snake
                contCharsMax = 0;
            }
            else if (dato->ptr[i] == '_')
            {
                flagType = UNDER;
                contCharsMax = 0;
            }
            else if (dato->ptr[i] >= 'A' && dato->ptr[i] <= 'Z')
            {
                flagType = MAYUS;
                contCharsMax = 1;
            }
            break;
        case SPACE:
            /* Chequeo doble espacio o letra mayuscula */
            if ((dato->ptr[i] == '_') || (dato->ptr[i] >= 'A' && dato->ptr[i] <= 'Z'))
            {
                return ERROR_INVALID_DATA;
            }
            else if (dato->ptr[i] == ' ')
            {
                words++;
                dato->ptr[i] = '_'; // Convierte separacion con espacio en snake
                contCharsMax = 0;
            }
            break;
        case UNDER:
            if ((dato->ptr[i] == ' ') || (dato->ptr[i] >= 'A' && dato->ptr[i] <= 'Z'))
            {
                return ERROR_INVALID_DATA;
            }
            else if (dato->ptr[i] == '_')
            {
                words++;
                contCharsMax = 0;
            }
            break;
        case MAYUS:
            if ((dato->ptr[i] == '_') || (dato->ptr[i] == ' '))
            {
                return ERROR_INVALID_DATA;
            }
            else if (dato->ptr[i] >= 'A' && dato->ptr[i] <= 'Z')
            {
                words++;
                contCharsMax = 0;
            }
            break;
        default:
            break;
        }

        if (dato->ptr[i] == ' ')
        {
            if (dato->ptr[i - 1] == ' ')
            {
                return ERROR_INVALID_DATA; // doble espacio
            }
        }
        else if (dato->ptr[i] == '_')
        {
            if (dato->ptr[i - 1] == '_')
            {
                return ERROR_INVALID_DATA; // doble guion bajo
            }
        }

        // Chequeo de cantidad de palabras en el frame
        if (words > MAX_NUMBER_OF_WORDS)
        {
            return ERROR_INVALID_DATA;
        }

        // Chequeo de cantidad de letras en el frame
        if (contCharsMax > MAX_NUMBER_OF_LETTERS)
        {
            return ERROR_INVALID_DATA;
        }
    }
    return NO_ERROR;
}
