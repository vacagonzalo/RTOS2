#include "C1.h"
#include <stdint.h>
#include <stddef.h>

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
	C1_ACQUIRING,
	C1_SENDDING
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

	// Crear cola para impresion de puntaje
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
		configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
		0,							  // Parameter passed into the task.
		tskIDLE_PRIORITY + 1,		  // Priority at which the task is created.
		0							  // Pointer to the task created in the system
	);
	configASSERT(res == pdPASS);
}

void onRx(void *noUsado)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable
	uint8_t c = uartRxRead(UART_USB);					  // <= está harcodeado la uart
	/*
	uint8_t c;
	while(reading) {
		while(c = uartRxRead(UART_USB)) {
			append al string
			ver el tiempo que pasó
			ver si ya leí una trama válida
			break?
		}
		cambio el valor de reading
	}
	*/
	xQueueSendFromISR(queueRecievedChar, &c, &xHigherPriorityTaskWoken);
}

void C1_task(void *param)
{
	uint8_t c;
	C1_FSM.countChars = 0;

	while (TRUE)
	{
		xQueueReceive(queueRecievedChar, &c, portMAX_DELAY); // Esperamos el caracter
		switch (C1_FSM.state)
		{
		case C1_IDLE:
			if (FRAME_START)
			{
				C1_FSM.state = C1_TRANSITION;
				C1_FSM.countChars = 1;
				C1_FSM.pktRecieved[0] = c; 
			}
			break;
		case C1_TRANSITION:
			if (VALID_CHAR)
			{
				C1_FSM.state = C1_ACQUIRING;
				C1_FSM.pktRecieved[C1_FSM.countChars] = c;
				C1_FSM.countChars++;
				// Que hacer con el caracter
			}
			else
			{
				C1_FSM.state = C1_IDLE;
				C1_FSM.countChars = 0;
			}
			break;
		case C1_ACQUIRING:
			if (VALID_CHAR)
			{
				C1_FSM.state = C1_ACQUIRING;
				C1_FSM.pktRecieved[C1_FSM.countChars] = c;
				C1_FSM.countChars++;
				if (C1_FSM.countChars == FRAME_MAX_LENGTH) //
				{
					C1_FSM.state = C1_IDLE;
					C1_FSM.countChars = 0;
				}
				// Que hacer con el caracter
			}
			else if (END_FRAME)
			{
				C1_FSM.state = C1_SENDDING;
				C1_FSM.pktRecieved[C1_FSM.countChars] = c;
				C1_FSM.countChars++;
			}
			else
			{
				C1_FSM.state = C1_IDLE;
				C1_FSM.countChars = 0;
			}
			break;

		case C1_SENDDING:

			C1_FSM.state = C1_IDLE;
			C1_FSM.countChars = 0;
			// MAndar la cola de mensajes

			//char * msg = pvPortMalloc( FRAME_MAX_LENGTH * sizeof( char ) );
			//configASSERT( msg != NULL );

			xQueueSend(queueC1C2, &C1_FSM.pktRecieved, portMAX_DELAY);

			break;
			
		default:
			break;
		}
	}
}