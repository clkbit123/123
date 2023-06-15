/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AlarmApp.h
* Author             : henry
* Brief              : header file of Alarm module
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2017/04/08          henry   First Issue
*******************************************************************************/

#ifndef APP_ALARM_LDC_H_
#define APP_ALARM_LDC_H_

#ifdef  APP_ALARM_LDC_C
#define APP_ALARM_LDC_DEC
#else
#define APP_ALARM_LDC_DEC  extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "AppAlarm.h"
#include "AppObcConfig.h"
#include "Rte_Temp.h"

/* Export typedef ------------------------------------------------------------*/
typedef enum
{
    TDAE_DCDC_INPUT_UNDERVOLTAGE = 0,
    TDAE_DCDC_INPUT_OVERVOLTAGE,
    TDAE_DCDC_OUTPUT_UNDERVOLTAGE,
    TDAE_DCDC_OUTPUT_OVERVOLTAGE,
    
    TDAE_DCDC_WATER_OVERTEMP,
    TDAE_DCDC_OUTPUT_OVERCURRENT,
	TDAE_DCDC_ON12V_UNDERVOLTAGE,	
	TDAE_DCDC_ON12V_OVERVOLTAGE,

    TDAE_DCDC_TEMP_OVER,
    TDAE_DCDC_HARDWARE_PROTECT,
    TDAE_DCDC_OUTPUT_LINE_BREAK,
    
    TDAE_DCDC_MAX_ALARM
}TE_DCDC_ALARM_EVENT;   /* dcdc alarm */

typedef enum
{
    DCDC_INPUT_VOLTAGE = 0,
    DCDC_INPUT_CURRENT,
    DCDC_OUTPUT_VOLTAGE,
    DCDC_OUTPUT_CURRENT,

    MAX_DCDC_ALARM_DATA
}TE_DCDC_ALARM_DATA;
    
typedef enum
{
    DCDC_FLAG_HW_PROTECT = 0,
    DCDC_FLAG_OVERCURRENT,
    DCDC_FLAG_NUM_MAX
}TE_DCDC_ALARM_FLAG;

typedef struct
{
    u08 uwAlarmFlag[DCDC_FLAG_NUM_MAX];
    u16 uwAlarmData[MAX_DCDC_ALARM_DATA];
}TS_DCDC_ALARM_DATA;

/* Export define -------------------------------------------------------------*/

#if((PRODUCT_CODE == FE2959)||(PRODUCT_CODE == FE2917))
    #define     DCDC_HV_OVERVOLTAGE         4000            /* 400.0V, scale: 0.1V */
    #define     DCDC_HV_RATE_UNDERVOLTAGE   1900            /* 190.0V, scale: 0.1V */
    #define     DCDC_HV_UNDERVOLTAGE        1300            /* 130.0V, scale: 0.1V */

#elif((PRODUCT_CODE == FE2928)||(PRODUCT_CODE == FE3001)||(PRODUCT_CODE == FE3065))
    #define     DCDC_HV_OVERVOLTAGE         4710            /* 471.0V, scale: 0.1V */
    #define     DCDC_HV_RATE_UNDERVOLTAGE   2300            /* 230.0V, scale: 0.1V */
    #define     DCDC_HV_UNDERVOLTAGE        1500            /* 157.0V, scale: 0.1V */

#else
#error WRONG PRODUCT_CODE
#endif

#define     DCDC_WATER_OVERTEMP         (105+TEMP_OFFSET)      /*DCDC water over temperature 95℃  */
#define     DCDC_THS_OVERTEMP           (130+TEMP_OFFSET)       /*DCDC ths over temperature 130℃  */
#define     DCDC_THSOVERTEMPEALARM      (120+TEMP_OFFSET)       /*DCDC Over temperature 120℃  alarm */
#define     DCDC_WATEROVERTEMPEALARM    (88+TEMP_OFFSET)        /*DCDC Over temperature 78℃  alarm */

#define     DCDC_LV_OVERVOLTAGE         1650    			/* 16.5V, scale: 0.01V */
#define     DCDC_LV_UNDERVOLTAGE        680     			/* 8.5V,  scale: 0.01V */
#define     DCDC_LLC_UNDERVOLTAGE       500     			/* 5.0V,  scale: 0.01V */
#define     DCDC_LV_INPUT_OVERCURRENT   2000    			/* 20.0A, scale: 0.1A */
#define     DCDC_LV_OUTPUTOVERCURRENT   2200    			/* 130.0A,scale: 0.1A */
#define 	DCDC_LV_UV_PROTECT_POINT	680					/* 6.80V  Low bat */
#define 	DCDC_LV_OV_PROTECT_POINT	1650				/* 16.5V  Low bat */

#define     DCDC_PRE_CHG_HV_HIGH         466            /* 466V, scale: 1V */
#define     DCDC_PRE_CHG_HV_LOW          200            /* 200V, scale: 1V */

#define     DCDC_PRE_CHG_LV_UV_PROTECT_POINT    800                /* 800V, scale: 1V */
#define     DCDC_PRE_CHG_LV_OV_PROTECT_POINT    1480               /* 1500V, scale: 1V */

#define     DCDC_PRE_CHG_INPUT_CURRENT_LOW         24            /* scale: 1A */
#define     DCDC_PRE_CHG_INPUT_CURRENT_HIGH        40            /* scale: 1A */

#define     DCDC_PRE_CHG_INPUT_VOL_CRITICAL_POINT       1200            /* scale: 1A */

#define     DCDC_PRE_CHG_MAX_DURATION               5000
#define     DCDC_PRE_CHG_MIN_DURATION               300
/* Export macro --------------------------------------------------------------*/
/* Export variables ----------------------------------------------------------*/
APP_ALARM_LDC_DEC TS_ALARM_MANAGE tDcdcAlarm;

/* Export function -----------------------------------------------------------*/
APP_ALARM_LDC_DEC void AppDcdcAlarmInit(void);
APP_ALARM_LDC_DEC u08 AppDcdcAlarmEnableCheck(void);
APP_ALARM_LDC_DEC u08 AppDcdcAlarmRunningCheck(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmDataObcOutputVolt(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmDataDcdcOutputVolt(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmDataDcdcTemp(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmWaterTemp(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmFlagOutputLineBreak(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmFlagHwProtect(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmFlagOutputOverCurrent(void);
APP_ALARM_LDC_DEC u32 AppDcdcAlarmDataLVVolt(void);
APP_ALARM_LDC_DEC u32 AppGetAllDcdcAlarmFlag(void);
APP_ALARM_LDC_DEC u08 AppGetDcdcAlarmBit(TE_DCDC_ALARM_EVENT tDcdcAlarmEvent);
APP_ALARM_LDC_DEC void AppSetDcdcAlarmBit(TE_DCDC_ALARM_EVENT tDcdcAlarmEvent, u08 ucAction);

#endif /* APP_ALARM_LDC_H_ */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
