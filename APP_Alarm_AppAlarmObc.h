/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AlarmApp.h
* Author             : henry
* Brief              : header file of Alarm module
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2017/04/08          henry   First Issue
*******************************************************************************/

#ifndef APP_ALARM_OBC_H_
#define APP_ALARM_OBC_H_

#ifdef  APP_ALARM_OBC_C
	#define APP_ALARM_OBC_DEC
#else
	#define APP_ALARM_OBC_DEC  extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "AppAlarm.h"
#include "AppObcConfig.h"
#include "Rte_Temp.h"

/* Export typedef ------------------------------------------------------------*/
typedef enum
{
    TDAE_OBC_OVERTEMP = 0,
    /* Table Can't Process Fault */
    TDAE_OBC_CC_FAULT,
    TDAE_OBC_CP_DUTY_FAULT,
    TDAE_OBC_CP_VOLT_FAULT,
    TDAE_OBC_CAN_TIMEOUT,
    TDAE_OBC_PFC_FAULT,
    TDAE_OBC_LLC_FAULT,
    TDAE_OBC_ELOCK_FAULT,
    TDAE_OBC_PORT_OPEN_FAULT,
    TDAE_OBC_RT1_OVERTEMP_FAULT,
	TDAE_OBC_WATER_OVERTEMP,
    TDAE_OBC_MAX_ALARM
    
}TE_OBC_ALARM_EVENT;   /* OBC alarm */
    
typedef enum
{
	/* Table Can Process Fault */
    TDAE_OBC_CHARGER_INPUT_UNDERVOLTAGE = 0,
    TDAE_OBC_CHARGER_INPUT_OVERVOLTAGE,
    TDAE_OBC_CHARGER_OUTPUT_UNDERVOLTAGE,
    TDAE_OBC_CHARGER_OUTPUT_OVERVOLTAGE,
    TDAE_OBC_CHARGER_OUTPUT_OVERCURRENT,
    TDAE_OBC_CHARGER_ALARM_TEST,
	TDAE_OBC_CHARGER_LV_UNDERVOLTAGE,
    TDAE_OBC_CHARGER_SINGLE_BAT_OVERVOLTAGE,
    TDAE_OBC_CHARGER_BMS_COM_FAULT,
    TDAE_OBC_CHARGER_AC_FREQ_OVER_FAULT,
    TDAE_OBC_CHARGER_AC_FREQ_UNDER_FAULT,

    TDAE_OBC_CHG_MAX_ALARM
    
}TE_OBC_CHARGER_ALARM_EVENT;   /* OBC alarm */
    
typedef enum
{
	/* Table Can Process Fault */
    TDAE_OBC_DISCHG_INPUT_UNDERVOLTAGE = 0,
    TDAE_OBC_DISCHG_INPUT_OVERVOLTAGE,
    TDAE_OBC_DISCHG_INPUT_OVERCURRENT,
    TDAE_OBC_DISCHG_OUTPUT_UNDERVOLTAGE,
    TDAE_OBC_DISCHG_OUTPUT_OVERVOLTAGE,
    TDAE_OBC_DISCHG_OUTPUT_OVERCURRENT,
    TDAE_OBC_DISCHG_OUTPUT_OVERPOWER,
    TDAE_OBC_DISCHG_AC_SHORT_CIRCUIT_FAULT,
    TDAE_OBC_DISCHG_SINGLE_BAT_UNDERVOLTAGE,
    TDAE_OBC_DISCHG_SINGLE_INSU,
	TDAE_OBC_DISCHG_VCU_COM_FAULT,
    TDAE_OBC_DISCHG_BMS_COM_FAULT,
    TDAE_OBC_DISCHG_MAX_ALARM
    
}TE_OBC_DISCHARGER_ALARM_EVENT;   /* OBC alarm */

typedef enum
{
    OBC_ALARM_FLAG_CC = 0,
    OBC_ALARM_FLAG_CP_DUTY,
    OBC_ALARM_FLAG_CP_VOLT,
    OBC_ALARM_FLAG_CAN,
    OBC_ALARM_FLAG_PFC,
    OBC_ALARM_FLAG_LLC,
    OBC_ALARM_FLAG_ELOCK,
    MAX_OBC_ALARM_FLAG
}TE_OBC_ALARM_FLAG;   /* OBC alarm flag*/

typedef enum
{
    OBC_INPUT_VOLTAGE = 0,
    OBC_INPUT_CURRENT,
    OBC_OUTPUT_VOLTAGE,
    OBC_OUTPUT_CURRENT,
    OBC_OUTPUT_POWER,
    OBC_PFC_VOLTAGE,
    OBC_12V_VOLTAGE,
    OBC_THS_TEMP,
    OBC_AIR_TEMP,
    OBC_RT1_TEMP,
    OBC_RT3_TEMP,
    OBC_MAX_TEMP,
    OBC_ALARM_TEST,

    MAX_OBC_ALARM_DATA
}TE_OBC_ALARM_DATA;   /* OBC alarm */

typedef struct
{
    u08 uwAlarmFlag[MAX_OBC_ALARM_FLAG];
    u16 uwAlarmData[MAX_OBC_ALARM_DATA];
}TS_OBC_ALARM_DATA;

/* Export define -------------------------------------------------------------*/
#if((PRODUCT_CODE == FE2959) ||(PRODUCT_CODE == FE2917))
    #define OBC_INPUT_UV_PROTECT_POINT     800
    #define OBC_INPUT_OV_PROTECT_POINT     2730
    #define OBC_OUTPUT_UV_PROTECT_POINT    1500-50
    #define OBC_OUTPUT_OV_PROTECT_POINT    MAX_SET_VOLT
    #define OBC_OUTPUT_OC_PROTECT_POINT    130

    #define OBC_DISCHG_INPUT_UV_PROTECT_POINT      1750
    #define OBC_DISCHG_INPUT_OV_PROTECT_POINT      MAX_SET_VOLT
    #define OBC_DISCHG_INPUT_OC_PROTECT_POINT      190
#elif((PRODUCT_CODE == FE2928)||(PRODUCT_CODE == FE3001)||(PRODUCT_CODE == FE3065))
    #define OBC_INPUT_UV_PROTECT_POINT     800
    #define OBC_INPUT_OV_PROTECT_POINT     2730
    #define OBC_OUTPUT_UV_PROTECT_POINT    2300
    #define OBC_OUTPUT_OV_PROTECT_POINT    MAX_SET_VOLT
    #define OBC_OUTPUT_OC_PROTECT_POINT    120

    #define OBC_DISCHG_INPUT_UV_PROTECT_POINT      2300
    #define OBC_DISCHG_INPUT_OV_PROTECT_POINT      MAX_SET_VOLT
    #define OBC_DISCHG_INPUT_OC_PROTECT_POINT      190
#else
    #error WRONG PRODUCT_CODE
#endif

#define OBC_RT_OVERTEMPERATRURE         (120+TEMP_OFFSET)
#define OBC_WATER_OVERTEMPERATRURE      (85+TEMP_OFFSET)    /*Reported temperature The shutdown point of temperature drop is 85¡æË®µÀÎÂ¶È½µ¶î¹Ø»úµãÎª85¡æ*/
#define OBC_CHG_PFCOVERTEMPWARNING      (108+TEMP_OFFSET)   /*OBC charge pfc over temperature 108¡æ  warning */
#define OBC_CHG_LLCOVERTEMPWARNING      (110+TEMP_OFFSET)   /*OBC charge llc over temperature 110¡æ  warning */
#define OBC_CHG_WATEROVERTEMPWARNING    (75+TEMP_OFFSET)    /*OBC charge water over temperature 75¡æ  warning */
#define OBC_DIS_PFCOVERTEMPWARNING      (105+TEMP_OFFSET)   /*OBC discharge pfc over temperature 105¡æ  warning */
#define OBC_DIS_LLCOVERTEMPWARNING      (105+TEMP_OFFSET)   /*OBC discharge llc over temperature 105¡æ  warning */
#define OBC_DIS_WATEROVERTEMPWARNING    (68+TEMP_OFFSET)    /*OBC discharge water over temperature 75¡æ  warning */

#define OBC_PLUG_OVERTEMPERATRURE       (85+TEMP_OFFSET)

#define OBC_OVER_TEMP_PROTECT_POINT     850

#define OBC_LV_UV_PROTECT_POINT 680

#define OBC_CHARGER_AC_FREQ_HIGH_PROTECT_POINT 680
#define OBC_CHARGER_AC_FREQ_LOW_PROTECT_POINT 420

#define OBC_DISCHG_OUTPUT_UV_PROTECT_POINT     850
#define OBC_DISCHG_OUTPUT_OV_PROTECT_POINT	   2400
#define OBC_DISCHG_OUTPUT_OC_PROTECT_POINT      165
#define OBC_DISCHG_OUTPUT_OP_PROTECT_POINT1	   3200

#define OBC_DCCHG_INPUT_OV_PROTECT_POINT       2560

/* Export macro --------------------------------------------------------------*/
/* Export variables ----------------------------------------------------------*/
APP_ALARM_OBC_DEC TS_ALARM_MANAGE  tObcAlarm;
APP_ALARM_OBC_DEC TS_ALARM_MANAGE  tObcChargerAlarm;
APP_ALARM_OBC_DEC TS_ALARM_MANAGE  tObcDischargerAlarm;
//APP_ALARM_OBC_DEC TS_OBC_ALARM_DATA    tObcAlarmData;

/* Export function -----------------------------------------------------------*/
APP_ALARM_OBC_DEC void AppObcAlarmInit(void);
APP_ALARM_OBC_DEC void AppObcMaskReset(TE_ALARM_KIND kind, u16 index);
APP_ALARM_OBC_DEC void AppObcMaskSet(TE_ALARM_KIND kind, u16 index);
APP_ALARM_OBC_DEC u08 AppObcMaskGet(TE_ALARM_KIND kind, u16 index);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataTemp(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagCC(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagCpDuty(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagCpVolt(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagComCan(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagPFC(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagLLC(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmPortOpen(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataRT1Temp(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmWaterTemp(void);
APP_ALARM_OBC_DEC u08 AppObcChargerIsRun(void);
APP_ALARM_OBC_DEC u08 AppObcChargerVacIsNormalBYD(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataInputVolt(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataOutputVolt(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataOutputDownVolt(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataOutputUpperVolt(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataInputCurr(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmDataOutputCurr(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmLVVolt(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagChargeBat(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagBmsComFault(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmVacFrequncyBYD(void);
APP_ALARM_OBC_DEC u08 AppObcDisChargerIsRun(void);
APP_ALARM_OBC_DEC u08 AppObcDisChargerInsu(void);
APP_ALARM_OBC_DEC u08 AppObcDisChargerIsCheckAcShortCircuit(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagDisChargePower(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmOutletDisChargeCru(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagDisChargeBat(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagDisChargeInsu(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagVcuComFault(void);
APP_ALARM_OBC_DEC u32 AppObcAlarmFlagPfcAcShortCircuitFault(void);
APP_ALARM_OBC_DEC u08 AppAlarmIsRecoveryFault(void);
APP_ALARM_OBC_DEC u08 AppAlarmIsLockFault(void);
APP_ALARM_OBC_DEC u32 AppGetAllObcAlarmFlag(void);
APP_ALARM_OBC_DEC u32 AppGetAllObcChgAlarmFlag(void);
APP_ALARM_OBC_DEC u32 AppGetAllObcDischgAlarmFlag(void);
APP_ALARM_OBC_DEC u08 AppGetObcAlarmBit(TE_OBC_ALARM_EVENT tObcAlarmEvent);
APP_ALARM_OBC_DEC u08 AppGetObcChargeAlarmBit(TE_OBC_CHARGER_ALARM_EVENT tObcChargerAlarmEvent);
APP_ALARM_OBC_DEC u08 AppGetObcDisChargeAlarmBit(TE_OBC_DISCHARGER_ALARM_EVENT tObcDisChargerAlarmEvent);

#endif /* APP_ALARM_LDC_H_ */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
