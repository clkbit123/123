/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : Calibration.h
* Author             : R&D SW
* Brief              : Calibration module header file
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2019/06/19          R&D SW   First Issue
*******************************************************************************/

#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#ifdef CALIBRATION_C_
#define CALIBRATION_DEC
#else
#define CALIBRATION_DEC	extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"
#include "Diag_External.h"

/* Export define -------------------------------------------------------------*/
#define PUBLIC_CALIBRATION_REQUST	 (0x00000966 | CAN_MESSAGE_EXTEND_FIX)
#define PUBLIC_CALIBRATION_RESPONSE  (0x00000968 | CAN_MESSAGE_EXTEND_FIX)

/* Export typedef ------------------------------------------------------------*/
typedef struct
{
    u32         TxId;
    u32         RxId;
    u08         Ext;
    isotp_pkg   tppkg;
    App_CanBasic *pCanBase_reps;
}CALIBRATION_TD;

/* Export variables ----------------------------------------------------------*/
CALIBRATION_DEC CALIBRATION_TD *pCalTp;

/* Export function -----------------------------------------------------------*/
CALIBRATION_DEC void CalbrationInit(CALIBRATION_TD *pdc, u32 TxId, u32 RxId, u08 Ext, App_CanBasic *pCanBase_reps);
CALIBRATION_DEC void CalbrationProcess(CALIBRATION_TD *pdc);
CALIBRATION_DEC u08 getCalibrationMode(void);

extern u32 calibrationDataGet(u08 id, u32 * pValue);

/* Interior typedef ----------------------------------------------------------*/
#define    	RSP_MSG_BIT_CHK     	(0x01<<7)
#define    	POS_RSP_BIT         	(0x40)
#define    	KEY_BYTE_NUM        	8
#define 	OBC_MAX_OUT_VOLT    	MAX_SET_VOLT
#define 	OBC_MAX_OUT_CURR    	MAX_SET_CURR
#define 	_MAX_K 					1050 	//1.0+5%
#define 	_MIN_K 					950  	//1.0-5%

#define 	CHK_RANGE_OF(v,min,max) (v=v>max?max:(v<min?min:v))
#define 	CALIBRA_VALUE(adc,k,b) 	(((adc)*(1000)+(0))/1000)	//(((adc)*(k)+(b))/1000)
#define 	CRC_POLY 				0x2f
#define		DATA_BYTE_SIZE			6
#define		INCOMING_BYTE_ONE		0x07
#define 	DID_VARS_ADDR_VALUIE(m) ((m) * (2))

#define		MAX_K_VALUE		2500
#define		MIN_K_VALUE		500
//#define		MAX_B_VALUE		2500
//#define		MIN_B_VALUE		0

#define		K_DEFAULTS		1000
#define		B_DEFAULTS		0

typedef struct
{
	s32 uwRateK;
	u08 uKFlag;		// 1 is TURE 	0 is FALSE
	s32 uwOffsetB;
	u08 uBFlag;
}TS_CALIBRATION_KB;

#define NRC_CALIB_SUCCESS		0x00
#define	NRC_NOT_ENTERING		0x7f
#define	NRC_SECRET_KEY_ERROR	0x35
#define NRC_KB_NOT_CLEAR		0x22
#define	NRC_OUT_OF_RANGE		0x31
#define	NRC_SEQUENTIAL			0x24
#define NRC_NOT_SUPPORT			0x12
#define NRC_IMLOIF              0x13  //NRC_INCORRECT_MESSAGE_LENGTH_OR_FORMAT

/* Interior variables --------------------------------------------------------*/

// Set the enumeration variable of the calibration mode
typedef enum
{
	calibration_mode_exit = 0,
	calibration_mode_enter = 3
}CALIBRATION_MODE_ENUM;

typedef struct
{
    u08 m_unlockFlg;
    u08 m_flowId;
}TS_PROCESS_FLOW;
/* Interior function ---------------------------------------------------------*/

#endif /* CALIBRATION_H_ */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
