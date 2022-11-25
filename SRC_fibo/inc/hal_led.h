/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_led.h
** Description:		LED相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/
#ifndef _HAL_LED_H_
#define _HAL_LED_H_

#include "comm.h"


#define LED_LOG_LEVEL_0		LOG_LEVEL_0
#define LED_LOG_LEVEL_1		LOG_LEVEL_1
#define LED_LOG_LEVEL_2		LOG_LEVEL_2
#define LED_LOG_LEVEL_3		LOG_LEVEL_3
#define LED_LOG_LEVEL_4		LOG_LEVEL_4
#define LED_LOG_LEVEL_5		LOG_LEVEL_5



typedef enum{

	PICCLEDRED = 0x01,
	PICCLEDBLUE,
	PICCLEDGREEN,
	PICCLEDYELLOW,
	
	LEDRED = 0x11,
	LEDBLUE,
	LEDYELLOW,
	
}LED_INDEX_ID;

/*
*Function:		hal_ledInit
*Description:	LED初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ledInit(void);

/*
*Function:		hal_ledCtl
*Description:	LED控制
*Input:			ucLedIndex:灯的索引值，参考LED_INDEX_ID;ucOnOff:灯的亮灭，0-灭，1-亮
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_ledCtl(uchar ucLedIndex,uchar ucOnOff);

/*
*Function:		hal_ledRedHandle
*Description:	Led Red灯执行句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		必须每100ms循环调用一次
*/
void hal_ledRedHandle(void);

/*
*Function:		hal_ledRun
*Description:	控制LED灯闪烁
*Input:			led_id:灯的索引值，参考LED_INDEX_ID;led_state:0-长灭；1-常亮；2-低频闪；3-高频闪；0xFF-释放底层对灯的控制
*				count:0：一直闪烁,其他：闪烁次数
*Output:		NULL
*Hardware:
*Return:		TRUE-成功；FALSE-失败
*Others:		
*/
BOOL hal_ledRun(uint8 led_id,uint8 led_state, int count);

extern int g_iLed_exist; 				//1-有       ，  2-无
extern int g_iTricolorLED_exist; 		//1-有三色灯  ， 2-无



#endif
