/**
 *  @file Watchdog.h
 *  @brief Handle Watchdog tasks
 *  @author SFC
 *
 **/

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

/* Includes */
#include "PAL.h"
#include "stm32l4xx_hal_iwdg.h"

/* Macros */

/* Types */

typedef enum {
	WD_SYSTASK = 0,
	WD_DISPTASK,
	WD_IOTASK,
	WD_DAQ,
	WD_CFG,
	WD_CMDUSB,
	WD_CMDTCM,
	WD_COMDATA,
	WD_EVTUSB,
	WD_EVTTCM,
	WD_AxMHOST,
	WD_AxM1,
	WD_AxM2,
	WD_UPDATEAxM,
	WD_TASK_N_ENUM,
}watchdogTask_t;

typedef enum {
	WD_UNKNOWN,
	WD_ASLEEP,
	WD_ALIVE,
}watchdogStatus_t;

/* Function Prototypes */

/* Update Watchdog Status */
void WD_Status(watchdogTask_t task, watchdogStatus_t status);
/* Initialize */
bool WD_Init(void);

#endif /* _WATCHDOG_H_ */
