/**
 **  @file dI2C.c
 **  @brief I2C driver
 **  @author JZJ
 **
 **/


/* Includes */
#include "dI2C.h"

/* Macros */
#define dI2C_MAX_WAIT_TIME  (1000 * 1000) // 100 msec
#define TIMING_CLEAR_MASK   (0xF0FFFFFFU)  /* I2C TIMING clear register Mask */

/* Types */


/* Externs */

/* Function Declarations */

/* Global Variables */

/* Static Variables */


/* Private Functions */

/* Config transfer params */
static void dI2C_TransferConfig(I2C_HandleTypeDef *hI2C, uint8_t DevAddr, uint32_t Size, uint32_t Mode, uint32_t Request)
{
    /* Update CR2 */
    MODIFY_REG(hI2C->Instance->CR2, \
            ((I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | \
            (I2C_CR2_RD_WRN & (uint32_t)(Request >> (31U - I2C_CR2_RD_WRN_Pos))) | \
            I2C_CR2_START | I2C_CR2_STOP)), (uint32_t)(((uint32_t)DevAddr & I2C_CR2_SADD) | \
            (((uint32_t)Size << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES) | \
            (uint32_t)Mode | (uint32_t)Request));
}

/* Flush Tx data register */
static void dI2C_FlushTXDR(I2C_HandleTypeDef *hI2C)
{
    /* If a pending TXIS flag is set */
    /* Write a dummy data in TXDR to clear it */
    if(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_TXIS) != RESET)
        hI2C->Instance->TXDR = 0x00U;

    /* Flush TX register if not empty */
    if(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_TXE) == RESET)
        __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_TXE);
}

/* NACK detected */
static StdReturn_t dI2C_IsAckSuccess(I2C_HandleTypeDef *hI2C)
{
    HRTime_t tickStart = HRT_GetTick();

    if(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_AF) == SET)
    {
        /* Wait until STOP Flag is reset */
        /* AutoEnd should be initiate after AF */
        while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_STOPF) == RESET)
        {
            if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
                return RET_TIMEDOUT;
        }

        /* Clear NACKF Flag */
        __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_AF);
        /* Clear STOP Flag */
        __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
        /* Flush TX register */
        dI2C_FlushTXDR(hI2C);
        /* Clear Configuration Register 2 */
        I2C_RESET_CR2(hI2C);

        return RET_NOK;
    }
    return RET_OK;
}

/* Wait on Flag till Status */
static StdReturn_t dI2C_WaitOnFlag(I2C_HandleTypeDef *hI2C, uint32_t Flag, FlagStatus Status)
{
    HRTime_t tickStart = HRT_GetTick();

    while(__HAL_I2C_GET_FLAG(hI2C, Flag) == Status)
    {
        if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
            return RET_TIMEDOUT;
    }
    return RET_OK;
}

/* Wait for TXIS */
static StdReturn_t dI2C_WaitOnTXISFlag(I2C_HandleTypeDef *hI2C)
{
    HRTime_t tickStart = HRT_GetTick();

    while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_TXIS) == RESET)
    {
        /* Check if a NACK is detected */
        if(RET_OK != dI2C_IsAckSuccess(hI2C))
            return RET_NOK;

        if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
            return RET_TIMEDOUT;
    }
    return RET_OK;
}

/* Wait for TC */
static StdReturn_t dI2C_WaitOnTCFlag(I2C_HandleTypeDef *hI2C)
{
    HRTime_t tickStart = HRT_GetTick();

    while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_TC) == RESET)
    {
        /* Check if a NACK is detected */
        if(RET_OK != dI2C_IsAckSuccess(hI2C))
            return RET_NOK;

        if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
            return RET_TIMEDOUT;
    }
    return RET_OK;
}

/* Wait for TCR */
static StdReturn_t dI2C_WaitOnTCRFlag(I2C_HandleTypeDef *hI2C)
{
    HRTime_t tickStart = HRT_GetTick();

    while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_TCR) == RESET)
    {
        /* Check if a NACK is detected */
        if(RET_OK != dI2C_IsAckSuccess(hI2C))
            return RET_NOK;

        if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
            return RET_TIMEDOUT;
    }
    return RET_OK;
}

/* Wait for RXNE */
static StdReturn_t dI2C_WaitOnRXNEFlag(I2C_HandleTypeDef *hI2C)
{
    HRTime_t tickStart = HRT_GetTick();

    while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_RXNE) == RESET)
    {
        /* Check if a NACK is detected */
        if(RET_OK != dI2C_IsAckSuccess(hI2C))
            return RET_NOK;

        /* Check if a STOPF is detected */
        if(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_STOPF) == SET)
        {
            if(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_RXNE) == SET)
            {
                return RET_OK;
            }
            else
            {
                /* Clear STOP Flag */
                __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
                /* Clear Configuration Register 2 */
                I2C_RESET_CR2(hI2C);
                return RET_NOK;
            }
        }

        if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
            return RET_TIMEDOUT;
    }
    return RET_OK;
}

/* Wait for STOP */
static StdReturn_t dI2C_WaitOnSTOPFlag(I2C_HandleTypeDef *hI2C)
{
    HRTime_t tickStart = HRT_GetTick();

    while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_STOPF) == RESET)
    {
        /* Check if a NACK is detected */
        if(RET_OK != dI2C_IsAckSuccess(hI2C))
            return RET_NOK;

        if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
            return RET_TIMEDOUT;
    }
    return RET_OK;
}

/* Wait for ready */
static StdReturn_t dI2C_WaitForReady(I2C_HandleTypeDef *hI2C)
{
    uint8_t tmpData;

    /* Readout */
    while(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_RXNE) == SET)
        tmpData = (uint8_t)hI2C->Instance->RXDR;
    (void)tmpData;

    return dI2C_WaitOnFlag(hI2C, I2C_FLAG_BUSY, SET);
}

/* Public Functions */
/* Init */
StdReturn_t dI2C_Init(I2C_HandleTypeDef *hI2C)
{
    if(hI2C == NULL)
        return RET_ARGS_NOK;

    __HAL_I2C_DISABLE(hI2C);

    /* TIMINGR */
    hI2C->Instance->TIMINGR = hI2C->Init.Timing & TIMING_CLEAR_MASK;

    /* CR2 */
    hI2C->Instance->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

    /* CR1 */
    hI2C->Instance->CR1 = (hI2C->Init.GeneralCallMode | hI2C->Init.NoStretchMode);

    __HAL_I2C_ENABLE(hI2C);

    return RET_OK;
}

/* DeInit */
StdReturn_t dI2C_DeInit(I2C_HandleTypeDef *hI2C)
{
    if(hI2C == NULL)
        return RET_ARGS_NOK;

    __HAL_I2C_DISABLE(hI2C);

     return RET_OK;
}

/* Enable */
StdReturn_t dI2C_Enable(I2C_HandleTypeDef *hI2C)
{
    if(hI2C == NULL)
        return RET_ARGS_NOK;

    __HAL_I2C_ENABLE(hI2C);

    return RET_OK;
}

/* Disable */
StdReturn_t dI2C_Disable(I2C_HandleTypeDef *hI2C)
{
    if(hI2C == NULL)
        return RET_ARGS_NOK;

    __HAL_I2C_DISABLE(hI2C);

    return RET_OK;
}

/* Write register */
StdReturn_t dI2C_WrReg(I2C_HandleTypeDef *hI2C, uint8_t DevAddr, uint8_t RegAddr, uint8_t *Data, uint32_t Size)
{
    StdReturn_t stdRet;

    if(hI2C == NULL)
        return RET_ARGS_NOK;

    if(Size > 255)
        return RET_ARGS_NOK;

    stdRet = dI2C_WaitForReady(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Prepare Write */
    dI2C_TransferConfig(hI2C, DevAddr, 1, I2C_RELOAD_MODE, I2C_GENERATE_START_WRITE);

    /* Send RegAddr */
    stdRet = dI2C_WaitOnTXISFlag(hI2C);
    if(stdRet != RET_OK)
        return stdRet;
    hI2C->Instance->TXDR = RegAddr;
    stdRet = dI2C_WaitOnTCRFlag(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Write */
    uint8_t *pData = Data;
    uint32_t len = Size;

    dI2C_TransferConfig(hI2C, DevAddr, len, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);

    /* Write data to TXDR */
    while(len > 0)
    {
        stdRet = dI2C_WaitOnTXISFlag(hI2C);
        if(stdRet != RET_OK)
            return stdRet;

        hI2C->Instance->TXDR = *pData;
        pData++;
        len--;
    }

    /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
    stdRet = dI2C_WaitOnSTOPFlag(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Clear STOP Flag */
    __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(hI2C);

    return RET_OK;
}

/* Read register */
StdReturn_t dI2C_RdReg(I2C_HandleTypeDef *hI2C, uint8_t DevAddr, uint8_t RegAddr, uint8_t *Data, uint32_t Size)
{
    StdReturn_t stdRet;

    if(hI2C == NULL)
        return RET_ARGS_NOK;

    if(Size > 255)
        return RET_ARGS_NOK;

    stdRet = dI2C_WaitForReady(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Prepare Read */
    dI2C_TransferConfig(hI2C, DevAddr, 1, I2C_SOFTEND_MODE, I2C_GENERATE_START_WRITE);

    /* Send RegAddr */
    stdRet = dI2C_WaitOnTXISFlag(hI2C);
    if(stdRet != RET_OK)
        return stdRet;
    hI2C->Instance->TXDR = RegAddr;
    stdRet = dI2C_WaitOnTCFlag(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Read */
    uint8_t *pData = Data;
    uint32_t len = Size;

    dI2C_TransferConfig(hI2C, DevAddr, len, I2C_AUTOEND_MODE, I2C_GENERATE_START_READ);

    while(len > 0)
    {
        stdRet = dI2C_WaitOnRXNEFlag(hI2C);
        if(stdRet != RET_OK)
            return stdRet;

        *pData = (uint8_t)hI2C->Instance->RXDR;
        pData++;
        len--;
    }

    /* Wait until STOPF flag is set */
    stdRet = dI2C_WaitOnSTOPFlag(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Clear STOP Flag */
    __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(hI2C);

    return RET_OK;
}

/* Device ready */
StdReturn_t dI2C_DeviceReady(I2C_HandleTypeDef *hI2C, uint8_t DevAddr)
{
    StdReturn_t stdRet;
    uint32_t tickStart;
    uint32_t numTrails = 5;

    if(hI2C == NULL)
        return RET_ARGS_NOK;

    stdRet = dI2C_WaitForReady(hI2C);
    if(stdRet != RET_OK)
        return stdRet;

    while(numTrails > 0)
    {
        FlagStatus tmp1;
        FlagStatus tmp2;

        /* Generate Start */
        I2C_RESET_CR2(hI2C);
        hI2C->Instance->CR2 = I2C_GENERATE_START(hI2C->Init.AddressingMode, DevAddr);

        /* Wait until STOPF flag is set or a NACK flag is set*/
        tickStart = HRT_GetTick();

        tmp1 = __HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_STOPF);
        tmp2 = __HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_AF);

        while((tmp1 == RESET) && (tmp2 == RESET))
        {
            if((HRT_GetTick() - tickStart) > dI2C_MAX_WAIT_TIME)
                return RET_TIMEDOUT;

            tmp1 = __HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_STOPF);
            tmp2 = __HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_AF);
        }

        /* Check if the NACKF flag has not been set */
        if(__HAL_I2C_GET_FLAG(hI2C, I2C_FLAG_AF) == RESET)
        {
            /* Wait until STOPF flag is reset */
            stdRet = dI2C_WaitOnFlag(hI2C, I2C_FLAG_STOPF, RESET);
            if(stdRet != RET_OK)
                return stdRet;

            /* Clear STOP Flag */
            __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
            return RET_OK;
        }
        else
        {
            /* Wait until STOPF flag is reset */
            stdRet = dI2C_WaitOnFlag(hI2C, I2C_FLAG_STOPF, RESET);
            if(stdRet != RET_OK)
                return stdRet;

            /* Clear NACK Flag */
            __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_AF);
            /* Clear STOP Flag, auto generated with autoend*/
            __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
        }

        if(--numTrails == 0)
        {
            /* Generate Stop */
            hI2C->Instance->CR2 |= I2C_CR2_STOP;

            /* Wait until STOPF flag is reset */
            stdRet = dI2C_WaitOnFlag(hI2C, I2C_FLAG_STOPF, RESET);
            if(stdRet != RET_OK)
                return stdRet;

            /* Clear STOP Flag */
            __HAL_I2C_CLEAR_FLAG(hI2C, I2C_FLAG_STOPF);
        }
    }

    return RET_NOK;
}


/******************************** End of File *********************************/
