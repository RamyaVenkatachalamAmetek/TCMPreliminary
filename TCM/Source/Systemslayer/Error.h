/**
 **  @file Error.h
 **  @brief Error handling
 **  @author JZJ
 **
 **/

#ifndef _ERROR_H_
#define _ERROR_H_

/* Includes */

/* Macros */

/* Types */

typedef enum {
	ERROR_NONE,					// no error
	ERROR_BOOTUP_CLOCK_INIT,	// error in clock initialization
	ERROR_BOOTUP_SDC,			// error in SD card initialization
	ERROR_BOOTUP_USB,			// error in USB initialization
	ERROR_TASK_CREATE,			// error in task creation
	ERROR_TASK_STACK,		    // error task stack overflow
	ERROR_BOOTUP_POWER_INIT1,	// error in power init stage 1
	ERROR_BOOTUP_POWER_INIT2,	// error in power init state 2
	ERROR_SETUP_USB_DEVICE,		// error in setting USB Device mode
	ERROR_BOOTUP_WATCHDOG_INIT,	// error in Watchdog initialization
	/* ADD NEW ERRORS HERE */
	ERROR_N_ENUM,
}errorCode_t;


typedef enum {
	LEVEL_INFO,				// no error - log info and continue
	LEVEL_WARN,				// warning - log warning and continue
	LEVEL_ERROR,			// error - log error and continue
	LEVEL_FATAL,			// fatal level - halt system
	LEVEL_N_ENUM
} errorLevel_t;


/* Function Prototypes */
/* Error Handler Initialization */
void Error_Handler_Init(void);
/* Log error and halt/restart */
void Error_Handler(uint32_t ErrNo);
/* Get error count */
uint32_t Error_GetCount(uint32_t ErrNo);
/* Clear error */
void Error_Clear(uint32_t ErrNo);
/* Get error description */
const char * Error_GetDesc(uint32_t ErrNo);
/* Get error level */
const errorLevel_t Error_GetLevel(uint32_t ErrNo);

/* Get log size */
uint32_t ErrorLog_GetSize(void);
/* Get log error code based on index */
errorCode_t ErrorLog_GetErrorCode(uint32_t index);
/* Get log error description based on index */
const char * ErrorLog_GetErrorDesc(uint32_t index);




#endif /*** _ERROR_H_ ***/
