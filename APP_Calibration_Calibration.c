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
#include "Diag_External_Service_22.h"
#include "Diag_External_Service_19.h"
#include "loggers.h"

//#include "App/Cfg/SysDebug.h"

/* Private function property -------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define GET_SUB_ID(buf, i) (buf[i] * 256 + buf[i + 1])

/* Private define ------------------------------------------------------------*/
#define SYS_DID_A100 0xA100
#define SYS_DID_A110 0xA110
#define SYS_DID_A111 0xA111
#define SYS_DID_A112 0xA112
#define SYS_DID_A113 0xA113
#define SYS_DID_A114 0xA114
#define SYS_DID_A115 0xA115
#define SYS_DID_A116 0xA116
#define SYS_DID_A117 0xA117
#define SYS_DID_A118 0xA118
#define SYS_DID_A119 0xA119
#define SYS_DID_A11A 0xA11A
#define SYS_DID_A11B 0xA11B
#define SYS_DID_A11C 0xA11C
#define SYS_DID_A11D 0xA11D
#define SYS_DID_A11E 0xA11E
#define SYS_DID_A11F 0xA11F

/* Private variables ---------------------------------------------------------*/
static TS_CALIBRATION_KB CalibrationData[CAL_MAX_VALUE];
static CALIBRATION_MODE_ENUM CalibrationModeFlag = calibration_mode_exit;
static CALIBRATION_TD CalTp;

CALIBRATION_TD *pCalTp = &CalTp;

/*******************************************************************************
  * @brief      :  App Calibration Data Init
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void AppCalibrationDataInit(void)
{
	memset(&CalibrationData, 0, sizeof (CalibrationData));
}

TS_PROCESS_FLOW ts_processFlow =
    {
        .m_flowId=0,
        .m_unlockFlg=0
    };
u08 getCalibrationMode(void)
{
    return (calibration_mode_exit == CalibrationModeFlag) ? FALSE : TRUE;
}

static void SetCalibrationMode(CALIBRATION_MODE_ENUM ModeFlag)
{
	CalibrationModeFlag = ModeFlag;
}

static u08 CalibrationDcdcOffset(u08 id)
{
    return ((id < 0x80) ? id:(id - 0x80 + CAL_DC_IN_VOLT));
}

static u08 CRC8Algorithm(u08 *dataByteArray)
{
	u08 crc = 0xffU;
	u08 byte_index = 0U;
	u08 bit_index = 0U;

	crc ^= 0x07U;
	for(bit_index = 0U; bit_index < 8; bit_index++)
	{
		if((crc & 0x80) != 0U)
		{
			crc = (crc << 1U) ^ CRC_POLY;
		}
		else
		{
			crc = crc << 1U;
		}
	}
	for(byte_index = 0U; byte_index < DATA_BYTE_SIZE; byte_index++)
	{
		crc ^= dataByteArray[byte_index];
		for(bit_index = 0U; bit_index < 8U; bit_index++)
		{
			if((crc & 0x80U) != 0U)
			{
				crc = (crc << 1U) ^ CRC_POLY;
			}
			else
			{
				crc = crc << 1U;
			}
		}
	}

	return ~crc & 0xFFU;
}

static u08 JudgeKValue(u32 Value)
{
    if((Value > MIN_K_VALUE) && (Value < MAX_K_VALUE))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static u08 IsIDVaild(u08 id)
{
	if((CALIBRATION_ENUM)id < CAL_MAX_VALUE)
	{
		return 1;
	}

	return 0;
}

static void WriteKValue(CALIBRATION_ENUM CalibrationKEnum, u32 Value, volatile u08 *ucRespCode)
{
	u32 rateK = 0;
	u32 offsetB = 0;

    if(JudgeKValue(Value))
    {
        *ucRespCode = NRC_OUT_OF_RANGE;
        return;
    }
    
    
	calPortPara(CalibrationKEnum, &rateK, &offsetB);

	if(K_DEFAULTS == rateK)
	{
		calPortWriteToEepromOne(CalibrationKEnum, Value, offsetB);
		setUpdateCalFlg(1);
		INFO("%d Ð´ÈëKÖµ = %d[%d]", CalibrationKEnum, Value, offsetB);

		CalibrationData[CalibrationKEnum].uKFlag = 1;
	}
	else
	{
		*ucRespCode = NRC_KB_NOT_CLEAR;
		ERROR("K ÒªÏÈÇå³ý²ÅÄÜÔÙÖØÐÂÐ´Èë...");
	}
}

static void WriteBValue(CALIBRATION_ENUM CalibrationKEnum, u32 Value, volatile u08 *ucRespCode)
{
	u32 rateK = 0, offsetB = 0;
    
	calPortPara(CalibrationKEnum, &rateK, &offsetB);

	if(CalibrationData[CalibrationKEnum].uKFlag)
	{
		calPortWriteToEepromOne(CalibrationKEnum, rateK, Value);
		setUpdateCalFlg(1);
		INFO("%d Ð´ÈëBÖµ = %d[%d]", CalibrationKEnum, Value, rateK);
	}
	else
	{
		CalibrationData[CalibrationKEnum].uBFlag = 0;
		*ucRespCode = NRC_SEQUENTIAL;
		ERROR("K ÔõÃ´¿ÉÄÜÊÇ0ÄØ...");
	}
}

static u32 GetValue(u08 *inBuf)
{
	u08 buf[4] = {0};
    
	buf[0] = GetByteFromBuf(inBuf, 5);
	buf[1] = GetByteFromBuf(inBuf, 4);
	buf[2] = GetByteFromBuf(inBuf, 3);
	buf[3] = GetByteFromBuf(inBuf, 2);
    
	return ((u32)(buf[3] & 0xff) << 24) + ((u32)(buf[2] & 0xff) << 16) + ((u32)(buf[1] & 0xff) << 8) + buf[0];
}

static void CalibrationRepsPosi(u08 *outBuf, u16 *outLen, u08 sid, u08 id, u32 data)
{
	u08 size = 0;

    PutByteToBuf(outBuf, size++, sid | POS_RSP_BIT);
    PutByteToBuf(outBuf, size++, id);
    PutByteToBuf(outBuf, size++, (data >> 24) & 0xff);
    PutByteToBuf(outBuf, size++, (data >> 16) & 0xff);
    PutByteToBuf(outBuf, size++, (data >> 8) & 0xff);
    PutByteToBuf(outBuf, size++, (data >> 0) & 0xff);
    PutByteToBuf(outBuf, size++, CRC8Algorithm(outBuf));

    *outLen = size;
}

static void CalibrationRepsNRC(u08 *outBuf, u16 *outLen, u08 sid, u08 resp_code)
{
	u08 size = 0;

	PutByteToBuf(outBuf, size++, 0x7F);
	PutByteToBuf(outBuf, size++, sid);
	PutByteToBuf(outBuf, size++, resp_code);
    
	*outLen = size;
}

static void ParseKB(u08 *inBuf,   u08 *outBuf, u16 *outLen,volatile u08 *ucRespCode)//u16 *inLen,
{
	u32 value = GetValue(inBuf);
	u08 sid = GetByteFromBuf(inBuf, 0);
	u08 ucId = GetByteFromBuf(inBuf, 1);
	u08 ID = CalibrationDcdcOffset(ucId);

    if(!IsIDVaild(ID))
    {
		*ucRespCode = NRC_NOT_SUPPORT;
		return ;
    }

    switch(sid)
    {
    	case 0xB2:
    	case 0xB3:
    	{
        	u32 offsetB = 0;
        	u32 KValue = 0;
    		calPortReadFromEepromOne((CALIBRATION_ENUM)ID, &KValue, &offsetB);
            if(0xB2 == sid)
            {
                value = KValue;
            }
            else
            {
                value = offsetB;
            }
    		break;
    	}
    	case 0xBC:
            WriteKValue((CALIBRATION_ENUM)ID, value, ucRespCode);
            value = 0;
    		break;
    	case 0xBD:
    		WriteBValue((CALIBRATION_ENUM)ID, value, ucRespCode);
            value = 0;
    		break;
    	case 0xBE:
            if(0 != calibrationDataGet(ID, &value))
            {
        		*ucRespCode = NRC_NOT_SUPPORT;
        		break;
            }
//          ucId = ID;//ÒªÓÃ×ª»»ºóµÄID
    		break;
    	default:
    		*ucRespCode = NRC_NOT_SUPPORT; //NRC_ROOR
    		break;
    }

	if(!(*ucRespCode))
	{
        CalibrationRepsPosi(outBuf, outLen, sid ,ucId, value);
	}

	return ;
}

//ECU Reset
static void Parse11(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen, volatile u08 *ucRespCode)
{
	volatile u08 custom;

	//power on reset
	if(*inLen != 7)
	{
		*ucRespCode = NRC_NOT_ENTERING;
		return ;
	}
	custom = GetByteFromBuf(inBuf, 1);
	switch(custom)
	{
		case 0x01:
			SystemSwReset();  // ECU Reset
			break;

		default:
			*ucRespCode = NRC_NOT_SUPPORT;
			break;
	}
	//we must reply message to the teseter then reset
	if(!(*ucRespCode))
	{
        CalibrationRepsPosi(outBuf, outLen, 0x11 ,custom, 0);
	}

	return ;
}

// Calibration mode
static void ParseB0(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen, volatile u08 *ucRespCode)
{
	u08 i = 0;
	volatile u08 custom;
	volatile u08 ID = 0 ;
    if(*inLen != 7)
    {
        *ucRespCode = NRC_NOT_ENTERING;
        return ;
    }

    custom = GetByteFromBuf(inBuf, 1);

    if(0xFD == custom)
    {
    	//ParseB0_FD(inBuf, outBuf, inLen, outLen, &ucRespCode);
    	if(GetByteFromBuf(inBuf, 3) != 0x30 
            || GetByteFromBuf(inBuf, 4) != 0x31 
            || GetByteFromBuf(inBuf, 5) != 0x37)
		{
			*ucRespCode = NRC_SECRET_KEY_ERROR;
			return ;
		}

		ID = GetByteFromBuf(inBuf, 2);

		if(ID == 0xFF)
		{
			for(i = CAL_OBC_OUT_VOLT;i < CAL_MAX_VALUE;i++)
			{
				calPortResetOne((CALIBRATION_ENUM)i);
			}
		}
		else if(CalibrationDcdcOffset(ID) < CAL_MAX_VALUE)
		{
			calPortResetOne((CALIBRATION_ENUM)CalibrationDcdcOffset(ID));
		}
		else
		{
			*ucRespCode = NRC_NOT_SUPPORT;
		}

		if(!(*ucRespCode))
		{
            CalibrationRepsPosi(outBuf, outLen, 0xb0 ,0xFD, ((u32)ID<<24));	/* ½â¾öÓ¦´ðÊý¾Ý¸ñÊ½²»ÕýÈ·ÎÊÌâ  */
		}
    }
    else
    {
    	if(GetByteFromBuf(inBuf, 2) != 0x32 
            || GetByteFromBuf(inBuf, 3) != 0x30 
            || GetByteFromBuf(inBuf, 4) != 0x31 
			|| GetByteFromBuf(inBuf, 5) != 0x37)
		{
			*ucRespCode = NRC_SECRET_KEY_ERROR;
			return ;
		}

		custom = GetByteFromBuf(inBuf, 1);

		switch(custom)
		{
			case 0x00: // exit calibration mode
				SetCalibrationMode(calibration_mode_exit);
				break;
			case 0x03: // enter calibration mode
				SetCalibrationMode(calibration_mode_enter);
				break;

			case 0xFC:
				// read version
				break;

			case 0xFE: // save all calibration data
				if(calPortWriteToEeprom())
				{
					setUpdateCalFlg(1);
				}
				else
				{
					*ucRespCode = NRC_SEQUENTIAL;
				}
				break;

			default:
				*ucRespCode = NRC_NOT_SUPPORT;
				break;
		}
		
		if(!(*ucRespCode))
		{
			if(0x00 == custom || 0x03 == custom || 0xFE == custom)
			{
                CalibrationRepsPosi(outBuf, outLen, 0xb0 ,custom, 0);
			}
			if(0xFC == custom)
			{
				u08 buf[6] = {0, 0, 'V', '0', '3', '1'};
                CalibrationRepsPosi(outBuf, outLen, 0xb0 ,custom, GetValue(buf));
			}
		}
    }
}

static void ParseProcessFlowID(u08 *inBuf, u08 *outBuf, u16 *outLen, volatile u08 *ucRespCode)//, u16 *inLen
{
    u32 value = 0;
    u32 offsetB = 0;
    u32 KValue = 0;
    u08 sid = GetByteFromBuf(inBuf, 0);
    u08 ucId = GetByteFromBuf(inBuf, 1);
    u08 ID = CalibrationDcdcOffset(ucId);

    switch(sid)
    {
    case 0xA0:

        if(GetByteFromBuf(inBuf, 2) != 0x32
            || GetByteFromBuf(inBuf, 3) != 0x30
            || GetByteFromBuf(inBuf, 4) != 0x32
            || GetByteFromBuf(inBuf, 5) != 0x31)
        {
            *ucRespCode = NRC_SECRET_KEY_ERROR;
            return ;
        }

        if(0x00 == ucId)
        {
            /*
             * ÍË³öÐ´Éú²úÁ÷³Ì±êÖ¾Ä£Ê½
             */
            ts_processFlow.m_unlockFlg = 0;
        }
        else if(0x03 == ucId)
        {
            /*
             * ½øÈëÐ´Éú²úÁ÷³Ì±êÖ¾Ä£Ê½
             */
            ts_processFlow.m_unlockFlg = 0x55;
        }
        ucId = 0x04;
        value =0;
        break;
    case 0xA1:
        /*
         *Ð´ÈëÉú²úÁ÷³Ì±êÖ¾Êý¾Ý
         */
        if((!IsIDVaild(ID))||(CAL_FLAG != ID))
        {
            *ucRespCode = NRC_NOT_SUPPORT;
            return ;
        }
        if(0x55 != ts_processFlow.m_unlockFlg)
        {
            *ucRespCode = NRC_NOT_SUPPORT;
            return ;
        }

        value = GetValue(inBuf);
        if(value > 0xFF)
        {
            *ucRespCode = NRC_NOT_SUPPORT;
            return ;
        }
        calPortWriteToEepromOne((CALIBRATION_ENUM)ID, K_DEFAULTS, value);
        calPortWriteToEeprom();
        value = 0;
        break;
    case 0xA2:
        /*
         *¶ÁÈ¡Éú²úÁ÷³Ì±êÖ¾Êý¾Ý
         */
        if((!IsIDVaild(ID))||(CAL_FLAG != ID))
        {
            *ucRespCode = NRC_NOT_SUPPORT;
            return ;
        }
        calPortReadFromEepromOne((CALIBRATION_ENUM)ID, &KValue, &offsetB);
        value = offsetB & 0xFF;
        break;
    default:
        *ucRespCode = NRC_NOT_SUPPORT; //NRC_ROOR
        break;
    }

    if(!(*ucRespCode))
    {
        CalibrationRepsPosi(outBuf, outLen, sid ,ucId, value);
    }

    return ;
}

static u08 Dcm_0x966_22_A100(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen)
{
    u08 ucIdx = 0;
    u08 mesID[MES_ID_LENGTH] = {0};
    u08 i = 0;

    appDidMesIDRead(mesID, MES_ID_LENGTH);

    PutByteToBuf(outBuf, ucIdx++, 0x22 + POS_RSP_BIT);
    PutByteToBuf(outBuf, ucIdx++, inBuf[1]);
    PutByteToBuf(outBuf, ucIdx++, inBuf[2]);

    for(i = 0; i < MES_ID_LENGTH; i++)
    {
        if(0 == mesID[i] )
        {
            break;
        }

        outBuf[ucIdx] = mesID[i];
        ucIdx++;
    }

    *outLen = ucIdx;
    return NRC_CALIB_SUCCESS;
}

static u08 Dcm_0x966_22_A110FactoryStationRead(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen)
{
    u08 ucIdx = 0;
    u16 stationData = 0;
    u16 StationDID = GET_SUB_ID(inBuf, 1);

    stationData = appDidFactoryStationRead(StationDID - SYS_DID_A110);

    PutByteToBuf(outBuf, ucIdx++, 0x22 + POS_RSP_BIT);
    PutByteToBuf(outBuf, ucIdx++, inBuf[1]);
    PutByteToBuf(outBuf, ucIdx++, inBuf[2]);
    PutByteToBuf(outBuf, ucIdx++, stationData >> 8);
    PutByteToBuf(outBuf, ucIdx++, stationData & 0xff);

    *outLen = ucIdx;
    return NRC_CALIB_SUCCESS;
}

static u08 Dcm_0x966_2E_A100(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen)
{
    u08 ucIdx = 0;
    u08 ucRespCode = NRC_CALIB_SUCCESS;

    if((MES_ID_LENGTH >= (*inLen - 3))
            && (0 == appDidMesIDWrite(&inBuf[3], *inLen - 3)))
    {
        ucRespCode = NRC_CALIB_SUCCESS;
    }
    else
    {
        ucRespCode = NRC_IMLOIF;
    }

    *outLen = ucIdx;
    return ucRespCode;
}

static u08 Dcm_0x966_2E_A110FactoryStationWrite(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen)
{
    u08 ucIdx = 0;
    u16 StationDID = GET_SUB_ID(inBuf, 1);
    u16 StationData = GET_SUB_ID(inBuf, 3);

    if(5 != *inLen)
    {
        return NRC_IMLOIF;
    }

    appDidFactoryStationWrite(StationDID - SYS_DID_A110, StationData);

    *outLen = ucIdx;
    return NRC_CALIB_SUCCESS;
}

static u08 Dcm_0x966_22(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen)
{
    u08 ucRespCode = NRC_NOT_ENTERING;
    u16 uwSubId = GET_SUB_ID(inBuf, 1);

    INFO("subID = %x", uwSubId);

    if(3 != *inLen)
    {
        return NRC_IMLOIF;
    }

    switch(uwSubId)
    {
        case SYS_DID_A100:
            ucRespCode = Dcm_0x966_22_A100(inBuf, inLen, outBuf, outLen);
            break;
        case SYS_DID_A110:
        case SYS_DID_A111:
        case SYS_DID_A112:
        case SYS_DID_A113:
        case SYS_DID_A114:
        case SYS_DID_A115:
        case SYS_DID_A116:
        case SYS_DID_A117:
        case SYS_DID_A118:
        case SYS_DID_A119:
        case SYS_DID_A11A:
        case SYS_DID_A11B:
        case SYS_DID_A11C:
        case SYS_DID_A11D:
        case SYS_DID_A11E:
        case SYS_DID_A11F:
            ucRespCode = Dcm_0x966_22_A110FactoryStationRead(inBuf, inLen, outBuf, outLen);
            break;
        default:
            ucRespCode = NRC_OUT_OF_RANGE;
            break;

    }
    return ucRespCode;
}

static u08 Dcm_0x966_2E(u08 *inBuf, u16 *inLen, u08 *outBuf, u16 *outLen)
{
    u16 uwSubId = 0;
    u16 ucIdx = 0;
    u08 ucRespCode = NRC_CALIB_SUCCESS;

    uwSubId = GET_SUB_ID(inBuf, 1);

    INFO("subID = %x, inlen = %d", uwSubId, *inLen);

    switch(uwSubId)
    {
        case SYS_DID_A100:
            ucRespCode = Dcm_0x966_2E_A100(inBuf, inLen, outBuf, outLen);
            break;
        case SYS_DID_A110:
        case SYS_DID_A111:
        case SYS_DID_A112:
        case SYS_DID_A113:
        case SYS_DID_A114:
        case SYS_DID_A115:
        case SYS_DID_A116:
        case SYS_DID_A117:
        case SYS_DID_A118:
        case SYS_DID_A119:
        case SYS_DID_A11A:
        case SYS_DID_A11B:
        case SYS_DID_A11C:
        case SYS_DID_A11D:
        case SYS_DID_A11E:
        case SYS_DID_A11F:
            ucRespCode = Dcm_0x966_2E_A110FactoryStationWrite(inBuf, inLen, outBuf, outLen);
            break;
        default:
            ucRespCode = NRC_OUT_OF_RANGE;
            break;
    }

    if(NRC_CALIB_SUCCESS == ucRespCode)
    {
        PutByteToBuf(outBuf, ucIdx++, 0x2E + POS_RSP_BIT);
        PutByteToBuf(outBuf, ucIdx++, inBuf[1]);
        PutByteToBuf(outBuf, ucIdx++, inBuf[2]);
        *outLen = ucIdx;
    }
    return ucRespCode;
}

/*******************************************************************************
  * @brief      :  Calibration Service
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static u08 CalibrationServiceHandle(u08 *inBuf, u16 *inLen, u08 *outbuf, u16 *outlen)
{
    volatile u08 ucRespCode = NRC_NOT_ENTERING;

    u08 sid = GetByteFromBuf(inBuf, 0);
    u08 temp1 = GetByteFromBuf(inBuf, 6U);
    u08 temp2 = CRC8Algorithm(inBuf);

    if(temp1 == temp2)
    {
        ucRespCode = NRC_SUCCESS;

        switch(sid)
        {
            case 0x11: //ECU Reset
                Parse11(inBuf, inLen, outbuf, outlen, &ucRespCode);
                break;
            case 0xA0:
            case 0xA1:
            case 0xA2:
                ParseProcessFlowID(inBuf, outbuf, outlen, &ucRespCode);//, inLen
                break;
            case 0xB0: // calibration mode
                ParseB0(inBuf, inLen, outbuf, outlen,  &ucRespCode);
                break;

            case 0xBC: //Write K-Data By Identifier
            case 0xBD: //Write B-Data By Identifier
            case 0xB2: //Read K-Data By Identifier
            case 0xB3: //Read B-Data By Identifier
            case 0xBE: //Input Output Control
                if(((0xBC == sid) || (0xBD == sid)) && (calibration_mode_exit == CalibrationModeFlag))
                {
                    ucRespCode = NRC_NOT_ENTERING;
                    break;
                }
                ParseKB(inBuf, outbuf, outlen,  &ucRespCode);//, inLen
                break;

            default:
                ucRespCode = NRC_SNS;
                break;
        }
    }

    /*handling problems during execution*/
    return ucRespCode;
}

/*******************************************************************************
  * @brief      :  Calibration Service
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
static void CalibrationService(CALIBRATION_TD *pdc)
{
	volatile u08 ucRespCode = NRC_NOT_ENTERING;
    
	u08 *inBuf = pdc->tppkg.rx.buf;
	u16 *inLen = &pdc->tppkg.rx.len;
	u08 *outbuf = pdc->tppkg.tx.buf;
	u16 *outlen = &pdc->tppkg.tx.len;
    
    u08 sid = GetByteFromBuf(pdc->tppkg.rx.buf, 0U);

    switch(sid)
    {
        case 0x11: //ECU Reset
        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xB0: // calibration mode
        case 0xBC: //Write K-Data By Identifier
        case 0xBD: //Write B-Data By Identifier
        case 0xB2: //Read K-Data By Identifier
        case 0xB3: //Read B-Data By Identifier
        case 0xBE: //Input Output Control
            ucRespCode = CalibrationServiceHandle(inBuf, inLen, outbuf, outlen);
            break;

        case 0x22:
            ucRespCode = Dcm_0x966_22(inBuf, inLen, outbuf, outlen);
            break;
        case 0x2E:
            ucRespCode = Dcm_0x966_2E(inBuf, inLen, outbuf, outlen);
            break;
        default:
            ucRespCode = NRC_SNS;
            break;
    }

	// handling problems during execution
    if(ucRespCode)
    {
    	CalibrationRepsNRC(outbuf, outlen, sid, ucRespCode);
        
        pdc->tppkg.Force_Negative_Response = FALSE;
    }

	isotp_sendmsg(&pdc->tppkg, pdc->tppkg.tx.buf, pdc-> tppkg.tx.len);
}

static void CalibrationCanTxFrame(isotp_pkg *pkg, Can_Frame *cf)
{
	CALIBRATION_TD *pdc = &CalTp;

    Can_Send_msg(pdc->pCanBase_reps, cf, pkg->IDEx);
}

/*******************************************************************************
  * @brief      :  Calbration Initialize
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void CalbrationInit(CALIBRATION_TD *pdc,u32 TxId, u32 RxId, u08 Ext,App_CanBasic *pCanBase_reps)
{
	AppCalibrationDataInit();

	pdc->pCanBase_reps = pCanBase_reps;
	isotp_init(&pdc->tppkg);
    pdc->tppkg.RxID = RxId;
    pdc->tppkg.TxID = TxId;
    pdc->tppkg.IDEx = Ext;
    pdc->tppkg.send_frame = CalibrationCanTxFrame;
    pdc->tppkg.tx.tx_ok = 1;

    calPortInit();
    appDidInit();

    ts_processFlow.m_unlockFlg = 0;
}

/*******************************************************************************
  * @brief      :  Calbration Process
  * @parameter  :  NONE
  * @return     :  NONE
*******************************************************************************/
void CalbrationProcess(CALIBRATION_TD *pdc)
{
	Can_Frame frame = {0};

    //Calibration Address Message Processing
    if(CanPublic_Recv_msg(CANMSG_CALIBRATION_REQUST, &frame) == SUCCESS)
	{
        if(frame.id == pdc->tppkg.RxID)
        {
        	isotp_recvmsg(CAL_MSG, &pdc->tppkg, &frame);
        }
	}

    // ISOTP Layer Data Processing
    isotp_realtime_process(&pdc->tppkg);

    if(0 != pdc->tppkg.rx.complete)
    {
        pdc->tppkg.rx.complete = 0;

        CalibrationService(pdc);
    }
}

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
