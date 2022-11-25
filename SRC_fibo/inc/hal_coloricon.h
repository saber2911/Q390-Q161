/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_coloricon.h
** Description:		彩屏图标相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_COLORICON_H_
#define _HAL_COLORICON_H_


#include "comm.h"

#define COLORICON_LOG_LEVEL_0	LOG_LEVEL_0
#define COLORICON_LOG_LEVEL_1	LOG_LEVEL_1
#define COLORICON_LOG_LEVEL_2	LOG_LEVEL_2
#define COLORICON_LOG_LEVEL_3	LOG_LEVEL_3
#define COLORICON_LOG_LEVEL_4	LOG_LEVEL_4
#define COLORICON_LOG_LEVEL_5	LOG_LEVEL_5


#define COLORICON_GRAY			0x9BC0//0xAB82//0xA382//0xA364

#if 0
#if(LCD_DIRECTION == 0)//横屏

#define COLORICON_BATTERY_X			296
#define COLORICON_GPRS_X			12
#define COLORICON_BT_X				60
#define COLORICON_WIFI_X			36
#define COLORICON_USB_X				272
#define COLORICON_ICCARD_X			248
#define COLORICON_LOCK_X			224

#define COLORICON_TIME_X			130

#define COLORBIGBATTION_X			68
#define COLORBIGBATTION_Y			56

#elif(LCD_DIRECTION == 1)//竖屏

#define COLORICON_BATTERY_X			216
#define COLORICON_GPRS_X			12
#define COLORICON_BT_X				60
#define COLORICON_WIFI_X			36
#define COLORICON_USB_X				192
#define COLORICON_ICCARD_X			248
#define COLORICON_LOCK_X			224

#define COLORICON_TIME_X			90

#define COLORBIGBATTION_X			30
#define COLORBIGBATTION_Y			80

#endif
#endif

typedef enum _COLORICON_ORDER
{
	COLORBATTERY_ICON=0,
	COLORGPRS_ICON,
	COLORBT_ICON,
	COLORWIFI_ICON,
	COLORUSB_ICON,
	COLORICCARD_ICON,
	COLORLOCK_ICON,
	COLORMAX_ICON
}COLORICON_ORDER;


typedef struct _COLORLCD_ICON_DISP{
	unsigned char * iconBuff[COLORMAX_ICON];   //用于存储图标点阵的数组地址
	unsigned char UseFlag[COLORMAX_ICON];  //用于存储图标的模式
	unsigned short usColor[COLORMAX_ICON];//存储图标颜色
	unsigned char g_ucIconDispMode[COLORMAX_ICON];//存储图标颜色
}COLORLCD_ICON_DISP;




void hal_ciconRefresh(void);
void hal_ciconLoop(void);
void hal_ciconInit(void);

void hal_icionHysteresistest(void);


#endif






