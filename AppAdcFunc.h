/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AppAdcFunc.h
* Author             : R&D SW
* Brief              : Application Adc Function hearder file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/05/15          R&D SW   First Issue
*******************************************************************************/

#ifndef APP_ADC_FUNC_H_
#define APP_ADC_FUNC_H_

#ifdef APP_ADC_FUNC_C_
#define APP_ADC_FUNC_DEC
#else
#define APP_ADC_FUNC_DEC extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* Export typedef ------------------------------------------------------------*/
typedef enum
{
//    ELOCK1_VOL,         /* Elock + Voltage, scale: 0.1V */
    LV_12V_VOL,         /* LowBattery Voltage, scale: 0.01V */
//    ELOCK2_VOL,         /* Elock - Voltage, scale: 0.1V */
//	OBC_EN_VOL,			/* Obc Enable Detect Voltage, scale: 0.1V */
	INSU_L_RESISTANCE,  /* L Insulation Resistance_VALUE, scale: 1Ω */
	INSU_N_RESISTANCE,  /* N Insulation Resistance_VALUE, scale: 1Ω */
	HW_VER_VOL,			/* Hardware Version Voltage, scale: 0.1V */
#if (PRODUCT_PLATFORM == DC_AND_OBC_3300W)
	INSU_N_CURRENT,         /* inverter current in the car, scale: 0.01A */
	INSU_L_DIFF_CURRENT,    /* N,L diff current, scale: 0.01A */
#elif (PRODUCT_PLATFORM == DC_AND_OBC_6600W)
	HV_K2_VOL,         	/* HV K2 Voltage, scale: 0.1V */
	HV_K5_VOL,         	/* HV K5 Voltage, scale: 0.1V */
#endif
	INTER_LOCK1_VOL,	/* lock1 Voltage, scale: 0.1V */
	INTER_LOCK2_VOL,	/* lock2 Voltage, scale: 0.1V */

    MAX_ADC_ITEM
}TE_ADC_ITEM;

typedef struct
{
    u16 uwMeasure[MAX_ADC_ITEM];
}TS_ADC_DATA;

/* Export define -------------------------------------------------------------*/
/* Export macro --------------------------------------------------------------*/
/* Export variables ----------------------------------------------------------*/
/* Export function -----------------------------------------------------------*/
APP_ADC_FUNC_DEC void InitAdcAppData(void);
APP_ADC_FUNC_DEC void UpdateAdcAppValue(void);
APP_ADC_FUNC_DEC u16  GetAdcAppValue(TE_ADC_ITEM tAdcItem);

#endif /* APP_ADC_FUNC_H_ */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
