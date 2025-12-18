/**
 **  @file USBDev.h
 **  @brief USB Device
 **  @author JZJ
 **
 **/

#ifndef _USBDEV_H_
#define _USBDEV_H_

/* Includes */

/* Macros */

/* Types */
/* USBDev config */
typedef struct {
    void (*TxCmpltCB) (void);
    void (*ReceiveCB) (uint8_t *Buf, uint32_t Len);
} USBDev_Config_t;

/* Function Prototypes */
/* Init */
StdReturn_t USBDev_Init(USBDev_Config_t *Config);
/* DeInit */
StdReturn_t USBDev_DeInit(void);
/* Start */
StdReturn_t USBDev_Start(void);
/* Stop */
StdReturn_t USBDev_Stop(void);
/* Send data */
StdReturn_t USBDev_Transmit(uint8_t *Data, uint32_t Size);


#endif /*** _USBDEV_H_ ***/
