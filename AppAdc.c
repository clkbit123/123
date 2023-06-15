/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******source file*****
* File name          : AppAdc.c
* Author             : R&D SW
* Brief              : ADC Driver module source file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/04/25          R&D SW   First Issue
*******************************************************************************/

#define     ADC_APP_C

/* Includes ------------------------------------------------------------------*/
#include "AppAdc.h"

//#include "ConfigurationIsr.h"

#include "AdcDriver.h"
#include "AppAdcFunc.h"

#include <string.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    TE_MEASURE_ITEM tMeasureItem;
    Tye_AdcDriverItem tDriverItem;
}TS_ADC_MANAGE_TABLE;

typedef struct
{
	volatile u32 AdcAverageValue[MAX_MEASURE_ITEM];          /* single measure value */
	volatile u32 AdcResult[MAX_MEASURE_ITEM];
	volatile u32 AdcSum[MAX_MEASURE_ITEM];
}TS_ADC_SAMPLE_DATA;

typedef struct
{
    u16 uwMeasure[MAX_MEASURE_ITEM];
}TS_MEASURE_DATA;

/* Private define ------------------------------------------------------------*/
#define ADC_MANAGE_NUMBER       (sizeof (tAdcTable) / sizeof (TS_ADC_MANAGE_TABLE))
#define ADC_AVG_CNT             50
#define ADC_AVG_CNT_CC1         5

/* Private macro -------------------------------------------------------------*/
/* Private function property -------------------------------------------------*/
static void AdcAutoScanRun(void);

/* Private variables ---------------------------------------------------------*/
static TS_ADC_SAMPLE_DATA tSampleData;

const TS_ADC_MANAGE_TABLE tAdcTable[] =
{
    {CC1,               Driver_CC1},
    {CS_OUT_LV1,        Driver_CS_OUT_LV1},
    {INSU_N_DET,        Driver_VN_G_F335_PIN40},/***/
    {INSU_L_DET,        Driver_VL_G_F335_PIN39},/***/
    {TAIR,              Driver_TAIR},
    {VDCIN_VS_2,        Driver_VDCIN_VS_2},
    {LV_12V_VOLTAGE,    Driver_12VDT},/***/
    {CPVT,              Driver_CPVT},/***/

    {THS4,              Driver_THS4},
    {THS3,              Driver_THS3},
    {RT5,               Driver_RT5},
    {RT6,               Driver_RT6},
    {WATER,             Driver_THS7},/***/
    {INSU_N_I_DET,      Driver_INSU_N_DET},/***/
    {DC_VS_1,           Driver_DC_VS_1},
    {HW_VER,            Driver_HW_EDITION}/***/
};

/*******************************************************************************
  * @brief      :  Init Adc App Sample Data
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void InitAdcAppSampleData(void)
{
    memset(&tSampleData, 0, sizeof(TS_ADC_SAMPLE_DATA));
}
/*******************************************************************************
  * @brief      :  all initialize of ADC Module
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppAdcInit(void)
{

    AdcGroupInit();
//	AdcChannelInit();

    InitAdcAppSampleData();
	InitAdcAppData();
}

/*******************************************************************************
  * @brief      :  ADC process
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppAdcProcess(void)
{
    AdcReadSamplingResults();

	AdcAutoScanRun();

	UpdateAdcAppValue();
}

/*******************************************************************************
  * @brief      :  get the sample value
  * @parameter  :  tItem: the item of measure value
  * @return     :  the sample value
*******************************************************************************/
u32 AppAdcGetSampleValue(TE_MEASURE_ITEM tItem)
{
    u32 uwResult = 0u;
    u08 i = 0;

    for (i = 0; i < ADC_MANAGE_NUMBER; i++)
    {
        if (tAdcTable[i].tMeasureItem == tItem)
        {
            break;
        }
    }

    if (i < ADC_MANAGE_NUMBER)
    {
    	if(tItem < MAX_MEASURE_ITEM)
    	{
    		uwResult = tSampleData.AdcAverageValue[tItem];
    		return uwResult;
    	}
    	else
    	{
    		return 0;
    	}
    }

    if (tItem < MAX_MEASURE_ITEM)
    {
//        uwResult = tSampleData.AdcAverageValue[tAdcTable[i].tAdcChannelId];
    }

    return uwResult;
}

/*******************************************************************************
  * @brief      :  set the sample value
  * @parameter  :  tItem: the item of sample value
  * @parameter  :  Value: sample value
  * @return     :  none
*******************************************************************************/
void AppAdcSetValue(TE_MEASURE_ITEM tItem, u32 Value)
{
    if (tItem < MAX_MEASURE_ITEM)
    {
        return ;
    }
    else
    {
    	tSampleData.AdcAverageValue[tItem] = Value;
    }
}

/*******************************************************************************
  * @brief      :  ADC Auto Scan run, get value from results register
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void AdcAutoScanRun(void)
{
    u32 chnIx;
    u08 ucMeasureItem;

    /* check results group0*/
//    u16 results[22];
//    AdcScanAllSofeWareTrigger();
//    AdcReadResult(results,22);

    for (chnIx = 0; chnIx < ADC_MANAGE_NUMBER; ++chnIx)
    {
        ucMeasureItem = tAdcTable[chnIx].tMeasureItem;
        tSampleData.AdcResult[ucMeasureItem]= AdcReadResult(tAdcTable[chnIx].tDriverItem);
        tSampleData.AdcSum[ucMeasureItem] = tSampleData.AdcSum[ucMeasureItem]
                                            + tSampleData.AdcResult[ucMeasureItem] - tSampleData.AdcAverageValue[ucMeasureItem];
        tSampleData.AdcAverageValue[ucMeasureItem] = tSampleData.AdcSum[ucMeasureItem] / ADC_AVG_CNT;
    }
}

/*******************************************************************************
  * @brief      :  Adc Start Queue
  * @parameter  :  tItem: the item of sample value
  * @return     :  none
*******************************************************************************/
void AdcStartQueue(TE_MEASURE_ITEM tItem)
{
	u16 i = 0;

    for (i = 0; i < ADC_MANAGE_NUMBER; i++)
    {
        if (tAdcTable[i].tMeasureItem == tItem)
        {
            break;
        }
    }

    if (i >= ADC_MANAGE_NUMBER)
    {
    	return;
    }

//    if(tAdcTable[i].tGroupId == IfxVadc_GroupId_0)
//    {
//    	IfxVadc_Adc_startQueue(&g_VadcAutoScan.adcGroup0);
//    }
//    else if(tAdcTable[i].tGroupId == IfxVadc_GroupId_1)
//    {
    	//IfxVadc_Adc_startQueue(&g_VadcAutoScan.adcGroup1);
//    }
//    else
//    {
//    	;
//    }
}

/*******************************************************************************
  * @brief      :  Adc Start Queue
  * @parameter  :  tItem: the item of sample value
  * @return     :  none
*******************************************************************************/
u32 AdcGetValFromResultReg(TE_MEASURE_ITEM tItem)
{
	u32 result = 0;
	u16 i = 0;

    for (i = 0; i < ADC_MANAGE_NUMBER; i++)
    {
        if (tAdcTable[i].tMeasureItem == tItem)
        {
            break;
        }
    }

    if (i >= ADC_MANAGE_NUMBER)
    {
    	return 0;
    }

//	if(tAdcTable[i].tGroupId == IfxVadc_GroupId_0)
	{
		//IfxVadc_Adc_getGroupResult(&g_VadcAutoScan.adcGroup0, &conversionResult, tAdcTable[i].tAdcChannelId, 1);
		//if(conversionResult.B.VF)
		{
			//result = conversionResult.B.RESULT;
		}
	}
//	else if(tAdcTable[i].tGroupId == IfxVadc_GroupId_1)
	{
		//IfxVadc_Adc_getGroupResult(&g_VadcAutoScan.adcGroup1, &conversionResult, tAdcTable[i].tAdcChannelId, 1);
		//if(conversionResult.B.VF)
		{
			//result = conversionResult.B.RESULT;
		}
	}
//	else
//	{
//		;
//	}

	return result;
}

u32 AdcGetResultValue(TE_MEASURE_ITEM tItem)
{
	return tSampleData.AdcResult[tItem];
}
/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/




