/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_icon.h
** Description:		黑白屏图标相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_ICON_H_
#define _HAL_ICON_H_


#include "comm.h"

#define ICON_LOG_LEVEL_0	LOG_LEVEL_0
#define ICON_LOG_LEVEL_1	LOG_LEVEL_1
#define ICON_LOG_LEVEL_2	LOG_LEVEL_2
#define ICON_LOG_LEVEL_3	LOG_LEVEL_3
#define ICON_LOG_LEVEL_4	LOG_LEVEL_4
#define ICON_LOG_LEVEL_5	LOG_LEVEL_5


#define ICON_GRAY			1
#if 0
#if(LCD_DIRECTION == 0)//横屏

#define ICON_BATTERY_X			116
#define ICON_GPRS_X				18
#define ICON_BT_X				42
#define ICON_WIFI_X				30
#define ICON_USB_X				104
#define ICON_ICCARD_X			92
#define ICON_LOCK_X				80
#define ICON_GPRSSIG_X			12

#define ICON_TIME_X				48

#define BIGBATTION_X			24
#define BIGBATTION_Y			16

#elif(LCD_DIRECTION == 1)//竖屏

#define ICON_BATTERY_X			84
#define ICON_GPRS_X				18
#define ICON_BT_X				42
#define ICON_WIFI_X				30
#define ICON_USB_X				72
#define ICON_ICCARD_X			92
#define ICON_LOCK_X				80
#define ICON_GPRSSIG_X			12


#define ICON_TIME_X				30

#define BIGBATTION_X			8
#define BIGBATTION_Y			24

#endif
#endif

typedef enum _ICON_ORDER
{
	BATTERY_ICON=0,
	GPRS_ICON,
	BT_ICON,
	WIFI_ICON,
	USB_ICON,
	ICCARD_ICON,
	LOCK_ICON,
	GPRSSIG_ICON,
	MAX_ICON
}ICON_ORDER;


typedef struct _LCD_ICON_DISP{
	unsigned char * iconBuff[MAX_ICON];   //用于存储图标点阵的数组地址
	unsigned char UseFlag[MAX_ICON];  //用于存储图标的模式
	unsigned short usColor[MAX_ICON];//存储图标颜色
	unsigned char g_ucIconDispMode[MAX_ICON];//存储屏幕已经显示的图标模式
}LCD_ICON_DISP;




void hal_iconRefresh(void);
void hal_iconLoop(void);
void hal_iconInit(void);

void hal_iconHysteresistest(void);


#endif





