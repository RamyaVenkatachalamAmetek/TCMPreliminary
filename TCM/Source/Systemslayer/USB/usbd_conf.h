/**
 * ST provided
 * Modified by jiss.joseph@ametek.com
 **/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Exported Defines */

/*---------- -----------*/
#define USBD_MAX_NUM_INTERFACES     (1U)
/*---------- -----------*/
#define USBD_MAX_NUM_CONFIGURATION  (1U)
/*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ       (512U)
/*---------- -----------*/
#define USBD_DEBUG_LEVEL            (0U)
/*---------- -----------*/
#define USBD_LPM_ENABLED            (0U)
/*---------- -----------*/
#define USBD_SELF_POWERED           (1U)

/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS 		(1)

/* Memory management macros */

/** Alias for memory allocation. */
#define USBD_malloc         malloc
/** Alias for memory release. */
#define USBD_free          free
/** Alias for memory set. */
#define USBD_memset         memset
/** Alias for memory copy. */
#define USBD_memcpy         memcpy

/* DEBUG macros */

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...)    printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define USBD_ErrLog(...)    printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

#endif /* __USBD_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
