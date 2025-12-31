/**
 *  @file CmdUSB.c
 *  @brief USB commands Module
 *  @author JZJ
 *
 **/

/* Includes */
#include "PAL.h"
#include "Cmds.h"
#include "Tasks.h"
#include "Test.h"
#include "Language.h"

#include "MxA.h"
#include "TCMi.h"
#include "AxMi.h"
#include "Config.h"
#include "CfgDev.h"
#include "CfgMxA.h"

#include "DAQ.h"
#include "COM.h"
#include "System.h"
#include "Users.h"
#include "Device.h"
#include "Export.h"
#include "Import.h"
#include "TestCfg.h"
#include "SrcLoad.h"
#include "Update.h"
#include "DispHome.h"
#include "DispUpdate.h"

#include "CRC8OS.h"

#include "Disp.h"
#include "IO.h"
#include "SM.h"
#include "Utils.h"

#include "Error.h"
#include "ComASCII.h"
/* Macros */

/* Applicaion protocol version - 1.2 */
#define APP_PROTOCOL_VER_MAJOR (0x01)
#define APP_PROTOCOL_VER_MINOR (0x02)


/* Types */

/* Externs */
extern bool DF2DataCollection;
extern bool DF2DataStreaming;
extern bool DF3BurstModeEnabled;
/* Function Declarations */

/* Global Variables */

/* Static Variables */
uint8_t CmdDevAddr = DF3_DEVADDR1;

/* Private Functions */

/* Get CRC */
static inline uint8_t GetCRC(uint8_t *Buf, uint32_t Len)
{
    return CRC8OS_Calc(Buf, Len, CRC8OS_Init());
}

/* Get Addr */
static inline uint8_t GetAddr(void)
{
    return CmdDevAddr;
}

/* Set Addr */
static inline bool SetAddr(uint8_t Addr)
{
    if((Addr >= DEVADDR_MIN) && (Addr <= DEVADDR_MAX)) {
    	CmdDevAddr = Addr;
    	return true;
    }
    return false;
}

/* Check Addr */
static inline uint8_t CheckAddr(uint8_t Addr)
{
    if(Addr == DF3_DEVADDR1)
        return true;
    if(Addr == DF3_DEVADDR2)
        return true;
    if(Addr == DF3_DEVADDR_ASCII)
        return true;
    return false;
}

/* Get arguments */
static inline uint8_t GetArgUINT8(uint8_t *Buf)
{
    return *Buf;
}
static inline uint16_t GetArgUINT16(uint8_t *Buf)
{
    uint16_t arg;
    memcpy((void*)&arg, (void*)Buf, sizeof(uint16_t));
    return arg;
}
static inline uint32_t GetArgUINT32(uint8_t *Buf)
{
    uint32_t arg;
    memcpy((void*)&arg, (void*)Buf, sizeof(uint32_t));
    return arg;
}
static inline int32_t GetArgINT32(uint8_t *Buf)
{
    int32_t arg;
    memcpy((void*)&arg, (void*)Buf, sizeof(int32_t));
    return arg;
}
static inline float32_t GetArgFLT32(uint8_t *Buf)
{
    float32_t arg;
    memcpy((void*)&arg, (void*)Buf, sizeof(float32_t));
    return arg;
}

/* Set values */
static inline void SetValUINT32(uint32_t Val, uint8_t *Buf)
{
    memcpy((void*)Buf, (void*)&Val, sizeof(uint32_t));
}
static inline void SetValINT32(int32_t Val, uint8_t *Buf)
{
    memcpy((void*)Buf, (void*)&Val, sizeof(int32_t));
}
static inline void SetValFLT32(float32_t Val, uint8_t *Buf)
{
    memcpy((void*)Buf, (void*)&Val, sizeof(float32_t));
}

static inline void SetValSTR(char *Str, char *Buf, uint32_t size)
{
	memcpy(Buf, Str, size);
}

/* ACK */
static void ACK(uint8_t FuncCode, uint8_t *RspBuf, uint32_t *RspLen)
{
    RspBuf[0] = GetAddr();
    RspBuf[1] = FuncCode;
    RspBuf[2] = 0x01;
    RspBuf[3] = 0x00;
    *RspLen = 4;
}

/* NACK */
static void NACK(uint8_t FuncCode, uint8_t Exception, uint8_t *RspBuf, uint32_t *RspLen)
{
    RspBuf[0] = GetAddr();
    RspBuf[1] = FuncCode;
    RspBuf[2] = 0x02;
    RspBuf[3] = CMD_EXC_CMDS;
    RspBuf[4] = Exception;
    *RspLen = 5;
}

/* Fill response */
static void RESP(uint8_t FuncCode, uint8_t *Data, uint8_t DataLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pData = Data;
    RspBuf[0] = GetAddr();
    RspBuf[1] = FuncCode;
    RspBuf[2] = DataLen;
    for(uint8_t i = 0; i < DataLen; i++)
        RspBuf[3 + i] = *pData++;
    *RspLen = (3 + DataLen);
}

/* Translate data source */
static inline uint32_t GetDataSource(uint8_t SrcOffset)
{
	if (SrcOffset == 0x00)
		return SRC_LOAD_PRIM;
	if (SrcOffset == 0x01)
		return SRC_LOAD_AUX1;
	if (SrcOffset == 0x02)
		return SRC_LOAD_AUX2;

	return 0;
}

/** Commands **/

/* Get DF3 Application Protocol version */
static void CmdProc_AppVer(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t data[2];
    data[0] = APP_PROTOCOL_VER_MAJOR;
    data[1] = APP_PROTOCOL_VER_MINOR;
    RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    return;
}

/* Set DF3 Device Address */
static void CmdProc_DevAddr(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t addr = GetAddr();
        RESP(CMDBYTE_FUNCCODE, &addr, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t addr = GetArgUINT8(pCmdBuf);
        if(!SetAddr(addr))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set time for device */
static void CmdProc_SetTime(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint32_t argTime = GetArgUINT32(pCmdBuf);
    if(!Sys_SetTime(argTime))
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    else
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    return;
}

/* Export File */
static void CmdProc_ExportFile(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    if(dataLen > 63) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    char fname[64];
    memset(fname, 0, sizeof(fname));
    strncpy(fname, (char*)pCmdBuf, dataLen);

    uint32_t fileSize;
    if(!ExportFile_GetInfo(fname, &fileSize)) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
        return;
    }
    if(fileSize == 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_FACCESSERR, RspBuf, RspLen);
        return;
    }

    uint8_t fsize[4];
    SetValUINT32(fileSize, fsize);
    RESP(CMDBYTE_FUNCCODE, fsize, sizeof(fsize), RspBuf, RspLen);

    ExportFile_StartExport();
    return;
}

/* Import File */
static void CmdProc_ImportFile(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t dataLen = CMDBYTE_DATALEN;
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

	uint8_t argOpt = GetArgUINT8(pCmdBuf);
	if(argOpt == 0x01) { // Start import
		if(dataLen > 63) {
			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
			return;
		}
		pCmdBuf += 1;
		char fname[64];
		memset(fname, 0, sizeof(fname));
		strncpy(fname, (char*)pCmdBuf, (dataLen - 1));
		if(!ImportFile_CreateFile(fname)) {
			NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
			return;
		}
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}
	if(argOpt == 0x02) { // Write data
		pCmdBuf += 1;
		if(!ImportFile_WriteData(pCmdBuf, (dataLen - 1))) {
			NACK(CMDBYTE_FUNCCODE, CMD_RET_FACCESSERR, RspBuf, RspLen);
			return;
		}
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}
	if(argOpt == 0x03) { // Stop import
		if(!ImportFile_CloseFile()) {
			NACK(CMDBYTE_FUNCCODE, CMD_RET_FACCESSERR, RspBuf, RspLen);
			return;
		}
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}

	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Reboot the device */
static void CmdProc_Reset(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    if(Sys_SetReqReboot())
    	ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    else
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    return;
}

/* Go to low power mode */
static void CmdProc_Sleep(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    if(Sys_SetReqSleep())
    	ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    else
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    return;
}

/* Go to bootloader mode */
static void CmdProc_Boot(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    if(Sys_SetReqBootloader())
    	ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    else
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    return;
}

/* Set Default values for configuration */
static void CmdProc_Defaults(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    /* Only one default set for now */
    uint8_t argDef = GetArgUINT8(pCmdBuf);
    if(argDef != 0x00) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    if(!Cfg_RestoreDefaults())
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    else
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    return;
}

/* User access privileges */
static void CmdProc_UserAccess(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSC = GetArgUINT8(pCmdBuf);
    if(argSC == 0x01) { // Set
        if(dataLen != 6) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        pCmdBuf += 1;
        uint8_t argUser = GetArgUINT8(pCmdBuf);

        pCmdBuf += 1;
        char data[5];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 2));

        if(!Users_SetAccessPerms(argUser, data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argSC == 0x02) { // Clear
        Users_SetAccessPerms(USER_NORMAL, NULL);
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set/Reset access pin */
static void CmdProc_UserPass(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSR = GetArgUINT8(pCmdBuf);
    if(argSR == 0x01) { // Set
        if(dataLen != 10) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        pCmdBuf += 1;
        uint8_t argUser = GetArgUINT8(pCmdBuf);

        pCmdBuf += 1;
        char data1[5];
        memset(data1, 0, sizeof(data1));
        strncpy(data1, (char*)pCmdBuf, 4);

        pCmdBuf += 4;
        char data2[5];
        memset(data2, 0, sizeof(data2));
        strncpy(data2, (char*)pCmdBuf, 4);

        if(!Users_SetPin(argUser, data1, data2))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argSR == 0x02) { // Reset
        if(dataLen != 6) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        pCmdBuf += 1;
        uint8_t argUser = GetArgUINT8(pCmdBuf);

        pCmdBuf += 1;
        char data[5];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, 4);

        if(!Users_ResetPin(argUser, data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Calibration Mode */
static void CmdProc_CalMode(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSS = GetArgUINT8(pCmdBuf);
    if(argSS == 0x01) { // Start

        if(!Device_StartCalibration()) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_GENERROR, RspBuf, RspLen);
            return;
        }
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argSS == 0x02) { // Stop

        if(!Device_StopCalibration()) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_GENERROR, RspBuf, RspLen);
            return;
        }
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Error recovery */
static void CmdProc_Recovery(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

	uint8_t argSrc = GetArgUINT8(pCmdBuf);
	if(argSrc > 0) {
		NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
		return;
	}

	pCmdBuf += 1;
	uint8_t argMem = GetArgUINT8(pCmdBuf);
	if(argMem == 0x01) { // Format sensor memory
		CfgMxA_ReqFormat();
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}
	if(argMem == 0x02) { // Format device memory
		CfgDev_ReqFormat();
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}
	if(argMem == 0x03) { // Restore sensor config from backup
		CfgMxA_ReqRestore();
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Update AxMs */
static void CmdProc_UpdateAxM(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

	uint8_t argSrc = GetArgUINT8(pCmdBuf);
	if(argSrc == 0x01) { // AxM1
		Update_ReqUpdate(1);
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}
	if(argSrc == 0x02) { // AxM2
		Update_ReqUpdate(2);
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get actual (raw) measurements */
static void CmdProc_ReadRaw(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    uint8_t data[4];
    int32_t reading = MxA_ReadRaw();
    SetValINT32(reading, data);

    RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    return;
}

/* Get true measurements */
static void CmdProc_ReadTrue(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);

    uint8_t data[4];
    float32_t reading;

    if(argSrc == 0x00) {
    	/* Return NACK if overloaded */
    	if(MxA_CheckOverloads()) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    		return;
    	}
    	reading = MxA_ReadAdjusted();
    	SetValFLT32(reading, data);
    	RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x01) {
    	/* Return NACK if overloaded */
    	if(AxM1_CheckOverloads()) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    		return;
    	}
    	reading = AxM1_ReadAdjusted();
    	SetValFLT32(reading, data);
    	RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x02) {
    	/* Return NACK if overloaded */
    	if(AxM2_CheckOverloads()) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    		return;
    	}
    	reading = AxM2_ReadAdjusted();
    	SetValFLT32(reading, data);
    	RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Read uncalibrated measurement */
static void CmdProc_ReadUnCal(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    uint8_t data[4];
    float32_t reading = MxA_ReadMeasured();
    SetValFLT32(reading, data);

    RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    return;
}

/* Read mode measurement */
static void CmdProc_ReadMode(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    return;
}

/* Burst mode measurements */
static void CmdProc_ReadBurst(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    /* Ignore first parameter - Source */

    pCmdBuf += 1;
    uint8_t argSS = GetArgUINT8(pCmdBuf);
    if(argSS == 0x01) { // Start
        pCmdBuf += 1;
        uint32_t argPeriod = GetArgUINT32(pCmdBuf);

        if(!CfgDev_Set_DataComTime(argPeriod)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        if(!CfgDev_Set_DataComEnable(true)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argSS == 0x02) { // Stop
        if(!CfgDev_Set_DataComEnable(false))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Burst mode logging */
static void CmdProc_LogBurst(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argSS = GetArgUINT8(pCmdBuf);
    if(argSS == 0x01) { // Start
        pCmdBuf += 1;
        uint32_t argPeriod = GetArgUINT32(pCmdBuf);

        if(!CfgDev_Set_DataLogTime(argPeriod)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        if(!CfgDev_Set_DataLogEnable(true)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argSS == 0x02) { // Stop
        if(!CfgDev_Set_DataLogEnable(false))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Reset measurements, peak values, break values etc… */
static void CmdProc_Zero(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint32_t srcId = GetDataSource(GetArgUINT8(pCmdBuf));
    if (srcId == 0) {
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    /* Do Zero options if source is the configured one */
    if (srcId == CfgDev_Get_SrcLoad()) {
    	pCmdBuf += 1;
    	uint8_t argZeroOpt = GetArgUINT8(pCmdBuf);
    	if(argZeroOpt == 0x01) {
    		Test_Zero(ZERO_LOAD);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x02) {
    		Test_Zero(ZERO_EXTN);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x03) {
    		Test_Zero(ZERO_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x04) {
    		Test_Zero(ZERO_ALL);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x05) {
    		Test_Zero(ZERO_RESET);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x06) {
    		Test_Zero(ZERO_LOAD_EXTN);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x07) {
    		Test_Zero(ZERO_LOAD_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZeroOpt == 0x08) {
    		Test_Zero(ZERO_EXTN_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    } else {
    	/* Zero the source */
    	if (srcId == SRC_LOAD_PRIM) {
    		if (MxA_IsConnected()) {
    			Test_Zero(ZERO_SRC_PRIM);
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    			return;
    		} else {
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    			return;
    		}
    	}
    	if (srcId == SRC_LOAD_AUX1) {
    		if (AxM1_IsConnected()) {
    			Test_Zero(ZERO_SRC_AUX1);
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    			return;
    		} else {
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    			return;
    		}
    	}
    	if (srcId == SRC_LOAD_AUX2) {
    		if (AxM2_IsConnected()) {
    			Test_Zero(ZERO_SRC_AUX2);
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    			return;
    		} else {
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    			return;
    		}
    	}
    }
}

/* Scaling factor */
static void CmdProc_ScalingFactor(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
	return;
}

/* Scaling offset */
static void CmdProc_ScalingOffset(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
	return;
}

/* Get device identifier */
static void CmdProc_Idn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

	char data[128];
	memset(data, 0, sizeof(data));

    uint8_t argSrc = GetArgUINT8(pCmdBuf);

    if(argSrc == 0x00) {
    	snprintf(data, (sizeof(data) - 1), "%s %s %s", SrcLoad_GetModel(SRC_LOAD_PRIM),
    			SrcLoad_GetSerial(SRC_LOAD_PRIM), CfgMxA_Get_CalUnits());
    	RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x01) {
    	if (!AxM1_InBootloader()) {
    		snprintf(data, (sizeof(data) - 1), "%s %s %s", SrcLoad_GetModel(SRC_LOAD_AUX1),
    				SrcLoad_GetSerial(SRC_LOAD_AUX1), CfgAxM1_Get_CalUnits());
    	} else {
    		strcpy(data, "AXM-CBL 12345 lbf");
    	}
    	RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x02) {
    	if (!AxM2_InBootloader()) {
    		snprintf(data, (sizeof(data) - 1), "%s %s %s", SrcLoad_GetModel(SRC_LOAD_AUX2),
    				SrcLoad_GetSerial(SRC_LOAD_AUX2), CfgAxM2_Get_CalUnits());
    	} else {
    		strcpy(data, "AXM-CBL 12345 lbf");
    	}
    	RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get firmware version */
static void CmdProc_VerFw(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);

    if(argSrc == 0x00) {
    	pCmdBuf += 1;
    	uint8_t argGV = GetArgUINT8(pCmdBuf);
    	if(argGV == 0x01) { // Firmware Version
    		char data[64];
    		memset(data, 0, sizeof(data));
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetFWVer(SRC_LOAD_PRIM));

    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGV == 0x02) { // Build Version
    		char data[64];
    		memset(data, 0, sizeof(data));
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetFWBuildVer(SRC_LOAD_PRIM));

    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    }

    if(argSrc == 0x01) {
    	pCmdBuf += 1;
    	uint8_t argGV = GetArgUINT8(pCmdBuf);
    	if(argGV == 0x01) { // Firmware Version
    		char data[64];
    		memset(data, 0, sizeof(data));
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetFWVer(SRC_LOAD_AUX1));

    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGV == 0x02) { // Build Version
    		char data[64];
    		memset(data, 0, sizeof(data));
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetFWBuildVer(SRC_LOAD_AUX1));

    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    }

    if(argSrc == 0x02) {
    	pCmdBuf += 1;
    	uint8_t argGV = GetArgUINT8(pCmdBuf);
    	if(argGV == 0x01) { // Firmware Version
    		char data[64];
    		memset(data, 0, sizeof(data));
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetFWVer(SRC_LOAD_AUX2));

    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGV == 0x02) { // Build Version
    		char data[64];
    		memset(data, 0, sizeof(data));
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetFWBuildVer(SRC_LOAD_AUX2));

    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get hardware version */
static void CmdProc_VerHw(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        char data[8];
        memset(data, 0, sizeof(data));
        snprintf(data, (sizeof(data) - 1), "%s", CfgDev_Get_HwVer());

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        char data[8];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 2));

        if(!CfgDev_Set_HwVer(data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set language */
static void CmdProc_Language(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        char data[16];
        memset(data, 0, sizeof(data));
        snprintf(data, (sizeof(data) - 1), "%s", Lang_GetStrFromIdx(CfgDev_Get_Lang()));

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        char data[16];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 1));

        if(!Lang_SetCurrLang(data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set/Get reading separator */
static void CmdProc_LocaleSep(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t sep;
        bool sepIsPnt = CfgDev_Get_SepIsPnt();
        if(sepIsPnt)
            sep = 0x01;
        else
            sep = 0x00;
        RESP(CMDBYTE_FUNCCODE, &sep, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        bool sepIsPnt;
        uint8_t argSep = GetArgUINT8(pCmdBuf);
        if(argSep == 0x00)
        	sepIsPnt = false;
        else
        	sepIsPnt = true;
        if(!CfgDev_Set_SepIsPnt(sepIsPnt))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set device model */
static void CmdProc_Model(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    char data[16];
    memset(data, 0, sizeof(data));

    uint8_t argSrc = GetArgUINT8(pCmdBuf);

    if(argSrc == 0x00) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetModel(SRC_LOAD_PRIM));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		pCmdBuf += 1;
    		strncpy(data, (char*)pCmdBuf, (dataLen - 2));
    		if(!SrcLoad_SetModel(SRC_LOAD_PRIM, data))
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    		else
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x01) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetModel(SRC_LOAD_AUX1));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x02) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetModel(SRC_LOAD_AUX2));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set serial number */
static void CmdProc_Serial(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    char data[16];
    memset(data, 0, sizeof(data));

    uint8_t argSrc = GetArgUINT8(pCmdBuf);

    if(argSrc == 0x00) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetSerial(SRC_LOAD_PRIM));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		pCmdBuf += 1;
    		strncpy(data, (char*)pCmdBuf, (dataLen - 2));
    		if(!SrcLoad_SetSerial(SRC_LOAD_PRIM, data))
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    		else
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x01) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetSerial(SRC_LOAD_AUX1));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x02) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", SrcLoad_GetSerial(SRC_LOAD_AUX2));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set device capacity */
static void CmdProc_Capacity(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        float32_t capacity = CfgMxA_Get_Range();
        uint8_t data[4];
        SetValFLT32(capacity, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        float32_t argCapacity = GetArgFLT32(pCmdBuf);

        if(!CfgMxA_Set_Range(argCapacity))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set resolution */
static void CmdProc_Resolution(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGSD = GetArgUINT8(pCmdBuf);
    if(argGSD == CMD_GET) {
        uint8_t resolution = (uint8_t) SrcLoad_GetConfResolution();
        RESP(CMDBYTE_FUNCCODE, &resolution, 1, RspBuf, RspLen);
        return;
    }
    if(argGSD == CMD_SET) {
        if(dataLen != 3) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }

        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        if(!SrcLoad_SetLoadResolution((uint32_t) argRes))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argGSD == CMD_DEFAULT) {
        CfgDev_Set_UseDefRes(true);
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set units */
static void CmdProc_Units(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    char data[8];
    memset(data, 0, sizeof(data));

    uint8_t argSrc = GetArgUINT8(pCmdBuf);

    if(argSrc == 0x00) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", Device_GetStrForceUnits(CfgDev_Get_UnitsForce(), false));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		pCmdBuf += 1;
    		strncpy(data, (char*)pCmdBuf, (dataLen - 2));
    		if(!CfgDev_Set_UnitsForce(Device_GetIdxForceUnits(data)))
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    		else
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x01) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", Device_GetStrForceUnits(CfgDev_Get_UnitsForce(), false));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		pCmdBuf += 1;
    		strncpy(data, (char*)pCmdBuf, (dataLen - 2));
    		if(!CfgDev_Set_UnitsForce(Device_GetIdxForceUnits(data)))
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    		else
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    if(argSrc == 0x02) {
    	pCmdBuf += 1;
    	uint8_t argGS = GetArgUINT8(pCmdBuf);
    	if(argGS == CMD_GET) {
    		snprintf(data, (sizeof(data) - 1), "%s", Device_GetStrForceUnits(CfgDev_Get_UnitsForce(), false));
    		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
    		return;
    	}
    	if(argGS == CMD_SET) {
    		pCmdBuf += 1;
    		strncpy(data, (char*)pCmdBuf, (dataLen - 2));
    		if(!CfgDev_Set_UnitsForce(Device_GetIdxForceUnits(data)))
    			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    		else
    			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set available units */
static void CmdProc_AvlUnits(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        char data[128];
        memset(data, 0, sizeof(data));
        snprintf(data, (sizeof(data) - 1), "%s", CfgMxA_Get_AvlUnits());

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        char data[128];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 2));

        if(!CfgMxA_Set_AvlUnits(data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set capacity units */
static void CmdProc_CapUnits(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        char data[8];
        memset(data, 0, sizeof(data));
        snprintf(data, (sizeof(data) - 1), "%s", CfgMxA_Get_RangeUnits());

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        char data[8];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 2));

        if(!CfgMxA_Set_RangeUnits(data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set number of calibration points */
static void CmdProc_CalNumPnts(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t numCalPnts = (uint8_t) CfgMxA_Get_CalNumPts();
        RESP(CMDBYTE_FUNCCODE, &numCalPnts, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argNumCalPnts = GetArgUINT8(pCmdBuf);
        if(!CfgMxA_Set_CalNumPts((uint32_t) argNumCalPnts))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set a calibration point */
static void CmdProc_CalPnt(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argCalPnt = GetArgUINT8(pCmdBuf);
        float32_t True;
        float32_t Meas;
        if(!CfgMxA_Get_CalPt((uint32_t)argCalPnt, &True, &Meas)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        uint8_t data[9];
        data[0] = argCalPnt;
        SetValFLT32(True, &data[1]);
        SetValFLT32(Meas, &data[5]);
        RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argCalPnt = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        float32_t argTrue = GetArgFLT32(pCmdBuf);
        pCmdBuf += 4;
        float32_t argMeas = GetArgFLT32(pCmdBuf);
        if(!CfgMxA_Set_CalPt((uint32_t)argCalPnt, argTrue, argMeas))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set last calibration date */
static void CmdProc_CalDate(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint32_t calTime = CfgMxA_Get_CalTime();
        RESP(CMDBYTE_FUNCCODE, (uint8_t*)&calTime, 4, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint32_t argCalTime = GetArgUINT32(pCmdBuf);
        if(!CfgMxA_Set_CalTime(argCalTime))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set calibration due date */
static void CmdProc_CalDue(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint32_t calDue = CfgMxA_Get_CalDue();
        RESP(CMDBYTE_FUNCCODE, (uint8_t*)&calDue, 4, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint32_t argCalDue = GetArgUINT32(pCmdBuf);
        if(!CfgMxA_Set_CalDue(argCalDue))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set number of due days for calibration warning */
static void CmdProc_CalWarn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint32_t calWarn = CfgMxA_Get_CalWarn();
        RESP(CMDBYTE_FUNCCODE, (uint8_t*)&calWarn, 4, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint32_t argCalWarn = GetArgUINT32(pCmdBuf);
        if(!CfgMxA_Set_CalWarn(argCalWarn))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set units used for calibration */
static void CmdProc_CalUnits(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        char data[8];
        memset(data, 0, sizeof(data));
        snprintf(data, (sizeof(data) - 1), "%s", CfgMxA_Get_CalUnits());

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        char data[8];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 1));

        if(!CfgMxA_Set_CalUnits(data))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set number of overloads */
static void CmdProc_NumOverloads(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint32_t numOverloads = CfgMxA_Get_NumOverloads();
        RESP(CMDBYTE_FUNCCODE, (uint8_t*)&numOverloads, 4, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint32_t argNumOverloads = GetArgUINT32(pCmdBuf);
        if(argNumOverloads == 0) {
            CfgMxA_Clear_Overloads();
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get an overload record */
static void CmdProc_OverloadRecord(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argOffset = GetArgUINT8(pCmdBuf);
        float32_t OvVal;
        uint32_t OvTStamp;
        if(!CfgMxA_Get_Overload((uint32_t)argOffset, &OvVal, &OvTStamp)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        uint8_t data[9];
        data[0] = argOffset;
        SetValFLT32(OvVal, &data[1]);
        SetValUINT32(OvTStamp, &data[5]);
        RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set gauge polarity */
static void CmdProc_Polarity(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t gaugePolarity;
        bool polarity = CfgDev_Get_Polarity();
        if(polarity)
            gaugePolarity = 0x01;
        else
            gaugePolarity = 0x00;
        RESP(CMDBYTE_FUNCCODE, &gaugePolarity, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        bool polarity;
        uint8_t argPolarity = GetArgUINT8(pCmdBuf);
        if(argPolarity == 0x00)
            polarity = false;
        else
            polarity = true;
        if(!CfgDev_Set_Polarity(polarity))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get connected sources */
static void CmdProc_ConnectedSources(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t data[4];

    if(MxA_IsConnected())
    	data[0] = 0x01;
    else
    	data[0] = 0x00;

    if(AxM1_IsConnected())
    	data[1] = 0x01;
    else
    	data[1] = 0x00;

    if(AxM2_IsConnected())
    	data[2] = 0x01;
    else
    	data[2] = 0x00;

    if(TCMi_IsConnected())
        data[3] = 0x01;
    else
        data[3] = 0x00;

    RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
    return;
}

/* Get/Set user defined units */
static void CmdProc_UduUnits(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t dataLen = CMDBYTE_DATALEN;
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    uint8_t argGSFT = GetArgUINT8(pCmdBuf+1);
    if(argGS == CMD_GET) {
        char data[8];
        memset(data, 0, sizeof(data));

        if(argGSFT == 0x11)
        	snprintf(data, (sizeof(data) - 1), "%s", CfgDev_GetForce_UDUnits());
        if(argGSFT == 0x22)
			snprintf(data, (sizeof(data) - 1), "%s", CfgDev_GetTorque_UDUnits());

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, strlen(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 2;
        char data[8];
        memset(data, 0, sizeof(data));
        strncpy(data, (char*)pCmdBuf, (dataLen - 1));

        if(argGSFT == 0x11)
        {
			if(!CfgDev_SetForce_UDUnits(data))
				NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
			else
				ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
			return;
        }
        if(argGSFT == 0x22)
		{
			if(!CfgDev_SetTorque_UDUnits(data))
				NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
			else
				ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
			return;
		}
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set conversion factor for user defined units */
static void CmdProc_UduConv(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    uint8_t argGSFT = GetArgUINT8(pCmdBuf+1);
    if(argGS == CMD_GET) {
        float32_t uduConv = 0;

        if(argGSFT == 0x11)
        	uduConv = CfgDev_GetForce_UDUConv();
        if(argGSFT == 0x22)
        	uduConv = CfgDev_GetTorque_UDUConv();

        uint8_t data[4];
        SetValFLT32(uduConv, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 2;
        float32_t argConv = GetArgFLT32(pCmdBuf);

        if(argGSFT == 0x11)
        {
			if(!CfgDev_SetForce_UDUConv(argConv))
				NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
			else
				ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
			return;
        }
        if(argGSFT == 0x22)
		{
			if(!CfgDev_SetTorque_UDUConv(argConv))
				NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
			else
				ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
			return;
		}
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}


/* Enable/Disable a feature */
static void CmdProc_FeatureEn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argFeature = GetArgUINT8(pCmdBuf);
    // Buzzer
    if(argFeature == 0x01) {
        pCmdBuf += 1;
        uint8_t argGS = GetArgUINT8(pCmdBuf);
        if(argGS == CMD_GET) {
            uint8_t buzzerOn = (uint8_t) CfgDev_Get_BuzzerOn();
            RESP(CMDBYTE_FUNCCODE, &buzzerOn, 1, RspBuf, RspLen);
            return;
        }
        if(argGS == CMD_SET) {
            pCmdBuf += 1;
            uint8_t argOnOff = GetArgUINT8(pCmdBuf);
            if(!CfgDev_Set_BuzzerOn((bool)argOnOff))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }
    // Hide Measurements
    if(argFeature == 0x02) {
        pCmdBuf += 1;
        uint8_t argGS = GetArgUINT8(pCmdBuf);
        if(argGS == CMD_GET) {
            uint8_t hideMeas = (uint8_t) CfgDev_Get_HideMeas();
            RESP(CMDBYTE_FUNCCODE, &hideMeas, 1, RspBuf, RspLen);
            return;
        }
        if(argGS == CMD_SET) {
            pCmdBuf += 1;
            uint8_t argOnOff = GetArgUINT8(pCmdBuf);
            if(!CfgDev_Set_HideMeas((bool)argOnOff))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }
    // Lock Home
    if(argFeature == 0x03) {
        pCmdBuf += 1;
        uint8_t argGS = GetArgUINT8(pCmdBuf);
        if(argGS == CMD_GET) {
            uint8_t lockHome = (uint8_t) CfgDev_Get_LockHome();
            RESP(CMDBYTE_FUNCCODE, &lockHome, 1, RspBuf, RspLen);
            return;
        }
        if(argGS == CMD_SET) {
            pCmdBuf += 1;
            uint8_t argOnOff = GetArgUINT8(pCmdBuf);
            if(!CfgDev_Set_LockHome((bool)argOnOff))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set/Get display mode */
static void CmdProc_DispMode(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t mode;
        bool dispNorm = CfgDev_Get_DispNorm();
        if(dispNorm)
            mode = 0x00;
        else
            mode = 0x01;
        RESP(CMDBYTE_FUNCCODE, &mode, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        bool dispNorm;
        uint8_t argMode = GetArgUINT8(pCmdBuf);
        if(argMode == 0x00)
            dispNorm = true;
        else
            dispNorm = false;
        if(!CfgDev_Set_DispNorm(dispNorm))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set/Get display brightness */
static void CmdProc_DispBrightness(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t brightness = (uint8_t) CfgDev_Get_Brightness();
        RESP(CMDBYTE_FUNCCODE, &brightness, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argBrightness = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_Brightness((uint32_t) argBrightness))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set/Get auto shutdown feature */
static void CmdProc_AutoShutDown(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t data[5];
        data[0] = (uint8_t) CfgDev_Get_AutoShutOn();
        SetValUINT32(CfgDev_Get_AutoShutTime(), &data[1]);
        RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        uint32_t argTime = GetArgUINT32(pCmdBuf);
        if(!CfgDev_Set_AutoShutOn((bool) argOn)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        if(!CfgDev_Set_AutoShutTime(argTime)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set/Get auto dimming feature */
static void CmdProc_AutoDimming(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t data[5];
        data[0] = (uint8_t) CfgDev_Get_AutoDimOn();
        SetValUINT32(CfgDev_Get_AutoDimTime(), &data[1]);
        RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        uint32_t argTime = GetArgUINT32(pCmdBuf);
        if(!CfgDev_Set_AutoDimOn((bool) argOn)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        if(!CfgDev_Set_AutoDimTime(argTime)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Save changes to configuration */
static void CmdProc_SaveCfg(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    CfgDev_SetDirty();
    CfgMxA_SetDirty();
    ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    return;
}

/* Filter option */
static void CmdProc_FilterOption(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argSrc = GetArgUINT8(pCmdBuf);
    if(argSrc > 0) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    pCmdBuf += 1;
    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argType = GetArgUINT8(pCmdBuf);
        if(argType == 0x00) { // reading filter
            uint8_t option = (uint8_t) CfgDev_Get_RdgFilter();
            RESP(CMDBYTE_FUNCCODE, &option, 1, RspBuf, RspLen);
            return;
        }
        if(argType == 0x01) { // peak filter
            uint8_t option = (uint8_t) CfgDev_Get_PeakFilter();
            RESP(CMDBYTE_FUNCCODE, &option, 1, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argType = GetArgUINT8(pCmdBuf);
        if(argType == 0x00) { // reading filter
            pCmdBuf += 1;
            uint8_t argOpt = GetArgUINT8(pCmdBuf);
            if(!CfgDev_Set_RdgFilter((uint32_t) argOpt)) {
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
                return;
            }
            MxA_StopSampling();
            MxA_ResetReadingFilter();
            MxA_StartSampling();
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        if(argType == 0x01) { // peak filter
            pCmdBuf += 1;
            uint8_t argOpt = GetArgUINT8(pCmdBuf);
            if(!CfgDev_Set_PeakFilter((uint32_t) argOpt)) {
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
                return;
            }
            Test_StopTask();
            Test_ResetPeakFilter();
            Test_StartTask();
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* ZeroOnStart */
static void CmdProc_ZeroOnStart(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
    	uint8_t option = (uint8_t) CfgDev_Get_ZeroOnStart();
    	RESP(CMDBYTE_FUNCCODE, &option, 1, RspBuf, RspLen);
    	return;
    }
    if(argGS == CMD_SET) {
    	pCmdBuf += 1;
    	uint8_t argZOS = GetArgUINT8(pCmdBuf);
    	if(argZOS == 0x00) {
    		CfgDev_Set_ZeroOnStart(ZERO_NONE);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x01) {
    		CfgDev_Set_ZeroOnStart(ZERO_LOAD);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x02) {
    		CfgDev_Set_ZeroOnStart(ZERO_EXTN);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x04) {
    		CfgDev_Set_ZeroOnStart(ZERO_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x07) {
    		CfgDev_Set_ZeroOnStart(ZERO_ALL);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
      	if(argZOS == 0x03) {
      		CfgDev_Set_ZeroOnStart(ZERO_LOAD_EXTN);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x05) {
    		CfgDev_Set_ZeroOnStart(ZERO_LOAD_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x06) {
    		CfgDev_Set_ZeroOnStart(ZERO_EXTN_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* ZeroDefinition */
static void CmdProc_ZeroDef(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
    	uint8_t option = (uint8_t) CfgDev_Get_ZeroDef();
    	RESP(CMDBYTE_FUNCCODE, &option, 1, RspBuf, RspLen);
    	return;
    }
    if(argGS == CMD_SET) {
    	pCmdBuf += 1;
    	uint8_t argZOS = GetArgUINT8(pCmdBuf);
    	if(argZOS == 0x00) {
    		CfgDev_Set_ZeroDef(ZERO_NONE);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x01) {
    		CfgDev_Set_ZeroDef(ZERO_LOAD);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x02) {
    		CfgDev_Set_ZeroDef(ZERO_EXTN);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x04) {
    		CfgDev_Set_ZeroDef(ZERO_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x07) {
    		CfgDev_Set_ZeroDef(ZERO_ALL);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
      	if(argZOS == 0x03) {
      		CfgDev_Set_ZeroDef(ZERO_LOAD_EXTN);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x05) {
    		CfgDev_Set_ZeroDef(ZERO_LOAD_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	if(argZOS == 0x06) {
    		CfgDev_Set_ZeroDef(ZERO_EXTN_RESULTS);
    		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    		return;
    	}
    	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    	return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Set test source */
static void CmdProc_TestSource(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argLE = GetArgUINT8(pCmdBuf);
        if(argLE == 0x01) { // Load
            uint8_t data;
            data = (uint8_t) CfgDev_Get_SrcLoad();
            data -= 0x01; // translate
            RESP(CMDBYTE_FUNCCODE, &data, sizeof(data), RspBuf, RspLen);
            return;
        }
        if(argLE == 0x02) { // Extn
            uint8_t data = 0x00; // only TCM
            RESP(CMDBYTE_FUNCCODE, &data, sizeof(data), RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argLE = GetArgUINT8(pCmdBuf);
        if(argLE == 0x01) { // Load
            pCmdBuf += 1;
            uint8_t argSource = GetArgUINT8(pCmdBuf);
            if(argSource >= 0x03) {
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
                return;
            }
            if(argSource == 0x00)
                CfgDev_Set_SrcLoad(SRC_LOAD_PRIM);
            if(argSource == 0x01)
                CfgDev_Set_SrcLoad(SRC_LOAD_AUX1);
            if(argSource == 0x02)
                CfgDev_Set_SrcLoad(SRC_LOAD_AUX2);
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        if(argLE == 0x02) { // Extn
            /* Not used now - Only TCM */
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
    return;
}

/* Start a test */
static void CmdProc_TestStart(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    if (Device_IsDFX()) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
        return;
    }

    if(Test_CheckErrorConditions()) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
        return;
    }

    if(Test_Start()) {
        Test_SetRunByHost(HOST_FT_USB);
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
    } else {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    }
    return;
}

/* Stop running test */
static void CmdProc_TestStop(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    if (Device_IsDFX()) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
        return;
    }

    if(Test_IsRunByHost()) {
        if(Test_Stop())
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        else
            NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    } else {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
    }
    return;
}

/* Get/Set test result */
static void CmdProc_TestResult(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        float32_t resVal = Test_GetResult((uint32_t) argRes);
        uint8_t data[4];
        SetValFLT32(resVal, data);
        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Use gauge parameters */
static void CmdProc_UseGaugeParams(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argYN = GetArgUINT8(pCmdBuf);
    if(argYN == 0x00) { // Use parameters set in gauge
        Test_SetUseHostCfg(false);
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }
    if(argYN == 0x01) { // Do NOT use parameters set in gauge
        Test_SetUseHostCfg(true);
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set test limit enable */
static void CmdProc_TestLmtEn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        uint8_t data[2];
        data[0] = (uint8_t) CfgDev_Get_ResCfg_Limits((uint32_t) argRes);
        data[1] = (uint8_t) CfgDev_Get_ResCfg_LmtMethod((uint32_t) argRes);
        RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        uint32_t argMethod = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_ResCfg_Limits((bool) argOn, (uint32_t) argRes)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        if(!CfgDev_Set_ResCfg_LmtMethod(argMethod, (uint32_t) argRes)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set test limit HSP */
static void CmdProc_TestLmtHSP(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        float32_t setPoint = CfgDev_Get_ResCfg_LmtHighSP((uint32_t) argRes);
        uint8_t data[4];
        SetValFLT32(setPoint, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        float32_t argSP = GetArgFLT32(pCmdBuf);
        if(!CfgDev_Set_ResCfg_LmtHighSP(argSP, (uint32_t) argRes))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set test limit LSP */
static void CmdProc_TestLmtLSP(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        float32_t setPoint = CfgDev_Get_ResCfg_LmtLowSP((uint32_t) argRes);
        uint8_t data[4];
        SetValFLT32(setPoint, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        float32_t argSP = GetArgFLT32(pCmdBuf);

        if(!CfgDev_Set_ResCfg_LmtLowSP(argSP, (uint32_t) argRes))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set test limit NSP */
static void CmdProc_TestLmtNSP(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        float32_t setPoint = CfgDev_Get_ResCfg_LmtNomSP((uint32_t) argRes);
        uint8_t data[4];
        SetValFLT32(setPoint, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        float32_t argSP = GetArgFLT32(pCmdBuf);

        if(!CfgDev_Set_ResCfg_LmtNomSP(argSP, (uint32_t) argRes))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set test limit BW */
static void CmdProc_TestLmtBW(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        uint8_t bandwidth = (uint8_t) CfgDev_Get_ResCfg_LmtNomBW((uint32_t) argRes);
        RESP(CMDBYTE_FUNCCODE, &bandwidth, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argRes = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        uint8_t argBandwidth = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_ResCfg_LmtNomBW((uint32_t) argBandwidth, (uint32_t) argRes))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set load limit enable */
static void CmdProc_LoadLmtEn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t on = (uint8_t) CfgDev_Get_LoadLmtEnable();
        RESP(CMDBYTE_FUNCCODE, &on, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_LoadLmtEnable((bool) argOn))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set load limit TSP */
static void CmdProc_LoadLmtTSP(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        float32_t setPoint = CfgDev_Get_LoadLmtT();
        uint8_t data[4];
        SetValFLT32(setPoint, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        float32_t argSP = GetArgFLT32(pCmdBuf);

        if(!CfgDev_Set_LoadLmtT(argSP))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set load limit CSP */
static void CmdProc_LoadLmtCSP(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        float32_t setPoint = CfgDev_Get_LoadLmtC();
        uint8_t data[4];
        SetValFLT32(setPoint, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        float32_t argSP = GetArgFLT32(pCmdBuf);

        if(!CfgDev_Set_LoadLmtC(argSP))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set load averaging enable */
static void CmdProc_LoadAvgEn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t on = (uint8_t) CfgDev_Get_LoadAvgEnable();
        RESP(CMDBYTE_FUNCCODE, &on, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_LoadAvgEnable((bool) argOn))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set load averaging preload */
static void CmdProc_LoadAvgPreload(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        float32_t preload = CfgDev_Get_LoadAvgPreload();
        uint8_t data[4];
        SetValFLT32(preload, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        float32_t argPreload = GetArgFLT32(pCmdBuf);

        if(!CfgDev_Set_LoadAvgPreload(argPreload))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set load averaging timeout */
static void CmdProc_LoadAvgTimeout(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint32_t timeout = CfgDev_Get_LoadAvgTimeout();
        uint8_t data[4];
        SetValUINT32(timeout, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint32_t argTimeout = GetArgUINT32(pCmdBuf);

        if(!CfgDev_Set_LoadAvgTimeout(argTimeout))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set peak detection enable */
static void CmdProc_PeakDetEn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t on = (uint8_t) CfgDev_Get_PeakDetEnable();
        RESP(CMDBYTE_FUNCCODE, &on, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_PeakDetEnable((bool) argOn))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set peak detection threshold */
static void CmdProc_PeakDetThreshold(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t pdt;
        bool usePDT = CfgDev_Get_UsePeakDetThr();
        if(usePDT)
            pdt = 0x01;
        else
            pdt = 0x02;
        RESP(CMDBYTE_FUNCCODE, &pdt, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        bool usePDT;
        uint8_t argPDT = GetArgUINT8(pCmdBuf);
        if(argPDT == 0x01)
            usePDT = true;
        else
            usePDT = false;
        if(!CfgDev_Set_UsePeakDetThr(usePDT))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set break detection enable */
static void CmdProc_BrkDetEn(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t data[2];
        data[0] = (uint8_t) CfgDev_Get_BrkDetEnable();
        data[1] = (uint8_t) CfgDev_Get_BrkDetMethod();
        RESP(CMDBYTE_FUNCCODE, data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argOn = GetArgUINT8(pCmdBuf);
        pCmdBuf += 1;
        uint32_t argMethod = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_BrkDetEnable((bool) argOn)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        if(!CfgDev_Set_BrkDetMethod(argMethod)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set break detection trigger */
static void CmdProc_BrkDetTrigger(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        float32_t trigger = CfgDev_Get_BrkDetTrigger();
        uint8_t data[4];
        SetValFLT32(trigger, data);

        RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        float32_t argTrigger = GetArgFLT32(pCmdBuf);

        if(!CfgDev_Set_BrkDetTrigger(argTrigger))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Get/Set break detection drop */
static void CmdProc_BrkDetDrop(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t drop = (uint8_t) CfgDev_Get_BrkDetDrop();
        RESP(CMDBYTE_FUNCCODE, &drop, 1, RspBuf, RspLen);
        return;
    }
    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argDrop = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_BrkDetDrop((uint32_t) argDrop))
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        else
            ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Test config - Start */
static void CmdProc_TestCfgStart(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t data[5];
        uint8_t cond = (uint8_t) CfgDev_Get_StartCond();
        data[0] = cond;
        if(cond == START_ON_TIME) {
            uint32_t time = CfgDev_Get_StartTime();
            SetValUINT32(time, &data[1]);
        }
        if(cond == START_ON_LOAD) {
            float32_t load = CfgDev_Get_StartLoad();
            SetValFLT32(load, &data[1]);
        }
        if(cond == START_ON_EXTN) {
            float32_t extn = CfgDev_Get_StartExtn();
            SetValFLT32(extn, &data[1]);
        }
        RESP(CMDBYTE_FUNCCODE, data, 5, RspBuf, RspLen);
        return;
    }

    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argCond = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_StartCond((uint32_t) argCond)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        pCmdBuf += 1;
        if(argCond == START_ON_NONE) {
        	ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        	return;
        }
        if(argCond == START_ON_TIME) {
            uint32_t time = GetArgUINT32(pCmdBuf);
            if(!CfgDev_Set_StartTime(time))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        if(argCond == START_ON_LOAD) {
            float32_t load = GetArgFLT32(pCmdBuf);
            if(!CfgDev_Set_StartLoad(load))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        if(argCond == START_ON_EXTN) {
            float32_t extn = GetArgFLT32(pCmdBuf);
            if(!CfgDev_Set_StartExtn(extn))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Test config - Stop */
static void CmdProc_TestCfgStop(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pCmdBuf = &CMDBYTE_DATA0;

    uint8_t argGS = GetArgUINT8(pCmdBuf);
    if(argGS == CMD_GET) {
        uint8_t data[5];
        memset(data, 0, sizeof(data));
        uint8_t cond = (uint8_t) CfgDev_Get_StopCond();
        data[0] = cond;
        if(cond == STOP_ON_LMT_TIME) {
            uint32_t time = CfgDev_Get_StopTime();
            SetValUINT32(time, &data[1]);
        }
        if(cond == STOP_ON_LMT_LOAD) {
            float32_t load = CfgDev_Get_StopLoad();
            SetValFLT32(load, &data[1]);
        }
        if(cond == STOP_ON_LMT_EXTN) {
            float32_t extn = CfgDev_Get_StopExtn();
            SetValFLT32(extn, &data[1]);
        }
        RESP(CMDBYTE_FUNCCODE, data, 5, RspBuf, RspLen);
        return;
    }

    if(argGS == CMD_SET) {
        pCmdBuf += 1;
        uint8_t argCond = GetArgUINT8(pCmdBuf);
        if(!CfgDev_Set_StopCond((uint32_t) argCond)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            return;
        }
        pCmdBuf += 1;
        if(argCond == STOP_ON_LMT_TIME) {
            uint32_t time = GetArgUINT32(pCmdBuf);
            if(!CfgDev_Set_StopTime(time))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        if(argCond == STOP_ON_LMT_LOAD) {
            float32_t load = GetArgFLT32(pCmdBuf);
            if(!CfgDev_Set_StopLoad(load))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        if(argCond == STOP_ON_LMT_EXTN) {
            float32_t extn = GetArgFLT32(pCmdBuf);
            if(!CfgDev_Set_StopExtn(extn))
                NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
            else
                ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
            return;
        }
        // ACK for options without parameters
        ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
        return;
    }

    NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
    return;
}

/* Test config - Current */
static void CmdProc_TestCfgCurr(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

	uint8_t argGS = GetArgUINT8(pCmdBuf);
	if(argGS == CMD_GET) {
		uint8_t idx = (uint8_t) CfgDev_Get_TestCfgIdx();
		RESP(CMDBYTE_FUNCCODE, &idx, 1, RspBuf, RspLen);
		return;
	}
	if(argGS == CMD_SET) {
		pCmdBuf += 1;
		uint8_t argIdx = GetArgUINT8(pCmdBuf);
		if(!TestCfg_ReqLoad(argIdx, TestCfg_LoadReqCB)) {
			NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
			return;
		}
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}
	if(argGS == CMD_DEFAULT) {
		pCmdBuf += 1;
		uint8_t argIdx = GetArgUINT8(pCmdBuf);
		if(!TestCfg_ReqReset(argIdx)) {
			NACK(CMDBYTE_FUNCCODE, CMD_RET_IMPROPERENV, RspBuf, RspLen);
			return;
		}
		ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}

	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
	return;
}

/* Process events */
static void CmdProc_Event(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	NACK(CMDBYTE_FUNCCODE, CMD_RET_NOIMPL, RspBuf, RspLen);
	return;
}

/* Process event mask */
static void CmdProc_EvtMask(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	uint8_t *pCmdBuf = &CMDBYTE_DATA0;

	uint8_t argGS = GetArgUINT8(pCmdBuf);
	if(argGS == CMD_GET) {
		uint32_t evtmask = CfgDev_Get_EventMask();
		uint8_t data[4];
		SetValUINT32(evtmask, data);

		RESP(CMDBYTE_FUNCCODE, (uint8_t*)data, sizeof(data), RspBuf, RspLen);
		return;
	}
	if(argGS == CMD_SET) {
		pCmdBuf += 1;
		uint32_t argEvtMask = GetArgUINT32(pCmdBuf);

		if(!CfgDev_Set_EventMask(argEvtMask))
			NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
		else
			ACK(CMDBYTE_FUNCCODE, RspBuf, RspLen);
		return;
	}

	NACK(CMDBYTE_FUNCCODE, CMD_RET_WRONGARGS, RspBuf, RspLen);
	return;
}

/* Command Table */
static const CmdHandler_t CmdTable[] =
{
    //{FuncCode, Perms, Rsvd1,  Rsvd2,   FuncHandler}

    // General
    {CMD_APPVER,        CMD_PERM_ALL, 0, 0, CmdProc_AppVer},
    {CMD_DEVADDR,       CMD_PERM_ALL, 0, 0, CmdProc_DevAddr},
    {CMD_SETTIME,       CMD_PERM_ALL, 0, 0, CmdProc_SetTime},
    {CMD_EXPORT_FILE,   CMD_PERM_ALL, 0, 0, CmdProc_ExportFile},
	{CMD_IMPORT_FILE,   CMD_PERM_ALL, 0, 0, CmdProc_ImportFile},

    // Operations
    {CMD_RESET,             CMD_PERM_ALL, 	0, 0, CmdProc_Reset},
    {CMD_SLEEP,             CMD_PERM_ALL, 	0, 0, CmdProc_Sleep},
    {CMD_BOOT,              CMD_PERM_ALL, 	0, 0, CmdProc_Boot},
    {CMD_DEFAULTS,          CMD_PERM_ALL, 	0, 0, CmdProc_Defaults},
    {CMD_USERACCESS,        CMD_PERM_ALL, 	0, 0, CmdProc_UserAccess},
    {CMD_USERPASS,          CMD_PERM_ALL, 	0, 0, CmdProc_UserPass},
	{CMD_CALMODE,           CMD_PERM_ALL, 	0, 0, CmdProc_CalMode},
	{CMD_RECOVERY,          CMD_PERM_SUPER,	0, 0, CmdProc_Recovery},
	{CMD_UPDATE_AXM,		CMD_PERM_SUPER,	0, 0, CmdProc_UpdateAxM},

    // Measurements
    {CMD_READ_RAW,          CMD_PERM_ALL, 0, 0, CmdProc_ReadRaw},
    {CMD_READ_TRUE,         CMD_PERM_ALL, 0, 0, CmdProc_ReadTrue},
    {CMD_READ_UNCAL,        CMD_PERM_ALL, 0, 0, CmdProc_ReadUnCal},
    {CMD_READ_MODE,         CMD_PERM_ALL, 0, 0, CmdProc_ReadMode},
    {CMD_READ_BURST,        CMD_PERM_ALL, 0, 0, CmdProc_ReadBurst},
    {CMD_LOG_BURST,         CMD_PERM_ALL, 0, 0, CmdProc_LogBurst},
    {CMD_ZERO,              CMD_PERM_ALL, 0, 0, CmdProc_Zero},
    {CMD_SCALE_FACTOR,      CMD_PERM_ALL, 0, 0, CmdProc_ScalingFactor},
    {CMD_SCALE_OFFSET,      CMD_PERM_ALL, 0, 0, CmdProc_ScalingOffset},

    // Information / Configuration
    {CMD_IDN,               CMD_PERM_ALL, 0, 0, CmdProc_Idn},
    {CMD_VER_FW,            CMD_PERM_ALL, 0, 0, CmdProc_VerFw},
    {CMD_VER_HW,            CMD_PERM_ALL, 0, 0, CmdProc_VerHw},
    {CMD_LANGUAGE,          CMD_PERM_ALL, 0, 0, CmdProc_Language},
	{CMD_LOCALE_SEP,        CMD_PERM_ALL, 0, 0, CmdProc_LocaleSep},
    {CMD_MODEL,             CMD_PERM_ALL, 0, 0, CmdProc_Model},
    {CMD_SERIAL,            CMD_PERM_ALL, 0, 0, CmdProc_Serial},
    {CMD_CAPACITY,          CMD_PERM_ALL, 0, 0, CmdProc_Capacity},
    {CMD_RESOLUTION,        CMD_PERM_ALL, 0, 0, CmdProc_Resolution},
    {CMD_UNITS,             CMD_PERM_ALL, 0, 0, CmdProc_Units},
    {CMD_AVLUNITS,          CMD_PERM_ALL, 0, 0, CmdProc_AvlUnits},
    {CMD_CAPUNITS,          CMD_PERM_ALL, 0, 0, CmdProc_CapUnits},
    {CMD_CALNUMPNTS,        CMD_PERM_ALL, 0, 0, CmdProc_CalNumPnts},
    {CMD_CALPNT,            CMD_PERM_ALL, 0, 0, CmdProc_CalPnt},
    {CMD_CALDATE,           CMD_PERM_ALL, 0, 0, CmdProc_CalDate},
    {CMD_CALDUE,            CMD_PERM_ALL, 0, 0, CmdProc_CalDue},
    {CMD_CALWARN,           CMD_PERM_ALL, 0, 0, CmdProc_CalWarn},
    {CMD_CALUNITS,          CMD_PERM_ALL, 0, 0, CmdProc_CalUnits},
    {CMD_OVERLOAD_NUM,      CMD_PERM_ALL, 0, 0, CmdProc_NumOverloads},
    {CMD_OVERLOAD_REC,      CMD_PERM_ALL, 0, 0, CmdProc_OverloadRecord},
    {CMD_POLARITY,          CMD_PERM_ALL, 0, 0, CmdProc_Polarity},
    {CMD_CONNSRCS,          CMD_PERM_ALL, 0, 0, CmdProc_ConnectedSources},
    {CMD_UDU_UNITS,         CMD_PERM_ALL, 0, 0, CmdProc_UduUnits},
    {CMD_UDU_CONV,          CMD_PERM_ALL, 0, 0, CmdProc_UduConv},
    {CMD_FEATURE_EN,        CMD_PERM_ALL, 0, 0, CmdProc_FeatureEn},
    {CMD_DISP_MODE,         CMD_PERM_ALL, 0, 0, CmdProc_DispMode},
    {CMD_DISP_BRIGHTNESS,   CMD_PERM_ALL, 0, 0, CmdProc_DispBrightness},
    {CMD_AUTO_SHUTDOWN,     CMD_PERM_ALL, 0, 0, CmdProc_AutoShutDown},
    {CMD_AUTO_DIMMING,      CMD_PERM_ALL, 0, 0, CmdProc_AutoDimming},
	{CMD_FILTER_OPTION,     CMD_PERM_ALL, 0, 0, CmdProc_FilterOption},
	{CMD_ZERO_ON_START,     CMD_PERM_ALL, 0, 0, CmdProc_ZeroOnStart},
	{CMD_ZERO_DEF,     		CMD_PERM_ALL, 0, 0, CmdProc_ZeroDef},
	{CMD_SAVECFG,           CMD_PERM_ALL, 0, 0, CmdProc_SaveCfg},

    // Testing
    {CMD_TEST_SOURCE,       CMD_PERM_ALL, 0, 0, CmdProc_TestSource},
    {CMD_TEST_START,        CMD_PERM_ALL, 0, 0, CmdProc_TestStart},
    {CMD_TEST_STOP,         CMD_PERM_ALL, 0, 0, CmdProc_TestStop},
    {CMD_TEST_RESULT,       CMD_PERM_ALL, 0, 0, CmdProc_TestResult},
    {CMD_USE_GAUGEPARAMS,   CMD_PERM_ALL, 0, 0, CmdProc_UseGaugeParams},
    {CMD_TESTLMT_EN,        CMD_PERM_ALL, 0, 0, CmdProc_TestLmtEn},
    {CMD_TESTLMT_HSP,       CMD_PERM_ALL, 0, 0, CmdProc_TestLmtHSP},
    {CMD_TESTLMT_LSP,       CMD_PERM_ALL, 0, 0, CmdProc_TestLmtLSP},
    {CMD_TESTLMT_NSP,       CMD_PERM_ALL, 0, 0, CmdProc_TestLmtNSP},
    {CMD_TESTLMT_BW,        CMD_PERM_ALL, 0, 0, CmdProc_TestLmtBW},
    {CMD_LOADLMT_EN,        CMD_PERM_ALL, 0, 0, CmdProc_LoadLmtEn},
    {CMD_LOADLMT_TSP,       CMD_PERM_ALL, 0, 0, CmdProc_LoadLmtTSP},
    {CMD_LOADLMT_CSP,       CMD_PERM_ALL, 0, 0, CmdProc_LoadLmtCSP},
    {CMD_LOADAVG_EN,        CMD_PERM_ALL, 0, 0, CmdProc_LoadAvgEn},
    {CMD_LOADAVG_PRELOAD,   CMD_PERM_ALL, 0, 0, CmdProc_LoadAvgPreload},
    {CMD_LOADAVG_TIMEOUT,   CMD_PERM_ALL, 0, 0, CmdProc_LoadAvgTimeout},
    {CMD_PKDET_EN,          CMD_PERM_ALL, 0, 0, CmdProc_PeakDetEn},
    {CMD_PKDET_THRESHOLD,   CMD_PERM_ALL, 0, 0, CmdProc_PeakDetThreshold},
    {CMD_BRKDET_EN,         CMD_PERM_ALL, 0, 0, CmdProc_BrkDetEn},
    {CMD_BRKDET_TRIGGER,    CMD_PERM_ALL, 0, 0, CmdProc_BrkDetTrigger},
    {CMD_BRKDET_DROP,       CMD_PERM_ALL, 0, 0, CmdProc_BrkDetDrop},
    {CMD_TESTCFG_START,     CMD_PERM_ALL, 0, 0, CmdProc_TestCfgStart},
    {CMD_TESTCFG_STOP,      CMD_PERM_ALL, 0, 0, CmdProc_TestCfgStop},
	{CMD_TESTCFG_CURR,      CMD_PERM_ALL, 0, 0, CmdProc_TestCfgCurr},

    {CMD_EVENT,             CMD_PERM_ALL, 0, 0, CmdProc_Event},
    {CMD_EVTMASK,           CMD_PERM_ALL, 0, 0, CmdProc_EvtMask},

	// End
	{CMD_MAX, CMD_PERM_ALL, 0, 0, NULL},
};


/** ASCII command processing **/

/* ACK */
static inline void USBASCII_ACK(uint8_t *Buf, uint32_t *Len)
{
	strcpy((char*) Buf, "!OK\r\n");
	*Len = 5;
}

/* NACK */
static inline void USBASCII_NACK(uint8_t *Buf, uint32_t *Len)
{
	strcpy((char*) Buf, "!NOK\r\n");
	*Len = 6;
}

/* Send event data - ASCII */
static void USBASCII_SetEVTData(uint8_t FuncCode, uint8_t *Data, uint8_t DataLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t *pData = Data;
    uint8_t i;

    if (FuncCode == CMD_EXPORT_ADATA_H) {
		for (i = 0; i < DataLen; i++)
			RspBuf[i] = *pData++;
		RspBuf[i] = '\r';
		RspBuf[1 + i] = '\n';
		*RspLen = (2 + DataLen);
	} else {
		RspBuf[0] = FuncCode;
		for (i = 0; i < DataLen; i++)
			RspBuf[1 + i] = *pData++;
		RspBuf[1 + i] = '\r';
		RspBuf[2 + i] = '\n';
		*RspLen = (3 + DataLen);
	}
}

/* Process ascii commands */
static void CmdProc_ASCIICmds(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
	/* Allow change of model for calibration */
	if (0 == strncmp((char*) CmdBuf, "DF3RST", strlen("DF3RST"))) {
		Cfg_RestoreDefaults();
		CfgDev_Set_IsProgrammed(0);
		USBASCII_ACK(RspBuf, RspLen);
		return;
	}

	uint32_t ascMode = CfgDev_Get_ASCIIMode();
	/* DF3 commands */
	if (ascMode == ASCIICOMM_DF3) {
		ASCIICmds_DF3_USB(CmdBuf, CmdLen, RspBuf, RspLen);
		return;
	}
	/* DF2 commands */
	if ((ascMode == ASCIICOMM_DF2_W) || (ascMode == ASCIICOMM_DF2_O)) {
		ASCIICmds_DF2_USB(CmdBuf, CmdLen, RspBuf, RspLen);
		return;
	}

	USBASCII_NACK(RspBuf, RspLen);
	return;
}

/* Public Functions */

/* Process command - USB */
CmdStatus_t CmdUSB_Process(uint8_t *CmdBuf, uint32_t CmdLen, uint8_t *RspBuf, uint32_t *RspLen)
{
    /* Do not process commands while sleeping */
    if (Sys_IsSleeping())
    	return CMDSTAT_DONE;

    /* Process ASCII commands */
    if (COM_IsASCIIMode()) {
    	if ((CmdBuf[CmdLen - 1] == '\n') && (CmdBuf[CmdLen - 2] == '\r')) {
    		CmdProc_ASCIICmds(CmdBuf, CmdLen, RspBuf, RspLen);
    		return CMDSTAT_DONE;
    	} else {
    		return CMDSTAT_PROCESSING;
    	}
    }

    /* Ignore messages for other devices */
    if (!CheckAddr(CMDBYTE_DEVADDR))
    	return CMDSTAT_DONE;

    /* At least 4 bytes for a message */
    if (CmdLen < 4)
        return CMDSTAT_PROCESSING;

    /* Check for entire message */
    uint32_t msgCnt = CMDBYTE_DATALEN + 4;
    if (CmdLen < msgCnt)
        return CMDSTAT_PROCESSING;

    /* Set current address */
    SetAddr(CMDBYTE_DEVADDR);

    /* Check CRC */
#if 0
    if (CmdBuf[msgCnt - 1] != GetCRC(CmdBuf, msgCnt - 1)) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_CRCERROR, RspBuf, RspLen);
        RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
        *RspLen += 1;
        return CMDSTAT_DONE;
    }
#endif

    /* Search CmdTable */
    uint8_t idx = 0;
    uint8_t funcCode = CMDBYTE_FUNCCODE;
    while (idx < CMD_MAX) {
        if (funcCode <= CmdTable[idx].FuncCode)
            break;
        idx++;
    }

    /* Unknown Command */
    if ((funcCode < CmdTable[idx].FuncCode) || (idx == CMD_MAX)) {
        NACK(CMDBYTE_FUNCCODE, CMD_RET_UNKNOWNCMD, RspBuf, RspLen);
        RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
        *RspLen += 1;
        return CMDSTAT_DONE;
    }

    /* Check permissions */
    uint8_t currUser = Users_GetCurrUser();
    if (CmdTable[idx].Perms == CMD_PERM_SUPER) {
        if (currUser != USER_SUPER) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_NOPERM, RspBuf, RspLen);
            RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
            *RspLen += 1;
            return CMDSTAT_DONE;
        }
    }
    if (CmdTable[idx].Perms == CMD_PERM_ADMIN) {
        if ((currUser != USER_ADMIN) && (currUser != USER_SUPER)) {
            NACK(CMDBYTE_FUNCCODE, CMD_RET_NOPERM, RspBuf, RspLen);
            RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
            *RspLen += 1;
            return CMDSTAT_DONE;
        }
    }

    /* Run Handler and set CRC */
    CmdTable[idx].FuncHandler(CmdBuf, CmdLen, RspBuf, RspLen);

    if (*RspLen != 0) {
    	RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
    	*RspLen += 1;
    }

    return CMDSTAT_DONE;
}

/* Tx reading */
void CmdUSB_Tx_Reading(uint32_t Src, float32_t Reading, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t data[5];
    /* Set source and reading **/
    data[0] = (uint8_t) (Src - 1);
    SetValFLT32(Reading, &data[1]);
    /* Set response and CRC */
    RESP(CMD_READ_BURST, data, sizeof(data), RspBuf, RspLen);
    RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
    *RspLen += 1;

    return;
}

/* Tx ASCII reading */
void CmdUSB_Tx_ASCIIReading(uint32_t Src, float32_t Reading, uint8_t *RspBuf, uint32_t *RspLen)
{
	char data[16];

	/* Source Load */
	uint32_t srcLoad = CfgDev_Get_SrcLoad();
	/* Check if source is torque */
	bool isTorque = SrcLoad_IsTorque();
	/* Calibration unit string */
	char * calUnit = SrcLoad_GetCalUnits(srcLoad);
	/* Current unit string */
	char * currUnit = SrcLoad_GetUnitsStr(SrcLoad_GetUnits(isTorque), isTorque, true);
	/* Unit conversion factor */
	float32_t convFactor = SrcLoad_GetConvFactor(isTorque, calUnit, currUnit);

	/* Float to string */
	Utils_FloatToString(data, sizeof(data) - 3, convFactor * Reading, SrcLoad_GetConfResolution());

	if (2 == COM_IsASCIIMode()) {  // ASCII - DF2_W (With unit)
		strncat(data, " ", 1);
		strncat(data, currUnit, strlen(currUnit));
	}

	strncat(data, "\r\n", 2);
	strcpy((char*) RspBuf, data);
	*RspLen = strlen(data);

    return;
}

/* Tx event */
void CmdUSB_Tx_Event(uint32_t Evt, uint8_t *RspBuf, uint32_t *RspLen)
{
    uint8_t data[256];
    uint32_t event;
	uint8_t i, size;
	bool addCrc = true;

	*RspLen = 0;

    switch(Evt) {

        case EVT_USB_TBREAK:
            event = EVT_USB_TBREAK;
            SetValUINT32(event, &data[0]);
            float32_t tBrkLoad = Test_GetTBreak();
            SetValFLT32(tBrkLoad, &data[4]);
            RESP(CMD_EVENT, data, 8, RspBuf, RspLen);
            break;

        case EVT_USB_CBREAK:
            event = EVT_USB_CBREAK;
            SetValUINT32(event, &data[0]);
            float32_t cBrkLoad = Test_GetCBreak();
            SetValFLT32(cBrkLoad, &data[4]);
            RESP(CMD_EVENT, data, 8, RspBuf, RspLen);
            break;

        case EVT_USB_TESTSTOP:
        case EVT_USB_OVERLOAD:
        case EVT_USB_TSAFELMT:
        case EVT_USB_CSAFELMT:
            event = Evt;
            SetValUINT32(event, &data[0]);
            RESP(CMD_EVENT, data, 4, RspBuf, RspLen);
            break;

        case EVT_USB_UPDSTAT:
        	event = EVT_USB_UPDSTAT;
        	SetValUINT32(event, &data[0]);
        	/* Convert to float - for easy parsing of host apps */
        	float32_t updStat = (float32_t) DispUpdate_GetError();
        	SetValFLT32(updStat, &data[4]);
        	RESP(CMD_EVENT, data, 8, RspBuf, RspLen);
        	break;

        case EVT_USB_BOOTERR:
        	size = ErrorLog_GetSize();
        	for (i=0; i<size; i++)
        		data[i] = ErrorLog_GetErrorCode(i);
        	RESP(CMD_EXC_BOOT, data, size, RspBuf, RspLen);
        	break;

        case EVT_USB_EXPORT_FILE:
        	ExportFile_Export(data, &size);
        	RESP(CMD_EXPORT_FILE, data, size, RspBuf, RspLen);
        	break;

        case EVT_USB_EXP_ADATA:
        	DAQ_GetExportData((char*)data, &size);
        	USBASCII_SetEVTData(CMD_EXPORT_ADATA, data, size, RspBuf, RspLen);
        	addCrc = false;
        	break;

        case EVT_USB_EXP_ADATA_H:
        	DAQ_GetExportData((char*)data, &size);
        	USBASCII_SetEVTData(CMD_EXPORT_ADATA_H, data, size, RspBuf, RspLen);
        	addCrc = false;
        	break;

        default:
            break;
    }

    if(addCrc && (RspLen != 0)) {
        RspBuf[*RspLen] = GetCRC(RspBuf, *RspLen);
        *RspLen += 1;
    }

    return;
}

/* Is streaming data in DF2 mode */
bool CmdUSB_IsDF2DataStreaming(void)
{
	return DF2DataStreaming;
}

/* Set DF2 Normal reading as response */
void CmdUSB_SetDF2Reading(uint8_t *RspBuf, uint32_t *RspLen)
{
	ASCII_DF2_SetReading(Test_GetNorm(), RspBuf, RspLen);
	return;
}

/******************************** End of File *********************************/
