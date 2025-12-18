/**
 **  @file USBi.h
 **  @brief USB Interface
 **  @author JZJ
 **
 **/

#ifndef _USBI_H_
#define _USBI_H_

/* Includes */

/* Macros */

/* Mode */
#define USBi_MODE_UNKNOWN		(0)
#define USBi_MODE_DEVICE    	(1)
#define USBi_MODE_HOST      	(2)

/* USB-C current level */
#define USBi_C_CURR_LVL_DEF_INA (0x11) // Unattached
#define USBi_C_CURR_LVL_DEF_ACT (0x10) // Attached
#define USBi_C_CURR_LVL_MEDIUM  (0x01) // Medium
#define USBi_C_CURR_LVL_HIGH    (0x00) // High

/* Types */
typedef struct {
    bool DFPAttached;
    bool VConnFault;
    uint8_t CurrLevel;
} USBi_Status_t;

/* Function Prototypes */
/* Start */
StdReturn_t USBi_Start(void);
/* Stop */
StdReturn_t USBi_Stop(void);
/* Enable */
StdReturn_t USBi_Enable(void);
/* Disable */
StdReturn_t USBi_Disable(void);
/* Check Tx status */
bool USBi_IsTxReady(void);
/* TX */
StdReturn_t USBi_Tx(uint8_t *Data, uint32_t Size);
/* Get status */
StdReturn_t USBi_GetStatus(USBi_Status_t *Status);
/* Get Mode */
uint8_t USBi_GetMode(void);
/* Set Mode */
StdReturn_t USBi_SetMode(uint8_t mode);
/* Process USB */
void USBi_Process(void);
/* INTR */
void USBi_ISR(void);

#endif /*** _USBI_H_ ***/
