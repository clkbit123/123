/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AlarmApp.c
* Author             : henry
* Brief              : source file of alarm module
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2016/07/28          henry   First Issue
*******************************************************************************/

#define APP_ALARM_LDC_C

/* Includes ------------------------------------------------------------------*/
#include "AppAlarmLdc.h"

#include "AppAdcFunc.h"
#include "AppCanDcdc.h"
#include "AppSciData.h"
#include "AppTemp.h"
#include "Stm.h"
#include "DC_Control.h"
#include "AppDcdcFsm.h"
#include "AppDcdcSeq.h"

#include <string.h>

/* Private define  -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function property -------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TS_ALARM_MANAGE tDcdcAlarm;

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Init
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppDcdcAlarmInit(void)
{
	memset(&tDcdcAlarm, 0, sizeof(TS_ALARM_MANAGE));
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Enable Check
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppDcdcAlarmEnableCheck(void)
{
	if((TDS_RUNNING == GetDcdcStateMachine()))
	{
		return TDAE_STATE_BUSY;
	}
	else
	{
		if((DCDC_ENABLE != DcdcGetWorkCmd()) && (DCDC_STABILIVOLT != DcdcGetWorkCmd()))
		{
			return TDAE_STATE_FREE;
		}
		else if(tDcdcAlarm.uwObcAlarmFlag & 0x01)
		{
			return TDAE_STATE_FAULT;
		}
		else
		{
			return TDAE_STATE_BUSY;
		}
	}
/*
	if((DCDC_ENABLE != DcdcGetWorkCmd()) && (DCDC_STABILIVOLT != DcdcGetWorkCmd()))
	{
		return TDAE_STATE_FREE;
	}
	else if(tDcdcAlarm.uwObcAlarmFlag & 0x01)
	{
		return TDAE_STATE_FAULT;
	}
	else
	{
		return ((DCDC_ENABLE == DcdcGetWorkCmd()) || (DCDC_STABILIVOLT == DcdcGetWorkCmd())) ? TDAE_STATE_BUSY : TDAE_STATE_FAULT;
	}
*/
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Running Check
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppDcdcAlarmRunningCheck(void)
{
	if(GetDcdcRequestFlag())
	{
		return TDAE_STATE_BUSY;
	}
	else 
	{
		if((DCDC_ENABLE != DcdcGetWorkCmd()) && (DCDC_STABILIVOLT != DcdcGetWorkCmd()))
		{
			return TDAE_STATE_FREE;
		}
		else
		{
			return TDAE_STATE_FAULT;
		}
	}
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Data Obc Output Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmDataObcOutputVolt(void)
{
    return Get_DCDC_InputVoltage()/10;
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Data Dcdc Output Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmDataDcdcOutputVolt(void)
{
    return Get_DCDC_OutputVoltage();
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Data Dcdc Temp
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmDataDcdcTemp(void)
{
	u32 uwObcTemp = MAX(AppGetTemp(THSTEMP),AppGetTemp(THS2TEMP));
    return uwObcTemp;
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Water Temp
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmWaterTemp(void)
{
    u16 uwDcdcTemp_v3[] = {105+TEMP_OFFSET,95+TEMP_OFFSET};
    u32 uwDcdcTemp = AppGetDcdcWaterTemp();


    uwDcdcTemp = uwDcdcTemp + (DCDC_WATER_OVERTEMP - uwDcdcTemp_v3[0] );

    return uwDcdcTemp;
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Flag Output Line Break
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmFlagOutputLineBreak(void)
{
	return (u32)GetDcdcBreakLineState();
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Flag Hw Protect
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmFlagHwProtect(void)
{
    return (Get_DCDC_State() & DISDCSig_BIT);
}

/*******************************************************************************
  * @brief      :  App Dcdc Alarm Flag Output Over Current
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmFlagOutputOverCurrent(void)
{
    return (Get_DCDC_State() & OCPSig_BIT);
}

/*******************************************************************************
  * @brief      :  App Obc Alarm Data LV Volt
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppDcdcAlarmDataLVVolt(void)
{
    return GetAdcAppValue(LV_12V_VOL);
}

/*******************************************************************************
  * @brief      :  get the total alarm flag of DCDC
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u32 AppGetAllDcdcAlarmFlag(void)
{
    return tDcdcAlarm.uwObcAlarmFlag;
}

/*******************************************************************************
  * @brief      :  get alarm bit of OBC
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u08 AppGetDcdcAlarmBit(TE_DCDC_ALARM_EVENT tDcdcAlarmEvent)
{
    u08 ucResult = 0U;  /* no alarm */

    if (tDcdcAlarm.uwObcAlarmFlag & (0x0001U << tDcdcAlarmEvent))
    {   /* already alarmed */
        ucResult = 1U;
    }

    return ucResult;
}

/*******************************************************************************
  * @brief      :  set and clear alarm bit of DCDC
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppSetDcdcAlarmBit(TE_DCDC_ALARM_EVENT tDcdcAlarmEvent, u08 ucAction)
{
    if (tDcdcAlarmEvent < TDAE_DCDC_MAX_ALARM)
    {
        if (ucAction == ALARM_SET)
        {
        	tDcdcAlarm.uwObcAlarmFlag |= (u16)(0x01UL << tDcdcAlarmEvent);
        }
        else if (ucAction == ALARM_CLEAR)
        {
        	tDcdcAlarm.uwObcAlarmFlag &= (u16)(~(0x01UL << tDcdcAlarmEvent));
        }
        tDcdcAlarm.uwObcTimers[tDcdcAlarmEvent] = STM_getMsCount();
    }
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
