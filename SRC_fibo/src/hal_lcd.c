/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_lcd.c
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


#include "comm.h"

/*


*屏幕128*96，划分成16个page(96*8)，一个page纵向包含8个像素，
*在一页中，刷屏方向为一次刷纵向8个点（占2个字节），刷完一列竖向8个点后，列地址递增，刷第二个列竖向8个点，以此类推。
*
*
*/

uint8 g_ui8ContrastValue = 36;
struct _LCD_CONFIG g_stLcdConfig;
LCD_GUI	g_stLcdGUI;


/***********************************刷缓存方式***************************************************/



/*
*Function:		hal_lcdWrite_uc1617s
*Description:	黑白屏刷屏，按块block刷屏
*Input:			col: 列地址
*				blockindex:block索引
*				*dat:需要刷新的内容指针
*				datLen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdWrite_uc1617s(unsigned int col, unsigned int blockindex, unsigned char *dat, unsigned int datLen)
{
	if (g_stLcdConfig.LCD_DIRECTION == 0)
	{
		int32 i = 0, j = 0, k = 0;
		uint8 rowtmp = 0;
		uint8 dattmp[128];
		uint8 clrtmp = 0;
		uint8 dd1 = 0;
		clrtmp = 0x11;

		memset(dattmp, 0, sizeof(dattmp));

		for(j=0; j<g_stLcdConfig.LCD_PIXWIDTH; j++)//从屏幕中一page_C的左上开始，即实际行地址从0x7F到0x00
		{
			rowtmp = g_stLcdConfig.LCD_PIXWIDTH-1-j;
			for(k=0; k<g_stLcdConfig.LCD_BLOCKHIGH; k++)//实际竖屏中一行的4个像素
			{
				dd1 = 0;
				dd1 |= (*(dat+j)&(1<<(k*2+0)));
				dd1 |= (*(dat+j)&(1<<(k*2+1)));

				if(LCD_GRAYMODE == 0)
				{
					dd1 = dd1 >> (k*2);
					if(dd1 == 0b01)dd1 = 0b00;
					else if(dd1 == 0b10)dd1 = 0b00;
					dd1 = dd1 << (k*2);
				}
				
				dattmp[rowtmp] |= dd1;

			}
		}
		
		fibo_dotMatrixLcdSendData(blockindex, 0, dattmp, g_stLcdConfig.LCD_PIXWIDTH);  
	}
	else if (g_stLcdConfig.LCD_DIRECTION == 1)
	{
		int32 i = 0, j = 0, k = 0;
		uint8 pageindextmp = 0;
		uint8 rowtmp = 0;
		uint8 dattmp[128];
		uint8 clrtmp = 0;
		uint8 dd1 = 0;
		clrtmp = 0x11;

		for(i=0; i<(LCD_PAGEMAX_UC1617S+1); i++)//总共24个page_C
		{
			pageindextmp = LCD_PAGEMAX_UC1617S-i;
			memset(dattmp, 0, sizeof(dattmp));

			for(j=g_stLcdConfig.LCD_BLOCKHIGH-1; j>=0; j--)//刷其中一个page_C中的4 Line，也就是实际刷屏的4行。
			{
				
				for(k=LCD_PAGEHIGH_UC1617S-1; k>=0; k--)
				{
					dd1 = 0;
					dd1 |= ( *(dat+(i*LCD_PAGEHIGH_UC1617S)+(k))&(0b11<<(j*2)))>>(j*2);

					if(LCD_GRAYMODE == 0)
					{			
						if(dd1 == 0b01)dd1 = 0b00;
						else if(dd1 == 0b10)dd1 = 0b00;			
					}
					
					switch(dd1){
						case 0:
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
						break;
						case 1:
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
						break;
						case 2:
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
						break;
						case 3:
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
							dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
						break;
					}
				}
				
			}
			fibo_dotMatrixLcdSendData(pageindextmp, (g_stLcdConfig.LCD_BLOCKBUFNUM-1-blockindex)*g_stLcdConfig.LCD_BLOCKHIGH, dattmp, g_stLcdConfig.LCD_BLOCKHIGH);   
		}
	}

}


/*
*Function:		hal_lcdWrite_st7571
*Description:	黑白屏刷屏，按块block刷屏
*Input:			col: 列地址
*				blockindex:block索引
*				*dat:需要刷新的内容指针
*				datLen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdWrite_st7571(unsigned int col, unsigned int blockindex, unsigned char *dat, unsigned int datLen)
{
	if (g_stLcdConfig.LCD_DIRECTION==0)
	{
		int8 i = 0, j = 0, k = 0;
		uint8 pageindextmp = 0;
		uint8 dattmp[16];
		uint8 clrtmp = 0;
		uint8 dd1 = 0;
		clrtmp = 0x11;
		
		for(i=0; i<16; i++)//屏幕宽像素为128,实际刷屏是竖屏，总共128/8=16page.
		{
			pageindextmp = LCD_PAGEMAX - i;

			memset(dattmp, 0, sizeof(dattmp));
			for(j=0; j<g_stLcdConfig.LCD_BLOCKHIGH; j++)//j表示的是实际刷屏的一纵列指针偏移量，一纵列8个点需要2个字节表示,j同时也表示了*dat中的bit位
			{
				for(k=7; k>=0; k--)//k表示的是实际刷屏一纵列中的bit位，从bit7开始到bit0,同时(7-k)也表示了在*dat中的指针
				{
					dd1 = 0;
					dd1 |= ((*(dat+(i*LCD_PAGEHIGH)+(7-k))&(1<<(j*2+0)))>>(j*2));
					dd1 |= ((*(dat+(i*LCD_PAGEHIGH)+(7-k))&(1<<(j*2+1)))>>(j*2));

					if(LCD_GRAYMODE == 0)
					{			
						if(dd1 == 0b01)dd1 = 0b00;
						else if(dd1 == 0b10)dd1 = 0b00;			
					}
					
					switch(dd1){
						case 0:
							dattmp[j*2+0] &= ~(1<<k);
							dattmp[j*2+1] &= ~(1<<k);
						break;
						case 1:
							dattmp[j*2+0] &= ~(1<<k);
							dattmp[j*2+1] |= (1<<k);
						break;
						case 2:
							dattmp[j*2+0] |= (1<<k);
							dattmp[j*2+1] &= ~(1<<k);
						break;
						case 3:
							dattmp[j*2+0] |= (1<<k);
							dattmp[j*2+1] |= (1<<k);
						break;
					}
				}
				sysLOG(LCD_LOG_LEVEL_5, "i=%d, j=%d, dd1:%d, dattmp[j*2+0]:%d\r\n", i, j, dd1, dattmp[j*2+0]);
				
			}
			
			fibo_dotMatrixLcdSendData(blockindex*g_stLcdConfig.LCD_BLOCKHIGH, pageindextmp, dattmp, g_stLcdConfig.LCD_BLOCKHIGH*2);
			
		}
	}
	else if (g_stLcdConfig.LCD_DIRECTION == 1)
	{
		int8 i = 0, j = 0, k = 0;
		uint8 pageindextmp = 0;
		uint8 blockindexother = 0;
		uint8 *datP = 0;
		uint8 dattmp[256];
		uint8 dd1 = 0;
		uint8 coltmp = 0;
		
		
		pageindextmp = LCD_PAGEMAX - (blockindex/2);
		blockindexother = (blockindex%2 == 0) ? (blockindex+1) : (blockindex-1);//一个page占8行也就是2个block,blockindexother是同一page中的另一个blockindex

		memset(dattmp, 0, sizeof(dattmp));
		if(blockindex%2 == 0)
		{
			datP = dat;
		}
		else if(blockindex%2 == 1)
		{
			datP = dat-g_stLcdConfig.LCD_BLOCKWIDTH;
		}
		
		for(j=0; j<g_stLcdConfig.LCD_PIXWIDTH; j++)//从屏幕左上开始，即实际行地址从95到0
		{
			coltmp = g_stLcdConfig.LCD_PIXWIDTH-1-j;
			for(k=0; k<g_stLcdConfig.LCD_BLOCKHIGH; k++)//第一个block,实际竖屏中一page中竖向8个像素点,每次刷新一个page,也就是2个block
			{
				dd1 = 0;
				dd1 |= (*(datP+g_stLcdConfig.LCD_PIXWIDTH+j)&(1<<(k*2+0)))>>(k*2);
				dd1 |= (*(datP+g_stLcdConfig.LCD_PIXWIDTH+j)&(1<<(k*2+1)))>>(k*2);

				if(LCD_GRAYMODE == 0)
				{
					if(dd1 == 0b01)dd1 = 0b00;
					else if(dd1 == 0b10)dd1 = 0b00;
				}
				
				switch(dd1){
					case 0:
						dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
						dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					break;
					case 1:
						dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
						dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					break;
					case 2:
						dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
						dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					break;
					case 3:
						dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
						dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					break;
				}

			}

			for(k=0; k<g_stLcdConfig.LCD_BLOCKHIGH; k++)//第二个block,实际竖屏中一page中竖向8个像素点,每次刷新一个page,也就是2个block
			{
				dd1 = 0;
				dd1 |= (*(datP+j)&(1<<(k*2+0)))>>(k*2);
				dd1 |= (*(datP+j)&(1<<(k*2+1)))>>(k*2);

				if(LCD_GRAYMODE == 0)
				{
					if(dd1 == 0b01)dd1 = 0b00;
					else if(dd1 == 0b10)dd1 = 0b00;
				}
				
				switch(dd1){
					case 0:
						dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
						dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					break;
					case 1:
						dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
						dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					break;
					case 2:
						dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
						dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					break;
					case 3:
						dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
						dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					break;
				}

			}
		}
		
		fibo_dotMatrixLcdSendData(0, pageindextmp, dattmp, g_stLcdConfig.LCD_PIXWIDTH*2);

	}

}

#if 0

/*
*Function:		hal_lcdWrite_uc1617s
*Description:	黑白屏刷屏，按块block刷屏
*Input:			col: 列地址
*				blockindex:block索引
*				*dat:需要刷新的内容指针
*				datLen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdWrite_uc1617s(unsigned int col, unsigned int blockindex, unsigned char *dat, unsigned int datLen)
{
	int32 i = 0, j = 0, k = 0;
	uint8 pageindextmp = 0;
	uint8 rowtmp = 0;
	uint8 dattmp[128];
	uint8 clrtmp = 0;
	uint8 dd1 = 0;
	clrtmp = 0x11;

	for(i=0; i<(LCD_PAGEMAX_UC1617S+1); i++)//总共24个page_C
	{
		pageindextmp = LCD_PAGEMAX_UC1617S-i;
		memset(dattmp, 0, sizeof(dattmp));

		for(j=g_stLcdConfig.LCD_BLOCKHIGH-1; j>=0; j--)//刷其中一个page_C中的4 Line，也就是实际刷屏的4行。
		{
			
			for(k=LCD_PAGEHIGH_UC1617S-1; k>=0; k--)
			{
				dd1 = 0;
				dd1 |= ( *(dat+(i*LCD_PAGEHIGH_UC1617S)+(k))&(0b11<<(j*2)))>>(j*2);

				if(LCD_GRAYMODE == 0)
				{			
					if(dd1 == 0b01)dd1 = 0b00;
					else if(dd1 == 0b10)dd1 = 0b00;			
				}
				
				switch(dd1){
					case 0:
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
					break;
					case 1:
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
					break;
					case 2:
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] &= ~(1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
					break;
					case 3:
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2));
						dattmp[(g_stLcdConfig.LCD_BLOCKHIGH-1-j)] |= (1<<((LCD_PAGEHIGH_UC1617S-1-k)*2+1));
					break;
				}
			}
			
		}
		fibo_dotMatrixLcdSendData(pageindextmp, (g_stLcdConfig.LCD_BLOCKBUFNUM-1-blockindex)*g_stLcdConfig.LCD_BLOCKHIGH, dattmp, g_stLcdConfig.LCD_BLOCKHIGH);   
	}
   
}

/*
*Function:		hal_lcdWrite_st7571
*Description:	黑白屏刷屏，按块block刷屏
*Input:			col: 列地址
*				blockindex:block索引
*				*dat:需要刷新的内容指针
*				datLen:数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdWrite_st7571(unsigned int col, unsigned int blockindex, unsigned char *dat, unsigned int datLen)
{
	int8 i = 0, j = 0, k = 0;
	uint8 pageindextmp = 0;
	uint8 blockindexother = 0;
	uint8 *datP = 0;
	uint8 dattmp[256];
	uint8 dd1 = 0;
	uint8 coltmp = 0;
	
	
	pageindextmp = LCD_PAGEMAX - (blockindex/2);
	blockindexother = (blockindex%2 == 0) ? (blockindex+1) : (blockindex-1);//一个page占8行也就是2个block,blockindexother是同一page中的另一个blockindex

	memset(dattmp, 0, sizeof(dattmp));
	if(blockindex%2 == 0)
	{
		datP = dat;
	}
	else if(blockindex%2 == 1)
	{
		datP = dat-g_stLcdConfig.LCD_BLOCKWIDTH;
	}
	
	for(j=0; j<g_stLcdConfig.LCD_PIXWIDTH; j++)//从屏幕左上开始，即实际行地址从95到0
	{
		coltmp = g_stLcdConfig.LCD_PIXWIDTH-1-j;
		for(k=0; k<g_stLcdConfig.LCD_BLOCKHIGH; k++)//第一个block,实际竖屏中一page中竖向8个像素点,每次刷新一个page,也就是2个block
		{
			dd1 = 0;
			dd1 |= (*(datP+g_stLcdConfig.LCD_PIXWIDTH+j)&(1<<(k*2+0)))>>(k*2);
			dd1 |= (*(datP+g_stLcdConfig.LCD_PIXWIDTH+j)&(1<<(k*2+1)))>>(k*2);

			if(LCD_GRAYMODE == 0)
			{
				if(dd1 == 0b01)dd1 = 0b00;
				else if(dd1 == 0b10)dd1 = 0b00;
			}
			
			switch(dd1){
				case 0:
					dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
				break;
				case 1:
					dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
				break;
				case 2:
					dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
				break;
				case 3:
					dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
					dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH-1-k));
				break;
			}

		}

		for(k=0; k<g_stLcdConfig.LCD_BLOCKHIGH; k++)//第二个block,实际竖屏中一page中竖向8个像素点,每次刷新一个page,也就是2个block
		{
			dd1 = 0;
			dd1 |= (*(datP+j)&(1<<(k*2+0)))>>(k*2);
			dd1 |= (*(datP+j)&(1<<(k*2+1)))>>(k*2);

			if(LCD_GRAYMODE == 0)
			{
				if(dd1 == 0b01)dd1 = 0b00;
				else if(dd1 == 0b10)dd1 = 0b00;
			}
			
			switch(dd1){
				case 0:
					dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
				break;
				case 1:
					dattmp[coltmp*2+0] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
				break;
				case 2:
					dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					dattmp[coltmp*2+1] &= ~(1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
				break;
				case 3:
					dattmp[coltmp*2+0] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
					dattmp[coltmp*2+1] |= (1<<(g_stLcdConfig.LCD_BLOCKHIGH+(g_stLcdConfig.LCD_BLOCKHIGH-1-k)));
				break;
			}

		}
	}
	
	fibo_dotMatrixLcdSendData(0, pageindextmp, dattmp, g_stLcdConfig.LCD_PIXWIDTH*2);
	
	
   

   
}



#endif


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
*Function:		hal_lcdDotRotate
*Description:	屏幕缓存内容转向
*Input:			*dotbuffin-缓存输入指针; len-需要处理的长度
*Output:		*dotbuffout-缓存输出指针
*Hardware:
*Return:		成功处理的长度
*Others:
*/
static int hal_lcdDotRotate(unsigned char *dotbuffin, unsigned char *dotbuffout, int len)
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
*Function:		hal_lcdIconRefresh
*Description:	根据需要刷新图标区域
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdIconRefresh(void)//后续要修改分开刷新
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;


	for (i=0;i<3;i++)
	{
	    //如果有一个块需要刷新
		if(g_stLcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			if(g_stLcdGUI.rotate == 0)
			{
				j = i;
				disp = g_stLcdGUI.DispBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j;
			}
			else if(g_stLcdGUI.rotate == 1)
			{
				j = g_stLcdConfig.LCD_BLOCKBUFNUM - 1 - i;
				
				hal_lcdDotRotate(g_stLcdGUI.DispBuff+g_stLcdConfig.LCD_BLOCKWIDTH*i, g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j, g_stLcdConfig.LCD_BLOCKWIDTH);
				for(int i=0; i<g_stLcdConfig.LCD_BLOCKWIDTH; i++)
				{
					*(g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j+i) = hal_lcdCharrotate(*(g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j+i));
				}
				
				disp = g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j;
			}

			if(g_ui32LcdDevID == FYTLCD_2_4_ST7571_ID)
			{
				hal_lcdWrite_st7571(0, j, disp, g_stLcdConfig.LCD_BLOCKWIDTH);
			}
			else if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)
			{
				hal_lcdWrite_uc1617s(0, j, disp, g_stLcdConfig.LCD_BLOCKWIDTH);
			}
//			sysLOG(LCD_LOG_LEVEL_2, "i=%d, j=%d\r\n", i, j);
		}
	}

	g_stLcdGUI.RefreshFlag[0] &= (~0x07);
	

}


/*
*Function:		hal_lcdRefresh
*Description:	根据需要刷新整个屏幕,图标区域除外
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdRefresh(void)
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;

	unsigned char refreshflagtmp = 0;
	uint8 startblocknum = 3;
	
	refreshflagtmp = g_stLcdGUI.RefreshFlag[0] & 0x07;


	if(g_ui8FullScreen == 0)
		startblocknum = 3;
	else if(g_ui8FullScreen == 1)
		startblocknum = 0;

	for (i=startblocknum;i<g_stLcdConfig.LCD_BLOCKBUFNUM;i++)
	{
	    //如果有一个块需要刷新
		if(g_stLcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			if(g_stLcdGUI.rotate == 0)
			{
				j = i;
				disp = g_stLcdGUI.DispBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j;
			}
			else if(g_stLcdGUI.rotate == 1)
			{
				j = g_stLcdConfig.LCD_BLOCKBUFNUM - 1 - i;
				
				hal_lcdDotRotate(g_stLcdGUI.DispBuff+g_stLcdConfig.LCD_BLOCKWIDTH*i, g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j, g_stLcdConfig.LCD_BLOCKWIDTH);
				for(int i=0; i<g_stLcdConfig.LCD_BLOCKWIDTH; i++)
				{
					*(g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j+i) = hal_lcdCharrotate(*(g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j+i));
				}
				
				disp = g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j;
			}
			
			if(g_ui32LcdDevID == FYTLCD_2_4_ST7571_ID)
			{
				hal_lcdWrite_st7571(0, j, disp, g_stLcdConfig.LCD_BLOCKWIDTH);
			}
			else if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)
			{
				hal_lcdWrite_uc1617s(0, j, disp, g_stLcdConfig.LCD_BLOCKWIDTH);
			}
//			sysLOG(LCD_LOG_LEVEL_2, "i=%d, j=%d\r\n", i, j);
		}
	}

	memset(g_stLcdGUI.RefreshFlag,0,sizeof(g_stLcdGUI.RefreshFlag));
	g_stLcdGUI.RefreshFlag[0] = refreshflagtmp;

}


/*
*Function:		hal_lcdRefreshRotate
*Description:	根据需要刷新整个屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdRefreshRotate(void)
{
	unsigned short int i,j;

    unsigned char *disp;
    unsigned char ucColorBit = 0;

	unsigned char refreshflagtmp = 0;
	refreshflagtmp = g_stLcdGUI.RefreshFlag[0] & 0x07;


	for (i=0;i<g_stLcdConfig.LCD_BLOCKBUFNUM;i++)
	{
	    //如果有一个块需要刷新
		if(g_stLcdGUI.RefreshFlag[i/8] & (1<<(i%8)))	
		{
			if(g_stLcdGUI.rotate == 0)
			{
				j = i;
				disp = g_stLcdGUI.DispBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j;
			}
			else if(g_stLcdGUI.rotate == 1)
			{
				j = g_stLcdConfig.LCD_BLOCKBUFNUM - 1 - i;
				
				hal_lcdDotRotate(g_stLcdGUI.DispBuff+g_stLcdConfig.LCD_BLOCKWIDTH*i, g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j, g_stLcdConfig.LCD_BLOCKWIDTH);
				for(int i=0; i<g_stLcdConfig.LCD_BLOCKWIDTH; i++)
				{
					*(g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j+i) = hal_lcdCharrotate(*(g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j+i));
				}
				
				disp = g_stLcdGUI.rotateBuff+g_stLcdConfig.LCD_BLOCKWIDTH*j;
			}
			
			if(g_ui32LcdDevID == FYTLCD_2_4_ST7571_ID)
			{
				hal_lcdWrite_st7571(0, j, disp, g_stLcdConfig.LCD_BLOCKWIDTH);
			}
			else if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)
			{
				hal_lcdWrite_uc1617s(0, j, disp, g_stLcdConfig.LCD_BLOCKWIDTH);
			}
//			sysLOG(LCD_LOG_LEVEL_2, "i=%d, j=%d\r\n", i, j);
		}
	}

	memset(g_stLcdGUI.RefreshFlag,0,sizeof(g_stLcdGUI.RefreshFlag));
	g_stLcdGUI.RefreshFlag[0] = refreshflagtmp;

}


/*
*Function:		hal_lcdDrawSinglePix
*Description:	黑白屏画一个点，不包括刷新
*Input:			x:x坐标;y:y坐标;dotColor:像素值，支持0-3,四个色阶
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdDrawSinglePix(unsigned short int x,unsigned short int y,unsigned char dotColor)
{
    int blocknum = 0;
	
    blocknum = y/g_stLcdConfig.LCD_BLOCKHIGH;
	g_stLcdGUI.RefreshFlag[blocknum/8] |= (1<<(blocknum%8));// 1字节记录8个block 看是否更新
	switch (dotColor){
		
		case 0:
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) &= ~(1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+0));
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) &= ~(1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+1));
		break;
		case 1:
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) |= 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+0);
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) &= ~(1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+1));
		break;
		case 2:
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) &= ~(1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+0));
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) |= 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+1);
		break;
		case 3:
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) |= 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+0);
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) |= 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+1);
		break;
		default:
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) |= 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+0);
			*(g_stLcdGUI.DispBuff+blocknum*g_stLcdConfig.LCD_BLOCKWIDTH+x) |= 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+1);
		break;
	}


}


/*
*Function:		hal_lcdDrawDotC
*Description:	画点并以渲染值显示,不带刷新
*Input:			x-x坐标；y-y坐标；color:颜色值支持0-3
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_lcdDrawDotC(unsigned short int x, unsigned short int y, unsigned short color)
{   
	if((x >= g_stLcdConfig.LCD_PIXWIDTH) || (y >= g_stLcdConfig.LCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(color<0 || color > 3)
	{
		return LCD_ERR_PARAM_ERROR;
	}
         
	hal_lcdDrawSinglePix(x,y,color);
   
	return 0;

}


/*
*Function:		hal_lcdIconDrawDot
*Description:	图标中画点并以前景色显示,不包括刷新
*Input:			x-x坐标；y-y坐标；Mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_lcdIconDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode)
{   
	if((x >= g_stLcdConfig.LCD_PIXWIDTH) || (y >= g_stLcdConfig.LCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
    if(Mode == 1)
    {     
		hal_lcdDrawSinglePix(x,y,g_stLcdGUI.icon_attr.iconColor);
    }
    else
    {
        hal_lcdDrawSinglePix(x,y,g_stLcdGUI.icon_attr.iconAreaColor);
    }
	return 0;

}


/*
*Function:		hal_lcdDrawDot
*Description:	内容区画点并以前景色显示,不带刷新
*Input:			x-x坐标；y-y坐标；Mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_lcdDrawDot(unsigned short int x,unsigned short int y,unsigned char Mode)
{   
	if((x >= g_stLcdConfig.LCD_PIXWIDTH) || (y >= g_stLcdConfig.LCD_PIXHIGH))
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	if(Mode<0 || Mode > 1)
	{
		return LCD_ERR_PARAM_ERROR;
	}
    if(((g_stLcdGUI.Mode == 1) && (Mode == 0)) || ((g_stLcdGUI.Mode == 0) && (Mode == 1)))
    {     
		hal_lcdDrawSinglePix(x,y,g_stLcdGUI.grapFrontColor);
    }
    else
    {
        hal_lcdDrawSinglePix(x,y,g_stLcdGUI.grapBackColor);
    }
	return 0;

}


/*
*Function:		hal_lcdWriteBitData
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
void hal_lcdWriteBitData(int x, int y, unsigned char *dat, int width, int height)
{
    unsigned char ucBitPix;
    int i,j;
    int des_x = (x >= 0) ? x : 0;
    int src_x = (x >= 0) ? 0 : -x;
    int des_y = (y >= 0) ? y : 0;
    int src_y = (y >= 0) ? 0 : -y;
    int w = (x >= 0) ? width  : width + x;
    int h = (y >= 0) ? height : height + y;
    
    if((x >= g_stLcdConfig.LCD_PIXWIDTH) || (x+width <= 0) || (width <= 0))
		return;
	if((y >= g_stLcdConfig.LCD_PIXHIGH) || (y+height<= 0) || (height <= 0))
		return;
    
	for( i=0; i<h; i++)
	{
		for( j=0; j<w; j++)
		{
            ucBitPix = (dat[i*w/8 + j/8] & (1 <<(7-j%8)));
            if(ucBitPix)
            {
                hal_lcdDrawDotC(des_x+j, des_y+i, g_stLcdGUI.grapFrontColor);
            }
            else
            {
                hal_lcdDrawDotC(des_x+j, des_y+i, g_stLcdGUI.grapBackColor);
            }
		}
	}

}


/*
*Function:		hal_lcdDrawSingleLine
*Description:	画直线，不包括刷新
*Input:			x1-起始点x坐标;y1-起始点y坐标;x2-结束点x坐标;y2-结束点y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdDrawSingleLine(int x1,int y1,int x2,int y2, unsigned char mode)
{
    unsigned short int i,j;
    
	if(x1<0)	x1=0;
	if(x2<0)	x2=0;
	if(y1<0)	y1=0;
	if(y2<0)	y2=0;
		
    if(x1 > x2 || y1 > y2 || x1 < 0 || y1 < 0 || x2 >= g_stLcdConfig.LCD_PIXWIDTH || y2 >= g_stLcdConfig.LCD_PIXHIGH)
        return;

    if(y1 < y2)
    {
        for (i = y1; i <= y2; i++)       //画线输出
        {
            if(x1 < x2)
            {
                for(j = x1; j<= x2; j++)
                {
                    hal_lcdDrawDot(j, i,mode);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_lcdDrawDot(j, i,mode);
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
                    hal_lcdDrawDot(j, i,mode);
                }
            }
            else
            {
                for(j = x2; j<= x1; j++)
                {
                    hal_lcdDrawDot(j, i,mode);
                }
            }
        }
    }
	
}

/*
*Function:		hal_lcdDrawStraightLine
*Description:	画直线
*Input:			x1-起始点x坐标;y1-起始点y坐标;x2-结束点x坐标;y2-结束点y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdDrawStraightLine(int x1,int y1,int x2,int y2, unsigned char mode)
{
	hal_lcdDrawSingleLine(x1, y1, x2, y2, mode);
	if(g_stLcdGUI.AutoRefresh == 1)
	{
		hal_lcdRefresh();
	}
}


/*
*Function:		hal_lcdDrawRectangle
*Description:	画矩形框
*Input:			x1-左上角x坐标;y1-左上角y坐标;x2-右下角x坐标;y2-右下角y坐标;
*				mode-显示模式,0 表示正显模式下画点，1表示反显模式下画点
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdDrawRectangle(int x1,int y1,int x2,int y2, unsigned char mode)
{
    hal_lcdDrawSingleLine(x1, y1, x2, y1, mode);
    hal_lcdDrawSingleLine(x1, y1, x1, y2, mode);
    hal_lcdDrawSingleLine(x1, y2, x2, y2, mode);
    hal_lcdDrawSingleLine(x2, y1, x2, y2, mode);
	if(g_stLcdGUI.AutoRefresh == 1)
	{
		hal_lcdRefresh();
	}
}


/*
*Function:		hal_lcdDrawRectBlock
*Description:	用指定颜色在背景层绘制实心矩形色块
*Input:			left:左；top:上；right:右；bottom:底；color:渲染颜色值
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_lcdDrawRectBlock(int left,int top,int right,int bottom, unsigned short color)
{
	int i,j;
	
	if (left > right || top > bottom || right >= g_stLcdConfig.LCD_PIXWIDTH || bottom >= g_stLcdConfig.LCD_PIXHIGH)
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
			hal_lcdDrawSinglePix(i,j,color);
		}
	}
	
	
	if(g_stLcdGUI.AutoRefresh == 1)
	{
	    hal_lcdRefresh();
	}
	return 0;
}


/*
*Function:		hal_lcdDrawCircle
*Description:	空心圆绘制
*Input:			x0-圆心x坐标;y0-圆心y坐标;r-半径
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdDrawCircle(unsigned short int x0, unsigned short int y0, unsigned short int r)
{
    int a, b;
    int di;
    a = 0; b = r;
    di = 3 - (r << 1);             //判断下个点位置的标志
    while (a <= b)
    {
        hal_lcdDrawDot(x0 + a, y0 - b,1);
        hal_lcdDrawDot(x0 + b, y0 - a,1);
        hal_lcdDrawDot(x0 + b, y0 + a,1);
        hal_lcdDrawDot(x0 + a, y0 + b,1);
        hal_lcdDrawDot(x0 - a, y0 + b,1);
        hal_lcdDrawDot(x0 - b, y0 + a,1);
        hal_lcdDrawDot(x0 - a, y0 - b,1);
        hal_lcdDrawDot(x0 - b, y0 - a,1);
        a++;
        if(di < 0) di += 4 * a + 6;   //使用Bresenham算法画圆
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
	if(g_stLcdGUI.AutoRefresh == 1)
	{
		hal_lcdRefresh();
	}
}



/*
*Function:		hal_lcdIconClsArea
*Description:	将图标指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_lcdIconClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 
	
	int i,j;
    
 	if (left > right || top > bottom || right >= g_stLcdConfig.LCD_PIXWIDTH || bottom >= g_stLcdConfig.LCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
	
	
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_lcdDrawSinglePix(i,j,g_stLcdGUI.icon_attr.iconAreaColor);
		}
	}
	
	hal_lcdIconRefresh();
	
	
    return 0;

}


/*
*Function:		hal_lcdClsArea
*Description:	将指定区域清除为背景色
*Input:			left:区域最左边；top:区域最上边；right:区域最右边；bottom:区域最底边
*Output:		NULL
*Hardware:
*Return:		<0-失败;0-成功
*Others:
*/
int hal_lcdClsArea(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{ 

	int i,j;
    
 	if (left > right || top > bottom || right >= g_stLcdConfig.LCD_PIXWIDTH || bottom >= g_stLcdConfig.LCD_PIXHIGH)
	{
		return LCD_ERR_AREA_INVALID;
	}
	
	
	for(i = left; i <= right; i++)
	{
		for(j = top;j <= bottom; j++)
		{
			hal_lcdDrawSinglePix(i,j,g_stLcdGUI.grapBackColor);
		}
	}
	if(g_stLcdGUI.AutoRefresh == 1)
	{
		hal_lcdRefresh();
		hal_lcdIconRefresh();
	}
	
    return 0;

}


/*
*Function:		hal_lcdClsUserArea
*Description:	清除整个屏幕不包括图标区域，执行该指令后，将光标移动到左上角（0,12）
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_lcdClsUserArea(void)
{
    int iRet = 0;
	unsigned int startrow = g_stLcdConfig.LCD_TEXTPIXEL;

	if(g_ui8FullScreen == 0)
		startrow = g_stLcdConfig.LCD_TEXTPIXEL;
	else if(g_ui8FullScreen == 1)
		startrow = 0;
    
    iRet = hal_lcdClsArea(0,startrow,g_stLcdConfig.LCD_PIXWIDTH-1,g_stLcdConfig.LCD_PIXHIGH-1);

	g_stCurPixel.x = 0;
	if(g_ui8FullScreen == 0)
		g_stCurPixel.y = g_stLcdConfig.LCD_TEXTPIXEL;
	else if(g_ui8FullScreen == 1)
		g_stCurPixel.y = 0;
	
    return iRet;
}


/*
*Function:		hal_lcdClrLine
*Description:	清除前景层指定的一行或若干行，参数不合理无动作，
*				执行该指令后光标停留在（0，startline）行号从1到7共7行，默认行高为12个bit
*Input:			startline:起始字符行号；endline:结束字符行号
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_lcdClrLine(unsigned char startline, unsigned char endline)
{
	int iRet = -1;


	iRet = hal_lcdDrawRectBlock(0, startline*g_stLcdConfig.LCD_TEXTPIXEL, g_stLcdConfig.LCD_PIXWIDTH-1, (endline+1)*g_stLcdConfig.LCD_TEXTPIXEL-1, g_stLcdGUI.grapBackColor);
	g_stCurPixel.x = 0;
	g_stCurPixel.y = startline*g_stLcdConfig.LCD_TEXTPIXEL;

	return iRet;
}

	
/*
*Function:		hal_lcdWriteBitMap
*Description:	根据data[0]来识别高位在前还是地位在前识别,把指定高度和宽度的点阵数据写到屏幕上
*Input:			left-x坐标;
*				top-y坐标;
*    			*data-显示点阵数据
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败
*Others:
*/
int hal_lcdWriteBitMap(int left, int top, unsigned char *data)
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
    
    if(w > g_stLcdConfig.LCD_PIXWIDTH || h > g_stLcdConfig.LCD_PIXHIGH)
        return LCD_ERR_PIC_DATA_OVERFLOW;
  
    if(w == 0 || h == 0)
        return LCD_ERR_PIC_DATA_EMPTY;
    if(w%8 != 0)
        return LCD_ERR_PARAM_ERROR;
    
	hal_lcdWriteBitData(left, top, p + 6, w, h);
	if(g_stLcdGUI.AutoRefresh == 1)
	{
		hal_lcdRefresh();
	}

    return LCD_ERR_SUCCESS;
}


/*
*Function:		hal_lcdIconDot2DispBuf
*Description:	图标中字模点阵写到显示缓存中
*Input:			x:x坐标; y:y坐标; width:字模宽度; height:字模高度; *dot:字模点阵指针; encode_witdh:1-单内码; 2-多内码
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_lcdIconDot2DispBuf(unsigned int x,unsigned int y,int width,int height,char *dot,int encode_width)//,int single_size,int multi_size)
{

    unsigned char ucBitPix;
    int k;
    int i,j;
    int value_width;
    int value_height;
    int offset;
        
	if(x+width>g_stLcdConfig.LCD_PIXWIDTH || y+height>g_stLcdConfig.LCD_PIXHIGH)
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
                    
					hal_lcdIconDrawDot(x+offset+k,y+i,ucBitPix);           
                   
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
                    
					hal_lcdIconDrawDot(x+offset+k,y+i,ucBitPix);           
                    
    			}
                offset += (8);
                dot++;
    		  }
            i++;
		} 
    }
}


/*
*Function:		hal_lcdDot2DispBuf
*Description:	字模点阵写到显示缓存中
*Input:			x:x坐标; y:y坐标; width:字模宽度; height:字模高度; *dot:字模点阵指针; encode_witdh:1-单内码; 2-多内码
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_lcdDot2DispBuf(unsigned int x,unsigned int y,int width,int height,char *dot,int encode_width)//,int single_size,int multi_size)
{

    unsigned char ucBitPix;
    int k;
    int i,j;
    int value_width;
    int value_height;
    int offset;
        
	if(x+width>g_stLcdConfig.LCD_PIXWIDTH || y+height>g_stLcdConfig.LCD_PIXHIGH)
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
                    
					hal_lcdDrawDot(x+offset+k,y+i,ucBitPix);           
                   
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
                    
					hal_lcdDrawDot(x+offset+k,y+i,ucBitPix);           
                    
    			}
                offset += (8);
                dot++;
    		  }
            i++;
		} 
    }
}



/*
*Function:		hal_lcdIconTextOut
*Description:	图标区域固定行间距的字符串输出
*Input:			x:x坐标; y:y坐标; *str:要显示的字符串内容;*CurStFont:当前字库指针
*Output:		*CurpixelX:当前x坐标;*CurpixelY:当前y坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdIconTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY)
{
	int single_fontsize = 2, multi_fontsize = 2;
	int encode_width = 0;      /* Record the *str 's encode's width is one byte or two bytes,or other.*/
	unsigned char char_width = 0,char_height = 0;  /* Record a char width & height.(display attribute.)*/
	int line_height = 12;//FontAttr.lcd_cur_font.single_code_font.Height;                         /* Record line height.                              */
	char dot_buf[72];                             /*---------------------------------------------!!!!Need hal_commConvert to malloc-point or others implement.!!!!!!!!!---------------*/
    unsigned char *p_str;                         /* Define a temp str-point for the argument of str. */
    int isReverse = 0;
    
	if((x >= g_stLcdConfig.LCD_PIXWIDTH)||(y >= g_stLcdConfig.LCD_PIXHIGH))
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
		encode_width = Ft_GetDot_ASCII(isReverse, &g_stSingleFont6X12, (unsigned char *)p_str, &char_width, &char_height, dot_buf);
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
			if(*CurpixelX + char_width*2 > g_stLcdConfig.LCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
			}
			if(*CurpixelY + line_height > g_stLcdConfig.LCD_PIXHIGH){
				
				hal_lcdIconRefresh();
				
				return; //超出屏幕底行返回不显示
			}
		}
		else if(encode_width==1)
		{
			if(*CurpixelX + char_width > g_stLcdConfig.LCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
			}
			if(*CurpixelY + line_height > g_stLcdConfig.LCD_PIXHIGH){
				
				hal_lcdIconRefresh();
				
				return;
			}
		}
		
		/* Use DrawBitBmp to display a char(*str). */	
        hal_lcdIconDot2DispBuf(*CurpixelX,*CurpixelY,char_width,char_height,dot_buf,encode_width);
        *CurpixelX += char_width*1;
    }

   
    hal_lcdIconRefresh();	
    
}


/*
*Function:		hal_lcdTextOut
*Description:	用户区域区域固定行间距的字符串输出
*Input:			x:x坐标; y:y坐标; *str:要显示的字符串内容;*CurStFont:当前字库指针;mode:0-根据编码格式来处理;1-按照Unicode格式处理
*Output:		*CurpixelX:当前x坐标;*CurpixelY:当前y坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdTextOut(unsigned int x,unsigned int y,const char *str, unsigned int *CurpixelX, unsigned int *CurpixelY, uint8 mode)
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
	
	
	if((x >= g_stLcdConfig.LCD_PIXWIDTH)||(y >= g_stLcdConfig.LCD_PIXHIGH))
    {
		goto exit;
	}
	if(str == NULL)
    {
		goto exit;
	}
    
	if(mode == 0)
	{
		if(g_stLcdGUI.fonttype == 0)
	   	{
	    	int strlenth = strUTF8tostrUnicode(str,strlen(str),texttmprP);
			p_str = texttmprP;
	   	}
		else if(g_stLcdGUI.fonttype == 1)
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
		else if(g_stLcdGUI.fonttype == 2)
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
			if(*CurpixelX + char_width > g_stLcdConfig.LCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
				sysLOG(LCD_LOG_LEVEL_5, "*CurpixelX=%d, *CurpixelY=%d\r\n", *CurpixelX, *CurpixelY);
			}
			if(*CurpixelY + line_height > g_stLcdConfig.LCD_PIXHIGH){
				
				if(g_stLcdGUI.AutoRefresh == 1)
				{
					hal_lcdRefresh();
				}
				goto exit; //超出屏幕底行返回不显示
			}
		}
		else if(encode_width==1)
		{
			if(*CurpixelX + char_width > g_stLcdConfig.LCD_PIXWIDTH){ 
				*CurpixelX  = 0;
				*CurpixelY += line_height;
				sysLOG(LCD_LOG_LEVEL_5, "*CurpixelX=%d, *CurpixelY=%d\r\n", *CurpixelX, *CurpixelY);
			}
			if(*CurpixelY + line_height > g_stLcdConfig.LCD_PIXHIGH){
				
				if(g_stLcdGUI.AutoRefresh == 1)
				{
					hal_lcdRefresh();
				}
				goto exit;
			}
		}
		
		/* Use DrawBitBmp to display a char(*str). */	
        hal_lcdDot2DispBuf(*CurpixelX,*CurpixelY,char_width,char_height,dot_buf,encode_width);
        *CurpixelX += char_width*1;
    }while(strtmp);

    if(g_stLcdGUI.AutoRefresh == 1)
	{
        hal_lcdRefresh();	
    }

exit:

	if(texttmprP != NULL)
		free(texttmprP);
	
	return;
}


/*
*Function:		hal_lcdWriteIcon
*Description:	LCD 写图标
*Input:			x-x坐标；y-y坐标；*data-图标内容；width-图标占用的宽度；height-图标占用的高度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdWriteIcon(uint16 x, uint16 y, uint8 *data, uint8 width, uint8 height, unsigned short color)
{
	int iRet = -1;
	uint8 bitPixtmp;
	uint16 ix, iy;
	uint8 heighttmp;
	uint8 widthtmp;

	if(width <= 8)
	{
		heighttmp = height;
		widthtmp = 8;
	}
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
	
	sysLOG(LCD_LOG_LEVEL_5, "x=%d, y=%d, color=%d\r\n", x, y, color);
	for(iy=0; iy<height; iy++)
	{
		for(ix=0; ix<width; ix++)
		{
			
			bitPixtmp = (data[iy*(widthtmp/8) + ix/8] & (1 <<(7-ix%8))) >> (7-ix%8);
			if(bitPixtmp)
			{
				iRet = hal_lcdDrawDotC(x+ix, y+iy, color);
			}
			else
			{
				iRet = hal_lcdDrawDotC(x+ix, y+iy, g_stLcdGUI.icon_attr.iconAreaColor);
			}
			if(iRet < 0)
			{
				sysLOG(LCD_LOG_LEVEL_5, "hal_lcdDrawDotC iRet=%d\r\n", iRet);
			}
		}
	}

	
	hal_lcdIconRefresh();
	
}


/*
*Function:		hal_lcdSetAttrib
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
int hal_lcdSetAttrib(LCD_ATTRIBUTE_t attr, int value)
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

			g_stLcdGUI.Mode = (unsigned short int)value;
			iRet = 0;
		break;
   		case LCD_ATTR_BACK_COLOR://设置黑白屏背景色，支持0-3,4个色阶
   			if(value < 0 || value > 3)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stLcdGUI.grapBackColor = (unsigned short)value;
			iRet = 0;
		break;
    	case LCD_ATTR_FRONT_COLOR://设置黑白屏前景色，支持0-3,4个色阶
    		if(value < 0 || value > 3)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stLcdGUI.grapFrontColor = (unsigned short)value;
			iRet = 0;
		break;
    	case LCD_ATTR_AUTO_UPDATE://此项控制是否自动刷新RAM中的内容到屏幕。
    		if(value < 0 || value > 1)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stLcdGUI.AutoRefresh = (unsigned char)value;
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
			g_stLcdGUI.BackLight = (unsigned char)value;
			hal_scrSetBackLightValue((unsigned char)value);
			iRet = 0;
		break;
		case LCD_ATTR_FONTTYPE:
			if(value < 0 || value > 2)
			{
				iRet = LCD_ERR_PARAM_ERROR;
				goto exit;
			}
			g_stLcdGUI.fonttype = (unsigned char)value;
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
*Function:		hal_lcdGetAttrib
*Description:	获取液晶显示器的功能属性
*Input:			attr:LCD_ATTRIBUTE_t
*Output:		NULL
*Hardware:
*Return:		<0-失败，>=0-成功，具体值为所读取的功能属性value
*Others:
*/
int hal_lcdGetAttrib(LCD_ATTRIBUTE_t attr)
{

	int iRet = -1;
	switch(attr)
	{
		case LCD_ATTR_POWER:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_TEXT_INVERT:
			iRet = g_stLcdGUI.Mode;
		break;
		case LCD_ATTR_BACK_COLOR:
			iRet = g_stLcdGUI.grapBackColor;
		break;
		case LCD_ATTR_FRONT_COLOR:
			iRet = g_stLcdGUI.grapFrontColor;
		break;
		case LCD_ATTR_AUTO_UPDATE:
			iRet = g_stLcdGUI.AutoRefresh;
		break;
		case LCD_ATTR_ICON_BACK_COLOR:
			iRet = SCR_NOT_SUPPORT;
		break;
		case LCD_ATTR_LIGHT_LEVEL:
			iRet = g_stLcdGUI.BackLight;
		break;
		case LCD_ATTR_FONTTYPE:
			iRet = g_stLcdGUI.fonttype;
		break;
		case LCD_ATTR_ICONHEIGHT:
			iRet = g_stLcdConfig.LCD_TEXTPIXEL;
		break;
		case LCD_ATTR_WIDTH:
			iRet = g_stLcdConfig.LCD_PIXWIDTH;
		break;
		case LCD_ATTR_HEIGHT:
			iRet = g_stLcdConfig.LCD_PIXHIGH;
		break;
		case LCD_ATTR_USERWIDTH:
			iRet = g_stLcdConfig.LCD_PIXWIDTH;
		break;
		case LCD_ATTR_USERHEIGHT:
			iRet = g_stLcdConfig.LCD_PIXHIGH-g_stLcdConfig.LCD_TEXTPIXEL;
		break;
		case LCD_ATTR_LCDCOLORVALUE:
			iRet = 1;
		break;
		case LCD_ATTR_BACKLIGHT_MODE:
			iRet = g_stLcdGUI.BackLightMode;
		break;
		default:
			iRet = SCR_NOT_SUPPORT;
		break;
	
	}
	return iRet;

}


/*
*Function:		hal_lcdGotoxyPix
*Description:	定位LCD显示光标，参数超出LCD范围时不改变原坐标位置
*Input:			pixel_X:横坐标；pixel_Y:纵坐标
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_lcdGotoxyPix(unsigned int pixel_X, unsigned int pixel_Y)
{
	if(pixel_X < 0 || pixel_X >=g_stLcdConfig.LCD_PIXWIDTH || pixel_Y < 0 || pixel_Y >= g_stLcdConfig.LCD_PIXHIGH)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}

	g_stCurPixel.x = pixel_X;
	g_stCurPixel.y = pixel_Y;
	return 0;
}


/*
*Function:		hal_lcdGetxyPix
*Description:	读取LCD上光标的当前位置。
*Input:			NULL
*Output:		*pixel_X:横坐标; *pixel_Y:纵坐标
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdGetxyPix(int *pixel_X, int *pixel_Y)
{
	
	*pixel_X = g_stCurPixel.x;
	*pixel_Y = g_stCurPixel.y;

}


/*
*Function:		hal_lcdGetPixel
*Description:	获取特定坐标点的颜色值
*Input:			x:x坐标0-127；y:y坐标0-95；
*Output:		*picolor:颜色值
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int hal_lcdGetPixel(unsigned short int x, unsigned short int y, unsigned int *picolor)
{
	uint pixcolor = 0;
	if(x < 0 || x >=g_stLcdConfig.LCD_PIXWIDTH || y < 0 || y >= g_stLcdConfig.LCD_PIXHIGH)
	{
		return LCD_ERR_COORDINATE_INVALID;
	}
	
	pixcolor |= (*(g_stLcdGUI.DispBuff+y/g_stLcdConfig.LCD_BLOCKHIGH*g_stLcdConfig.LCD_BLOCKWIDTH+x) & 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+0))>>((y%g_stLcdConfig.LCD_BLOCKHIGH)*2);
	pixcolor |= (*(g_stLcdGUI.DispBuff+y/g_stLcdConfig.LCD_BLOCKHIGH*g_stLcdConfig.LCD_BLOCKWIDTH+x) & 1<<((y%g_stLcdConfig.LCD_BLOCKHIGH)*2+1))>>((y%g_stLcdConfig.LCD_BLOCKHIGH)*2);
	*picolor = pixcolor;
	return 0;
}


/*
*Function:		hal_lcdGetLcdSize
*Description:	读取LCD显示区域大小
*Input:			NULL
*Output:		*width:LCD显示宽度；*height:LCD显示高度
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdGetLcdSize(int *width, int *height)
{
	*width = g_stLcdConfig.LCD_PIXWIDTH;
	*height = g_stLcdConfig.LCD_PIXHIGH;

}


/*
*Function:		hal_lcdPrintf
*Description:	在屏幕的前景层当前位置格式化显示字符串
*Input:			*fmt:显示输出的字符串及格式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdPrintf(const char *fmt, ...)
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
	hal_lcdTextOut(g_stCurPixel.x,g_stCurPixel.y,bufrP, &g_stCurPixel.x, &g_stCurPixel.y, 0);

exit:

	if(bufrP != NULL)
		free(bufrP);

	return;


}


/*
*Function:		hal_lcdPrint
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
void hal_lcdPrint(uint col, unsigned char row, unsigned char mode, const char *str, ...)
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

	if(coltmp >= g_stLcdConfig.LCD_PIXWIDTH)
	{
		coltmp = 0;
		row += 1;
	}

	if(row >= g_stLcdConfig.LCD_PIXHIGH/g_stLcdConfig.LCD_TEXTPIXEL)
		return;

	unsigned char *rP1 = NULL;
	unsigned char offsetenter = 0;
	
	disMode = g_stLcdGUI.Mode;
	dismodetmp = (mode&0x80) >> 7;//0-正显，1-反显
	aligntmp = (mode&0x06) >> 1;//0-左对齐，1-居中，2-右对齐
	
	sysLOG(LCD_LOG_LEVEL_5, "mode=%x,dismodetmp=%d,aligntmp=%d\r\n", mode, dismodetmp, aligntmp);

	va_list args;
	
	memset(bufrP, 0, bufSIZE);
	va_start(args, str);
	vsnprintf(bufrP, bufSIZE-4,str, args);
	va_end(args);

	sysLOG(LCD_LOG_LEVEL_5, "start fonttype\r\n");

	if(g_stLcdGUI.fonttype == 0)
   	{
    	int strlenth = strUTF8tostrUnicode(bufrP,strlen(bufrP),texttmprP);
		p_str = texttmprP;
   	}
	else if(g_stLcdGUI.fonttype == 1)
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
	else if(g_stLcdGUI.fonttype == 2)
	{
		p_str = bufrP;
	}

	sysLOG(LCD_LOG_LEVEL_5, "end fonttype, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", *p_str,*(p_str+1),*(p_str+2),*(p_str+3),*(p_str+4),*(p_str+5),*(p_str+6),*(p_str+7),*(p_str+8),*(p_str+9));

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
			sysLOG(LCD_LOG_LEVEL_5, "sumlinenum=%d,rP=%x,&bufrP+sumlinenum=%x,oncenum=%d\r\n", sumlinenum, rP, bufrP+sumlinenum, oncenum);
			
		}
		else
		{
			oncenum = Getlenstr(p_str)-sumlinenum;
			sysLOG(LCD_LOG_LEVEL_5, "2 Getlenstr(p_str)=%d,sumlinenum=%d\r\n", Getlenstr(p_str), sumlinenum);
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
		}
		memset(oncebufrP, 0, oncebufSIZE);
		memcpy(oncebufrP, p_str+sumlinenum, oncenum);//copy从已经显示内容之后到下一次\r\n为止，然后在逐行显示
		sumlinenum += oncenum;
		sysLOG(LCD_LOG_LEVEL_5, "2 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenum = CalTextline(coltmp, oncebufrP, oncenum, aligntmp, startlinepix, startstrlen, g_stLcdConfig.LCD_PIXWIDTH);//oncenum/((COLORLCD_PIXWIDTH-coltmp)/(textpixeltmp/2));		
		sysLOG(LCD_LOG_LEVEL_5, "1 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
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

			ytmp = row*g_stLcdConfig.LCD_TEXTPIXEL+sumline*textpixeltmp;
			sysLOG(LCD_LOG_LEVEL_5, "xtmp=%d, ytmp=%d, aligntmp=%d, linenumfinal=%d,onelinebufrP:%d,%d,%d,%d\r\n", xtmp, ytmp, aligntmp, linenumfinal, *onelinebufrP, *(onelinebufrP+1), *(onelinebufrP+2), *(onelinebufrP+3));
			g_stLcdGUI.Mode = dismodetmp;
			hal_lcdTextOut(xtmp, ytmp, onelinebufrP, &g_stCurPixel.x, &g_stCurPixel.y, 1);
			g_stLcdGUI.Mode = disMode;
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
*Function:		hal_lcdPrintxy
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
void hal_lcdPrintxy(uint col, uint row, unsigned char mode, const char *str, ...)
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

	if(coltmp >= g_stLcdConfig.LCD_PIXWIDTH)
	{
		coltmp = 0;
		row += g_stLcdConfig.LCD_TEXTPIXEL;
	}

	if(row >= g_stLcdConfig.LCD_PIXHIGH)
		return;


	unsigned char *rP1 = NULL;
	unsigned char offsetenter = 0;
	
	disMode = g_stLcdGUI.Mode;
	dismodetmp = (mode&0x80) >> 7;//0-正显，1-反显
	aligntmp = (mode&0x06) >> 1;//0-左对齐，1-居中，2-右对齐
	
	sysLOG(LCD_LOG_LEVEL_5, "mode=%x,dismodetmp=%d,aligntmp=%d\r\n", mode, dismodetmp, aligntmp);

	va_list args;
	
	memset(bufrP, 0, bufSIZE);
	va_start(args, str);
	vsnprintf(bufrP, bufSIZE-4,str, args);
	va_end(args);

	sysLOG(LCD_LOG_LEVEL_5, "start fonttype\r\n");

	if(g_stLcdGUI.fonttype == 0)
   	{
    	int strlenth = strUTF8tostrUnicode(bufrP,strlen(bufrP),texttmprP);
		p_str = texttmprP;
   	}
	else if(g_stLcdGUI.fonttype == 1)
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
	else if(g_stLcdGUI.fonttype == 2)
	{
		p_str = bufrP;
	}

	sysLOG(LCD_LOG_LEVEL_5, "end fonttype, %d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", *p_str,*(p_str+1),*(p_str+2),*(p_str+3),*(p_str+4),*(p_str+5),*(p_str+6),*(p_str+7),*(p_str+8),*(p_str+9));

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
			sysLOG(LCD_LOG_LEVEL_5, "sumlinenum=%d,rP=%x,&bufrP+sumlinenum=%x,oncenum=%d\r\n", sumlinenum, rP, bufrP+sumlinenum, oncenum);
			
		}
		else
		{
			oncenum = Getlenstr(p_str)-sumlinenum;
			sysLOG(LCD_LOG_LEVEL_5, "2 Getlenstr(p_str)=%d,sumlinenum=%d\r\n", Getlenstr(p_str), sumlinenum);
			if(oncenum == 0)
			{
				goto exit;
			}
			offsetenter = 0;
		}
		memset(oncebufrP, 0, oncebufSIZE);
		memcpy(oncebufrP, p_str+sumlinenum, oncenum);//copy从已经显示内容之后到下一次\r\n为止，然后在逐行显示
		sumlinenum += oncenum;
		sysLOG(LCD_LOG_LEVEL_5, "2 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
		linenum = CalTextline(coltmp, oncebufrP, oncenum, aligntmp, startlinepix, startstrlen, g_stLcdConfig.LCD_PIXWIDTH);//oncenum/((COLORLCD_PIXWIDTH-coltmp)/(textpixeltmp/2));		
		sysLOG(LCD_LOG_LEVEL_5, "1 linenum=%d,sumlinenum=%d,oncenum=%d\r\n", linenum, sumlinenum, oncenum);
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
			sysLOG(LCD_LOG_LEVEL_5, "xtmp=%d, ytmp=%d, aligntmp=%d, linenumfinal=%d,onelinebufrP:%d,%d,%d,%d\r\n", xtmp, ytmp, aligntmp, linenumfinal, *onelinebufrP, *(onelinebufrP+1), *(onelinebufrP+2), *(onelinebufrP+3));
			g_stLcdGUI.Mode = dismodetmp;
			hal_lcdTextOut(xtmp, ytmp, onelinebufrP, &g_stCurPixel.x, &g_stCurPixel.y, 1);
			g_stLcdGUI.Mode = disMode;
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
*Function:		hal_lcdContrastSet
*Description:	设置对比度
*Input:			value:0-63
*Output:		NULL
*Hardware:
*Return:		0-成功; <0-失败
*Others:
*/
int8 hal_lcdContrastSet(uint8 value)
{
	if(value < 0 || value > 63)
	{
		return -1;
	}
	if(g_ui32LcdDevID == FYTLCD_2_4_ST7571_ID)
	{
		g_ui8ContrastValue = value;
		fibo_dotMatrixLcdSetDisplayParam(0x06, g_ui8ContrastValue, 0x07);
	}
	else if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)
	{
		g_ui8ContrastValue = value;
		fibo_dotMatrixLcdSetDisplayParam(0x03, g_ui8ContrastValue*2, 0x03);
	}
	return 0;
}


/*
*Function:		hal_lcdContrastGet
*Description:	读取对比度
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0-对比度值；<0-失败
*Others:
*/
int8 hal_lcdContrastGet(void)
{

	return g_ui8ContrastValue;
}


/*
*Function:		hal_lcdInit
*Description:	LCD 黑白屏初始化参数
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_lcdInit(void)
{
	
	//memset(ucIconBuf,0x00,sizeof*(ucIconBuf));
	g_stLcdGUI.Mode = 0;//正常显示
	g_stLcdGUI.AutoRefresh = 1;//自动刷新
	g_stLcdGUI.grapBackColor = g_stLogoinfoJson.lcd_bg_color;
	g_stLcdGUI.grapFrontColor = g_stLogoinfoJson.lcd_fg_color;
	g_stLcdGUI.BackLight = g_ui8BackLightValue/2.55;
	g_stLcdGUI.BackLightEN = 1;
	g_stLcdGUI.BackLightMode = 1;
	g_stLcdGUI.BackLightTimeout = BACKLIGHTTIMEOUT;
	g_stLcdGUI.icon_attr.iconAreaColor = 0;
	g_stLcdGUI.icon_attr.iconColor = 3;
	g_stLcdGUI.fonttype = 0;
	g_stLcdGUI.rotate = 0;
	g_stCurPixel.x = 0;
	g_stCurPixel.y = 12;
	if(FONTFS == hal_fontGetFontType())
		hal_scrSelectFont(&g_stSingleFont6X12, &g_stGBMultiFont12X12);
//	else
//		hal_scrSelectFont(&g_stSingleFont6X12, &g_stUniMultiFont12X12);

	g_stLcdGUI.rotateBuff = malloc(g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH);
	memset(g_stLcdGUI.rotateBuff,0x0,g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH);
	
	
}




/*
*Function:		hal_lcdGetDotP
*Description:	获取显存中指定位置的地址指针
*Input:			x:坐标x; y:坐标y;
*Output:		NULL
*Hardware:
*Return:		当前位置的指针
*Others:
*/
uint8_t *hal_lcdGetDotP(unsigned short x,unsigned short y)
{
	return (uint8_t *)(g_stLcdGUI.DispBuff+(y*g_stLcdConfig.LCD_PIXWIDTH+x)/4);
}


/*
*Function:		hal_lcdPopDot
*Description:	将显存中的点阵数据导出
*Input:			lenth：缓存空间大小
*Output:		popDot：缓存地址；
*Hardware:
*Return:		导出的数据字节长度
*Others:
*/
int hal_lcdPopDot(unsigned char *popDot,int lenth)
{
	uint8_t *dotp;
	int maxDataLen;
	
	dotp = hal_lcdGetDotP(0,g_stLcdConfig.LCD_TEXTPIXEL);

	maxDataLen = (g_stLcdConfig.LCD_SUMPIXNUM - g_stLcdConfig.LCD_PIXWIDTH*g_stLcdConfig.LCD_TEXTPIXEL)/g_stLcdConfig.LCD_BLOCKHIGH;
	
	if((lenth < 0) || (lenth > maxDataLen))
		lenth = maxDataLen;
	
	memcpy(popDot, dotp, lenth);

	return lenth;
}

/*
*Function:		hal_lcdPushDot
*Description:	将内存中的ram数据导入显存 并强制刷屏
*Input:			pushDot：缓存地址；lenth：缓存空间大小
*Output:		NULL
*Hardware:
*Return:		导入的数据字节长度
*Others:
*/
int hal_lcdPushDot(unsigned char *pushDot,int lenth)
{
	uint8_t *dotp;	
	int maxDataLen;
	
	dotp = hal_lcdGetDotP(0,g_stLcdConfig.LCD_TEXTPIXEL);

	maxDataLen = (g_stLcdConfig.LCD_SUMPIXNUM - g_stLcdConfig.LCD_PIXWIDTH*g_stLcdConfig.LCD_TEXTPIXEL)/g_stLcdConfig.LCD_BLOCKHIGH;

	if((lenth < 0) || (lenth > maxDataLen))
		lenth = maxDataLen;
	
	memcpy(dotp, pushDot, lenth);

	int m;
	for(m=0; m<g_stLcdConfig.LCD_BLOCKBUFNUM; m++)
		g_stLcdGUI.RefreshFlag[m] = 0xFF;// 
		
	hal_lcdRefresh();

	return lenth;
}


/*
*Function:		hal_lcdRotate180
*Description:	旋转屏幕180
*Input:			value:0-正常显示，1-倒显
*Output:		NULL
*Hardware:
*Return:		<0-失败；>=0-成功，成功处理的字节数
*Others:
*/
int hal_lcdRotate180(uint8 value)
{
	int iRet = -1;

	if(g_stLcdGUI.rotate == value)//方向一致，不需要旋转
	{
		iRet = LCD_ERR_SUCCESS;
		goto exit;
	}

	if(value == 0)//正常显示
	{
		
		iRet = hal_lcdDotRotate(g_stLcdGUI.rotateBuff, g_stLcdGUI.DispBuff, g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH);
		for(int i=0; i<g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH; i++)
		{
			*(g_stLcdGUI.DispBuff+i) = hal_lcdCharrotate(*(g_stLcdGUI.DispBuff+i));
		}

		g_stLcdGUI.rotate = 0;

		int m;
		for(m=0; m<g_stLcdConfig.LCD_BLOCKBUFNUM; m++)
		g_stLcdGUI.RefreshFlag[m] = 0xFF;

		hal_lcdRefreshRotate();
		
	
		
	}
	else if(value == 1)//倒显
	{
		
		iRet = hal_lcdDotRotate(g_stLcdGUI.DispBuff, g_stLcdGUI.rotateBuff, g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH);
		for(int i=0; i<g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKHIGH; i++)
		{
			*(g_stLcdGUI.rotateBuff+i) = hal_lcdCharrotate(*(g_stLcdGUI.rotateBuff+i));
		}

		g_stLcdGUI.rotate = 1;

		int m;
		for(m=0; m<g_stLcdConfig.LCD_BLOCKBUFNUM; m++)
		g_stLcdGUI.RefreshFlag[m] = 0xFF;

		hal_lcdRefreshRotate();
		
		
	}
	else

	{
		iRet = LCD_ERR_PARAM_ERROR;
		goto exit;
	}


exit:

	return iRet;
}




/******************************TEST***********************************/


