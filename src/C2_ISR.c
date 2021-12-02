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
#include <stdlib.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "crc8.h"
#include "wrapper.h"
#include "appConfig.h"

#define RECIEVED_CHAR_QUEUE_SIZE 10
#define FRAME_START (c == '(')
#define END_FRAME (c == ')')
#define VALID_CHAR (c == ' ' || c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
#define MAX_AMOUNT_OF_UARTS 3
#define FRAME_MINIMUN_VALID_LENGTH 9
#define VALID_ID_CHAR (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9')
#define ID_LOCATION 5
#define VALID_CRC_CHAR1 ((config->fsm.pktRecieved[config->fsm.countChars - 2] >= 'A' && config->fsm.pktRecieved[config->fsm.countChars - 2] <= 'F') || (config->fsm.pktRecieved[config->fsm.countChars - 2] >= '0' && config->fsm.pktRecieved[config->fsm.countChars - 2] <= '9'))
#define VALID_CRC_CHAR2 ((config->fsm.pktRecieved[config->fsm.countChars - 1] >= 'A' && config->fsm.pktRecieved[config->fsm.countChars - 1] <= 'F') || (config->fsm.pktRecieved[config->fsm.countChars - 1] >= '0' && config->fsm.pktRecieved[config->fsm.countChars - 1] <= '9'))

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

uint8_t *pDataToSend;

void ISR_init(config_t *config)
{
	uartConfig(config->uart, config->baud);
	uartCallbackSet(config->uart, UART_RECEIVE, onRx, (void *)config);
	uartInterrupt(config->uart, true);

	config->fsm.timeOut = xTimerCreate("timeOut", TIMEOUT_PERIOD_TICKS, pdFALSE, (void *)config, onTime);
	configASSERT(config->fsm.timeOut);
	config->fsm.state = ISR_IDLE;
}

void onRx(void *param)
{
	config_t *config = (config_t *)param;				  // Casteo del index
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE; // Comenzamos definiendo la variable

	uint8_t c = uartRxRead(config->uart); // Selecciona la UART
	// Reseteamos el timer

	switch (config->fsm.state)
	{
	case ISR_IDLE:
		if (FRAME_START)
		{
			config->fsm.state = ISR_ACQUIRING;
			config->fsm.countChars = 0;
			config->fsm.pktRecieved[config->fsm.countChars] = c;
			config->fsm.countChars++;
			xTimerResetFromISR(config->fsm.timeOut, &xHigherPriorityTaskWoken);
		}
		break;
	case ISR_ACQUIRING:
		if (VALID_CHAR)
		{
			config->fsm.pktRecieved[config->fsm.countChars] = c;
			config->fsm.countChars++;
			if ((config->fsm.countChars == FRAME_MAX_LENGTH) ||
				(!(VALID_ID_CHAR) && config->fsm.countChars < ID_LOCATION))
			{
				config->fsm.state = ISR_IDLE;
			}
			xTimerResetFromISR(config->fsm.timeOut, &xHigherPriorityTaskWoken);
		}
		else if (FRAME_START)
		{
			config->fsm.countChars = 1;
			xTimerResetFromISR(config->fsm.timeOut, &xHigherPriorityTaskWoken);
		}
		else if (END_FRAME)
		{
			config->fsm.state = ISR_IDLE;
			if ((config->fsm.countChars > FRAME_MINIMUN_VALID_LENGTH - 1) &&
				(VALID_CRC_CHAR1) && (VALID_CRC_CHAR2))
			{
				// CRC Check
				uint8_t crcCalc = crc8_calc(crc8_init(), config->fsm.pktRecieved + OFFSET_SOF, config->fsm.countChars - OFFSET_CRC - OFFSET_SOF);
				uint8_t crcRecieved = ascii2hex(config->fsm.pktRecieved + config->fsm.countChars - OFFSET_CRC);
				if (crcCalc == crcRecieved)
				{
					// Mandar la cola de mensajes
					config->fsm.pktRecieved[config->fsm.countChars] = c;
					config->fsm.countChars++;
					config->fsm.pktRecieved[FRAME_MAX_LENGTH] = config->fsm.countChars;
					xQueueSendFromISR(config->queueISRC2, config->fsm.pktRecieved, &xHigherPriorityTaskWoken);
				}
			}
		}
		else
		{
			config->fsm.state = ISR_IDLE;
			config->fsm.pktRecieved[config->fsm.countChars] = '_';
			config->fsm.pktRecieved[FRAME_MAX_LENGTH] = config->fsm.countChars + 4;
			xQueueSendFromISR(config->queueISRC2, config->fsm.pktRecieved, &xHigherPriorityTaskWoken);
		}
		break;
	default:
		config->fsm.state = ISR_IDLE;
		break;
	}
}

// Envio a la PC desde la UART hasta NULL y deshabilito Callback
void uartUsbSendCallback(void *param)
{
	config_t *config = (config_t *)param;
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static uint32_t lastCharSended;

	if (lastCharSended != (uint32_t)pDataToSend)
	{
		while (uartTxReady(config->uart) == FALSE)
			;
		uartTxWrite(config->uart, (uint8_t)*pDataToSend);
		lastCharSended = (uint32_t)pDataToSend;
	}
	uartClearPendingInterrupt(config->uart);
	xSemaphoreGiveFromISR(config->semphrC2ISR, &xHigherPriorityTaskWoken);
}

void onTime(TimerHandle_t xTimer)
{
	config_t *config = (config_t *)pvTimerGetTimerID(xTimer);
	config->fsm.state = ISR_IDLE;
}

uint8_t ascii2hex(uint8_t *p)
{
	uint8_t result = 0;
	if (p[0] >= 'A' && p[0] <= 'F')
		result = (10 + p[0] - 'A') * 16;
	else
		result = (p[0] - '0') * 16;

	if (p[1] >= 'A' && p[1] <= 'F')
		result += 10 + p[1] - 'A';
	else
		result += p[1] - '0';

	return result;
}
