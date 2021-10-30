#include "C1.h"
#include <stdint.h>
#include <stddef.h>

#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

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
	char c = uartRxRead(UART_USB);
	printf("Recibimos <<%c>> por UART\r\n", c);
}
