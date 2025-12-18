/**
 **  @file POST.h
 **  @brief Power On Self Tests
 **  @author JZJ
 **
 **/
 
#ifndef _POST_H_
#define _POST_H_

/* Includes */
#include "PAL.h"

/* Macros */

/* Types */

/* Function Prototypes */
/* Run phase 1 tests */
StdReturn_t POST_Run1(void);
/* Run phase 2 tests */
StdReturn_t POST_Run2(void);

#endif /*** _POST_H_ ***/
