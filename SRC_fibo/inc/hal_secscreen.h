/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_secscreen.h
** Description:		屏幕相关接口
**
** Version:	1.0, 渠忠磊,2022-07-06
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/
#ifndef _HAL_SECSCREEN_H_
#define _HAL_SECSCREEN_H_

#include "hal_font.h"
#include "comm.h"

#define SECSCR_LOG_LEVEL_0			LOG_LEVEL_0
#define SECSCR_LOG_LEVEL_1			LOG_LEVEL_1
#define SECSCR_LOG_LEVEL_2			LOG_LEVEL_2
#define SECSCR_LOG_LEVEL_3			LOG_LEVEL_3
#define SECSCR_LOG_LEVEL_4			LOG_LEVEL_4
#define SECSCR_LOG_LEVEL_5			LOG_LEVEL_5


#define SECSCR_BACKLIGHTTIMEOUT	10000//屏幕背光延时变暗超时时间，单位ms

	
extern volatile int32 g_i32SegLcdBacklightTimeout;
extern GUI_POINT  g_stSegLcdCurPixel;


void hal_secscrBackLightHandle(void);
void hal_secscrBackLightWakeup(void);
int hal_secscrGetBackLightTime(void);

int hal_secscrCls(void);
int hal_secscrClrLine(unsigned char startline, unsigned char endline);
void hal_secscrSetBackLightMode(ushort mode, ushort time);
void hal_secscrSetBackLightValue(unsigned char ucValue);
int hal_secscrSetAttrib(LCD_ATTRIBUTE_t attr, int value);
int hal_secscrGetAttrib(LCD_ATTRIBUTE_t attr);
void hal_secscrGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y);
void hal_secscrGetxyPix(int *pixel_X, int *pixel_Y);
void hal_secscrPrintf(const char *fmt, ...);
void hal_secscrPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...);
void hal_secscrTextOut(uint pixel_X, uint pixel_Y, unsigned char *str);
int hal_secscrDrawDot(unsigned short int x, unsigned short int y, unsigned char Mode);
int hal_secscrGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor);
int hal_secscrDrawStraightLine(unsigned short int sx, unsigned short int sy, unsigned short int ex, unsigned short int ey, unsigned char Mode);
void hal_secscrDrawRect(int left, int top, int right, int bottom, unsigned char Mode);
int hal_secscrDrawRectBlock(int left,int top,int right,int bottom, unsigned int color);
int hal_secscrWriteLogo(int left, int top, unsigned char *data);
void hal_secscrGetLcdSize(int *width, int *height);
void hal_secscrRefresh(void);
void hal_secscrOpen(void);
void hal_secscrClose(void);

void hal_secscrDrawRectColor(int left, int top, int right, int bottom, unsigned int color);
void hal_secscrPrintxy(uint col, uint row, unsigned char mode, const char *str, ...);



void secscrtest(void);



#endif



