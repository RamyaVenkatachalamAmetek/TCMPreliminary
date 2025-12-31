/**
 *  @file Watchdog.c
 *  @brief Handle Watchdog tasks
 *  @author SFC
 *
 **/


/* Includes */
#include "Tasks.h"
#include "Error.h"
#include "Watchdog.h"

/* Macros */

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */

/* Static Variables */
watchdogStatus_t currentStatus[WD_TASK_N_ENUM];
static IWDG_HandleTypeDef IWDG_Handle;

/* Private Functions */

/* Watchdog Process */
static void Watchdog_Task(void *Args)
{
    uint8_t i;
    bool refreshWatchdog = false;
    TickType_t xLastWakeTime;

    /* set all states to unknown */
    for (i = 0; i < WD_TASK_N_ENUM; i++)
    	currentStatus[i] = WD_UNKNOWN;

    while(1) {
        xLastWakeTime = xTaskGetTickCount();

        /* check if every task has checked in */
        refreshWatchdog = true;
        for (i=0; i < WD_TASK_N_ENUM; i++) {
        	if (currentStatus[i] == WD_UNKNOWN)
        		refreshWatchdog = false;
        }

        if (refreshWatchdog) {

        	/* refresh watchdog */
        	HAL_IWDG_Refresh(&IWDG_Handle);

        	for (i=0; i< WD_TASK_N_ENUM; i++) {
        		/* reset all alive tasks to unknown */
        		if (currentStatus[i] != WD_ASLEEP)
        			currentStatus[i] = WD_UNKNOWN;
        	}
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}


static void WatchdogTask_Create(void)
{
    static StaticTask_t xWatchdogTaskTCB;
    static StackType_t uxWatchdogTaskStack[WATCHDOGTASK_STACKSZ];

    xWatchdogTaskHandle = xTaskCreateStatic(Watchdog_Task,
                                        	WATCHDOGTASK_NAME,
											WATCHDOGTASK_STACKSZ,
											NULL,
											WATCHDOGTASK_PRIO,
											uxWatchdogTaskStack,
											&xWatchdogTaskTCB);
    if(xWatchdogTaskHandle == NULL)
        Error_Handler(ERROR_TASK_CREATE);
}


/* Initialize */

bool WD_Init(void)
{
	/* independent watchdog */
	IWDG_Handle.Instance = IWDG;
	/* set prescaler for 2ms tick time = (4 * 16)/32000 */
	IWDG_Handle.Init.Prescaler = IWDG_PRESCALER_16;
	/* set watchdog time to 2 seconds (1000 * 2ms) (max value: 4095) */
	IWDG_Handle.Init.Reload = 1000;
	/* disable window option */
	IWDG_Handle.Init.Window = 0xfff;

	if (HAL_IWDG_Init(&IWDG_Handle) != HAL_OK)
		Error_Handler(ERROR_BOOTUP_WATCHDOG_INIT);

	WatchdogTask_Create();

	return true;
}

void WD_Status(watchdogTask_t task, watchdogStatus_t status)
{
	/* set task state */
	currentStatus[task] = status;
}

