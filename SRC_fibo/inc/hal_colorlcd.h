/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_colorlcd.h
** Description:		彩屏相关接口
**
** Version:	1.0, 渠忠磊,2022-02-25
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_COLORLCD_H_
#define _HAL_COLORLCD_H_

#include "hal_font.h"
#include "comm.h"

#define COLORLCD_LOG_LEVEL_0 LOG_LEVEL_0
#define COLORLCD_LOG_LEVEL_1 LOG_LEVEL_1
#define COLORLCD_LOG_LEVEL_2 LOG_LEVEL_2
#define COLORLCD_LOG_LEVEL_3 LOG_LEVEL_3
#define COLORLCD_LOG_LEVEL_4 LOG_LEVEL_4
#define COLORLCD_LOG_LEVEL_5 LOG_LEVEL_5



#if 0
#if(LCD_DIRECTION == 0)//横屏

#define COLORLCD_PIXWIDTH			320
#define COLORLCD_PIXHIGH			240
#define COLORLCD_SUMPIXNUM           (COLORLCD_PIXWIDTH * COLORLCD_PIXHIGH)
#define COLORLCD_PIXNUM				((COLORLCD_PIXWIDTH * COLORLCD_PIXHIGH)/8)

#define COLORLCD_BLOCKWIDTH		    320
#define COLORLCD_BLOCKHIGH			8
#define COLORLCD_BLOCKPIX			(COLORLCD_BLOCKWIDTH * COLORLCD_BLOCKHIGH)//一个block大小
#define COLORLCD_BLOCKBUFNUM		(COLORLCD_SUMPIXNUM/COLORLCD_BLOCKPIX)//总共多少个block	

#elif(LCD_DIRECTION == 1)//竖屏

#define COLORLCD_PIXWIDTH			240
#define COLORLCD_PIXHIGH			320
#define COLORLCD_SUMPIXNUM           (COLORLCD_PIXWIDTH * COLORLCD_PIXHIGH)
#define COLORLCD_PIXNUM				((COLORLCD_PIXWIDTH * COLORLCD_PIXHIGH)/8)

#define COLORLCD_BLOCKWIDTH		    240
#define COLORLCD_BLOCKHIGH			8
#define COLORLCD_BLOCKPIX			(COLORLCD_BLOCKWIDTH * COLORLCD_BLOCKHIGH)//一个block大小
#define COLORLCD_BLOCKBUFNUM		(COLORLCD_SUMPIXNUM/COLORLCD_BLOCKPIX)//总共多少个block

#endif
#endif
#define COLORLCD_TEXTPIXEL			24

#define COLORLCD_ICONHIGH 			COLORLCD_TEXTPIXEL
typedef struct _COLORLCD_GUI
{
    
	unsigned short	    grapBackColor;//背景色
	unsigned short	    grapFrontColor;//前景色    
    ICON_ATTR 			icon_attr;   
	unsigned short int	Mode;			//正显反显标志,0-正显；1-反显	
	//unsigned char 		RefreshFlag[COLORLCD_BLOCKBUFNUM];//刷新标志位
	unsigned char 		*RefreshFlag;//刷新标志位    
    unsigned short      *DispBuff;//[COLORLCD_SUMPIXNUM];//像素缓存
    unsigned char		AutoRefresh;
	unsigned char		BackLight;//背光值
	unsigned char		BackLightEN;//背光是否打开
	unsigned char		BackLightMode;//背光模式，0-关闭，1-延时关闭，2-常亮
	unsigned int		BackLightTimeout;//当延时关闭模式打开时，保存延时关闭时间，单位ms
	unsigned char		fonttype;//中文字体编码类型//0:UTF-8; 1:GB2312; 2:UNICODE
	
}COLORLCD_GUI;



extern COLORLCD_GUI	g_stColorlcdGUI;
extern SPIHANDLE g_stspiFd;

int hal_clcdInitfirst(void);

void hal_clcdInit(void);
void hal_clcdIconRefresh(void);
void hal_clcdRefresh(void);
void hal_clcdDrawSinglePix(unsigned short x,unsigned short y,unsigned short dotColor);
int hal_clcdDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode);
void hal_clcdDrawStraightLine(int x1,int y1,int x2,int y2, unsigned char mode);
void hal_clcdDrawRectangle(int x1,int y1,int x2,int y2, unsigned char mode);
int hal_clcdDrawRectBlock(int left,int top,int right,int bottom, unsigned short color);
void hal_clcdDrawCircle(unsigned short int x0, unsigned short int y0, unsigned short int r);
int hal_clcdIconClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);
int hal_clcdClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);
int hal_clcdClsUserArea(void);
int hal_clcdClrLine(unsigned char startline, unsigned char endline);
void hal_clcdIconTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, ST_FONT *CurStFont);
void hal_clcdTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, uint8 mode);
void hal_clcdWriteIcon(uint16 x, uint16 y, uint8 *data, uint8 width, uint8 height, unsigned short color);
int hal_clcdSetAttrib(LCD_ATTRIBUTE_t attr, int value);
int hal_clcdGetAttrib(LCD_ATTRIBUTE_t attr);
int hal_clcdGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y);
void hal_clcdGetxyPix(int *pixel_X, int *pixel_Y);
int hal_clcdGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor);
void hal_clcdGetLcdSize(int *width, int *height);
void hal_clcdPrintf(const char *fmt, ...);
void hal_clcdPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...);
void hal_clcdPrintxy(uint col, uint row, unsigned char mode, const char *str, ...);
int CalTextline(unsigned int col, unsigned char *str, unsigned int strlen, unsigned int align, uint32 *startlinepix, uint32 *startstrlen, uint32 lcdpixwidth);
int Getlenstr(char *str);
int hal_clcdPopDot(unsigned char *popDot, int lenth);
int hal_clcdPushDot(unsigned char *pushDot, int lenth);
int hal_clcdRotate180(uint8 value);

void hal_clcdDrawLineC(int x1,int y1,int x2,int y2, unsigned short color);
void hal_clcdDrawRectangleC(int x1,int y1,int x2,int y2, unsigned short color);
int hal_clcdWriteBMP(int left, int top, int width, int height, unsigned short *data, int datalen);
void hal_clcdDot2DispBuf(unsigned int x,unsigned int y,int width,int height,char *dot,int encode_width);

void hal_lcdDrawLamp(unsigned int color,bool flag);

#endif

