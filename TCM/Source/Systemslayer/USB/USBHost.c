/**
 **  @file USBHost.c
 **  @brief USB Host
 **  @author JZJ
 **
 **/

/* Includes */
#include "PAL.h"

#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_msc.h"

#include "ff_gen_drv.h"

/* Macros */

/* Types */

/* Externs */

/* Function Declarations */

/* Global Variables */
USBH_HandleTypeDef  USBHost;

/* Drv volume */
char USBDISKVOL[4];

/* Filesystem */
FATFS USBDiskFF;

/* Static Variables */
static DWORD scratch[_MAX_SS / 4];

/* Private Functions */

/* Driver API */
DSTATUS USBDisk_Drv_Init(BYTE);
DSTATUS USBDisk_Drv_Status(BYTE);
DRESULT USBDisk_Drv_Read(BYTE, BYTE*, DWORD, UINT);
DRESULT USBDisk_Drv_Write(BYTE, const BYTE*, DWORD, UINT);
DRESULT USBDisk_Drv_IOCtl(BYTE, BYTE, void*);
const Diskio_drvTypeDef USBDisk_DrvOps =
{
        USBDisk_Drv_Init,
        USBDisk_Drv_Status,
        USBDisk_Drv_Read,
        USBDisk_Drv_Write,
        USBDisk_Drv_IOCtl,
};

DSTATUS USBDisk_Drv_Init(BYTE lun)
{
    return RES_OK;
}

DSTATUS USBDisk_Drv_Status(BYTE lun)
{
    if(USBH_MSC_UnitIsReady(&USBHost, lun))
        return RES_OK;
    else
        return RES_ERROR;
}

DRESULT USBDisk_Drv_Read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;
    USBH_StatusTypeDef  status = USBH_OK;

    if(((DWORD)buff & 3) && (((HCD_HandleTypeDef *)USBHost.pData)->Init.dma_enable)) {

        while((count--)&&(status == USBH_OK)) {
            status = USBH_MSC_Read(&USBHost, lun, sector + count, (uint8_t *)scratch, 1);
            if(status == USBH_OK)
                memcpy(&buff[count * _MAX_SS] ,scratch, _MAX_SS);
            else
                break;
        }
    } else {
        status = USBH_MSC_Read(&USBHost, lun, sector, buff, count);
    }

    if(status == USBH_OK) {
        res = RES_OK;
    } else {
    	MSC_LUNTypeDef info;
    	memset(&info, 0, sizeof(info));

        USBH_MSC_GetLUNInfo(&USBHost, lun, &info);

        switch(info.sense.asc) {
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                // Error_Handler(0); // Handle error
                res = RES_NOTRDY;
                break;

            default:
                res = RES_ERROR;
                break;
        }
    }

    return res;
}

DRESULT USBDisk_Drv_Write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res = RES_ERROR;
    USBH_StatusTypeDef  status = USBH_OK;

    if(((DWORD)buff & 3) && (((HCD_HandleTypeDef *)USBHost.pData)->Init.dma_enable)) {

        while(count--) {
            memcpy (scratch, &buff[count * _MAX_SS], _MAX_SS);
            status = USBH_MSC_Write(&USBHost, lun, sector + count, (BYTE *)scratch, 1) ;
            if(status == USBH_FAIL)
                break;
        }
    } else {
        status = USBH_MSC_Write(&USBHost, lun, sector, (BYTE *)buff, count);
    }

    if(status == USBH_OK) {
        res = RES_OK;
    } else {
    	MSC_LUNTypeDef info;
    	memset(&info, 0, sizeof(info));

        USBH_MSC_GetLUNInfo(&USBHost, lun, &info);

        switch (info.sense.asc) {
            case SCSI_ASC_WRITE_PROTECTED:
                // Error_Handler(0); // Handle error
                res = RES_WRPRT;
                break;

            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                // Error_Handler(0); // Handle error
                res = RES_NOTRDY;
                break;

            default:
                res = RES_ERROR;
                break;
        }
    }

    return res;
}

DRESULT USBDisk_Drv_IOCtl(BYTE lun, BYTE cmd, void *buff)
{
    DRESULT res = RES_ERROR;
    MSC_LUNTypeDef info;

    switch(cmd) {
        /* Make sure that no pending write process */
        case CTRL_SYNC:
            res = RES_OK;
            break;

        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT:
            if(USBH_MSC_GetLUNInfo(&USBHost, lun, &info) == USBH_OK) {
                *(DWORD*)buff = info.capacity.block_nbr;
                res = RES_OK;
            } else {
                res = RES_ERROR;
            }
            break;

        /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE:
            if(USBH_MSC_GetLUNInfo(&USBHost, lun, &info) == USBH_OK) {
                *(DWORD*)buff = info.capacity.block_size;
                res = RES_OK;
            } else {
                res = RES_ERROR;
            }
            break;

        /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE:
            if(USBH_MSC_GetLUNInfo(&USBHost, lun, &info) == USBH_OK) {
                *(DWORD*)buff = info.capacity.block_size;
                res = RES_OK;
            } else {
                res = RES_ERROR;
            }
            break;

        default:
            res = RES_PARERR;
    }

    return res;
}

/* Application process */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            if(f_mount(NULL, "", 0) != FR_OK) {
                // Error_Handler(0); // Handle error
            }
            if(FATFS_UnLinkDriver(USBDISKVOL) != 0) {
                // Error_Handler(0); // Handle error
            }
            break;

        case HOST_USER_CLASS_ACTIVE:
            if(FATFS_LinkDriver(&USBDisk_DrvOps, USBDISKVOL) == 0) {
                if(f_mount(&USBDiskFF, (TCHAR const*)USBDISKVOL, 1) != FR_OK) {
                    // Error_Handler(0); // Handle error
                }
            }
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }
}

/* Public Functions */
/* Init */
StdReturn_t USBHost_Init(void)
{
    /* Init Host Library */
    USBH_Init(&USBHost, USBH_UserProcess, 0);

    /* Add Supported Class */
    USBH_RegisterClass(&USBHost, USBH_MSC_CLASS);

    return RET_OK;
}

/* DeInit */
StdReturn_t USBHost_DeInit(void)
{
    USBH_DeInit(&USBHost);

    return RET_OK;
}

/* Start */
StdReturn_t USBHost_Start(void)
{
    /* Start Host Process */
    USBH_Start(&USBHost);

    return RET_OK;
}

/* Stop */
StdReturn_t USBHost_Stop(void)
{
	/* Stop Host Process */
	USBH_Stop(&USBHost);

	return RET_OK;
}

/******************************** End of File *********************************/
