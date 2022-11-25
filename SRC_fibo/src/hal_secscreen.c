/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_secscreen.c
** Description:		副屏相关接口
**
** Version:	1.0, 渠忠磊,2022-07-06
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"



volatile int32 g_i32SegLcdBacklightTimeout = 0;

GUI_POINT  g_stSegLcdCurPixel = {0,0}; /* 记录字符串输出时的当前位置。*/


/*
*Function:		hal_secscrCls
*Description:	LCD 清屏，不包括图标
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
int hal_secscrCls(void)
{
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);
	
	hal_segClsUserArea();

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
}


/*
*Function:		hal_secscrClrLine
*Description:	清除指定的一行或者若干行，
*Input:			startline-起始字符行号，endline-结束字符行号
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_secscrClrLine(unsigned char startline, unsigned char endline)
{

	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	if(startline==0 || endline==0 || startline>SEGLCD_LINEMAX || endline>SEGLCD_LINEMAX || startline>endline)
	{
		hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
		return LCD_ERR_COORDINATE_INVALID;
	}
	iRet = hal_segClrLine(startline, endline);
	
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
	return iRet;
}

/*
*Function:		hal_secscrBackLightHandle
*Description:	背光点亮倒计时接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrBackLightHandle(void)
{
	
	if(g_i32SegLcdBacklightTimeout > 0)
	{
		g_i32SegLcdBacklightTimeout -= 100;
		if(g_i32SegLcdBacklightTimeout <= 0)
		{
			g_i32SegLcdBacklightTimeout = 0;
			
			g_stSegLcdGUI.BackLightEN = 0;
			
			hal_segBackLightCtl(0);//计时到了，屏幕关背光
		}
	}
}


/*
*Function:		hal_secscrBackLightWakeup
*Description:	按键等事件唤醒屏幕接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrBackLightWakeup(void)
{

		if(g_stSegLcdGUI.BackLightMode == 1)
		{
			g_i32SegLcdBacklightTimeout = g_stSegLcdGUI.BackLightTimeout;
			g_stSegLcdGUI.BackLightEN = 1;
			hal_segSetBackLightValue(g_stSegLcdGUI.BackLight);
		}
	
}


/*
*Function:		hal_secscrGetBackLightTime
*Description:	读取背光超时时间，单位10ms
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		读到的返回值
*Others:
*/
int hal_secscrGetBackLightTime(void)
{
	return g_i32SegLcdBacklightTimeout/10;
}


/*
*Function:		hal_secscrSetBackLightMode
*Description:	设置屏幕背光模式
*Input:			mode:0-背光关闭: 1-背光保持,time*10ms后关闭,2-背光常亮; time:背光时间，单位10ms,仅对mode=1时有效
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrSetBackLightMode(ushort mode, ushort time)
{
	sysLOG(SECSCR_LOG_LEVEL_1, "mode=%d, time=%d\r\n", mode, time);
	switch(mode)
	{
		case 0:
			g_i32SegLcdBacklightTimeout = 0;
			
			g_stSegLcdGUI.BackLightEN = 0;
			g_stSegLcdGUI.BackLightMode = mode;
			
			hal_segBackLightCtl(0);
		break;
		case 1:
			g_i32SegLcdBacklightTimeout = time*10;
			
			g_stSegLcdGUI.BackLightEN = 1;
			g_stSegLcdGUI.BackLightMode = mode;
			g_stSegLcdGUI.BackLightTimeout = g_i32SegLcdBacklightTimeout;
			hal_segSetBackLightValue(g_stSegLcdGUI.BackLight);
			
		break;
		case 2:
			g_i32SegLcdBacklightTimeout = 0;
			
			g_stSegLcdGUI.BackLightEN = 1;
			g_stSegLcdGUI.BackLightMode = mode;
			hal_segSetBackLightValue(g_stSegLcdGUI.BackLight);
			
		break;
	
	}


}


/*
*Function:		hal_secscrSetBackLightValue
*Description:	设置屏幕亮度大小
*Input:			ucValue:亮度值，0~100；
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrSetBackLightValue(unsigned char ucValue)
{
	int iRet = -1;
	float valuetmp;
	
	g_stSegLcdGUI.BackLight = ucValue;
	valuetmp = ucValue*2.55;
	if(g_stSegLcdGUI.BackLightEN == 1)
	{
		iRet = hal_segBackLightCtl((uint8) valuetmp);
	}
	
	sysLOG(SECSCR_LOG_LEVEL_3, "iRet:%d,valuetmp:%d,%f,ucValue=%d\r\n", iRet, (uint8) valuetmp, valuetmp, ucValue);
}


/*
*Function:		hal_secscrSetAttrib
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
int hal_secscrSetAttrib(LCD_ATTRIBUTE_t attr, int value)
{

	sysLOG(SECSCR_LOG_LEVEL_3, "attr=%d, value=%d\r\n", attr, value);

	return hal_segSetAttrib(attr, value);

}


/*
*Function:		hal_secscrGetAttrib
*Description:	获取液晶显示器的功能属性
*Input:			attr:LCD_ATTRIBUTE_t
*Output:		NULL
*Hardware:
*Return:		<0-失败，>=0-成功，具体值为所读取的功能属性value
*Others:
*/
int hal_secscrGetAttrib(LCD_ATTRIBUTE_t attr)
{
	
	return hal_segGetAttrib(attr);

}


/*
*Function:		hal_secscrGotoxyPix
*Description:	定位LCD显示光标，参数超出LCD范围时不改变原坐标位置
*Input:			pixel_X:横坐标；pixel_Y:纵坐标
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y)
{
	
	hal_segGotoxyPix(pixel_X, pixel_Y);
	
}


/*
*Function:		hal_secscrGetxyPix
*Description:	读取LCD上光标的当前位置。
*Input:			NULL
*Output:		*pixel_X:横坐标; *pixel_Y:纵坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrGetxyPix(int *pixel_X, int *pixel_Y)
{
	
	hal_segGetxyPix(pixel_X, pixel_Y);
	
}


/*
*Function:		hal_secscrPrintf
*Description:	在屏幕的前景层当前位置格式化显示字符串
*Input:			*fmt:显示输出的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrPrintf(const char *fmt, ...)
{
	va_list vp;
	int8 *rP = NULL;	

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	va_start(vp, fmt);
	vsprintf((int8 *)rP, fmt, vp);
	va_end(vp);
	
	hal_segPrintf("%s", rP);

	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
}


/*
*Function:		hal_secscrPrint
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
void hal_secscrPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...)
{
	va_list vp;
	int8 *rP = NULL;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	va_start(vp, str);
	vsprintf((int8 *)rP, str, vp);
	va_end(vp);
	
	hal_segPrint(col, row, mode, "%s", rP);

	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);


}


/*
*Function:		hal_secscrPrintxy
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
void hal_secscrPrintxy(uint col, uint row, unsigned char mode, const char *str, ...)
{
	va_list vp;
	int8 *rP = NULL;	

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	va_start(vp, str);
	vsprintf((int8 *)rP, str, vp);
	va_end(vp);

	hal_segPrintxy(col, row, mode, "%s", rP);
	
	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);


}


/*
*Function:		hal_secscrPrintEx
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
void hal_secscrPrintEx(uint col, unsigned char row, unsigned char mode, const char *chstr, const char *enstr)
{
	int8 *rP = NULL;	

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	int iLang = g_ui8ScrLanguage;
	if(1 == iLang)
	{
		strcpy(rP, chstr);
	}
	else
	{
		strcpy(rP, enstr);
	}

	hal_segPrint(col, row, mode, "%s", rP);

	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

}


/*
*Function:		hal_secscrTextOut
*Description:	在指定的屏幕位置显示字符串
*Input:			pixel_X:横坐标; pixel_Y:纵坐标; *str:需要显示的字符串
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrTextOut(uint pixel_X, uint pixel_Y, unsigned char *str)
{
	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	hal_segTextOut(pixel_X, pixel_Y, str, &g_stSegLcdCurPixel.x, &g_stSegLcdCurPixel.y, 0);

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
}


/*
*Function:		hal_secscrDrawDot
*Description:	LCD 画点
*Input:			px-x坐标；py-y坐标；Mode-显示模式，0-表示在反显模式下画点，1-在正显模式下画点
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_secscrDrawDot(unsigned short int x, unsigned short int y, unsigned char Mode)
{
	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);
	
	iRet = hal_segDrawDot(x, y, Mode);
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
		hal_segRefresh();
	}
	
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
	return iRet;

}


/*
*Function:		hal_secscrGetPixel
*Description:	获取特定坐标点的颜色值
*Input:			x:x坐标0-319；y:y坐标0-239；*picolor:颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_secscrGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor)
{
	int iRet = -1;

	iRet = hal_segGetPixel(x, y, picolor);

	return iRet;
}


/*
*Function:		hal_secscrDrawStraightLine
*Description:	LCD 画线
*Input:			sx-划线起始点x坐标；sy-划线起始点y坐标；ex-划线结束点x坐标；ey-划线结束点y坐标；Mode-划线颜色，0-无色，1-前景色
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_secscrDrawStraightLine(unsigned short int sx, unsigned short int sy, unsigned short int ex, unsigned short int ey, unsigned char Mode)
{
	int iRet = -1;
	if(sx<SEGLCD_X_USERPIXELMIN || sy<SEGLCD_Y_USERPIXELMIN || ex<SEGLCD_X_USERPIXELMIN || ey<SEGLCD_Y_USERPIXELMIN || sx>SEGLCD_X_USERPIXELMAX || sy>SEGLCD_Y_USERPIXELMAX || ex>SEGLCD_X_USERPIXELMAX || ey>SEGLCD_Y_USERPIXELMAX)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);
	
	hal_segDrawStraightLine(sx, sy, ex, ey, Mode);

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

	return 0;
}


/*
*Function:		hal_secscrDrawRect
*Description:	画矩形框
*Input:			left-左；top-顶；right-右；bottom-底；Mode--划线颜色，0-无色，1-前景色
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrDrawRect(int left, int top, int right, int bottom, unsigned char Mode)
{
	int iRet;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	hal_segDrawRectangle(left, top, right, bottom, Mode);

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

}


/*
*Function:		hal_secscrDrawRectColor
*Description:	画矩形框,并以传参颜色值显示
*Input:			left-左；top-顶；right-右；bottom-底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrDrawRectColor(int left, int top, int right, int bottom, unsigned int color)
{
	int iRet;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	hal_segDrawRectangle(left, top, right, bottom, g_stSegLcdGUI.grapFrontColor);

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

}


/*
*Function:		hal_secscrDrawRectBlock
*Description:	用指定颜色在背景层绘制实心矩形色块
*Input:			left:左；top:上；right:右；bottom:底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_secscrDrawRectBlock(int left,int top,int right,int bottom, unsigned int color)
{

	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	iRet = hal_segDrawRectBlock(left, top, right, bottom, (unsigned short)color);
		
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

	return iRet;
	
}


/*
*Function:		hal_secscrWriteLogo
*Description:	显示开机LOGO，若外部FLASH中无LOGO文件,则显示默认LOGO，根据data[0]来识别高位在前还是地位在前识别
*Input:			left:图片左上角x坐标，top:图片左上角y坐标，*data:logo图片数据
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_secscrWriteLogo(int left, int top, unsigned char *data)
{
	int iRet = -1;

	if(data == NULL)
	{
		return LCD_ERR_PIC_DATA_EMPTY;
	}

	sysLOG(SECSCR_LOG_LEVEL_4, "left:%d,top:%d\r\n", left, top);

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);
	
	if((left >= SEGLCD_PIXWIDTH) || (top >= SEGLCD_PIXHIGH))
	{
		hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
		return LCD_ERR_COORDINATE_INVALID;
	}
	hal_segWriteBitMap(left, top, data);
	
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

	return 0;
}

/*
*Function:		hal_secscrGetLcdSize
*Description:	读取LCD显示区域大小
*Input:			NULL
*Output:		*width:LCD显示宽度；*height:LCD显示高度
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrGetLcdSize(int *width, int *height)
{
	if(width == NULL || height == NULL)
	{
		return LCD_ERR_PARAM_ERROR;
	}

	hal_segGetLcdSize(width, height);

}

/*
*Function:		hal_secscrRefresh
*Description:	根据需要刷新整个屏幕
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrRefresh(void)
{
	
	hal_segRefresh();
	
}


/*
*Function:		hal_secscrOpen
*Description:	打开副屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrOpen(void)
{
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);
	hal_segOpen(g_stSegLcdGUI.BackLight);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
}

/*
*Function:		hal_secscrClose
*Description:	关闭副屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_secscrClose(void)
{
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);
	hal_segClose();
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);
}


/*********************************TEST*********************************/

#if MAINTEST_FLAG


static void hal_secondlcdtest(void *param)
{
	int iRet;
	int count = 0;

	while(1)
	{
		hal_secscrCls();
		hal_secscrPrint(0, 0, 0x02, "艾体威尔\r\nWeChat: %02d元", count);
		count ++;
		sysDelayMs(100);
	}
	
}

static void hal_firstlcdtest(void *param)
{
	int iRet;
	int count = 0;

	while(1)
	{

		hal_scrPrint(0, 5, 0x00, "hello,你好,%d", count);
		count ++;
		sysDelayMs(100);
	}
	
}

void secscrtest(void)
{
	
#if 1

	secscrOpen_lib();

	hal_scrSelectFont(&g_stSingleFont8X16, &g_stGBMultiFont16X16);
	
	secscrCls_lib();
	secscrDrawRectBlock_lib(0, 0, 16, 31, 1);
	secscrDrawRectBlock_lib(64, 0, 127, 16, 1);
	sysDelayMs(1000);

	secscrSetBackLightMode_lib(0, 500);
	secscrCls_lib();
	secscrTextOut_lib(0, 0, "艾体123");
	sysDelayMs(1000);

	secscrSetBackLightMode_lib(2, 500);
	secscrCls_lib();
	secscrPrint_lib(0, 0, 0x00, "艾体威尔\r\nWeChat: 99.00元");
	sysDelayMs(1000);

	secscrCls_lib();
	secscrPrint_lib(0, 0, 0x02, "艾体威尔\r\nWeChat: 99.00元");
	sysDelayMs(1000);

	secscrCls_lib();
	secscrPrint_lib(0, 0, 0x84, "艾体威尔\r\nWeChat: 99.00元");
	sysDelayMs(1000);

	secscrCls_lib();
	secscrPrint_lib(0, 0, 0x04, "艾体威尔\r\nWeChat: 99.00元");
	sysDelayMs(1000);

	
	for(uint8 i=0; i<100; i+=10)
	{
		secscrSetBackLightValue_lib(i);
		secscrCls_lib();
		secscrPrint_lib(0, 0, 0x04, "背光(0~100):%d\r\n", i);
		sysDelayMs(200);
	}
	
	secscrSetBackLightValue_lib(100);
	sysDelayMs(1000);

	secscrClose_lib();
	secscrPrint_lib(0, 4, 0x04, "secscrClose_lib");
	sysDelayMs(1000);
	secscrOpen_lib();
	sysDelayMs(1000);

	secscrSetBackLightMode_lib(1, 500);
	scrPrint_lib(0, 4, 0x00, "hello,你好,%d", g_ui8BackLightValue);
	sysDelayMs(2000);
#endif

	secscrWriteLogo_lib(0, 0, gImage_New_Aisino_128x32);
	sysDelayMs(2000);

	fibo_thread_create(hal_firstlcdtest, "hal_firstlcdtest", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	fibo_thread_create(hal_secondlcdtest, "hal_firstlcdtest", 1024*4, NULL, OSI_PRIORITY_NORMAL);


}

#endif



