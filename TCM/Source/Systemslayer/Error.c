/**
 **  @file Error.c
 **  @brief Error handling
 **  @author JZJ
 **
 **/

/* Includes */
#include "PAL.h"
#include "Error.h"

/* Macros */

/* Types */

#define ERROR_LOG_SIZE		100


typedef struct {
	errorLevel_t level;
	const char *description;
}errorHandler_t;

/* Externs */

/* Function Declarations */

/* Global Variables */

/* Static Variables */

static errorCode_t Log[ERROR_LOG_SIZE];
static uint32_t errorCount = 0;

static const errorHandler_t ErrorHandler[ERROR_N_ENUM] = {
	[ERROR_NONE] = 					{LEVEL_INFO, 	"NONE"},
	[ERROR_BOOTUP_CLOCK_INIT] = 	{LEVEL_FATAL, 	"CLK INIT"},
	[ERROR_BOOTUP_SDC] = 			{LEVEL_ERROR, 	"SDC INIT"},
	[ERROR_BOOTUP_USB] =			{LEVEL_FATAL, 	"USB INIT"},
	[ERROR_TASK_CREATE] = 			{LEVEL_FATAL, 	"TASK CREATION"},
	[ERROR_TASK_STACK] = 		    {LEVEL_FATAL, 	"TASK STACK"},
	[ERROR_BOOTUP_POWER_INIT1] = 	{LEVEL_ERROR, 	"POWER INIT 1"},
	[ERROR_BOOTUP_POWER_INIT2] = 	{LEVEL_ERROR, 	"POWER INIT 2"},
	[ERROR_SETUP_USB_DEVICE] = 		{LEVEL_ERROR,	"USBD SETUP"},
	[ERROR_BOOTUP_WATCHDOG_INIT] =  {LEVEL_FATAL,	"WDOG INIT"},
};


/* Private Functions */

/* Log error and continue */
static void Error_Log(uint32_t ErrNo)
{
	/* log error */
	Log[errorCount] = ErrNo;

	/* increase log size */
	if (errorCount < ERROR_LOG_SIZE)
		errorCount++;
}

/* Public Functions */
void Error_Handler_Init(void)
{
	/* clear error log */
	errorCount = 0;
	memset(&Log, ERROR_NONE, sizeof(Log));
}

/* Log error and halt/restart */
void Error_Handler(uint32_t ErrNo)
{
	Error_Log(ErrNo);
    
    if (ErrorHandler[ErrNo].level != LEVEL_FATAL) return;

#ifdef NDEBUG
    /* TBD - Restart */
#else
    while(1);
#endif
}

/* Get error count for specific error number*/
uint32_t Error_GetCount(uint32_t ErrNo)
{
	uint8_t i = 0;
	uint8_t count = 0;

	while ((Log[i] != ERROR_NONE) && (i < ERROR_LOG_SIZE)) {
		if (Log[i++] == ErrNo)
			count++;
	}

	return count;
}

/* Clear error */
void Error_Clear(uint32_t ErrNo)
{
    uint8_t i = 0;

    /* find error */
    while ((Log[i] != ERROR_NONE) && (i < ERROR_LOG_SIZE) && (Log[i] != ErrNo))
        i++;

    if (Log[i] == ErrNo){
    	/* clear error and shift log entries */
        while ((Log[i] != ERROR_NONE) && (i < ERROR_LOG_SIZE)) {
            if (i == (ERROR_LOG_SIZE - 1))
                Log[i] = ERROR_NONE;
            else
                Log[i] = Log[i + 1];

            i++;
        }

        /* decrease log count */
        errorCount--;
    }
}

/* Get error description */
const char * Error_GetDesc(uint32_t ErrNo)
{
	return ErrorHandler[ErrNo].description;
}

/* Get error level */
const errorLevel_t Error_GetLevel(uint32_t ErrNo)
{
	return ErrorHandler[ErrNo].level;
}


/* Get log size */
uint32_t ErrorLog_GetSize(void)
{
	return errorCount;
}

/* Get log error code based on index */
errorCode_t ErrorLog_GetErrorCode(uint32_t index)
{
	if (index < ERROR_LOG_SIZE)
		return Log[index];
	else
		return ERROR_NONE;
}

/* Get error level */
const errorLevel_t ErrorLog_GetErrorLevel(uint32_t index)
{
	if (index < ERROR_LOG_SIZE)
		return ErrorHandler[Log[index]].level;
	else
		return ErrorHandler[ERROR_NONE].level;
}

/* Get log error description based on index */
const char * ErrorLog_GetErrorDesc(uint32_t index)
{
	if (index < ERROR_LOG_SIZE)
		return ErrorHandler[Log[index]].description;
	else
		return ErrorHandler[ERROR_NONE].description;
}

/******************************** End of File *********************************/
