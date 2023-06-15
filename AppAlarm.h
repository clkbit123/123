/***** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******header file*****
* File name          : AlarmApp.h
* Author             : henry
* Brief              : header file of Alarm module
********************************************************************************
* modify
* Version   Date(YYYY/MM/DD)    Author  Described
* V1.00     2017/04/08          henry   First Issue
*******************************************************************************/
#ifndef     APP_ALARM_H
#define     APP_ALARM_H

#ifdef      APP_ALARM_C
#define     APP_ALARM_DEC
#else
#define     APP_ALARM_DEC  extern
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* Export typedef ------------------------------------------------------------*/
typedef u08 (*P_FUN_DEF)(void);

typedef enum
{
    TAT_UPLIMIT = 0,
    TAT_DOWNLIMIT,
    TAT_FLAG
}TE_ALARM_TYPE;

typedef enum
{
    TAT_LOCKFAULT = 0,
    TAT_RECOVERYFAULT
}TE_FAULT_TYPE;

typedef enum
{
    TAT_DATA_CHECK_UP = 0,
    TAT_DATA_CHECK_DOWN,
    TAT_DATA_CHECKING
}TE_DATA_CHECK;

typedef enum
{
    TDAE_STATE_FREE = 0,
    TDAE_STATE_FAULT,
    TDAE_STATE_BUSY
}TE_OBC_ALARM_STATE;   /* OBC alarm */

typedef struct
{
    const char * const name;
    u08 ucEvent;                    /* enent */
    P_FUN_DEF pPrecondition;        /* precondition of alarm manage */
    u32 uwTimeoutSet;                  /* timeout value(ms) */
    u32 uwTimeoutRst;
    u32 uwLimit;                    /* limit data */
    u32 uwHysteresis;               /* hysteresis */
    u32 (* pDataGet)(void);
    TE_ALARM_TYPE tType;            /* alarm type */
    TE_FAULT_TYPE tFaultType;
}TS_ALARM_TABLE_MANAGE;

typedef enum
{
	OBC_ALARM_KIND = 0,
    OBC_CHARGER_ALARM_KIND,
    OBC_DISCHARGER_ALARM_KIND,
	DCDC_ALARM_KIND,

	MAX_ALARM_KIND
}TE_ALARM_KIND;

#define OBC_ALARM_NUM_MAX 32/* timers for timeout max 32 event because uwObcAlarmFlag !!!*/

typedef struct
{
    u32 uwObcAlarmFlag;
    u32 uwObcAlarmMaskRecovery;
    u32 uwObcAlarmMaskLock;
	u32 uwObcTimers[OBC_ALARM_NUM_MAX]; 
    u16 uwObcCount[OBC_ALARM_NUM_MAX];
}TS_ALARM_MANAGE;

typedef struct
{
    const TE_ALARM_KIND kind;
    TS_ALARM_MANAGE * const pAlarm;
    const TS_ALARM_TABLE_MANAGE * pTable;
    const u08 ucMaxEvent;                 /* max event number */
}TS_ALARM_CHECK;

/* Export define -------------------------------------------------------------*/
#define ALARM_SET       				1U
#define ALARM_CLEAR     				0U

/* Export macro --------------------------------------------------------------*/
/* Export variables ----------------------------------------------------------*/
APP_ALARM_DEC const TS_ALARM_TABLE_MANAGE tDcdcAlarmMngTable[];
APP_ALARM_DEC const TS_ALARM_TABLE_MANAGE tObcAlarmMngTable[];
APP_ALARM_DEC const TS_ALARM_TABLE_MANAGE tObcChargerAlarmMngTable[];
APP_ALARM_DEC const TS_ALARM_TABLE_MANAGE tObcDischargerAlarmMngTable[];

/* Export function -----------------------------------------------------------*/
APP_ALARM_DEC void AppAlarmInit(void);
APP_ALARM_DEC void AppAlarmProcess(void);
APP_ALARM_DEC u08 DcdcNoClearAlarm(void);

#endif /* End ALARM_APP_H */

/******** (C) Copyright, Shenzhen SHINRY Technology Co.,Ltd. ******** End *****/
