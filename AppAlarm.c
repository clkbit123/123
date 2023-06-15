/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AlarmApp.c
* Author             : henry
* Brief              : source file of alarm module
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2016/07/28          henry   First Issue
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "AppAlarm.h"
#include "loggers.h"
#include "funcModuleDebug.h"

#include "Timer.h"
#include "Stm.h"
#include "AppTimer.h"

#include "DC_Control.h"
#include "AppAlarmLdc.h"
#include "AppAlarmObc.h"
#include "AppSci.h"
#include "AppSciData.h"
#include "AppTemp.h"
#include "AppCc.h"

/* Private define  -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

typedef struct{
    const TE_ALARM_KIND kind;
    const TS_ALARM_TABLE_MANAGE * pObj;
    TS_ALARM_MANAGE * const pData;
} TS_ALARM_PARA;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function property -------------------------------------------------*/
static void AlarmCheck(const TS_ALARM_CHECK *ptCheck);

//static TS_DCDC_ALARM_DATA   tDcdcAlarmData;
//static TS_OBC_ALARM_DATA    tObcAlarmData; 

/* Private variables ---------------------------------------------------------*/
/* Dcdc Alarm Table */
const TS_ALARM_TABLE_MANAGE tDcdcAlarmMngTable[] =
{
    {"DCDC输入欠压", 	TDAE_DCDC_INPUT_UNDERVOLTAGE,   &AppDcdcAlarmEnableCheck,  	500 * T_MS, 200 * T_MS, DCDC_HV_UNDERVOLTAGE,  	100,	 	AppDcdcAlarmDataObcOutputVolt, 	TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"DCDC输入过压", 	TDAE_DCDC_INPUT_OVERVOLTAGE,  	NULL, 						2 * T_S, 	200 * T_MS, DCDC_HV_OVERVOLTAGE,   	100, 	AppDcdcAlarmDataObcOutputVolt, 	TAT_UPLIMIT,   	TAT_RECOVERYFAULT},
    {"DCDC输出欠压", 	TDAE_DCDC_OUTPUT_UNDERVOLTAGE,  &AppDcdcAlarmRunningCheck, 	200 * T_MS,	200 * T_MS, DCDC_LV_UNDERVOLTAGE,  	130, 	AppDcdcAlarmDataDcdcOutputVolt, TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"DCDC输出过压", 	TDAE_DCDC_OUTPUT_OVERVOLTAGE,   NULL,   					200 * T_MS, 200 * T_MS, DCDC_LV_OVERVOLTAGE,   	130, 	AppDcdcAlarmDataDcdcOutputVolt, TAT_UPLIMIT,   	TAT_RECOVERYFAULT},

    {"DCDC水道过温",   TDAE_DCDC_WATER_OVERTEMP,       NULL,                       3 * T_S,    2 * T_S,    DCDC_WATER_OVERTEMP,    10,     AppDcdcAlarmWaterTemp,      TAT_UPLIMIT,    TAT_RECOVERYFAULT},
	{"DCDC输出过流",	TDAE_DCDC_OUTPUT_OVERCURRENT,   NULL,						0, 			200*T_MS,	0, 							0, 	AppDcdcAlarmFlagOutputOverCurrent,	TAT_FLAG,      	TAT_RECOVERYFAULT},
    {"DCDCLV欠压",	TDAE_DCDC_ON12V_UNDERVOLTAGE,   NULL,						200*T_MS, 	200*T_MS, 	  	DCDC_LV_UV_PROTECT_POINT, 	40, 	AppDcdcAlarmDataLVVolt,  		TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"DCDCLV过压",	TDAE_DCDC_ON12V_OVERVOLTAGE,    NULL,						500*T_MS, 	200*T_MS, 	  	DCDC_LV_OV_PROTECT_POINT, 	40, 	AppDcdcAlarmDataLVVolt,  		TAT_UPLIMIT, 	TAT_RECOVERYFAULT},

    {"DCDC过温",   	TDAE_DCDC_TEMP_OVER,            NULL,   					2 * T_S, 	2 * T_S, DCDC_THS_OVERTEMP, 	10, 		AppDcdcAlarmDataDcdcTemp, 		TAT_UPLIMIT,  	TAT_RECOVERYFAULT},
    {"DCDC硬件保护", 	TDAE_DCDC_HARDWARE_PROTECT,		NULL,           			0, 			200*T_MS,	0, 							0,	 	AppDcdcAlarmFlagHwProtect, 		TAT_FLAG,      	TAT_RECOVERYFAULT},
    {"DCDC输出断线",   TDAE_DCDC_OUTPUT_LINE_BREAK,	NULL,						500*T_MS, 	500*T_MS,	0, 							0, AppDcdcAlarmFlagOutputLineBreak, 	TAT_FLAG,   	TAT_RECOVERYFAULT},
};

/* Obc common Alarm Table */
const TS_ALARM_TABLE_MANAGE tObcAlarmMngTable[] =
{
    {"OBC过温",		TDAE_OBC_OVERTEMP,             	NULL, 		2*T_S,	 	2*T_S, 		OBC_RT_OVERTEMPERATRURE,	10, 	AppObcAlarmDataTemp, 	TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
    {"CC故障",		TDAE_OBC_CC_FAULT,              NULL, 		500*T_MS, 	500*T_MS,	0,							0, 	AppObcAlarmFlagCC, 		TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"CP占空比故障",	TDAE_OBC_CP_DUTY_FAULT,    		NULL,       2500*T_MS, 	500*T_MS, 	0,							0, 	AppObcAlarmFlagCpDuty, 	TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"CP电压故障",	TDAE_OBC_CP_VOLT_FAULT,       	NULL,       500 * T_MS, 500 * T_MS, 0,							0, 	AppObcAlarmFlagCpVolt, 	TAT_FLAG, 		TAT_RECOVERYFAULT},

    {"CAN通信故障",	TDAE_OBC_CAN_TIMEOUT,       	NULL,       0,			200*T_MS, 		0,							0, 	AppObcAlarmFlagComCan, 	TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"PFC故障",		TDAE_OBC_PFC_FAULT,            	NULL,       0,			200*T_MS, 		0,							0, 	AppObcAlarmFlagPFC, 	TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"LLC故障",		TDAE_OBC_LLC_FAULT,            	NULL,       0,			200*T_MS, 		0,							0, 	AppObcAlarmFlagLLC, 	TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"电子锁故障",		TDAE_OBC_ELOCK_FAULT,         	NULL,       0,			0, 		0,							0, 	NULL, 					TAT_FLAG, 		TAT_RECOVERYFAULT},

    {"开盖故障",		TDAE_OBC_PORT_OPEN_FAULT,      	NULL,       0,			0, 		0,							0, 	NULL, 					TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"枪座过温故障",	TDAE_OBC_RT1_OVERTEMP_FAULT, 	NULL, 		2*T_S,	 	2*T_S, 	OBC_PLUG_OVERTEMPERATRURE, 	15, NULL, 					TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
	{"水道过温",		TDAE_OBC_WATER_OVERTEMP,    	NULL, 		2*T_S,	 	3*T_S, 	OBC_WATER_OVERTEMPERATRURE,	5, 	AppObcAlarmWaterTemp, 	TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
};

/* Obc charger Alarm Table */
const TS_ALARM_TABLE_MANAGE tObcChargerAlarmMngTable[] =
{
    {"输入欠压", 		TDAE_OBC_CHARGER_INPUT_UNDERVOLTAGE,      	&AppObcChargerIsRun,	500*T_MS,	2*T_S, 	OBC_INPUT_UV_PROTECT_POINT,  	50,	AppObcAlarmDataInputVolt, 	TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"输入过压",		TDAE_OBC_CHARGER_INPUT_OVERVOLTAGE,        	&AppObcChargerIsRun, 	500*T_MS,	2*T_S, 	OBC_INPUT_OV_PROTECT_POINT,  	80, AppObcAlarmDataInputVolt, 	TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
    {"输出欠压",		TDAE_OBC_CHARGER_OUTPUT_UNDERVOLTAGE,      	&AppObcChargerIsRun, 	5*T_S, 		5*T_S, 	OBC_OUTPUT_UV_PROTECT_POINT, 	50, AppObcAlarmDataOutputDownVolt, 	TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"输出过压",		TDAE_OBC_CHARGER_OUTPUT_OVERVOLTAGE,       	&AppObcChargerIsRun, 	500*T_MS,	1*T_S, 	OBC_OUTPUT_OV_PROTECT_POINT, 	50, AppObcAlarmDataOutputUpperVolt, TAT_UPLIMIT, 	TAT_LOCKFAULT},

    {"输出过流",		TDAE_OBC_CHARGER_OUTPUT_OVERCURRENT,       	&AppObcChargerIsRun, 	5*T_S, 		5*T_S, 	OBC_OUTPUT_OC_PROTECT_POINT, 	30, AppObcAlarmDataOutputCurr, 	TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
    {"OBC告警测试",	TDAE_OBC_CHARGER_ALARM_TEST,        		&AppObcChargerIsRun, 	0,     		0, 	  							0, 		0,  NULL, 						TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
    {"LV欠压", 		TDAE_OBC_CHARGER_LV_UNDERVOLTAGE,         	NULL, 					100*T_MS, 	1*T_S, 	OBC_LV_UV_PROTECT_POINT, 		40, AppObcAlarmLVVolt,  		TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"单节电池过压",	TDAE_OBC_CHARGER_SINGLE_BAT_OVERVOLTAGE, 	&AppObcChargerIsRun, 	0, 			200*T_MS,								0, 		0, 	AppObcAlarmFlagChargeBat, 	TAT_FLAG, 		TAT_LOCKFAULT},

    {"BMS通讯故障",	TDAE_OBC_CHARGER_BMS_COM_FAULT, 			&AppObcChargerIsRun, 		0, 		200*T_MS, 								0, 		0, 	AppObcAlarmFlagBmsComFault, 	TAT_FLAG, 		TAT_RECOVERYFAULT},
    {"充电电网频率高",   TDAE_OBC_CHARGER_AC_FREQ_OVER_FAULT,    &AppObcChargerVacIsNormalBYD, 500*T_MS,  500*T_MS,    OBC_CHARGER_AC_FREQ_HIGH_PROTECT_POINT,     20, AppObcAlarmVacFrequncyBYD,  TAT_UPLIMIT,    TAT_RECOVERYFAULT},
    {"充电电网频率低",   TDAE_OBC_CHARGER_AC_FREQ_UNDER_FAULT,   &AppObcChargerVacIsNormalBYD, 500*T_MS,  500*T_MS,    OBC_CHARGER_AC_FREQ_LOW_PROTECT_POINT,      20, AppObcAlarmVacFrequncyBYD,  TAT_DOWNLIMIT,  TAT_RECOVERYFAULT}

};

/* Obc discharger Alarm Table */
const TS_ALARM_TABLE_MANAGE tObcDischargerAlarmMngTable[] =
{
    {"逆变输入欠压",			TDAE_OBC_DISCHG_INPUT_UNDERVOLTAGE,       	&AppObcDisChargerIsRun,		500*T_MS, 	500*T_MS, 	OBC_DISCHG_INPUT_UV_PROTECT_POINT, 	50, AppObcAlarmDataOutputVolt, 		TAT_DOWNLIMIT, 	TAT_RECOVERYFAULT},
    {"逆变输入过压",			TDAE_OBC_DISCHG_INPUT_OVERVOLTAGE,        	&AppObcDisChargerIsRun, 	500*T_MS, 	500*T_MS, 	OBC_DISCHG_INPUT_OV_PROTECT_POINT, 	50, AppObcAlarmDataOutputVolt, 		TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
    {"逆变输入过流",			TDAE_OBC_DISCHG_INPUT_OVERCURRENT,        	&AppObcDisChargerIsRun, 	500*T_MS, 	500*T_MS, 	OBC_DISCHG_INPUT_OC_PROTECT_POINT, 	190, AppObcAlarmDataOutputCurr, 		TAT_UPLIMIT, 	TAT_RECOVERYFAULT},
    {"逆变输出欠压",			TDAE_OBC_DISCHG_OUTPUT_UNDERVOLTAGE,		&AppObcDisChargerIsRun, 	5*T_S, 		5*T_S, 		OBC_DISCHG_OUTPUT_UV_PROTECT_POINT, 50, AppObcAlarmDataInputVolt, 		TAT_DOWNLIMIT, 	TAT_LOCKFAULT},

    {"逆变输出过压",			TDAE_OBC_DISCHG_OUTPUT_OVERVOLTAGE,       	&AppObcDisChargerIsRun, 	5*T_S, 		5*T_S, 		OBC_DISCHG_OUTPUT_OV_PROTECT_POINT, 50, AppObcAlarmDataInputVolt, 		TAT_UPLIMIT, 	TAT_LOCKFAULT},
    {"逆变输出过流",			TDAE_OBC_DISCHG_OUTPUT_OVERCURRENT,       	&AppObcDisChargerIsRun, 	3000*T_MS, 	500*T_MS, 	OBC_DISCHG_OUTPUT_OC_PROTECT_POINT, 5, 	AppObcAlarmDataInputCurr, 		TAT_UPLIMIT, 	TAT_LOCKFAULT},
    {"逆变输出过功率 ",			TDAE_OBC_DISCHG_OUTPUT_OVERPOWER,       	&AppObcDisChargerIsRun,  	1000*T_MS, 	500*T_MS,	0, 									0, 	AppObcAlarmFlagDisChargePower, 	TAT_FLAG, 		TAT_LOCKFAULT},
    {"逆变AC短路故障",        TDAE_OBC_DISCHG_AC_SHORT_CIRCUIT_FAULT,     &AppObcDisChargerIsCheckAcShortCircuit,  3*T_S,      0,          0,                      0,  AppObcAlarmFlagPfcAcShortCircuitFault,  TAT_FLAG,       TAT_RECOVERYFAULT},

    {"单节电池电压放电欠压",		TDAE_OBC_DISCHG_SINGLE_BAT_UNDERVOLTAGE, 	&AppObcDisChargerIsRun, 	0,      200*T_MS,   0, 									0, 	AppObcAlarmFlagDisChargeBat,	TAT_FLAG, 		TAT_LOCKFAULT},
	{"绝缘检测告警",			TDAE_OBC_DISCHG_SINGLE_INSU,           		&AppObcDisChargerInsu, 		500*T_MS, 	800*T_MS, 	0, 									0, 	AppObcAlarmFlagDisChargeInsu, 	TAT_FLAG, 		TAT_LOCKFAULT},
    {"VCU通讯故障",         TDAE_OBC_DISCHG_VCU_COM_FAULT,              &AppObcDisChargerIsRun,     0,          200*T_MS,   0,                                  0,  AppObcAlarmFlagVcuComFault,     TAT_FLAG,       TAT_RECOVERYFAULT},
    {"BMS通讯故障",         TDAE_OBC_DISCHG_BMS_COM_FAULT,              &AppObcDisChargerIsRun,     0,          200*T_MS,   0,                                  0,  AppObcAlarmFlagBmsComFault,     TAT_FLAG,       TAT_RECOVERYFAULT}
};

/* Check Table */
static TS_ALARM_CHECK tAlarmCheckTable[] =
{
	{OBC_ALARM_KIND,            &tObcAlarm,          tObcAlarmMngTable,            sizeof(tObcAlarmMngTable)/sizeof(TS_ALARM_TABLE_MANAGE)},
	{OBC_CHARGER_ALARM_KIND,    &tObcChargerAlarm,   tObcChargerAlarmMngTable,     sizeof(tObcChargerAlarmMngTable)/sizeof(TS_ALARM_TABLE_MANAGE)},
	{OBC_DISCHARGER_ALARM_KIND, &tObcDischargerAlarm,tObcDischargerAlarmMngTable,  sizeof(tObcDischargerAlarmMngTable)/sizeof(TS_ALARM_TABLE_MANAGE)},
    {DCDC_ALARM_KIND,           &tDcdcAlarm,         tDcdcAlarmMngTable,           sizeof(tDcdcAlarmMngTable)/sizeof(TS_ALARM_TABLE_MANAGE)},
};

/*******************************************************************************
  * @brief      :  data initialize of alarm process.
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppAlarmInit(void)
{
	AppObcAlarmInit();
	AppDcdcAlarmInit();
}

/*******************************************************************************
  * @brief      :  data initialize of alarm process
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppAlarmProcess(void)
{
    u32 i;
    u32 size = sizeof(tAlarmCheckTable) / sizeof(tAlarmCheckTable[0]);
    for(i = 0; i < size; i++)
    {
        AlarmCheck(&tAlarmCheckTable[i]);
    }
}
#if 0
/*******************************************************************************
  * @brief      :  App Alarm Data Update
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AppAlarmDataUpdate(void)
{
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_CC] = AppObcAlarmFlagCC();
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_CP_DUTY] = AppObcAlarmFlagCpDuty();
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_CP_VOLT] = AppObcAlarmFlagCpVolt();
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_CAN] = AppObcAlarmFlagComCan();
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_PFC] = AppObcAlarmFlagPFC();
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_LLC] = AppObcAlarmFlagLLC();
    tObcAlarmData.uwAlarmFlag[OBC_ALARM_FLAG_ELOCK] = 0;
    //obc alarm data update
	tObcAlarmData.uwAlarmData[OBC_INPUT_VOLTAGE] = AppObcGetInputVol();
    tObcAlarmData.uwAlarmData[OBC_INPUT_CURRENT] = AppObcGetInputCurr();
    
    if(FALSE != fmDebug_getState(FMDEBUG_INDEX_ALARM))
    {
    	u16 debugVal = (u16)fmDebug_readData(FMDEBUG_INDEX_ALARM);
		if(TDAE_STATE_BUSY < debugVal)
		{
	        tObcAlarmData.uwAlarmData[OBC_OUTPUT_VOLTAGE] = debugVal;
		}
    }
    else
    {
	    tObcAlarmData.uwAlarmData[OBC_OUTPUT_VOLTAGE] = AppObcGetOutputVolt();
    }
    
    tObcAlarmData.uwAlarmData[OBC_OUTPUT_CURRENT] = AppObcGetOutputCurr();

    tObcAlarmData.uwAlarmData[OBC_MAX_TEMP] = MAX(AppGetTemp(THSTEMP),AppGetTemp(THS2TEMP));

    u32 power = AppObcGetOutputVolt() * AppObcGetOutputCurr() / 100;    
    if(RESISTANCE_2000 == AppGetCcRes())
    {
        power += OBC_DISCHG_OUTPUT_OP_PROTECT_POINT2 - OBC_DISCHG_OUTPUT_OP_PROTECT_POINT1;
    }
    tObcAlarmData.uwAlarmData[OBC_OUTPUT_POWER] = (u16)power;

    //dcdc alarm data update
    tDcdcAlarmData.uwAlarmData[DCDC_INPUT_VOLTAGE] = AppObcGetOutputVolt();
    tDcdcAlarmData.uwAlarmData[DCDC_OUTPUT_VOLTAGE] = (u16)Get_DCDC_OutputVoltage();
    tDcdcAlarmData.uwAlarmData[DCDC_OUTPUT_CURRENT] = (u16)Get_DCDC_OutputCurrent() / 10;
    tDcdcAlarmData.uwAlarmFlag[DCDC_FLAG_HW_PROTECT] = (Get_DCDC_State() & DISDCSig_BIT);
    tDcdcAlarmData.uwAlarmFlag[DCDC_FLAG_OVERCURRENT] = (Get_DCDC_State() & OCPSig_BIT);
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static inline void AlarmTimeInit(u32 * timer)
{
    *timer = STM_getMsCount();
}
#endif
/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static inline u08 AlarmTimeCheck(u32 * timer, u32 timeout)
{
    if((STM_getMsCount() - *timer) > timeout)
    {
        *timer = STM_getMsCount();
        return 1;
    }

    return 0;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static inline u08 AlarmFlgGet(u32 *flg, u16 index)
{
    return (*(flg + index / 32) & (1UL << (index % 32))) ? 1 : 0;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static inline void AlarmFlgSet(u32 *flg, u16 index)
{
    *(flg + index / 32) |= (1UL << (index % 32));
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static inline void AlarmFlgReset(u32 *flg, u16 index)
{
    *(flg + index / 32) &= ~(1UL << (index % 32));
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static TE_DATA_CHECK AlarmCheckData(const TS_ALARM_TABLE_MANAGE * pObj)
{
    u32 data = pObj->pDataGet();
    TE_DATA_CHECK result = TAT_DATA_CHECKING;
    
    switch (pObj->tType)
    {
        case TAT_UPLIMIT:
            if(data >= pObj->uwLimit)
            {
                result = TAT_DATA_CHECK_UP;
            }
            else if(data <= (pObj->uwLimit - pObj->uwHysteresis))
            {
                result = TAT_DATA_CHECK_DOWN;
            }
            else
            {}
            break;
        case TAT_DOWNLIMIT:
            if(data <= pObj->uwLimit)
            {
                result = TAT_DATA_CHECK_UP;
            }
            else if(data >= (pObj->uwLimit + pObj->uwHysteresis))
            {
                result = TAT_DATA_CHECK_DOWN;
            }
            else
            {}
            break;
        case TAT_FLAG:
            if(data)
            {
                result = TAT_DATA_CHECK_UP;
            }
            else
            {
                result = TAT_DATA_CHECK_DOWN;
            }
            break;
        default:
            break;
    }

    return result;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AlarmCheckSetUp(TS_ALARM_PARA * para)
{
    ERROR("alarm %d[%s] SET", para->pObj->ucEvent, para->pObj->name);
    AlarmFlgSet(&para->pData->uwObcAlarmFlag, para->pObj->ucEvent);
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static u08 AlarmCheckSet(TS_ALARM_PARA * para)
{
    if(TAT_DATA_CHECK_UP == AlarmCheckData(para->pObj))
    {
        if(AlarmTimeCheck(&para->pData->uwObcTimers[para->pObj->ucEvent], para->pObj->uwTimeoutSet))
        {
            AlarmCheckSetUp(para);
            return FALSE;
        }
        
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AlarmCheckResetAction(TS_ALARM_PARA * para)
{
    INFO("alarm %d[%s] CLEAR", para->pObj->ucEvent, para->pObj->name);
    AlarmFlgReset(&para->pData->uwObcAlarmFlag, para->pObj->ucEvent);
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static u08 AlarmCheckReset(TS_ALARM_PARA * para)
{
    if(TAT_DATA_CHECK_DOWN == AlarmCheckData(para->pObj))
    {
        if (AlarmTimeCheck(&para->pData->uwObcTimers[para->pObj->ucEvent], para->pObj->uwTimeoutRst))
        {
            AlarmCheckResetAction(para);
            return FALSE;
        }
        
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************************
  * @brief      :  check the alarm
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AlarmCheck(const TS_ALARM_CHECK *ptCheck)
{
#if 1
    u08 i;
    TS_ALARM_PARA para = {ptCheck->kind, 0, ptCheck->pAlarm};
    
    for (i = 0; i < ptCheck->ucMaxEvent; i++)
    {
        if(NULL == ptCheck->pTable[i].pDataGet)
        {
            continue;
        }
        
        para.pObj = &ptCheck->pTable[i];
        
        if(AlarmFlgGet(&para.pData->uwObcAlarmFlag, para.pObj->ucEvent))
        {
            if((para.pObj->pPrecondition == NULL) ? 1 : (TDAE_STATE_FAULT == para.pObj->pPrecondition()))
            {
                if(TAT_RECOVERYFAULT == para.pObj->tFaultType)
                {
                    if (TRUE == AlarmCheckReset(&para))
                    {
                        continue;
                    }
                }
            }
            else if(TDAE_STATE_FREE == para.pObj->pPrecondition())
            {
                AlarmCheckResetAction(&para);
            }
        }
        else if((para.pObj->pPrecondition == NULL) ? 1 : (TDAE_STATE_BUSY == para.pObj->pPrecondition()))
        {
            if (TRUE == AlarmCheckSet(&para))
            {
                continue;
            }
        }

        para.pData->uwObcTimers[para.pObj->ucEvent]  = STM_getMsCount();
    }
#else    
    for (u08 i = 0; i < ptCheck->ucMaxEvent; i++)
    {
        if(NULL == ptTable[i].pData)
        {
            continue;
        }
        
        if ((ptTable[i].pPrecondition == NULL)   
         ||((ptTable[i].pPrecondition != NULL) && ptTable[i].pPrecondition()))
        {
            switch (ptTable[i].tType)
            {
            case TAT_UPLIMIT:
                if (*ptTable[i].pData >= ptTable[i].uwLimit)
                {
                    if (STM_getMsCount() - ptCheck->puwTimers[i] >= ptTable[i].uwTimeout)
                    {
                        *ptCheck->puwAlarmFlag |= (u16)(0x0001UL << ptTable[i].ucEvent);
                         ptCheck->puwTimers[i] = STM_getMsCount();
                    }
                }
                else if (*ptCheck->puwAlarmFlag & (0x0001UL << ptTable[i].ucEvent))
                {
                    if ((*ptTable[i].pData <= ptTable[i].uwLimit - ptTable[i].uwHysteresis)
                     &&(ptTable[i].tFaultType == TAT_RECOVERYFAULT))
                    {
                        if (STM_getMsCount() - ptCheck->puwTimers[i] >= ptTable[i].uwTimeout)
                        {
                            if ((pNoClearAlarm == NULL) || ((pNoClearAlarm != NULL) && !pNoClearAlarm()))
                            {
                                *ptCheck->puwAlarmFlag &= (u16)(~(0x0001UL << ptTable[i].ucEvent));
                                ptCheck->puwTimers[i] = STM_getMsCount();
                            }
                        }
                    }
                }
                else
                {
                    ptCheck->puwTimers[i] = STM_getMsCount();
                }
                break;
            case TAT_DOWNLIMIT:
                if (*ptTable[i].pData <= ptTable[i].uwLimit)
                {
                    if (STM_getMsCount() - ptCheck->puwTimers[i] >= ptTable[i].uwTimeout)
                    {
                        *ptCheck->puwAlarmFlag |= (u16)(0x0001UL << ptTable[i].ucEvent);
                        ptCheck->puwTimers[i] = STM_getMsCount();
                    }
                }
                else if (*ptCheck->puwAlarmFlag & (0x0001UL << ptTable[i].ucEvent))
                {
                    if ((*ptTable[i].pData >= ptTable[i].uwLimit + ptTable[i].uwHysteresis)
                      &&(ptTable[i].tFaultType == TAT_RECOVERYFAULT))
                    {
                        if (STM_getMsCount() - ptCheck->puwTimers[i] >= ptTable[i].uwTimeout)
                        {
                            if ((pNoClearAlarm == NULL) || ((pNoClearAlarm != NULL) && !pNoClearAlarm()))
                            {
                                *ptCheck->puwAlarmFlag &= (u16)(~(0x0001UL << ptTable[i].ucEvent));
                                ptCheck->puwTimers[i] = STM_getMsCount();
                            }
                        }
                    }
                }
                else
                {
                    ptCheck->puwTimers[i] = STM_getMsCount();
                }
                break;
            default:
                break;
            }
        }
        else if ((ptTable[i].pPrecondition != NULL) && (!ptTable[i].pPrecondition()))
        {
            if ((pNoClearAlarm == NULL) || ((pNoClearAlarm != NULL) && !pNoClearAlarm()))
            {
                *ptCheck->puwAlarmFlag &= (u16)(~(0x0001UL << ptTable[i].ucEvent));
                ptCheck->puwTimers[i] = STM_getMsCount();
            }
        }
    }
#endif
}

//因为CC故障需要结合的流程比较多，
//因此使用CAN通信故障替代检测也是可以的。
void AlarmCheck_test(u32 index)
{
    switch(index)
    {
        case 0:
        default:
            ERROR("非法索引 %d", index);
            break;
    }
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/

