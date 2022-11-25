/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_led.c
** Description:		LED相关接口
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


static uint8 ui8LedRedFlag=0, ui8LedBlueFlag=0, ui8LedYellowFlag=0;
static uint8 ui8LedRedFlagLast=0, ui8LedBlueFlagLast=0, ui8LedYellowFlagLast=0;
static uint8 ui8LedRedEn = 0, ui8LedBlueEn = 0, ui8LedYellowEn = 0;
static uint8 ui8LedRedcnt=0, ui8LedBluecnt=0, ui8LedYellowcnt=0;
static long long ui64LedRedLoopcnt = 0, ui64LedBlueLoopcnt = 0, ui64LedYellowLoopcnt = 0;

int g_iLed_exist = 0; 				//0-无，	1-有 
int g_iTricolorLED_exist = 0; 		//0-无，	1-有 三色灯

/*
*Function:		hal_ledInit
*Description:	LED初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ledInit(void)
{

	fibo_gpio_mode_set(LED_RED_GPIO, GpioFunction5);
	fibo_gpio_cfg(LED_RED_GPIO, GpioCfgOut);
	fibo_gpio_set(LED_RED_GPIO, FALSE);
	
	fibo_gpio_mode_set(LED_BLUE_GPIO, GpioFunction4);
	fibo_gpio_cfg(LED_BLUE_GPIO, GpioCfgOut);
	fibo_gpio_set(LED_BLUE_GPIO, false);

	fibo_gpio_mode_set(LED_YELLOW_GPIO, GpioFunction4);
	fibo_gpio_cfg(LED_YELLOW_GPIO, GpioCfgOut);
	fibo_gpio_set(LED_YELLOW_GPIO, false);

	hal_ledCtl(LEDRED, FALSE);
	hal_ledCtl(LEDBLUE, FALSE);
	hal_ledCtl(LEDYELLOW, FALSE);
	
}

/*
*Function:		hal_ledCtl
*Description:	LED控制
*Input:			ucLedIndex:灯的索引值，参考LED_INDEX_ID;ucOnOff:灯的亮灭，0-灭，1-亮
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_ledCtl(uchar ucLedIndex,uchar ucOnOff)
{

	int iRet = -1;

	if(ucOnOff != 0 && ucOnOff != 1)
	{
		return iRet;
	}
	if(ucLedIndex < LEDRED || ucLedIndex > LEDYELLOW)
	{
		return iRet;
	}
	
	switch(ucLedIndex){

		case LEDRED:
			
			fibo_gpio_set(LED_RED_GPIO, ucOnOff);
			
		break;

		case LEDBLUE:
			iRet = fibo_gpio_set(LED_BLUE_GPIO, ucOnOff);
		break;

		case LEDYELLOW:

			iRet = fibo_gpio_set(LED_YELLOW_GPIO, ucOnOff);
			
		break;

		default:
			
		break;

	}
	if(iRet > 0)
		iRet = 0;

	return iRet;
}


/*
*Function:		hal_ledRedHandle
*Description:	Led Red灯执行句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		必须每100ms循环调用一次
*/
void hal_ledRedHandle(void)
{
	if(ui8LedRedEn == 1)
	{
		if(ui8LedRedFlag == 0)//长灭
		{
			
			hal_ledCtl(LEDRED, FALSE);
		}
		else if(ui8LedRedFlag == 1)//常亮
		{
			
			hal_ledCtl(LEDRED, TRUE);
		}
		else if(ui8LedRedFlag == 2)//低频
		{
			
			if(ui8LedRedcnt == 0)
			{			
				hal_ledCtl(LEDRED, TRUE);						
			}
			else if(ui8LedRedcnt == 1)
			{
				
				hal_ledCtl(LEDRED, FALSE);
				ui64LedRedLoopcnt -= 1;
				if(0 == ui64LedRedLoopcnt)
					ui8LedRedEn = 0;
				
			}
			ui8LedRedcnt++;
			if(ui8LedRedcnt > 9)ui8LedRedcnt = 0;	
			
		}
		else if(ui8LedRedFlag==3)//高频
		{
			
			if(ui8LedRedcnt == 0)
			{		
				hal_ledCtl(LEDRED, TRUE);				
			}
			else if(ui8LedRedcnt == 1)
			{
				
				hal_ledCtl(LEDRED, FALSE);
				ui64LedRedLoopcnt -= 1;
				if(0 == ui64LedRedLoopcnt)
					ui8LedRedEn = 0;
			}
			ui8LedRedcnt++;
			if(ui8LedRedcnt > 4)ui8LedRedcnt = 0;
			
		}	
	}
}



/*
*Function:		hal_ledBlueHandle
*Description:	Led Blue灯执行句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		必须每100ms循环调用一次
*/
void hal_ledBlueHandle(void)
{
	if(ui8LedBlueEn == 1)
	{
		if(ui8LedBlueFlag == 0)//长灭
		{
			
			hal_ledCtl(LEDBLUE, FALSE);
		}
		else if(ui8LedBlueFlag == 1)//常亮
		{
			
			hal_ledCtl(LEDBLUE, TRUE);
		}
		else if(ui8LedBlueFlag == 2)//低频
		{
		
			if(ui8LedBluecnt == 0)
			{		
				hal_ledCtl(LEDBLUE, TRUE);					
			}
			else if(ui8LedBluecnt == 1)
			{
				
				hal_ledCtl(LEDBLUE, FALSE);
				ui64LedBlueLoopcnt -= 1;
				if(0 == ui64LedBlueLoopcnt)
					ui8LedBlueEn = 0;
				
			}
			ui8LedBluecnt++;
			if(ui8LedBluecnt > 9)ui8LedBluecnt = 0;
			
		}
		else if(ui8LedBlueFlag==3)//高频
		{
		
			if(ui8LedBluecnt == 0)
			{		
				hal_ledCtl(LEDBLUE, TRUE);						
			}
			else if(ui8LedBluecnt == 1)
			{
				
				hal_ledCtl(LEDBLUE, FALSE);
				ui64LedBlueLoopcnt -= 1;
				if(0 == ui64LedBlueLoopcnt)
					ui8LedBlueEn = 0;
			}
			ui8LedBluecnt++;
			if(ui8LedBluecnt > 4)ui8LedBluecnt = 0;
		
		}
	}
}


/*
*Function:		hal_ledYellowHandle
*Description:	Led Red灯执行句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		必须每100ms循环调用一次
*/
void hal_ledYellowHandle(void)
{
	if(ui8LedYellowEn == 1)
	{
		if(ui8LedYellowFlag == 0)//长灭
		{
			
			hal_ledCtl(LEDYELLOW, FALSE);
		}
		else if(ui8LedYellowFlag == 1)//常亮
		{
			
			hal_ledCtl(LEDYELLOW, TRUE);
		}
		else if(ui8LedYellowFlag == 2)//低频
		{
		
			if(ui8LedYellowcnt == 0)
			{		
				hal_ledCtl(LEDYELLOW, TRUE);
			}
			else if(ui8LedYellowcnt == 1)
			{
				
				hal_ledCtl(LEDYELLOW, FALSE);
				ui64LedYellowLoopcnt -= 1;
				if(0 == ui64LedYellowLoopcnt)
					ui8LedYellowEn = 0;
				
			}
			ui8LedYellowcnt++;
			if(ui8LedYellowcnt > 9)ui8LedYellowcnt = 0;
			
		}
		else if(ui8LedYellowFlag==3)//高频
		{
		
			if(ui8LedYellowcnt == 0)
			{	
				
				hal_ledCtl(LEDYELLOW, TRUE);
								
			}
			else if(ui8LedYellowcnt == 1)
			{
				
				hal_ledCtl(LEDYELLOW, FALSE);
				ui64LedYellowLoopcnt -= 1;
				if(0 == ui64LedYellowLoopcnt)
					ui8LedYellowEn = 0;
			}
			ui8LedYellowcnt++;
			if(ui8LedYellowcnt > 4)ui8LedYellowcnt = 0;
		
		}
	}
}


/*
*Function:		hal_ledRun
*Description:	控制LED灯闪烁
*Input:			led_id:灯的索引值，参考LED_INDEX_ID;led_state:0-长灭；1-常亮；2-低频闪；3-高频闪；0xFF-释放底层对灯的控制
*				count:0：一直闪烁,其他：闪烁次数
*Output:		NULL
*Hardware:
*Return:		TRUE-成功；FALSE-失败
*Others:		
*/
BOOL hal_ledRun(uint8 led_id,uint8 led_state, int count)
{

	if(led_id < LEDRED || led_id > LEDYELLOW)
	{
		return FALSE;
	}
	
	ui64LedRedLoopcnt = 0xFFFFFFFFFFFFFFFF;
	ui64LedBlueLoopcnt = 0xFFFFFFFFFFFFFFFF;
	ui64LedYellowLoopcnt = 0xFFFFFFFFFFFFFFFF;
	
	switch(led_id)
	{
		case LEDRED:
			if(led_state == 0xFF)
			{
				ui8LedRedEn = 0;
				
				ui8LedRedFlag = led_state;
				ui8LedRedFlagLast=ui8LedRedFlag;
				hal_ledCtl(LEDRED, FALSE);
			}
			else
			{
				
				ui8LedRedFlag=led_state;
				if(ui8LedRedFlag==0)//长灭
				{
					ui8LedRedFlagLast=ui8LedRedFlag;
					hal_ledCtl(LEDRED, FALSE);
				}
				else if(ui8LedRedFlag==1)//常亮
				{
					ui8LedRedFlagLast=ui8LedRedFlag;
					hal_ledCtl(LEDRED, TRUE);
				}
				else if(ui8LedRedFlag>=2)//频闪
				{
					ui8LedRedEn = 1;
					if(count != 0)
						ui64LedRedLoopcnt = count;
					
					if(ui8LedRedFlagLast!=ui8LedRedFlag)
					{
						hal_ledCtl(LEDRED, FALSE);
						ui8LedRedFlagLast=ui8LedRedFlag;
					}
					
				}
			}
			return TRUE;
		break;

		case LEDBLUE:
			if(led_state == 0xFF)
			{
				ui8LedBlueEn = 0;
				
				ui8LedBlueFlag = led_state;
				ui8LedBlueFlagLast=ui8LedBlueFlag;
				hal_ledCtl(LEDBLUE, FALSE);
			}
			else
			{
				
				ui8LedBlueFlag=led_state;
				if(ui8LedBlueFlag==0)//长灭
				{
					ui8LedBlueFlagLast=ui8LedBlueFlag;
					hal_ledCtl(LEDBLUE, FALSE);
				}
				else if(ui8LedBlueFlag==1)//常亮
				{
					ui8LedBlueFlagLast=ui8LedBlueFlag;
					hal_ledCtl(LEDBLUE, TRUE);
				}
				else if(ui8LedBlueFlag>=2)//频闪
				{
					ui8LedBlueEn = 1;
					if(count != 0)
						ui64LedBlueLoopcnt = count;
					
					if(ui8LedBlueFlagLast!=ui8LedBlueFlag)
					{
						hal_ledCtl(LEDBLUE, FALSE);
						ui8LedBlueFlagLast=ui8LedBlueFlag;
					}
					
				}
			}
			return TRUE;
		break;

		case LEDYELLOW:
			if(led_state == 0xFF)
			{
				ui8LedYellowEn = 0;
				
				ui8LedYellowFlag = led_state;
				ui8LedYellowFlagLast=ui8LedYellowFlag;
				hal_ledCtl(LEDYELLOW, FALSE);
			}
			else
			{
				
				ui8LedYellowFlag=led_state;
				if(ui8LedYellowFlag==0)//长灭
				{
					ui8LedYellowFlagLast=ui8LedYellowFlag;
					hal_ledCtl(LEDYELLOW, FALSE);
				}
				else if(ui8LedYellowFlag==1)//常亮
				{
					ui8LedYellowFlagLast=ui8LedYellowFlag;
					hal_ledCtl(LEDYELLOW, TRUE);
				}
				else if(ui8LedYellowFlag>=2)//频闪
				{
					ui8LedYellowEn = 1;
					if(count != 0)
						ui64LedYellowLoopcnt = count;
					
					if(ui8LedYellowFlagLast!=ui8LedYellowFlag)
					{
						hal_ledCtl(LEDYELLOW, FALSE);
						ui8LedYellowFlagLast=ui8LedYellowFlag;
					}
					
				}
			}
			return TRUE;
		break;
		
		default:
			return FALSE;
		break;


	}
}



