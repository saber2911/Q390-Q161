/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_cblcd.c
** Description:		断码屏相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"
#include "dev_cblcd.h"




/*
*Function:		hal_cblcdBackLightCtl
*Description:	控制断码屏背光的开关
*Input:			val:TRUE-打开，FALSE-关闭
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdBackLightCtl(BOOL val)
{

	if(0 == val)
		fibo_SetPwlLevel(1, 0);
	else
		fibo_SetPwlLevel(1, 0xFF);
	
	return 0;
}


/*
*Function:		hal_cblcdClr
*Description:	清屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdClr(void)
{
	
	fibo_gpio_set(CBLCD_CS_GPIO, FALSE);
	Delayss(2);
	dev_cblcdSendBit(1);
	dev_cblcdSendBit(0);
	dev_cblcdSendBit(1);
	dev_cblcdAddr(9);
	for(uint8 i=0; i<14; i++)
	{
		dev_cblcdData(0x00);

	}
		
	fibo_gpio_set(CBLCD_CS_GPIO, TRUE);
	Delayss(2);
	
	return 0;

}


/*
*Function:		hal_cblcdHalfCode
*Description:	数码管刷完整的一个地址中的显示
*Input:			addr:地址，data:数据
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdHalfCode(uint8 addr, uint8 data)
{
	fibo_gpio_set(CBLCD_CS_GPIO, FALSE);
	Delayss(2);
	dev_cblcdSendBit(1);
	dev_cblcdSendBit(0);
	dev_cblcdSendBit(1);
	dev_cblcdAddr(addr);
	dev_cblcdData(data);
	fibo_gpio_set(CBLCD_CS_GPIO, TRUE);
	Delayss(2);

	return 0;
}


/*
*Function:		hal_cblcdCode
*Description:	写一个完整的数字(包含小数点)
*Input:			addr0:第一个地址，addr1:第二个地址，data&0x0F表示的是需要显示的数值，data&0x10:表示的是否有小数点
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdCode(uint8 addr0, uint8 addr1, uint8 data)
{
	
	uint8 tmp1, tmp2;
	
	switch(data&0x0F)
	{
		case 0:
			tmp1 = 0b1111;
			tmp2 = 0b0101;
		break;
		case 1:
			tmp1 = 0b0000;
			tmp2 = 0b0101;
		break;
		case 2:
			tmp1 = 0b1101;
			tmp2 = 0b0011;
		break;
		case 3:
			tmp1 = 0b1001;
			tmp2 = 0b0111;
		break;
		case 4:
			tmp1 = 0b0010;
			tmp2 = 0b0111;
		break;
		case 5:
			tmp1 = 0b1011;
			tmp2 = 0b0110;
		break;
		case 6:
			tmp1 = 0b1111;
			tmp2 = 0b0110;
		break;
		case 7:
			tmp1 = 0b0001;
			tmp2 = 0b0101;
		break;
		case 8:
			tmp1 = 0b1111;
			tmp2 = 0b0111;
		break;
		case 9:
			tmp1 = 0b1011;
			tmp2 = 0b0111;
		break;
		default:
			tmp1 = 0;
			tmp2 = 0;

		break;
		
	}
	if(data&0x10)
	{
		tmp2 |= 0b1000;
	}
	hal_cblcdHalfCode(addr0, tmp1);
	hal_cblcdHalfCode(addr1, tmp2);

	return 0;

}


/*
*Function:		hal_cblcdDisplay
*Description:	数码管显示数字(包括小数点)
*Input:			num:0-6，第几个数码管(从右往左递增)，
*				val:val&0x0F表示的是需要显示的数值，val&0x10:表示的是否有小数点,eg:0x12显示2.,0x02显示2
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdDisplay(uint8 num, uint8 val)
{

	switch(num)
	{
		case 0:
			hal_cblcdCode(19, 18, val);
		break;
		case 1:
			hal_cblcdCode(21, 20, val);
		break;
		case 2:
			hal_cblcdCode(9, 22, val);
		break;
		case 3:
			hal_cblcdCode(11, 10, val);
		break;
		case 4:
			hal_cblcdCode(13, 12, val);
		break;
		case 5:
			hal_cblcdCode(15, 14, val);
		break;
		case 6:
			hal_cblcdCode(17, 16, val);
		break;
		default:
		break;
		
		
	}

	return 0;
	

}


/*
*Function:		hal_cblcdDispAmount
*Description:	显示人民币金额
*Input:			amount:金额值，最大可输入 7 位数字。即 9999999。单位：分。
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdDispAmount(int amount)
{
	int iRet = -1;
	uint8 i,len;
	int tmp1;
	uint8 bufftmp[8];
	
	if(amount > 9999999 || amount < 0)
	{
		sysLOG(CBLCD_LOG_LEVEL_1, "<ERR> amount=%d\r\n", amount);
		iRet = -2;
		return iRet;
	}
	
	hal_cblcdClr();

	if(amount == 0)
	{
		hal_cblcdDisplay(0, 0);
		hal_cblcdDisplay(1, 0);
		hal_cblcdDisplay(2, 0x10);
		iRet = 0;
		goto exit;
	}
	

	memset(bufftmp, 0, sizeof(bufftmp));
	tmp1 = amount;
	len = 3;
	for(i=7; i>0; i--)
	{
		bufftmp[i-1] = tmp1/(pow(10,i-1));
		tmp1 -= bufftmp[i-1]*(pow(10,i-1));
		if(bufftmp[i-1] != 0 && len < i)
		{
			len = i;
		}
		
	}

	for(i=0; i<len; i++)
	{
		
		if(i == 2)
		{
			hal_cblcdDisplay(i, bufftmp[i]+0x10);
		}
		else
		{
			hal_cblcdDisplay(i, bufftmp[i]);
		}

	}

	iRet = 0;
	
exit:

	return iRet;
	
}


/*
*Function:		hal_cblcdDispNum
*Description:	显示数字
*Input:			num:数字，最大可输入7位数字只可以是整数。即 9999999
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdDispNum(int num)
{
	int iRet = -1;
	uint8 i,len;
	int tmp1;
	uint8 bufftmp[8];
	
	if(num > 9999999 || num < 0)
	{
		sysLOG(CBLCD_LOG_LEVEL_1, "<ERR> num=%d\r\n", num);
		iRet = -2;
		return iRet;
	}
	
	hal_cblcdClr();

	if(num == 0)
	{
		hal_cblcdDisplay(0, 0);
		
		iRet = 0;
		goto exit;
	}
	

	memset(bufftmp, 0, sizeof(bufftmp));
	tmp1 = num;
	len = 1;
	for(i=7; i>0; i--)
	{
		bufftmp[i-1] = tmp1/(pow(10,i-1));
		tmp1 -= bufftmp[i-1]*(pow(10,i-1));
		if(bufftmp[i-1] != 0 && len < i)
		{
			len = i;
		}
		
	}

	for(i=0; i<len; i++)
	{
		
		if(i == 2)
		{
			hal_cblcdDisplay(i, bufftmp[i]);
		}
		else
		{
			hal_cblcdDisplay(i, bufftmp[i]);
		}

	}

	iRet = 0;
	
exit:

	return iRet;
	
}


/*
*Function:		hal_cblcdOpen
*Description:	开启断码屏显示
*Input:			backlight:是否打开背光，TRUE-打开背光;FALSE-不打开背光
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_cblcdOpen(int backlight)
{
	sysDelayMs(200);
	dev_cblcdSYSEN();
	dev_cblcdON();
	dev_cblcdBIAS(3);
	hal_cblcdClr();
	hal_cblcdBackLightCtl(backlight);

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
int hal_cblcdClose(void)
{

	dev_cblcdOFF();
	dev_cblcdSYSDIS();
	hal_cblcdClr();
	hal_cblcdBackLightCtl(FALSE);


	return 0;
}


/****************************TEST****************************/

void CblcdTest(void)
{
	uint8 i=0, j=0;
	
	dpyInit_lib();
	dpySetBackLight_lib(TRUE);

#if 1
	//sysDelayMs(2000);
	
//	for(i=9; i<23; i++)
//	{
//		hal_cblcdHalfCode(i, 0x0F);
//		sysDelayMs(500);
//	}
//	hal_cblcdClr();
//	sysDelayMs(2000);
	for(i=0; i<7; i++)
	{
		for(j=0; j<10; j++)
		{
			hal_cblcdDisplay(i, j);
			sysDelayMs(50);
		}
		hal_cblcdDisplay(i, 0x10+i);
		sysDelayMs(50);
		
	}
	sysDelayMs(2000);
	dpyCls_lib();
	sysDelayMs(2000);
	dpySetBackLight_lib(FALSE);
	sysDelayMs(2000);
	dpySetBackLight_lib(TRUE);
	sysDelayMs(2000);
	hal_cblcdClose();
	sysDelayMs(2000);
	dpyInit_lib();
	dpySetBackLight_lib(TRUE);

	dpyDispAmount_lib(0);
	sysDelayMs(2000);
	dpyDispAmount_lib(100);
	sysDelayMs(2000);
	dpyDispAmount_lib(10000);
	sysDelayMs(2000);
	dpyDispAmount_lib(2);
	sysDelayMs(2000);
	dpyDispAmount_lib(884812);
	sysDelayMs(2000);
	dpyDispAmount_lib(9999999);
	sysDelayMs(1000);

//	hal_cblcdClose();
#endif	
}



