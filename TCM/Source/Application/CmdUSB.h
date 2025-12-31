/**
 **  @file CmdUSB.h
 **  @brief USB commands Module
 **  @author JZJ
 **
 **/

#ifndef _CMDUSB_H_
#define _CMDUSB_H_

/* Includes */

/* Macros */

/* Types */

/* Function Prototypes */

/* Process command - USB */
CmdStatus_t CmdUSB_Process(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen);

/* Transmit reading */
void CmdUSB_Tx_Reading(uint32_t Src, float32_t Reading, uint8_t *RspBuf, uint32_t *RspLen);
/* Tx ASCII reading */
void CmdUSB_Tx_ASCIIReading(uint32_t Src, float32_t Reading, uint8_t *RspBuf, uint32_t *RspLen);
/* Tx event */
void CmdUSB_Tx_Event(uint32_t Evt, uint8_t *RspBuf, uint32_t *RspLen);
/* Is streaming data in DF2 mode */
bool CmdUSB_IsDF2DataStreaming(void);
/* Set DF2 Normal reading as response */
void CmdUSB_SetDF2Reading(uint8_t *RspBuf, uint32_t *RspLen);

#endif /* _CMDUSB_H_ */
