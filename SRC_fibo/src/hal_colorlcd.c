/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_colorlcd.c
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

#include "comm.h"

COLORLCD_GUI	g_stColorlcdGUI;

SPIHANDLE g_stspiFd;

/*3.3V spi lcd init*/
int hal_clcdInitfirst(void)
{

#define LCD_SPI_BAUD_W                      FIRSTSCREEN_SPISPEED_W//max
#define LCD_SPI_BAUD_R                      FIRSTSCREEN_SPISPEED_R
#define LCD_CS_SEL                          SPI_I2C_CS0
#define LCD_DIREC	                        LCD_DIRECT_NORMAL
#define LCD_PIN_DCX                         (3)
#define LCD_PIN_RST                         (120)
#define LCD_SPI_NAME                        DRV_NAME_SPI2
#define LCD_PIN_CS                          (26)
#define LCD_PIN_MOSI                        (24)
#define LCD_PIN_MISO                        (25)
#define LCD_PIN_CLK                         (27)
#define LCD_MODE_CS                         (2)
#define LCD_MODE_MOSI                       (2)
#define LCD_MODE_MISO                       (2)
#define LCD_MODE_CLK                        (2)



	int32_t ret = -1;
	uint32_t id = 0;
	lcd_reg_t lcd_param_tab[] = {
        {0x11,  0, (uint8_t*)""},
        {0xff,  1, (uint8_t*)"\x78"}, // delay 120ms
        {0x36,  1, (uint8_t*)"\x00"},
        {0xb2,  5, (uint8_t*)"\x0c\x0c\x00\x33\x33"},
        {0xb7,  1, (uint8_t*)"\x35"},
        {0xbb,  1, (uint8_t*)"\x37"},
        {0xc0,  1, (uint8_t*)"\x2c"},
        {0xc2,  1, (uint8_t*)"\x01"},
        {0xc3,  1, (uint8_t*)"\x09"},
        {0xc4,  1, (uint8_t*)"\x20"},
        {0xc6,  1, (uint8_t*)"\x03"},
        {0xd0,  2, (uint8_t*)"\xa4\xa1"},
        {0xb0,  2, (uint8_t*)"\x00\xd0"},
        {0xe0, 14, (uint8_t*)"\xd0\x00\x05\x0e\x15\x0d\x37\x43\x47\x09\x15\x12\x16\x19"},
        {0xe1, 14, (uint8_t*)"\xd0\x00\x05\x0d\x0c\x06\x2d\x44\x40\x0e\x1c\x18\x16\x19"},
        {0x35,  1, (uint8_t*)"\x00"},
        {0x3A,  1, (uint8_t*)"\x55"},
        {0x29,  0, (uint8_t*)""},
    };
    genspi_lcd_cfg_t lcd_cfg = {
        .spi_name = LCD_SPI_NAME,
        .spi_baud_w = LCD_SPI_BAUD_W,
        .spi_baud_r = LCD_SPI_BAUD_R,
        .cs_sel = LCD_CS_SEL,
        .csx_pin = 0,
        .dcx_pin = LCD_PIN_DCX,
        .rst_pin = LCD_PIN_RST,
        .wire_mode = GENSPI_WIRE_4LINE_II,
    };
    lcdDisplay_t lcdRec = {
        .x = 0,
        .y = 0,
        .width = g_stLcdConfig.COLORLCD_PIXWIDTH,
        .height = g_stLcdConfig.COLORLCD_PIXHIGH,
        };

	fibo_gpio_mode_set(LCD_PIN_CS  , LCD_MODE_CS  );  // cs
    fibo_gpio_mode_set(LCD_PIN_MOSI, LCD_MODE_MOSI);  // mosi
    fibo_gpio_mode_set(LCD_PIN_MISO, LCD_MODE_MISO);  // miso
    fibo_gpio_mode_set(LCD_PIN_CLK , LCD_MODE_CLK );  // clk

	g_stColorlcdGUI.DispBuff = malloc(g_stLcdConfig.COLORLCD_SUMPIXNUM*2);
	g_stColorlcdGUI.RefreshFlag = malloc(g_stLcdConfig.COLORLCD_BLOCKBUFNUM);
	memset(g_stColorlcdGUI.DispBuff,0x0,g_stLcdConfig.COLORLCD_SUMPIXNUM*2);
	memset(g_stColorlcdGUI.RefreshFlag,0,sizeof(g_stColorlcdGUI.RefreshFlag));

	if(hal_fileExist(g_stLogoinfoJson.logofile) != 1)//厂家未初始化LCD屏
	{
		// init
		ret = cus_export_api->fibo_genspi_lcd_init(lcd_cfg, &g_stspiFd);
		sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_genspi_lcd_init=%d", ret);
		if (ret != 0) {
			goto __EXIT;
		}
		
		uint32_t num = OSI_ARRAY_SIZE(lcd_param_tab);
		ret = cus_export_api->fibo_genspi_lcd_send_reg_tab(lcd_param_tab, num);
		sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_genspi_lcd_send_reg_tab=%d", ret);
		if (ret != 0) {
			goto __EXIT;
		}

		for (uint32_t j = 0; j < (g_stLcdConfig.COLORLCD_SUMPIXNUM); j++)
		{
			g_stColorlcdGUI.DispBuff[j] = WHITE;
		}
	    cus_export_api->fibo_genspi_lcd_send_buff(&lcdRec, g_stColorlcdGUI.DispBuff);
	}

	ret = cus_export_api->fibo_genspi_lcd_get_id(&id);
	sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_genspi_lcd_get_id=%d, id=0x%x", ret, id);
	if (ret != 0) {
		goto __EXIT;
	}
	g_ui32LcdDevID = id;

	if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏
	{
		ret = cus_export_api->fibo_genspi_lcd_set_direction(LCD_DIRECT_ROT_270);//json.rotation
	}
	else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
	{
		ret = cus_export_api->fibo_genspi_lcd_set_direction(LCD_DIRECT_NORMAL);
	}
	sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_genspi_lcd_set_direction=%d", ret);
	if (ret != 0) {
		goto __EXIT;
	}
	
	
__EXIT:
	if(ret >= 0)
		ret = 1;
	sysLOG(COLORLCD_LOG_LEVEL_2, "g_ui32LcdDevID=0x%x", g_ui32LcdDevID);
	return ret;
}

/*
*Function:		hal_clcdInit
*Description:	LCD 彩屏初始化参数
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdInit(void)
{
	int iRet = -1;
	uint32 lcd_width, lcd_height;
	
	//memset(ucIconBuf,0x00,sizeof*(ucIconBuf));
	g_stColorlcdGUI.Mode = 0;//正常显示
	g_stColorlcdGUI.AutoRefresh = 1;//自动刷新
	g_stColorlcdGUI.grapBackColor = g_stLogoinfoJson.colorlcd_bg_color;
	g_stColorlcdGUI.grapFrontColor = g_stLogoinfoJson.colorlcd_fg_color;
	g_stColorlcdGUI.BackLight = g_ui8BackLightValue/2.55;
	g_stColorlcdGUI.BackLightEN = 1;
	g_stColorlcdGUI.BackLightMode = 1;
	g_stColorlcdGUI.BackLightTimeout = BACKLIGHTTIMEOUT;
	g_stColorlcdGUI.icon_attr.iconAreaColor = BROWN;
	g_stColorlcdGUI.icon_attr.iconColor = BLACK;
	g_stColorlcdGUI.fonttype = 0;
	g_stCurPixel.x = 0;
	g_stCurPixel.y = 24;
	if(FONTFS == hal_fontGetFontType())
		hal_scrSelectFont(&g_stSingleFont12X24, &g_stGBMultiFont24X24);
//	else
//		hal_scrSelectFont(&g_stSingleFont12X24, &g_stUniMultiFont24X24);


}


/*
*Function:		hal_clcdWrite
*Description:	LCD 彩屏写数据
*Input:			col:x坐标;blockindex:块索引值;*dat:数据指针;datLen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdWrite(unsigned int col, unsigned int blockindex, unsigned short *dat, unsigned int datLen)
{

	lcdDisplay_t startPoint;
	lcdFrameBuffer_t dataBufferWin;

	startPoint.x = col;
	startPoint.y = blockindex*g_stLcdConfig.COLORLCD_BLOCKHIGH;
	startPoint.height = g_stLcdConfig.COLORLCD_BLOCKHIGH;
	startPoint.width = g_stLcdConfig.COLORLCD_BLOCKWIDTH;
	
	dataBufferWin.buffer = dat;
	dataBufferWin.region_x = col;
	dataBufferWin.region_y = blockindex*g_stLcdConfig.COLORLCD_BLOCKHIGH;
	dataBufferWin.width = g_stLcdConfig.COLORLCD_BLOCKWIDTH;
	dataBufferWin.height = g_stLcdConfig.COLORLCD_BLOCKHIGH;
	dataBufferWin.widthOriginal = g_stLcdConfig.COLORLCD_BLOCKWIDTH;
	dataBufferWin.colorFormat = LCD_RESOLUTION_RGB565;
	dataBufferWin.rotation = 0;
	dataBufferWin.keyMaskEnable = FALSE;
	dataBufferWin.maskColor = 0;
	cus_export_api->fibo_genspi_lcd_send_buff(&startPoint, dat);//fibo_lcd_FrameTransfer(&dataBufferWin, &startPoint);


}


/*
*Function:		hal_clcdIconRefresh
*Description:	根据需要刷新图标区域
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdIconRefresh(void)
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;
	
	if(g_iCamera_exist == 1)
	{
		if(g_ui8CameraPreviewEn == 1)
		{
			return;
		}
	}

#if 1
	for (i=0;i<3;i++)
	{
	    //如果有一个块需要刷新
		if(g_stColorlcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			disp = g_stColorlcdGUI.DispBuff+g_stLcdConfig.COLORLCD_BLOCKWIDTH*8*i;

			hal_clcdWrite(0, i, disp, g_stLcdConfig.COLORLCD_BLOCKWIDTH*8);
//			sysLOG(COLORLCD_LOG_LEVEL_2, "i=%d\r\n", i);
		}
	}
#endif
//	hal_clcdWrite(0, 0, g_stColorlcdGUI.DispBuff, 0);

	g_stColorlcdGUI.RefreshFlag[0] &= (~0x07);

}


/*
*Function:		hal_clcdRefresh
*Description:	根据需要刷新整个屏，图标区域除外
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdRefresh(void)
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;

	unsigned char refreshflagtmp = 0;
	uint8 startblocknum = 3;
	
	refreshflagtmp = g_stColorlcdGUI.RefreshFlag[0] & 0x07;

	if (g_iCamera_exist==1)
	{
		if(g_ui8CameraPreviewEn == 1)
		{
			return;
		}
	}

	if(g_ui8FullScreen == 0)
		startblocknum = 3;
	else if(g_ui8FullScreen == 1)
		startblocknum = 0;
#if 1
	for (i=startblocknum;i<g_stLcdConfig.COLORLCD_BLOCKBUFNUM;i++)
	{
	    //如果有一个块需要刷新
		if(g_stColorlcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			disp = g_stColorlcdGUI.DispBuff+g_stLcdConfig.COLORLCD_BLOCKWIDTH*8*i;

			hal_clcdWrite(0, i, disp, g_stLcdConfig.COLORLCD_BLOCKWIDTH*8);
//			sysLOG(COLORLCD_LOG_LEVEL_2, "i=%d\r\n", i);
		}
	}
#endif
//	hal_clcdWrite(0, 0, g_stColorlcdGUI.DispBuff, 0);
	memset(g_stColorlcdGUI.RefreshFlag,0,sizeof(g_stColorlcdGUI.RefreshFlag));
	g_stColorlcdGUI.RefreshFlag[0] = refreshflagtmp;

}


/*
*Function:		hal_clcdRefreshRotate
*Description:	根据需要刷新整个屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdRefreshRotate(void)
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;

	unsigned char refreshflagtmp = 0;
	refreshflagtmp = g_stColorlcdGUI.RefreshFlag[0] & 0x07;

	if (g_iCamera_exist==1)
	{
		if(g_ui8CameraPreviewEn == 1)
		{
			return;
		}
	}

	for (i=0;i<g_stLcdConfig.COLORLCD_BLOCKBUFNUM;i++)
	{
	    //如果有一个块需要刷新
		if(g_stColorlcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			disp = g_stColorlcdGUI.DispBuff+g_stLcdConfig.COLORLCD_BLOCKWIDTH*8*i;

			hal_clcdWrite(0, i, disp, g_stLcdConfig.COLORLCD_BLOCKWIDTH*8);
//			sysLOG(COLORLCD_LOG_LEVEL_2, "i=%d\r\n", i);
		}
	}

	memset(g_stColorlcdGUI.RefreshFlag,0,sizeof(g_stColorlcdGUI.RefreshFlag));
	g_stColorlcdGUI.RefreshFlag[0] = refreshflagtmp;

}

/*
*Function:		hal_clcdDrawSinglePix
*Description:	彩屏画一个点，不包括刷新
*Input:			x:x坐标;y:y坐标;dotColor:像素点渲染值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawSinglePix(unsigned short x,unsigned short y,unsigned short dotColor)
{
    int blocknum = 0;
	
    blocknum = y/g_stLcdConfig.COLORLCD_BLOCKHIGH;
	g_stColorlcdGUI.RefreshFlag[blocknum/8] |= (1<<(blocknum%8));// 1字节记录8个block 看是否更新
	*(g_stColorlcdGUI.DispBuff+y*g_stLcdConfig.COLORLCD_PIXWIDTH+x) = dotColor;
	
}


/*
*Function:		hal_clcdIconDrawDotC
*Description:	图标区域画点并以传参颜色显示,不包括刷新
*Input:			x-x坐标；y-y坐标；color:RGB565颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_clcdIconDrawDotC(unsigned short int x,unsigned short int y,unsigned short color)
{   
	if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH) || (y >= g_stLcdConfig.COLORLCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	
	hal_clcdDrawSinglePix(x,y,color);
    
	return 0;
}


/*
*Function:		hal_clcdIconDrawDot
*Description:	图标区域画点并以前景色显示,不包括刷新
*Input:			x-x坐标；y-y坐标；Mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_clcdIconDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode)
{   
	if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH) || (y >= g_stLcdConfig.COLORLCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
    if(Mode == 1)
    {     
		hal_clcdDrawSinglePix(x,y,g_stColorlcdGUI.icon_attr.iconColor);
    }
    else
    {
        hal_clcdDrawSinglePix(x,y,g_stColorlcdGUI.icon_attr.iconAreaColor);
    }
	return 0;
}


/*
*Function:		hal_clcdGetDotP
*Description:	获取显存中指定位置的地址指针
*Input:			x:坐标x; y:坐标y;
*Output:		NULL
*Hardware:
*Return:		当前位置的指针
*Others:
*/
uint8_t *hal_clcdGetDotP(unsigned short x,unsigned short y)
{
	return (uint8_t *)(g_stColorlcdGUI.DispBuff+y*g_stLcdConfig.COLORLCD_PIXWIDTH+x);
}


/*
*Function:		hal_clcdPopDot
*Description:	将显存中的点阵数据导出
*Input:			lenth：缓存空间大小
*Output:		popDot：缓存地址；
*Hardware:
*Return:		导出的数据字节长度
*Others:
*/
int hal_clcdPopDot(unsigned char *popDot, int lenth)
{
	uint8_t *dotp;
	int maxDataLen;
		
	dotp = hal_clcdGetDotP(0,COLORLCD_ICONHIGH);

	maxDataLen = (g_stLcdConfig.COLORLCD_SUMPIXNUM - g_stLcdConfig.COLORLCD_PIXWIDTH*COLORLCD_ICONHIGH)*2;

	if(lenth > maxDataLen)
		lenth = maxDataLen;
	
	memcpy(popDot, dotp, lenth);

	return lenth;
}

/*
*Function:		hal_clcdPushDot
*Description:	将内存中的ram数据导入显存 并强制刷屏
*Input:			pushDot：缓存地址；lenth：缓存空间大小
*Output:		NULL
*Hardware:
*Return:		导入的数据字节长度
*Others:
*/
int hal_clcdPushDot(unsigned char *pushDot, int lenth)
{
	uint8_t *dotp;
	int maxDataLen;
	
	dotp = hal_clcdGetDotP(0,COLORLCD_ICONHIGH);

	maxDataLen = (g_stLcdConfig.COLORLCD_SUMPIXNUM - g_stLcdConfig.COLORLCD_PIXWIDTH*COLORLCD_ICONHIGH)*2;

	if(lenth > maxDataLen)
		lenth = maxDataLen;
	
	memcpy(dotp, pushDot, lenth);

	int m;
	for(m=0; m<g_stLcdConfig.COLORLCD_BLOCKBUFNUM; m++)
		g_stColorlcdGUI.RefreshFlag[m] = 0xFF;
		
	hal_clcdRefresh();

	return lenth;
}


/*
*Function:		hal_clcdDrawDot
*Description:	画点并以前景色显示,不带刷新
*Input:			x-x坐标；y-y坐标；Mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_clcdDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode)
{   
	if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH) || (y >= g_stLcdConfig.COLORLCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
    if(((g_stColorlcdGUI.Mode == 1) && (Mode == 0)) || ((g_stColorlcdGUI.Mode == 0) && (Mode == 1)))
    {     
		hal_clcdDrawSinglePix(x,y,g_stColorlcdGUI.grapFrontColor);
    }
    else
    {
        hal_clcdDrawSinglePix(x,y,g_stColorlcdGUI.grapBackColor);
    }
	return 0;
}


/*
*Function:		hal_clcdDrawDotC
*Description:	画点并以渲染值显示,不带刷新
*Input:			x-x坐标；y-y坐标；value:RGB565值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_clcdDrawDotC(unsigned short int x,unsigned short int y,unsigned short value)
{   
	if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH) || (y >= g_stLcdConfig.COLORLCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	
	hal_clcdDrawSinglePix(x,y,value);
	
	return 0;
}



/*
*Function:		hal_clcdWriteBitData
*Description:	把指定高度和宽度的点阵数据写到指定的行和列的屏幕缓冲中，不包括刷新
*Input:			x-x坐标;
*				y-y坐标;
*    			*dat-显示点阵数据
*    			width-显示点阵数据宽度(0 - LCD_MAX_X)
*    			height-显示点阵数据高度(0 - LCD_MAX_Y)
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdWriteBitData(int x, int y, unsigned char *dat, int width, int height)
{
    unsigned short ucBitPix;
    int i,j;
    int des_x = (x >= 0) ? x : 0;
    int src_x = (x >= 0) ? 0 : -x;
    int des_y = (y >= 0) ? y : 0;
    int src_y = (y >= 0) ? 0 : -y;
    int w = (x >= 0) ? width  : width + x;
    int h = (y >= 0) ? height : height + y;
    
    if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH) || (x+width <= 0) || (width <= 0))
		return;
	if((y >= g_stLcdConfig.COLORLCD_PIXHIGH) || (y+height<= 0) || (height <= 0))
		return;
    sysLOG(COLORLCD_LOG_LEVEL_1, "x=%d, y=%d, h=%d, w=%d\r\n", x, y, h, w);
	for( i=0; i<h; i++)
	{
		for( j=0; j<w; j++)
		{
            
            ucBitPix = (dat[i*w/8 + j/8] & (1 <<(7-j%8)));
            if(ucBitPix)
            {
                hal_clcdDrawSinglePix(des_x+j, des_y+i, g_stColorlcdGUI.grapFrontColor);
            }
            else
            {
                hal_clcdDrawSinglePix(des_x+j, des_y+i, g_stColorlcdGUI.grapBackColor);
            }
		}
	}

}


/*
*Function:		hal_clcdDrawSingleLine
*Description:	画直线，不包括刷新
*Input:			x1-起始点x坐标;y1-起始点y坐标;x2-结束点x坐标;y2-结束点y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawSingleLine(int x1,int y1,int x2,int y2, unsigned char mode)
{
    unsigned short int i,j;
    
	if(x1<0)	x1=0;
	if(x2<0)	x2=0;
	if(y1<0)	y1=0;
	if(y2<0)	y2=0;
		
    if(x1 > x2 || y1 > y2 || x1 < 0 || y1 < 0 || x2 >= g_stLcdConfig.COLORLCD_PIXWIDTH || y2 >= g_stLcdConfig.COLORLCD_PIXHIGH)
        return;

    if(y1 < y2)
    {
        for (i = y1; i <= y2; i++)//画线输出
        {
            if(x1 < x2)
            {
                for(j = x1; j<= x2; j++)
                {
                    hal_clcdDrawDot(j, i, mode);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_clcdDrawDot(j, i, mode);
                }
            }
        }
    }
    else
    {
        for (i = y2; i <= y1; i++)       //画线输出
        {
            if(x1 < x2)
            {
                for(j = x1; j<= x2; j++)
                {
                    hal_clcdDrawDot(j, i, mode);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_clcdDrawDot(j, i, mode);
                }
            }
        }
    }
	
}

/*
*Function:		hal_clcdDrawStraightLine
*Description:	画直线
*Input:			x1-起始点x坐标;y1-起始点y坐标;x2-结束点x坐标;y2-结束点y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawStraightLine(int x1,int y1,int x2,int y2, unsigned char mode)
{
	hal_clcdDrawSingleLine(x1, y1, x2, y2, mode);
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}

}



/*
*Function:		hal_clcdDrawRectangle
*Description:	画矩形框
*Input:			x1-左上角x坐标;y1-左上角y坐标;x2-右下角x坐标;y2-右下角y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawRectangle(int x1,int y1,int x2,int y2, unsigned char mode)
{
    hal_clcdDrawSingleLine(x1, y1, x2, y1, mode);
    hal_clcdDrawSingleLine(x1, y1, x1, y2, mode);
    hal_clcdDrawSingleLine(x1, y2, x2, y2, mode);
    hal_clcdDrawSingleLine(x2, y1, x2, y2, mode);
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}
}


/*
*Function:		hal_clcdDrawLineC
*Description:	画直线,不包括刷新
*Input:			x1-起始点x坐标;y1-起始点y1坐标;x2-结束点x坐标;y2-结束点y坐标;
*				color-渲染值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawLineC(int x1,int y1,int x2,int y2, unsigned short color)
{
    unsigned short int i,j;
    
	if(x1<0)	x1=0;
	if(x2<0)	x2=0;
	if(y1<0)	y1=0;
	if(y2<0)	y2=0;
		
    if(x1 > x2 || y1 > y2 || x1 < 0 || y1 < 0 || x2 >= g_stLcdConfig.COLORLCD_PIXWIDTH || y2 >= g_stLcdConfig.COLORLCD_PIXHIGH)
        return;

    if(y1 < y2)
    {
        for (i = y1; i <= y2; i++)//画线输出
        {
            if(x1 < x2)
            {
                for(j = x1; j<= x2; j++)
                {
                    hal_clcdDrawSinglePix(j, i, color);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_clcdDrawSinglePix(j, i, color);
                }
            }
        }
    }
    else
    {
        for (i = y2; i <= y1; i++)       //画线输出
        {
            if(x1 < x2)
            {
                for(j = x1; j<= x2; j++)
                {
                    hal_clcdDrawSinglePix(j, i, color);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_clcdDrawSinglePix(j, i, color);
                }
            }
        }
    }
	
}


/*
*Function:		hal_clcdDrawRectangleC
*Description:	画矩形框
*Input:			x1-左上角x坐标;y1-左上角y坐标;x2-右下角x坐标;y2-右下角y坐标;
*				color-渲染值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawRectangleC(int x1,int y1,int x2,int y2, unsigned short color)
{
    hal_clcdDrawLineC(x1, y1, x2, y1, color);
    hal_clcdDrawLineC(x1, y1, x1, y2, color);
    hal_clcdDrawLineC(x1, y2, x2, y2, color);
    hal_clcdDrawLineC(x2, y1, x2, y2, color);
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}
}


/*
*Function:		hal_clcdDrawRectBlock
*Description:	用指定颜色在背景层绘制实心矩形色块
*Input:			left:左；top:上；right:右；bottom:底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_clcdDrawRectBlock(int left,int top,int right,int bottom, unsigned short color)
{
	int i,j;
	
	if (left > right || top > bottom || right >= g_stLcdConfig.COLORLCD_PIXWIDTH || bottom >= g_stLcdConfig.COLORLCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
    if(left<0||top<0||right<0||bottom<0)
    {
		return LCD_ERR_AREA_INVALID;
	}
	if((color < 0) || (color > 0xFFFF))
	{
		return LCD_ERR_NO_COLOR_INDEX;
	}
	
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_clcdDrawSinglePix(i, j, color);
		}
	}
	
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
	    hal_clcdRefresh();
	}
	return 0;
}


/*
*Function:		hal_clcdDrawCircle
*Description:	空心圆绘制
*Input:			x0-圆心x坐标;y0-圆心y坐标;r-半径
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawCircle(unsigned short int x0, unsigned short int y0, unsigned short int r)
{
    int a, b;
    int di;
    a = 0; b = r;
    di = 3 - (r << 1);//判断下个点位置的标志
    while (a <= b)
    {
        hal_clcdDrawDot(x0 + a, y0 - b,1);
        hal_clcdDrawDot(x0 + b, y0 - a,1);
        hal_clcdDrawDot(x0 + b, y0 + a,1);
        hal_clcdDrawDot(x0 + a, y0 + b,1);
        hal_clcdDrawDot(x0 - a, y0 + b,1);
        hal_clcdDrawDot(x0 - b, y0 + a,1);
        hal_clcdDrawDot(x0 - a, y0 - b,1);
        hal_clcdDrawDot(x0 - b, y0 - a,1);
        a++;
        if(di < 0) di += 4 * a + 6;//使用Bresenham算法画圆
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}
}


/*
*Function:		hal_clcdIconClsArea
*Description:	将图标指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_clcdIconClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 
	
	int i,j;
    
 	if (left > right || top > bottom || right >= g_stLcdConfig.COLORLCD_PIXWIDTH || bottom >= g_stLcdConfig.COLORLCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
		
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_clcdDrawSinglePix(i,j,g_stColorlcdGUI.icon_attr.iconAreaColor);
		}
	}
	
	hal_clcdIconRefresh();
	
    return 0;

}


/*
*Function:		hal_clcdClsArea
*Description:	将指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_clcdClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 
	
	int i,j;
    
 	if (left > right || top > bottom || right >= g_stLcdConfig.COLORLCD_PIXWIDTH || bottom >= g_stLcdConfig.COLORLCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
	
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_clcdDrawSinglePix(i,j,g_stColorlcdGUI.grapBackColor);
		}
	}
	
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
		hal_clcdIconRefresh();
	}
		
    return 0;

}


/*
*Function:		hal_clcdClsUserArea
*Description:	清除整个屏幕不包括图标区域，执行该指令后，将光标移动到左上角
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_clcdClsUserArea(void)
{
    int iRet = 0;
    unsigned int startrow = COLORLCD_TEXTPIXEL;

	if(g_ui8FullScreen == 0)
		startrow = COLORLCD_TEXTPIXEL;
	else if(g_ui8FullScreen == 1)
		startrow = 0;
    iRet = hal_clcdClsArea(0, startrow,g_stLcdConfig.COLORLCD_PIXWIDTH-1,g_stLcdConfig.COLORLCD_PIXHIGH-1);

	g_stCurPixel.x = 0;
	if(g_ui8FullScreen == 0)
		g_stCurPixel.y = COLORLCD_TEXTPIXEL;
	else if(g_ui8FullScreen == 1)
		g_stCurPixel.y = 0;
	
    return iRet;
}


/*
*Function:		hal_clcdClrLine
*Description:	清除前景层指定的一行或若干行，参数不合理无动作，
*				执行该指令后光标停留在（0，startline）行号从1到9共9行，默认行高为24个bit
*Input:			startline:起始字符行号；endline:结束字符行号
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_clcdClrLine(unsigned char startline, unsigned char endline)
{
	int iRet = -1;

	iRet = hal_clcdDrawRectBlock(0, startline*24, g_stLcdConfig.COLORLCD_PIXWIDTH-1, (endline+1)*24-1, g_stColorlcdGUI.grapBackColor);
	g_stCurPixel.x = 0;
	g_stCurPixel.y = startline*COLORLCD_TEXTPIXEL;

	return iRet;
}


/*
*Function:		hal_clcdWriteBitMap
*Description:	根据data[0]来识别高位在前还是地位在前识别,把指定高度和宽度的点阵数据写到屏幕上
*Input:			left-x坐标;
*				top-y坐标;
*    			*data-显示点阵数据
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败
*Others:
*/
int hal_clcdWriteBitMap(int left, int top, unsigned char *data)
{
    int w = 0,h = 0;
    unsigned char *p = data;
    if(NULL == data)
        return LCD_ERR_PIC_DATA_EMPTY;
	
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
    if(w > g_stLcdConfig.COLORLCD_PIXWIDTH || h > g_stLcdConfig.COLORLCD_PIXHIGH)
        return LCD_ERR_PIC_DATA_OVERFLOW;
  
    if(w == 0 || h == 0)
        return LCD_ERR_PIC_DATA_EMPTY;
    if(w%8 != 0)
        return LCD_ERR_PARAM_ERROR;
    
	hal_clcdWriteBitData(left, top, p + 6, w, h);
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}
    return LCD_ERR_SUCCESS;
}

	
/*
*Function:		hal_clcdWriteBMP
*Description:	显示BMP格式的图片
*Input:			left:横坐标;top:纵坐标;width:图片宽度;height:图片高度;*data:图片数据;datalen;数据长度
*Output:		NULL
*Hardware:
*Return:		0-success; <0:failed
*Others:
*/
int hal_clcdWriteBMP(int left, int top, int width, int height, unsigned short *data, int datalen)
{
	int i, j;
    int w = 0,h = 0;
    unsigned short *p = data;
    if(NULL == data)
        return LCD_ERR_PIC_DATA_EMPTY;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++){

			hal_clcdDrawSinglePix(left+j, top+height-1-i,*(data+j+i*width));
		}
	}
	
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}
    return LCD_ERR_SUCCESS;
}


/*
*Function:		hal_clcdIconDot2DispBuf
*Description:	图标中字模点阵写到显示缓存中
*Input:			x:x坐标; y:y坐标; width:字模宽度; height:字模高度; *dot:字模点阵指针; encode_witdh:1-单内码; 2-多内码
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_clcdIconDot2DispBuf(unsigned int x,unsigned int y,int width,int height,char *dot,int encode_width)
{

    unsigned char ucBitPix;
    int k;
    int i,j;
    int value_width;
    int value_height;
    int offset;
        
	if(x+width>g_stLcdConfig.COLORLCD_PIXWIDTH || y+height>g_stLcdConfig.COLORLCD_PIXHIGH)
		return ;
    
	if(width == 6 || width == 8 || width == 16)
	{
		if(width <8)//针对6x8字体
			value_width = width;
		else
			value_width = 8;

        value_height = height;
        i = 0;
		while(i<value_height)
		{ 
		    offset = 0;
    		for(j=0;j<width;j+=8)
    		{
    			for(k=0;k<value_width;k++)
    			{				
					ucBitPix =(*dot & (0x80>>k)? 1:0);
					hal_clcdIconDrawDot(x+offset+k,y+i,ucBitPix);           

    			}
                offset += 8*1;
                dot++;
    		  }
            i++;
		}      
	 }
    else if(((width == 12)&&(encode_width == 1))||((width == 24)&&(encode_width == 2))||((width == 12)&&(encode_width == 2)))
    {
        value_width = width;
        value_height = height;
        i = 0;
		while(i<value_height)
		{ 
		    offset = 0;
    		for(j=0;j<width;j+=8)
    		{
    			for(k=0;k<8;k++)
    			{				
					ucBitPix =(*dot & (0x80>>k)? 1:0);
					hal_clcdIconDrawDot(x+offset+k,y+i,ucBitPix);           

    			}
                offset += (8);
                dot++;
    		  }
            i++;
		} 
    }
}


/*
*Function:		hal_clcdDot2DispBuf
*Description:	字模点阵写到显示缓存中
*Input:			x:x坐标; y:y坐标; width:字模宽度; height:字模高度; *dot:字模点阵指针; encode_witdh:1-单内码; 2-多内码
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDot2DispBuf(unsigned int x,unsigned int y,int width,int height,char *dot,int encode_width)
{

    unsigned char ucBitPix;
    int k;
    int i,j;
    int value_width;
    int value_height;
    int offset;
        
	if(x+width>g_stLcdConfig.COLORLCD_PIXWIDTH || y+height>g_stLcdConfig.COLORLCD_PIXHIGH)
		return ;
    
	if(width == 6 || width == 8 || width == 16)
	{
		if(width <8)//针对6x8字体
			value_width = width;
		else
			value_width = 8;

        value_height = height;
        i = 0;
		while(i<value_height)
		{ 
		    offset = 0;
    		for(j=0;j<width;j+=8)
    		{
    			for(k=0;k<value_width;k++)
    			{				
					ucBitPix =(*dot & (0x80>>k)? 1:0);
					hal_clcdDrawDot(x+offset+k,y+i,ucBitPix);           

    			}
                offset += 8*1;
                dot++;
    		  }
            i++;
		}      
	 }
    else if(((width == 12)&&(encode_width == 1))||((width == 24)&&(encode_width == 2))||((width == 12)&&(encode_width == 2)))
    {
        value_width = width;
        value_height = height;
        i = 0;
		while(i<value_height)
		{ 
		    offset = 0;
    		for(j=0;j<width;j+=8)
    		{
    			for(k=0;k<8;k++)
    			{				
					ucBitPix =(*dot & (0x80>>k)? 1:0);
					hal_clcdDrawDot(x+offset+k,y+i,ucBitPix);           

    			}
                offset += (8);
                dot++;
    		  }
            i++;
		} 
    }
}


/*
*Function:		hal_clcdIconTextOut
*Description:	图标区域固定行间距的字符串输出
*Input:			x:x坐标; y:y坐标; *str:要显示的字符串内容;*CurStFont:当前字库指针
*Output:		*CurpixelX:当前x坐标;*CurpixelY:当前y坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdIconTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, ST_FONT *CurStFont)
{
	int single_fontsize = 2, multi_fontsize = 2;
	int encode_width = 0;      /* Record the *str 's encode's width is one byte or two bytes,or other.*/
	unsigned char char_width = 0,char_height = 0;  /* Record a char width & height.(display attribute.)*/
	int line_height = 24;//FontAttr.lcd_cur_font.single_code_font.Height;                         /* Record line height.                              */
	char dot_buf[72];                             /*---------------------------------------------!!!!Need hal_commConvert to malloc-point or others implement.!!!!!!!!!---------------*/
    unsigned char *p_str;                         /* Define a temp str-point for the argument of str. */
    int isReverse = 0;
    
	if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH)||(y >= g_stLcdConfig.COLORLCD_PIXHIGH))
    {
		return ;
	}
	if(str == NULL)
    {
		return;
	}
    
	p_str = (unsigned char *)str;
	*CurpixelX = x;
	*CurpixelY = y;

	while(*p_str)
    {
		if(*p_str == '\n')
        {
			p_str++;
			*CurpixelX = 0;
			*CurpixelY += line_height;
			continue;
		}
		else if(*p_str < 0x20)
        {
			p_str++;
			continue;
		}
        memset(dot_buf,0,sizeof(dot_buf));
		/* ---Get the current *str's dot-value and *str 's encode width.---*/
		isReverse = 0;
		encode_width = Ft_GetDot_ASCII(isReverse, CurStFont, (unsigned char *)p_str, &char_width, &char_height, dot_buf);
		if(encode_width > 0){
			p_str += encode_width;
		}
		else 
        {
			return; /* if encode_width <= 0; the Ft_GetDot operation is work error,so retrun back-out.*/
		}
		/* Check the Followed display operation need to line-feed or not.  */
		if(encode_width==2)
		{
			if(*CurpixelX + char_width*2 > g_stLcdConfig.COLORLCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
			}
			if(*CurpixelY + line_height > g_stLcdConfig.COLORLCD_PIXHIGH){
				
				hal_clcdIconRefresh();
				
				return; //超出屏幕底行返回不显示
			}
		}
		else if(encode_width==1)
		{
			if(*CurpixelX + char_width > g_stLcdConfig.COLORLCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
			}
			if(*CurpixelY + line_height > g_stLcdConfig.COLORLCD_PIXHIGH){
				
				hal_clcdIconRefresh();
				
				return;
			}
		}
		
		/* Use DrawBitBmp to display a char(*str). */	
        hal_clcdIconDot2DispBuf(*CurpixelX,*CurpixelY,char_width,char_height,dot_buf,encode_width);
        *CurpixelX += char_width*1;
    }

   
    hal_clcdIconRefresh();	
    
}


/*
*Function:		hal_clcdTextOut
*Description:	用户区域区域固定行间距的字符串输出
*Input:			x:x坐标; y:y坐标; *str:要显示的字符串内容;*CurStFont:当前字库指针;mode:0-根据编码格式来处理;1-按照Unicode格式处理
*Output:		*CurpixelX:当前x坐标;*CurpixelY:当前y坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, uint8 mode)
{
	int single_fontsize = 2, multi_fontsize = 2;
	int encode_width = 0;      /* Record the *str 's encode's width is one byte or two bytes,or other.*/
	unsigned int char_width = 0,char_height = 0;  /* Record a char width & height.(display attribute.)*/
	int line_height = FontAttr.lcd_cur_font.single_code_font.Height;                         /* Record line height.                              */
	char dot_buf[72];                             /*---------------------------------------------!!!!Need hal_commConvert to malloc-point or others implement.!!!!!!!!!---------------*/ 
    unsigned char *p_str;                         /* Define a temp str-point for the argument of str. */
    int isReverse = 0;
	uint8 asciiflag = 0;
	unsigned short strtmp = 0;
	unsigned char *texttmprP = NULL;

	#define texttmpSIZE		4096

	texttmprP = malloc(texttmpSIZE);
	if(texttmprP == NULL)
		goto exit;
	memset(texttmprP, 0, texttmpSIZE);
	
	
	if((x >= g_stLcdConfig.COLORLCD_PIXWIDTH)||(y >= g_stLcdConfig.COLORLCD_PIXHIGH))
    {
		goto exit;
	}
	if(str == NULL)
    {
		goto exit;
	}
    
	if(mode == 0)
	{
		if(g_stColorlcdGUI.fonttype == 0)
	   	{
	    	int strlenth = strUTF8tostrUnicode(str,strlen(str),texttmprP);
			p_str = texttmprP;
	   	}
		else if(g_stColorlcdGUI.fonttype == 1)
		{
			uint16 cnt = 0;
			uint16 asciicnt = 0;
			while(1)
			{
				if(cnt*2+asciicnt > (strlen(str)-1))
				{
					break;
				}
				if(*(str+cnt*2+asciicnt) > 0x80)
				{
					strtmp = hal_fontGBcode2Unicode((unsigned short)((unsigned short)(*(str+cnt*2+asciicnt)<<8) + *(str+cnt*2+asciicnt+1)));
					*(texttmprP+cnt*2+asciicnt*2) = (strtmp&0xFF00)>>8;
					*(texttmprP+cnt*2+asciicnt*2+1) = (strtmp&0xFF);
					cnt ++;
				}
				else
				{
					*(texttmprP+cnt*2+asciicnt*2) = 0x00;
					*(texttmprP+cnt*2+asciicnt*2+1) = *(str+cnt*2+asciicnt);
					asciicnt++;
				}
				
			}
			p_str = texttmprP;
		}
		else if(g_stColorlcdGUI.fonttype == 2)
		{
			p_str = str;
		}
	}
	else

	{
		p_str = str;
	}
	
	*CurpixelX = x;
	*CurpixelY = y;

	do
    {
    	strtmp = (((*p_str)<<8)+*(p_str+1));
		
		if(strtmp == '\n')
        {
			p_str+=2;
			*CurpixelX = 0;
			*CurpixelY += line_height;
			continue;
		}
		else if(strtmp < 0x20)
        {
			p_str+=2;
			continue;
		}
        memset(dot_buf,0,sizeof(dot_buf));
		/* ---Get the current *str's dot-value and *str 's encode width.---*/
		if(FONTFS == hal_fontGetFontType())
			encode_width = hal_fontGetDot(isReverse,&FontAttr.lcd_cur_font,(unsigned char *)p_str,dot_buf,&char_width,&char_height);
//		else
//			encode_width = Ft_GetDot(isReverse,&FontAttr.lcd_cur_font,(unsigned char *)p_str,dot_buf,&char_width,&char_height);//,single_fontsize,multi_fontsize);

		isReverse = 0;
		
		if(encode_width > 0){
			p_str +=2;
		}
		else 
        {
			goto exit; /* if encode_width <= 0; the Ft_GetDot operation is work error,so retrun back-out.*/
		}
		/* Check the Followed display operation need to line-feed or not.  */
		if(encode_width==2)
		{
			if(*CurpixelX + char_width > g_stLcdConfig.COLORLCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
				sysLOG(COLORLCD_LOG_LEVEL_2, "*CurpixelX=%d, *CurpixelY=%d\r\n", *CurpixelX, *CurpixelY);
			}
			if(*CurpixelY + line_height > g_stLcdConfig.COLORLCD_PIXHIGH){
				
				if(g_stColorlcdGUI.AutoRefresh == 1)
				{
					hal_clcdRefresh();
				}
				goto exit; //超出屏幕底行返回不显示
			}
		}
		else if(encode_width==1)
		{
			if(*CurpixelX + char_width > g_stLcdConfig.COLORLCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
				sysLOG(COLORLCD_LOG_LEVEL_5, "*CurpixelX=%d, *CurpixelY=%d\r\n", *CurpixelX, *CurpixelY);
			}
			if(*CurpixelY + line_height > g_stLcdConfig.COLORLCD_PIXHIGH){
				
				if(g_stColorlcdGUI.AutoRefresh == 1)
				{
					hal_clcdRefresh();
				}
				goto exit;
			}
		}
		
		/* Use DrawBitBmp to display a char(*str). */	
        hal_clcdDot2DispBuf(*CurpixelX,*CurpixelY,char_width,char_height,dot_buf,encode_width);
        *CurpixelX += char_width*1;
    }while(strtmp);

    if(g_stColorlcdGUI.AutoRefresh == 1)
	{
        hal_clcdRefresh();	
    }

exit:

	if(texttmprP != NULL)
		free(texttmprP);
	
	return;
}


/*
*Function:		hal_clcdWriteIcon
*Description:	LCD 写图标
*Input:			x-x坐标；y-y坐标；*data-图标内容；width-图标占用的宽度；height-图标占用的高度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdWriteIcon(uint16 x, uint16 y, uint8 *data, uint8 width, uint8 height, unsigned short color)
{
	int iRet = -1;
	uint8 bitPixtmp;
	uint16 ix, iy;
	uint8 heighttmp;
	uint8 widthtmp;

	if(width == 12)
	{
		heighttmp = height;
		widthtmp = 16;
	}
	else if(width == 16)
	{
		heighttmp = height;
		widthtmp = width;
	}
	else if(width == 24)
	{
		heighttmp = height;
		widthtmp = width;
	}
	
	sysLOG(COLORLCD_LOG_LEVEL_5, "x=%d, y=%d, color=%d\r\n", x, y, color);
	for(iy=0; iy<height; iy++)
	{
		for(ix=0; ix<width; ix++)
		{
			
			bitPixtmp = (data[iy*(widthtmp/8) + ix/8] & (1 <<(7-ix%8))) >> (7-ix%8);
			if(bitPixtmp)
			{
				iRet = hal_clcdIconDrawDotC(x+ix, y+iy, color);
			}
			else

			{
				iRet = hal_clcdIconDrawDotC(x+ix, y+iy, g_stColorlcdGUI.icon_attr.iconAreaColor);
			}
			if(iRet < 0)
			{
				sysLOG(COLORLCD_LOG_LEVEL_5, "hal_clcdIconDrawDotC iRet=%d\r\n", iRet);
			}
		}
	}

	hal_clcdIconRefresh();

}


/*
*Function:		hal_clcdSetAttrib
*Description:	设置屏幕参数，
*Input:			attr:具体参考LCD_ATTRIBUTE_t
*				value:视attr参数而定，
*				LCD_ATTR_POWER	value:0-关闭，1-打开
*				LCD_ATTR_TEXT_INVERT	value:0-正显, 1-反显
*				LCD_ATTR_BACK_COLOR		value:0-0xFFFF
*				LCD_ATTR_FRONT_COLOR	value:0-0xFFFF
*				LCD_ATTR_AUTO_UPDATE	value:0-关闭自动刷新屏幕, 1-开启自动刷新，默认情况下是自动刷新
*				LCD_ATTR_ICON_BACK_COLOR	value:0-0xFFFF
*				LCD_ATTR_LIGHT_LEVEL	value:0-100
*				LCD_ATTR_FONTTYPE
*Output:		NULL
*Hardware:
*Return:		0-设置成功, <0-失败
*Others:
*/
int hal_clcdSetAttrib(LCD_ATTRIBUTE_t attr, int value)
{
	int iRet = 0;
	switch(attr)
	{	
		case LCD_ATTR_POWER://液晶电源控制
			if(value < 0 || value > 1)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			
			iRet = SCR_NOT_SUPPORT;
			
		break;
    	case LCD_ATTR_TEXT_INVERT://字体反显
			if(value < 0 || value > 1)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}

			g_stColorlcdGUI.Mode = (unsigned short int)value;
			iRet = 0;
		break;
   		case LCD_ATTR_BACK_COLOR://设置彩屏背景色
   			if(value < 0 || value > 0xFFFF)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stColorlcdGUI.grapBackColor = (unsigned short)value;
			iRet = 0;
		break;
    	case LCD_ATTR_FRONT_COLOR://设置彩屏前景色
    		if(value < 0 || value > 0xFFFF)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stColorlcdGUI.grapFrontColor = (unsigned short)value;
			iRet = 0;
		break;
    	case LCD_ATTR_AUTO_UPDATE://此项控制是否自动刷新RAM中的内容到屏幕。
    		if(value < 0 || value > 1)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stColorlcdGUI.AutoRefresh = (unsigned char)value;
			iRet = 0;
		break;
    	case LCD_ATTR_ICON_BACK_COLOR://设置图标区背景色

			iRet = SCR_NOT_SUPPORT;

		break;
   		case LCD_ATTR_LIGHT_LEVEL:
			if(value < 0 || value > 100)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stColorlcdGUI.BackLight = (unsigned char)value;
			hal_scrSetBackLightValue((unsigned char)value);
			iRet = 0;
		break;
		case LCD_ATTR_FONTTYPE:
			if(value < 0 || value > 2)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stColorlcdGUI.fonttype = (unsigned char)value;
		break;
		case LCD_ATTR_ICONHEIGHT:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_WIDTH:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_HEIGHT:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_USERWIDTH:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_USERHEIGHT:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_LCDCOLORVALUE:
			iRet = SCR_NOT_SUPPORT;
		break;
		default:
			
			iRet = SCR_NOT_SUPPORT;

		break;

		
	}

exit:
	return iRet;
}


/*
*Function:		hal_clcdGetAttrib
*Description:	获取液晶显示器的功能属性
*Input:			attr:LCD_ATTRIBUTE_t
*Output:		NULL
*Hardware:
*Return:		<0-失败，>=0-成功，具体值为所读取的功能属性value
*Others:
*/
int hal_clcdGetAttrib(LCD_ATTRIBUTE_t attr)
{

	int iRet = -1;
	switch(attr)
	{
		case LCD_ATTR_POWER:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_TEXT_INVERT:
			iRet = g_stColorlcdGUI.Mode;
		break;
		case LCD_ATTR_BACK_COLOR:
			iRet = g_stColorlcdGUI.grapBackColor;
		break;
		case LCD_ATTR_FRONT_COLOR:
			iRet = g_stColorlcdGUI.grapFrontColor;
		break;
		case LCD_ATTR_AUTO_UPDATE:
			iRet = g_stColorlcdGUI.AutoRefresh;
		break;
		case LCD_ATTR_ICON_BACK_COLOR:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_LIGHT_LEVEL:
			iRet = g_stColorlcdGUI.BackLight;
		break;
		case LCD_ATTR_FONTTYPE:
			iRet = g_stColorlcdGUI.fonttype;
		break;
		case LCD_ATTR_ICONHEIGHT:
			iRet = COLORLCD_TEXTPIXEL;
		break;
		case LCD_ATTR_WIDTH:
			iRet = g_stLcdConfig.COLORLCD_PIXWIDTH;
		break;
		case LCD_ATTR_HEIGHT:
			iRet = g_stLcdConfig.COLORLCD_PIXHIGH;
		break;
		case LCD_ATTR_USERWIDTH:
			iRet = g_stLcdConfig.COLORLCD_PIXWIDTH;
		break;
		case LCD_ATTR_USERHEIGHT:
			iRet = g_stLcdConfig.COLORLCD_PIXHIGH-COLORLCD_TEXTPIXEL;
		break;
		case LCD_ATTR_LCDCOLORVALUE:
			iRet = 3;
		break;
		case LCD_ATTR_BACKLIGHT_MODE:
			iRet = g_stColorlcdGUI.BackLightMode;
		break;		
		default:
			iRet = SCR_NOT_SUPPORT;
		break;
	
	}
	return iRet;

}


/*
*Function:		hal_clcdGotoxyPix
*Description:	定位LCD显示光标，参数超出LCD范围时不改变原坐标位置
*Input:			pixel_X:横坐标；pixel_Y:纵坐标
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_clcdGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y)
{
	if(pixel_X < 0 || pixel_X >=g_stLcdConfig.COLORLCD_PIXWIDTH || pixel_Y < 0 || pixel_Y >= g_stLcdConfig.COLORLCD_PIXHIGH)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}

	g_stCurPixel.x = pixel_X;
	g_stCurPixel.y = pixel_Y;
	return 0;
}


/*
*Function:		hal_clcdGetxyPix
*Description:	读取LCD上光标的当前位置。
*Input:			NULL
*Output:		*pixel_X:横坐标; *pixel_Y:纵坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdGetxyPix(int *pixel_X, int *pixel_Y)
{
	
	*pixel_X = g_stCurPixel.x;
	*pixel_Y = g_stCurPixel.y;

}


/*
*Function:		hal_clcdGetPixel
*Description:	获取特定坐标点的颜色值
*Input:			x:x坐标0-319；y:y坐标0-239；
*Output:		*picolor:颜色值
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_clcdGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor)
{
	if(x < 0 || x >=g_stLcdConfig.COLORLCD_PIXWIDTH || y < 0 || y >= g_stLcdConfig.COLORLCD_PIXHIGH)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	*picolor = *(g_stColorlcdGUI.DispBuff+y*g_stLcdConfig.COLORLCD_PIXWIDTH+x);
	return 0;
}


/*
*Function:		hal_clcdGetLcdSize
*Description:	读取LCD显示区域大小
*Input:			NULL
*Output:		*width:LCD显示宽度；*height:LCD显示高度
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdGetLcdSize(int *width, int *height)
{
	*width = g_stLcdConfig.COLORLCD_PIXWIDTH;
	*height = g_stLcdConfig.COLORLCD_PIXHIGH;

}


/*
*Function:		hal_clcdPrintf
*Description:	在屏幕的前景层当前位置格式化显示字符串
*Input:			*fmt:显示输出的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdPrintf(const char *fmt, ...)
{
	va_list args;
	char *bufrP = NULL;

	#define bufSIZE		1024

	bufrP = malloc(bufSIZE);
	if(bufrP == NULL)
		goto exit;
	memset(bufrP, 0, bufSIZE);
	
	va_start(args, fmt);
	vsnprintf(bufrP, bufSIZE-4,fmt, args);
	va_end(args);
	hal_clcdTextOut(g_stCurPixel.x,g_stCurPixel.y,bufrP, &g_stCurPixel.x, &g_stCurPixel.y, 0);

exit:

	if(bufrP != NULL)
		free(bufrP);

	return;


}


/*
*Function:		CalTextline
*Description:	计算每一行的起始点阵
*Input:			col:起始点阵列号; *str:显示内容; align:对齐方式,0-左对齐,1-居中,2-右对齐;
*Output:		*startlinepix:起始点阵输出指针,*startstrlen:每一行的字节长度
*Hardware:
*Return:		>0总共几行
*Others:
*/
int CalTextline(unsigned int col, unsigned char *str, unsigned int strlen, unsigned int align, uint32 *startlinepix, uint32 *startstrlen, uint32 lcdpixwidth)
{
	int iRet;
	int strlentmp = 0;
	int onelinecharnum;
	int onelinecharcnt = 0;
	int offset = 0;
	int onelineoffset = 0;
	unsigned char *strtmp = NULL;
	unsigned char coltmp = 0;
	int linenumtmp = 0;
	uint32 startlinepixtmp = 0;
	if(strlen <= 0)
	{
		return 0;
	}
	strlentmp = strlen;
	strtmp = str;
	coltmp = col;

	while(1)
	{
		if(linenumtmp != 0)
		{
			coltmp = 0;
		}
		onelinecharnum = (lcdpixwidth-coltmp)/((FontAttr.lcd_cur_font.single_code_font.Height)/2);//一行能存多少单内码;
		//sysLOG(COLORLCD_LOG_LEVEL_1, "strlentmp=%d,onelinecharnum=%d\r\n", strlentmp, onelinecharnum);

		if(*(strtmp+offset) == 0x00)
		{
			onelinecharcnt+=1;	
		}
		else

		{
			onelinecharcnt +=2;
		}
			
		offset+=2;
		strlentmp-=2;
		if(strlentmp < 0)strlentmp = 0;
		if(strlentmp == 0)
		{
			
			if(onelinecharcnt == onelinecharnum)
			{
				startlinepixtmp = coltmp;

				onelineoffset = offset - onelineoffset;
				*(startlinepix+linenumtmp)=startlinepixtmp;
				*(startstrlen+linenumtmp) = onelineoffset;
				linenumtmp++;
			}
			else if(onelinecharcnt > onelinecharnum)//汉字要保证双内码在一行
			{
				offset-=2;
				strlentmp+=2;
				onelinecharcnt-=2;
				startlinepixtmp = coltmp;

				onelineoffset = offset - onelineoffset;
				*(startlinepix+linenumtmp)=startlinepixtmp;
				*(startstrlen+linenumtmp) = onelineoffset;
				linenumtmp++;
			}
			else

			{
				if(align == 0)
				{
					startlinepixtmp = coltmp;
				}
				else if(align == 1)
				{
					startlinepixtmp = coltmp+((onelinecharnum-onelinecharcnt)/2)*((FontAttr.lcd_cur_font.single_code_font.Height)/2);
				}
				else
 if(align == 2)
				{
					startlinepixtmp = coltmp+((onelinecharnum-onelinecharcnt))*((FontAttr.lcd_cur_font.single_code_font.Height)/2);
					
				}

				onelineoffset = offset - onelineoffset;
				*(startlinepix+linenumtmp)=startlinepixtmp;
				*(startstrlen+linenumtmp) = onelineoffset;
				linenumtmp++;
			}
			sysLOG(COLORLCD_LOG_LEVEL_4, "1 onelinecharcnt=%d,strlentmp=%d,offset=%d, linenumtmp=%d,startlinepixtmp=%d, *(startlinepix+linenumtmp-1)=%d\r\n", onelinecharcnt, strlentmp, offset, linenumtmp, startlinepixtmp, *(startlinepix+linenumtmp-1));
			onelineoffset = offset;
			return linenumtmp;
		}
		if(onelinecharcnt == onelinecharnum)
		{
			onelineoffset = offset - onelineoffset;
			*(startlinepix+linenumtmp) = coltmp;
			*(startstrlen+linenumtmp) = onelineoffset;
			linenumtmp++;
			sysLOG(COLORLCD_LOG_LEVEL_4, "2 onelinecharcnt=%d,strlentmp=%d,offset=%d, linenumtmp=%d,*(startlinepix+linenumtmp-1)=%d\r\n", onelinecharcnt, strlentmp, offset, linenumtmp, *(startlinepix+linenumtmp-1));
			onelineoffset = offset;
			onelinecharcnt = 0;
			if(strlentmp <= 0)
			{
				return linenumtmp;
			}
		}
		else if(onelinecharcnt > onelinecharnum)//汉字要保证双内码在一行
		{
			offset-=2;
			strlentmp+=2;
			onelinecharcnt-=2;
			
			onelineoffset = offset - onelineoffset;
			*(startlinepix+linenumtmp) = coltmp;
			*(startstrlen+linenumtmp) = onelineoffset;
			linenumtmp++;
			sysLOG(COLORLCD_LOG_LEVEL_4, "3 onelinecharcnt=%d,strlentmp=%d,offset=%d, linenumtmp=%d,*(startlinepix+linenumtmp-1)=%d\r\n", onelinecharcnt, strlentmp, offset, linenumtmp, *(startlinepix+linenumtmp-1));
			onelineoffset = offset;
			onelinecharcnt = 0;
			if(strlentmp <= 0)
			{
				return linenumtmp;
			}
		}
		//sysDelayMs(10);
	}

	return linenumtmp;
	
}

/*
*Function:		Getlenstr
*Description:	计算字符串中字节个数
*Input:			*str:字符串指针;
*Output:		NULL
*Hardware:
*Return:		总共多少个字节
*Others:
*/
int Getlenstr(char *str)
{
	int iRet = 0;
	int sumlen = 0;
	int offset = 0;
	while(1)
	{
		
		
		if(*(str+sumlen) == 0x00)
		{
			if(*(str+sumlen+1) == 0)
			{
				return sumlen;
			}
			sumlen +=1;
		}
		else
		{
			sumlen +=1;
			
		}
	}
	
}
	

/*
*Function:		hal_clcdPrint
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
void hal_clcdPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...)
{
	unsigned int disMode;
	unsigned int dismodetmp = 0;
	unsigned int aligntmp = 0;
	unsigned int xtmp = 0, ytmp = 0;
	unsigned char *onelinebufrP = NULL;
	unsigned char *oncebufrP = NULL;
	unsigned int oncenum = 0;//没有回车换行的完整长度值
	//unsigned int onelinenum = 0;//一行显示的长度
	unsigned int sumlinenum = 0;//已经显示的长度
	char *rP = NULL;
	unsigned int linenum = 0;
	unsigned int linenumfinal = 0;
	unsigned char sumline = 0;
	unsigned int coltmp = 0;
	uint8 textpixeltmp = 24;
	
	char *p_str; 

	unsigned short strtmp = 0;
	unsigned char *texttmprP = NULL;

	uint32 startlinepix[32];
	uint32 startstrlen[32];
	uint32 linecharnumAlloffset = 0;
	char *bufrP = NULL;

	#define onelinebufSIZE	512
	#define oncebufSIZE		1024
	#define texttmpSIZE		1024
	#define bufSIZE			1024
	
	onelinebufrP = malloc(onelinebufSIZE);
	if(onelinebufrP == NULL)
		goto exit;
	memset(onelinebufrP, 0, onelinebufSIZE);

	oncebufrP = malloc(oncebufSIZE);
	if(oncebufrP == NULL)
		goto exit;
	memset(oncebufrP, 0, oncebufSIZE);

	texttmprP = malloc(texttmpSIZE);
	if(texttmprP == NULL)
		goto exit;
	memset(texttmprP, 0, texttmpSIZE);

	bufrP = malloc(bufSIZE);
	if(bufrP == NULL)
		goto exit;
	memset(bufrP, 0, bufSIZE);
	
	
	textpixeltmp = FontAttr.lcd_cur_font.single_code_font.Height;
	if(col%(textpixeltmp/2) != 0)
	{
		coltmp = col + ((textpixeltmp/2)-col%(textpixeltmp/2));
	}
	else

	{
		coltmp = col;
	}

	if(coltmp >= g_stLcdConfig.COLORLCD_PIXWIDTH)
	{
		coltmp = 0;
		row += 1;
	}

	if(row >= g_stLcdConfig.COLORLCD_PIXHIGH/COLORLCD_TEXTPIXEL)
		return;

	unsigned char *rP1 = NULL;
	unsigned char offsetenter = 0;
	
	disMode = g_stColorlcdGUI.Mode;
	dismodetmp = (mode&0x80) >> 7;//0-正显，1-反显
	aligntmp = (mode&0x06) >> 1;//0-左对齐，1-居中，2-右对齐
	
	sysLOG(COLORLCD_LOG_LEVEL_5, "mode=%x,dismodetmp=%d,aligntmp=%d\r\n", mode, dismodetmp, aligntmp);//modify by lbb

	va_list args;
	
	memset(bufrP, 0, bufSIZE);
	va_start(args, str);
	vsnprintf(bufrP, bufSIZE-4,str, args);
	va_end(args);

	sysLOG(COLORLCD_LOG_LEVEL_5, "start fonttype\r\n");//modify by lbb

	if(g_stColorlcdGUI.fonttype == 0)
   	{
    	int strlenth = strUTF8tostrUnicode(bufrP,strlen(bufrP),texttmprP);
		p_str = texttmprP;
   	}
	else if(g_stColorlcdGUI.fonttype == 1)
	{
		uint16 cnt = 0;
		uint16 asciicnt = 0;
		while(1)
		{
			if(cnt*2+asciicnt > (strlen(bufrP)-1))//(*(str+cnt*2) == 0 && *(str+cnt*2+1) == 0)
			{
				break;
			}
			if(*(bufrP+cnt*2+asciicnt) > 0x80)
			{
				strtmp = hal_fontGBcode2Unicode((unsigned short)((unsigned short)(*(bufrP+cnt*2+asciicnt)<<8) + *(bufrP+cnt*2+asciicnt+1)));
				*(texttmprP+cnt*2+asciicnt*2) = (strtmp&0xFF00)>>8;
				*(texttmprP+cnt*2+asciicnt*2+1) = (strtmp&0xFF);
				cnt ++;
			}
			else
			{
				*(texttmprP+cnt*2+asciicnt*2) = 0x00;
				*(texttmprP+cnt*2+asciicnt*2+1) = *(bufrP+cnt*2+asciicnt);
				asciicnt++;
			}
			
		}
		p_str = texttmprP;
	}
	else if(g_stColorlcdGUI.fonttype == 2)
	{
		p_str = bufrP;
	}
	

	sysLOG(COLORLCD_LOG_LEVEL_5, "end fonttype, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", *p_str,*(p_str+1),*(p_str+2),*(p_str+3),*(p_str+4),*(p_str+5),*(p_str+6),*(p_str+7),*(p_str+8),*(p_str+9));

	while(1)
	{
		rP = NULL;
		rP1 = NULL;
		
		rP = MyStrStr(p_str+sumlinenum, "\n", 0, 1024-sumlinenum);
		if(rP != NULL && *(rP-1) == 0x00)//有换行
		{
			
			oncenum = rP - (p_str+sumlinenum) + 1;
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
			offsetenter ++;
			rP1 = MyStrStr((char *)(p_str+sumlinenum), "\r", 0, 1024-sumlinenum);
			if(rP1 != NULL)
			{
				offsetenter ++;
			}
			sysLOG(COLORLCD_LOG_LEVEL_5, "sumlinenum=%d,rP=%x,&bufrP+sumlinenum=%x,oncenum=%d\r\n", sumlinenum, rP, bufrP+sumlinenum, oncenum);
			
		}
		else
		{
			oncenum = Getlenstr(p_str)-sumlinenum;
			sysLOG(COLORLCD_LOG_LEVEL_5, "2 Getlenstr(p_str)=%d,sumlinenum=%d\r\n", Getlenstr(p_str), sumlinenum);
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
		}
		memset(oncebufrP, 0, oncebufSIZE);
		memcpy(oncebufrP, p_str+sumlinenum, oncenum);//copy从已经显示内容之后到下一次\r\n为止，然后在逐行显示
		sumlinenum += oncenum;
		sysLOG(COLORLCD_LOG_LEVEL_5, "2 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenum = CalTextline(coltmp, oncebufrP, oncenum, aligntmp, startlinepix, startstrlen, g_stLcdConfig.COLORLCD_PIXWIDTH);//oncenum/((COLORLCD_PIXWIDTH-coltmp)/(textpixeltmp/2));		
		sysLOG(COLORLCD_LOG_LEVEL_5, "1 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenumfinal = 0;
		linecharnumAlloffset = 0;
		
		for(uint8 j=0; j<linenum; j++)
		{
			linecharnumAlloffset +=linenumfinal;
			linenumfinal = *(startstrlen+j);

			
			memset(onelinebufrP, 0, onelinebufSIZE);
			memcpy(onelinebufrP, oncebufrP+linecharnumAlloffset, linenumfinal);
			xtmp = *(startlinepix+j);
			if(aligntmp == 0)//居左
			{
				
			}
			else if(aligntmp == 1)//居中
			{
				if(j == linenum-1)
				{
					xtmp = xtmp+offsetenter*(textpixeltmp/2)/2;//最后一行判断下后面是否有\r\n，有的话需要偏移下
				}
				
			}
			else if(aligntmp == 2)//居右
			{
				if(j == linenum-1)
				{
					xtmp = xtmp+offsetenter*(textpixeltmp/2);//最后一行判断下后面是否有\r\n，有的话需要偏移下
				}
								
			}

			ytmp = row*COLORLCD_TEXTPIXEL+sumline*textpixeltmp;
			sysLOG(COLORLCD_LOG_LEVEL_5, "xtmp=%d, ytmp=%d, aligntmp=%d, linenumfinal=%d,onelinebufrP:%d,%d,%d,%d\r\n", xtmp, ytmp, aligntmp, linenumfinal, *onelinebufrP, *(onelinebufrP+1), *(onelinebufrP+2), *(onelinebufrP+3));
			g_stColorlcdGUI.Mode = dismodetmp;
			hal_clcdTextOut(xtmp, ytmp, onelinebufrP, &g_stCurPixel.x, &g_stCurPixel.y, 1);
			g_stColorlcdGUI.Mode = disMode;
			sumline ++;
		}
		

	}
	
exit:

	if(onelinebufrP != NULL)
		free(onelinebufrP);

	if(oncebufrP != NULL)
		free(oncebufrP);

	if(texttmprP != NULL)
		free(texttmprP);

	if(bufrP != NULL)
		free(bufrP);
		
	return;
}


/*
*Function:		hal_clcdPrintxy
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
void hal_clcdPrintxy(uint col, uint row, unsigned char mode, const char *str, ...)
{
	unsigned int disMode;
	unsigned int dismodetmp = 0;
	unsigned int aligntmp = 0;
	unsigned int xtmp = 0, ytmp = 0;
	unsigned char *onelinebufrP = NULL;
	unsigned char *oncebufrP = NULL;
	unsigned int oncenum = 0;//没有回车换行的完整长度值
	//unsigned int onelinenum = 0;//一行显示的长度
	unsigned int sumlinenum = 0;//已经显示的长度
	char *rP = NULL;
	unsigned int linenum = 0;
	unsigned int linenumfinal = 0;
	unsigned char sumline = 0;
	unsigned int coltmp = 0;
	uint8 textpixeltmp = 24;
	
	char *p_str; 

	unsigned short strtmp = 0;
	unsigned char *texttmprP = NULL;

	uint32 startlinepix[32];
	uint32 startstrlen[32];
	uint32 linecharnumAlloffset = 0;
	char *bufrP = NULL;

	#define onelinebufSIZE	512
	#define oncebufSIZE		1024
	#define texttmpSIZE		1024
	#define bufSIZE			1024
	
	onelinebufrP = malloc(onelinebufSIZE);
	if(onelinebufrP == NULL)
		goto exit;
	memset(onelinebufrP, 0, onelinebufSIZE);

	oncebufrP = malloc(oncebufSIZE);
	if(oncebufrP == NULL)
		goto exit;
	memset(oncebufrP, 0, oncebufSIZE);

	texttmprP = malloc(texttmpSIZE);
	if(texttmprP == NULL)
		goto exit;
	memset(texttmprP, 0, texttmpSIZE);

	bufrP = malloc(bufSIZE);
	if(bufrP == NULL)
		goto exit;
	memset(bufrP, 0, bufSIZE);
	
	
	textpixeltmp = FontAttr.lcd_cur_font.single_code_font.Height;
//	if(col%(textpixeltmp/2) != 0)
//	{
//		coltmp = col + ((textpixeltmp/2)-col%(textpixeltmp/2));
//	}
//	else

//	{
//		coltmp = col;
//	}
	coltmp = col;
	if(coltmp >= g_stLcdConfig.COLORLCD_PIXWIDTH)
	{
		coltmp = 0;
		row += COLORLCD_TEXTPIXEL;
	}

	if(row >= g_stLcdConfig.COLORLCD_PIXHIGH)
		return;

	unsigned char *rP1 = NULL;
	unsigned char offsetenter = 0;
	
	disMode = g_stColorlcdGUI.Mode;
	dismodetmp = (mode&0x80) >> 7;//0-正显，1-反显
	aligntmp = (mode&0x06) >> 1;//0-左对齐，1-居中，2-右对齐
	
	sysLOG(COLORLCD_LOG_LEVEL_5, "mode=%x,dismodetmp=%d,aligntmp=%d\r\n", mode, dismodetmp, aligntmp);//modify by lbb

	va_list args;
	
	memset(bufrP, 0, bufSIZE);
	va_start(args, str);
	vsnprintf(bufrP, bufSIZE-4,str, args);
	va_end(args);

	sysLOG(COLORLCD_LOG_LEVEL_5, "start fonttype\r\n");//modify by lbb

	if(g_stColorlcdGUI.fonttype == 0)
   	{
    	int strlenth = strUTF8tostrUnicode(bufrP,strlen(bufrP),texttmprP);
		p_str = texttmprP;
   	}
	else if(g_stColorlcdGUI.fonttype == 1)
	{
		uint16 cnt = 0;
		uint16 asciicnt = 0;
		while(1)
		{
			if(cnt*2+asciicnt > (strlen(bufrP)-1))//(*(str+cnt*2) == 0 && *(str+cnt*2+1) == 0)
			{
				break;
			}
			if(*(bufrP+cnt*2+asciicnt) > 0x80)
			{
				strtmp = hal_fontGBcode2Unicode((unsigned short)((unsigned short)(*(bufrP+cnt*2+asciicnt)<<8) + *(bufrP+cnt*2+asciicnt+1)));
				*(texttmprP+cnt*2+asciicnt*2) = (strtmp&0xFF00)>>8;
				*(texttmprP+cnt*2+asciicnt*2+1) = (strtmp&0xFF);
				cnt ++;
			}
			else
			{
				*(texttmprP+cnt*2+asciicnt*2) = 0x00;
				*(texttmprP+cnt*2+asciicnt*2+1) = *(bufrP+cnt*2+asciicnt);
				asciicnt++;
			}
			
		}
		p_str = texttmprP;
	}
	else if(g_stColorlcdGUI.fonttype == 2)
	{
		p_str = bufrP;
	}
	

	sysLOG(COLORLCD_LOG_LEVEL_5, "end fonttype, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", *p_str,*(p_str+1),*(p_str+2),*(p_str+3),*(p_str+4),*(p_str+5),*(p_str+6),*(p_str+7),*(p_str+8),*(p_str+9));

	while(1)
	{
		rP = NULL;
		rP1 = NULL;
		
		rP = MyStrStr(p_str+sumlinenum, "\n", 0, 1024-sumlinenum);
		if(rP != NULL && *(rP-1) == 0x00)//有换行
		{
			
			oncenum = rP - (p_str+sumlinenum) + 1;
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
			offsetenter ++;
			rP1 = MyStrStr((char *)(p_str+sumlinenum), "\r", 0, 1024-sumlinenum);
			if(rP1 != NULL)
			{
				offsetenter ++;
			}
			sysLOG(COLORLCD_LOG_LEVEL_5, "sumlinenum=%d,rP=%x,&bufrP+sumlinenum=%x,oncenum=%d\r\n", sumlinenum, rP, bufrP+sumlinenum, oncenum);
			
		}
		else
		{
			oncenum = Getlenstr(p_str)-sumlinenum;
			sysLOG(COLORLCD_LOG_LEVEL_5, "2 Getlenstr(p_str)=%d,sumlinenum=%d\r\n", Getlenstr(p_str), sumlinenum);
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
		}
		memset(oncebufrP, 0, oncebufSIZE);
		memcpy(oncebufrP, p_str+sumlinenum, oncenum);//copy从已经显示内容之后到下一次\r\n为止，然后在逐行显示
		sumlinenum += oncenum;
		sysLOG(COLORLCD_LOG_LEVEL_5, "2 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenum = CalTextline(coltmp, oncebufrP, oncenum, aligntmp, startlinepix, startstrlen, g_stLcdConfig.COLORLCD_PIXWIDTH);//oncenum/((COLORLCD_PIXWIDTH-coltmp)/(textpixeltmp/2));		
		sysLOG(COLORLCD_LOG_LEVEL_5, "1 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenumfinal = 0;
		linecharnumAlloffset = 0;
		
		for(uint8 j=0; j<linenum; j++)
		{
			linecharnumAlloffset +=linenumfinal;
			linenumfinal = *(startstrlen+j);

			
			memset(onelinebufrP, 0, onelinebufSIZE);
			memcpy(onelinebufrP, oncebufrP+linecharnumAlloffset, linenumfinal);
			xtmp = *(startlinepix+j);
			if(aligntmp == 0)//居左
			{
				
			}
			else if(aligntmp == 1)//居中
			{
				if(j == linenum-1)
				{
					xtmp = xtmp+offsetenter*(textpixeltmp/2)/2;//最后一行判断下后面是否有\r\n，有的话需要偏移下
				}
				
			}
			else if(aligntmp == 2)//居右
			{
				if(j == linenum-1)
				{
					xtmp = xtmp+offsetenter*(textpixeltmp/2);//最后一行判断下后面是否有\r\n，有的话需要偏移下
				}
								
			}

			ytmp = row+sumline*textpixeltmp;
			sysLOG(COLORLCD_LOG_LEVEL_5, "xtmp=%d, ytmp=%d, aligntmp=%d, linenumfinal=%d,onelinebufrP:%d,%d,%d,%d\r\n", xtmp, ytmp, aligntmp, linenumfinal, *onelinebufrP, *(onelinebufrP+1), *(onelinebufrP+2), *(onelinebufrP+3));
			g_stColorlcdGUI.Mode = dismodetmp;
			hal_clcdTextOut(xtmp, ytmp, onelinebufrP, &g_stCurPixel.x, &g_stCurPixel.y, 1);
			g_stColorlcdGUI.Mode = disMode;
			sumline ++;
		}
		

	}
	
exit:

	if(onelinebufrP != NULL)
		free(onelinebufrP);

	if(oncebufrP != NULL)
		free(oncebufrP);

	if(texttmprP != NULL)
		free(texttmprP);

	if(bufrP != NULL)
		free(bufrP);
		
	return;
}



/*
*Function:		hal_clcdRotate180
*Description:	旋转屏幕180
*Input:			value:0-正常显示，1-倒显
*Output:		NULL
*Hardware:
*Return:		<0-失败；>=0-成功，成功处理的字节数
*Others:
*/
int hal_clcdRotate180(uint8 value)
{
	int iRet = -1;
	
	if(g_ui32LcdDevID == ST7789V2_ID)//2.8寸彩屏
	{	
		if(value == 0)
		{
			if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_90);
			}		
			else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL);
			}
			sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_lcd_SetBrushDirection iRet:%d\r\n", iRet);
		}
		else if(value == 1)
		{
			if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_270);
			}			
			else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_180);
			}
			sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_lcd_SetBrushDirection iRet:%d\r\n", iRet);
		}
	}
	else if(g_ui32LcdDevID == ST7789V3A_ID)//2.4寸彩屏
	{
		if(value == 0)
		{
			if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_270);
			}		
			else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL);
			}
			sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_lcd_SetBrushDirection iRet:%d\r\n", iRet);
		}
		else if(value == 1)
		{
			if(g_stLcdConfig.LCD_DIRECTION == 0)//横屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_90);
			}
			else if(g_stLcdConfig.LCD_DIRECTION == 1)//竖屏
			{
				iRet = fibo_lcd_SetBrushDirection(LCD_DIRECT_ROT_270);
			}
			sysLOG(COLORLCD_LOG_LEVEL_2, "fibo_lcd_SetBrushDirection iRet:%d\r\n", iRet);

		}
	}
	int m;
	for(m=0; m<g_stLcdConfig.COLORLCD_BLOCKBUFNUM; m++)
	g_stColorlcdGUI.RefreshFlag[m] = 0xFF;

	hal_clcdRefreshRotate();

}




/*
*Function:		hal_clcdDrawColorCircle
*Description:	空心圆绘制可改变颜色
*Input:			x0-圆心x坐标;y0-圆心y坐标;r-半径
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawColorCircle(unsigned short int x0, unsigned short int y0, unsigned short int r, unsigned short int color)
{
    int a, b;
    int di;
    a = 0; b = r;
    di = 3 - (r << 1);//判断下个点位置的标志
    while (a <= b)
    {
        hal_clcdDrawSinglePix(x0 + a, y0 - b,color);
        hal_clcdDrawSinglePix(x0 + b, y0 - a,color);
        hal_clcdDrawSinglePix(x0 + b, y0 + a,color);
        hal_clcdDrawSinglePix(x0 + a, y0 + b,color);
        hal_clcdDrawSinglePix(x0 - a, y0 + b,color);
        hal_clcdDrawSinglePix(x0 - b, y0 + a,color);
        hal_clcdDrawSinglePix(x0 - a, y0 - b,color);
        hal_clcdDrawSinglePix(x0 - b, y0 - a,color);
        a++;
        if(di < 0) di += 4 * a + 6;//使用Bresenham算法画圆
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }

}


/*
*Function:		hal_clcdDrawCircle
*Description:	实心圆绘制(认证接口——屏幕灯)
*Input:			x0-圆心x坐标;y0-圆心y坐标;r-半径,color：颜色，flag：0-清除颜色，1-使能颜色
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_clcdDrawFullCircle(unsigned short int x0, unsigned short int y0, unsigned short int r, unsigned short int color ,bool flag)
{
	unsigned short int temp;

	if (flag == FALSE)
	{
		temp = g_stColorlcdGUI.grapBackColor;
	}
	else
	{
		temp = color; 
	}
	while (r != 0)
	{

		hal_clcdDrawColorCircle(x0,y0,r,temp);
		r--;
	}
	if(g_stColorlcdGUI.AutoRefresh == 1)
	{
		hal_clcdRefresh();
	}
	
}

/*
*Function:		hal_clcdDrawCircle
*Description:	实心圆绘制(认证接口——屏幕灯)
*Input:			color：颜色，flag：0-清除颜色，1-使能颜色
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdDrawLamp(unsigned int color,bool flag)
{
	switch (color)
	{
	case BLUE:
		if (g_iType_machine == 0)
		{
			hal_clcdDrawFullCircle(45,150,17,BLUE,flag);
		}
		else
		{
			hal_clcdDrawFullCircle(45,250,17,BLUE,flag);
		}
		break;

	case YELLOW:
		if (g_iType_machine == 0)
		{
			hal_clcdDrawFullCircle(95,150,17,YELLOW,flag);
		}
		else
		{
			hal_clcdDrawFullCircle(95,250,17,YELLOW,flag);
		}
		break;

	case GREEN:
		if (g_iType_machine == 0)
		{
			hal_clcdDrawFullCircle(145,150,17,GREEN,flag);
		}
		else
		{
			hal_clcdDrawFullCircle(145,250,17,GREEN,flag);
		}	
		break;
	case RED:
		if (g_iType_machine == 0)
		{
			hal_clcdDrawFullCircle(195,150,17,RED,flag);
		}
		else
		{
			hal_clcdDrawFullCircle(195,250,17,RED,flag);
		}
		break;
	default:
		break;
	}
	
}

/**********************TEST************************************/

