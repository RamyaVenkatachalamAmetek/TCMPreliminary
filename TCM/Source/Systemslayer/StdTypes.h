/**
 **  @file StdTypes.h
 **  @brief Standard Types
 **  @author JZJ
 **
 **/

#ifndef _StdTypes_H_
#define _StdTypes_H_

/* Includes */
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <float.h>

/* Macros */
#ifndef NULL
#define	NULL ((void*)0)
#endif

#ifndef MIN
#define MIN(x,y) ((x < y) ? x : y)
#endif
#ifndef MAX
#define MAX(x,y) ((x > y) ? x : y)
#endif

#ifndef ABS
#define ABS(x) ((x >= 0) ? x : -x)
#endif

#define UINT_MAX (0xFFFFFFFF)
#define UINT_MIN (0x00000000)

/* Endian swap - from alpi_endian.h */
#define _eREV16(x) ((uint16_t) ((((x) >> 8) & 0XFF) | (((x) & 0XFF) << 8)))

#define _eREV32(x) ((uint32_t) ((((x) >> 24) & 0XFF) |	\
                    (((x) >> 8) & 0XFF00) |	\
				    (((x) & 0XFF00) << 8) |	\
				    (((x) & 0XFF) << 24)))

#define _eREV64(x)  ((uint64_t) ((((x) >> 56) & 0XFF) |		\
                      (((x) >> 40) & 0XFF00) |		\
				      (((x) >> 24) & 0XFF0000) |	\
				      (((x) >> 8) & 0XFF000000) |	\
				      (((x) & 0XFF000000) << 8) |	\
				      (((x) & 0XFF0000) << 24) |	\
				      (((x) & 0XFF00) << 40) |		\
				      (((x) & 0XFF) << 56)))

/* Get bytes */
#define X32_BYTE3(x)    ((uint8_t)((x & 0xFF000000) >> 24))
#define X32_BYTE2(x)    ((uint8_t)((x & 0x00FF0000) >> 16))
#define X32_BYTE1(x)    ((uint8_t)((x & 0x0000FF00) >> 8))
#define X32_BYTE0(x)    ((uint8_t)((x & 0x000000FF)))
#define X16_BYTE1(x)    ((uint8_t)((x & 0xFF00) >> 8))
#define X16_BYTE0(x)    ((uint8_t)((x & 0x00FF)))

/* Types */

/* F32 */
typedef float float32_t;

/* Standard Return Type */
typedef enum {
    RET_OK = 0,     // Success
    RET_NOK,        // General failure
    RET_HW_NOK,     // Bad hardware / not responding
    RET_ARGS_NOK,   // Improper arguments
    RET_ENV_NOK,    // Improper conditions
    RET_TIMEDOUT,   // Timed out
    RET_NO_IMPL,    // Not implemented
    RET_BUF_FULL,   // Buffer is full
    RET_BUF_EMPTY,  // Buffer is empty
    RET_PROCESSING, // Processing
} StdReturn_t;

/* Function Prototypes */



#endif /*** _StdTypes_H_ ***/
