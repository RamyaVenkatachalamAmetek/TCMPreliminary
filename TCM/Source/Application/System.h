/**
 **  @file System.h
 **  @brief System Management
 **  @author JZJ
 **
 **/
 
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/* Includes */
#include "PAL.h"
#include "Power.h"

/* Macros */

/* System error conditions */
#define SYS_ERR_TEMP_LOW    (1)
#define SYS_ERR_TEMP_HIGH   (2)

/* Types */

/* Function Prototypes */
/* Init - Stage1 */
bool Sys_InitStage1(void);
/* Init - Stage2 */
bool Sys_InitStage2(void);
/* Set reboot request */
bool Sys_SetReqReboot(void);
/* Set sleep request */
bool Sys_SetReqSleep(void);
/* Is sleeping */
bool Sys_IsSleeping(void);
/* Set bootloader request */
bool Sys_SetReqBootloader(void);
/* Set enable/disable charging */
bool Sys_SetReqCharging(bool Enable);
/* Get Time */
uint32_t Sys_GetTime(void);
/* Set Time */
bool Sys_SetTime(uint32_t SysTime);
/* Is Charging */
bool Sys_IsCharging(void);
/* Is charging enabled */
bool Sys_IsChargeEnabled(void);
/* Is battery connected */
bool Sys_IsBattConnected(void);
/* Set battery connection status */
void Sys_SetBattConnected(bool Connected);
/* Is USB connected */
bool Sys_IsUSBConnected(void);
/* Is TCM connected */
bool Sys_IsTCMConnected(void);
/* Battery level */
uint32_t Sys_GetBattLevel(void);
/* Battery voltage */
float32_t Sys_GetBattVoltage(void);
/* Battery temeprature */
float32_t Sys_GetBattTemp(void);
/* Temperature Limit Alarm */
bool Sys_AlarmBattTemp(void);
/* Low charge Alarm - when RSOC is < 8% */
bool Sys_AlarmLowCharge(void);
/* Check operating conditions */
uint32_t Sys_CheckOpConditions(void);
/* Set keypad active */
void Sys_SetKeypadActive(uint32_t Key);
/* Set comm active */
void Sys_SetCommActive(void);
/* Shutdown Mode */
void Sys_Shutdown(bool IsRunning);
/* Set wakeup from EXT_PWR */
void Sys_SetWakeupOnExtPwr(bool Wakeup);
/* Get EXT_PWR wakeup status */
bool Sys_GetWakeupOnExtPwr(void);
/* Check Power configuration */
StdReturn_t Sys_CheckPwrConfig(void);
/* Disables BQ2414x OTG mode */
void Sys_DisableOTGmode(void);

#endif /*** _SYSTEM_H_ ***/
 
