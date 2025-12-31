/**
 *  @file COM.c
 *  @brief Communication Handler
 *  @author JZJ
 *
 **/

/* Includes */
#include "COM.h"
#include "DAQ.h"
#include "Tasks.h"
#include "Cmds.h"
#include "System.h"

#include "Error.h"

#include "USBi.h"
#include "TCMi.h"
#include "AxMi.h"

#include "IO.h"
#include "Watchdog.h"

#include "Device.h"

/* Macros */

/* Rx/Tx Len */
#define COM_RXBUF_LEN (256)
#define COM_TXBUF_LEN (256)

/* COM receive timeout */
#define COM_RX_TIMEOUT (10) // 10 msecs
/* COM transmit timeout */
#define COM_TX_TIMEOUT (100) // 100 msecs

/* COM - TCM Tx lock threshold - around 0.5 secs */
#define COM_TCM_TX_LOCK (5)

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */

/* Static Variables */
/* USB Tx/Rx */
static uint8_t COMUSB_RxBuf[COM_RXBUF_LEN];
static uint8_t COMUSB_TxBuf[COM_TXBUF_LEN];
static uint32_t COMUSB_RxLen = 0;
static uint32_t COMUSB_TxLen = 0;
static bool COMUSBRxInProgress = false;

/* TCM Tx/Rx */
static uint8_t COMTCM_RxBuf[COM_RXBUF_LEN];
static uint8_t COMTCM_TxBuf[COM_TXBUF_LEN];
static uint32_t COMTCM_RxLen = 0;
static uint32_t COMTCM_TxLen = 0;


/* Private Functions */

/* Reset USB */
static inline void COMUSB_ResetRx(void)
{
    COMUSB_RxLen = 0;
    COMUSBRxInProgress = false;
}
static inline void COMUSB_ResetTx(void)
{
	COMUSB_TxLen = 0;
}

/* Reset TCM */
static inline void COMTCM_ResetRxTx(void)
{
    COMTCM_RxLen = 0;
    COMTCM_TxLen = 0;
}
static inline void COMTCM_ResetRx(void)
{
    COMTCM_RxLen = 0;
}
static inline void COMTCM_ResetTx(void)
{
	COMTCM_TxLen = 0;
}

/* USB Tx function */
static void COMUSB_TxData(uint8_t *Data, uint32_t Size)
{
	if (pdTRUE == xSemaphoreTake(COMUSBSem, (TickType_t) COM_TX_TIMEOUT)) {
		USBi_Tx(Data, Size);
		COMUSB_ResetTx();
	} else {
		/* If sem is not signalled from ISR, signal it and acquire it from here */
		/* This is not the best place to do it, but keep it here for now */
		xSemaphoreGive(COMUSBSem);
		if (pdTRUE == xSemaphoreTake(COMUSBSem, (TickType_t) 0)) {
			USBi_Tx(Data, Size);
			COMUSB_ResetTx();
		}
	}
}

/* Communication Process - Commands */
static void COM_CMDUSBTask(void *Args)
{
    uint8_t newByte;
    CmdStatus_t cmdStatus;

    /* Wait till Config notifies completion */
    uint32_t notifiedValue;
    xTaskNotifyWait(UINT_MIN, UINT_MAX, &notifiedValue, portMAX_DELAY);

    /* Release Tx Lock semaphore */
    xSemaphoreGive(COMUSBSem);

    while(1) {

    	/* set watchdog status to asleep */
    	WD_Status(WD_CMDUSB, WD_ASLEEP);

        /* Commands over USB */
        if (pdPASS == xQueueReceive(CmdUSBQ, &newByte, pdMS_TO_TICKS(COM_RX_TIMEOUT))) {
        	/* set watchdog status to alive */
        	WD_Status(WD_CMDUSB, WD_ALIVE);

            COMUSB_RxBuf[COMUSB_RxLen++] = newByte;
            COMUSBRxInProgress = true;

            cmdStatus = CmdUSB_Process(COMUSB_RxBuf, COMUSB_RxLen, COMUSB_TxBuf, &COMUSB_TxLen);
            /* Binary commands */
            if (cmdStatus == CMDSTAT_DONE) {
                if (COMUSB_TxLen > 0)
                	COMUSB_TxData(COMUSB_TxBuf, COMUSB_TxLen);
                COMUSB_ResetRx();
                /* Set active, if we have a command */
                Sys_SetCommActive();
            }

        } else if (CmdUSB_IsDF2DataStreaming()) {
			CmdUSB_SetDF2Reading(COMUSB_TxBuf, &COMUSB_TxLen);
			COMUSB_TxData(COMUSB_TxBuf, COMUSB_TxLen);
			COMUSB_ResetRx();

			Sys_SetCommActive();

        } else {

        	/* set watchdog status to alive */
        	WD_Status(WD_CMDUSB, WD_ALIVE);
            /* Handle comm timeout */
            if (COMUSBRxInProgress)
            	COMUSB_ResetRx();
        }
    }
}

static void COM_CMDTCMTask(void *Args)
{
    bool rxInProgress = false;
    CmdStatus_t cmdStatus;
    uint32_t txLockCnt = 0;
	uint8_t newByte;

    /* Wait till Config notifies completion */
    uint32_t notifiedValue;
    xTaskNotifyWait(UINT_MIN, UINT_MAX, &notifiedValue, portMAX_DELAY);

    while(1) {

        /* set watchdog status to asleep */
        WD_Status(WD_CMDTCM, WD_ASLEEP);

        /* Commands over TCM */
        if (pdPASS == xQueueReceive(CmdTCMQ, &newByte, pdMS_TO_TICKS(COM_RX_TIMEOUT))) {

            /* set watchdog status to alive */
            WD_Status(WD_CMDTCM, WD_ALIVE);

            COMTCM_RxBuf[COMTCM_RxLen++] = newByte;
            rxInProgress = true;

            cmdStatus = CmdTCM_Process(COMTCM_RxBuf, COMTCM_RxLen, COMTCM_TxBuf, &COMTCM_TxLen);
            if (cmdStatus == CMDSTAT_DONE) {
            	/* Tx only when there is data in buffer */
            	if (COMTCM_TxLen > 0) {
            		if (TCMi_IsTxReady()) {
            			/* Tx is not locked */
            			txLockCnt = 0;
            			TCMi_Tx(COMTCM_TxBuf, COMTCM_TxLen);
            		} else {
            			/* Reset TxReady, if Tx is locked continously */
            			if (++txLockCnt >= COM_TCM_TX_LOCK) {
            				TCMi_ResetTxReady();
            				txLockCnt = 0;
            			}
            		}
                }
                rxInProgress = false;
                COMTCM_ResetRxTx();
                if (!COM_IsASCIIMode()) {
					/* Set active, if we have a command */
					Sys_SetCommActive();
					TCMi_SetConnected(true);
                }
            }
        } else {
            /* set watchdog status to alive */
            WD_Status(WD_CMDTCM, WD_ALIVE);

            /* Handle comm timeout */
            if (rxInProgress) {
            	COMTCM_ResetRx();
                rxInProgress = false;
            }
        }
    }
}

/* Communication Process - Data */
static void COM_DATATask(void *Args)
{
    ChnReading_t loadReading;
    uint32_t txTCMLockCnt = 0;

    /* Wait till Config notifies completion */
    uint32_t notifiedValue;
    xTaskNotifyWait(UINT_MIN, UINT_MAX, &notifiedValue, portMAX_DELAY);

    while(1) {

    	/* set watchdog status to asleep */
    	WD_Status(WD_COMDATA, WD_ASLEEP);

        /* Send data from MxA */
        if (pdPASS == xQueueReceive(ComDataQ, &loadReading, portMAX_DELAY)) {

            /* set watchdog status to alive */
            WD_Status(WD_COMDATA, WD_ALIVE);


            /* If TCM burst is enabled, send data to TCM port. Otherwise, USB port */
            if (TCMi_IsConnected() && TCMi_GetBurstMode()) {
            	CmdTCM_Tx_Reading(loadReading.Reading, TCMi_GetReading(), COMTCM_TxBuf, &COMTCM_TxLen);
            	if (TCMi_IsTxReady()) {
            		txTCMLockCnt = 0;
            		TCMi_Tx(COMTCM_TxBuf, COMTCM_TxLen);
            	} else {
            		/* Reset TxReady, if Tx is locked continously */
            		if (++txTCMLockCnt >= COM_TCM_TX_LOCK) {
            			TCMi_ResetTxReady();
            			txTCMLockCnt = 0;
            		}
                }
                COMTCM_ResetTx();
	        } else {
        		if (COM_IsASCIIMode())
        			CmdUSB_Tx_ASCIIReading(loadReading.Src, loadReading.Reading, COMUSB_TxBuf, &COMUSB_TxLen);
        		else
        			CmdUSB_Tx_Reading(loadReading.Src, loadReading.Reading, COMUSB_TxBuf, &COMUSB_TxLen);
        		COMUSB_TxData(COMUSB_TxBuf, COMUSB_TxLen);
        	}

        }
    }
}

/* Communication Process - Events - USB */
static void COM_EVTUSBTask(void *Args)
{
    uint32_t comEvent = 0;
    bool txEventUSB = false;

    /* Wait till Config notifies completion */
    uint32_t notifiedValue;
    xTaskNotifyWait(UINT_MIN, UINT_MAX, &notifiedValue, portMAX_DELAY);

    while(1) {
    	/* set watchdog status to asleep */
    	WD_Status(WD_EVTUSB, WD_ASLEEP);

        /* Wait for an event */
        xTaskNotifyWait(UINT_MIN, UINT_MAX, &comEvent, portMAX_DELAY);

    	/* set watchdog status to alive */
    	WD_Status(WD_EVTUSB, WD_ALIVE);

    	/* Process events */
    	if ((comEvent & EVT_USB_MASKALL) && !COM_IsASCIIMode()) {
    		CmdUSB_Tx_Event(comEvent, COMUSB_TxBuf, &COMUSB_TxLen);
    		txEventUSB = true;
    	}

    	if ((comEvent & EVT_USB_EXP_ADATA) && COM_IsASCIIMode()) {
    		CmdUSB_Tx_Event(comEvent, COMUSB_TxBuf, &COMUSB_TxLen);
    		txEventUSB = true;
    	}

    	if ((comEvent & EVT_USB_EXP_ADATA_H) && COM_IsASCIIMode()) {
    		CmdUSB_Tx_Event(comEvent, COMUSB_TxBuf, &COMUSB_TxLen);
    		txEventUSB = true;
    	}

        comEvent = 0;

        if (txEventUSB) {
            txEventUSB = false;
            COMUSB_TxData(COMUSB_TxBuf, COMUSB_TxLen);
        }
    }
}

/* Communication Process - Events - TCM */
static void COM_EVTTCMTask(void *Args)
{
    uint32_t comEvent = 0;
    bool txEventTCM = false;

    /* Wait till Config notifies completion */
    uint32_t notifiedValue;
    xTaskNotifyWait(UINT_MIN, UINT_MAX, &notifiedValue, portMAX_DELAY);

    while(1) {
        /* set watchdog status to asleep */
        WD_Status(WD_EVTTCM, WD_ASLEEP);

        /* Wait for an event */
        xTaskNotifyWait(UINT_MIN, UINT_MAX, &comEvent, portMAX_DELAY);

        /* set watchdog status to alive */
        WD_Status(WD_EVTTCM, WD_ALIVE);

        /* Check for TCM COM events */
        if (comEvent & EVT_TCM_ALL) {
        	/* If Tx is not ready, retry */
        	if (TCMi_IsTxReady()) {
        		CmdTCM_Tx_Event(comEvent, COMTCM_TxBuf, &COMTCM_TxLen);
        		txEventTCM = true;
        	} else {
        		/* Schedule TCM com event again */
        		xTaskNotify(xComEvtTCMTaskHandle, comEvent, eSetBits);
            }
        }

        comEvent = 0;

        if (txEventTCM) {
            txEventTCM = false;
            if (COMTCM_TxLen > 0)
            	TCMi_Tx(COMTCM_TxBuf, COMTCM_TxLen);
            COMTCM_ResetTx();
        }
    }
}

/* Create communication task */
static void COMTasks_Create(void)
{
    /* Command processing task for USB */
    static StaticTask_t xComCmdUSBTaskTCB;
    static StackType_t uxComCmdUSBTaskStack[COMCMDUSBTASK_STACKSZ];

    xComCmdUSBTaskHandle = xTaskCreateStatic(COM_CMDUSBTask,
                                        COMCMDUSBTASK_NAME,
                                        COMCMDUSBTASK_STACKSZ,
                                        NULL,
                                        COMCMDUSBTASK_PRIO,
                                        uxComCmdUSBTaskStack,
                                        &xComCmdUSBTaskTCB);
    if (xComCmdUSBTaskHandle == NULL)
        Error_Handler(ERROR_TASK_CREATE);

    /* Command processing task for TCM */
    static StaticTask_t xComCmdTCMTaskTCB;
    static StackType_t uxComCmdTCMTaskStack[COMCMDTCMTASK_STACKSZ];

    xComCmdTCMTaskHandle = xTaskCreateStatic(COM_CMDTCMTask,
            COMCMDTCMTASK_NAME,
            COMCMDTCMTASK_STACKSZ,
            NULL,
            COMCMDTCMTASK_PRIO,
            uxComCmdTCMTaskStack,
            &xComCmdTCMTaskTCB);
    if (xComCmdTCMTaskHandle == NULL)
        Error_Handler(ERROR_TASK_CREATE);

    /* Data handling task */
    static StaticTask_t xComDataTaskTCB;
    static StackType_t uxComDataTaskStack[COMDATATASK_STACKSZ];

    xComDataTaskHandle = xTaskCreateStatic(COM_DATATask,
            COMDATATASK_NAME,
            COMDATATASK_STACKSZ,
            NULL,
            COMDATATASK_PRIO,
            uxComDataTaskStack,
            &xComDataTaskTCB);
    if (xComDataTaskHandle == NULL)
        Error_Handler(ERROR_TASK_CREATE);

    /* Event handling task - USB */
    static StaticTask_t xComEvtUSBTaskTCB;
    static StackType_t uxComEvtUSBTaskStack[COMEVTUSBTASK_STACKSZ];

    xComEvtUSBTaskHandle = xTaskCreateStatic(COM_EVTUSBTask,
            COMEVTUSBTASK_NAME,
            COMEVTUSBTASK_STACKSZ,
            NULL,
            COMEVTUSBTASK_PRIO,
            uxComEvtUSBTaskStack,
            &xComEvtUSBTaskTCB);
    if (xComEvtUSBTaskHandle == NULL)
        Error_Handler(ERROR_TASK_CREATE);

    /* Event handling task - TCM */
    static StaticTask_t xComEvtTCMTaskTCB;
    static StackType_t uxComEvtTCMTaskStack[COMEVTTCMTASK_STACKSZ];

    xComEvtTCMTaskHandle = xTaskCreateStatic(COM_EVTTCMTask,
            COMEVTTCMTASK_NAME,
            COMEVTTCMTASK_STACKSZ,
            NULL,
            COMEVTTCMTASK_PRIO,
            uxComEvtTCMTaskStack,
            &xComEvtTCMTaskTCB);
    if (xComEvtTCMTaskHandle == NULL)
        Error_Handler(ERROR_TASK_CREATE);
}

/* Public Functions */

/* Initialize */
bool COM_Init(void)
{
	StdReturn_t stdRet;

    /* Communication Task */
    COMTasks_Create();

    /* USB */
    stdRet = USBi_Start();
    if(stdRet != RET_OK)
       Error_Handler(ERROR_BOOTUP_USB);

    /* Start TCM */
    stdRet = TCMi_ComStart();
    if (stdRet != RET_OK)
    	Error_Handler(ERROR_COMSTART_TCMi);

    /* Start AxMs */
    AxMi_Init();

    return true;
}

/* Is ASCII mode */
uint32_t COM_IsASCIIMode(void)
{
	if (!Device_IsDFX())
		return CfgDev_Get_ASCIIMode();
	else
		return 0;
}

/******************************** End of File *********************************/
