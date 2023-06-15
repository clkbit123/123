/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : Calibration.h
* Author             : R&D SW
* Brief              : Calibration module header file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/06/19          R&D SW   First Issue
*******************************************************************************/
#ifndef _CALIBRATION_PORT_H_
#define _CALIBRATION_PORT_H_

/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* Export define -------------------------------------------------------------*/
#define CALIBRATION_MAGIC_NUM   0xAA55A55A
#define CALIBRATION_VERSION     1001

#define MES_ID_LENGTH 32

/* Export typedef ------------------------------------------------------------*/
typedef enum
{
	CAL_FLAG = 0,
	CAL_OBC_OUT_VOLT,			//==OBCÊä³öµçÑ¹
	CAL_OBC_OUT_CURR,			//==OBCÊä³öµçÁ÷
	CAL_OBC_IN_VOLT,			//==ÊäÈëµçÑ¹
	CAL_OBC_IN_CURR,			//ÊäÈëµçÁ÷
	CAL_OBC_INV_OUT_CURR,		//inverter output currunt calibration
	CAL_OBC_SET_VOLT,
	CAL_OBC_SET_CURR,
	CAL_OBC_BAT_VOLT,			//µç³ØµçÑ¹
	CAL_OBC_PFC_VOLT,			//PFCµçÑ¹
	CAL_OBC_CP_DUTY_CYCLE,		//CPÕ¼¿Õ±È
	CAL_OBC_CP_AMPLITUDE,		//CPµçÑ¹
	CAL_OBC_KL30_VOLT,			//KL30µçÑ¹
	CAL_OBC_MUZZLE_TEMP_ONE,
	CAL_OBC_MUZZLE_TEMP_THREE,
	CAL_OBC_CORE_TEMP_TWO,
	CAL_OBC_AIR_TEMP,
	CAL_DC_IN_VOLT,				//DCDCÊäÈëµçÑ¹
	CAL_DC_IN_CURR,				//DCDCÊäÈëµçÁ÷
	CAL_DC_RESERVE1,
	CAL_DC_RESERVE2,
	CAL_DC_OUT_VOLT_ONE,		//==DCDCÊä³öµçÑ¹
	CAL_DC_RESERVE3,
	CAL_DC_RESERVE4,
	CAL_DC_RESERVE5,
	CAL_DC_RESERVE6,
	CAL_DC_RESERVE7,
	CAL_DC_PRECHG_IN_CURR,      //DCDC ·´ÏòÔ¤³äÊäÈëµçÁ÷
	CAL_DC_OUT_CURR,			//==DCDCÊä³öµçÁ÷
	CAL_DC_OUT_VOLT_SETTING,
	CAL_DC_PEAK_OUT_VOLT_SETTING,
	
	CAL_MAX_VALUE
}CALIBRATION_ENUM;

typedef enum{
    APP_DID_INDEX_FACTORY_STATION1 = 0,
    APP_DID_INDEX_FACTORY_STATION2,
    APP_DID_INDEX_FACTORY_STATION3,
    APP_DID_INDEX_FACTORY_STATION4,
    APP_DID_INDEX_FACTORY_STATION5,
    APP_DID_INDEX_FACTORY_STATION6,
    APP_DID_INDEX_FACTORY_STATION7,
    APP_DID_INDEX_FACTORY_STATION8,
    APP_DID_INDEX_FACTORY_STATION9,
    APP_DID_INDEX_FACTORY_STATION10,
    APP_DID_INDEX_FACTORY_STATION11,
    APP_DID_INDEX_FACTORY_STATION12,
    APP_DID_INDEX_FACTORY_STATION13,
    APP_DID_INDEX_FACTORY_STATION14,
    APP_DID_INDEX_FACTORY_STATION15,
    APP_DID_INDEX_FACTORY_STATION16,
    APP_DID_INDEX_MES_ID,

    APP_DID_INDEX_MAX
}TE_APP_DID_INDEX;

typedef struct{
    s32 rateK;
    s32 offsetB;
}TS_CAL_KB;

typedef struct {
    u32 magicNum;
    u32 version;
    TS_CAL_KB cali[CAL_MAX_VALUE];
    u32 crc;
}TS_CALIBRATION_OBJ;

typedef struct{
    char * pName;
    TE_APP_DID_INDEX index;
    u16 offset;
    u16 size;
}TS_APP_DID_OBJ;

typedef struct{
    u08 factoryStationindex;
    TE_APP_DID_INDEX Didindex;
}TS_FACTORY_STATION;

void calPortReset(void);
void calPortResetOne(CALIBRATION_ENUM type);
u08 calPortWriteToEeprom(void);
void calPortInit(void);
void calPortWriteToEepromOne(CALIBRATION_ENUM type, u32 rateK, u32 offsetB);
void calPortReadFromEepromOne(CALIBRATION_ENUM type, u32 * pRateK, u32 * pOffsetB);
u32 calPortConversion(CALIBRATION_ENUM index, u32 src);
float calPortConversion1(CALIBRATION_ENUM index, float src);
float calPortConversionFloat(CALIBRATION_ENUM index, float src);
void calPortPara(CALIBRATION_ENUM index, u32 * pRateK, u32 * pOffsetB);
u08 isUpdateCalDate(void);
void setUpdateCalFlg(u32 flg);
s16 GetCalPortKvalue(CALIBRATION_ENUM index);
s16 GetCalPortBvalue(CALIBRATION_ENUM index);
s32 GetKv_acValue(void);
s32 GetKi_acValue(void);
s32 GetBv_acValue(void);
s32 GetBi_acValue(void);
s32 GetKhv_dcValue(void);
s32 GetKhi_dcValue(void);
s32 GetBhv_dcValue(void);
s32 GetBhi_dcValue(void);
s32 GetInvKi_acValue(void);
s32 GetInvBi_acValue(void);
#if 0
s32 GetR_KValue(void);
s32 GetU_KpValue(void);
s32 GetU_KiValue(void);
s32 GetI_KpValue(void);
s32 GetI_KiValue(void);
#endif

u08 appDidMesIDRead(u08 * pData, u08 length);
u08 appDidMesIDWrite(u08 * pData, u08 length);
void appDidFactoryStationWrite(u08 index, u16 data);
u16 appDidFactoryStationRead(u08 index);
void appDidInit(void);

#endif /* _CALIBRATION_PORT_H_ */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/


