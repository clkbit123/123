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
#include "CalAdcOffset.h"
#include "M95640.h"
#include "Lib.h"
#include "AppSciData.h"

/* Private function property -------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  ADC_OFFSET_MAGIC_NUM        0xAA55A55A
#define  OFFSET_VALUE_MASK          0x8000
#define  DEFAULT_OFFSET_VALUE       0x8800
#define  MIN_OFFSET_VALUE           (DEFAULT_OFFSET_VALUE - 174)
#define  MAX_OFFSET_VALUE           (DEFAULT_OFFSET_VALUE + 174)
/* Private variables ---------------------------------------------------------*/

static TS_ADC_OFFSET_OBJ tsAdcOffSetObj;
static TS_ADC_OFFSET_INFO tsAdcOffSetInfo;

static void  AcInputCurrentAdcOffsetSave(void);
/*******************************************************************************
  * @brief      :  AdcOffsetInit
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void  AcInputCurrentAdcOffsetInit(void)
{
    u32 crcCheck = 0;

    memset(&tsAdcOffSetInfo, 0x00, sizeof(TS_ADC_OFFSET_INFO));
    tsAdcOffSetInfo.e2StartAddr = ADC_OFFSET_ADDR_START;
    tsAdcOffSetInfo.magicNum = ADC_OFFSET_MAGIC_NUM;
    tsAdcOffSetInfo.resetFlg = 1;
    tsAdcOffSetInfo.updateInd = 0;
    tsAdcOffSetInfo.updateCnt = 0;
    tsAdcOffSetInfo.defaultValue = DEFAULT_OFFSET_VALUE;
    tsAdcOffSetInfo.minVlaue = MIN_OFFSET_VALUE;
    tsAdcOffSetInfo.maxVlaue = MAX_OFFSET_VALUE;

    EepromReadData(tsAdcOffSetInfo.e2StartAddr, (u08 *)&tsAdcOffSetObj, sizeof(tsAdcOffSetObj));
    crcCheck =  GetCRC32(&tsAdcOffSetObj.OffsetValue,sizeof(tsAdcOffSetObj.OffsetValue));
    if((tsAdcOffSetInfo.magicNum != tsAdcOffSetObj.magicNum ) ||  (tsAdcOffSetObj.Crc != crcCheck ))
    {
        tsAdcOffSetObj.magicNum = tsAdcOffSetInfo.magicNum;
        tsAdcOffSetObj.OffsetValue = tsAdcOffSetInfo.defaultValue;
        tsAdcOffSetObj.Crc = GetCRC32(&tsAdcOffSetObj.OffsetValue,sizeof(tsAdcOffSetObj.OffsetValue));

        EepromWriteData(tsAdcOffSetInfo.e2StartAddr, (u08 *)&tsAdcOffSetObj, sizeof(TS_ADC_OFFSET_OBJ));
    }
}
/*******************************************************************************
  * @brief      :  AcInputCurrentAdcOffsetUpdate
  * @parameter  :  index,Data
  * @return     :  NONE
*******************************************************************************/
void AcInputCurrentAdcOffsetUpdate(void)
{
    u32 offSet = 0;
    u32 updateFlg  = 0;

    offSet = AppObcGetInpuCurrOffset();
    updateFlg  = offSet & OFFSET_VALUE_MASK;

    if(updateFlg)
    {
        if((offSet != tsAdcOffSetObj.OffsetValue)\
                &&(offSet > tsAdcOffSetInfo.minVlaue)\
                &&(offSet < tsAdcOffSetInfo.maxVlaue))
        {
            tsAdcOffSetObj.OffsetValue = offSet;
            tsAdcOffSetInfo.updateInd = 1;
        }
    }
    AcInputCurrentAdcOffsetSave();
}
/*******************************************************************************
  * @brief      :  AcInputCurrentAdcOffsetSave
  * @parameter  :  index
  * @return     :  Data
*******************************************************************************/
static void  AcInputCurrentAdcOffsetSave(void)
{
    if( (1 == tsAdcOffSetInfo.updateInd) && (1 == tsAdcOffSetInfo.resetFlg) )
    {
        tsAdcOffSetObj.magicNum = ADC_OFFSET_MAGIC_NUM;
        tsAdcOffSetObj.Crc = GetCRC32(&tsAdcOffSetObj.OffsetValue,sizeof(tsAdcOffSetObj.OffsetValue));
        EepromWriteData(tsAdcOffSetInfo.e2StartAddr, (u08 *)&tsAdcOffSetObj, sizeof(TS_ADC_OFFSET_OBJ));

        tsAdcOffSetInfo.updateCnt ++;
        tsAdcOffSetInfo.updateInd = 0;
        tsAdcOffSetInfo.resetFlg = 0;
    }
}

/*******************************************************************************
  * @brief      :  AcInputCurrentAdcOffsetGet
  * @parameter  :  index,Data
  * @return     :  NONE
*******************************************************************************/
u32 AcInputCurrentAdcOffsetGet(void)
{
    return tsAdcOffSetObj.OffsetValue;
}


/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
