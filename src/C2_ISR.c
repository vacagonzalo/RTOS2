/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

#include "C2_ISR.h"
#include "msg.h"
#include "appConfig.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "crc8.h"

#define RECIEVED_CHAR_QUEUE_SIZE 10
#define FRAME_START (c == '(')
#define END_FRAME (c == ')')
#define VALID_CHAR (c == ' ' || c == '_' || (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A) || (c >= 0x30 && c <= 0x39))
#define MAX_AMOUNT_OF_UARTS 3
#define FRAME_MINIMUN_VALID_LENGTH 9
#define VALID_ID_CHAR (c >= 0x41 && c <= 0x46) || (c >= 0x30 && c <= 0x39)
#define ID_LOCATION 5
#define VALID_CRC_CHAR1 ((ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 2] >= 0x41 && ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 2] <= 0x46) || (ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 2] >= 0x30 && ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 2] <= 0x39))
#define VALID_CRC_CHAR2 ((ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 1] >= 0x41 && ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 1] <= 0x46) || (ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 1] >= 0x30 && ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars - 1] <= 0x39))

typedef enum
{
	ISR_IDLE,
	ISR_ACQUIRING
} ISR_states_t;

typedef struct
{
	ISR_states_t state;
	uint8_t countChars;
	uint8_t pktRecieved[FRAME_MAX_LENGTH + 1];
	TimerHandle_t timeOut;
} ISR_FSM_t;

const t_UART_config uart_configs[] = {
	{.uartName = UART_USB, .baudRate = DEFAULT_BAUD_RATE}};

ISR_FSM_t ISR_FSM[UARTS_TO_USE];

void onRx(void *param);
void uartUsbSendCallback(void *param);
void onTime(TimerHandle_t xTimer);
uint8_t ascii2hex(uint8_t *p);

/*
   	   UART_GPIO = 0, // Hardware UART0 via GPIO1(TX), GPIO2(RX) pins on header P0
	   UART_485  = 1, // Hardware UART0 via RS_485 A, B and GND Borns
	   UART_USB  = 3, // Hardware UART2 via USB DEBUG port
	   UART_ENET = 4, // Hardware UART2 via ENET_RXD0(TX), ENET_CRS_DV(RX) pins on header P0
	   UART_232  = 5, // Hardware UART3 via 232_RX and 232_tx pins on header P1
*/

extern msg_t msg[UARTS_TO_USE];

uint8_t *pDataToSend;

void ISR_init(void)
{
	for (uint32_t i = 0; i < UARTS_TO_USE; ++i)
	{
		if (i > MAX_AMOUNT_OF_UARTS)
			break;
		uartConfig(uart_configs[i].uartName, uart_configs[i].baudRate);
		uartCallbackSet(uart_configs[i].uartName, UART_RECEIVE, onRx, (void *)i);
		uartInterrupt(uart_configs[i].uartName, true);

		ISR_FSM[i].timeOut = xTimerCreate("timeOut", TIMEOUT_PERIOD_TICKS, pdFALSE, (void *)i, onTime);
		configASSERT(ISR_FSM[i].timeOut);
		ISR_FSM[i].state = ISR_IDLE;
		ISR_FSM[i].countChars = 0;
	}
}

void onRx(void *param)
{
	uint32_t index = (uint32_t)param;					  // Casteo del index
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE; // Comenzamos definiendo la variable

	uint8_t c = uartRxRead(uart_configs[index].uartName); // Selecciona la UART
	// Reseteamos el timer

	switch (ISR_FSM[index].state)
	{
	case ISR_IDLE:
		if (FRAME_START)
		{
			ISR_FSM[index].state = ISR_ACQUIRING;
			ISR_FSM[index].countChars = 0;
			ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars] = c;
			ISR_FSM[index].countChars++;
			xTimerResetFromISR(ISR_FSM[index].timeOut, &xHigherPriorityTaskWoken);
		}
		break;
	case ISR_ACQUIRING:
		if (VALID_CHAR)
		{
			ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars] = c;
			ISR_FSM[index].countChars++;
			if ((ISR_FSM[index].countChars == FRAME_MAX_LENGTH) ||
				(!(VALID_ID_CHAR) && ISR_FSM[index].countChars < ID_LOCATION))
			{
				ISR_FSM[index].state = ISR_IDLE;
			}
			xTimerResetFromISR(ISR_FSM[index].timeOut, &xHigherPriorityTaskWoken);
		}
		else if (FRAME_START)
		{
			ISR_FSM[index].countChars = 1;
			xTimerResetFromISR(ISR_FSM[index].timeOut, &xHigherPriorityTaskWoken);
		}
		else if (END_FRAME)
		{
			ISR_FSM[index].state = ISR_IDLE;
			if ((ISR_FSM[index].countChars > FRAME_MINIMUN_VALID_LENGTH - 1) &&
				(VALID_CRC_CHAR1) && (VALID_CRC_CHAR2))
			{
				// CRC Check
				uint8_t crcCalc = crc8_calc(crc8_init(), ISR_FSM[index].pktRecieved + OFFSET_SOF, ISR_FSM[index].countChars - OFFSET_CRC - OFFSET_SOF);
				uint8_t crcRecieved = ascii2hex(ISR_FSM[index].pktRecieved + ISR_FSM[index].countChars - OFFSET_CRC);
				if (crcCalc == crcRecieved)
				{
					// Mandar la cola de mensajes
					ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars] = c;
					ISR_FSM[index].countChars++;
					ISR_FSM[index].pktRecieved[FRAME_MAX_LENGTH] = ISR_FSM[index].countChars;
					xQueueSendFromISR(msg[index].queueISRC2, ISR_FSM[index].pktRecieved, &xHigherPriorityTaskWoken);
				}
			}
		}
		else
		{
			ISR_FSM[index].state = ISR_IDLE;
			ISR_FSM[index].pktRecieved[ISR_FSM[index].countChars] = '_';
			ISR_FSM[index].pktRecieved[FRAME_MAX_LENGTH] = ISR_FSM[index].countChars + 4;
			xQueueSendFromISR(msg[index].queueISRC2, ISR_FSM[index].pktRecieved, &xHigherPriorityTaskWoken);
		}
		break;
	default:
		ISR_FSM[index].state = ISR_IDLE;
		break;
	}
}

// Envio a la PC desde la UART hasta NULL y deshabilito Callback
void uartUsbSendCallback(void *param)
{
	uint32_t index = (uint32_t)param;
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	while (uartTxReady(uart_configs[index].uartName) == FALSE)
		;
	uartTxWrite(uart_configs[index].uartName, (uint8_t)*pDataToSend);
	uartClearPendingInterrupt(uart_configs[index].uartName);
	xSemaphoreGiveFromISR(msg[index].semphrC2ISR, &xHigherPriorityTaskWoken);
}

void onTime(TimerHandle_t xTimer)
{
	uint32_t index = (uint32_t)pvTimerGetTimerID(xTimer);
	ISR_FSM[index].state = ISR_IDLE;
}

uint8_t ascii2hex(uint8_t *p)
{
	uint8_t result = 0;
	if (p[0] >= 0x41 && p[0] <= 0x46)
	{
		result = (10 + p[0] - 'A') * 16;
	}
	else
	{
		result = (p[0] - '0') * 16;
	}

	if (p[1] >= 0x41 && p[1] <= 0x46)
	{
		result += 10 + p[1] - 'A';
	}
	else
	{
		result += p[1] - '0';
	}
	return result;
}
