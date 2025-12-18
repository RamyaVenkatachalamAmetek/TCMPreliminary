/**
 **  @file POST.c
 **  @brief Power On Self Tests
 **  @author JZJ
 **
 **/
 
/* Includes */
#include "POST.h"
#include "TestRam.h"

#include "System.h"
#include "Power.h"

/* Macros */

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */

/* Static Variables */

/* Private Functions */

/* Public Functions */
/* Run phase 1 tests */
StdReturn_t POST_Run1(void)
{
    StdReturn_t stdRet = RET_OK;
    uint32_t pwrKeyOnCounter = 0;
    uint32_t extPwrOnCounter = 0;
    uint32_t startTime = 0;

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    /* Check if system is coming out of a shutdown */
    if(0xA5A5A5A5 == RTC->BKP31R) {
        RTC->BKP31R = 0;

        startTime = HRT_GetTick();
        while(1) {

            HRT_Delay(1000);

            /* Check for PWR key */
            if(GPIO_PIN_RESET == PAL_GetIO(GPIOA, GPIO_PIN_0)) {
                pwrKeyOnCounter++;
            } else {
                if(pwrKeyOnCounter)
                    pwrKeyOnCounter--;
            }
            /* Check for EXT_PWR */
            if(GPIO_PIN_RESET == PAL_GetIO(GPIOE, GPIO_PIN_6)) {
                extPwrOnCounter++;
            } else {
                if(extPwrOnCounter)
                    extPwrOnCounter--;
            }

            if(HRT_IsTimedOut(startTime, (1000 * 1000))) {
                /* Shutdown if spurious signals */
                if((pwrKeyOnCounter < 900) && (extPwrOnCounter < 900))
                    Sys_Shutdown(false);
                /* Mark wakeup from EXT_PWR */
                if((pwrKeyOnCounter < 900) && (extPwrOnCounter >= 900))
                    Sys_SetWakeupOnExtPwr(true);
                else
                    Sys_SetWakeupOnExtPwr(false);
                break;
            }
        }
    }

    /* Ram test */
#if 0 // Do NOT run Ram test now
    stdRet = TestRam_Run();
    if (RET_OK != stdRet)
        return stdRet;
#endif
    return stdRet;
}

/* Run phase 2 tests */
StdReturn_t POST_Run2(void)
{
    return RET_OK;
}

/******************************** End of File *********************************/
