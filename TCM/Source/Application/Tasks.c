/**
 *  @file Tasks.c
 *  @brief Tasks and Inter Task Communication
 *  @author JZJ
 *
 **/

/* Includes */
#include "Tasks.h"

#include "DAQ.h"

/* Macros */

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */
TaskHandle_t xDaqTaskHandle;        // DAQ
TaskHandle_t xDispTaskHandle;       // DISP
TaskHandle_t xCfgTaskHandle;        // CFG
TaskHandle_t xComCmdUSBTaskHandle;  // COMCMDUSB
TaskHandle_t xComCmdTCMTaskHandle;  // COMCMDTCM
TaskHandle_t xComDataTaskHandle;    // COMDAT
TaskHandle_t xComEvtUSBTaskHandle;  // COMEVTUSB
TaskHandle_t xComEvtTCMTaskHandle;  // COMEVTTCM
TaskHandle_t xAxMHostTaskHandle;    // AxMHOST
TaskHandle_t xAxM1RxTaskHandle;     // AxM1RX
TaskHandle_t xAxM2RxTaskHandle;     // AxM2RX
TaskHandle_t xSysTaskHandle;        // SYS
TaskHandle_t xIOTaskHandle;         // IO
TaskHandle_t xWatchdogTaskHandle;	// WATCHDOG
TaskHandle_t xUpdateAxMTaskHandle;	// UPAxM

QueueHandle_t LogDataQ; // Data samples for logging
QueueHandle_t ComDataQ; // Data samples for communication
QueueHandle_t CmdUSBQ;  // Commands over USB
QueueHandle_t CmdTCMQ;  // Commands over TCM
QueueHandle_t CmdAxM1Q;  // Commands over AxM1
QueueHandle_t CmdAxM2Q;  // Commands over AxM2

SemaphoreHandle_t COMUSBSem; // Mutex for USB COM sync

/* Static Variables */

/* Private Functions */
/* Create Inter Task Communciation Objects */
static void CreateSyncObjects(void)
{
#define SAMPLEQ_LEN    (500)
#define SAMPLEQ_SIZE   (sizeof(ChnReading_t)) // Timestamp and measurement

    /* For data logging - from MxA to DAQ */
    static StaticQueue_t xLogDataQStruct;
    static uint8_t logDataQStorage[SAMPLEQ_LEN * SAMPLEQ_SIZE];

    LogDataQ = xQueueCreateStatic(SAMPLEQ_LEN,
            SAMPLEQ_SIZE,
            logDataQStorage,
            &xLogDataQStruct);
    configASSERT(LogDataQ);

    /* For data communication - from MxA to COM */
    static StaticQueue_t xComDataQStruct;
    static uint8_t comDataQStorage[SAMPLEQ_LEN * SAMPLEQ_SIZE];

    ComDataQ = xQueueCreateStatic(SAMPLEQ_LEN,
            SAMPLEQ_SIZE,
            comDataQStorage,
            &xComDataQStruct);
    configASSERT(ComDataQ);

#define CMDQ_LEN    (256)
#define CMDQ_SIZE   (sizeof(uint8_t)) // byte stream

    /* For command receive - USB */
    static StaticQueue_t xCmdUSBQStruct;
    static uint8_t cmdUSBQStorage[CMDQ_LEN * CMDQ_SIZE];

    CmdUSBQ = xQueueCreateStatic(CMDQ_LEN,
            CMDQ_SIZE,
            cmdUSBQStorage,
            &xCmdUSBQStruct);
    configASSERT(CmdUSBQ);

    /* For command receive - TCM */
    static StaticQueue_t xCmdTCMQStruct;
    static uint8_t cmdTCMQStorage[CMDQ_LEN * CMDQ_SIZE];

    CmdTCMQ = xQueueCreateStatic(CMDQ_LEN,
            CMDQ_SIZE,
            cmdTCMQStorage,
            &xCmdTCMQStruct);
    configASSERT(CmdTCMQ);

    /* For COM sync - USB */
    static StaticSemaphore_t xCOMUSBSemStruct;

    COMUSBSem = xSemaphoreCreateBinaryStatic(&xCOMUSBSemStruct);
    configASSERT(COMUSBSem);

    /* For commands - AxM1 */
    static StaticQueue_t xCmdAxM1QStruct;
    static uint8_t cmdAxM1QStorage[CMDQ_LEN * CMDQ_SIZE];

    CmdAxM1Q = xQueueCreateStatic(CMDQ_LEN,
            CMDQ_SIZE,
            cmdAxM1QStorage,
            &xCmdAxM1QStruct);
    configASSERT(CmdAxM1Q);

    /* For commands - AxM2 */
    static StaticQueue_t xCmdAxM2QStruct;
    static uint8_t cmdAxM2QStorage[CMDQ_LEN * CMDQ_SIZE];

    CmdAxM2Q = xQueueCreateStatic(CMDQ_LEN,
            CMDQ_SIZE,
            cmdAxM2QStorage,
            &xCmdAxM2QStruct);
    configASSERT(CmdAxM2Q);
}

/* Public Functions */

/* Initialize */
bool Tasks_Init(void)
{
    CreateSyncObjects();

    return true;
}

/******************************** End of File *********************************/
