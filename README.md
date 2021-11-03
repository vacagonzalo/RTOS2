# RTOS2

## Generales

R_TP_1:
> Cada capa de abstracción deberá estar diseñada de forma tal que se pueda instanciar (y
eventualmente se pueda reutilizar varias instancias dentro de un mismo Firmware, por ej, para
utilizarla con varias USARTs). Eso significa que cada capa deberá ser una instancia de clase
(implementada en C). Los métodos de cada clase, deberán tener uniformidad de nombres (ej: c2_init,
c3_process_buffer, etc)

* Se genera un manejo de interrupciones que recibe datos por medio de las UART disponibles que posee la EDUCIAA.
* Existe una tarea C1_task por cada una de las interfaces de UART de la placa EDUCIAA.
* La tarea C1_task se encarga de parsear y validar cada dato recibido.
* Cada paquete de datos es enviado desde el servicio de interrupción a la tarea C2_task_in.
* Las tareas C1 escriben a través una cola (queueC1C2) en común para pasarle los datos a C2.

R_TP_2
> Las diferentes capas no deben conocer detalles privados de las otras capas. Cada capa debe
encargarse de su responsabilidad. Ej. C3 NO debe conocer que el paquete inicial tenía CRC o ID, va a
recibir de C2 un buffer cuyo 1er bytes es el campo C y el resto serán datos y va a devolver a C2 la
respuesta en el mismo formato.

* C1 se comunica con C2 solo por medio de la cola (queueC1C2).

R_TP_3
> Todas las justificaciones deberán estar plasmadas en un archivo readme.md (markdown). Deberá
estar versionado con el código fuente.

Se utilizó *vscode* con una conexión *liveshare* para trabajar en conjunto.
Además, se creó un [repositorio de Github](https://github.com/vacagonzalo/RTOS2) donde todos los integrantes realizaron *commits*.

R_TP_4
> El grupo deberá justificar la elección de las arquitecturas que utilice (gestión de datos, tareas, técnicas
aplicadas).

* Se define tener un handler de interrupciones con la menor cantidad de código posible.
* Se genera una cola de caracteres recibidos por cada una de las UART que va a una C1_task que hace la validación de lo recibido.
* Una vez recibido un frame valido se lo pone en una estructura msg junto con el largo *length* del paquete recibido y el identificador de la UART que produjo el paquete *index*. 

## Capa separación de frames (C2)

R_C2_1
> El grupo deberá justificar la elección del esquema de memoria dinámica utilizada.

* Se utiliza una estrategia de pedido de memoria dinámica mediante pvPortMalloc en la tarea C1_task con el tamaño del paquete recibido por UART.
* El tamaño máximo permitido de ese pedido de memoria es de *FRAME_MAX_LENGTH 209*.
* Se libera la memoria dinamica una vez ya fue utilizada en la tarea C2_task_in.
* Se evaluará utilizar otra estrategia de manejo de menoria dinamica en el transcurso del desarrollo del TP.

R_C2_2
> La cantidad máxima de bytes de cualquier paquete de datos será de 200 caracteres.

* Se limita el tamaño del paquete que pueden recibir las C1_task a *FRAME_MAX_LENGTH = 209* para cumplir con este requerimiento.

R_C2_3
> Se deberá procesar paquetes que comienzan con SOM = "(" y finalizan con un EOM= ")".

* En la tarea C1_task se implementa una maquina de estado que detecta SOM y EOM.
* Estos frames son pasados como parte del dato a la tarea C2.

R_C2_4
> Recibir un caracter SOM reiniciará el paquete.

* Se construye una máquina de estado para tal propósito.

```c
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
}
}
```

R_C2_5
> Todos los bytes entrantes deberán procesarse en contexto de ISR durante la recepción de un paquete.

* Se propone que el handler de interrupcion solo tenga la generación de una cola para transmitir los caracteres.
* De este modo el handler será lo más corto posible y se considera una buena práctica de diseño.
* El procesamiento y validación de los caracteres se hace en la tarea C1_task.

R_C2_6
> Los bytes entrantes, deberán almacenarse en un bloque de memoria dinámica.

* Una vez recibido un paquete válido se genera una bloque de memoria dinámica para realizar el pasaje de datos entre la capa C1 y capa C2.

```c
else if (END_FRAME)
{
	// Mandar la cola de mensajes
	msg.index = index;
	C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
	C1_FSM[index].countChars++;
	msg.length = C1_FSM[index].countChars;
	msg.ptr = pvPortMalloc(msg.length * sizeof(uint8_t));
	configASSERT(msg.ptr != NULL);
	memcpy(msg.ptr, C1_FSM[index].pktRecieved, msg.length);
	xQueueSend(queueC1C2, &msg, portMAX_DELAY);
	C1_FSM[index].state = C1_IDLE;
}
```
R_C2_7
> Deberá tener control sobre la máxima cantidad de bytes recibidos.

* Se implementa una constante que determina la cantidad máxima de bytes recibidos por un paquete. *FRAME_MAX_LENGTH*

```c
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
```

R_C2_8
> Al elevar un paquete recibido a la capa de aplicación (C3), la C2 deberá poder seguir recibiendo otro frame en otro bloque de memoria dinámica, distinto al anterior.

* _ToDo_ _Entregar en clase 3._

R_C2_9
> En caso de no haber memoria, la recepción de datos del driver de la C1 deberá anularse.

* Se chequea luego del pvPortMalloc en la tarea C1_Task que si no se asigna memoria la máquina de estado vuelve a Idle y no envía el paquete a la capa C2.

```c
else if (END_FRAME)
{
	// Mandar la cola de mensajes
	msg.index = index;
	C1_FSM[index].pktRecieved[C1_FSM[index].countChars] = c;
	C1_FSM[index].countChars++;
	msg.length = C1_FSM[index].countChars;
	msg.ptr = pvPortMalloc(msg.length * sizeof(uint8_t));
	//configASSERT(msg.ptr != NULL);
	if (msg.ptr != NULL)
	{
		memcpy(msg.ptr, C1_FSM[index].pktRecieved, mlength);
		xQueueSend(queueC1C2, &msg, portMAX_DELAY);
	}
	C1_FSM[index].state = C1_IDLE;
}
```

R_C2_10
> Deberá parsear el campo ID y el CRC

* _ToDo_ _Entregar en clase 4._

R_C2_11
> El campo de ID y CRC que llegan en cada paquete estará formado por números ASCII hexadecimales.

* _ToDo_  _Entregar en clase 4._

R_C2_12
> Si el campo de ID o el del CRC poseen un caracter invalido, la trama deberá descartarse (válido: '0' a '9' y 'A' a 'F').

* _ToDo_ _Entregar en clase 4._

R_C2_13
> Cuando la aplicación (C3) desee enviar un mensaje por el canal de comunicación, C2 deberá agregarle el código de comprobación, la identificación y los delimitadores.

* _ToDo_ _Entregar en clase 4._

R_C2_14
> Deberá procesar en contexto de ISR todos los bytes salientes en C2

* _ToDo_ _Entregar en clase 4._

R_C2_15
> Al finalizar la transmisión, se deberá liberar la memoria dinámica utilizada para la transacción.

* _ToDo_ _Entregar en clase 4._

R_C2_16
> El campo de identificación deberá incluirse en la respuesta asociada al paquete.

* _ToDo_ _Entregar en clase 4._

R_C2_17
> Deberá sumar otro criterio de descarte de trama. Cuando transcurre un tiempo sin haber recibido un byte y el frame no haya sido "cerrado", el paquete deberá descartarse.

* _ToDo_ _Entregar en clase 5._

R_C2_18
> El timeout deberá implementarse con un timer de FreeRTOS.

* _ToDo_ _Entregar en clase 5._

R_C2_19
> El tiempo para cancelar la trama de datos inconclusa será de T = 4 ms desde el último byte recibido..

R_C2_20
> Para cada paquete, se deberá calcular el código de comprobación utilizando el algoritmo CRC8 con semilla = 0.

* _ToDo_ _Entregar en clase 5._

R_C2_21
> Al recibir un mensaje correcto, se deberá señalizar a la aplicación de su ocurrencia, para ser procesada.

* _ToDo_ _Entregar en clase 5._

R_C2_22
> Al recibir un mensaje correcto, se deberá señalizar a la aplicación de su ocurrencia, para ser procesada.

* _ToDo_ _Entregar en clase 5._
## Capa de aplicación (C3)


R_C3_1
> La cantidad máxima de palabras será de 15.

* _ToDo_ _Entregar en clase 6._

R_C3_2
> La cantidad mínima de palabras será de 1.

* _ToDo_ _Entregar en clase 6._

R_C3_3
> La cantidad máxima de caracteres por palabra será de 10.

* _ToDo_ _Entregar en clase 6._

R_C3_4
> La cantidad mínima de caracteres por palabras será de 1.

* _ToDo_ _Entregar en clase 6._

R_C3_5
> No existen paquetes con datos nulos.

* _ToDo_ _Entregar en clase 6._

R_C3_6
> El campo C deberá ser solamente 'S , 'C' o 'P' y estará asociado a la conversión a snake_case, camelCase y PascalCase, respectivamente.

* _ToDo_ _Entregar en clase 6._

R_C3_7
> El campo DATOS deberá procesar solo paquetes de texto a-z , A-Z, guiones bajo y espacios.

* _ToDo_ _Entregar en clase 6._

R_C3_8
> No se permitirán dobles “_” o dobles espacios “ “ entre palabras.

* _ToDo_ _Entregar en clase 6._

R_C3_9
> No se permitirán “_” o espacios “ “ al final de una trama de datos.

* _ToDo_ _Entregar en clase 6._

R_C3_10
> Si hay paquetes con caracteres que no cumplan o cuyo contenido sea inválido, deberán descartarse, y enviar a la capa C2 un mensaje de error con nro de error ERROR_INVALID_DATA.

* _ToDo_ _Entregar en clase 6._

R_C3_11
> Si el campo C no es válido, el paquetes deberá descartarse y enviar a la capa C2 un mensaje de error con nro de error ERROR_INVALID_OPCODE.

* _ToDo_ _Entregar en clase 6._

R_C3_12
> El texto entrante deberá procesarse según el campo C.

* _ToDo_ _Entregar en clase 6._

R_C3_13
> Los mensajes de error que devolverá la aplicación tendrán el formato: E de error, nn un número de error codificado en ascii (ver tabla). Notar que luego, C2 agregará la identificación y el CRC resultando una respuesta del este estilo (SSSSEnnCC).

* _ToDo_ _Entregar en clase 6._

R_AO_1
> La aplicación deberá transformarse en un objeto activo. OAapp

R_AO_2
> La aplicación deberá esperar dos tipos de evento:
a- un evento proveniente del driver que signifique “llegó un paquete procesar”.
b- un evento, con la respuesta procesada.

R_AO_3
> El evento “a” desencadenara tres acciones:
a1- validar el paquete a nivel C3
a2- si es válido, enviar un evento dinámico al OA asociado a la operación solicitada.
a3- si es inválido, enviar la respuesta de error a C2.

R_AO_4
> El evento “b” desencadenará una sola acción:
b1- Enviar la respuesta procesada a C2.

R_AO_5
> Existirán tres tipos de OSs de procesamiento: el de snake_case, camelCase y PascalCase (OAs,OAc y OAp)

R_AO_6
> En caso de que cualquiera de las OAs,OAc y OAp no existan, deberán instanciarse en tiempo de ejecución.

R_AO_7
> Al finalizar la operación, cada OAs,OAc y OAp deberá enviar la respuesta a través de un evento a OAapp.

R_AO_8
> Al finalizar la operación, cada OAs,OAc y OAp, si no quedan elementos para procesar en la cola de entrada, deberá destruirse.

R_AO_9
> Si algún proceso de creación de objetos fallará (falta de memoria por ejemplo) se deberá
enviar a C2 un mensaje de error con código ERROR_SYSTEM


## Opcionales

R_O_1
> Optimice la compilación del FreeRTOS desactivando en freertosconfig.h los elementos que NO utilice en su implementación.

* _ToDo_ _Entregar en clase 8._

R_O_2
> Optimice la compilación de toda la imagen de su firmware, empleando optimización de nivel
2 en el compilador. ¿ Mejoró la performance ? Cambie config.mk la opción OPT=g por OPT=2. Ver [volatile keyword](https://barrgroup.com/embedded-systems/how-to/c-volatile-keyword).

* _ToDo_ _Entregar en clase 8._




