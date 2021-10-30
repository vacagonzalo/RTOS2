#include "C1.h"

void C1_init( void )
{
	/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
	uartConfig(UART_USB, 115200);
	// Seteo un callback al evento de recepcion y habilito su interrupcion
	uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
	// Habilito todas las interrupciones de UART_USB
	uartInterrupt(UART_USB, true);
}

void onRx( void *noUsado )
{
   char c = uartRxRead( UART_USB );
   printf( "Recibimos <<%c>> por UART\r\n", c );
}
