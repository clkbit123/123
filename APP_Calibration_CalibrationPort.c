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
#include "CalibrationPort.h"

#include "global.h"
#include "M95640.h"
#include "loggers.h"
#include "AppCanBitlib.h"

#include <stdio.h>
#include <string.h>

extern u32 crc32(const char* s, int len);

/* Private macro -------------------------------------------------------------*/
#define SET_BITU08(reg,bit)    (reg |= (unsigned char)(1UL<<bit))
#define CLR_BITU08(reg,bit)    (reg &= (unsigned char)(~(1UL<<bit)))

/* Private define ------------------------------------------------------------*/
#define APP_DID_MAGIC_MUM 0xA5
#define APP_DID_DATA_SIZE 72

/* Private typedef ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const TS_CALIBRATION_OBJ calibrationObjDefault = {
    CALIBRATION_MAGIC_NUM,
    CALIBRATION_VERSION,
    {{1000, 0},/*CAL_FLAG*/
     {1000, 0},/*CAL_OBC_OUT_VOLT*/
     {1000, 0},/*CAL_OBC_OUT_CURR*/
     {1000, 0},/*CAL_OBC_IN_VOLT*/
     {1000, 0},/*CAL_OBC_IN_CURR*/
     {1000, 0},/*CAL_OBC_INV_OUT_CURR*/
     {1000, 0},/*CAL_OBC_SET_VOLT*/
     {1000, 0},/*CAL_OBC_SET_CURR*/
     {1000, 0},/*CAL_OBC_BAT_VOLT*/
     {1000, 0},/*CAL_OBC_PFC_VOLT*/
     {1000, 0},/*CAL_OBC_CP_DUTY_CYCLE*/
     {1000, 0},/*CAL_OBC_CP_AMPLITUDE*/
     {1000, 0},/*CAL_OBC_KL30_VOLT*/
     {1000, 0},/*CAL_OBC_MUZZLE_TEMP_ONE*/
     {1000, 0},/*CAL_OBC_MUZZLE_TEMP_THREE*/
     {1000, 0},/*CAL_OBC_CORE_TEMP_TWO*/
     {1000, 0},/*CAL_OBC_AIR_TEMP*/
     {1000, 0},/*CAL_DC_IN_VOLT*/
     {1000, 0},/*CAL_DC_IN_CURR*/
     {1000, 0},
     {1000, 0},
     {1000, 0},/*CAL_DC_OUT_VOLT_ONE*/
     {1000, 0},
     {1000, 0},
     {1000, 0},
     {1000, 0},
     {1000, 0},
     {1000, 0},/*CAL_DC_PRECHG_IN_CURR*/
     {1000, 0},/*CAL_DC_OUT_CURR*/
     {1000, 0},/*CAL_DC_OUT_VOLT_SETTING*/
     {1000, 0} /*CAL_DC_PEAK_OUT_VOLT_SETTING*/},
    0
};

static const TS_APP_DID_OBJ appDidObjTab[] =
{
    {"factory station 1",   APP_DID_INDEX_FACTORY_STATION1,         8,      16},
    {"factory station 2",   APP_DID_INDEX_FACTORY_STATION2,         24,     16},
    {"factory station 3",   APP_DID_INDEX_FACTORY_STATION3,         40,     16},
    {"factory station 4",   APP_DID_INDEX_FACTORY_STATION4,         56,     16},
    {"factory station 5",   APP_DID_INDEX_FACTORY_STATION5,         72,     16},
    {"factory station 6",   APP_DID_INDEX_FACTORY_STATION6,         88,     16},
    {"factory station 7",   APP_DID_INDEX_FACTORY_STATION7,         104,    16},
    {"factory station 8",   APP_DID_INDEX_FACTORY_STATION8,         120,    16},
    {"factory station 9",   APP_DID_INDEX_FACTORY_STATION9,         136,    16},
    {"factory station 10",  APP_DID_INDEX_FACTORY_STATION10,        152,    16},
    {"factory station 11",  APP_DID_INDEX_FACTORY_STATION11,        168,    16},
    {"factory station 12",  APP_DID_INDEX_FACTORY_STATION12,        184,    16},
    {"factory station 13",  APP_DID_INDEX_FACTORY_STATION13,        200,    16},
    {"factory station 14",  APP_DID_INDEX_FACTORY_STATION14,        216,    16},
    {"factory station 15",  APP_DID_INDEX_FACTORY_STATION15,        232,    16},
    {"factory station 16",  APP_DID_INDEX_FACTORY_STATION16,        248,    16},
    {"MES ID",              APP_DID_INDEX_MES_ID,                   264,    32 * 8},
};

const TS_FACTORY_STATION station[] =
{
    {1, APP_DID_INDEX_FACTORY_STATION1},
    {2, APP_DID_INDEX_FACTORY_STATION2},
    {3, APP_DID_INDEX_FACTORY_STATION3},
    {4, APP_DID_INDEX_FACTORY_STATION4},
    {5, APP_DID_INDEX_FACTORY_STATION5},
    {6, APP_DID_INDEX_FACTORY_STATION6},
    {7, APP_DID_INDEX_FACTORY_STATION7},
    {8, APP_DID_INDEX_FACTORY_STATION8},
    {9, APP_DID_INDEX_FACTORY_STATION9},
    {10, APP_DID_INDEX_FACTORY_STATION10},
    {11, APP_DID_INDEX_FACTORY_STATION11},
    {12, APP_DID_INDEX_FACTORY_STATION12},
    {13, APP_DID_INDEX_FACTORY_STATION13},
    {14, APP_DID_INDEX_FACTORY_STATION14},
    {15, APP_DID_INDEX_FACTORY_STATION15},
    {16, APP_DID_INDEX_FACTORY_STATION16}
};

static TS_CALIBRATION_OBJ calibrationObj;
static u32 calAddressStart = CALIBRA_DATA_START;
static u08 appDidData[APP_DID_DATA_SIZE];
static u08 appDidDataNow[APP_DID_DATA_SIZE];
static const u32 appDidDataAddr = SHINRY_DID_START;
u08 calDataInitOkFlag = 0;

/* Private function property -------------------------------------------------*/
/*******************************************************************************
 * @brief      :
 * @parameter  :
 * @return     :
 *******************************************************************************/
static u08 checkSum(const u08 * pData, u32 size)
{
    u08 sum = 0;

    while(size--)
    {
        sum += *pData;
        pData++;
    }

    return sum;
}

s16 GetCalPortKvalue(CALIBRATION_ENUM index)
{
    s32 k = calibrationObj.cali[index].rateK;

    return (s16)k;
}
s16 GetCalPortBvalue(CALIBRATION_ENUM index)
{
    s32 b = calibrationObj.cali[index].offsetB;

    b /= 10;
    return (s16)b;
}
s32 GetKv_acValue(void)
{
	return calibrationObj.cali[CAL_OBC_IN_VOLT].rateK;
}
s32 GetKi_acValue(void)
{
	return calibrationObj.cali[CAL_OBC_IN_CURR].rateK;
}
s32 GetBv_acValue(void)
{
	return calibrationObj.cali[CAL_OBC_IN_VOLT].offsetB;
}
s32 GetBi_acValue(void)
{
	return calibrationObj.cali[CAL_OBC_IN_CURR].offsetB;
}

s32 GetKhv_dcValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].rateK;
}
s32 GetKhi_dcValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_CURR].rateK;
}
s32 GetBhv_dcValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].offsetB;
}
s32 GetBhi_dcValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_CURR].offsetB;
}
s32 GetInvKi_acValue(void)
{
	return calibrationObj.cali[CAL_OBC_INV_OUT_CURR].rateK;
}
s32 GetInvBi_acValue(void)
{
	return calibrationObj.cali[CAL_OBC_INV_OUT_CURR].offsetB;
}

#if 0
s32 GetR_KValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].rateK;
}
s32 GetU_KpValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].rateK;
}
s32 GetU_KiValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].rateK;
}
s32 GetI_KpValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].rateK;
}
s32 GetI_KiValue(void)
{
	return calibrationObj.cali[CAL_OBC_OUT_VOLT].rateK;
}
#endif
void calPortReset(void)
{
    memcpy(&calibrationObj, &calibrationObjDefault, sizeof(calibrationObj));
}

void calPortResetOne(CALIBRATION_ENUM type)
{
	calPortWriteToEepromOne(type, calibrationObjDefault.cali[type].rateK, calibrationObjDefault.cali[type].offsetB);
	setUpdateCalFlg(1);
}

static u16 Pdata[256] = {0};

u08 calPortWriteToEeprom(void)
{
	u08 i = 0;
	u08 ret = 0;

    calibrationObj.crc = crc32((const char *)&calibrationObj, sizeof(calibrationObj) - sizeof (u32));

	for(i=0; i<EEPROM_READ_RETRY_COUNT; i++)
    {
    	EepromWriteData(calAddressStart, (u08 *)&calibrationObj, sizeof(calibrationObj));
	
		memset(Pdata, 0, sizeof(calibrationObj));
		EepromReadData(calAddressStart, (u08 *)Pdata, sizeof(calibrationObj));
		if(!memcmp((u08 *)&calibrationObj, Pdata, sizeof(calibrationObj)))
		{
			break;
		}
	}

	if (i >= EEPROM_READ_RETRY_COUNT)
	{
		return ret;
	}

	for(i=0; i<EEPROM_READ_RETRY_COUNT; i++)
    {
        /* for ti chips, the offset address 512 must divided  by 2 for word address(two bytes) */
    	EepromWriteData(calAddressStart + 512 / 2, (u08 *)&calibrationObj, sizeof(calibrationObj));
	
		memset(Pdata, 0, sizeof(calibrationObj));
		EepromReadData(calAddressStart + 512 / 2, (u08 *)Pdata, sizeof(calibrationObj));
		if(!memcmp((u08 *)&calibrationObj, Pdata, sizeof(calibrationObj)))
		{
			break;
		}
	}

	if(i>=10)
	{
		return ret;
	}

	return 1;
}

void calPortWriteToEepromOne(CALIBRATION_ENUM type, u32 rateK, u32 offsetB)
{
    calibrationObj.cali[type].rateK = rateK;
	calibrationObj.cali[type].offsetB = offsetB;

	EepromWriteData(calAddressStart + OFFSETOF(TS_CALIBRATION_OBJ, cali) + type * sizeof (TS_CAL_KB), \
        (u08 *)&calibrationObj.cali[type].rateK, sizeof (TS_CAL_KB));
	
    //calibrationObj.crc = crc32((const char *)&calibrationObj, sizeof(calibrationObj) - 4);

    //EepromWriteData(calAddressStart + sizeof(calibrationObj) - 4, (u08 *)&calibrationObj.crc, 4);
}

void calPortReadFromEepromOne(CALIBRATION_ENUM type, u32 * pRateK, u32 * pOffsetB)
{
	TS_CAL_KB cali = {0};
	
    EepromReadData(calAddressStart + OFFSETOF(TS_CALIBRATION_OBJ, cali) + type * sizeof (TS_CAL_KB), \
        (u08 *)&cali, sizeof (TS_CAL_KB));

	*pRateK = cali.rateK;
	*pOffsetB = cali.offsetB;
}

void calPortInit(void)
{
	u08 i = 0;

	for(i=0; i<EEPROM_READ_RETRY_COUNT; i++)
	{	
		memset((u08 *)&calibrationObj, 0, sizeof(calibrationObj));
		
	    EepromReadData(calAddressStart, (u08 *)&calibrationObj, sizeof(calibrationObj));
		
	    if(CALIBRATION_MAGIC_NUM != calibrationObj.magicNum)
	    {
	        ERROR("magic num fail %x", calibrationObj.magicNum);
	    }
	    else if(calibrationObj.crc != crc32((const char *)&calibrationObj, sizeof(calibrationObj) - sizeof (u32)))
	    {
	        ERROR("CRC fail %x", calibrationObj.crc);
	    }
	    else
	    {
	        INFO("calibration version is %d", calibrationObj.version);
			calDataInitOkFlag = 1;
			setUpdateCalFlg(1);
	        return;
	    }
	}

	for(i=0; i<EEPROM_READ_RETRY_COUNT; i++)
	{	
		memset((u08 *)&calibrationObj, 0, sizeof(calibrationObj));
		
	    EepromReadData(calAddressStart + 512 / 2U, (u08 *)&calibrationObj, sizeof(calibrationObj));
		
	    if(CALIBRATION_MAGIC_NUM != calibrationObj.magicNum)
	    {
	        ERROR("magic num fail %x", calibrationObj.magicNum);
	    }
	    else if(calibrationObj.crc != crc32((const char *)&calibrationObj, sizeof(calibrationObj) - sizeof (u32)))
	    {
	        ERROR("CRC fail %x", calibrationObj.crc);
	    }
	    else
	    {
	        INFO("calibration version is %d", calibrationObj.version);	  			
			calDataInitOkFlag = 1;
			setUpdateCalFlg(1);
	        return;
	    }
	}

    calPortReset(); 
	calDataInitOkFlag = 0;
    setUpdateCalFlg(1);
}

u32 calPortConversion(CALIBRATION_ENUM index, u32 src)
{
    s32 result = 0;
    s32 k = 0;
    s32 b = 0;

	/*Fix QAC bug-M3CM-1_Rule-21.18: Apparent: Dereference of an invalid pointer value.*/
	if(index >= CAL_MAX_VALUE)
	{
		 index = (CALIBRATION_ENUM)(CAL_MAX_VALUE - 1);
	}

    k = calibrationObj.cali[index].rateK;
    b = calibrationObj.cali[index].offsetB;

    if (index >= CAL_MAX_VALUE)
    {
        return result;
    }
	switch(index)
	{
		case CAL_OBC_OUT_VOLT:
		case CAL_OBC_OUT_CURR:
		case CAL_OBC_IN_VOLT:
		case CAL_OBC_IN_CURR:
		case CAL_OBC_INV_OUT_CURR:
			result = src;
			break;
		case CAL_OBC_SET_VOLT:
		case CAL_OBC_SET_CURR:
		case CAL_DC_OUT_VOLT_SETTING:
			result = src;
			break;
		case CAL_OBC_KL30_VOLT:
		case CAL_DC_IN_VOLT:
			result = k * src / (double)1000.0 + b / (double)1000.0;
			break;
		default:
			result = k * src / (double)1000.0 + b / (double)1000.0;
			break;
	}
    return result <= 0 ? 0 : result;
}

float calPortConversion1(CALIBRATION_ENUM index, float src)
{
	float result = 0;
	float k = 0 ;
	float b = 0 ;

	/*Fix QAC bug-M3CM-1_Rule-21.18: Apparent: Dereference of an invalid pointer value.*/
	if(index >= CAL_MAX_VALUE)
	{
	 	index = (CALIBRATION_ENUM)(CAL_MAX_VALUE - 1);
	}   
	k = (float)calibrationObj.cali[index].rateK;
	b = (float)calibrationObj.cali[index].offsetB;

	if (index >= CAL_MAX_VALUE)
    {
        return result;
    }

	result = k * src / (double)1000.0 + b / (double)1000.0;
    return result <= 0 ? 0 : result;
}

#pragma CODE_SECTION(calPortConversionFloat,".TI.ramfunc");
float calPortConversionFloat(CALIBRATION_ENUM index, float src)
{
	float result = 0;
	float k = 0;
	float b = 0;

	/*Fix QAC bug-M3CM-1_Rule-21.18: Apparent: Dereference of an invalid pointer value.*/
	if(index >= CAL_MAX_VALUE)
	{
		index = (CALIBRATION_ENUM)(CAL_MAX_VALUE - 1);
	}    
	k = (float)calibrationObj.cali[index].rateK;
	b = (float)calibrationObj.cali[index].offsetB;

	result = k * src / (double)1000.0 + b / (double)1000.0;
    return result <= 0 ? 0 : result;
}

void calPortPara(CALIBRATION_ENUM index, u32 * pRateK, u32 * pOffsetB)
{
	*pRateK = calibrationObj.cali[index].rateK;
	*pOffsetB = calibrationObj.cali[index].offsetB;
}
u32 calUpdateflg=0;
void setUpdateCalFlg(u32 flg)
{
	calUpdateflg = flg;
}
u08 isUpdateCalDate(void)
{
	return ((calUpdateflg)?TRUE:FALSE);
}

/*****************************************************************************
 * Function      : u32 appDidRead(TE_APP_DID_INDEX index)
 * Description   : read did data
 * Input         : TE_APP_DID_INDEX index
 * Output        : None
 * Return        : u32
*****************************************************************************/
static u32 appDidRead(TE_APP_DID_INDEX index)
{
    u32 value = 0;
    u16 byteStart = appDidObjTab[index].offset / 8;
    u08 byteOffset = appDidObjTab[index].offset - byteStart * 8;

    if(APP_DID_INDEX_MAX <= index)
    {
        ERROR("Ë÷ÒýÖµ %d >= ×î´óÖµ", index);
        return 0;
    }

    get_frame_data(&appDidData[byteStart],
                    INTEL_STANDARD,
                    byteOffset,
                    appDidObjTab[index].size,
                    &value);

    return value;
}

/*****************************************************************************
 * Function      : void appDidWrite(TE_APP_DID_INDEX index, u32 value)
 * Description   : write did data
 * Input         : TE_APP_DID_INDEX index
 *                 u32 value
 * Output        : None
 * Return        : void
*****************************************************************************/
static void appDidWritePort(u08 data[], TE_APP_DID_INDEX index, u32 value)
{
    u32 i = 0;
    u16 size = appDidObjTab[index].size;
    u16 offset = appDidObjTab[index].offset;
    u16 byteStart = offset / 8;

    if(APP_DID_INDEX_MAX <= index)
    {
        ERROR("Ë÷ÒýÖµ %d >= ×î´óÖµ", index);
        return;
    }

    for(i = 0; i < size; i++)
    {
        CLR_BITU08(data[(offset + i) / 8], ((offset + i) % 8));
    }

    set_frame_data(&data[byteStart],
                    INTEL_STANDARD,
                    offset % 8,
                    size,
                    value);
}

/*void appDidWrite(TE_APP_DID_INDEX index, u32 value)
{
    appDidWritePort(appDidData, index, value);
}*/

/*****************************************************************************
 * Function      : void appDidStoreCheck(void)
 * Description   : check did data
 * Input         : none
 * Output        : None
 * Return        : void
*****************************************************************************/
static void appDidStoreCheck(void)
{
    u32 addr = appDidDataAddr + APP_DID_DATA_SIZE - 1;

    appDidDataNow[APP_DID_DATA_SIZE - 1]
        = checkSum(appDidDataNow, APP_DID_DATA_SIZE - 1);

    EepromWriteData(addr, &appDidDataNow[APP_DID_DATA_SIZE - 1], 1);
}

/*****************************************************************************
 * Function      : void appDidWriteNow(TE_APP_DID_INDEX index, u32 value)
 * Description   : write did data now
 * Input         : TE_APP_DID_INDEX index
 *                 u32 value
 * Output        : None
 * Return        : void
*****************************************************************************/
static void appDidWriteNow(TE_APP_DID_INDEX index, u32 value)
{
    u16 byteStart = appDidObjTab[index].offset / 8;
    u16 size = appDidObjTab[index].size / 8 + 2;

    appDidWritePort(appDidDataNow, index, value);
    appDidWritePort(appDidData, index, value);
    EepromWriteData(appDidDataAddr + byteStart, &appDidDataNow[byteStart], size);

    appDidStoreCheck();
}

/*****************************************************************************
 * Function      :
 * Description   :
 * Input         :
 * Output        : None
 * Return        : void
*****************************************************************************/
u08 appDidMesIDRead(u08 * pData, u08 length)
{
    u16 addrStart = 264 / 8;
    u08 sizeMax = 32;
    u32 i = 0;

    if(sizeMax < length)
    {
        return 1;
    }

    for(i = 0; i < length; i++)
    {
        pData[i] = appDidDataNow[addrStart + i];
    }

    return 0;
}

/*****************************************************************************
 * Function      :
 * Description   :
 * Input         :
 * Output        : None
 * Return        : void
*****************************************************************************/
u08 appDidMesIDWrite(u08 * pData, u08 length)
{
    u16 addrStart = 264/8;
    u16 sizeMax = 32;
    u32 i = 0;

    if(sizeMax < length)/*(MES_ID_LENGTH != length)*/
    {
        return 1;
    }

    for(i = 0; i < length; i++)
    {
        appDidData[addrStart + i] = pData[i];
        appDidDataNow[addrStart + i] = pData[i];
    }

    if(length < sizeMax)
    {
        appDidData[addrStart + length] = 0;
        appDidDataNow[addrStart + length] = 0;
        length++;
    }

    EepromWriteData(appDidDataAddr + addrStart, &appDidDataNow[addrStart], length);

    appDidStoreCheck();

    return 0;
}

/*****************************************************************************
 * Function      :
 * Description   :
 * Input         :
 * Output        : None
 * Return        : void
*****************************************************************************/
void appDidFactoryStationWrite(u08 index, u16 data)
{
    appDidWriteNow(station[index].Didindex, data);
}

/*****************************************************************************
 * Function      :
 * Description   :
 * Input         :
 * Output        : None
 * Return        : void
*****************************************************************************/
u16 appDidFactoryStationRead(u08 index)
{
    return (u16)appDidRead(station[index].Didindex);
}

/*****************************************************************************
 * Function      : static void appDidReset(void)
 * Description   : did data reset
 * Input         : None
 * Output        : None
 * Return        : void
*****************************************************************************/
static void appDidReset(void)
{
    memset(appDidData, 0, APP_DID_DATA_SIZE);
    appDidData[0] = APP_DID_MAGIC_MUM;
}

/*****************************************************************************
 * Function      : void appDidStore(void)
 * Description   : store did data
 * Input         : none
 * Output        : None
 * Return        : void
*****************************************************************************/
static void appDidStore(void)
{
    appDidData[APP_DID_DATA_SIZE - 1]
        = checkSum(appDidData, APP_DID_DATA_SIZE - 1);

    EepromWriteData(appDidDataAddr, appDidData, APP_DID_DATA_SIZE);
}

/*****************************************************************************
 * Function      : void appDidInit(void)
 * Description   : did data init
 * Input         : None
 * Output        : None
 * Return        : void
*****************************************************************************/
void appDidInit(void)
{
    EepromReadData(appDidDataAddr, appDidData, APP_DID_DATA_SIZE);
    memcpy(appDidDataNow, appDidData, APP_DID_DATA_SIZE);

    if(APP_DID_MAGIC_MUM != appDidData[0])
    {
        ERROR("APP DID magic num error!");
    }
    else if(checkSum(appDidData, APP_DID_DATA_SIZE - 1)
        != appDidData[APP_DID_DATA_SIZE - 1])
    {
        ERROR("APP DID check sum error!");
    }
    else
    {
        return;
    }

    appDidReset();
    appDidStore();

    memcpy(appDidDataNow, appDidData, APP_DID_DATA_SIZE);
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
