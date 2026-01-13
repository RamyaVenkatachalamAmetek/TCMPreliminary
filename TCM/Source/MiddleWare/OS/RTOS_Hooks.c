/**
 *  @file RTOS_Hooks.c
 *  @brief RTOS application hook functions
 *  @author JZJ
 *
 **/

/* Includes */
#include "PAL.h"

#include "RTOS.h"
#include "Error.h"

/* Macros */

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */

/* Static Variables */

/* Private Functions */

/* Public Functions */

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    /* configSUPPORT_STATIC_ALLOCATION is set to 1 */
    /* Provide stack for idle task */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

TaskHandle_t OVTaskHandle;
char OVTaskName[16];
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    OVTaskHandle = xTask;
    strncpy(OVTaskName, pcTaskName, 15);

    Error_Handler(ERROR_TASK_STACK);
}

/******************************** End of File *********************************/
