/**
 *  @file RTOS.h
 *  @brief RTOS Abstractions
 *  @author JZJ
 *
 **/

#ifndef _RTOS_H_
#define _RTOS_H_

/* Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/* Macros */

/* Types */

/* Function Prototypes */
void xPortSysTickHandler( void );

#endif /* _RTOS_H_ */
