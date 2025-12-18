/**
 **  @file Power.c
 **  @brief Power management
 **  @author PS, JZJ
 **
 **/

/* Includes */
#include "Power.h"
#include "dI2C.h"
#include "Error.h"

/* Macros */

/* Board mapping */

/* PWR_EN_3V3 */
#define PWR_PWR_EN_3V3_PORT (GPIOD)
#define PWR_PWR_EN_3V3_PIN  (GPIO_PIN_9)
/* PWR_EN_2V8 */
#define PWR_PWR_EN_2V8_PORT (GPIOD)
#define PWR_PWR_EN_2V8_PIN  (GPIO_PIN_10)
/* PWR_MODE */
#define PWR_PWR_MODE_PORT   (GPIOD)
#define PWR_PWR_MODE_PIN    (GPIO_PIN_8)
/* /EXT_PWR */
#define PWR_nEXT_PWR_PORT   (GPIOE)
#define PWR_nEXT_PWR_PIN    (GPIO_PIN_6)

/* Use lookup table for battery profile */
#undef USE_BAT_PROF_TABLE

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */
I2C_HandleTypeDef hPwrI2C;



/* Get /EXT_PWR status */
static bool IsExtPowerPresent(void)
{
    if(GPIO_PIN_SET == PAL_GetIO(PWR_nEXT_PWR_PORT, PWR_nEXT_PWR_PIN))
        return false;
    else
        return true;
}

/* Public Functions */

/* Init - Stage1 */
StdReturn_t Power_InitStage1(Pwr_Status_t *Status)
{
    StdReturn_t stdRet;

    Power_DeInit(Status);

    /* PWR_MODE - LTC3621 in Pulse skipping mode */
    PAL_ResetIO(PWR_PWR_MODE_PORT, PWR_PWR_MODE_PIN);

    /* 3V3 */
    PAL_SetIO(PWR_PWR_EN_3V3_PORT, PWR_PWR_EN_3V3_PIN);
    HRT_Delay(100);


    /* Settle */
    HRT_Delay(100);
    
    /* I2C channel */
    __HAL_RCC_I2C3_CLK_ENABLE();

    hPwrI2C.Instance             = I2C3;
    /* Clock - 100KHz, Clock HIGH - 4us, Clock LOW - 5us,
     * Data Hold Time - 500ns, Data Setup Time - 1250ns  */
    hPwrI2C.Init.Timing          = 0x30420F13;//10KHz --> 0x3042C3C7;
    hPwrI2C.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hPwrI2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hPwrI2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hPwrI2C.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    stdRet = dI2C_Init(&hPwrI2C);
    if(stdRet != RET_OK)
        return stdRet;

    /* Settle */
    HRT_Delay(100);

    Status->LC_Present = 0;
    Status->BQ_Present = 0;

    return RET_OK;
}

/* Init - Stage2 */
StdReturn_t Power_InitStage2(Pwr_Status_t *Status)
{
    /* 2V8 */
    PAL_SetIO(PWR_PWR_EN_2V8_PORT, PWR_PWR_EN_2V8_PIN);
    HRT_Delay(100);

    HRT_Delay(100);
    return RET_OK;
}

/* DeInit */
StdReturn_t Power_DeInit(Pwr_Status_t *Status)
{
    /* 2V8 */
    PAL_ResetIO(PWR_PWR_EN_2V8_PORT, PWR_PWR_EN_2V8_PIN);

    /* 3V3 */
    PAL_ResetIO(PWR_PWR_EN_3V3_PORT, PWR_PWR_EN_3V3_PIN);

    HRT_Delay(100);
    return RET_OK;
}

/* Reset */
StdReturn_t Power_ResetDevice(void)
{
    /* Reset */
    HAL_NVIC_SystemReset();

    return RET_OK;
}

/* Power Down */
StdReturn_t Power_Down(void)
{
    /* 2V8 */
    PAL_ResetIO(PWR_PWR_EN_2V8_PORT, PWR_PWR_EN_2V8_PIN);
    HRT_Delay(100);

    /* 3V3 */
    PAL_ResetIO(PWR_PWR_EN_3V3_PORT, PWR_PWR_EN_3V3_PIN);
    HRT_Delay(100);

    return RET_OK;
}

/* Power Up */
StdReturn_t Power_Up(void)
{
    /* 3V3 */
    PAL_SetIO(PWR_PWR_EN_3V3_PORT, PWR_PWR_EN_3V3_PIN);
    HRT_Delay(100);

    /* 2V8 */
    PAL_SetIO(PWR_PWR_EN_2V8_PORT, PWR_PWR_EN_2V8_PIN);

    return RET_OK;
}




/******************************** End of File *********************************/
