/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_pwrkey.c
** Description:		电源按键相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"



/*
*Function:		hal_pkeyInit
*Description:	电源按键初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pkeyInit(void)
{
	int iRet = -1;
	iRet = fibo_pwrkeypad_config(KEYLONGTIMEOUT, FALSE);
	sysLOG(PKEY_LOG_LEVEL_4, "fibo_pwrkeypad_config iRet=%d\r\n", iRet);
}


/*
*Function:		hal_pkeyHandle
*Description:	PWRKEY按键数据处理句柄，
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pkeyHandle(void)
{
	int iRet = -1;
	keypad_info_t Pwrkey_info;

	Pwrkey_info = fibo_get_pwrkeypad_status();
	
	if(Pwrkey_info.long_or_short_press == 2 && Pwrkey_info.press_or_release == 1)//按下
	{
		g_ui32KeypadStatus |= (1<<Pwrkey_info.key_id);
		sysLOG(PKEY_LOG_LEVEL_4, "<SUCC> Press key g_ui32KeypadStatus=%x\r\n", g_ui32KeypadStatus);
	}
	if(((g_ui32KeypadStatus&(1<<Pwrkey_info.key_id))>>Pwrkey_info.key_id == 1 && Pwrkey_info.long_or_short_press == 3) ||
		((g_ui32KeypadStatus&(1<<Pwrkey_info.key_id))>>Pwrkey_info.key_id == 1 && Pwrkey_info.press_or_release == 0))//按键没有处理过&&长按 或 按键没有处理过&&按键已释放
	{
		g_fcKeysCallback(Pwrkey_info.key_id, Pwrkey_info.long_or_short_press - 1);
		g_ui32KeypadStatus &= ~(1<<Pwrkey_info.key_id);
		sysLOG(PKEY_LOG_LEVEL_4, "dealt key g_ui32KeypadStatus=%x, key_id=%d, %d, %d\r\n", g_ui32KeypadStatus, Pwrkey_info.key_id, Pwrkey_info.long_or_short_press, Pwrkey_info.press_or_release);
		
	}
	else
	{
		//sysLOG(KEYS_LOG_LEVEL_1, "<WARN> release long key, g_ui32KeypadStatus=%x\r\n", g_ui32KeypadStatus);
	}



	
	
	
}


/*
*Function:		hal_pkeyDeal
*Description:	PWRKEY按键注册处理接口，主要用在关机充电时使用
*Input:			key_ID：按键ID；key_V：按键键值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pkeyDeal(uint8 key_ID, uint8 key_V)
{
	uint32 keyidtmp = 0xFF;
	sysLOG(PKEY_LOG_LEVEL_2, "hal_pkeyDeal, key_ID:%d, key_V:%d\r\n", key_ID, key_V);
	/*充电时按任意键即可点亮屏幕背光*/
	g_iBackLightCnt = 1;
	hal_scrSetBackLightMode(2, 10);

	if(key_V == 1)//短按
	{
		switch(key_ID)
		{
				
			case PWRKEY_ID:
				keyidtmp = KEYCANCEL;
			break;

			default:

			break;
			

		}
		if(keyidtmp != 0xFF)
		{
			fibo_queue_put(g_ui32QueueKeyValue, &keyidtmp, 100);
		}
	}
	else if(key_V == 2)//长按
	{

		if(key_ID == PWRKEY_ID)
		{

			g_ui8SystemStatus = 2;//启动开机流程
		}
		
	}
	
}



