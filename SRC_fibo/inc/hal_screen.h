/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_screen.h
** Description:		屏幕相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/
#ifndef _HAL_SCREEN_H_
#define _HAL_SCREEN_H_

#include "hal_font.h"
#include "comm.h"

#define SCR_LOG_LEVEL_0			LOG_LEVEL_0
#define SCR_LOG_LEVEL_1			LOG_LEVEL_1
#define SCR_LOG_LEVEL_2			LOG_LEVEL_2
#define SCR_LOG_LEVEL_3			LOG_LEVEL_3
#define SCR_LOG_LEVEL_4			LOG_LEVEL_4
#define SCR_LOG_LEVEL_5			LOG_LEVEL_5

#if 0
#define LCD_DIRECTION		1//0-横屏，1-竖屏
#endif
/*
*横向logo指的是正向的logo文件,竖向logo时指的正向logo文件顺时针旋转90度后的图片取模，
*因161是竖屏,厂家没有适配竖屏刷logo，我们考虑将logo顺时针旋转90度后写入设备中
*/
#define LCD_LOGODIRECTION	1//0-横向logo,1-竖向logo


#define BACKLIGHTTIMEOUT	90000//屏幕背光延时变暗超时时间，单位ms

#define FIRSTSCREEN_SPISPEED_W	50000000
#define FIRSTSCREEN_SPISPEED_R	500000

#define SECONDSCREEN_SPISPEED	20000000

typedef struct _GUI_POINT
{
	unsigned int x;
	unsigned int y;
}GUI_POINT;

typedef enum
{
    LCD_ATTR_POWER = 0,//液晶电源控制
    LCD_ATTR_TEXT_INVERT,//字体反显
    LCD_ATTR_BACK_COLOR,//设置彩屏背景色
    LCD_ATTR_FRONT_COLOR,//设置彩屏前景色
    LCD_ATTR_AUTO_UPDATE, //此项控制是否自动刷新RAM中的内容到屏幕。
    LCD_ATTR_ICON_BACK_COLOR,//设置图标区背景色
   	LCD_ATTR_LIGHT_LEVEL,//设置背光亮度
   	LCD_ATTR_FONTTYPE,//0:UTF-8; 1:GB2312; 2:UNICODE


   	LCD_ATTR_ICONHEIGHT,//标题栏高度
   	LCD_ATTR_WIDTH,//LCD屏实际宽度
   	LCD_ATTR_HEIGHT,//LCD屏实际高度
   	LCD_ATTR_USERWIDTH,//LCD屏可使用宽度
   	LCD_ATTR_USERHEIGHT,//LCD屏可使用高度
   	LCD_ATTR_LCDCOLORVALUE,//1-单色,2-RGB332,3-RGB565,4-RGB888
   	LCD_ATTR_BACKLIGHT_MODE,//1-背光模式
    
}LCD_ATTRIBUTE_t;

typedef enum
{
	SCREEN_UNLOCK = 0,//屏幕竞争解锁
	SCREEN_LOCK = 1,//屏幕竞争加锁
	
	
}SCREEN_MUTEX;

typedef enum
{

	SCREEN_FIRST = 0,//主屏
	SCREEN_SECOND = 1,//副屏
	
}SCREEN_INDEX;

extern uint8 g_ui8ScreenType;					//屏幕类型：0 段码屏幕，1 点阵屏幕

#define LAN_ENG (0)
#define LAN_CHN (1)	
extern volatile uint8 g_ui8ScrLanguage;
extern volatile uint8 g_ui8FullScreen;
extern volatile int32 g_i32BacklightTimeout;
extern GUI_POINT  g_stCurPixel;
extern GUI_POINT  g_stCurPixelIcon;
extern volatile uint32 g_ui32LcdDevID;
extern volatile uint8 g_ui8BackLightValue;


extern ST_FONT g_stSingleFont12X24;
extern ST_FONT g_stSingleFont6X12;
extern ST_FONT g_stSingleFont8X16;
extern ST_FONT g_stUniMultiFont12X12;
extern ST_FONT g_stUniMultiFont16X16;
extern ST_FONT g_stUniMultiFont24X24;
extern ST_FONT g_stGBMultiFont12X12;
extern ST_FONT g_stGBMultiFont16X16;
extern ST_FONT g_stGBMultiFont24X24;

int hal_scrMutexCS(SCREEN_MUTEX locksta, SCREEN_INDEX lcdcs);


void hal_scrBackLightHandle(void);
void hal_scrBackLightWakeup(void);
int hal_scrGetBackLightTime(void);

int hal_scrCls(void);
int hal_scrClrLine(unsigned char startline, unsigned char endline);
void hal_scrSetBackLightMode(ushort mode, ushort time);
void hal_scrSetBackLightValue(unsigned char ucValue);
int hal_scrSetAttrib(LCD_ATTRIBUTE_t attr, int value);
int hal_scrGetAttrib(LCD_ATTRIBUTE_t attr);
void hal_scrGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y);
void hal_scrGetxyPix(int *pixel_X, int *pixel_Y);
void hal_scrPrintf(const char *fmt, ...);
void hal_scrPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...);
void hal_scrTextOut(uint pixel_X, uint pixel_Y, unsigned char *str);
int hal_scrDrawDot(unsigned short int x, unsigned short int y, unsigned char Mode);
int hal_scrGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor);
int hal_scrDrawStraightLine(unsigned short int sx, unsigned short int sy, unsigned short int ex, unsigned short int ey, unsigned char Mode);
void hal_scrDrawRect(int left, int top, int right, int bottom, unsigned char Mode);
int hal_scrDrawRectBlock(int left,int top,int right,int bottom, unsigned int color);
int hal_scrWriteLogo(int left, int top, unsigned char *data);
int hal_scrLogoVertical2Trans(unsigned char *indata, unsigned char *outdata, unsigned int *width, unsigned int *height);
void hal_scrGetLcdSize(int *width, int *height);
void hal_scrRefresh(void);
int hal_scrEnumFont(ST_FONT *Fonts,int MaxFontNums);
int hal_scrPopDot(unsigned char *popDot,int lenth);
int hal_scrPushDot(unsigned char * pushDot,int lenth);
void hal_scrDrawRectColor(int left, int top, int right, int bottom, unsigned int color);
void hal_scrPrintxy(uint col, uint row, unsigned char mode, const char *str, ...);
int hal_scrFullScreenSet(uint8 value);


int8 hal_scrBackLightCtl(uint8 value);
void hal_scrGpioInit(uint32 value);
int hal_scrInit(void);

int Ft_GetDot_ASCII(int reverse, ST_FONT *CurStFont, const unsigned char *str, unsigned char *width, unsigned char *height, unsigned char *dot);

int hal_scrIconClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);
int hal_scrClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);
int hal_scrWriteBigBatIcon(int left, int top, int num);
void hal_scrIconTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, ST_FONT *CurStFont);
void hal_scrWriteIcon(uint16 x, uint16 y, uint8 *data, uint8 width, uint8 height, unsigned short color);




#endif


