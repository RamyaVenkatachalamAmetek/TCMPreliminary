/**
 **  @file dI2C.h
 **  @brief I2C driver
 **  @author JZJ
 **
 **/


#ifndef _dI2C_H_
#define _dI2C_H_

/* Includes */
#include "PAL.h"

#include "stm32l4xx_hal_i2c.h"

/* Macros */

/* Types */

/* Function Prototypes */
/* Init */
StdReturn_t dI2C_Init(I2C_HandleTypeDef *hI2C);
/* DeInit */
StdReturn_t dI2C_DeInit(I2C_HandleTypeDef *hI2C);
/* Enable */
StdReturn_t dI2C_Enable(I2C_HandleTypeDef *hI2C);
/* Disable */
StdReturn_t dI2C_Disable(I2C_HandleTypeDef *hI2C);
/* Write register */
StdReturn_t dI2C_WrReg(I2C_HandleTypeDef *hI2C, uint8_t DevAddr, uint8_t RegAddr, uint8_t *Data, uint32_t Size);
/* Read register */
StdReturn_t dI2C_RdReg(I2C_HandleTypeDef *hI2C, uint8_t DevAddr, uint8_t RegAddr, uint8_t *Data, uint32_t Size);
/* Device ready */
StdReturn_t dI2C_DeviceReady(I2C_HandleTypeDef *hI2C, uint8_t DevAddr);

#endif /*** _dI2C_H_ ***/
 
