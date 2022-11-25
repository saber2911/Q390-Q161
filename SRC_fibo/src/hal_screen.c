/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_screen.c
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

#include "comm.h"

uint8 g_ui8ScreenType=0;					//屏幕类型：0 段码屏幕，1 点阵屏幕
volatile uint8 g_ui8ScrLanguage = 0;//0-英文;1-中文
volatile uint8 g_ui8FullScreen = 0;//0-非全屏显示(有图标)；1-全屏显示(没图标)
volatile int32 g_i32BacklightTimeout = 0;

volatile uint8 g_ui8BackLightValue = 0;
volatile uint32 g_ui32LcdDevID = 0;//lcd屏幕ID:HLTLCD_2_4_UC1617S_ID/FYTLCD_2_4_ST7571_ID/FRDLCD_2_8_IST3980CA1_ID/ST7789V2_ID/ST7789V3A_ID


GUI_POINT  g_stCurPixel = {0,0}; /* 记录字符串输出时的当前位置。*/
GUI_POINT  g_stCurPixelIcon = {0,0}; /* 记录字符串输出时的当前位置。*/

ST_FONT g_stSingleFont12X24 = {
	.CharSet = SINGLE_CHAR,
	.Width = 12,
	.Height = 24,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_WEST
};

ST_FONT g_stSingleFont6X12 = {
	.CharSet = SINGLE_CHAR,
	.Width = 6,
	.Height = 12,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_WEST
};

ST_FONT g_stSingleFont8X16 = {
	.CharSet = SINGLE_CHAR,
	.Width = 8,
	.Height = 16,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_WEST
};

ST_FONT g_stUniMultiFont12X12 = {
	.CharSet = MULTI_CHAR,
	.Width = 12,
	.Height = 12,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_UNICODE
};
	
ST_FONT g_stUniMultiFont16X16 = {
	.CharSet = MULTI_CHAR,
	.Width = 16,
	.Height = 16,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_UNICODE
};

ST_FONT g_stUniMultiFont24X24 = {
	.CharSet = MULTI_CHAR,
	.Width = 24,
	.Height = 24,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_UNICODE
};

ST_FONT g_stGBMultiFont12X12 = {
	.CharSet = MULTI_CHAR,
	.Width = 12,
	.Height = 12,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_GB2312
};
	
ST_FONT g_stGBMultiFont16X16 = {
	.CharSet = MULTI_CHAR,
	.Width = 16,
	.Height = 16,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_GB2312
};

ST_FONT g_stGBMultiFont24X24 = {
	.CharSet = MULTI_CHAR,
	.Width = 24,
	.Height = 24,
	.Bold = 0,
	.Italic = 0,
	.EncodeSet = ENCODE_GB2312
};
	


int hal_scrMutexCS(SCREEN_MUTEX locksta, SCREEN_INDEX lcdcs)
{
	int iRet = -1;
	static unsigned char scrCSLastIndex = 0;
	
	if(SCREEN_LOCK == locksta)
	{
		if(scrCSLastIndex != lcdcs)
		{
			switch(lcdcs)
			{
				case SCREEN_FIRST:
					cus_export_api->fibo_genspi_lcd_set_baud(FIRSTSCREEN_SPISPEED_W, FIRSTSCREEN_SPISPEED_R);
				
				break;
				case SCREEN_SECOND:
					cus_export_api->fibo_genspi_lcd_set_baud(SECONDSCREEN_SPISPEED, SECONDSCREEN_SPISPEED);

				break;
				default:
					return iRet;
				break;
			}

			scrCSLastIndex = lcdcs;
			
		}
		
		fibo_mutex_lock(g_ui32MutexLcdHandle);
		
	}
	else if(SCREEN_UNLOCK == locksta)
	{
		
		fibo_mutex_unlock(g_ui32MutexLcdHandle);
	}
	
	iRet = 0;
	
	return iRet;
}


/*
*Function:		hal_scrCls
*Description:	LCD 清屏，不包括图标
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
int hal_scrCls(void)
{
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{

		hal_lcdClsUserArea();

	}
	else

	{
		hal_clcdClsUserArea();
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
}


/*
*Function:		hal_scrClrLine
*Description:	清除指定的一行或者若干行，
*Input:			startline-起始字符行号，endline-结束字符行号
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_scrClrLine(unsigned char startline, unsigned char endline)
{

	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	if(g_ui8LcdType == 0)
	{
		if(startline==0 || endline==0 || startline>g_stLcdConfig.LCD_LINEMAX || endline>g_stLcdConfig.LCD_LINEMAX || startline>endline)
		{
			hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
			return LCD_ERR_COORDINATE_INVALID;
		}
		iRet = hal_lcdClrLine(startline, endline);
	}
	else

	{
		if(startline==0 || endline==0 || startline>9 || endline>9 || startline>endline)
		{
			hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
			return LCD_ERR_COORDINATE_INVALID;
		}
		iRet = hal_clcdClrLine(startline, endline);
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
	return iRet;
}

/*
*Function:		hal_scrBackLightHandle
*Description:	背光点亮倒计时接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrBackLightHandle(void)
{
	
	if(g_i32BacklightTimeout > 0)
	{
		g_i32BacklightTimeout -= 100;
		if(g_i32BacklightTimeout <= 0)
		{
			g_i32BacklightTimeout = 0;
			if(g_ui8LcdType == 0)
			{
				g_stLcdGUI.BackLightEN = 0;
			}
			else

			{
				g_stColorlcdGUI.BackLightEN = 0;
			}
			hal_scrBackLightCtl(5);//计时到了，屏幕变暗
		}
	}
}


/*
*Function:		hal_scrBackLightWakeup
*Description:	按键等事件唤醒屏幕接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrBackLightWakeup(void)
{

	if(g_ui8LcdType == 0)
	{
		if(g_stLcdGUI.BackLightMode == 1)
		{
			g_i32BacklightTimeout = g_stLcdGUI.BackLightTimeout;
			g_stLcdGUI.BackLightEN = 1;
			hal_scrSetBackLightValue(g_stLcdGUI.BackLight);
		}
	}
	else

	{
		if(g_stColorlcdGUI.BackLightMode == 1)
		{
			g_i32BacklightTimeout = g_stColorlcdGUI.BackLightTimeout;
			g_stColorlcdGUI.BackLightEN = 1;
			hal_scrSetBackLightValue(g_stColorlcdGUI.BackLight);
		}
	}
}


/*
*Function:		hal_scrGetBackLightTime
*Description:	读取背光超时时间，单位10ms
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		读到的返回值
*Others:
*/
int hal_scrGetBackLightTime(void)
{
	return g_i32BacklightTimeout/10;
}


/*
*Function:		hal_scrSetBackLightMode
*Description:	设置屏幕背光模式
*Input:			mode:0-背光关闭: 1-背光保持,time*10ms后关闭,2-背光常亮; time:背光时间，单位10ms,仅对mode=1时有效
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrSetBackLightMode(ushort mode, ushort time)
{
	sysLOG(SCR_LOG_LEVEL_1, "mode=%d, time=%d\r\n", mode, time);
	switch(mode)
	{
		case 0:
			g_i32BacklightTimeout = 0;
			if(g_ui8LcdType == 0)
			{
				g_stLcdGUI.BackLightEN = 0;
				g_stLcdGUI.BackLightMode = mode;
			}
			else

			{
				g_stColorlcdGUI.BackLightEN = 0;
				g_stColorlcdGUI.BackLightMode = mode;
			}
			hal_scrBackLightCtl(0);
		break;
		case 1:
			g_i32BacklightTimeout = time*10;
			if(g_ui8LcdType == 0)
			{
				g_stLcdGUI.BackLightEN = 1;
				g_stLcdGUI.BackLightMode = mode;
				g_stLcdGUI.BackLightTimeout = g_i32BacklightTimeout;
				hal_scrSetBackLightValue(g_stLcdGUI.BackLight);
			}
			else

			{
				g_stColorlcdGUI.BackLightEN = 1;
				g_stColorlcdGUI.BackLightMode = mode;
				g_stColorlcdGUI.BackLightTimeout = g_i32BacklightTimeout;
				hal_scrSetBackLightValue(g_stColorlcdGUI.BackLight);
			}
		break;
		case 2:
			g_i32BacklightTimeout = 0;
			if(g_ui8LcdType == 0)
			{
				g_stLcdGUI.BackLightEN = 1;
				g_stLcdGUI.BackLightMode = mode;
				hal_scrSetBackLightValue(g_stLcdGUI.BackLight);
			}
			else

			{
				g_stColorlcdGUI.BackLightEN = 1;
				g_stColorlcdGUI.BackLightMode = mode;
				hal_scrSetBackLightValue(g_stColorlcdGUI.BackLight);
			}
		break;
	
	}


}


/*
*Function:		hal_scrSetBackLightValue
*Description:	设置屏幕亮度大小
*Input:			ucValue:亮度值，0~100；
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrSetBackLightValue(unsigned char ucValue)
{
	int iRet = -1;
	float valuetmp;
	if(g_ui8LcdType == 0)
	{
		g_stLcdGUI.BackLight = ucValue;
		valuetmp = ucValue*2.55;
		if(g_stLcdGUI.BackLightEN == 1)
		{
			iRet = hal_scrBackLightCtl((uint8) valuetmp);
		}
	}
	else

	{
		g_stColorlcdGUI.BackLight = ucValue;
		valuetmp = ucValue*2.55;
		if(g_stColorlcdGUI.BackLightEN == 1)
		{
			iRet = hal_scrBackLightCtl((uint8) valuetmp); 
		}
		
	}
	sysLOG(SCR_LOG_LEVEL_3, "iRet:%d,valuetmp:%d,%f,ucValue=%d\r\n", iRet, (uint8) valuetmp, valuetmp, ucValue);
}


/*
*Function:		hal_scrSetAttrib
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
int hal_scrSetAttrib(LCD_ATTRIBUTE_t attr, int value)
{

	sysLOG(SCR_LOG_LEVEL_3, "attr=%d, value=%d\r\n", attr, value);

	if (g_ui8LcdType == 0)
	{
		return hal_lcdSetAttrib(attr, value);
	}
	else
	{
		return hal_clcdSetAttrib(attr, value);
	}

}


/*
*Function:		hal_scrGetAttrib
*Description:	获取液晶显示器的功能属性
*Input:			attr:LCD_ATTRIBUTE_t
*Output:		NULL
*Hardware:
*Return:		<0-失败，>=0-成功，具体值为所读取的功能属性value
*Others:
*/
int hal_scrGetAttrib(LCD_ATTRIBUTE_t attr)
{
	if(g_ui8LcdType == 0)
	{
		return hal_lcdGetAttrib(attr);
	}
	else
	{
		return hal_clcdGetAttrib(attr);
	}

}


/*
*Function:		hal_scrGotoxyPix
*Description:	定位LCD显示光标，参数超出LCD范围时不改变原坐标位置
*Input:			pixel_X:横坐标；pixel_Y:纵坐标
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y)
{
	if(g_ui8LcdType == 0)
	{
		hal_lcdGotoxyPix(pixel_X, pixel_Y);
	}
	else

	{
		hal_clcdGotoxyPix(pixel_X, pixel_Y);
	}
}


/*
*Function:		hal_scrGetxyPix
*Description:	读取LCD上光标的当前位置。
*Input:			NULL
*Output:		*pixel_X:横坐标; *pixel_Y:纵坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrGetxyPix(int *pixel_X, int *pixel_Y)
{
	if(g_ui8LcdType == 0)
	{
		hal_lcdGetxyPix(pixel_X, pixel_Y);
	}
	else

	{
		hal_clcdGetxyPix(pixel_X, pixel_Y);
	}
}


/*
*Function:		hal_scrPrintf
*Description:	在屏幕的前景层当前位置格式化显示字符串
*Input:			*fmt:显示输出的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrPrintf(const char *fmt, ...)
{
	va_list vp;
	int8 *rP = NULL;	

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	va_start(vp, fmt);
	vsprintf((int8 *)rP, fmt, vp);
	va_end(vp);


	if(g_ui8LcdType == 0)
	{
		hal_lcdPrintf("%s", rP);
	}
	else

	{
		hal_clcdPrintf("%s", rP);
	}

	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
}


/*
*Function:		hal_scrPrint
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
void hal_scrPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...)
{
	va_list vp;
	int8 *rP = NULL;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	va_start(vp, str);
	vsprintf((int8 *)rP, str, vp);
	va_end(vp);


	if(g_ui8LcdType == 0)
	{
		hal_lcdPrint(col, row, mode, "%s", rP);
	}
	else

	{
		hal_clcdPrint(col, row, mode, "%s", rP);
	}

	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);


}


/*
*Function:		hal_scrPrintxy
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
void hal_scrPrintxy(uint col, uint row, unsigned char mode, const char *str, ...)
{
	va_list vp;
	int8 *rP = NULL;	

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	rP = malloc(1024);
	if(rP == NULL)
		return;
	
	memset(rP, 0, 1024);
	va_start(vp, str);
	vsprintf((int8 *)rP, str, vp);
	va_end(vp);


	if(g_ui8LcdType == 0)
	{
		hal_lcdPrintxy(col, row, mode, "%s", rP);
	}
	else

	{
		hal_clcdPrintxy(col, row, mode, "%s", rP);
	}
	
	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);


}


/*
*Function:		hal_scrPrintEx
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
void hal_scrPrintEx(uint col, unsigned char row, unsigned char mode, const char *chstr, const char *enstr)
{
	int8 *rP = NULL;	

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

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

	if(g_ui8LcdType == 0)
	{
		hal_lcdPrint(col, row, mode, "%s", rP);
	}
	else
	{
		hal_clcdPrint(col, row, mode, "%s", rP);
	}

	free(rP);
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

}


/*
*Function:		hal_scrTextOut
*Description:	在指定的屏幕位置显示字符串
*Input:			pixel_X:横坐标; pixel_Y:纵坐标; *str:需要显示的字符串
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrTextOut(uint pixel_X, uint pixel_Y, unsigned char *str)
{
	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{
		hal_lcdTextOut(pixel_X, pixel_Y, str, &g_stCurPixel.x, &g_stCurPixel.y, 0);
	}
	else

	{
		hal_clcdTextOut(pixel_X, pixel_Y, str, &g_stCurPixel.x, &g_stCurPixel.y, 0);
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
}


/*
*Function:		hal_scrDrawDot
*Description:	LCD 画点
*Input:			px-x坐标；py-y坐标；Mode-显示模式，0-表示在反显模式下画点，1-在正显模式下画点
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_scrDrawDot(unsigned short int x, unsigned short int y, unsigned char Mode)
{
	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{
		iRet = hal_lcdDrawDot(x, y, Mode);
		if(g_stLcdGUI.AutoRefresh == 1)
		{
			hal_lcdRefresh();
		}
	}
	else

	{
		iRet = hal_clcdDrawDot(x, y, Mode);
		if(g_stColorlcdGUI.AutoRefresh == 1)
		{
			hal_clcdRefresh();
		}
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
	return iRet;

}


/*
*Function:		hal_scrGetPixel
*Description:	获取特定坐标点的颜色值
*Input:			x:x坐标0-319；y:y坐标0-239；*picolor:颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_scrGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor)
{
	int iRet = -1;

	if(g_ui8LcdType == 0)
	{
		iRet = hal_lcdGetPixel(x, y, picolor);
	}
	else

	{
		iRet = hal_clcdGetPixel(x, y, picolor);
	}

	return iRet;
}


/*
*Function:		hal_scrDrawStraightLine
*Description:	LCD 画线
*Input:			sx-划线起始点x坐标；sy-划线起始点y坐标；ex-划线结束点x坐标；ey-划线结束点y坐标；Mode-划线颜色，0-无色，1-前景色
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_scrDrawStraightLine(unsigned short int sx, unsigned short int sy, unsigned short int ex, unsigned short int ey, unsigned char Mode)
{
	int iRet = -1;
	if(sx<g_stLcdConfig.LCD_X_USERPIXELMIN || sy<g_stLcdConfig.LCD_Y_USERPIXELMIN || ex<g_stLcdConfig.LCD_X_USERPIXELMIN || ey<g_stLcdConfig.LCD_Y_USERPIXELMIN || sx>g_stLcdConfig.LCD_X_USERPIXELMAX || sy>g_stLcdConfig.LCD_Y_USERPIXELMAX || ex>g_stLcdConfig.LCD_X_USERPIXELMAX || ey>g_stLcdConfig.LCD_Y_USERPIXELMAX)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{
		hal_lcdDrawStraightLine(sx, sy, ex, ey, Mode);
	}
	else
	{
		hal_clcdDrawStraightLine(sx, sy, ex, ey, Mode);
	}

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

	return 0;
}


/*
*Function:		hal_scrDrawRect
*Description:	画矩形框
*Input:			left-左；top-顶；right-右；bottom-底；Mode--划线颜色，0-无色，1-前景色
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrDrawRect(int left, int top, int right, int bottom, unsigned char Mode)
{
	int iRet;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	if(g_ui8LcdType == 0)
	{
		hal_lcdDrawRectangle(left, top, right, bottom, Mode);
	}
	else

	{
		hal_clcdDrawRectangle(left, top, right, bottom, Mode);
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

}


/*
*Function:		hal_scrDrawRectColor
*Description:	画矩形框,并以传参颜色值显示
*Input:			left-左；top-顶；right-右；bottom-底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrDrawRectColor(int left, int top, int right, int bottom, unsigned int color)
{
	int iRet;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	if(g_ui8LcdType == 0)
	{
		hal_lcdDrawRectangle(left, top, right, bottom, g_stLcdGUI.grapFrontColor);
	}
	else

	{
		hal_clcdDrawRectangleC(left, top, right, bottom, color);
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

}


/*
*Function:		hal_scrDrawRectBlock
*Description:	用指定颜色在背景层绘制实心矩形色块
*Input:			left:左；top:上；right:右；bottom:底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_scrDrawRectBlock(int left,int top,int right,int bottom, unsigned int color)
{


	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	if(g_ui8LcdType == 0)
	{
		iRet = hal_lcdDrawRectBlock(left, top, right, bottom, (unsigned short)color);
	}
	else

	{
		iRet = hal_clcdDrawRectBlock(left, top, right, bottom, (unsigned short)color);
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

	return iRet;
	
}


/*
*Function:		hal_scrWriteLogo
*Description:	显示开机LOGO，若外部FLASH中无LOGO文件,则显示默认LOGO，根据data[0]来识别高位在前还是地位在前识别
*Input:			left:图片左上角x坐标，top:图片左上角y坐标，*data:logo图片数据
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_scrWriteLogo(int left, int top, unsigned char *data)
{
	int iRet = -1;

	if(data == NULL)
	{
		return LCD_ERR_PIC_DATA_EMPTY;
	}

	sysLOG(SCR_LOG_LEVEL_4, "left:%d,top:%d\r\n", left, top);

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{
		if((left >= g_stLcdConfig.LCD_PIXWIDTH) || (top >= g_stLcdConfig.LCD_PIXHIGH))
		{
			hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
			return LCD_ERR_COORDINATE_INVALID;
		}
		hal_lcdWriteBitMap(left, top, data);
	}
	else

	{
		if((left >= g_stLcdConfig.COLORLCD_PIXWIDTH) || (top >= g_stLcdConfig.COLORLCD_PIXHIGH))
		{
			hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
			return LCD_ERR_COORDINATE_INVALID;
		}
		hal_clcdWriteBitMap(left, top, data);
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

	return 0;
}


/*
*Function:		hal_scrLogoVertical2Trans
*Description:	logo文件竖向旋转为横向
*Input:			left:图片左上角x坐标，top:图片左上角y坐标，*data:logo图片数据
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_scrLogoVertical2Trans(unsigned char *indata, unsigned char *outdata, unsigned int *width, unsigned int *height)
{
	int iRet;
	int w = 0,h = 0;
	unsigned char *p,*OutP;
	int i=0, j=0;

	p = indata;
	OutP = outdata;
	
	if((p[0] & 0x10) == 0)
    {
	    w = (p[3] << 8) | p[2]; 
	    h = (p[5] << 8) | p[4]; 
    }
	else

	{
		w = (p[2] << 8) | p[3]; 
	    h = (p[4] << 8) | p[5]; 
	}
	
	*OutP = *indata;
	*(OutP+1) = *(indata+1);
	*(OutP+2) = *(indata+4);
	*(OutP+3) = *(indata+5);
	*(OutP+4) = *(indata+2);
	*(OutP+5) = *(indata+3);

	
	for(i=0; i<w; i++)
	{
		for(j=0; j<h; j++)
		{
			OutP[6+(i*(h/8))+j/8] |= (((*(indata+6+((w-i-1)/8)+j*(w/8)))>>(7-((w-i-1)%8)))&0x01)<<(7-(j%8));

		}
	}

	*width = h;
	*height = w;
	
	sysLOG(SCR_LOG_LEVEL_1, "*width=%d, *height=%d\r\n", *width, *height);
	
	return 0; 
	
}

/*
*Function:		hal_scrGetLcdSize
*Description:	读取LCD显示区域大小
*Input:			NULL
*Output:		*width:LCD显示宽度；*height:LCD显示高度
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrGetLcdSize(int *width, int *height)
{
	if(width == NULL || height == NULL)
	{
		return LCD_ERR_PARAM_ERROR;
	}

	if(g_ui8LcdType == 0)
	{
		hal_lcdGetLcdSize(width, height);
	}
	else

	{
		hal_clcdGetLcdSize(width, height);
	}

}


/*
*Function:		hal_scrRefresh
*Description:	根据需要刷新整个屏幕
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrRefresh(void)
{
	if(g_ui8LcdType == 0)
	{
		hal_lcdRefresh();
	}
	else

	{
		hal_clcdRefresh();
	}
}



/*
*Function:		hal_scrEnumFont
*Description:	枚举当前设备支持的字体
*Input:			MaxFontNums:最多可以放的字体个数
*Output:		*Fonts:存放枚举出的字体
*Hardware:
*Return:		>=0:实际读到的字体个数；other:失败
*Others:
*/
int hal_scrEnumFont(ST_FONT *Fonts,int MaxFontNums)
{
    int iRet = 0;

	if(FONTFS == hal_fontGetFontType())
	{
		iRet = hal_fontEnum(Fonts, MAX_FONT_NUMS);
	}
//	else
//	{
//		iRet = Ft_EnumFont(Fonts, MAX_FONT_NUMS);
//	}

    if(iRet < 0)
    {
        return iRet;
    }
    if(iRet > MaxFontNums)
    {
        return MaxFontNums;
    }
    return iRet;
}


/*
*Function:		hal_scrSelectFont
*Description:	选择LCD显示字体
*Input:			*SingleCodeFont:单内码字体(比如英文等),可以为 NULL
*				*MultiCodeFont:多内码字体(比如中文等),可以为 NULL
*Output:		NULL
*Hardware:
*Return:		0:实际读到的字体个数；<0:失败
*Others:
*/
int hal_scrSelectFont(ST_FONT *SingleCodeFont, ST_FONT *MultiCodeFont)
{
	sysLOG(SCR_LOG_LEVEL_5, "\r\n");
	if(FONTFS == hal_fontGetFontType())
		return hal_fontSelect(SingleCodeFont,MultiCodeFont);
//	else
//		return Ft_SelectFont(SingleCodeFont,MultiCodeFont);
}


/*
*Function:		hal_scrSetLanguage
*Description:	设置系统语言
*Input:			value:0-英文,1-中文
*Output:		NULL
*Hardware:
*Return:		0:成功;<0:失败
*Others:
*/
int hal_scrSetLanguage(unsigned char value)
{
	int iRet = 0;
    int8 lg_data[10] = {0};
	lg_data[0] = value;
	iRet = hal_nvWriteLanguageString(lg_data);
	if(iRet >= 0)
	{
		g_ui8ScrLanguage = value;
	}
    sysLOG(SCR_LOG_LEVEL_4, "iRet = %d, value=%d\r\n", iRet, value);
	return iRet;
}


/*
*Function:		hal_scrGetLanguage
*Description:	获取系统语言
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-英文,1-中文
*Others:
*/
int hal_scrGetLanguage(void)
{
	int iRet = 0;
    int8 lg_data[10] = {0};
	iRet = hal_nvReadLanguageString(lg_data);
	if(iRet >= 0)
	{
		g_ui8ScrLanguage = lg_data[0];
	}
    sysLOG(SCR_LOG_LEVEL_4, "iRet = %d, value=%d\r\n", iRet, lg_data[0]);
	return lg_data[0];
}


/*
*Function:		hal_scrGetPopLen
*Description:	读取缓存数据长度
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		缓存数据长度
*Others:
*/
int hal_scrGetPopLen(void)
{
	int datalen;
	
	if(g_ui8LcdType == 0)
	{
		datalen = 128*(96-12)/4;
	}
	else

	{
		datalen = 320*(240-24)*2;
		
	}
	
	return datalen;

}


/*
*Function:		hal_scrPopDot
*Description:	LCD 导出缓存数据
*Input:			lenth:缓存空间大小
*Output:		*popDot：导出数据指针
*Hardware:
*Return:		缓存数据长度
*Others:
*/
int hal_scrPopDot(unsigned char *popDot,int lenth)
{
	int ret;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{
		ret = hal_lcdPopDot(popDot, lenth);
	}
	else

	{
		ret = hal_clcdPopDot(popDot, lenth);
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
	sysLOG(SCR_LOG_LEVEL_4, "popDot lenth =%d\r\n", ret);

	return ret;
}


/*
*Function:		hal_scrPushDot
*Description:	LCD 导入缓存数据
*Input:			*pushDot：导入数据指针，lenth:数据长度
*Output:		
*Hardware:
*Return:		导入的缓存数据长度
*Others:
*/
int hal_scrPushDot(unsigned char * pushDot,int lenth)
{
	int ret;
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{		
		ret = hal_lcdPushDot(pushDot, lenth);
	}
	else

	{
		ret = hal_clcdPushDot(pushDot, lenth);
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
	sysLOG(SCR_LOG_LEVEL_4, "pushDot lenth =%d\r\n", ret);	

	return ret;
}


/*
*Function:		hal_scrSetTurnDegree
*Description:	设置转向度数
*Input:			uiDegree-转向度数，只支持 0，180，其他值无效，0-正向、180-倒向
*Output:		
*Hardware:
*Return:		0-succ，other-失败
*Others:
*/
int hal_scrSetTurnDegree(unsigned int uiDegree)
{

	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	switch(uiDegree){

		case 0:
			if(g_ui8LcdType == 0)
			{		
				iRet = hal_lcdRotate180(0);
			}
			else

			{
				iRet = hal_clcdRotate180(0);
				
			}
		break;
		case 180:
			if(g_ui8LcdType == 0)
			{		
				iRet = hal_lcdRotate180(1);
			}
			else

			{
				iRet = hal_clcdRotate180(1);
				
			}

		break;
		default:
			iRet = LCD_ERR_PARAM_ERROR;
		break;
	}

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
	return iRet;
}


/*
*Function:		hal_scrFullScreenSet
*Description:	设置全屏模式
*Input:			value:0-退出全屏模式，1-设置全屏模式
*Output:		
*Hardware:
*Return:		0-succ，other-失败
*Others:
*/
int hal_scrFullScreenSet(uint8 value)
{
	int iRet = -1;

	switch(value)
	{
		case 0:
			g_ui8FullScreen = 0;
			
			if(g_ui8LcdType == 0)
			{
				hal_iconLoop();
			}
			else
			{

				hal_ciconLoop();
			}
			
			iRet = 0;
		break;
		case 1:
			g_ui8FullScreen = 1;
			iRet = 0;
		break;
		default:
		break;
	}

	return iRet;
}


/*
*Function:		hal_scrBackLightCtl
*Description:	LCD 背光控制
*Input:			value: 0-255
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int8 hal_scrBackLightCtl(uint8 value)
{
	int8 iRet=-1;

	sysLOG(SCR_LOG_LEVEL_4, "value=%d\r\n", value);

	if(value < 0 || value > 255)
	{
		return iRet;
	}
	
	if(value == 0)
	{
		fibo_pwtStartorStop(FALSE);
	}
	else if(value == 255)
	{
		fibo_pwtConfig(1024, 16, 1024);
		if(g_ui8BackLightValue == 0)
		{
			fibo_pwtStartorStop(TRUE);
		}
	}
	else
	{
		fibo_pwtConfig(1024, 16, value*4);
		if(g_ui8BackLightValue == 0)
		{
			fibo_pwtStartorStop(TRUE);
		}
	}
	
	g_ui8BackLightValue = value;
	
	return 0;
	
}


/*
*Function:		hal_scrGpioInit
*Description:	LCD GPIO 初始化
*Input:			value:亮度值，0~100
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrGpioInit(uint32 value)
{
	
	fibo_pwmOpen();
	if(value == 0)
	{
		fibo_pwtStartorStop(FALSE);
		fibo_pwtConfig(1024, 16, 256);//PWM频率：200Mhz/(prescaler+1)/(period_count*8 + 1)
	}
	else

	{
		fibo_pwtStartorStop(TRUE);
		fibo_pwtConfig(1024, 16, 256);//PWM频率：200Mhz/(prescaler+1)/(period_count*8 + 1)
		hal_scrSetBackLightValue(value);

	}
	
}



/*
*Function:		hal_scrInit
*Description:	初始化屏显，并判断出黑白屏还是彩屏
*Input:			value:亮度值，0~100
*Output:		NULL
*Hardware:
*Return:		0-黑白屏;>0-彩屏，(1-2.8寸彩屏ST7789V2,2-2.4寸彩屏ST7789v3)
*Others:
*/
int hal_scrInit(void)
{
	int iRet = -1;
	int Ret = -1;
	int8 lcdinitcnt = 10;
	uint32 lcd_width, lcd_height;
	
//	if(g_ui8LcdLogoRefresh == 0)
//	{
		do{
			
			iRet = fibo_lcd_init();
			lcdinitcnt --;
			if(iRet == 0)break;
			sysDelayMs(100);
		}while(iRet != 0 && lcdinitcnt>0);
		
//	}
	sysLOG(SCR_LOG_LEVEL_1, "fibo_lcd_init iRet:%d, lcdinitcnt=%d\r\n", iRet, lcdinitcnt);
	Ret = fibo_lcd_Getinfo(&g_ui32LcdDevID, &lcd_width, &lcd_height);
	sysLOG(SCR_LOG_LEVEL_2, "fibo_lcd_Getinfo Ret:%d, g_ui32LcdDevID=%x, lcd_width=%d, lcd_height=%d\r\n", Ret, g_ui32LcdDevID, lcd_width, lcd_height);
	if(g_ui32LcdDevID == FYTLCD_2_4_ST7571_ID)//st7571黑白屏
	{
		hal_lcdContrastSet(g_stLogoinfoJson.lcd_contrast_st7571);
		iRet = 0;//确认是黑白屏，并且初始化成功
	}
	else if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)//uc1617s黑白屏
	{
		hal_lcdContrastSet(g_stLogoinfoJson.lcd_contrast_uc1617s);
		fibo_dotMatrixLcdSetFlushDirection(1);//暂时注掉，后面驱动uc1617s黑白屏需要放开
		//fibo_dotMatrixLcdSetLcmRate(1);
		iRet = 0;
	}
	else if(g_ui32LcdDevID == ST7789V2_ID || g_ui32LcdDevID == FYTCOLORLCD_2_4_ST7789V2_ID ||
		g_ui32LcdDevID == GZGCOLORLCD_2_8_ST7789V2_ID || g_ui32LcdDevID == FRDCOLORLCD_2_4_ST7789V2_ID)//2.4寸彩屏
	{
		g_ui32LcdDevID = ST7789V2_ID;
		if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏		
		{
			iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_90);
		}		
		else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
		{
			iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL);
		}
		sysLOG(SCR_LOG_LEVEL_4, "fibo_lcd_SetBrushDirection iRet:%d\r\n", iRet);
		iRet =  1;//2.8寸彩屏
	}
	else if(g_ui32LcdDevID == ST7789V3A_ID || g_ui32LcdDevID == ZXCOLORLCD_2_4_ST7789V3A_ID)//2.4寸彩屏
	{
		g_ui32LcdDevID = ST7789V3A_ID;
		if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏
		{
			iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_270);
		}	
		else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
		{
			iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL);
		}
		sysLOG(SCR_LOG_LEVEL_4, "fibo_lcd_SetBrushDirection iRet:%d\r\n", iRet);
		iRet =  2;
	}
	else

	{
		g_ui32LcdDevID = FYTLCD_2_4_ST7571_ID;
		iRet =  0;//默认读不到id则为黑白屏
	}

	if(iRet == 0)
	{
		g_stLcdGUI.DispBuff = malloc(g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH);
		memset(g_stLcdGUI.DispBuff,0x0,g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH);
		memset(g_stLcdGUI.RefreshFlag,0,sizeof(g_stLcdGUI.RefreshFlag));
	}
	else

	{
		g_stColorlcdGUI.DispBuff = malloc(g_stLcdConfig.COLORLCD_SUMPIXNUM*2);
		memset(g_stColorlcdGUI.DispBuff,0x0,g_stLcdConfig.COLORLCD_SUMPIXNUM*2);
		memset(g_stColorlcdGUI.RefreshFlag,0,sizeof(g_stColorlcdGUI.RefreshFlag));
	}
	return iRet;

}




/*
*@Brief:		获取ASCII字模
*@Param IN：		reverse:是否反显，0-正显；1-反显；*CurStFont:当前字体结构体; *str:字符；
*@Param OUT:	*width:字符宽度；*height:字符高度；*dot:字符的字模点阵
*@Return:		If reading-operation is right,return the *str 's encode width(1 or 2 is normal),else <0 reading is error,
*/
int Ft_GetDot_ASCII(int reverse, ST_FONT *CurStFont, const unsigned char *str, unsigned char *width, unsigned char *height, unsigned char *dot)
{
	int iRet = -1;
	unsigned int buf_size = 0; 

	*height = CurStFont->Height;
	*width = CurStFont->Width;

	switch (*height) {
		 
		case 12:
			buf_size = 12;
			if (str[0] < 0x20 || str[0] >= 0x80)
			{
				memset(dot, 0, buf_size);
			}
			else
			{
				memcpy(dot, &DefASCFont6X12[(str[0] - 0x20) * 12], buf_size);
			}
			iRet = 1;
		break;
		case 16:
			buf_size = 16;
			if (str[0] < 0x20 || str[0] >= 0x80)
			{
				memset(dot, 0, buf_size);
			}
			else
			{
				memcpy(dot, &DefASCFont8X16[(str[0] - 0x20) * 16], buf_size);
			}
			iRet = 1;
		break;
		case 24:
			buf_size = 2*24;
			if (str[0] < 0x20 || str[0] >= 0x80)
			{
				memset(dot, 0, buf_size);
			}
			else
			{
				memcpy(dot, &DefASCFont12X24[(str[0] - 0x20) * 48], buf_size);
			}
			iRet = 1;
		break;

		default:
		break;
	}
	if (reverse)
	{
		DotBufReverse(dot, buf_size);
	}

	return iRet;
}


/*
*Function:		hal_scrIconClsArea
*Description:	将图标指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_scrIconClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 
	int ret;
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{		
		ret = hal_lcdIconClsArea(left, top, right, bottom);
	}
	else

	{
		ret = hal_clcdIconClsArea(left, top, right, bottom);
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);	

	return ret;

}

/*
*Function:		hal_scrClsArea
*Description:	将指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_scrClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 

	int ret;
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{		
		ret = hal_lcdClsArea(left, top, right, bottom);
	}
	else

	{
		ret = hal_clcdClsArea(left, top, right, bottom);
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

	return ret;

}


/*
*Function:		hal_scrWriteBigBatIcon
*Description:	用来显示关机充电时的电量图标
*Input:			left:x坐标;top:y坐标;num:电量几格
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_scrWriteBigBatIcon(int left, int top, int num)
{
	int iRet = -1;

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{		
		switch(num)
		{
			case 0:
				iRet = hal_lcdWriteBitMap(left, top, gImage_BigBattery80X55_0);
			break;
			case 1:
				iRet = hal_lcdWriteBitMap(left, top, gImage_BigBattery80X55_1);
			break;
			case 2:
				iRet = hal_lcdWriteBitMap(left, top, gImage_BigBattery80X55_2);
			break;
			case 3:
				iRet = hal_lcdWriteBitMap(left, top, gImage_BigBattery80X55_3);
			break;
			case 4:
				iRet = hal_lcdWriteBitMap(left, top, gImage_BigBattery80X55_4);
			break;

			default:

			break;
		}
	}
	else

	{
		switch(num)
		{
			case 0:
				iRet = hal_clcdWriteBitMap(left, top, gImage_BigBattery184X128_0);
			break;
			case 1:
				iRet = hal_clcdWriteBitMap(left, top, gImage_BigBattery184X128_1);
			break;
			case 2:
				iRet = hal_clcdWriteBitMap(left, top, gImage_BigBattery184X128_2);
			break;
			case 3:
				iRet = hal_clcdWriteBitMap(left, top, gImage_BigBattery184X128_3);
			break;
			case 4:
				iRet = hal_clcdWriteBitMap(left, top, gImage_BigBattery184X128_4);
			break;
	
			default:
	
			break;
		}
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);
	

	return iRet;

}

/*
*Function:		hal_scrIconTextOut
*Description:	图标区域固定行间距的字符串输出
*Input:			x:x坐标; y:y坐标; *str:要显示的字符串内容;*CurStFont:当前字库指针
*Output:		*CurpixelX:当前x坐标;*CurpixelY:当前y坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrIconTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, ST_FONT *CurStFont)
{
	int ret;
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{		
		hal_lcdIconTextOut(x, y, str, CurpixelX, CurpixelY);
	}
	else

	{
		hal_clcdIconTextOut(x, y, str, CurpixelX, CurpixelY, CurStFont);
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);


}

/*
*Function:		hal_scrWriteIcon
*Description:	LCD 写图标
*Input:			x-x坐标；y-y坐标；*data-图标内容；width-图标占用的宽度；height-图标占用的高度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_scrWriteIcon(uint16 x, uint16 y, uint8 *data, uint8 width, uint8 height, unsigned short color)
{

	int ret;
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);
	if(g_ui8LcdType == 0)
	{		
		hal_lcdWriteIcon(x, y, data, width, height, color);
	}
	else

	{
		hal_clcdWriteIcon(x, y, data, width, height, color);
		
	}
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

}



