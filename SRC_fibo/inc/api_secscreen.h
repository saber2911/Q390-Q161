/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_secscreen.h
** Description:		屏幕相关API接口
**
** Version:	1.0, 渠忠磊,2022-07-06
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/
#ifndef _API_SECSCREEN_H_
#define _API_SECSCREEN_H_

#include "hal_font.h"
#include "comm.h"


/*
*Function:		secscrCls_lib
*Description:	LCD 清屏，不包括图标
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
int secscrCls_lib(void);


/*
*Function:		secscrClrLine_lib
*Description:	清除指定的一行或者若干行，
*Input:			startline-起始字符行号，endline-结束字符行号
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int secscrClrLine_lib(unsigned char startline, unsigned char endline);


/*
*Function:		secscrSetBackLightMode_lib
*Description:	设置屏幕背光模式
*Input:			mode:0-背光关闭: 1-背光保持,time*10ms后关闭,2-背光常亮; time:背光时间，单位10ms,仅对mode=1时有效
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrSetBackLightMode_lib(ushort mode, ushort time);


/*
*Function:		secscrSetBackLightValue_lib
*Description:	设置屏幕亮度大小
*Input:			ucValue:亮度值，0~100；
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrSetBackLightValue_lib(unsigned char ucValue);


/*
*Function:		secscrSetAttrib_lib
*Description:	设置屏幕参数，
*Input:			attr:具体参考LCD_ATTRIBUTE_t
*				value:视attr参数而定，
*				LCD_ATTR_POWER	value:0-关闭，1-打开
*				LCD_ATTR_TEXT_INVERT	value:0-反显, 1-正常显示
*				LCD_ATTR_BACK_COLOR		value:0-0xFFFF
*				LCD_ATTR_FRONT_COLOR	value:0-0xFFFF
*				LCD_ATTR_AUTO_UPDATE	value:0-关闭自动刷新屏幕, 1-开启自动刷新，默认情况下是自动刷新
*				LCD_ATTR_ICON_BACK_COLOR	value:0-0xFFFF
*				LCD_ATTR_LIGHT_LEVEL	value:0-100
*				LCD_ATTR_FONTTYPE	0:UTF-8; 1:GB2312; 2:UNICODE
*Output:		NULL
*Hardware:
*Return:		0-设置成功, <0-失败
*Others:
*/
int secscrSetAttrib_lib(LCD_ATTRIBUTE_t attr, int value);


/*
*Function:		secscrGetAttrib_lib
*Description:	获取液晶显示器的功能属性
*Input:			attr:LCD_ATTRIBUTE_t
*Output:		NULL
*Hardware:
*Return:		<0-失败，>=0-成功，具体值为所读取的功能属性value
*Others:
*/
int secscrGetAttrib_lib(LCD_ATTRIBUTE_t attr);


/*
*Function:		secscrGotoxyPix_lib
*Description:	定位LCD显示光标，参数超出LCD范围时不改变原坐标位置
*Input:			pixel_X:横坐标；pixel_Y:纵坐标
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrGotoxyPix_lib(unsigned int pixel_X, unsigned int pixel_Y);


/*
*Function:		secscrGetxyPix_lib
*Description:	读取LCD上光标的当前位置。
*Input:			NULL
*Output:		*pixel_X:横坐标; *pixel_Y:纵坐标
*Hardware:
*Return:		NULL
*Others:
*/
void secscrGetxyPix_lib(int *pixel_X, int *pixel_Y);


/*
*Function:		secscrPrintf_lib
*Description:	在屏幕的前景层当前位置格式化显示字符串
*Input:			*fmt:显示输出的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrPrintf_lib(const char *fmt, ...);


/*
*Function:		secscrPrint_lib
*Description:	在屏幕前景层指定位置格式化显示字符串
*Input:			col:显示的起始点阵列号，row:显示的起始字符行号；
*				mode:显示模式设置，bit0保留，bit1-bit2:对齐方式，bit2bit1:0b00-左对齐，0b01-居中，0b10-右对齐
*				bit3-bit6:保留，bit7:控制反显 1-反显，0-正显
*				*str:显示输入的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrPrint_lib(uint col, unsigned char row, unsigned char mode, const char *str, ...);



/*
*Function:		secscrPrint_xy
*Description:	在屏幕前景层指定位置格式化显示字符串
*Input:			col:显示的起始点阵列号，row:显示的起始点阵行号；
*				mode:显示模式设置，bit0保留，bit1-bit2:对齐方式，bit2bit1:0b00-左对齐，0b01-居中，0b10-右对齐
*				bit3-bit6:保留，bit7:控制反显 1-反显，0-正显
*				*str:显示输入的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrPrint_xy(uint col, uint row, unsigned char mode, const char *str, ...);



/*
*Function:		secscrPrint_Ex
*Description:	在屏幕前景层指定位置格式化显示字符串,通过中英文控制显示
*Input:			col:显示的起始点阵列号，row:显示的起始字符行号；
*				mode:显示模式设置，bit0保留，bit1-bit2:对齐方式，bit2bit1:0b00-左对齐，0b01-居中，0b10-右对齐
*				bit3-bit6:保留，bit7:控制反显 1-反显，0-正显
*				*chstr:显示输入的中文字符串及格式
*				*enstr:显示输入的英文字符串
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrPrint_Ex(uint col, unsigned char row, unsigned char mode, const char *chstr, const char *enstr);


/*
*Function:		secscrTextOut_lib
*Description:	在指定的屏幕位置显示字符串
*Input:			pixel_X:横坐标; pixel_Y:纵坐标; *str:需要显示的字符串
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrTextOut_lib(uint pixel_X, uint pixel_Y, unsigned char *str);


/*
*Function:		secscrDrawDot_lib
*Description:	LCD 画点
*Input:			px-x坐标；py-y坐标；Mode-显示模式，0-表示在反显模式下画点，1-在正显模式下画点
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int secscrDrawDot_lib(unsigned short int x, unsigned short int y, unsigned char Mode);


/*
*Function:		secscrGetPixel_lib
*Description:	获取特定坐标点的颜色值
*Input:			x:x坐标0-319；y:y坐标0-239；*picolor:颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int secscrGetPixel_lib(unsigned short int x, unsigned short int y, unsigned int *picolor);


/*
*Function:		secscrDrawStraightLine_lib
*Description:	LCD 画线
*Input:			sx-划线起始点x坐标；sy-划线起始点y坐标；ex-划线结束点x坐标；ey-划线结束点y坐标；Mode-划线颜色，0-无色，1-前景色
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int secscrDrawStraightLine_lib(unsigned short int sx, unsigned short int sy, unsigned short int ex, unsigned short int ey, unsigned char Mode);


/*
*Function:		secscrDrawRect_lib
*Description:	画矩形框
*Input:			left-左；top-顶；right-右；bottom-底；Mode--划线颜色，0-无色，1-前景色
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrDrawRect_lib(int left, int top, int right, int bottom, unsigned char Mode);


/*
*Function:		secscrDrawRectColor_lib
*Description:	画矩形框,并以传参颜色值显示
*Input:			left-左；top-顶；right-右；bottom-底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrDrawRectColor_lib(int left, int top, int right, int bottom, unsigned int color);


/*
*Function:		secscrDrawRectBlock_lib
*Description:	用指定颜色在背景层绘制实心矩形色块
*Input:			left:左；top:上；right:右；bottom:底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int secscrDrawRectBlock_lib(int left,int top,int right,int bottom, unsigned int color);


/*
*Function:		secscrWriteLogo_lib
*Description:	显示开机LOGO，若外部FLASH中无LOGO文件,则显示默认LOGO，根据data[0]来识别高位在前还是地位在前识别
*Input:			left:图片左上角x坐标，top:图片左上角y坐标，*data:logo图片数据
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int secscrWriteLogo_lib(int left, int top, unsigned char *data);


/*
*Function:		secscrGetLcdSize_lib
*Description:	读取LCD显示区域大小
*Input:			NULL
*Output:		*width:LCD显示宽度；*height:LCD显示高度
*Hardware:
*Return:		NULL
*Others:
*/
void secscrGetLcdSize_lib(int *width, int *height);


/*
*Function:		secscrRefresh_lib
*Description:	根据需要刷新整个屏幕
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrRefresh_lib(void);

/*
*Function:		secscrOpen_lib
*Description:	打开副屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrOpen_lib(void);

/*
*Function:		secscrClose_lib
*Description:	关闭副屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void secscrClose_lib(void);


#endif


