/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

#include "C1.h"
#include "msg.h"
#include "appConfig.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#define RECIEVED_CHAR_QUEUE_SIZE 10
#define FRAME_START (c == '(')
#define END_FRAME (c == ')')
#define VALID_CHAR (c == ' ' || c == '_' || (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A) || (c >= 0x30 && c <= 0x39))
#define MAX_AMOUNT_OF_UARTS 3
#define FRAME_MINIMUN_VALID_LENGTH 9
#define VALID_ID_CHAR (c >= 0x41 && c <= 0x46) || (c >= 0x30 && c <= 0x39)
#define ID_LOCATION 5
#define VALID_CRC_CHAR1 ((C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 2] >= 0x41 && C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 2] <= 0x46) || (C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 2] >= 0x30 && C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 2] <= 0x39))
#define VALID_CRC_CHAR2 ((C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 1] >= 0x41 && C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 1] <= 0x46) || (C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 1] >= 0x30 && C1_FSM[index].pktRecieved[C1_FSM[index].countChars - 1] <= 0x39))

typedef enum
{
	C1_IDLE,
	C1_ACQUIRING
} C1_states_t;

typedef struct
{
	C1_states_t state;
	uint8_t uart_index;
	uint8_t countChars;
	uint8_t pktRecieved[FRAME_MAX_LENGTH];
	QueueHandle_t queueRecievedChar;
} C1_FSM_t;

typedef struct
{
	uartMap_t uartName;
	uint32_t baudRate;
} t_UART_config;

const t_UART_config uart_configs[] = {
	{.uartName = UART_USB, .baudRate = DEFAULT_BAUD_RATE}};

C1_FSM_t C1_FSM[UARTS_TO_USE];

void onRx(void *param);
void C1_task(void *param);

/*
   	   UART_GPIO = 0, // Hardware UART0 via GPIO1(TX), GPIO2(RX) pins on header P0
	   UART_485  = 1, // Hardware UART0 via RS_485 A, B and GND Borns
	   UART_USB  = 3, // Hardware UART2 via USB DEBUG port
	   UART_ENET = 4, // Hardware UART2 via ENET_RXD0(TX), ENET_CRS_DV(RX) pins on header P0
	   UART_232  = 5, // Hardware UART3 via 232_RX and 232_tx pins on header P1
*/

extern msg_t msg[UARTS_TO_USE];

void C1_init(void)
{
	for (uint32_t i = 0; i < UARTS_TO_USE; ++i)
	{
		if (i > MAX_AMOUNT_OF_UARTS)
			break;
		uartConfig(uart_configs[i].uartName, uart_configs[i].baudRate);
		uartCallbackSet(uart_configs[i].uartName, UART_RECEIVE, onRx, (void *)i);
		uartInterrupt(uart_configs[i].uartName, true);
		C1_FSM[i].state = C1_IDLE;
		C1_FSM[i].countChars = 0;
		C1_FSM[i].uart_index = i;
		BaseType_t res;
		// Create a task in freeRTOS with dynamic memory
		res = xTaskCreate(
			C1_task,					  // Function that implements the task.
			(const char *)"C1_task",	  // Text name for the task.
			configMINIMAL_STACK_SIZE * 4, // Stack size in words, not bytes.
			(void *)i,					  // Parameter passed into the task.
			tskIDLE_PRIORITY + 1,		  // Priority at which the task is created.
			0							  // Pointer to the task created in the system
		);
		configASSERT(res == pdPASS);
		// Crear cola para recepcion de caracteres
		C1_FSM[i].queueRecievedChar = xQueueCreate(RECIEVED_CHAR_QUEUE_SIZE, sizeof(uint8_t));
		configASSERT(C1_FSM[i].queueRecievedChar != NULL);
	}
}

void onRx(void *param)
{
	uint32_t index = (uint32_t)param;									 // Casteo del index
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;				 // Comenzamos definiendo la variable
	uint8_t c = uartRxRead(uart_configs[index].uartName);				 // Selecciona la UART
	xQueueSendFromISR(C1_FSM[index].queueRecievedChar, &c, &xHigherPriorityTaskWoken); // Manda el char a a la queue
}

void C1_task(void *param)
{
	uint32_t index = (uint32_t)param; // Casteo del index
	uint8_t c;
	queueRecievedFrame_t msgSend;

	while (TRUE)
	{
		xQueueReceive(C1_FSM[index].queueRecievedChar, &c, portMAX_DELAY); // Esperamos el caracter
		switch (C1_FSM[index].state)
		{
		case C1_IDLE:
			if (FRAME_START)
			{
				C1_FSM[index].state = C1_ACQUIRING;
				C1_FSM[index].countChars = 0;
				C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
				C1_FSM[index].countChars++;
			}
			break;
		case C1_ACQUIRING:
			if (VALID_CHAR)
			{
				C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
				C1_FSM[index].countChars++;
				if ((C1_FSM[index].countChars == FRAME_MAX_LENGTH) ||
					(!(VALID_ID_CHAR) && C1_FSM[index].countChars < ID_LOCATION))
				{
					C1_FSM[index].state = C1_IDLE;
				}
			}
			else if (FRAME_START)
			{
				C1_FSM[index].countChars = 1;
			}
			else if (END_FRAME)
			{
				if ((C1_FSM[index].countChars > FRAME_MINIMUN_VALID_LENGTH - 1) &&
					(VALID_CRC_CHAR1) && (VALID_CRC_CHAR2))
				{ // Mandar la cola de mensajes
					msgSend.index = index;
					C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
					C1_FSM[index].countChars++;
					msgSend.length = C1_FSM[index].countChars;
					msgSend.ptr = pvPortMalloc(FRAME_MAX_LENGTH * sizeof(uint8_t));
					if (msgSend.ptr != NULL)
					{
						memcpy(msgSend.ptr, C1_FSM[index].pktRecieved, msgSend.length);
						xQueueSend(msg[index].queueC1C2, &msgSend, portMAX_DELAY);
					}
				}
				C1_FSM[index].state = C1_IDLE;
			}
			else
			{
				C1_FSM[index].state = C1_IDLE;
			}
			break;
		default:
			C1_FSM[index].state = C1_IDLE;
			break;
		}
	}
}
