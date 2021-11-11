/*=============================================================================
 * Copyright (c) 2021, Gonzalo Nahuel Vaca <vacagonzalo@gmail.com>
 *                     Lucas Zalazar <lucas.zalazar6@gmail.com>
 *                     Carlos Maffrand <carlosmaffrand5@gmail.com>
 * All rights reserved.
 * License: mit (see LICENSE.txt)
 * Date: 2021/10/30
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "main.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sapi.h"
#include "userTasks.h"
#include "msg.h"
#include "C1.h"
#include "C2.h"
#include "C3.h"
#include "appConfig.h"
/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main(void)
{
   boardInit();

   queue_init();
   C1_init(UARTS_TO_USE);
   C2_init();
   C3_init();

   BaseType_t res;

   // Create a task in freeRTOS with dynamic memory
   res = xTaskCreate(
       myTask,                       // Function that implements the task.
       (const char *)"myTask",       // Text name for the task.
       configMINIMAL_STACK_SIZE * 2, // Stack size in words, not bytes.
       0,                            // Parameter passed into the task.
       tskIDLE_PRIORITY + 1,         // Priority at which the task is created.
       0                             // Pointer to the task created in the system
   );
   configASSERT(res == pdPASS);

   vTaskStartScheduler(); // Initialize scheduler

   while (true)
      ; // If reach heare it means that the scheduler could not start

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the
   // case of a PC program.
   return 0;
}
