/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_seg.c
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
#include "comm.h"
#include "drv_spi.h"
#include "fibo_opencpu.h"


uint8 g_ui8SegLcdContrastValue = 36;

LCD_GUI	g_stSegLcdGUI;

#define SEG_CS_H	{fibo_gpio_set(SEGLCDSPICSN_GPIO, true);\
					fibo_gpio_mode_set(LCDSPICSN_GPIO, GpioFunction2);}

#define SEG_CS_L	{fibo_gpio_set(SEGLCDSPICSN_GPIO, false);\
					fibo_gpio_mode_set(LCDSPICSN_GPIO, GpioFunction0);\
					fibo_gpio_cfg(LCDSPICSN_GPIO, GpioCfgOut);\
					fibo_gpio_set(LCDSPICSN_GPIO, true);}

void hal_segWriteCmd(uint8_t data)
{
	
	SEG_CS_L
		
	cus_export_api->fibo_genspi_lcd_write_cmd(data);
	
	SEG_CS_H
		
}

void hal_segWriteData(uint8_t data)
{

	SEG_CS_L
		
	cus_export_api->fibo_genspi_lcd_write_data(data);
	
	SEG_CS_H
		
}

void hal_segWriteDataBuff(uint8_t *data, uint32 datalen)
{
	uint32 i;
	
	SEG_CS_L
	
	for(i=0; i<datalen; i++)
		cus_export_api->fibo_genspi_lcd_write_data(*(data+i));
	
	SEG_CS_H
		
}


void hal_segCls(void)
{
	unsigned char row,col;
	unsigned char data[SEGLCD_PIXWIDTH];

	for (row=0xb0; row<0xb4; row++)
	{
		hal_segWriteCmd(row);//set page address
		hal_segWriteCmd(0x10);//set column address
		hal_segWriteCmd(0x00);

		memset(data, 0, sizeof(data));
		hal_segWriteDataBuff(data, SEGLCD_PIXWIDTH);
//		for(col=0;col<SEGLCD_PIXWIDTH;col++)
//			hal_segWriteData(data);
	} 
}


/*
*Function:		hal_segSendData
*Description:	刷内存到屏幕中
*Input:			pagenum:0~3; col:纵坐标0~127; *data:屏显缓存指针, datelen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segSendData(int pagenum, unsigned char col, unsigned char *data, uint32 datalen)
{
	unsigned char pageaddr;
	unsigned char columnaddr;
	uint32 len = 0;

	pageaddr = pagenum | 0xb0;
	hal_segWriteCmd(pageaddr);//set page address


	hal_segWriteCmd(0x00 | (col&0x0F));
	hal_segWriteCmd(0x10 | ((col&0xF0)>>4));//set column address
	

	if(datalen > (128-col))
		len = 128-col;
	else
		len = datalen;

	hal_segWriteDataBuff(data, len);
//	for(columnaddr=0; columnaddr<len; columnaddr++)
//		hal_segWriteData(*data++);
   
}

/*
*Function:		hal_segLcdWrite
*Description:	刷屏，按块block刷屏
*Input:			col: 列地址
*				blockindex:block索引
*				*dat:需要刷新的内容指针
*				datLen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segLcdWrite(unsigned int col, unsigned int blockindex, unsigned char *dat, unsigned int datLen)
{

	uint8 pageindextmp = 0;

	
	pageindextmp = blockindex;
	
	hal_segSendData(pageindextmp, 0, dat, SEGLCD_BLOCKWIDTH);
	

}

static uint8 hal_lcdCharrotate(uint8 value)
{
	uint8 dottmp = 0;
	
	dottmp |= (value&0b00000011)<<6;
	dottmp |= (value&0b00001100)<<2;
	dottmp |= (value&0b00110000)>>2;
	dottmp |= (value&0b11000000)>>6;

	return dottmp;
}


/*
*Function:		hal_segDotRotate
*Description:	屏幕缓存内容转向
*Input:			*dotbuffin-缓存输入指针; len-需要处理的长度
*Output:		*dotbuffout-缓存输出指针
*Hardware:
*Return:		成功处理的长度
*Others:
*/
static int hal_segDotRotate(unsigned char *dotbuffin, unsigned char *dotbuffout, int len)
{
	unsigned char dottmp1 = 0, dottmp2 = 0;
	
	for(int i=0; i<len/2; i++)
	{
		dottmp1 = *(dotbuffin+i);
		dottmp2 = *(dotbuffin+len-1-i);
		*(dotbuffout+len-1-i) = dottmp1;
		*(dotbuffout+i) = dottmp2;
	}
	return len;
}

/*
*Function:		hal_segRefresh
*Description:	根据需要刷新整个屏幕,图标区域除外
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segRefresh(void)
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;

	unsigned char refreshflagtmp = 0;
	uint8 startblocknum = 0;
	
	refreshflagtmp = g_stSegLcdGUI.RefreshFlag[0] & 0x07;

	for (i=startblocknum; i<SEGLCD_BLOCKBUFNUM; i++)
	{
	    //如果有一个块需要刷新
		if(g_stSegLcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			if(g_stSegLcdGUI.rotate == 0)
			{
				j = i;
				disp = g_stSegLcdGUI.DispBuff+SEGLCD_BLOCKWIDTH*j;
			}
//			else if(g_stSegLcdGUI.rotate == 1)
//			{
//				j = SEGLCD_BLOCKBUFNUM - 1 - i;
//				
//				hal_segDotRotate(g_stSegLcdGUI.DispBuff+SEGLCD_BLOCKWIDTH*i, g_stSegLcdGUI.rotateBuff+SEGLCD_BLOCKWIDTH*j, SEGLCD_BLOCKWIDTH);
//				for(int i=0; i<SEGLCD_BLOCKWIDTH; i++)
//				{
//					*(g_stSegLcdGUI.rotateBuff+SEGLCD_BLOCKWIDTH*j+i) = hal_lcdCharrotate(*(g_stSegLcdGUI.rotateBuff+SEGLCD_BLOCKWIDTH*j+i));
//				}
//				
//				disp = g_stSegLcdGUI.rotateBuff+SEGLCD_BLOCKWIDTH*j;
//			}
			
			
			hal_segLcdWrite(0, j, disp, SEGLCD_BLOCKWIDTH);
			

		}
	}

	memset(g_stSegLcdGUI.RefreshFlag,0,sizeof(g_stSegLcdGUI.RefreshFlag));
	g_stSegLcdGUI.RefreshFlag[0] = refreshflagtmp;

}



/*
*Function:		hal_segDrawSinglePix
*Description:	黑白屏画一个点，不包括刷新
*Input:			x:x坐标;y:y坐标;dotColor:像素值，支持0-1,两个色阶
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segDrawSinglePix(unsigned short int x,unsigned short int y,unsigned char dotColor)
{
    int blocknum = 0;
	
    blocknum = y/SEGLCD_BLOCKHIGH;
	g_stSegLcdGUI.RefreshFlag[blocknum/8] |= (1<<(blocknum%8));// 1字节记录8个block 看是否更新
	switch (dotColor){
		
		case 0:
			*(g_stSegLcdGUI.DispBuff+blocknum*SEGLCD_BLOCKWIDTH+x) &= ~(1<<((y%SEGLCD_BLOCKHIGH)));
		break;
		case 1:
			*(g_stSegLcdGUI.DispBuff+blocknum*SEGLCD_BLOCKWIDTH+x) |= 1<<((y%SEGLCD_BLOCKHIGH));
		break;
		default:
			*(g_stSegLcdGUI.DispBuff+blocknum*SEGLCD_BLOCKWIDTH+x) |= 1<<((y%SEGLCD_BLOCKHIGH));
		break;
	}


}



/*
*Function:		hal_segDrawDot
*Description:	内容区画点并以前景色显示,不带刷新
*Input:			x-x坐标；y-y坐标；Mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_segDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode)
{   
	if((x >= SEGLCD_PIXWIDTH) || (y >= SEGLCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
    if(((g_stSegLcdGUI.Mode == 1) && (Mode == 0)) || ((g_stSegLcdGUI.Mode == 0) && (Mode == 1)))
    {     
		hal_segDrawSinglePix(x, y, g_stSegLcdGUI.grapFrontColor);
    }
    else
    {
        hal_segDrawSinglePix(x, y, g_stSegLcdGUI.grapBackColor);
    }
	return 0;

}


/*
*Function:		hal_segDrawDotC
*Description:	画点并以渲染值显示,不带刷新
*Input:			x-x坐标；y-y坐标；color:颜色值支持0-1
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_segDrawDotC(unsigned short int x, unsigned short int y, unsigned short color)
{   
	if((x >= SEGLCD_PIXWIDTH) || (y >= SEGLCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(color<0 || color > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
         
	hal_segDrawSinglePix(x,y,color);
   
	return 0;

}


/*
*Function:		hal_segWriteBitData
*Description:	把指定高度和宽度的点阵数据写到指定的行和列的屏幕缓冲中，不包括刷新
*Input:			x-x坐标;
*				y-y坐标;
*    			*dat-显示点阵数据
*    			width-显示点阵数据宽度(0 - SEGLCD_MAX_X)
*    			height-显示点阵数据高度(0 - SEGLCD_MAX_Y)
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segWriteBitData(int x, int y, unsigned char *dat, int width, int height)
{
    unsigned char ucBitPix;
    int i,j;
    int des_x = (x >= 0) ? x : 0;
    int src_x = (x >= 0) ? 0 : -x;
    int des_y = (y >= 0) ? y : 0;
    int src_y = (y >= 0) ? 0 : -y;
    int w = (x >= 0) ? width  : width + x;
    int h = (y >= 0) ? height : height + y;
    
    if((x >= SEGLCD_PIXWIDTH) || (x+width <= 0) || (width <= 0))
		return;
	if((y >= SEGLCD_PIXHIGH) || (y+height<= 0) || (height <= 0))
		return;
    
	for( i=0; i<h; i++)
	{
		for( j=0; j<w; j++)
		{
            ucBitPix = (dat[i*w/8 + j/8] & (1 <<(7-j%8)));
            if(ucBitPix)
            {
                hal_segDrawDotC(des_x+j, des_y+i, g_stSegLcdGUI.grapFrontColor);
            }
            else
            {
                hal_segDrawDotC(des_x+j, des_y+i, g_stSegLcdGUI.grapBackColor);
            }
		}
	}

}


/*
*Function:		hal_segDrawSingleLine
*Description:	画直线，不包括刷新
*Input:			x1-起始点x坐标;y1-起始点y坐标;x2-结束点x坐标;y2-结束点y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segDrawSingleLine(int x1,int y1,int x2,int y2, unsigned char mode)
{
    unsigned short int i,j;
    
	if(x1<0)	x1=0;
	if(x2<0)	x2=0;
	if(y1<0)	y1=0;
	if(y2<0)	y2=0;
		
    if(x1 > x2 || y1 > y2 || x1 < 0 || y1 < 0 || x2 >= SEGLCD_PIXWIDTH || y2 >= SEGLCD_PIXHIGH)
        return;

    if(y1 < y2)
    {
        for (i = y1; i <= y2; i++)       //画线输出
        {
            if(x1 < x2)
            {
                for(j = x1; j<= x2; j++)
                {
                    hal_segDrawDot(j, i,mode);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_segDrawDot(j, i,mode);
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
                    hal_segDrawDot(j, i,mode);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_segDrawDot(j, i,mode);
                }
            }
        }
    }
	
}

/*
*Function:		hal_segDrawStraightLine
*Description:	画直线
*Input:			x1-起始点x坐标;y1-起始点y坐标;x2-结束点x坐标;y2-结束点y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segDrawStraightLine(int x1,int y1,int x2,int y2, unsigned char mode)
{
	hal_segDrawSingleLine(x1, y1, x2, y2, mode);
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
		hal_segRefresh();
	}
}


/*
*Function:		hal_segDrawRectangle
*Description:	画矩形框
*Input:			x1-左上角x坐标;y1-左上角y坐标;x2-右下角x坐标;y2-右下角y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segDrawRectangle(int x1,int y1,int x2,int y2, unsigned char mode)
{
    hal_segDrawSingleLine(x1, y1, x2, y1, mode);
    hal_segDrawSingleLine(x1, y1, x1, y2, mode);
    hal_segDrawSingleLine(x1, y2, x2, y2, mode);
    hal_segDrawSingleLine(x2, y1, x2, y2, mode);
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
		hal_segRefresh();
	}
}


/*
*Function:		hal_segDrawRectBlock
*Description:	用指定颜色在背景层绘制实心矩形色块
*Input:			left:左；top:上；right:右；bottom:底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_segDrawRectBlock(int left,int top,int right,int bottom, unsigned short color)
{
	int i,j;
	
	if (left > right || top > bottom || right >= SEGLCD_PIXWIDTH || bottom >= SEGLCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
    if(left<0||top<0||right<0||bottom<0)
    {
		return LCD_ERR_AREA_INVALID;
	}
	if((color < 0)||(color > 3))
	{
		return LCD_ERR_NO_COLOR_INDEX;
	}
	
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_segDrawSinglePix(i, j, color);
		}
	}
	
	
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
	    hal_segRefresh();
	}
	return 0;
}


/*
*Function:		hal_segDrawCircle
*Description:	空心圆绘制
*Input:			x0-圆心x坐标;y0-圆心y坐标;r-半径
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segDrawCircle(unsigned short int x0, unsigned short int y0, unsigned short int r)
{
    int a, b;
    int di;
    a = 0; b = r;
    di = 3 - (r << 1);             //判断下个点位置的标志
    while (a <= b)
    {
        hal_segDrawDot(x0 + a, y0 - b,1);
        hal_segDrawDot(x0 + b, y0 - a,1);
        hal_segDrawDot(x0 + b, y0 + a,1);
        hal_segDrawDot(x0 + a, y0 + b,1);
        hal_segDrawDot(x0 - a, y0 + b,1);
        hal_segDrawDot(x0 - b, y0 + a,1);
        hal_segDrawDot(x0 - a, y0 - b,1);
        hal_segDrawDot(x0 - b, y0 - a,1);
        a++;
        if(di < 0) di += 4 * a + 6;   //使用Bresenham算法画圆
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
		hal_segRefresh();
	}
}


/*
*Function:		hal_segClsArea
*Description:	将指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_segClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 

	int i,j;
    
 	if (left > right || top > bottom || right >= SEGLCD_PIXWIDTH || bottom >= SEGLCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
	
	
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_segDrawSinglePix(i, j, g_stSegLcdGUI.grapBackColor);
		}
	}
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
		hal_segRefresh();
	}
	
    return 0;

}


/*
*Function:		hal_segClsUserArea
*Description:	清除整个屏幕不包括图标区域，执行该指令后，将光标移动到左上角（0,12）
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_segClsUserArea(void)
{
    int iRet = 0;
	unsigned int startrow = 0;
    
    iRet = hal_segClsArea(0,startrow,SEGLCD_PIXWIDTH-1,SEGLCD_PIXHIGH-1);

	g_stSegLcdCurPixel.x = 0;
	g_stSegLcdCurPixel.y = 0;
	
    return iRet;
}


/*
*Function:		hal_segClrLine
*Description:	清除前景层指定的一行或若干行，参数不合理无动作，
*				执行该指令后光标停留在（0，startline）行号从1到7共7行，默认行高为12个bit
*Input:			startline:起始字符行号；endline:结束字符行号
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_segClrLine(unsigned char startline, unsigned char endline)
{
	int iRet = -1;


	iRet = hal_segDrawRectBlock(0, startline*SEGLCD_TEXTPIXEL, SEGLCD_PIXWIDTH-1, (endline+1)*SEGLCD_TEXTPIXEL-1, g_stSegLcdGUI.grapBackColor);
	g_stSegLcdCurPixel.x = 0;
	g_stSegLcdCurPixel.y = startline*SEGLCD_TEXTPIXEL;

	return iRet;
}

	
/*
*Function:		hal_segWriteBitMap
*Description:	根据data[0]来识别高位在前还是地位在前识别,把指定高度和宽度的点阵数据写到屏幕上
*Input:			left-x坐标;
*				top-y坐标;
*    			*data-显示点阵数据
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败
*Others:
*/
int hal_segWriteBitMap(int left, int top, unsigned char *data)
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
    
    if(w > SEGLCD_PIXWIDTH || h > SEGLCD_PIXHIGH)
        return LCD_ERR_PIC_DATA_OVERFLOW;
  
    if(w == 0 || h == 0)
        return LCD_ERR_PIC_DATA_EMPTY;
    if(w%8 != 0)
        return LCD_ERR_PARAM_ERROR;
    
	hal_segWriteBitData(left, top, p + 6, w, h);
	if(g_stSegLcdGUI.AutoRefresh == 1)
	{
		hal_segRefresh();
	}

    return LCD_ERR_SUCCESS;
}


/*
*Function:		hal_segDot2DispBuf
*Description:	字模点阵写到显示缓存中
*Input:			x:x坐标; y:y坐标; width:字模宽度; height:字模高度; *dot:字模点阵指针; encode_witdh:1-单内码; 2-多内码
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_segDot2DispBuf(unsigned int x,unsigned int y,int width,int height,char *dot,int encode_width)//,int single_size,int multi_size)
{

    unsigned char ucBitPix;
    int k;
    int i,j;
    int value_width;
    int value_height;
    int offset;
        
	if(x+width>SEGLCD_PIXWIDTH || y+height>SEGLCD_PIXHIGH)
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
                    
					hal_segDrawDot(x+offset+k,y+i,ucBitPix);           
                   
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
                    
					hal_segDrawDot(x+offset+k,y+i,ucBitPix);           
                    
    			}
                offset += (8);
                dot++;
    		  }
            i++;
		} 
    }
}


/*
*Function:		hal_segTextOut
*Description:	用户区域区域固定行间距的字符串输出
*Input:			x:x坐标; y:y坐标; *str:要显示的字符串内容;*CurStFont:当前字库指针;mode:0-根据编码格式来处理;1-按照Unicode格式处理
*Output:		*CurpixelX:当前x坐标;*CurpixelY:当前y坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, uint8 mode)
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
	
	
	if((x >= SEGLCD_PIXWIDTH)||(y >= SEGLCD_PIXHIGH))
    {
		goto exit;
	}
	if(str == NULL)
    {
		goto exit;
	}
    
	if(mode == 0)
	{
		if(g_stSegLcdGUI.fonttype == 0)
	   	{
	    	int strlenth = strUTF8tostrUnicode(str,strlen(str),texttmprP);
			p_str = texttmprP;
	   	}
		else if(g_stSegLcdGUI.fonttype == 1)
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
		else if(g_stSegLcdGUI.fonttype == 2)
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
			if(*CurpixelX + char_width > SEGLCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
				sysLOG(SEGLCD_LOG_LEVEL_5, "*CurpixelX=%d, *CurpixelY=%d\r\n", *CurpixelX, *CurpixelY);
			}
			if(*CurpixelY + line_height > SEGLCD_PIXHIGH){
				
				if(g_stSegLcdGUI.AutoRefresh == 1)
				{
					hal_segRefresh();
				}
				goto exit; //超出屏幕底行返回不显示
			}
		}
		else if(encode_width==1)
		{
			if(*CurpixelX + char_width > SEGLCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
				sysLOG(SEGLCD_LOG_LEVEL_5, "*CurpixelX=%d, *CurpixelY=%d\r\n", *CurpixelX, *CurpixelY);
			}
			if(*CurpixelY + line_height > SEGLCD_PIXHIGH){
				
				if(g_stSegLcdGUI.AutoRefresh == 1)
				{
					hal_segRefresh();
				}
				goto exit;
			}
		}
		
		/* Use DrawBitBmp to display a char(*str). */	
        hal_segDot2DispBuf(*CurpixelX,*CurpixelY,char_width,char_height,dot_buf,encode_width);
        *CurpixelX += char_width*1;
    }while(strtmp);

    if(g_stSegLcdGUI.AutoRefresh == 1)
	{
        hal_segRefresh();	
    }

exit:

	if(texttmprP != NULL)
		free(texttmprP);
	
	return;
}


/*
*Function:		hal_segSetAttrib
*Description:	设置屏幕参数，
*Input:			attr:具体参考LCD_ATTRIBUTE_t
*				value:视attr参数而定，
*				LCD_ATTR_POWER	value:0-关闭，1-打开
*				LCD_ATTR_TEXT_INVERT	value:0-正显, 1-反显
*				LCD_ATTR_BACK_COLOR		value:0-1
*				LCD_ATTR_FRONT_COLOR	value:0-1
*				LCD_ATTR_AUTO_UPDATE	value:0-关闭自动刷新屏幕, 1-开启自动刷新，默认情况下是自动刷新
*				LCD_ATTR_ICON_BACK_COLOR	value:0-1
*				LCD_ATTR_LIGHT_LEVEL	value:0-100
*				LCD_ATTR_FONTTYPE
*Output:		NULL
*Hardware:
*Return:		0-设置成功, <0-失败
*Others:
*/
int hal_segSetAttrib(LCD_ATTRIBUTE_t attr, int value)
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

			g_stSegLcdGUI.Mode = (unsigned short int)value;
			iRet = 0;
		break;
   		case LCD_ATTR_BACK_COLOR://设置黑白屏背景色，支持0-3,4个色阶
   			if(value < 0 || value > 3)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stSegLcdGUI.grapBackColor = (unsigned short)value;
			iRet = 0;
		break;
    	case LCD_ATTR_FRONT_COLOR://设置黑白屏前景色，支持0-3,4个色阶
    		if(value < 0 || value > 3)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stSegLcdGUI.grapFrontColor = (unsigned short)value;
			iRet = 0;
		break;
    	case LCD_ATTR_AUTO_UPDATE://此项控制是否自动刷新RAM中的内容到屏幕。
    		if(value < 0 || value > 1)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stSegLcdGUI.AutoRefresh = (unsigned char)value;
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
			g_stSegLcdGUI.BackLight = (unsigned char)value;
			hal_scrSetBackLightValue((unsigned char)value);
			iRet = 0;
		break;
		case LCD_ATTR_FONTTYPE:
			if(value < 0 || value > 2)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stSegLcdGUI.fonttype = (unsigned char)value;
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
*Function:		hal_segGetAttrib
*Description:	获取液晶显示器的功能属性
*Input:			attr:LCD_ATTRIBUTE_t
*Output:		NULL
*Hardware:
*Return:		<0-失败，>=0-成功，具体值为所读取的功能属性value
*Others:
*/
int hal_segGetAttrib(LCD_ATTRIBUTE_t attr)
{

	int iRet = -1;
	switch(attr)
	{
		case LCD_ATTR_POWER:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_TEXT_INVERT:
			iRet = g_stSegLcdGUI.Mode;
		break;
		case LCD_ATTR_BACK_COLOR:
			iRet = g_stSegLcdGUI.grapBackColor;
		break;
		case LCD_ATTR_FRONT_COLOR:
			iRet = g_stSegLcdGUI.grapFrontColor;
		break;
		case LCD_ATTR_AUTO_UPDATE:
			iRet = g_stSegLcdGUI.AutoRefresh;
		break;
		case LCD_ATTR_ICON_BACK_COLOR:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_LIGHT_LEVEL:
			iRet = g_stSegLcdGUI.BackLight;
		break;
		case LCD_ATTR_FONTTYPE:
			iRet = g_stSegLcdGUI.fonttype;
		break;
		case LCD_ATTR_ICONHEIGHT:
			iRet = 0;
		break;
		case LCD_ATTR_WIDTH:
			iRet = SEGLCD_PIXWIDTH;
		break;
		case LCD_ATTR_HEIGHT:
			iRet = SEGLCD_PIXHIGH;
		break;
		case LCD_ATTR_USERWIDTH:
			iRet = SEGLCD_PIXWIDTH;
		break;
		case LCD_ATTR_USERHEIGHT:
			iRet = SEGLCD_PIXHIGH-0;
		break;
		case LCD_ATTR_LCDCOLORVALUE:
			iRet = 1;
		break;
		case LCD_ATTR_BACKLIGHT_MODE:
			iRet = g_stSegLcdGUI.BackLightMode;
		break;
		default:
			iRet = SCR_NOT_SUPPORT;
		break;
	
	}
	return iRet;

}


/*
*Function:		hal_segGotoxyPix
*Description:	定位LCD显示光标，参数超出LCD范围时不改变原坐标位置
*Input:			pixel_X:横坐标；pixel_Y:纵坐标
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_segGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y)
{
	if(pixel_X < 0 || pixel_X >=SEGLCD_PIXWIDTH || pixel_Y < 0 || pixel_Y >= SEGLCD_PIXHIGH)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}

	g_stSegLcdCurPixel.x = pixel_X;
	g_stSegLcdCurPixel.y = pixel_Y;
	return 0;
}


/*
*Function:		hal_segGetxyPix
*Description:	读取LCD上光标的当前位置。
*Input:			NULL
*Output:		*pixel_X:横坐标; *pixel_Y:纵坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segGetxyPix(int *pixel_X, int *pixel_Y)
{
	
	*pixel_X = g_stSegLcdCurPixel.x;
	*pixel_Y = g_stSegLcdCurPixel.y;

}


/*
*Function:		hal_segGetPixel
*Description:	获取特定坐标点的颜色值
*Input:			x:x坐标0-127；y:y坐标0-95；
*Output:		*picolor:颜色值
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_segGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor)
{
	uint pixcolor = 0;
	if(x < 0 || x >=SEGLCD_PIXWIDTH || y < 0 || y >= SEGLCD_PIXHIGH)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	
	pixcolor |= (*(g_stSegLcdGUI.DispBuff+y/SEGLCD_BLOCKHIGH*SEGLCD_BLOCKWIDTH+x) & 1<<((y%SEGLCD_BLOCKHIGH)*2+0))>>((y%SEGLCD_BLOCKHIGH)*2);
	pixcolor |= (*(g_stSegLcdGUI.DispBuff+y/SEGLCD_BLOCKHIGH*SEGLCD_BLOCKWIDTH+x) & 1<<((y%SEGLCD_BLOCKHIGH)*2+1))>>((y%SEGLCD_BLOCKHIGH)*2);
	*picolor = pixcolor;
	
	return 0;
}


/*
*Function:		hal_segGetLcdSize
*Description:	读取LCD显示区域大小
*Input:			NULL
*Output:		*width:LCD显示宽度；*height:LCD显示高度
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segGetLcdSize(int *width, int *height)
{
	*width = SEGLCD_PIXWIDTH;
	*height = SEGLCD_PIXHIGH;

}


/*
*Function:		hal_segPrintf
*Description:	在屏幕的前景层当前位置格式化显示字符串
*Input:			*fmt:显示输出的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segPrintf(const char *fmt, ...)
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
	hal_segTextOut(g_stSegLcdCurPixel.x, g_stSegLcdCurPixel.y, bufrP, &g_stSegLcdCurPixel.x, &g_stSegLcdCurPixel.y, 0);

exit:

	if(bufrP != NULL)
		free(bufrP);

	return;


}


/*
*Function:		hal_segPrint
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
void hal_segPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...)
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

	if(coltmp >= SEGLCD_PIXWIDTH)
	{
		coltmp = 0;
		row += 1;
	}

	if(row >= SEGLCD_PIXHIGH/SEGLCD_TEXTPIXEL)
		return;

	unsigned char *rP1 = NULL;
	unsigned char offsetenter = 0;
	
	disMode = g_stSegLcdGUI.Mode;
	dismodetmp = (mode&0x80) >> 7;//0-正显，1-反显
	aligntmp = (mode&0x06) >> 1;//0-左对齐，1-居中，2-右对齐
	
	sysLOG(SEGLCD_LOG_LEVEL_5, "mode=%x,dismodetmp=%d,aligntmp=%d\r\n", mode, dismodetmp, aligntmp);

	va_list args;
	
	memset(bufrP, 0, bufSIZE);
	va_start(args, str);
	vsnprintf(bufrP, bufSIZE-4,str, args);
	va_end(args);

	sysLOG(SEGLCD_LOG_LEVEL_5, "start fonttype\r\n");

	if(g_stSegLcdGUI.fonttype == 0)
   	{
    	int strlenth = strUTF8tostrUnicode(bufrP,strlen(bufrP),texttmprP);
		p_str = texttmprP;
   	}
	else if(g_stSegLcdGUI.fonttype == 1)
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
	else if(g_stSegLcdGUI.fonttype == 2)
	{
		p_str = bufrP;
	}

	sysLOG(SEGLCD_LOG_LEVEL_5, "end fonttype, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", *p_str,*(p_str+1),*(p_str+2),*(p_str+3),*(p_str+4),*(p_str+5),*(p_str+6),*(p_str+7),*(p_str+8),*(p_str+9));

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
			sysLOG(SEGLCD_LOG_LEVEL_5, "sumlinenum=%d,rP=%x,&bufrP+sumlinenum=%x,oncenum=%d\r\n", sumlinenum, rP, bufrP+sumlinenum, oncenum);
			
		}
		else
		{
			oncenum = Getlenstr(p_str)-sumlinenum;
			sysLOG(SEGLCD_LOG_LEVEL_5, "2 Getlenstr(p_str)=%d,sumlinenum=%d\r\n", Getlenstr(p_str), sumlinenum);
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
		}
		memset(oncebufrP, 0, oncebufSIZE);
		memcpy(oncebufrP, p_str+sumlinenum, oncenum);//copy从已经显示内容之后到下一次\r\n为止，然后在逐行显示
		sumlinenum += oncenum;
		sysLOG(SEGLCD_LOG_LEVEL_5, "2 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenum = CalTextline(coltmp, oncebufrP, oncenum, aligntmp, startlinepix, startstrlen, SEGLCD_PIXWIDTH);//oncenum/((COLORLCD_PIXWIDTH-coltmp)/(textpixeltmp/2));		
		sysLOG(SEGLCD_LOG_LEVEL_5, "1 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
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

			ytmp = row*SEGLCD_TEXTPIXEL+sumline*textpixeltmp;
			sysLOG(SEGLCD_LOG_LEVEL_5, "xtmp=%d, ytmp=%d, aligntmp=%d, linenumfinal=%d,onelinebufrP:%d,%d,%d,%d\r\n", xtmp, ytmp, aligntmp, linenumfinal, *onelinebufrP, *(onelinebufrP+1), *(onelinebufrP+2), *(onelinebufrP+3));
			g_stSegLcdGUI.Mode = dismodetmp;
			hal_segTextOut(xtmp, ytmp, onelinebufrP, &g_stSegLcdCurPixel.x, &g_stSegLcdCurPixel.y, 1);
			g_stSegLcdGUI.Mode = disMode;
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
*Function:		hal_segPrintxy
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
void hal_segPrintxy(uint col, uint row, unsigned char mode, const char *str, ...)
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

	if(coltmp >= SEGLCD_PIXWIDTH)
	{
		coltmp = 0;
		row += SEGLCD_TEXTPIXEL;
	}

	if(row >= SEGLCD_PIXHIGH)
		return;


	unsigned char *rP1 = NULL;
	unsigned char offsetenter = 0;
	
	disMode = g_stSegLcdGUI.Mode;
	dismodetmp = (mode&0x80) >> 7;//0-正显，1-反显
	aligntmp = (mode&0x06) >> 1;//0-左对齐，1-居中，2-右对齐
	
	sysLOG(SEGLCD_LOG_LEVEL_5, "mode=%x,dismodetmp=%d,aligntmp=%d\r\n", mode, dismodetmp, aligntmp);

	va_list args;
	
	memset(bufrP, 0, bufSIZE);
	va_start(args, str);
	vsnprintf(bufrP, bufSIZE-4,str, args);
	va_end(args);

	sysLOG(SEGLCD_LOG_LEVEL_5, "start fonttype\r\n");

	if(g_stSegLcdGUI.fonttype == 0)
   	{
    	int strlenth = strUTF8tostrUnicode(bufrP,strlen(bufrP),texttmprP);
		p_str = texttmprP;
   	}
	else if(g_stSegLcdGUI.fonttype == 1)
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
	else if(g_stSegLcdGUI.fonttype == 2)
	{
		p_str = bufrP;
	}

	sysLOG(SEGLCD_LOG_LEVEL_5, "end fonttype, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", *p_str,*(p_str+1),*(p_str+2),*(p_str+3),*(p_str+4),*(p_str+5),*(p_str+6),*(p_str+7),*(p_str+8),*(p_str+9));

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
			sysLOG(SEGLCD_LOG_LEVEL_5, "sumlinenum=%d,rP=%x,&bufrP+sumlinenum=%x,oncenum=%d\r\n", sumlinenum, rP, bufrP+sumlinenum, oncenum);
			
		}
		else
		{
			oncenum = Getlenstr(p_str)-sumlinenum;
			sysLOG(SEGLCD_LOG_LEVEL_5, "2 Getlenstr(p_str)=%d,sumlinenum=%d\r\n", Getlenstr(p_str), sumlinenum);
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
		}
		memset(oncebufrP, 0, oncebufSIZE);
		memcpy(oncebufrP, p_str+sumlinenum, oncenum);//copy从已经显示内容之后到下一次\r\n为止，然后在逐行显示
		sumlinenum += oncenum;
		sysLOG(SEGLCD_LOG_LEVEL_5, "2 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenum = CalTextline(coltmp, oncebufrP, oncenum, aligntmp, startlinepix, startstrlen, SEGLCD_PIXWIDTH);//oncenum/((COLORLCD_PIXWIDTH-coltmp)/(textpixeltmp/2));		
		sysLOG(SEGLCD_LOG_LEVEL_5, "1 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
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
			sysLOG(SEGLCD_LOG_LEVEL_5, "xtmp=%d, ytmp=%d, aligntmp=%d, linenumfinal=%d,onelinebufrP:%d,%d,%d,%d\r\n", xtmp, ytmp, aligntmp, linenumfinal, *onelinebufrP, *(onelinebufrP+1), *(onelinebufrP+2), *(onelinebufrP+3));
			g_stSegLcdGUI.Mode = dismodetmp;
			hal_segTextOut(xtmp, ytmp, onelinebufrP, &g_stSegLcdCurPixel.x, &g_stSegLcdCurPixel.y, 1);
			g_stSegLcdGUI.Mode = disMode;
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
*Function:		hal_segContrastSet
*Description:	设置对比度
*Input:			value:0-63
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int8 hal_segContrastSet(uint8 value)
{
	if(value < 0 || value > 63)
	{
		return -1;
	}

	hal_segWriteCmd(0x81);//Set Contrast
	hal_segWriteCmd(value*4);//Contrast Value:0~255
	
	g_ui8SegLcdContrastValue = value;
		
		
	return 0;
}


/*
*Function:		hal_segContrastGet
*Description:	读取对比度
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0-对比度值；<0-失败
*Others:
*/
int8 hal_segContrastGet(void)
{

	return g_ui8SegLcdContrastValue;
}

/*
*Function:		hal_segBackLightCtl
*Description:	LCD 背光控制
*Input:			value: 0-255
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_segBackLightCtl(uint8 value)
{
	int iRet = -1;
	
	if(value < 0 || value > 255)
	{
		return iRet;
	}

	fibo_SetPwlLevel(1, value);

	iRet = 0;

	return iRet;
}

/*
*Function:		hal_segSetBackLightValue
*Description:	设置屏幕亮度大小
*Input:			ucValue:亮度值，0~100；
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_segSetBackLightValue(unsigned char ucValue)
{
	int iRet = -1;
	float valuetmp;
	
	g_stSegLcdGUI.BackLight = ucValue;
	valuetmp = ucValue*2.55;
	if(g_stSegLcdGUI.BackLightEN == 1)
	{
		iRet = hal_segBackLightCtl((uint8) valuetmp);
	}
	
	sysLOG(SCR_LOG_LEVEL_3, "iRet:%d,valuetmp:%d,%f,ucValue=%d\r\n", iRet, (uint8) valuetmp, valuetmp, ucValue);
}



/*
*Function:		hal_cblcdOpen
*Description:	开启断码屏显示
*Input:			*backlight:0~100
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_segOpen(int backlight)
{

	if(1 == g_stSegLcdGUI.status)
		return 0;

	hal_segCls();
	hal_segWriteCmd(0xAF);//Display on and power on mode.
	
	if(backlight >=0 && backlight <= 100)
		hal_segSetBackLightValue(backlight);
	else
		hal_segSetBackLightValue(g_stSegLcdGUI.BackLight);

	g_stSegLcdGUI.status = 1;
	return 0;
}


/*
*Function:		hal_cblcdClose
*Description:	关闭断码屏显示
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_segClose(void)
{
	if(0 == g_stSegLcdGUI.status)
		return 0;
	
	hal_segWriteCmd(0xAE);//Display on and power on mode.
	hal_segBackLightCtl(0);
	g_stSegLcdGUI.status = 0;
}




void hal_segInit(void)
{

	sysLOG(SCR_LOG_LEVEL_3, "START\r\n");
	
	hal_scrMutexCS(SCREEN_LOCK, SCREEN_SECOND);

	fibo_gpio_mode_set(LCDSPICSN_GPIO, GpioFunction0);
	fibo_gpio_cfg(LCDSPICSN_GPIO, GpioCfgOut);
	fibo_gpio_set(LCDSPICSN_GPIO, TRUE);
	
	fibo_gpio_mode_set(SEGLCDSPICSN_GPIO, GpioFunction0);//SEG CS初始化
	fibo_gpio_cfg(SEGLCDSPICSN_GPIO, GpioCfgOut);
	fibo_gpio_set(SEGLCDSPICSN_GPIO, TRUE);

	fibo_gpio_mode_set(SEGLCD_BLEN_GPIO, GpioFunction3);
	fibo_SetPwlLevel(1, 0);

   	hal_segWriteCmd(0xE2);//soft reset
	sysDelayMs(5);  	     
	hal_segWriteCmd(0x2F);//set pump control

	hal_segCls();
   	
	hal_segWriteCmd(0x60);//set scroll line 指定DDRAM第一显示行的行地址为32
	//hal_segWriteCmd(0x40);//Set Scroll Line

//	hal_segWriteCmd(0x81);//Set Contrast
//	hal_segWriteCmd(0x60);//Contrast Value:0~255

    hal_segWriteCmd(0x84);//Full display mode.
    hal_segWriteCmd(0x85);//Partial screen mode.

 	hal_segWriteCmd(0x88);//设置横向刷屏方向

    hal_segWriteCmd(0xA0);//Set Frame Rate 76
   	hal_segWriteCmd(0xA4);//设置正常显示模式
	hal_segWriteCmd(0xA6);//Normal display mode
	

	hal_segWriteCmd(0xC4);// scan direction: Col0->Col127; Com0->Com63
	hal_segWriteCmd(0xE9);// Set Bias to 1/7
	
	hal_segWriteCmd(0xF1);//Set COM End    	
	hal_segWriteCmd(0x1F);//Set COM End:0x1F;//32com
	
	hal_segWriteCmd(0xF2);//Set Partial Start Address    	
	hal_segWriteCmd(0x20);//Set Partial Start Address:0x20
	
	hal_segWriteCmd(0xF3);//Set Partial End Address    	
	hal_segWriteCmd(0x3F);//Set Partial End Address:0x3F
    		   	
	hal_segWriteCmd(0xAE);//Display on and power on mode.

	hal_segContrastSet(30);
	

	g_stSegLcdGUI.Mode = 0;//正常显示
	g_stSegLcdGUI.AutoRefresh = 1;//自动刷新
	g_stSegLcdGUI.grapBackColor = 0;
	g_stSegLcdGUI.grapFrontColor = 1;
	g_stSegLcdGUI.BackLight = 0;
	g_stSegLcdGUI.BackLightEN = 1;
	g_stSegLcdGUI.BackLightMode = 1;
	g_stSegLcdGUI.BackLightTimeout = SECSCR_BACKLIGHTTIMEOUT;
	g_stSegLcdGUI.icon_attr.iconAreaColor = 0;
	g_stSegLcdGUI.icon_attr.iconColor = 1;
	g_stSegLcdGUI.fonttype = 0;
	g_stSegLcdGUI.rotate = 0;
	g_stSegLcdGUI.status = 0;
	g_stSegLcdCurPixel.x = 0;
	g_stSegLcdCurPixel.y = 0;

	g_stSegLcdGUI.rotateBuff = malloc(SEGLCD_SUMPIXNUM/SEGLCD_BLOCKHIGH);
	memset(g_stSegLcdGUI.rotateBuff, 0x0, SEGLCD_SUMPIXNUM/SEGLCD_BLOCKHIGH);

	g_stSegLcdGUI.RefreshFlag = malloc(g_stLcdConfig.LCD_BLOCKBUFNUM);
	g_stSegLcdGUI.DispBuff = malloc(SEGLCD_SUMPIXNUM/SEGLCD_BLOCKHIGH);
	memset(g_stSegLcdGUI.DispBuff, 0x0, SEGLCD_SUMPIXNUM/SEGLCD_BLOCKHIGH);
	memset(g_stSegLcdGUI.RefreshFlag, 0, sizeof(g_stSegLcdGUI.RefreshFlag));

	hal_segOpen(60);
	hal_segWriteBitMap(0, 0, gImage_New_Aisino_128x32);
	sysDelayMs(1000);
	hal_segClsArea(SEGLCD_X_PIXELMIN, SEGLCD_Y_PIXELMIN, SEGLCD_X_PIXELMAX, SEGLCD_Y_PIXELMAX);
	hal_segClose();

	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_SECOND);

}


