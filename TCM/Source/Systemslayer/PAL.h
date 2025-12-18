/**
 **  @file PAL.h
 **  @brief Platform Abstraction Layer
 **  @author JZJ
 **
 **/

#ifndef _PAL_H_
#define _PAL_H_

/* Includes */
#include "stm32l4xx_hal.h"

#include "StdTypes.h"

#include "HRT.h"

/* Macros */

/* Types */

/* Function Prototypes */
/* Init Platform - Stage1 */
void PAL_InitStage1(void);
/* Init Platform - Stage2 */
void PAL_InitStage2(void);
/* Configure GPIO for shutdown */
void PAL_PwrDown(void);
/* Set Priority */
void PAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t Priority);
/* Get Priority */
uint32_t PAL_NVIC_GetPriority(IRQn_Type IRQn);
/* Enable Interrupt */
void PAL_NVIC_EnableIRQ(IRQn_Type IRQn);
/* Disable Interrupt */
void PAL_NVIC_DisableIRQ(IRQn_Type IRQn);
/* Read GPIO */
GPIO_PinState PAL_GetIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
/* Set GPIO */
void PAL_SetIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
/* Reset GPIO */
void PAL_ResetIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
/* Toggle GPIO */
void PAL_ToggleIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
/* Bootloader */
void PAL_ActivateBootloader(void);

#endif /*** _PAL_H_ ***/

