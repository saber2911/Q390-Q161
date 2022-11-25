/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_icon.c
** Description:		黑白屏图标相关接口
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



LCD_ICON_DISP     g_stIconDisp;

int g_iIconBatteryTmp = 0, g_iIconWiFiTmp = 0, g_iIconGPRSTmp = 0;//缓存电量、WiFi信号、GPRS信号
int g_iIconLoopCnt = 0;

static uint8 aui8iconModeTmp[MAX_ICON] = {0};//如果图标读取结果失败，按照上一次的结果显示

/*
*Function:		hal_iconSelectIconMode
*Description:	图标模式选择
*Input:			icon_no:图标序号；mode:图标模式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
int hal_iconSelectIconMode(ICON_ORDER icon_no,unsigned int mode)
{

    switch(icon_no)
    {
        int i;
        
        case BATTERY_ICON:
            
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;//LCD_COLOR_BLACK;  
            g_stIconDisp.UseFlag[icon_no]= mode;
          	//sysLOG(ICON_LOG_LEVEL_2, "mode:%d\r\n", mode);
            if (mode == 6){           
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_batterycharge12X12;}           
            else if (mode == 5){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery12X12_0;}
            else if (mode == 4){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery12X12_4;}
            else if (mode == 3){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery12X12_3;}
            else if (mode == 2){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery12X12_2;}
            else if (mode == 1){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery12X12_1;}
            else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery12X12_4;
                g_stIconDisp.usColor[icon_no]= ICON_GRAY; } 
            else 
                return -2;
            break;
        case GPRS_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
        
            if (mode == 1){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_0;
				g_stIconDisp.usColor[icon_no]= ICON_GRAY;}
            else if (mode == 2){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_1;}
            else if (mode == 3){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_2;}
            else if (mode == 4){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_3;}
            else if (mode == 5){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_4;}
           else if (mode == 6){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_5;}
           else if((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs12X12_5;
                g_stIconDisp.usColor[icon_no]= ICON_GRAY;}       
            else 
                return -2;
            break;
        case BT_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_bt12X12;}
            else if (mode ==2){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_bt_n12X12;
				g_stIconDisp.usColor[icon_no]= ICON_GRAY;}
            else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_bt12X12;
                g_stIconDisp.usColor[icon_no]= ICON_GRAY;} 
            else 
                return -2;
            break;
        case WIFI_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi12X12_1;}
            else if (mode == 2){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi12X12_2;}
            else if (mode == 3){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi12X12_3;}
            else if (mode == 4){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi12X12_4;}
			 else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi12X12_4;
                g_stIconDisp.usColor[icon_no]= ICON_GRAY;} 
            else 
                return -2;
            break;
        case USB_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
            if (mode == 1)
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_usb12X12;
			 else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_usb12X12;
                g_stIconDisp.usColor[icon_no]= ICON_GRAY;} 			
            else 
                return -2;
            break;
        case ICCARD_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_ic12X12;}
			else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_ic12X12;
				g_stIconDisp.usColor[icon_no]= ICON_GRAY;}			
            else 
                return -2;
            break;
        case LOCK_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_lock12X12;}
			else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_lock12X12;
				g_stIconDisp.usColor[icon_no]= ICON_GRAY;}		
            else 
                return -2;
            break;
		case GPRSSIG_ICON:
            g_stIconDisp.usColor[icon_no]= g_stLcdGUI.icon_attr.iconColor;
            g_stIconDisp.UseFlag[icon_no]=mode;
            if (mode == 1)
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs6X12_S;
			 else if ((mode == 0xFF)||(mode == 0)){
                g_stIconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs6X12_S;
                g_stIconDisp.usColor[icon_no]= ICON_GRAY;} 			
            else 
                return -2;
            break;
        default:
            return -1;      
        
    }
 
	return 0;
		
}


/*
*Function:		hal_iconRefresh
*Description:	图标刷新
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_iconRefresh(void)
{

	unsigned char * imageAddr;
	unsigned char i;
	unsigned short w,h;
	int x,y;
	
	//1.查询是否需要刷新
	for(i=0;i<MAX_ICON;i++)
	{
		if(g_stIconDisp.UseFlag[i] != g_stIconDisp.g_ucIconDispMode[i])
		{
			sysLOG(ICON_LOG_LEVEL_5, "i=%d, g_stIconDisp.UseFlag[i]=%d, g_stIconDisp.g_ucIconDispMode[i]=%d\r\n", i, g_stIconDisp.UseFlag[i], g_stIconDisp.g_ucIconDispMode[i]);
			//calc the pos

			imageAddr = g_stIconDisp.iconBuff[i];
			w = 12;
			h = 12;
			y = 0;
//			x = 128-(i+1)*12; 
//			x = x<0?0:x;
			switch(i)
			{
				case BATTERY_ICON:
					x = g_stLcdConfig.ICON_BATTERY_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case GPRS_ICON:
					x = g_stLcdConfig.ICON_GPRS_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case BT_ICON:
					x = g_stLcdConfig.ICON_BT_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case WIFI_ICON:
					x = g_stLcdConfig.ICON_WIFI_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case USB_ICON:
					x = g_stLcdConfig.ICON_USB_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case ICCARD_ICON:
					x = g_stLcdConfig.ICON_ICCARD_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case LOCK_ICON:
					x = g_stLcdConfig.ICON_LOCK_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				case GPRSSIG_ICON:
					x = g_stLcdConfig.ICON_GPRSSIG_X;
					w = 6;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
				break;
				default:

				break;
			}
			
			//set the color
			//hal_setIconColor(g_stIconDisp.usColor[i]);
			//send the data to lcd	
			sysLOG(ICON_LOG_LEVEL_5, "i=%d, x=%d, y=%d, w=%d, h=%d, g_stIconDisp.usColor[i]=%d, imageAddr+20:%x\r\n", i, x, y, w, h, g_stIconDisp.usColor[i], *(imageAddr+20));
			
			//hal_scrWriteIcon(x, y, imageAddr, w, h, g_stIconDisp.usColor[i]);
			
			//update the record info
			g_stIconDisp.g_ucIconDispMode[i] = g_stIconDisp.UseFlag[i];
		}
	}

}



/*
*Function:		hal_iconLoop
*Description:	图标Loop调用接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_iconLoop(void)
{
	int iRet = -1;

	int rssitmp, bertmp;
	RTC_time rtctimetmp;
	uint8 rtctimebuf[16];
	uint8 simstatustmp = 0;

	static uint8 fullscreenflag = 0;
	
	if(g_ui8FullScreen == 1)
	{
		if(fullscreenflag != g_ui8FullScreen)
			fullscreenflag = g_ui8FullScreen;
		return;
	}
	else if(g_ui8FullScreen == 0)
	{
		if(fullscreenflag != g_ui8FullScreen)//如果退出全屏，则需要将图标全部刷新下
		{
			fullscreenflag = g_ui8FullScreen;

			hal_scrIconClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, 11);
			for(uint8 i=0; i<(MAX_ICON); i++)
			{
				g_stIconDisp.g_ucIconDispMode[i] = 0xFF;
			}
		}
	}

	g_iIconLoopCnt ++;
	if(g_iIconLoopCnt >= 10)
	{
		g_iIconLoopCnt = 0;
	}
	
/*************刷新RTC图标*************/	

	memset(rtctimebuf, 0, sizeof(rtctimebuf));
	hal_sysGetRTC(&rtctimetmp);
	sprintf(rtctimebuf, "%02d:%02d", rtctimetmp.hour, rtctimetmp.min);

	hal_scrIconTextOut(g_stLcdConfig.ICON_TIME_X, 0, rtctimebuf, &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, NULL);


	
/******************************************************/	
/*************刷新电量及USB检测图标*************/	
	memcpy(aui8iconModeTmp, g_stIconDisp.UseFlag, MAX_ICON);
	iRet = hal_pmGetChg();
	if(iRet == 2)//未充电
	{
		hal_ledRun(LEDRED, 0, 0);
		iRet = hal_pmBatGetValue();
		if(iRet >= 0)
		{
			iRet = Hysteresis(iRet, g_iIconBatteryTmp, 2, 2, 10, 1);
			iRet = Hysteresis(iRet, g_iIconBatteryTmp, 2, 2, 30, 1);
			iRet = Hysteresis(iRet, g_iIconBatteryTmp, 2, 2, 55, 1);
			iRet = Hysteresis(iRet, g_iIconBatteryTmp, 2, 2, 80, 1);
			g_iIconBatteryTmp = iRet;
			
			aui8iconModeTmp[USB_ICON] = 0;
			if(iRet>=0 && iRet<10)
			{
				aui8iconModeTmp[BATTERY_ICON] = 5;
			}
			else if(iRet>=10 && iRet<30)
			{
				aui8iconModeTmp[BATTERY_ICON] = 1;
			}
			else if(iRet>=30 && iRet<55)
			{
				aui8iconModeTmp[BATTERY_ICON] = 2;
			}
			else if(iRet>=55 && iRet<80)
			{
				aui8iconModeTmp[BATTERY_ICON] = 3;
			}
			else if(iRet>=80 && iRet<=100)
			{
				aui8iconModeTmp[BATTERY_ICON] = 4;
			}
			
		}
		
	}
	else if(iRet == 1)//充电中
	{
		hal_ledRun(LEDRED, 2, 0);
		aui8iconModeTmp[USB_ICON] = 1;
		aui8iconModeTmp[BATTERY_ICON] = 6;
	}
	else if(iRet == 4)
	{
		hal_ledRun(LEDRED, 1, 0);
		aui8iconModeTmp[USB_ICON] = 1;
		aui8iconModeTmp[BATTERY_ICON] = 4;
	}
	
	hal_iconSelectIconMode(BATTERY_ICON, aui8iconModeTmp[BATTERY_ICON]);
	hal_iconSelectIconMode(USB_ICON, aui8iconModeTmp[USB_ICON]);
/******************************************************/
/*************刷新WiFi图标*************/	

	if(OPEN_SUCC == g_stWifiState.cOpenState)
	{
		if(g_iIconLoopCnt == 0 || g_iIconLoopCnt == 5)// || aui8iconModeTmp[WIFI_ICON] == 0)
		{
			ST_AP_INFO stAp;
			memset(stAp.cSsid, 0, sizeof(stAp.cSsid));
			memset(stAp.cBssid, 0, sizeof(stAp.cBssid));
			stAp.iChannel = 0;
			stAp.iRssi = 0;
			iRet = fibo_mutex_try_lock(g_ui32ApiWiFiSendMutex, 500);
			if(iRet >= 0)
			{
				fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
				//Lprintf("hal_espCheckAp\r\n");
				iRet = hal_espCheckApAsync(&stAp);
				if(iRet > 0)
				{
					
					sysLOG(COLORICON_LOG_LEVEL_4, "<SUCC> hal_espCheckAp,iRet=%d,cSsid:%s,cBssid:%s,iChannel=%d,iRssi=%d\r\n", iRet, stAp.cSsid, stAp.cBssid, stAp.iChannel, stAp.iRssi);
					iRet = Hysteresis(stAp.iRssi, g_iIconWiFiTmp, 3, 3, -75, 1);
					iRet = Hysteresis(iRet, g_iIconWiFiTmp, 3, 3, -65, 1);
					g_iIconWiFiTmp = iRet;

					if(iRet < (-75))
					{
						iRet = 1;
					}
					else if((iRet >= (-75)) && (iRet < (-65)))
					{
						iRet = 2;
					}
					else 
					{
						iRet = 3;
					}
					
					aui8iconModeTmp[WIFI_ICON] = iRet+1;
				}
				else if(iRet = WIFI_NOT_APCONNECT_ERR)
				{
					aui8iconModeTmp[WIFI_ICON] = 0;
					sysLOG(COLORICON_LOG_LEVEL_4, "<WARN> hal_espCheckAp,iRet=%d,cSsid:%s,cBssid:%s,iChannel=%d,iRssi=%d\r\n", iRet, stAp.cSsid, stAp.cBssid, stAp.iChannel, stAp.iRssi);
				}
				else
				{
					sysLOG(COLORICON_LOG_LEVEL_4, "<ERR> hal_espCheckAp,iRet=%d,cSsid:%s,cBssid:%s,iChannel=%d,iRssi=%d\r\n", iRet, stAp.cSsid, stAp.cBssid, stAp.iChannel, stAp.iRssi);
				}
			}
		}
	}
	else

	{
		aui8iconModeTmp[WIFI_ICON] = 0;
	
	}
	hal_iconSelectIconMode(WIFI_ICON, aui8iconModeTmp[WIFI_ICON]);


/******************************************************/
/*************刷新2G or 4G图标*************/	

	iRet = hal_wiresockGetGSMorLTE();
	if(iRet == 1)//LTE
	{

		hal_scrIconTextOut(0, 0, "4G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, NULL);		
		
	}
	else if(iRet == 2)//GPRS
	{
	
		hal_scrIconTextOut(0, 0, "2G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, NULL);	
		
	}
	

/******************************************************/
/*************刷新GPRS信号图标*************/	

	if(g_iIconLoopCnt == 0 || g_iIconLoopCnt == 5)
	{
		
		iRet = fibo_get_sim_status(&simstatustmp);
		if(iRet == 0)
		{
			
			if(simstatustmp == 1)
			{
				iRet = hal_wiresockGetCSQ(&rssitmp, &bertmp);
				if(iRet == 0)
				{
					iRet = Hysteresis(rssitmp, g_iIconGPRSTmp, 2, 2, 24, 1);
					iRet = Hysteresis(iRet, g_iIconGPRSTmp, 2, 2, 18, 1);
					iRet = Hysteresis(iRet, g_iIconGPRSTmp, 2, 2, 12, 1);
					iRet = Hysteresis(iRet, g_iIconGPRSTmp, 3, 3, 6, 1);
					sprintf(rtctimebuf, "%d,%d,%d", iRet, rssitmp, g_iIconGPRSTmp);

					g_iIconGPRSTmp = iRet;
						
					if(iRet > 24)
					{
						aui8iconModeTmp[GPRS_ICON] = 6;
					}
					else if(iRet <= 24 && iRet > 18)
					{
						aui8iconModeTmp[GPRS_ICON] = 5;
					}
					else if(iRet <= 18 && iRet > 12)
					{
						aui8iconModeTmp[GPRS_ICON] = 4;
					}
					else if(iRet <= 12 && iRet > 6)
					{
						aui8iconModeTmp[GPRS_ICON] = 3;
					}
					else if(iRet <= 6)
					{
						aui8iconModeTmp[GPRS_ICON] = 2;
					}
				}
			}
			else

			{
				aui8iconModeTmp[COLORGPRS_ICON] = 1;
			}
			hal_iconSelectIconMode(GPRS_ICON, aui8iconModeTmp[GPRS_ICON]);
		}
	}

/******************************************************/

	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	hal_iconRefresh();
	
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

	

}


/*
*Function:		hal_iconInit
*Description:	黑白屏图标初始化接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_iconInit(void)
{
	hal_scrIconClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, 11);
	
	for(uint8 i=0; i<(MAX_ICON); i++)
	{
		g_stIconDisp.g_ucIconDispMode[i] = 0xFF;
	}

	hal_scrIconTextOut(0, 0, "4G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, NULL);
	hal_iconSelectIconMode(BATTERY_ICON, 4);
	hal_iconSelectIconMode(GPRS_ICON, 6);
	hal_iconSelectIconMode(BT_ICON, 0);
	hal_iconSelectIconMode(WIFI_ICON, 0);
	hal_iconSelectIconMode(USB_ICON, 1);
	hal_iconSelectIconMode(ICCARD_ICON, 0);
	hal_iconSelectIconMode(LOCK_ICON, 0);
	hal_iconSelectIconMode(GPRSSIG_ICON, 1);


}




/*****************************TEST**************************************/

#if MAINTEST_FLAG

void hal_iconHysteresistest(void)
{
	float iRet = -1;

	sysLOG(ICON_LOG_LEVEL_2, "<START1>\r\n");
	/*mode = 0时*/
	iRet = Hysteresis(13, 9, 2, 4, 0, 0);//13
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(13, 9, 2, 5, 0, 0);//9
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9, 13, 4, 2, 0, 0);//9
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9, 13, 5, 2, 0, 0);//13
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(8.7, 9.3, 0.5, 1, 0, 0);//8.7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9.3, 8.7, 1, 0.5, 0, 0);//9.3
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);

	sysLOG(ICON_LOG_LEVEL_2, "<START2>\r\n");
	/*mode = 1时*/
	iRet = Hysteresis(8.7, 9.3, 0.5, 0.2, 9, 1);//9.3
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(8.7, 9.3, 0.3, 0.2, 9, 1);//8.7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9.3, 8.7, 0.2, 0.3, 9, 1);//9.3
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9.3, 8.7, 0.2, 0.5, 9, 1);//8.7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(7, 13, 3, 2, 10, 1);//7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(7, 13, 4, 2, 10, 1);//13
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(13, 7, 2, 3, 10, 1);//13
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(13, 7, 2, 4, 10, 1);//7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
													
	sysLOG(ICON_LOG_LEVEL_2, "<START3>\r\n");
	/*mode = 1时*/
	iRet = Hysteresis(-9.3, -8.7, 0.5, 0.2, -9, 1);//-8.7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-9.3, -8.7, 0.3, 0.2, -9, 1);//-9.3
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-8.7, -9.3, 0.2, 0.3, -9, 1);//-8.7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-8.7, -9.3, 0.2, 0.5, -9, 1);//-9.3
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-13, -7, 3, 2, -10, 1);//-13
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-13, -7, 4, 2, -10, 1);//-7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-7, -13, 2, 3, -10, 1);//-7
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-7, -13, 2, 4, -10, 1);//-13
	sysLOG(ICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
													



}

#endif








