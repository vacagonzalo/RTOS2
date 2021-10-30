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

typedef enum
{
	C1_IDLE,
	C1_TRANSITION,
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

C1_FSM_t C1_FSM;

const t_UART_config uart_configs[] = {
	{.uartName = UART_USB, .baudRate = DEFAULT_BAUD_RATE}};

void onRx(void *noUsado);

extern QueueHandle_t queueC1C2;
QueueHandle_t queueRecievedChar;

void C1_init(uint8_t count)
{
	for (uint8_t i = 0; i < count; ++i)
	{
		uartConfig(uart_configs[i].uartName, uart_configs[i].baudRate);
		uartCallbackSet(uart_configs[i].uartName, UART_RECEIVE, onRx, NULL);
		uartInterrupt(UART_USB, true);
	}

	// Crear cola para recepcion de caracteres
	queueRecievedChar = xQueueCreate(RECIEVED_CHAR_QUEUE_SIZE, sizeof(uint8_t));
	configASSERT(queueRecievedChar != NULL);

	C1_FSM.state = C1_IDLE;
	C1_FSM.countChars = 0;
	C1_FSM.uart_index = 0;

	BaseType_t res;
	// Create a task in freeRTOS with dynamic memory
	res = xTaskCreate(
		C1_task,					  // Function that implements the task.
		(const char *)"C1_task",	  // Text name for the task.
		configMINIMAL_STACK_SIZE * 4, // Stack size in words, not bytes.
		0,							  // Parameter passed into the task.
		tskIDLE_PRIORITY + 1,		  // Priority at which the task is created.
		0							  // Pointer to the task created in the system
	);
	configASSERT(res == pdPASS);
}

void onRx(void *noUsado)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;				 //Comenzamos definiendo la variable
	uint8_t c = uartRxRead(UART_USB);									 // <= está harcodeado la uart
	xQueueSendFromISR(queueRecievedChar, &c, &xHigherPriorityTaskWoken); // Manda el char a ala queue
}

void C1_task(void *param)
{
	uint8_t c;
	queueRecievedFrame_t msg;

	while (TRUE)
	{
		xQueueReceive(queueRecievedChar, &c, portMAX_DELAY); // Esperamos el caracter
		switch (C1_FSM.state)
		{
		case C1_IDLE:
			if (FRAME_START)
			{
				C1_FSM.state = C1_TRANSITION;
				C1_FSM.countChars = 0;
			}
			break;
		case C1_TRANSITION:
			if (VALID_CHAR)
			{
				C1_FSM.state = C1_ACQUIRING;
				C1_FSM.pktRecieved[C1_FSM.countChars] = c;
				C1_FSM.countChars++;
			}
			else if (FRAME_START)
			{
				C1_FSM.state = C1_TRANSITION;
				C1_FSM.countChars = 0;
			}
			else
			{
				C1_FSM.state = C1_IDLE;
			}
			break;
		case C1_ACQUIRING:
			if (VALID_CHAR)
			{
				C1_FSM.state = C1_ACQUIRING;
				C1_FSM.pktRecieved[C1_FSM.countChars] = c;
				C1_FSM.countChars++;
				if (C1_FSM.countChars == FRAME_MAX_LENGTH)
				{
					C1_FSM.state = C1_IDLE;
				}
			}
			else if (FRAME_START)
			{
				C1_FSM.state = C1_TRANSITION;
				C1_FSM.countChars = 0;
			}
			else if (END_FRAME)
			{
				// MAndar la cola de mensajes
				msg.length = C1_FSM.countChars;
				msg.ptr = pvPortMalloc(msg.length * sizeof(uint8_t));
				configASSERT(msg.ptr != NULL);
				memcpy(msg.ptr, C1_FSM.pktRecieved, msg.length);
				xQueueSend(queueC1C2, &msg, portMAX_DELAY);
				C1_FSM.state = C1_IDLE;
			}
			else
			{
				C1_FSM.state = C1_IDLE;
			}
			break;
		default:
			break;
		}
	}
}
