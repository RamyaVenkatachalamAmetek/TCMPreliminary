/**
 *  @file Tasks.h
 *  @brief Tasks and Inter Task Communication
 *  @author JZJ
 *
 **/

#ifndef _TASKS_H_
#define _TASKS_H_

/* Includes */
#include "PAL.h"
#include "RTOS.h"

/* Macros */

/* Tasks info */

/* Display Task */
#define DISPTASK_NAME       ("DISP")
#define DISPTASK_PRIO       (1)
#define DISPTASK_STACKSZ    (1024 + 512)
/* Config Task */
#define CFGTASK_NAME       ("CFG")
#define CFGTASK_PRIO       (2)
#define CFGTASK_STACKSZ    (1024)
/* System Task */
#define SYSTASK_NAME    ("SYS")
#define SYSTASK_PRIO    (3)
#define SYSTASK_STACKSZ (256)
/* AxM Host */
#define AxMHOSTTASK_NAME    ("AxMHOST")
#define AxMHOSTTASK_PRIO    (3)
#define AXMHOSTTASK_STACKSZ (256)
/* Update AxM */
#define UPDATEAxMTASK_NAME    ("UPAxM")
#define UPDATEAxMTASK_PRIO    (6)
#define UPDATEAxMTASK_STACKSZ (256)
/* Daq Task */
#define DAQTASK_NAME    ("DAQ")
#define DAQTASK_PRIO    (4)
#define DAQTASK_STACKSZ (1024)
/* Command Task - USB/TCM */
#define COMCMDUSBTASK_NAME  	("COMCMDUSB")
#define COMCMDUSBTASK_PRIO  	(5)
#define COMCMDUSBTASK_STACKSZ	(512 + 256)
#define COMCMDTCMTASK_NAME  	("COMCMDTCM")
#define COMCMDTCMTASK_PRIO  	(5)
#define COMCMDTCMTASK_STACKSZ  	(512 + 256)
/* Data Task */
#define COMDATATASK_NAME    ("COMDAT")
#define COMDATATASK_PRIO    (5)
#define COMDATATASK_STACKSZ (256)
/* Event Task - USB/TCM */
#define COMEVTUSBTASK_NAME       ("COMEVTUSB")
#define COMEVTUSBTASK_PRIO       (5)
#define COMEVTUSBTASK_STACKSZ    (512)
#define COMEVTTCMTASK_NAME       ("COMEVTTCM")
#define COMEVTTCMTASK_PRIO       (5)
#define COMEVTTCMTASK_STACKSZ    (256)
/* AxM Tasks - Rx */
#define AxM1RXTASK_NAME     ("AxM1RX")
#define AxM1RXTASK_PRIO     (5)
#define AxM2RXTASK_NAME     ("AxM2RX")
#define AxM2RXTASK_PRIO     (5)
#define AXMRXTASK_STACKSZ   (256)
/* IO Task */
#define IOTASK_NAME     ("IO")
#define IOTASK_PRIO     (6)
#define IOTASK_STACKSZ  (256)
/* Daq Task */
#define WATCHDOGTASK_NAME    ("WATCHDOG")
#define WATCHDOGTASK_PRIO    (6)
#define WATCHDOGTASK_STACKSZ (256)

/* Task priority levels */
#define TASKPRIO_MIN (1)
#define TASKPRIO_MAX (6)

/* USB COM events */
#define EVT_USB_TBREAK      (0x00000001)
#define EVT_USB_CBREAK      (0x00000002)
#define EVT_USB_OVERLOAD    (0x00000004)
#define EVT_USB_TESTSTOP    (0x00000008)
#define EVT_USB_BOOTERR     (0x00000010)
#define EVT_USB_TSAFELMT    (0x00000020)
#define EVT_USB_CSAFELMT    (0x00000040)
#define EVT_USB_EXPORT_FILE (0x00000100)
#define EVT_USB_UPDSTAT		(0x00000200)
#define EVT_USB_MASKALL     (0x0000037F)
#define EVT_USB_EXP_ADATA	(0x00010000)
#define EVT_USB_EXP_ADATA_H	(0x00020000)

/* TCM COM events */
#define EVT_TCM_RTZ   	(0x00000001)
#define EVT_TCM_ZERO    (0x00000002)
#define EVT_TCM_SPEED   (0x00000004)
#define EVT_TCM_TLMT    (0x00000008)
#define EVT_TCM_CLMT    (0x00000010)
#define EVT_TCM_TBRK    (0x00000020)
#define EVT_TCM_CBRK    (0x00000040)
#define EVT_TCM_COVLD   (0x00000080)
#define EVT_TCM_TOVLD   (0x00000100)
#define EVT_TCM_TSTART	(0x00000200)
#define EVT_TCM_TSTOP	(0x00000400)
#define EVT_TCM_PSTOP   (0x00000800)
#define EVT_TCM_CLEAR   (0x00001000)
#define EVT_TCM_HSCR    (0x00002000)
#define EVT_TCM_ALL     (0x00003FFF)

/* Types */
extern TaskHandle_t xDaqTaskHandle;
extern TaskHandle_t xDispTaskHandle;
extern TaskHandle_t xCfgTaskHandle;
extern TaskHandle_t xComCmdUSBTaskHandle;
extern TaskHandle_t xComCmdTCMTaskHandle;
extern TaskHandle_t xComDataTaskHandle;
extern TaskHandle_t xComEvtUSBTaskHandle;
extern TaskHandle_t xComEvtTCMTaskHandle;
extern TaskHandle_t xAxMHostTaskHandle;
extern TaskHandle_t xAxM1RxTaskHandle;
extern TaskHandle_t xAxM2RxTaskHandle;
extern TaskHandle_t xSysTaskHandle;
extern TaskHandle_t xIOTaskHandle;
extern TaskHandle_t xWatchdogTaskHandle;
extern TaskHandle_t xUpdateAxMTaskHandle;

extern QueueHandle_t LogDataQ;
extern QueueHandle_t ComDataQ;
extern QueueHandle_t CmdUSBQ;
extern QueueHandle_t CmdTCMQ;
extern QueueHandle_t CmdAxM1Q;
extern QueueHandle_t CmdAxM2Q;

extern SemaphoreHandle_t COMUSBSem;

/* Function Prototypes */
/* Initialize */
bool Tasks_Init(void);

#endif /* _TASKS_H_ */
