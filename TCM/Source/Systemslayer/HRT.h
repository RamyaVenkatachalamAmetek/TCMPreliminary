/**
 **  @file HRT.h
 **  @brief High Resolution Timer
 **  @author JZJ
 **
 **/

#ifndef _HRT_H_
#define _HRT_H_

/* Includes */

/* Macros */

/* Types */
typedef uint32_t HRTime_t;

/* Function Prototypes */
/* Init */
void HRT_Init(void);
/* Increment */
void HRT_IncTick(void);
/* Get time */
HRTime_t HRT_GetTick(void);
/* Delay (usecs) */
void HRT_Delay(uint32_t Delay);
/* Check timeout */
bool HRT_IsTimedOut(uint32_t tickStart, uint32_t Timeout);
/* Pause timer */
void HRT_Pause(void);
/* Resume timer */
void HRT_Resume(void);

#endif /*** _HRT_H_ ***/
