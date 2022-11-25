/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_lcd.h
** Description:		黑白屏相关接口
**
** Version:	1.0, 渠忠磊,2022-02-25
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_LCD_H_
#define _HAL_LCD_H_

#include "hal_font.h"
#include "comm.h"

#define LCD_LOG_LEVEL_0		LOG_LEVEL_0
#define LCD_LOG_LEVEL_1		LOG_LEVEL_1
#define LCD_LOG_LEVEL_2		LOG_LEVEL_2
#define LCD_LOG_LEVEL_3		LOG_LEVEL_3
#define LCD_LOG_LEVEL_4		LOG_LEVEL_4
#define LCD_LOG_LEVEL_5		LOG_LEVEL_5


/*******************硬件ID*******************************/
#define HLTLCD_2_4_UC1617S_ID					0b000//7
#define FYTLCD_2_4_ST7571_ID					0b001//0
#define FRDLCD_2_8_IST3980CA1_ID				0b011

#define FYTCOLORLCD_2_4_ST7789V2_ID				0b101
#define GZGCOLORLCD_2_8_ST7789V2_ID				0b111
#define ZXCOLORLCD_2_4_ST7789V3A_ID				0b110
#define FRDCOLORLCD_2_4_ST7789V2_ID				0b110

/*******************Device ID*******************************/
#define ST7789V2_ID								0x858552
#define ST7789V3A_ID							0x898966

 

#define LCD_GRAYMODE		0//0-不要灰显，1-要灰显，LcdWrite_uc1617s只支持黑白显

#define SIZE_1K     						0x400
#define SIZE_2K								0x800
#define SIZE_4K								0x1000
#define SIZE_8K								0x2000
#define SIZE_16K							0x4000
#define SIZE_64K							0x10000
#define SIZE_128K							0x20000
#define SIZE_256K							0x40000
#define SIZE_512K							0x80000
#define SIZE_1M								0x100000
#define SIZE_2M								0x200000
#define SIZE_4M								0x400000
#define SIZE_8M								0x800000
#define SIZE_16M							0x1000000
#define SIZE_32M							0x2000000


#define EXT_FLASH_BASE 						(0x00000000)/*外挂*/
#define EXT_FLASH_MAX_ADDR     				(SIZE_8M)
#define EXT_FLASH_SIZE         				EXT_FLASH_MAX_ADDR


 

/*===========================FONT===========================*/

#define EXT_FLASH_FONT_START                        (EXT_FLASH_BASE)
#define EXT_FLASH_FONT_SIZE                         (SIZE_1M+SIZE_512K-SIZE_4K)

#define EXT_FLASH_FONT_INFO_START                   (EXT_FLASH_FONT_START+EXT_FLASH_FONT_SIZE)
#define EXT_FLASH_FONT_INFO_SIZE		            (SIZE_4K)

#if 0
#if(LCD_DIRECTION == 0)//横屏

#define LCD_X_PIXELMIN		0//
#define LCD_Y_PIXELMIN		0
#define LCD_X_PIXELMAX		127//0-127,X像素最大值
#define LCD_Y_PIXELMAX		95//0-95,Y像素最大值


#define LCD_LINEMAX			7//0-7,包括图标这一行
#define LCD_X_USERPIXELMIN	LCD_X_PIXELMIN
#define LCD_Y_USERPIXELMIN	12//
#define LCD_X_USERPIXELMAX	LCD_X_PIXELMAX
#define LCD_Y_USERPIXELMAX	LCD_Y_PIXELMAX

#define LCD_TEXTPIXEL		12//字符占用12*12

#define LCD_PIXWIDTH			128
#define LCD_PIXHIGH			    96
#define LCD_SUMPIXNUM           (LCD_PIXWIDTH * LCD_PIXHIGH)
#define LCD_PIXNUM				((LCD_PIXWIDTH * LCD_PIXHIGH)/8)


#define LCD_BLOCKBYTE			8//一个block实际存储为只占用了低4bit,实际上开辟空间一个block为8*LCD_BLOCKWIDTH
#define LCD_BLOCKWIDTH		    128//一个block宽为128个像素点
#define LCD_BLOCKHIGH			4//一个block高为4个像素点
#define LCD_BLOCKPIX			(LCD_BLOCKWIDTH * LCD_BLOCKHIGH)//一个block大小
#define LCD_BLOCKBUFNUM			(LCD_SUMPIXNUM/LCD_BLOCKPIX)//总共多少个block

#elif(LCD_DIRECTION == 1)//竖屏

#define LCD_X_PIXELMIN		0//
#define LCD_Y_PIXELMIN		0
#define LCD_X_PIXELMAX		95//0-95,Y像素最大值
#define LCD_Y_PIXELMAX		127//0-127,X像素最大值


#define LCD_LINEMAX			9//0-9,包括图标这一行
#define LCD_X_USERPIXELMIN	LCD_X_PIXELMIN
#define LCD_Y_USERPIXELMIN	12//
#define LCD_X_USERPIXELMAX	LCD_X_PIXELMAX
#define LCD_Y_USERPIXELMAX	LCD_Y_PIXELMAX

#define LCD_TEXTPIXEL		12//字符占用12*12

#define LCD_PIXWIDTH			96
#define LCD_PIXHIGH			    128
#define LCD_SUMPIXNUM           (LCD_PIXWIDTH * LCD_PIXHIGH)
#define LCD_PIXNUM				((LCD_PIXWIDTH * LCD_PIXHIGH)/8)


#define LCD_BLOCKBYTE			8//一个block实际存储为只占用了低4bit,实际上开辟空间一个block为8*LCD_BLOCKWIDTH
#define LCD_BLOCKWIDTH		    96//一个block宽为128个像素点
#define LCD_BLOCKHIGH			4//一个block高为4个像素点
#define LCD_BLOCKPIX			(LCD_BLOCKWIDTH * LCD_BLOCKHIGH)//一个block大小
#define LCD_BLOCKBUFNUM			(LCD_SUMPIXNUM/LCD_BLOCKPIX)//总共多少个block
#endif
#endif


#define LCD_PAGEMAX				15//0~15,总共16页，1页像素96*8
#define LCD_PAGEHIGH			8//一页高为8个像素

#define LCD_PAGEMAX_UC1617S			23//0~23,总共24页，1页像素128*4，仅适用于uc1617s
#define LCD_PAGEHIGH_UC1617S		4//一页高为4个像素，仅适用于uc1617s


#define LCD_ERR_SUCCESS					0//成功
#define SINGLE_CODE_FONT_NON			-2501//SingleCodeFont不存在
#define MULTICODEFONT_CODE_FONT_NON		-2502//MultiCodeFont不存在
#define SINGLE_CODE_FONT_OVER			-2503//SingleCodeFont字体超范围
#define MULTICODEFONT_CODE_FONT_OVER	-2504//MultiCodeFont字体超范围
#define FONTS_BUF_OVERFLOW				-2505//实际读到的字体个数超过Fonts缓冲可以存放的字体个数
#define LCD_ERR_PARAM_ERROR				-2506//传入指针为空/参数错误
#define LCD_ERR_PIC_DATA_EMPTY			-2507//数据为空
#define LCD_ERR_PIC_DATA_OVERFLOW		-2508//图片文件太大或内存不足
#define LCD_ERR_AREA_INVALID			-2509//指定区域无效
#define LCD_ERR_COORDINATE_INVALID		-2510//坐标无效
#define LCD_ERR_MODE					-2511//错误模式
#define LCD_ERR_OUT_OF_SCREEN			-2512//超出屏幕范围
#define LCD_ERR_FONT_PARAM				-2513//错误的字库参数
#define SCR_SET_ATTRIB_ERR				-2514//设置液晶显示屏功能属性失败
#define SCR_GET_ATTRIB_ERR				-2515//获取液晶显示屏功能属性失败
#define SCR_NOT_SUPPORT					-2516//不支持该配置参数
#define LCD_ERR_ICON_PARAM				-2517//图标传入参数错误
#define LCD_ERR_ICON_MODE_PARAM			-2518//图标模式参数错误
#define LCD_ERR_ICON_PRIORITY_PARAM		-2519//图标优先级参数错误
#define LCD_ERR_NO_COLOR_INDEX			-2520//没有颜色索引表
#define LCD_ERR_COLOR_INDEX_OVER		-2521//超出索引表范围
#define LCD_ERR_NO_FONT					-2522//所选字库不存在


typedef struct{

    unsigned short iconAreaColor;//图标区 标题栏颜色
    unsigned short iconColor;//图标的颜色
} ICON_ATTR;

typedef struct _LCD_CONFIG{
	int LCD_DIRECTION;
	int LOGODEFAULTPOSITION_X;
	int LOGODEFAULTPOSITION_Y;
	int COLORLOGODEFAULTPOSITION_X;
	int COLORLOGODEFAULTPOSITION_Y;
	int COLORICON_BATTERY_X;
	int COLORICON_GPRS_X;
	int COLORICON_BT_X;
	int COLORICON_WIFI_X;
	int COLORICON_USB_X;
	int COLORICON_ICCARD_X;
	int COLORICON_LOCK_X;
	int COLORICON_TIME_X;
	int COLORBIGBATTION_X;
	int COLORBIGBATTION_Y;
	int COLORLCD_PIXWIDTH;
	int COLORLCD_PIXHIGH;
	unsigned long COLORLCD_SUMPIXNUM;
	unsigned long COLORLCD_PIXNUM;
	int COLORLCD_BLOCKWIDTH;
	int COLORLCD_BLOCKHIGH;
	unsigned long COLORLCD_BLOCKPIX;//一个block大小
	unsigned long COLORLCD_BLOCKBUFNUM;//总共多少个block	
	int ICON_BATTERY_X;
	int ICON_GPRS_X;
	int ICON_BT_X;
	int ICON_WIFI_X;
	int ICON_USB_X;
	int ICON_ICCARD_X;
	int ICON_LOCK_X;
	int ICON_GPRSSIG_X;
	int ICON_TIME_X;
	int BIGBATTION_X;
	int BIGBATTION_Y;
	int LCD_X_PIXELMIN;//
	int LCD_Y_PIXELMIN;
	int LCD_X_PIXELMAX;//0-127,X像素最大值
	int LCD_Y_PIXELMAX;//0-95,Y像素最大值
	int LCD_LINEMAX;//0-7,包括图标这一行
	int LCD_X_USERPIXELMIN;
	int LCD_Y_USERPIXELMIN;//
	int LCD_X_USERPIXELMAX;
	int LCD_Y_USERPIXELMAX;
	int LCD_TEXTPIXEL;//字符占用12*12
	int LCD_PIXWIDTH;
	int LCD_PIXHIGH;
	unsigned long LCD_SUMPIXNUM;
	unsigned long LCD_PIXNUM;
	int LCD_BLOCKBYTE;//一个block实际存储为只占用了低4bit,实际上开辟空间一个block为8*LCD_BLOCKWIDTH
	int LCD_BLOCKWIDTH;//一个block宽为128个像素点
	int LCD_BLOCKHIGH;//一个block高为4个像素点
	unsigned long LCD_BLOCKPIX;//一个block大小
	unsigned long LCD_BLOCKBUFNUM;//总共多少个block	

}LCD_CONFIG;


typedef struct _LCD_GUI{
    
	unsigned short	    grapBackColor;//背景色
	unsigned short	    grapFrontColor;//前景色    
    ICON_ATTR 			icon_attr;   
	unsigned short int	Mode;//正显反显标志	
	//unsigned char 		RefreshFlag[LCD_BLOCKBUFNUM];//刷新标志位   
	unsigned char 		*RefreshFlag;//刷新标志位  
    unsigned char      	*DispBuff;//[LCD_PIXNUM];//像素缓存
    unsigned char		AutoRefresh;
	unsigned char		BackLight;//背光值
	unsigned char		BackLightEN;//背光是否打开
	unsigned char		BackLightMode;//背光模式，0-关闭，1-延时关闭，2-常亮
	unsigned int		BackLightTimeout;//当延时关闭模式打开时，保存延时关闭时间，单位ms
	unsigned char		fonttype;//0:UTF-8; 1:GB2312; 2:UNICODE
	unsigned char		rotate;//0-正显;1-倒显，旋转180°显示
	unsigned char		*rotateBuff;//旋转之后的缓存
	unsigned char		status;//0-close;1-open
}LCD_GUI;



extern LCD_GUI	g_stLcdGUI;
extern struct _LCD_CONFIG g_stLcdConfig;
void hal_lcdRefresh(void);
void hal_lcdDrawSinglePix(unsigned short int x,unsigned short int y,unsigned char dotColor);
int hal_lcdDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode);//不带refresh
void hal_lcdDrawSingleLine(int x1,int y1,int x2,int y2, unsigned char mode);
void hal_lcdDrawStraightLine(int x1,int y1,int x2,int y2, unsigned char mode);
void hal_lcdDrawRectangle(int x1,int y1,int x2,int y2, unsigned char mode);
int hal_lcdDrawRectBlock(int left,int top,int right,int bottom, unsigned short color);
void hal_lcdDrawCircle(unsigned short int x0, unsigned short int y0, unsigned short int r);
int hal_lcdClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);
int hal_lcdClsUserArea(void);
int hal_lcdClrLine(unsigned char startline, unsigned char endline);
void hal_lcdIconTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY);
void hal_lcdTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, uint8 mode);
void hal_lcdWriteIcon(uint16 x, uint16 y, uint8 *data, uint8 width, uint8 height, unsigned short color);
int hal_lcdSetAttrib(LCD_ATTRIBUTE_t attr, int value);
int hal_lcdGetAttrib(LCD_ATTRIBUTE_t attr);
int hal_lcdGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y);
void hal_lcdGetxyPix(int *pixel_X, int *pixel_Y);
int hal_lcdGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor);
void hal_lcdGetLcdSize(int *width, int *height);
void hal_lcdPrintf(const char *fmt, ...);
void hal_lcdPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...);
void hal_lcdPrintxy(uint col, uint row, unsigned char mode, const char *str, ...);

int hal_lcdPopDot(unsigned char *popDot,int lenth);
int hal_lcdPushDot(unsigned char *pushDot,int lenth);
int hal_lcdRotate180(uint8 value);
int hal_lcdIconClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);

void hal_lcdInit(void);




#endif


