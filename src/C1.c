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

#define DEFAULT_BAUD_RATE 115200
#define UART_COUNT sizeof(uart_configs) / sizeof(t_UART_config)
#define FRAME_START (c == '(')
#define END_FRAME (c == ')')
#define VALID_CHAR (c == ' ' || c == '_' || (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A) || (c >= 0x30 && c <= 0x39))
#define MAX_AMOUNT_OF_UARTS 3
#define FRAME_MINIMUN_VALID_LENGTH 10

typedef enum
{
	C1_IDLE,
	C1_ACQUIRING
} C1_states_t;

typedef struct
{
	uartMap_t uartName;
	uint32_t baudRate;
} t_UART_config;

typedef struct
{
	C1_states_t state;
	uint8_t uart_index;
	uint8_t countChars;
	uint8_t pktRecieved[FRAME_MAX_LENGTH];
} C1_FSM_t;

C1_FSM_t C1_FSM[MAX_AMOUNT_OF_UARTS];

const t_UART_config uart_configs[] = {
	{.uartName = UART_USB, .baudRate = DEFAULT_BAUD_RATE},
	{.uartName = UART_232, .baudRate = DEFAULT_BAUD_RATE},
	{.uartName = UART_485, .baudRate = DEFAULT_BAUD_RATE}};

void onRx(void *param);

/*
   	   UART_GPIO = 0, // Hardware UART0 via GPIO1(TX), GPIO2(RX) pins on header P0
	   UART_485  = 1, // Hardware UART0 via RS_485 A, B and GND Borns
	   UART_USB  = 3, // Hardware UART2 via USB DEBUG port
	   UART_ENET = 4, // Hardware UART2 via ENET_RXD0(TX), ENET_CRS_DV(RX) pins on header P0
	   UART_232  = 5, // Hardware UART3 via 232_RX and 232_tx pins on header P1
*/

extern QueueHandle_t queueC1C2;
QueueHandle_t queueRecievedChar;

void C1_init(uint8_t count)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		if (i > MAX_AMOUNT_OF_UARTS)
			break;
		uartConfig(uart_configs[i].uartName, uart_configs[i].baudRate);
		uartCallbackSet(uart_configs[i].uartName, UART_RECEIVE, onRx, (void *)i);
		uartInterrupt(uart_configs[i].uartName, true);
		C1_FSM[i].state = C1_IDLE;
		C1_FSM[i].countChars = 0;
		C1_FSM[i].uart_index = 0;
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
	}

	// Crear cola para recepcion de caracteres
	queueRecievedChar = xQueueCreate(RECIEVED_CHAR_QUEUE_SIZE, sizeof(uint8_t));
	configASSERT(queueRecievedChar != NULL);
}

void onRx(void *param)
{
	uint32_t index = (uint32_t) param;									 // Casteo del index
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;				 // Comenzamos definiendo la variable
	uint8_t c = uartRxRead(uart_configs[index].uartName);				 // Selecciona la UART
	xQueueSendFromISR(queueRecievedChar, &c, &xHigherPriorityTaskWoken); // Manda el char a a la queue
}

void C1_task(void *param)
{
	uint32_t index = (uint32_t) param; // Casteo del index
	uint8_t c;
	queueRecievedFrame_t msg;

	while (TRUE)
	{
		xQueueReceive(queueRecievedChar, &c, portMAX_DELAY); // Esperamos el caracter
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
				C1_FSM[index].state = C1_ACQUIRING;
				C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
				C1_FSM[index].countChars++;
				if (C1_FSM[index].countChars == FRAME_MAX_LENGTH)
				{
					C1_FSM[index].state = C1_IDLE;
				}
			}
			else if (FRAME_START)
			{
				C1_FSM[index].state = C1_ACQUIRING;
				C1_FSM[index].countChars = 1;
			}
			else if (END_FRAME)
			{
				if (C1_FSM[index].countChars > FRAME_MINIMUN_VALID_LENGTH-1)
				{	// Mandar la cola de mensajes
					msg.index = index;
					C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
					C1_FSM[index].countChars++;
					msg.length = C1_FSM[index].countChars;
					msg.ptr = pvPortMalloc(msg.length * sizeof(uint8_t));
					//configASSERT(msg.ptr != NULL);
					if (msg.ptr != NULL)
					{
						memcpy(msg.ptr, C1_FSM[index].pktRecieved, msg.length);
						xQueueSend(queueC1C2, &msg, portMAX_DELAY);
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
