/**
 **  @file USBDev.c
 **  @brief USB Device
 **  @author JZJ
 **
 **/

/* Includes */
#include "PAL.h"
#include "USBDev.h"

//RV:#include "Tasks.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"

/* Macros */
/* Buffer lengths */
#define USBDEV_RX_BUF_LEN (CDC_DATA_FS_OUT_PACKET_SIZE)

/* Types */

/* Externs */
extern PCD_HandleTypeDef hpcd;

/* Function Declarations */

/* Global Variables */


/* Static Variables */
static USBDev_Config_t USBDev_Cfg;
/* COM port */
static USBD_CDC_LineCodingTypeDef LineCoding =
{
        115200, /* baud rate*/
        0x00,   /* stop bits-0*/
        0x00,   /* parity - none*/
        0x08    /* nb. of bits 8*/
};

/* USBD Handle */
USBD_HandleTypeDef USBD_Device;

/* CDC Tx/Rx buffers */
uint8_t USBDev_CDC_RxBuf[USBDEV_RX_BUF_LEN];

/* Private Functions */

/* CDC interfaces */
static int8_t USBDev_CDC_Init(void)
{
    USBD_CDC_SetRxBuffer(&USBD_Device, USBDev_CDC_RxBuf);

    return (USBD_OK);
}

static int8_t USBDev_CDC_DeInit(void)
{
    return (USBD_OK);
}

static int8_t USBDev_CDC_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
    switch (cmd)
    {
        case CDC_SEND_ENCAPSULATED_COMMAND:
            break;

        case CDC_GET_ENCAPSULATED_RESPONSE:
            break;

        case CDC_SET_COMM_FEATURE:
            break;

        case CDC_GET_COMM_FEATURE:
            break;

        case CDC_CLEAR_COMM_FEATURE:
            break;

        case CDC_SET_LINE_CODING:
            LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                    (pbuf[2] << 16) | (pbuf[3] << 24));
            LineCoding.format     = pbuf[4];
            LineCoding.paritytype = pbuf[5];
            LineCoding.datatype   = pbuf[6];
            break;

        case CDC_GET_LINE_CODING:
            pbuf[0] = (uint8_t)(LineCoding.bitrate);
            pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
            pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
            pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
            pbuf[4] = LineCoding.format;
            pbuf[5] = LineCoding.paritytype;
            pbuf[6] = LineCoding.datatype;
            break;

        case CDC_SET_CONTROL_LINE_STATE:
            break;

        case CDC_SEND_BREAK:
            break;

        default:
            break;
    }

    return (USBD_OK);
}

static int8_t USBDev_CDC_Receive(uint8_t* Buf, uint32_t *Len)
{
    USBDev_Cfg.ReceiveCB(Buf, *Len);
    return (USBD_OK);
}

static int8_t USBDev_CDC_TransmitCmplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    USBDev_Cfg.TxCmpltCB();
    return (USBD_OK);
}

USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
    USBDev_CDC_Init,
    USBDev_CDC_DeInit,
    USBDev_CDC_Control,
    USBDev_CDC_Receive,
    USBDev_CDC_TransmitCmplt
};

/* Public Functions */
/* Init */
StdReturn_t USBDev_Init(USBDev_Config_t *Config)
{
    /* Init Device Library */
    if(USBD_OK != USBD_Init(&USBD_Device, &FS_Desc, DEVICE_FS))
        return RET_NOK;

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

    /* Add CDC Interface Class */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

    /* Set callbacks */
    USBDev_Cfg.ReceiveCB = Config->ReceiveCB;
    USBDev_Cfg.TxCmpltCB = Config->TxCmpltCB;

    return RET_OK;
}

/* DeInit */
StdReturn_t USBDev_DeInit(void)
{
    USBD_DeInit(&USBD_Device);

    return RET_OK;
}

/* Start */
StdReturn_t USBDev_Start(void)
{
    /* Start Device Process */
    USBD_Start(&USBD_Device);

    return RET_OK;
}

/* Stop */
StdReturn_t USBDev_Stop(void)
{
    USBD_Stop(&USBD_Device);

    return RET_OK;
}

/* Send data */
StdReturn_t USBDev_Transmit(uint8_t *Data, uint32_t Size)
{
    USBD_CDC_SetTxBuffer(&USBD_Device, Data, Size);
    USBD_CDC_TransmitPacket(&USBD_Device);
    return RET_OK;
}

/******************************** End of File *********************************/
