/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_keys.c
** Description:		按键相关接口
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


struct _KEY_TABLE g_stKeyTable;


uint8 g_ui8KeyPwroffFree=0;//关机按键释放标志位，0-按键释放；1-按键未释放
uint8 g_ui8KeyPwronFree=1;//开机按键释放标志位，0-按键释放；1-按键未释放


KEYS_CALLBACK g_fcKeysCallback = NULL;

KEYS_CALLBACK_PWROFF g_fcKeysCallbackPwroff = NULL;


struct _KEYIRQ_S g_stKeyPwrIRQ;

uint32 g_ui32QueueKeyValue;//键值队列

volatile uint8 g_ui8KeySoundFlag = 0;//0-关闭按键音;1-开启按键音
uint32 g_ui32KeypadStatus = 0;
uint8 g_ui8PwrkeyFlag = 0;//电源键按下，0-未按下，1-短按，2-长按

int g_iTypeofKeyboard = 0; // 0:无，1:AP ,2:SE, 

/*
*Function:		hal_keypadReg
*Description:	按键回调函数注册接口
*Input:			*keys_callback_P:按键回调接口指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadReg(void (*keys_callback_P)(uint8 key_ID, uint8 key_V))
{
	g_fcKeysCallback = keys_callback_P;
}


/*
*Function:		hal_keypadPwrOffReg
*Description:	按键关机回调函数注册接口
*Input:			*keys_callback_P:按键关机回调接口指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadPwrOffReg(void (*keys_callback_pwroff_P)(void))
{
	g_fcKeysCallbackPwroff = keys_callback_pwroff_P;
}


/*
*Function:		hal_keypadPwrkeyDeal
*Description:	充电中启动开机流程接口
*Input:			key_V:电源按键键值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadPwrkeyDeal(uint8 key_V)
{	
	sysLOG(KEYS_LOG_LEVEL_5, "<SUCC> <PWR> pressed! key_V:%d\r\n", key_V); 		
	if(key_V == 2)
	{
		g_ui8SystemStatus = 2;//启动开机流程
	}
		
}


/*
*Function:		hal_keypadInit
*Description:	键盘初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadInit(void)
{
	g_ui32QueueKeyValue = fibo_queue_create(KEYQUEUELEN, sizeof(int));
	sysLOG(KEYS_LOG_LEVEL_4, "fibo_queue_create g_ui32QueueKeyValue:%x\r\n", g_ui32QueueKeyValue);
	fibo_keypad_queue_init(KEYLONGTIMEOUT);
}



/*
*Function:		hal_keypadCheck
*Description:	检测队列中是否有未取走的键值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:队列中无键值；非0:队列中有键值
*Others:
*/
unsigned char hal_keypadCheck(void)
{
	int iRet;
	iRet = fibo_queue_space_available(g_ui32QueueKeyValue);
	iRet = KEYQUEUELEN - iRet;
	return (uint8)iRet;
}

/*
*Function:		hal_keypadHit
*Description:	检测键盘缓冲区中是否有尚未取走的按键值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:缓冲区有按键值；非0:缓冲区没有键值
*Others:
*/
unsigned char hal_keypadHit(void)
{
	uint8 ret;
	ret = hal_keypadCheck();
	ret = ret == 0 ? 1 : 0;
	return ret;
}


/*
*Function:		hal_keypadFlush
*Description:	键盘队列清空
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadFlush(void)
{
	fibo_queue_reset(g_ui32QueueKeyValue);
}


/*
*Function:		hal_keypadGetKey
*Description:	读取一个键盘队列中最早放入的一个键值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		读到的键值，没有键值则反0xFF
*Others:
*/
unsigned char hal_keypadGetKey(void)
{
	int iRet = -1;
	uint32 keyvaluetmp;
	iRet = fibo_queue_get(g_ui32QueueKeyValue, &keyvaluetmp, 10);
	if(iRet == 0)
	{
		return keyvaluetmp;
	}
	else




	{
		return 0xFF;
	}
}



/*
*Function:		hal_keypadHandle
*Description:	键盘数据处理句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadHandle(void)
{
	int iRet = -1;
	keypad_info_t keypad_info;
	
	iRet = fibo_keypad_queue_output(&keypad_info);
	if(iRet == TRUE)
	{
		if(keypad_info.long_or_short_press == 2 && keypad_info.press_or_release == 1)//按下
		{
			g_ui32KeypadStatus |= (1<<keypad_info.key_id);
		}
		if(((g_ui32KeypadStatus&(1<<keypad_info.key_id))>>keypad_info.key_id == 1 && keypad_info.long_or_short_press == 3) ||
			((g_ui32KeypadStatus&(1<<keypad_info.key_id))>>keypad_info.key_id == 1 && keypad_info.press_or_release == 0))//按键没有处理过&&长按 或 按键没有处理过&&按键已释放
		{
			g_fcKeysCallback(keypad_info.key_id, keypad_info.long_or_short_press - 1);
			g_ui32KeypadStatus &= ~(1<<keypad_info.key_id);
			sysLOG(KEYS_LOG_LEVEL_5, "dealt key g_ui32KeypadStatus=%x, key_id=%d, %d, %d\r\n", g_ui32KeypadStatus, keypad_info.key_id, keypad_info.long_or_short_press, keypad_info.press_or_release);
			
		}
		else


		{
			//sysLOG(KEYS_LOG_LEVEL_1, "<WARN> release long key, g_ui32KeypadStatus=%x\r\n", g_ui32KeypadStatus);
		}
		
		
	}
}


/*
*Function:		hal_keypadDeal
*Description:	键盘注册处理接口包括关机PWRKEY在内的所有按键
*Input:			key_ID：按键ID；key_V：按键键值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_keypadDeal(uint8 key_ID, uint8 key_V)
{
	int iRet = -1;
	uint32 keyidtmp = 0xFF;
	sysLOG(KEYS_LOG_LEVEL_1, "hal_keypadDeal, key_ID:%d, key_V:%d\r\n", key_ID, key_V);

	hal_scrBackLightWakeup();

	if(key_ID == PWRKEY_ID)
	{
		g_ui8PwrkeyFlag = key_V;
	}
	
	if(key_V == 1)//短按
	{
		if(g_ui8KeySoundFlag != 0)
			sysBeepF_lib(6, 50);
		
		switch(key_ID)
		{

			case KEY_1_ID:
				keyidtmp = KEY1;
			break;
			case KEY_2_ID:
				keyidtmp = KEY2;
			break;
			case KEY_3_ID:
				keyidtmp = KEY3;
			break;
			case KEY_4_ID:
				keyidtmp = KEY4;
			break;
			case KEY_5_ID:
				keyidtmp = KEY5;
			break;
			case KEY_6_ID:
				keyidtmp = KEY6;
			break;
			case KEY_7_ID:
				keyidtmp = KEY7;
			break;
			case KEY_8_ID:
				keyidtmp = KEY8;
			break;
			case KEY_9_ID:
				keyidtmp = KEY9;
			break;
			case KEY_0_ID:
				keyidtmp = KEY0;
			break;
			case KEY_STAR_ID:	
				keyidtmp = KEYSTAR;	
			break;
			case KEY_SHARP_ID:	
				keyidtmp = KEYPOINT;	
			break;
			case KEY_CANCLE_ID:	
				keyidtmp = KEYCANCEL;	
			break;
			case KEY_CLEAR_ID:
				keyidtmp = KEYCLEAR;
			break;
			case KEY_ENTER_ID:
				keyidtmp = KEYENTER;
			break;
			case KEY_PLUS_ID:	
				keyidtmp = KEYPLUS;	
			break;
			case KEY_DCASH_ID:	
				keyidtmp = KEYF1;	
			break;
			case KEY_MENU_ID:			//Q390默认菜单键为功能键
				keyidtmp = KEYMENU;	
			break;
			// case PWRKEY_ID:			//Q390电源短按和取消冲突 保留取消功能	
			// 	keyidtmp = KEYPWR;	
			// break;
			default:
				sysLOG(KEYS_LOG_LEVEL_3, "<warn> Invalid key value,key_ID:%d\r\n", key_ID);
			break;
			
		}
		
		if(keyidtmp != 0xFF)
		{
			iRet = fibo_queue_put(g_ui32QueueKeyValue, &keyidtmp, 100);
			sysLOG(KEYS_LOG_LEVEL_4, "fibo_queue_put, iRet:%d\r\n", iRet);
		}
	}
	else if(key_V == 2)//长按
	{
		if(g_ui8KeySoundFlag != 0)
			sysBeepF_lib(6, 50);
		switch(key_ID)
		{
			case PWRKEY_ID:
			
				if(g_fcKeysCallbackPwroff != NULL)//有注册关机回调
				{
					sysLOG(KEYS_LOG_LEVEL_4, "g_fcKeysCallbackPwroff\r\n");
					g_fcKeysCallbackPwroff();
					return;
				}
				else//没有关机回调，就放队列中即可
				{
					keyidtmp = KEYPWROFF;
				}
			break;
			case KEY_CLEAR_ID:
				keyidtmp = KEYCLEAR_LONG;
			break;
			default:

			break;
		}
		if(keyidtmp != 0xFF)
		{
			iRet = fibo_queue_put(g_ui32QueueKeyValue, &keyidtmp, 100);
			sysLOG(KEYS_LOG_LEVEL_5, "fibo_queue_put, iRet:%d\r\n", iRet);
		}
		
	}
	
}

/*
*Function:		hal_keypadSetSound
*Description:	设置按键板在按键时是否发声
*Input:			flag:0-不发声;其他-发声
*Output:		NULL
*Hardware:
*Return:		0-成功；<0:失败
*Others:
*/
int hal_keypadSetSound(unsigned char flag)
{
	if(flag == 0)
		g_ui8KeySoundFlag = 0;
	else
		g_ui8KeySoundFlag = 1;

	return 0;
}

/*
*Function:		hal_keypadGetSound
*Description:	读取按键音状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-不发声；1-发声
*Others:
*/
int hal_keypadGetSound(void)
{
	return g_ui8KeySoundFlag;
}

static unsigned char g_ucKeyTable[12][10] = {
"0# \0", "1.,\0", "2ABCabc\0", "3DEFdef\0", "4GHIghi\0", "5JKLjkl\0", "6MNOmno\0",\
"7PQRSpqrs\0", "8TUVtuv\0", "9WXYZwxyz\0", "*\0", ".\0"};

static unsigned char keyindexrecord = 0;


/*
*Function:		hal_kbCashDeal
*Description:	处理带小数点的输入(金额)
*Input:			*input:输入的字符;*cashStr:之前读取到的字符,numIndex:已按键输入的数字键个数，不包括'.'
*Output:		*cashStr:输出转换后的字符串
*Hardware:
*Return:		本次按键所输入的数字个数，不包含'.'
*Others:
*/
static int hal_kbCashDeal(unsigned char *input, unsigned char *cashStr, unsigned char numIndex)
{
	int iRet = 0;
	unsigned char *rPStrtmp = NULL;
	static unsigned char ucPointInput = 0;//是否输入'.'
	static unsigned char writeindex = 0;//输入数字个数的序号
	static unsigned char PointWriteindex = 0;//输入"."后输入数字的个数
	
	rPStrtmp = malloc(128);
	if(rPStrtmp == NULL)
		goto EXIT;

	memset(rPStrtmp, 0, 128);

	if((*input < 0x30 || *input > 0x39) && *input != '.')
	{
		sysLOG(KEYS_LOG_LEVEL_5, "illegal input, *input=0x%x\r\n", *input);
		goto EXIT;
	}

	if(numIndex == 0)//第一次键盘输入
	{
		ucPointInput = 0;
		writeindex = 0;
		PointWriteindex = 0;
		sprintf(rPStrtmp, "0.00");
		memcpy(cashStr, rPStrtmp, strlen(rPStrtmp));
		sysLOG(KEYS_LOG_LEVEL_5, "numIndex == 0\r\n");
		
	}
	else
	{
		memcpy(rPStrtmp, cashStr, strlen(cashStr));
	}

	sysLOG(KEYS_LOG_LEVEL_5, "rPStrtmp:%s\r\n", rPStrtmp);

	if(numIndex == 0 && *input == 0x30)//没有输入其他之前输入0,不计数
	{
		writeindex = 0;
		sysLOG(KEYS_LOG_LEVEL_5, "*input == 0x30\r\n");
		goto EXIT;
	}

	if(ucPointInput == 0)//没有输入小数点时
	{
		if(*input >= 0x30 && *input <= 0x39)//正常输入数字
		{
			if(writeindex == 0)
			{
				memcpy(cashStr, rPStrtmp, strlen(rPStrtmp));
				*(cashStr+3) = *input;
			}
			else if(writeindex == 1)
			{
				memcpy(cashStr, rPStrtmp, strlen(rPStrtmp));
				*(cashStr+2) = *(rPStrtmp+3);
				*(cashStr+3) = *input;
			}
			else if(writeindex == 2)
			{
				*(cashStr+0) = *(rPStrtmp+2);
				*(cashStr+1) = '.';
				*(cashStr+2) = *(rPStrtmp+3);
				*(cashStr+3) = *input;
			}
			else if(writeindex >= 2)
			{
				memcpy(cashStr, rPStrtmp, strlen(rPStrtmp)-3);
				*(cashStr+(strlen(rPStrtmp)-3)) = *(rPStrtmp+strlen(rPStrtmp)-2);
				*(cashStr+(strlen(rPStrtmp)-2)) = '.';
				*(cashStr+(strlen(rPStrtmp)-1)) = *(rPStrtmp+strlen(rPStrtmp)-1);
				*(cashStr+(strlen(rPStrtmp))) = *input;
			}
			
			writeindex += 1;
			sysLOG(KEYS_LOG_LEVEL_5, "Normal input numbers cashStr:%s, writeindex=%d\r\n", cashStr, writeindex);
		}

		if(*input == '.')//第一次输入"."
		{
			
			if(writeindex == 0)
			{
				writeindex += 1;
			}
			else if(writeindex == 1)
			{
				*(cashStr) = *(rPStrtmp+strlen(rPStrtmp)-1);
				*(cashStr+1) = '.';
				*(cashStr+2) = 0x30;
				*(cashStr+3) = 0x30;
			}
			else if(writeindex == 2)
			{
				*(cashStr) = *(rPStrtmp+strlen(rPStrtmp)-2);
				*(cashStr+1) = *(rPStrtmp+strlen(rPStrtmp)-1);
				*(cashStr+2) = '.';
				*(cashStr+3) = 0x30;
				*(cashStr+4) = 0x30;

			}
			else if(writeindex >= 3)
			{
				memcpy(cashStr, rPStrtmp, strlen(rPStrtmp)-3);
				*(cashStr+(strlen(rPStrtmp)-3)) = *(rPStrtmp+strlen(rPStrtmp)-2);
				*(cashStr+(strlen(rPStrtmp)-2)) = *(rPStrtmp+strlen(rPStrtmp)-1);
				*(cashStr+(strlen(rPStrtmp)-1)) = '.';
				*(cashStr+(strlen(rPStrtmp)-0)) = 0x30;
				*(cashStr+(strlen(rPStrtmp)+1)) = 0x30;

				
			}
				
			ucPointInput = 1;
			sysLOG(KEYS_LOG_LEVEL_5, "first input . cashStr:%s, writeindex=%d\r\n", cashStr, writeindex);
		}
	}
	else if(ucPointInput == 1)//输入过小数点时
	{
		if(*input != '.')
		{
			if(PointWriteindex == 0)//输入小数点后第一位
			{
				memcpy(cashStr, rPStrtmp, strlen(rPStrtmp)-2);
				*(cashStr+(strlen(rPStrtmp)-2)) = *input;
				*(cashStr+(strlen(rPStrtmp)-1)) = 0x30;
				
				PointWriteindex += 1;
			}
			else if(PointWriteindex == 1)//输入小数点后第二位
			{
				memcpy(cashStr, rPStrtmp, strlen(rPStrtmp)-1);
				*(cashStr+(strlen(rPStrtmp)-1)) = *input;
				
				PointWriteindex += 1;
			}
			else//小数点后面只支持到2位
			{
				goto EXIT;
			}
			writeindex += 1;
		}
		sysLOG(KEYS_LOG_LEVEL_5, "already input . cashStr:%s, writeindex=%d\r\n", cashStr, writeindex);
	}
	

EXIT:

	iRet = writeindex;
	
	if(rPStrtmp != NULL)
		free(rPStrtmp);

	return iRet;
	
}

/*
*Function:		hal_kbGetChar
*Description:	读取本次按键的字符
*Input:			keyindex:按键ID;charindex:上一次按键的字符序号;ulsysTime:上次按键后累计到现在的超时时间+滴答时钟
*Output:		*outstr:读取到的字符
*Hardware:
*Return:		本次按键的字符序号
*Others:
*/
static unsigned char hal_kbGetChar(unsigned char keyindex, unsigned char charindex, unsigned char *outstr, unsigned long long ulsysTime)
{
	unsigned char iRet = 0;
	unsigned char lentmp = 0;

	lentmp = strlen(g_ucKeyTable[keyindex]);
	if(keyindexrecord != keyindex || hal_sysGetTickms() > (ulsysTime))
	{
		iRet = 0;
		*outstr = g_ucKeyTable[keyindex][iRet];
		sysLOG(KEYS_LOG_LEVEL_5, "1,*outstr=0x%x, iRet:%d\r\n", *outstr, iRet);
	}
	else
	{
		if((charindex+1) >= lentmp)
			iRet = 0;
		else
			iRet = charindex + 1;

		*outstr = g_ucKeyTable[keyindex][iRet];
		sysLOG(KEYS_LOG_LEVEL_5, "2,*outstr=0x%x, iRet:%d\r\n", *outstr, iRet);
		
	}
	
	sysLOG(KEYS_LOG_LEVEL_5, "keyindex=%d, keyindexrecord:%d\r\n", keyindex, keyindexrecord);
	
	keyindexrecord = keyindex;
	
	
	
	return iRet;
	
}

/*
*Function:		hal_kbGetString
*Description:	阻塞等待按键字符串输入，并以相应的模式显示在屏幕上
*Input:			ucMode:
*					D7 1(0) 保留
*					D6 1(0) 保留
*					D5 1(0) 是(否)输入字母
*					D4 1(0) 是(否)密文显示方式,显示为‘*’
*					D3D2 00-左对齐输入;01-居中对齐输入;10(11)-右对齐
*					D1 1(0) 有(否)小数点
*					D0 1(0) 正(反)显示
*				ucMinLen:需要输入串的最小长度
*				ucMaxLen:需要输入串的最大长度(最大允许值是 128 字节)。
*				usTimeOutSec:等待输入时间，单位（秒）当 timeoutsec等于0时，默认为120秒，最大等待输入时间为600秒，超过600秒按600秒算。
*Output:		*pucStr:读取到的字符串
*Hardware:
*Return:		0-成功;<0-失败
*Others:		1. 密文一律右对齐输入，有反显模式，不能是小数点模式，既可选mode为0x08或0x09。
*				2. 输出串中不记录和包含功能键。
*				3. 按下CLEAR键,如果是明文显示,变成退格的功能,如果是密文显示,清除整个输入，如果是小数点模式清除整个输入。
*				4. 小数点模式一律右对齐输入，长度最大12位（包括小数点后两位），不能是密文，有反显模式，既可选mode为0x02或0x03。
*/
int hal_kbGetString(unsigned char *pucStr, unsigned char ucMode, unsigned char ucMinLen, unsigned char ucMaxLen, unsigned short usTimeOutSec)
{
	
	int iRet;
	
	unsigned int  timeoutshorttmp = 1000;//同一个物理按键切换输出不同字符的最大时间间隔	
	unsigned char ucInType = 0;//0-数字;1-ASCII
	unsigned char ucModeTmp = 0;//显示模式设置
	unsigned char ucIsPWD = 0;//0-明文显示;1-密文显示
	unsigned char ucIsCash = 0;//0-非金额显示;1-金额显示
	unsigned char ucReverse = 1;//1-正显;0-反显
	unsigned char ucAlignmentType = 0;//0b00-左对齐，0b01-居中，0b10-右对齐
	unsigned long long uTime, timeouttmp;
	unsigned long long uTimeshort;
	unsigned char *ucPStrOut = NULL, *ucPStrDisplay = NULL;
	int iStrLen = 0;
	unsigned char charindextmp = 0;
	unsigned char chartmp = 0;
	unsigned char charrefresh = 0;
	unsigned char keyidlastpush = 0;
	unsigned int uiDispX = g_stCurPixel.x;
	unsigned int uiDispY = g_stCurPixel.y;
	unsigned char CashInputed = 0;
	
	if (pucStr == NULL || ucMinLen > ucMaxLen || ucMaxLen <= 0 || ucMaxLen > 128)
	{
		return UART_ERR_PARAMETER;
	}

	ucPStrOut = malloc(128);
	if(ucPStrOut == NULL)
		goto END;
	
	ucPStrDisplay = malloc(128);
	if(ucPStrDisplay == NULL)
		goto END;

	memset(ucPStrOut, 0, 128);
	memset(ucPStrDisplay, 0, 128);
	
	if(usTimeOutSec == 0)
		timeouttmp = 120*1000;
	else if(usTimeOutSec > 600)
		timeouttmp = 600*1000;
	else timeouttmp = usTimeOutSec*1000;

	if(ucMode & (0x01 << 5))
		ucInType = 1;
	
	if(ucMode & (0x01 << 4))
		ucIsPWD = 1;
	
	if(ucMode & (0x01 << 1))
		ucIsCash = 1;
	
	if(ucMode & 0x01)
		ucReverse = 0;
	
	if((ucMode & 0x0C) >> 2 == 0b11)
		ucAlignmentType = 0b10;
	else ucAlignmentType = (ucMode & 0x0C) >> 2;

	if(ucIsCash == 1 || ucIsPWD == 1)
	{
		ucAlignmentType = 0b10;
	}

	if(ucIsCash == 1 && ucIsPWD == 1)//密文不能是小数点模式
	{
		iRet = UART_ERR_PARAMETER;
		goto END;
	}

	if(ucIsCash == 1 && ucInType == 1)//小数点模式只能有数字输入模式
	{
		iRet = UART_ERR_PARAMETER;
		goto END;
	}

	if(ucIsCash == 1)
	{
		ucInType = 0;
	}

	if(ucInType == 0)
	{
		timeoutshorttmp = 0;
	}

	ucModeTmp = (ucAlignmentType << 1) | (ucReverse << 7);

	if(ucIsCash == 1)
	{
		hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp, "0.00");
	}
	

	hal_keypadFlush();

	keyindexrecord = 0;
	charindextmp = 0;
	uTime = hal_sysGetTickms() + timeouttmp;
	uTimeshort = hal_sysGetTickms() + timeoutshorttmp;
	
	while(1)
	{
		if(hal_sysGetTickms() > uTime)
		{
			sysLOG(KEYS_LOG_LEVEL_1, "<WARN> Timeout.\n");
			break;
		}
		if(charrefresh == 1 && hal_sysGetTickms() > uTimeshort)//按键的字符没有更新并且时间已超时，则更新
		{
			iStrLen += 1;
			charrefresh = 0;

			if(ucIsPWD)
				hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp, ucPStrDisplay);//输入密码时，按键超时后更新为*
		}
		
		if(hal_keypadHit())
		{
			sysDelayMs(100);
			continue;
		}

		iRet = hal_keypadGetKey();
		if(iRet == 0xFF)
		{
			sysDelayMs(100);
			continue;
		}
		sysLOG(KEYS_LOG_LEVEL_1, "hal_keypadGetKey, iRet:%d\r\n", iRet);

		if(charrefresh == 1 && keyidlastpush != iRet)//按键的字符没有更新并且本次按键键值和上一次的按键键值不是同一个，则更新
		{
			iStrLen += 1;
			charrefresh = 0;
		}

		keyidlastpush = iRet;
		
		switch(iRet)
		{
			case KEYCANCEL:
				
				if(iStrLen > 0)
				{
					memset(ucPStrOut, ' ', iStrLen);
					hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp, ucPStrOut);
				}
				
				iStrLen = 0;
				memset(ucPStrOut, 0x00, 128);
				g_stCurPixel.x = uiDispX;
				g_stCurPixel.y = uiDispY;
				hal_keypadFlush();
				goto END;
				
			break;
			
			case KEYENTER:
				
				hal_keypadFlush();
				goto END;
			
			break;
			
			case KEYCLEAR:
				
				if(ucIsPWD == 1 || ucIsCash == 1)//清除输入
				{
					if(iStrLen > 0)
					{
						memset(ucPStrOut, ' ', iStrLen);
						hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp & ~(0x01<<7), ucPStrOut);
						if(ucIsCash == 1)
						{
							CashInputed = 0;
							hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp, "0.00");
						}
					}
					
					iStrLen = 0;
					memset(ucPStrOut, 0x00, 128);
					g_stCurPixel.x = uiDispX;
					g_stCurPixel.y = uiDispY;
				}
				else//退格
				{
					if(iStrLen > 0)
					{
						memset(ucPStrDisplay, 0, 128);
						memset(ucPStrDisplay, ' ', iStrLen);
						iStrLen -= 1;
						hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp & ~(0x01<<7), ucPStrDisplay);
						
					}
					ucPStrOut[iStrLen] = 0x00;
					hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp, ucPStrOut);
					
				}
				
			break;
			
			case KEY0:
			case KEY1:
			case KEY2:
			case KEY3:
			case KEY4:
			case KEY5:
			case KEY6:
			case KEY7:
			case KEY8:
			case KEY9:
				
				iRet = hal_kbGetChar(iRet-0x30, charindextmp, &chartmp, uTimeshort);
				charindextmp = iRet;
				uTimeshort = hal_sysGetTickms() + timeoutshorttmp;

				if(ucIsCash == 1)
				{
					CashInputed = hal_kbCashDeal(&chartmp, ucPStrOut, CashInputed);
				}
				else
				{
					ucPStrOut[iStrLen] = chartmp;
				}
					
				charrefresh = 1;
					
			break;

			case KEYSTAR:
				
				iRet = hal_kbGetChar(10, charindextmp, &chartmp, uTimeshort);
				charindextmp = iRet;
				uTimeshort = hal_sysGetTickms() + timeoutshorttmp;
				ucPStrOut[iStrLen] = chartmp;
				charrefresh = 1;

			break;

			case KEYPOINT:
					
				iRet = hal_kbGetChar(11, charindextmp, &chartmp, uTimeshort);
				charindextmp = iRet;
				uTimeshort = hal_sysGetTickms() + timeoutshorttmp;

				if(ucIsCash == 1)
				{
					CashInputed = hal_kbCashDeal(&chartmp, ucPStrOut, CashInputed);
				}
				else
				{
					ucPStrOut[iStrLen] = chartmp;
				}
				
				charrefresh = 1;

			break;
				
			case KEYF1:
			case KEYMENU:
			case KEYPLUS:
			case KEYPWR:
				

			break;
			
			default:
			
				continue;
				
			break;
				
		}

		if(strlen(ucPStrOut) > 0)
		{
			if(ucIsPWD)
			{
				memset(ucPStrDisplay, 0, 128);
				memset(ucPStrDisplay, '*', strlen(ucPStrOut));
				ucPStrDisplay[iStrLen] = ucPStrOut[iStrLen];
			}
			else
			{
				memset(ucPStrDisplay, 0, 128);
				memcpy(ucPStrDisplay, ucPStrOut, strlen(ucPStrOut));
			}
			
			hal_scrPrintxy(uiDispX, uiDispY, ucModeTmp, ucPStrDisplay);

			if(ucIsPWD)
				ucPStrDisplay[iStrLen] = '*';

		}
		
		sysLOG(KEYS_LOG_LEVEL_4, "x=%d, y=%d, ucPStrOut:%s,ucPStrDisplay:%s\r\n", uiDispX, uiDispX, ucPStrOut, ucPStrDisplay);
		
		uTime = hal_sysGetTickms() + timeouttmp;

		
	}

END:
	
	iRet = 0;

	if(iStrLen > 0)
		memcpy(pucStr, ucPStrOut, strlen(ucPStrOut));
	
	if(ucPStrOut != NULL)
		free(ucPStrOut);
	
	if(ucPStrDisplay != NULL)
		free(ucPStrDisplay);

	sysLOG(KEYS_LOG_LEVEL_1, "pucStr:%s, iRet:%d\r\n", pucStr, iRet);
	
	return iRet;

}

/*
*Function:		hal_keypadWaitOneKey
*Description:	阻塞等待按键事件，并读取键值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		读到的键值
*Others:
*/
unsigned char hal_keypadWaitOneKey(void)
{
	hal_keypadFlush();
	
    while(hal_keypadHit())
    {
       sysDelayMs(10); 
    }
    
    return hal_keypadGetKey();
}

//菜单中关机接口
void keypwrofftest(void)
{
	hal_ttsQueuePlay("正在关机", NULL, NULL, 0);
	hal_pmPwrOFF();
}


/************************************TEST************************************/

#if MAINTEST_FLAG

void keytest(void)
{
	int iRet = -1;
	uint32 keyvalue;
	uint8 bufftmp[512];
	iRet = hal_keypadHit();
	//sysLOG(KEYS_LOG_LEVEL_1, "hal_keypadHit iRet=%d\r\n", iRet);
	if(iRet == 0)
	{
		sysLOG(KEYS_LOG_LEVEL_1, "hal_keypadCheck\r\n");

        //USB_ThreadEntry();
		//gmSm2Test();
		//pedTest();
		//apiSysTest();
		//sysDelayMs(2000);
		//apiPiccTest();
		//apiSysTest();

		//PED_task();

		keyvalue = hal_keypadGetKey();
		if(keyvalue != 0xFF)
		{
			sysLOG(KEYS_LOG_LEVEL_1, "<SUCC> hal_keypadGetKey keyvalue:%x\r\n", keyvalue);
			switch(keyvalue)
			{
				case KEYPWR:
					iRet = hal_pmBatGetValue();
					if(iRet <0)
					{
						hal_ttsQueuePlay("充电中", NULL, NULL, 0);
					}
					else
					{
						int8 buf[256];
						memset(buf, 0, sizeof(buf));
						sprintf(buf, "电量百分之 %d\r\n", iRet);
						hal_ttsQueuePlay(buf, NULL, NULL, 0);
					}
				break;
				case KEY0:
			
					hal_ttsQueuePlay("0", NULL, NULL, 0);
					hal_portOpen(10, NULL);
				break;
				case KEY1:
			
					hal_ttsQueuePlay("1", NULL, NULL, 0);
					hal_portClose(10);
				break;
				case KEY2:
			
					hal_ttsQueuePlay("2", NULL, NULL, 0);
					hal_portFlushBuf(10);
				break;
				case KEY3:
			
					hal_ttsQueuePlay("3", NULL, NULL, 0);
					memset(bufftmp, 0, sizeof(bufftmp));
					iRet = hal_portRecvs(10, bufftmp, 64, 500);
					sysLOG(KEYS_LOG_LEVEL_1, "hal_portRecvs iRet=%d, %s\r\n", iRet, bufftmp);
				break;
				case KEY4:
			
					hal_ttsQueuePlay("4", NULL, NULL, 0);
					memset(bufftmp, 0, sizeof(bufftmp));
					sprintf(bufftmp, "0123456789abc,hello world!\r\n");
					iRet = hal_portSends(10, bufftmp, strlen(bufftmp));
				break;
				case KEY5:
			
					hal_ttsQueuePlay("5", NULL, NULL, 0);
				break;
				case KEY6:
			
					hal_ttsQueuePlay("6", NULL, NULL, 0);
				break;
				case KEY7:
			
					hal_ttsQueuePlay("7", NULL, NULL, 0);
				break;
				case KEY8:
			
					hal_ttsQueuePlay("8", NULL, NULL, 0);
				break;
				case KEY9:
			
					hal_ttsQueuePlay("9", NULL, NULL, 0);
				break;
				case KEYENTER:
			
					hal_ttsQueuePlay("确认", NULL, NULL, 0);
				break;
				case KEYCLEAR:
			
					hal_ttsQueuePlay("清除", NULL, NULL, 0);
				break;
				case KEY_FN:
			
					hal_ttsQueuePlay("功能", NULL, NULL, 0);
				break;
				case KEYSTAR:
			
					hal_ttsQueuePlay("星", NULL, NULL, 0);
				break;
				case KEYSHARP:
			
					hal_ttsQueuePlay("井", NULL, NULL, 0);
				break;
				case KEYPOINT:
			
					hal_ttsQueuePlay("点", NULL, NULL, 0);
				break;
				case KEYCANCEL:
			
					hal_ttsQueuePlay("取消", NULL, NULL, 0);
				break;
				case KEYCLEAR_LONG:

					hal_ttsQueuePlay("清除长按", NULL, NULL, 0);
				break;
				case KEYDCASH:

					hal_ttsQueuePlay("数币", NULL, NULL, 0);
				break;
				case KEYF1:

					hal_ttsQueuePlay("功能1", NULL, NULL, 0);
				break;
				case KEYMENU:

					hal_ttsQueuePlay("菜单", NULL, NULL, 0);
				break;
				case KEYPLUS:

					hal_ttsQueuePlay("加", NULL, NULL, 0);
				break;
				
			}
		}
	}
}


int kbFlushEx(void)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 6;
    int lenth;
	unsigned char ucCmdHead[6] = {0x00, 0xF4, 0x01, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(KEYS_LOG_LEVEL_1, "  ucCmd = %s\r\n", caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) 
    {
        lenth = retfrm.data[4]|retfrm.data[5];
        if(lenth == 1)
            iRet = retfrm.data[6];
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(KEYS_LOG_LEVEL_1, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}


#endif


