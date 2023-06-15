/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : Calibration.h
* Author             : R&D SW
* Brief              : Calibration module header file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/06/19          R&D SW   First Issue
*******************************************************************************/

#ifndef APP_CALIBRATION_CALADCOFFSET_H_
#define APP_CALIBRATION_CALADCOFFSET_H_

#ifdef APP_CALIBRATION_CALADCOFFSET_H_
#define APP_CALIBRATION_CALADCOFFSET_H_
#else
#define APP_CALIBRATION_CALADCOFFSET_H_ extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* Export define -------------------------------------------------------------*/

/* Export typedef ------------------------------------------------------------*/


typedef struct
{
    u32 magicNum;
    u32 OffsetValue;
    u32 Crc;
}TS_ADC_OFFSET_OBJ;

typedef struct
{
    u32 e2StartAddr;
    u32 magicNum;
    u32 resetFlg;
    u32 updateInd;
    u32 updateCnt;
    u32 defaultValue;
    u32 minVlaue;
    u32 maxVlaue;
}TS_ADC_OFFSET_INFO;

/* Export variables ----------------------------------------------------------*/

/* Export function -----------------------------------------------------------*/
void AcInputCurrentAdcOffsetInit(void);
u32  AcInputCurrentAdcOffsetGet(void);
void AcInputCurrentAdcOffsetUpdate(void);
#endif /* APP_CALIBRATION_CALADCOFFSET_H_ */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
