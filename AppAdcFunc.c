/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******source file*****
* File name          : AppAdcFun.c
* Author             : R&D SW
* Brief              : ADC Driver module source file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/04/25          R&D SW   First Issue
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "AppAdcFunc.h"
#include "AppAdc.h"
#include "CalibrationPort.h"

#include <string.h>

/* Private macro -------------------------------------------------------------*/
#define MAX_HARDWARE_VERSON 16

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static u16 uAdcData[MAX_ADC_ITEM];
/* Sample voltage reference 3V */
const u16 HardwareSmpValue[MAX_HARDWARE_VERSON] = {136,273,409,546,682,819,955,1092,1228,1365,1501,1638,1774,1911};
const u16 HardwareVerson[MAX_HARDWARE_VERSON] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E};

/* Private function property -------------------------------------------------*/

/*******************************************************************************
  * @brief      :  all initialize of ADC module
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void InitAdcAppData(void)
{
    memset(&uAdcData, 0, sizeof (uAdcData));
}

/*******************************************************************************
  * @brief      :  calculate Hardware Version
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
u16 GetHardWareVersion(u16 uAdcValue)
{
	int i = 0;

	for(i = 0;i < MAX_HARDWARE_VERSON;i++)
	{
		if(ABS(uAdcValue, HardwareSmpValue[i]) <= 50)
		{
			break;
		}
	}

	return HardwareVerson[i];
}

/*******************************************************************************
  * @brief      :  update the measure value
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void UpdateAdcAppValue(void)
{
    static u32 ultemps = 0UL;

    ultemps = ((AppAdcGetSampleValue(LV_12V_VOLTAGE)*100)*30*11)/40960 + 60;
    uAdcData[LV_12V_VOL] = (u16)calPortConversion(CAL_OBC_KL30_VOLT, ultemps);
    uAdcData[HW_VER_VOL] = GetHardWareVersion(AppAdcGetSampleValue(HW_VER));
}

/*******************************************************************************
  * @brief      :  get the measure value
  * @parameter  :  tMeasureItem: the item of measure value
  * @return     :  the measure value
*******************************************************************************/
u16 GetAdcAppValue(TE_ADC_ITEM tAdcItem)
{
    u16 uwResult = 0u;
    
    if (tAdcItem < MAX_ADC_ITEM)
    {
        uwResult = uAdcData[tAdcItem];
    }

    return uwResult;
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/

