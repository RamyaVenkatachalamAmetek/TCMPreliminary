/**
 * ST provided
 * Modified by jiss.joseph@ametek.com
 **/

/* Includes ------------------------------------------------------------------*/
#include "PAL.h"
#include "Error.h"

#include "usbd_def.h"
#include "usbd_core.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

PCD_HandleTypeDef hpcd;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
 *******************************************************************************/

void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{

}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{

}

/**
 * @brief  Setup stage callback
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
 * @brief  Data Out stage callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
 * @brief  Data In stage callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
 * @brief  SOF callback.
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief  Reset callback.
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
    /* Set Speed. */
    USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, USBD_SPEED_FULL);

    /* Reset Device. */
    USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief  Suspend callback.
 * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
    __HAL_PCD_GATE_PHYCLOCK(hpcd);
    /* Inform USB library that core enters in suspend Mode. */
    USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);

    /* Enter in STOP mode. */
    if (hpcd->Init.low_power_enable)
    {
        /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
        SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
}

/**
 * @brief  Resume callback.
 * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
    __HAL_PCD_UNGATE_PHYCLOCK(hpcd);

    if (hpcd->Init.low_power_enable)
    {
        /* Reset SLEEPDEEP bit of Cortex System Control Register. */
        SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
        /* REVISIT - JZJ - Do we need to reconfigure clocks ? */
        /* Currently low power mode is not enabled */
        /* SystemClock_Config(); */
    }

    USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief  ISOOUTIncomplete callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
 * @brief  ISOINIncomplete callback.
 * @param  hpcd: PCD handle
 * @param  epnum: Endpoint number
 * @retval None
 */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
 * @brief  Connect callback.
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
 * @brief  Disconnect callback.
 * @param  hpcd: PCD handle
 * @retval None
 */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
 *******************************************************************************/

/**
 * @brief  Initializes the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
    /* Set LL Driver parameters */
    hpcd.Instance = USB_OTG_FS;
    hpcd.Init.dev_endpoints = 6;
    hpcd.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd.Init.Sof_enable = DISABLE;
    hpcd.Init.low_power_enable = DISABLE;
    hpcd.Init.lpm_enable = DISABLE;
    hpcd.Init.battery_charging_enable = DISABLE;
    hpcd.Init.use_dedicated_ep1 = DISABLE;
    hpcd.Init.vbus_sensing_enable = DISABLE;

    /* Link The driver to the stack */
    hpcd.pData = pdev;
    pdev->pData = &hpcd;

    if(HAL_OK != HAL_PCD_Init(&hpcd))
        return USBD_FAIL;

    HAL_PCDEx_SetRxFiFo(&hpcd, 0x80);
    HAL_PCDEx_SetTxFiFo(&hpcd, 0, 0x40);
    HAL_PCDEx_SetTxFiFo(&hpcd, 1, 0x80);

    return USBD_OK;
}

/**
 * @brief  De-Initializes the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_DeInit(pdev->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Starts the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Start(pdev->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Stops the low level portion of the device driver.
 * @param  pdev: Device handle
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_Stop(pdev->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Opens an endpoint of the low level driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @param  ep_type: Endpoint type
 * @param  ep_mps: Endpoint max packet size
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Closes an endpoint of the low level driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Flushes an endpoint of the Low Level Driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Returns Stall condition.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval Stall (1: Yes, 0: No)
 */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;

    if((ep_addr & 0x80) == 0x80)
    {
        return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
    }
    else
    {
        return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
    }
}

/**
 * @brief  Assigns a USB address to the device.
 * @param  pdev: Device handle
 * @param  dev_addr: Device address
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Transmits data over an endpoint.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @param  pbuf: Pointer to data to be sent
 * @param  size: Data size
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Prepares an endpoint for reception.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @param  pbuf: Pointer to data to be received
 * @param  size: Data size
 * @retval USBD status
 */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBD_StatusTypeDef usb_status = USBD_OK;

    hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBD_OK;
            break;
        case HAL_ERROR :
            usb_status = USBD_FAIL;
            break;
        case HAL_BUSY :
            usb_status = USBD_BUSY;
            break;
        case HAL_TIMEOUT :
            usb_status = USBD_FAIL;
            break;
        default :
            usb_status = USBD_FAIL;
            break;
    }
    return usb_status;
}

/**
 * @brief  Returns the last transfered packet size.
 * @param  pdev: Device handle
 * @param  ep_addr: Endpoint number
 * @retval Recived Data Size
 */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

/**
 * @brief  Send LPM message to user layer
 * @param  hpcd: PCD handle
 * @param  msg: LPM message
 * @retval None
 */
void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg)
{
    switch (msg)
    {
        case PCD_LPM_L0_ACTIVE:
            if (hpcd->Init.low_power_enable)
            {
                /* REVISIT - JZJ - Do we need to reconfigure clocks ? */
                /* Currently low power mode is not enabled */
                /* SystemClock_Config(); */

                /* Reset SLEEPDEEP bit of Cortex System Control Register. */
                SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
            }
            __HAL_PCD_UNGATE_PHYCLOCK(hpcd);
            USBD_LL_Resume(hpcd->pData);
            break;

        case PCD_LPM_L1_ACTIVE:
            __HAL_PCD_GATE_PHYCLOCK(hpcd);
            USBD_LL_Suspend(hpcd->pData);

            /* Enter in STOP mode. */
            if (hpcd->Init.low_power_enable)
            {
                /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
                SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
            }
            break;
    }
}

/**
 * @brief  Delays routine for the USB Device Library.
 * @param  Delay: Delay in ms
 * @retval None
 */
void USBD_LL_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
