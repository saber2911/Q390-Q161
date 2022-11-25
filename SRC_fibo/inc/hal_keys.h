/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_keys.h
** Description:		按键相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_KEYS_H_
#define _HAL_KEYS_H_

#include "comm.h"


#define KEYS_LOG_LEVEL_0		LOG_LEVEL_0
#define KEYS_LOG_LEVEL_1		LOG_LEVEL_1
#define KEYS_LOG_LEVEL_2		LOG_LEVEL_2
#define KEYS_LOG_LEVEL_3		LOG_LEVEL_3
#define KEYS_LOG_LEVEL_4		LOG_LEVEL_4
#define KEYS_LOG_LEVEL_5		LOG_LEVEL_5


/*****按键ID*****/

#define KEY_1_ID			2
#define KEY_2_ID			3
#define	KEY_3_ID			4
#define KEY_4_ID			5
#define KEY_5_ID			6
#define KEY_6_ID			7
#define KEY_7_ID			8
#define KEY_8_ID			9
#define KEY_9_ID			10
#define KEY_0_ID			1
#define KEY_STAR_ID			15
#define KEY_SHARP_ID		16
#define KEY_CANCLE_ID		0
#define KEY_CLEAR_ID		18
#define KEY_ENTER_ID		17
#define KEY_PLUS_ID			24
#define KEY_MENU_ID			23
#define KEY_DCASH_ID		11

#define PWRKEY_ID			0//模块自身的PWRKEY

#define IRQ_NUM 4
#define DEAL_SHARK_TIME 3

#define KEY_SHORT_IRQV		50
#define KEY_LONG_IRQV		2000


#define KEY_LONG_V		(20-5)*10
#define	KEY_PWR_V		(25-5)*10

#define KEYQUEUELEN		256
#define KEYLONGTIMEOUT	1500

typedef struct _KEY_S{
	uint8 key_state;//按键状态
	uint8 key_value;//按键值
	uint32 key_count;//按键计数
	uint8 key_wc;//按键抖动误差
	uint32 key_count_done;//最终按键计数值
	
}KEY_S;
typedef struct _KEY_TABLE{
	struct _KEY_S key_s_wifi;
	struct _KEY_S key_s_voll;
	struct _KEY_S key_s_volh;
	struct _KEY_S key_s_pwr;
}KEY_TABLE;

typedef struct _KEYIRQ_S{
	uint32 key_uTime;
	uint32 key_uTimeout;
	uint8 key_uTimeOK;
	uint8 key_wc;
	uint32 key_on;//0-未按键;1-按下按键;2-短按;3-长按
	uint8 key_value;//最终按键值，1-短按;2-长按
	

}KEYIRQ_S;

extern int g_iTypeofKeyboard;

typedef void (*KEY_PWR_CALLBACK)(uint8 key_V);
typedef void (*KEYS_CALLBACK)(uint8 key_ID, uint8 key_V);
typedef void (*KEYS_CALLBACK_PWROFF)(void);


extern KEYS_CALLBACK g_fcKeysCallback;
extern uint8 g_ui8KeyPwroffFree;
extern uint8 g_ui8KeyPwronFree;

extern struct _KEY_TABLE g_stKeyTable;

extern struct _KEYIRQ_S g_stKeyPwrIRQ;

extern uint32 g_ui32KeypadStatus;
extern uint32 g_ui32QueueKeyValue;//键值队列
extern uint8 g_ui8PwrkeyFlag;
extern volatile uint8 g_ui8KeySoundFlag;

void hal_keypadPwrOffReg(void (*keys_callback_pwroff_P)(void));
void hal_keypadReg(void (*keys_callback_P)(uint8 key_ID, uint8 key_V));

void hal_keypadPwrkeyDeal(uint8 key_V);
void hal_keypadInit(void);
unsigned char hal_keypadCheck(void);
unsigned char hal_keypadHit(void);
void hal_keypadFlush(void);
unsigned char hal_keypadGetKey(void);
void hal_keypadHandle(void);
void hal_keypadDeal(uint8 key_ID, uint8 key_V);
int hal_keypadSetSound(unsigned char flag);
int hal_keypadGetSound(void);
int hal_kbGetString(unsigned char *pucStr, unsigned char ucMode, unsigned char ucMinLen, unsigned char ucMaxLen, unsigned short usTimeOutSec);


void keytest(void);
void keypwrofftest(void);


#endif


