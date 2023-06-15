/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AppCan.c
* Author             : R&D SW
* Brief              : ADC APP module hearder file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/04/23          R&D SW   First Issue
*******************************************************************************/

#ifndef     ADC_APP_H
#define     ADC_APP_H

#ifdef      ADC_APP_C
#define     ADC_APP_DEC
#else
#define     ADC_APP_DEC  extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* Export typedef -----------------------------------------------------------*/
typedef enum
{
    CC1 = 0,            //CC电阻检测
    CS_OUT_LV1,         //DCDC输出电流检测
    INSU_N_DET,         //VN_G_F335_PIN40,//N线绝缘检测
    INSU_L_DET,         //VL_G_F335_PIN39,//L线绝缘检测
    TAIR,               //内部空气温度检测
    VDCIN_VS_2,         //DCDC原边输入电压检测
    LV_12V_VOLTAGE,     //_12VDT,//12V常电检测
    CPVT,               //CP幅值检测

    THS4 = 8,           //DCDC MOS管温度检测1
    THS3,               //DCDC MOS管温度检测2
    RT5,                //充电枪温度检测
    RT6,                //充电枪温度检测
    WATER,              //THS7,//水道温度检测
    INSU_N_I_DET,       //INSU_N_DET,//外部模拟信号检测1
    DC_VS_1,            //DCDC输出电压检测1
    HW_VER,             //HW_EDITION,//硬件版本(主芯片）

	MAX_MEASURE_ITEM
}TE_MEASURE_ITEM;

/* Export define -------------------------------------------------------------*/
/* Export macro --------------------------------------------------------------*/
/* Export variables ----------------------------------------------------------*/
/* Export function property -------------------------------------------------*/
ADC_APP_DEC void AppAdcInit(void);
ADC_APP_DEC void AppAdcProcess(void);
ADC_APP_DEC u32  AppAdcGetSampleValue(TE_MEASURE_ITEM tItem);
ADC_APP_DEC void AppAdcSetValue(TE_MEASURE_ITEM tItem, u32 Value);
ADC_APP_DEC void AdcStartQueue(TE_MEASURE_ITEM tItem);
ADC_APP_DEC u32  AdcGetValFromResultReg(TE_MEASURE_ITEM tItem);
ADC_APP_DEC u32 AdcGetResultValue(TE_MEASURE_ITEM tItem);

#endif

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
