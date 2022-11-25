/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_seg.h
** Description:		副屏相关接口
**
** Version:	1.0, 渠忠磊,2022-07-05
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/


#ifndef _HAL_SEG_H_
#define _HAL_SEG_H_

#include "comm.h"

#define SEGLCD_LOG_LEVEL_0		LOG_LEVEL_0
#define SEGLCD_LOG_LEVEL_1		LOG_LEVEL_1
#define SEGLCD_LOG_LEVEL_2		LOG_LEVEL_2
#define SEGLCD_LOG_LEVEL_3		LOG_LEVEL_3
#define SEGLCD_LOG_LEVEL_4		LOG_LEVEL_4
#define SEGLCD_LOG_LEVEL_5		LOG_LEVEL_5


#define SEGLCD_GRAYMODE		0//0-不要灰显，1-要灰显(此处只有单色)

#define SEGLCD_X_PIXELMIN		0
#define SEGLCD_Y_PIXELMIN		0
#define SEGLCD_X_PIXELMAX		127//0-127,X像素最大值
#define SEGLCD_Y_PIXELMAX		31//0-31,Y像素最大值


#define SEGLCD_LINEMAX			2
#define SEGLCD_X_USERPIXELMIN	0
#define SEGLCD_Y_USERPIXELMIN	0
#define SEGLCD_X_USERPIXELMAX	SEGLCD_X_PIXELMAX
#define SEGLCD_Y_USERPIXELMAX	SEGLCD_Y_PIXELMAX

#define SEGLCD_TEXTPIXEL		12//字符占用12*12

#define SEGLCD_PIXWIDTH			128
#define SEGLCD_PIXHIGH			32
#define SEGLCD_SUMPIXNUM		(SEGLCD_PIXWIDTH * SEGLCD_PIXHIGH)
#define SEGLCD_PIXNUM			((SEGLCD_PIXWIDTH * SEGLCD_PIXHIGH)/8)


#define SEGLCD_BLOCKBYTE		8//一个block实际存储为只占用了低4bit,实际上开辟空间一个block为8*LCD_BLOCKWIDTH
#define SEGLCD_BLOCKWIDTH		128//一个block宽为128个像素点
#define SEGLCD_BLOCKHIGH		8//一个block高为8个像素点
#define SEGLCD_BLOCKPIX			(SEGLCD_BLOCKWIDTH * SEGLCD_BLOCKHIGH)//一个block大小
#define SEGLCD_BLOCKBUFNUM		(SEGLCD_SUMPIXNUM/SEGLCD_BLOCKPIX)//总共多少个block

#define SEGLCD_PAGEMAX				3//0~3,总共4页，1页像素128*8
#define SEGLCD_PAGEHIGH				8//一页高为8个像素

extern LCD_GUI	g_stSegLcdGUI;


void hal_segCls(void);
void hal_segSendData(int pagenum, unsigned char col, unsigned char *data, uint32 datalen);

void hal_segDrawSinglePix(unsigned short int x,unsigned short int y,unsigned char dotColor);

void hal_segWriteBitData(int x, int y, unsigned char *dat, int width, int height);
void hal_segDrawSingleLine(int x1,int y1,int x2,int y2, unsigned char mode);
void hal_segDrawStraightLine(int x1,int y1,int x2,int y2, unsigned char mode);
void hal_segDrawRectangle(int x1,int y1,int x2,int y2, unsigned char mode);
int hal_segDrawRectBlock(int left,int top,int right,int bottom, unsigned short color);
void hal_segDrawCircle(unsigned short int x0, unsigned short int y0, unsigned short int r);
int hal_segClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);
int hal_segClsUserArea(void);
int hal_segClrLine(unsigned char startline, unsigned char endline);
int hal_segWriteBitMap(int left, int top, unsigned char *data);
void hal_segTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, uint8 mode);
int hal_segSetAttrib(LCD_ATTRIBUTE_t attr, int value);
int hal_segGetAttrib(LCD_ATTRIBUTE_t attr);
int hal_segGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y);
void hal_segGetxyPix(int *pixel_X, int *pixel_Y);
int hal_segGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor);
void hal_segGetLcdSize(int *width, int *height);
void hal_segPrintf(const char *fmt, ...);
void hal_segPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...);
void hal_segPrintxy(uint col, uint row, unsigned char mode, const char *str, ...);
int8 hal_segContrastSet(uint8 value);
int8 hal_segContrastGet(void);
int hal_segBackLightCtl(uint8 value);
void hal_segSetBackLightValue(unsigned char ucValue);
int hal_segOpen(int backlight);
int hal_segClose(void);





void hal_segInit(void);


#endif
