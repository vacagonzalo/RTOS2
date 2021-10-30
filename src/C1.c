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

typedef struct
{
	uartMap_t uartName;
	uint32_t baudRate;
} t_UART_config;

const t_UART_config uart_configs[] = {
	{.uartName = UART_USB, .baudRate = DEFAULT_BAUD_RATE}};

void onRx(void *noUsado);

extern QueueHandle_t queueC1C2;

void C1_init(uint8_t count)
{
	for (uint8_t i = 0; i < count; ++i)
	{
		uartConfig(uart_configs[i].uartName, uart_configs[i].baudRate);
		uartCallbackSet(uart_configs[i].uartName, UART_RECEIVE, onRx, NULL);
		uartInterrupt(UART_USB, true);
	}
}

void onRx(void *noUsado)
{
	uint8_t c = uartRxRead(UART_USB);
	xQueueSend(queueC1C2, &c, portMAX_DELAY);
}
