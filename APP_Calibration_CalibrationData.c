/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******Source file*****
* File name          : Calibration.c
* Author             : R&D SW
* Brief              : Calibration module source file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/06/19          R&D SW   First Issue
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Calibration.h"
#include "CalibrationPort.h"
#include "AppDcdcSeq.h"
#include "AppSciData.h"
#include "AppTemp.h"
#include "AppCp.h"
#include "AppCanObc.h"
#include "AppCanDcdc.h"
#include "AppAdc.h"
#include "DC_Control.h"
#include "M95640.h"
#include "loggers.h"
#include "AppAdcFunc.h"


/* Private function property -------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static u32 AppCaliGetLvKl30Voltage(void)
{
	return (u32)(GetAdcAppValue(LV_12V_VOL)/10);
}

static u32 AppCaliAppGetThs2Temp(void)
{
	return (u32)AppGetTemp(THS2TEMP);
}

static u32 AppCaliAppGetAirTemp(void)
{
	return (u32)AppGetTemp(AIRTEMP);
}

static u32 AppCaliAppGetRt1Temp(void)
{
	return (u32)AppGetTemp(RT1TEMP);
}

static u32 AppCaliAppGetRt3Temp(void)
{
	return (u32)AppGetTemp(RT3TEMP);
}

static u32 AppCaliAppGetCpDuty(void)
{
	return (u32)AppGetCpDuty();
}

static u32 AppCaliAppGetCpFreq(void)
{
	return (u32)AppGetCpFreq();
}

static u32 AppCaliGetDCDCInputVoltage(void)
{
	return (u32)Get_DCDC_InputVoltage();
}

static u32 AppCaliGetDCDCOutputVoltage(void)
{
	return (u32)Get_DCDC_OutputVoltage();
}

static u32 AppCaliGetDCDCOutputCurrent(void)
{
	return (u32)Get_DCDC_OutputCurrent();
}

static u32 AppCaliObcGetOutputVolt(void)
{
	return (u32)AppObcGetOutputVolt();
}
/*Solve the calibration negative current bug*/
static u32 AppCaliObcGetOutputCurr(void)
{
	s16 temp = AppObcGetOutputCurrCali();
	return temp;
}

static u32 AppCaliObcGetInputVol(void)
{
	return (u32)AppObcGetInputVol();
}

static u32 AppCaliObcGetInpuCurr(void)
{
	return (u32)AppObcGetInputCurr();
}

static u32 AppCaliGetObcDcVolt(void)
{
	return (u32)AppObcGetDcVolt();
}

static u32 AppCaliGetObcDcCurr(void)
{
	return (u32)GetAppObcDcCurr();
}

static u32 AppCaliGetPfcVol(void)
{
	return (u32)AppObcGetPfcVol();
}

static u32 AppCaliGetDcdcVoltage(void)
{
	return (u32)GetDcdcVoltage();
}
static u32 AppCaliGetProcessFlowId(void)
{
    u32 k_value=1;
    u32 b_value=1;
    calPortPara(CAL_FLAG,&k_value,&b_value);
    return (u32)(b_value & 0xFF);
}
static u32 (* funcGetData[])(void) = {
    AppCaliGetProcessFlowId,
	AppCaliObcGetOutputVolt,
	AppCaliObcGetOutputCurr,
	AppCaliObcGetInputVol,
	AppCaliObcGetInpuCurr,
	AppCaliObcGetInpuCurr,		/* Add inverter output currunt calibration */
	AppCaliGetObcDcVolt,
	AppCaliGetObcDcCurr,
	NULL,
	AppCaliGetPfcVol,
	AppCaliAppGetCpDuty,
	AppCaliAppGetCpFreq,
	AppCaliGetLvKl30Voltage,
	AppCaliAppGetRt1Temp,
	AppCaliAppGetRt3Temp,
	AppCaliAppGetThs2Temp,
	AppCaliAppGetAirTemp,
	AppCaliGetDCDCInputVoltage,
	AppCaliObcGetOutputCurr,
	NULL,
	NULL,
	AppCaliGetDCDCOutputVoltage,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	AppCaliGetDCDCOutputCurrent,
	AppCaliGetDcdcVoltage,
	NULL,
};

u32 calibrationDataGet(u08 id, u32 * pValue)
{
	if(NULL != funcGetData[id])
	{
		*pValue = funcGetData[id]();
	}
	else
	{
		return 1;
	}    

    return 0;
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/


