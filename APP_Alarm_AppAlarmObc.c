/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
 * File name          : AppObcFsm.c
 * Author             : R&D SW
 * Brief              : Application Obc Finite State Machine
 ********************************************************************************
 * modify
 * Version   Date(YYYY/MM/DD)    Author  Described
 * V1.00     2019/04/24          R&D SW  First Issue
 *******************************************************************************/

#define APP_ALARM_OBC_C

/* Includes ------------------------------------------------------------------*/
#include "AppAlarmObc.h"

#include "Timer.h"
#include "Stm.h"

#include "AppAdcFunc.h"
#include "AppTimer.h"
#include "AppObcConfig.h"
#include "AppCtrlGlobal.h"
#include "AppObcFsm.h"
#include "AppObcChgFsm.h"
#include "AppObcDisChgFsm.h"
#include "AppObcDPC.h"
#include "funcModuleDebug.h"
#include "AppCp.h"
#include "AppCc.h"
#include "AppCanObc.h"
#include "AppSci.h"
#include "AppSciData.h"
#include "AppAlarmLdc.h"
#include "AppTemp.h"
#include "AppPort.h"
#include "AppPortFunc.h"
#include "AppDcdcFsm.h"
#include "AppDcdcSeq.h"
#include "DC_Control.h"

#include "loggers.h"

#include <string.h>

/* Private macro -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DISCHG_PFC_AC_SHORT_CURRENT_MAX                 50      /*  5A  */
#define DISCHG_PFC_AC_SHORT_VOLT_MAX                    500     /* 50V */
#define DISCHG_PFC_AC_SHORT_CIRCUIT_FAULT_COUNT_MAX     10
#define DISCHG_PFC_AC_SHORT_CIRCUIT_FAULT_TIME          (10 * T_S)
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TS_ALARM_MANAGE  tObcAlarm;
TS_ALARM_MANAGE  tObcChargerAlarm;
TS_ALARM_MANAGE  tObcDischargerAlarm;

extern u16 uwInsu_L_Voltage;
extern u16 uwInsu_N_Voltage;
u08 ucAcShortCircuitFlag = 0;   /* AC short circuit fault Flag */
u08 ucAcShortCircuitFaultCnt = 0;  /* AC short circuit fault count */
u32 ulPfcFaultTimes = 0;    /* Interval for triggering AC short circuit faults */

/* Private function property -------------------------------------------------*/
/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AppObcMaskInit(void)
{
    int i;
    for (i = 0; i<TDAE_OBC_MAX_ALARM; i++)
    {
        switch(tObcAlarmMngTable[i].tFaultType)
        {
            case TAT_LOCKFAULT:
                tObcAlarm.uwObcAlarmMaskLock |= (1UL << i);
                break;
            case TAT_RECOVERYFAULT:
                tObcAlarm.uwObcAlarmMaskRecovery |= (1UL << i);
                break;
            default :
                break;
        }
    }
    
    for (i = 0; i<TDAE_OBC_CHG_MAX_ALARM; i++)
    {
        switch(tObcChargerAlarmMngTable[i].tFaultType)
        {
            case TAT_LOCKFAULT:
                tObcChargerAlarm.uwObcAlarmMaskLock |= (1UL << i);
                break;
            case TAT_RECOVERYFAULT:
                tObcChargerAlarm.uwObcAlarmMaskRecovery |= (1UL << i);
                break;
            default :
                break;
        }
    }
    
    for (i=0; i<TDAE_OBC_DISCHG_MAX_ALARM; i++)
    {
        switch(tObcDischargerAlarmMngTable[i].tFaultType)
        {
            case TAT_LOCKFAULT:
                tObcDischargerAlarm.uwObcAlarmMaskLock |= (1UL << i);
                break;
            case TAT_RECOVERYFAULT:
                tObcDischargerAlarm.uwObcAlarmMaskRecovery |= (1UL << i);
                break;
            default :
                break;
        }
    }    

    for (i=0; i<TDAE_DCDC_MAX_ALARM; i++)
    {
        switch(tDcdcAlarmMngTable[i].tFaultType)
        {
            case TAT_LOCKFAULT:
                tDcdcAlarm.uwObcAlarmMaskLock |= (1UL << i);
                break;
            case TAT_RECOVERYFAULT:
                tDcdcAlarm.uwObcAlarmMaskRecovery |= (1UL << i);
                break;
            default :
                break;
        }
    }  
}

/*******************************************************************************
  * @brief      :  Application Obc Alarm Initialize
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppObcAlarmInit(void)
{
	memset(&tObcAlarm, 0, sizeof (TS_ALARM_MANAGE));
    memset(&tObcChargerAlarm, 0, sizeof (TS_ALARM_MANAGE));
    memset(&tObcDischargerAlarm, 0, sizeof (TS_ALARM_MANAGE));

    AppObcMaskInit();
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static u08 AppObcMaskObj(TE_ALARM_KIND kind, u32 ** ppMaskLock, u32 **ppMaskRecovery)
{
    switch(kind)
    {
        case OBC_ALARM_KIND:
            *ppMaskLock = &tObcAlarm.uwObcAlarmMaskLock;
            *ppMaskRecovery = &tObcAlarm.uwObcAlarmMaskRecovery;
            break;
        case OBC_CHARGER_ALARM_KIND:
            *ppMaskLock = &tObcChargerAlarm.uwObcAlarmMaskLock;
            *ppMaskRecovery = &tObcChargerAlarm.uwObcAlarmMaskRecovery;
            break;
        case OBC_DISCHARGER_ALARM_KIND:
            *ppMaskLock = &tObcDischargerAlarm.uwObcAlarmMaskLock;
            *ppMaskRecovery = &tObcDischargerAlarm.uwObcAlarmMaskRecovery;
            break;
        case DCDC_ALARM_KIND:
            *ppMaskLock = &tDcdcAlarm.uwObcAlarmMaskLock;
            *ppMaskRecovery = &tDcdcAlarm.uwObcAlarmMaskRecovery;
            break;
        default :
            ERROR("invalid kind %d", kind);
            return 0;
    } 

    return 1;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppObcMaskReset(TE_ALARM_KIND kind, u16 index)
{
    u32 * pMaskLock = NULL;
    u32 * pMaskRecovery = NULL;

    if(AppObcMaskObj(kind, &pMaskLock, &pMaskRecovery))
    {
        *pMaskLock &= ~(1UL << index);
        *pMaskRecovery |= (1UL << index);
    }
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppObcMaskSet(TE_ALARM_KIND kind, u16 index)
{
    u32 * pMaskLock = NULL;
    u32 * pMaskRecovery = NULL;

    if(AppObcMaskObj(kind, &pMaskLock, &pMaskRecovery))
    {
        *pMaskLock |= (1UL << index);
        *pMaskRecovery &= ~(1UL << index);
    }
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppObcMaskGet(TE_ALARM_KIND kind, u16 index)
{
    u32 * pMaskLock = NULL;
    u32 * pMaskRecovery = NULL;

    if(AppObcMaskObj(kind, &pMaskLock, &pMaskRecovery))
    {
        return *pMaskLock & (1UL << index) ? 1 : 0;
    }

    return 1;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Temp
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataTemp(void)
{
	u32 uwObcTemp = MAX(AppGetTemp(TPFCTEMP),AppGetTemp(TLLCTEMP));
    return uwObcTemp;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagCC(void)
{
#if(CC_CHECK_SEL == CC_CHECK_DONE)
    if(AppGetCcType() == CC_ERROR_SHORT)
    {
        return ALARM_SET;
    }
#endif

    return ALARM_CLEAR;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagCpDuty(void)
{
#if(CP_DUTY_CHECK_SEL == CP_DUTY_CHECK_DONE)
    if((AppGetCpDutyState() == CP_DUTY_ABNORMAL)
            && (AppGetCpFreqState() == CP_FREQ_NORMAL)
            && (AppGetCcType() == CC_AC_CHARGE))
    {
        return ALARM_SET;
    }
	else
	{
		return ALARM_CLEAR;	
	}
#else
    return ALARM_CLEAR;
#endif
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagCpVolt(void)
{
#if(CP_VOLT_CHECK_SEL == CP_VOLT_CHECK_DONE)
    if(AppGetCpVoltState() == CP_VOLT_ABNORMAL && (AppGetCcType() == CC_AC_CHARGE))
    {
        return ALARM_SET;
    }
	else
	{
		return ALARM_CLEAR;	
	}
#else
    return ALARM_CLEAR;
#endif
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagComCan(void)
{
	if(GetObcCanComState() == OBC_CAN_ABNORMAL)
	{
		return ALARM_SET;
	}
    
    return ALARM_CLEAR;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag PFC LLC
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static u08 AppObcAlarmFlagPFCLLC(u32 (* func)(void), u32 goal)
{
    u08 en = 0;
    
	if(AppGetObcMode() == INVERT_MODE)
	{
		en = (ObcGetDisChargerState() == DISCHARGER_STATE_DISCHARGING) && (AppObcGetPfcOnOffState()== PFC_ON);
	}
	else if(AppGetObcMode() == AC_CHARGER_MODE)
	{
		en = (ObcGetChargerState() == CHARGER_STATE_CHARGING) && (AppObcGetLlcOnOffState()== LLC_ON);
	}
    
	if(en)
	{
        if(goal)
        {
            en = (func() == goal) ? ALARM_SET : 2;
        }
        else
        {
            en = (func() != goal) ? ALARM_SET : 2;
        }
	}
	else
	{
        if(goal)
        {
            en = (func() != goal) ? ALARM_CLEAR : 2;
        }
        else
        {
            en = (func() == goal) ? ALARM_CLEAR : 2;
        }
	}

    return en;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagPFC(void)
{
    static u08 alarmLast = 0;

    u08 alarm_ = AppObcAlarmFlagPFCLLC(AppObcGetPfcFault, 0);
    if(2 > alarm_)
    {
        alarmLast = alarm_;
    }
    
    return alarmLast;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagLLC(void)
{
    static u08 alarmLast = 0;

    u08 alarm_ = AppObcAlarmFlagPFCLLC(AppObcGetLlcFault, 0);
    if(2 > alarm_)
    {
        alarmLast = alarm_;
    }
    
    return alarmLast;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmPortOpen(void)
{	
	return ALARM_CLEAR;
/*    if(AppGetPortOpenState() == HIGH)
	{
		return ALARM_SET;
	}
	
	return ALARM_CLEAR;
*/
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data RT1 Temp
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataRT1Temp(void)
{
	u32 uwObcTemp = MAX(AppGetTemp(RT1TEMP),AppGetTemp(RT3TEMP));
    return uwObcTemp;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Water Temp
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmWaterTemp(void)
{
    const u16 uwObcTempDerateChg_v3[] = {83U + TEMP_OFFSET,78U + TEMP_OFFSET};
    const u16 uwObcTempDerateDischg_v3[] = {78U + TEMP_OFFSET,73U + TEMP_OFFSET};

	u32 uwObcTemp = AppGetObcWaterTemp();

    u16 uwObcTempDerateDischgValue = uwObcTempDerateDischg_v3[0];
    u16 uwObcTempDerateChgValue = uwObcTempDerateChg_v3[0];


    if(INVERT_MODE == AppGetObcMode() )
    {
        uwObcTemp += (OBC_WATER_OVERTEMPERATRURE - uwObcTempDerateDischgValue);
    }
    else
    {
        uwObcTemp += (OBC_WATER_OVERTEMPERATRURE - uwObcTempDerateChgValue);
    }

    return uwObcTemp;
}

/*******************************************************************************
  * @brief      :  Application Obc Is Run
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppObcChargerIsRun(void)
{
    if((AppGetObcMode() != AC_CHARGER_MODE) || (AppGetCcType() != CC_AC_CHARGE))
    {
        return TDAE_STATE_FREE;
    }
    
    if(FALSE != fmDebug_getState(FMDEBUG_INDEX_ALARM))
    {
    	u08 debugVal = (u08)fmDebug_readData(FMDEBUG_INDEX_ALARM);
		if(TDAE_STATE_BUSY >= debugVal)
		{
        	return debugVal;
		}
    }
   
    if ((CHARGER_STATE_RECOVERY_FAULT == ObcGetChargerState())
        || (CHARGER_STATE_LOCK_FAULT == ObcGetChargerState()))
	{
		return TDAE_STATE_FAULT;
	}
    else if (AppObcGetLlcOnOffState()== LLC_ON)
    {
        return TDAE_STATE_BUSY;
    }
	else
	{
		return TDAE_STATE_FREE;
	}	
}

/*******************************************************************************
  * @brief      :App Obc Charger VAC Is Normal
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppObcChargerVacIsNormalBYD(void)
{
    if((AppGetObcMode() != AC_CHARGER_MODE)
            || (AppGetCcType() != CC_AC_CHARGE)
            || (AppObcGetInputVol() < OBC_INPUT_UV_PROTECT_POINT))
    {
        return TDAE_STATE_FREE;
    }

    if((CHARGER_STATE_RECOVERY_FAULT == ObcGetChargerState())
            || (CHARGER_STATE_LOCK_FAULT == ObcGetChargerState()))
    {
        return TDAE_STATE_FAULT;
    }
    else if(AppObcGetInputVol() >= OBC_INPUT_UV_PROTECT_POINT)
    {
        return TDAE_STATE_BUSY;
    }
    else
    {
        return TDAE_STATE_FREE;
    }
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Input Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataInputVolt(void)
{
    return AppObcGetInputVol();
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Output Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataOutputVolt(void)
{
    return AppObcGetOutputVolt();
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Output Down Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataOutputDownVolt(void)
{
#if 0
	if(!GetObcCanBmsComState())
	{
		if(OBC_OUTPUT_UV_PROTECT_POINT > (AppCanGetObcVoltLowerLimit()*10-100))
		{
			return AppObcGetOutputVolt() + (OBC_OUTPUT_UV_PROTECT_POINT - (AppCanGetObcVoltLowerLimit()*10-100));
		}
		else
		{
			return AppObcGetOutputVolt();
		}
	}
	else
	{
    	return AppObcGetOutputVolt();
	}
#endif
	/*The lower limit voltage issued by the BMS cannot be lower than the lower limit of the hardware*/
	return AppObcGetOutputVolt();
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Output Upper Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataOutputUpperVolt(void)
{
	if(!GetObcCanBmsComState())
	{
		if(OBC_OUTPUT_OV_PROTECT_POINT > (AppCanGetObcVoltUpperLimit()*10+100))
		{
			return AppObcGetOutputVolt() + (OBC_OUTPUT_OV_PROTECT_POINT - (AppCanGetObcVoltUpperLimit()*10+100));
		}
		else
		{
			return AppObcGetOutputVolt();
		}
	}
	else
	{
    	return AppObcGetOutputVolt();
	}
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Input Curr
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataInputCurr(void)
{
	if(AppGetCcCurr() == CC_CURRENT_10A)
	{
		return AppObcGetInputCurr() + 50;
	}
	else
	{
		return AppObcGetInputCurr();
	}
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data Output Curr
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmDataOutputCurr(void)
{
    return AppObcGetOutputCurr();
}

/*******************************************************************************
  * @brief      :  App Obc Alarm LV Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmLVVolt(void)
{
    return GetAdcAppValue(LV_12V_VOL);
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag Charge Bat
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagChargeBat(void)
{
	u32 ret = ALARM_CLEAR;
	static u32 SingleBatOvervoltageTimes = 0;

	if(AppCanGetSingleBatHighVolt() > (AppCanGetSigleBatVoltUpperLimit()*20))
    {
		if(STM_timeoutCheck(SingleBatOvervoltageTimes, 5 * T_S))
    	{
    		ret = ALARM_SET;
    	}
    }
    else
    {
		STM_timeoutReset(&SingleBatOvervoltageTimes);
		ret = ALARM_CLEAR;
    }

    return ret;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag Bms Com Fault
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagBmsComFault(void)
{
	if(GetObcCanBmsComState() == OBC_CAN_ABNORMAL)
    {
        return ALARM_SET;
    }

    return ALARM_CLEAR;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm AC Volt Frequncy
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmVacFrequncyBYD(void)
{
    return AppObcGetVacFrequncy();
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppObcDisChargerIsRun(void)
{   
    if(AppGetObcMode() != INVERT_MODE)
    {
        return TDAE_STATE_FREE;
    }
    
    if(FALSE != fmDebug_getState(FMDEBUG_INDEX_ALARM))
    {
    	u08 debugVal = (u08)fmDebug_readData(FMDEBUG_INDEX_ALARM);
		if(TDAE_STATE_BUSY >= debugVal)
		{
        	return debugVal;
		}
    }
	
    if ((DISCHARGER_STATE_RECOVERY_FAULT == ObcGetDisChargerState())
        || (DISCHARGER_STATE_LOCK_FAULT == ObcGetDisChargerState()))
	{
		return TDAE_STATE_FAULT;
	}
    else if (AppObcGetPfcOnOffState()== PFC_ON)
    {
        return TDAE_STATE_BUSY;
    }
	else
	{
		return TDAE_STATE_FREE;
	}	
}

/*******************************************************************************
  * @brief      :
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppObcDisChargerInsu(void)
{
    if(AppGetObcMode() == INVERT_MODE)
    {
        return TDAE_STATE_BUSY;
    }
	else
	{
        return TDAE_STATE_FREE;
    }
}

/*******************************************************************************
  * @brief      :  App Obc DisCharger Pfc Ac Short CirCuit Fault Cnt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AppObcDisChargerPfcAcShortCirCuitFaultCnt(void)
{
    static u32 ulLastVtolDisChargeCmd = 0;
    static u08 ucLastAcShortCircuitFaultCnt = 0;
    static u08 ucLastAcShortCircuitFlag = 0;

    /* Check out the AC short circuit fault */
    if ((AppObcGetInputVol() < DISCHG_PFC_AC_SHORT_VOLT_MAX)
            && (AppObcGetInputCurr() > DISCHG_PFC_AC_SHORT_CURRENT_MAX))
    {
        ucAcShortCircuitFlag = 1;
    }
    else
    {
        ucAcShortCircuitFlag = 0;
    }

    if((ucAcShortCircuitFlag != 0)
            && (ucLastAcShortCircuitFlag == 0))
    {
        ucAcShortCircuitFaultCnt++;
    }

    if(ucLastAcShortCircuitFaultCnt != ucAcShortCircuitFaultCnt)
    {
        STM_timeoutReset(&ulPfcFaultTimes);
    }

    /* The count is cleared in non-inverting mode */
    if(AppGetObcMode() != INVERT_MODE)
    {
        ucAcShortCircuitFaultCnt = 0;
    }
    /* Clear the count when a restart command is received */
    if((ulLastVtolDisChargeCmd != DISCHG_FUN_RESTART)
            && (AppCanObcGetBmsVtolDisChargeAllow() == DISCHG_FUN_RESTART))
    {
        ucAcShortCircuitFaultCnt = 0;
    }
    /* Clear counting when the inverter AC voltage becomes normal */
    if(AppObcGetInputVol() >= OBC_DISCHG_OUTPUT_UV_PROTECT_POINT)
    {
        ucAcShortCircuitFaultCnt = 0;
    }
    /* Clear count when faults are not continuously triggered */
    if(STM_timeoutCheck(ulPfcFaultTimes, DISCHG_PFC_AC_SHORT_CIRCUIT_FAULT_TIME))
    {
        ucAcShortCircuitFaultCnt = 0;
    }

    ucLastAcShortCircuitFlag = ucAcShortCircuitFlag;
    ucLastAcShortCircuitFaultCnt = ucAcShortCircuitFaultCnt;
    ulLastVtolDisChargeCmd = AppCanObcGetBmsVtolDisChargeAllow();
}

/*******************************************************************************
  * @brief      :  App Obc DisCharger Is Check Ac Short Circuit
  * @parameter  :  NONE
  * @return     :  TE_OBC_ALARM_STATE
*******************************************************************************/
u08 AppObcDisChargerIsCheckAcShortCircuit(void)
{
    u08 ret;

    /* AC short circuit fault count processing */
    AppObcDisChargerPfcAcShortCirCuitFaultCnt();

    ret = AppObcDisChargerIsRun();

    return ret;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag DisCharge Power
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagDisChargePower(void)
{
	u32 uwObcTemp = ((u32)AppObcGetInputVol() * (u32)AppObcGetInputCurr()); /* + (Get_DCDC_OutputCurrent() * 138 / 10);*/

    if(uwObcTemp > AppObcInvertPowerLimit())
    {
        return ALARM_SET;
    }

    return ALARM_CLEAR;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Outlet DisCharge Cru
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmOutletDisChargeCru(void)
{
	return ALARM_CLEAR;
/*
    if(AppCanObcGetOutletDischargeCru() > 1000)
    {
        return ALARM_SET;
    }

    return ALARM_CLEAR;
*/
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag DisCharge Bat
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagDisChargeBat(void)
{
	u32 ret = ALARM_CLEAR;
	static u32 SingleBatUndervoltageTimes = 0;

    if(AppCanGetSingleBatLowVolt() < (AppCanGetSigleBatVoltLowerLimit()*20))
    {
    	if(++SingleBatUndervoltageTimes > 3)
    	{
    		ret = ALARM_SET;
    	}
    }
    else
    {
    	SingleBatUndervoltageTimes = 0;
		ret = ALARM_CLEAR;
    }

    return ret;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag DisCharge Insu
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagDisChargeInsu(void)
{
    const u16 uwInsuProtection_L = 102U;
    const u16 uwInsuProtection_N = 109U;

    if((DISCHARGER_STATE_DISCHARGING == ObcGetDisChargerState())
        && ((uwInsu_L_Voltage > uwInsuProtection_L) || (uwInsu_N_Voltage > uwInsuProtection_N))
        )
    {
        return ALARM_SET;
    }

    return ALARM_CLEAR;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag Vcu Com Fault
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppObcAlarmFlagVcuComFault(void)
{
	if(GetObcCanVCUComState() == OBC_CAN_ABNORMAL)
    {
        return ALARM_SET;
    }

    return ALARM_CLEAR;
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Flag Pfc Ac Short Circuit Fault
  * @parameter  :  NONE
  * @return     :  Fault setting flag
*******************************************************************************/
u32 AppObcAlarmFlagPfcAcShortCircuitFault(void)
{
    u32 ret = ALARM_CLEAR;

    if ((ucAcShortCircuitFaultCnt <= DISCHG_PFC_AC_SHORT_CIRCUIT_FAULT_COUNT_MAX)
            && (ucAcShortCircuitFlag == 1))
    {
        ret = ALARM_SET;
    }

    return ret;
}

/*******************************************************************************
  * @brief      :  Obc Alarm Is Recovery Fault
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppAlarmIsRecoveryFault(void)
{
	if (/*(tDcdcAlarm.uwObcAlarmFlag & DCDC_ALARM_MASK)
		|| */ (tObcAlarm.uwObcAlarmFlag & tObcAlarm.uwObcAlarmMaskRecovery)
        || (tObcChargerAlarm.uwObcAlarmFlag & tObcChargerAlarm.uwObcAlarmMaskRecovery)
        || (tObcDischargerAlarm.uwObcAlarmFlag & tObcDischargerAlarm.uwObcAlarmMaskRecovery))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*******************************************************************************
  * @brief      :  Obc Alarm Is Lock Fault
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppAlarmIsLockFault(void)
{
	if ((tObcAlarm.uwObcAlarmFlag & tObcAlarm.uwObcAlarmMaskLock)
        || (tObcChargerAlarm.uwObcAlarmFlag & tObcChargerAlarm.uwObcAlarmMaskLock)
        || (tObcDischargerAlarm.uwObcAlarmFlag & tObcDischargerAlarm.uwObcAlarmMaskLock))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*******************************************************************************
  * @brief      :  Application Get the total alarm flag of Obc
  * @parameter  :  NONE
  * @return     :  total alarm flag of Obc
*******************************************************************************/
u32 AppGetAllObcAlarmFlag(void)
{
    return tObcAlarm.uwObcAlarmFlag;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  
*******************************************************************************/
u32 AppGetAllObcChgAlarmFlag(void)
{
    return tObcChargerAlarm.uwObcAlarmFlag;
}

/*******************************************************************************
  * @brief      :  
  * @parameter  :  NONE
  * @return     :  
*******************************************************************************/
u32 AppGetAllObcDischgAlarmFlag(void)
{
    return tObcDischargerAlarm.uwObcAlarmFlag;
}

/*******************************************************************************
  * @brief      :  get alarm bit of OBC
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppGetObcAlarmBit(TE_OBC_ALARM_EVENT tObcAlarmEvent)
{
    u08 ucResult = 0U;  /* no alarm */

    if (tObcAlarm.uwObcAlarmFlag & (0x0001U << tObcAlarmEvent))
    {   /* already alarmed */
        ucResult = 1U;
    }

    return ucResult;
}

/*******************************************************************************
  * @brief      :  get alarm bit of OBC charger
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppGetObcChargeAlarmBit(TE_OBC_CHARGER_ALARM_EVENT tObcChargerAlarmEvent)
{
    u08 ucResult = 0U;  /* no alarm */

    if (tObcChargerAlarm.uwObcAlarmFlag & (0x0001U << tObcChargerAlarmEvent))
    {   /* already alarmed */
        ucResult = 1U;
    }

    return ucResult;
}

/*******************************************************************************
  * @brief      :  get alarm bit of OBC discharger
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppGetObcDisChargeAlarmBit(TE_OBC_DISCHARGER_ALARM_EVENT tObcDisChargerAlarmEvent)
{
    u08 ucResult = 0U;  /* no alarm */

    if (tObcDischargerAlarm.uwObcAlarmFlag & (0x0001U << tObcDisChargerAlarmEvent))
    {   /* already alarmed */
        ucResult = 1U;
    }

    return ucResult;
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/

