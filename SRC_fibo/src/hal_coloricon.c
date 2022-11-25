/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_coloricon.c
** Description:		彩屏图标相关接口
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



COLORLCD_ICON_DISP     g_stColoriconDisp;

int g_iColorIconBatteryTmp = 0, g_iColorIconWiFiTmp = 0, g_iColorIconGPRSTmp = 0;//缓存电量、WiFi信号、GPRS信号
int g_iColorIconLoopCnt = 0;

static uint8 aui8iconModetmp[COLORMAX_ICON] = {0};//如果图标读取结果失败，按照上一次的结果显示


/*
*Function:		hal_ciconSelectIconMode
*Description:	图标模式选择
*Input:			icon_no:图标序号；mode:图标模式
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
int hal_ciconSelectIconMode(COLORICON_ORDER icon_no,unsigned int mode)
{

    switch(icon_no)
    {
        int i;
        
        case COLORBATTERY_ICON:
            
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;//LCD_COLOR_BLACK;  
            g_stColoriconDisp.UseFlag[icon_no]= mode;
            if (mode == 6){           
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_batterycharge24X24;}           
            else if (mode == 5){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery24X24_0;}
            else if (mode == 4){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery24X24_4;}
            else if (mode == 3){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery24X24_3;}
            else if (mode == 2){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery24X24_2;}
            else if (mode == 1){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery24X24_1;}
            else if ((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_battery24X24_4;
                g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;}
            else 
                return -2;
            break;
        case COLORGPRS_ICON:
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;
            g_stColoriconDisp.UseFlag[icon_no]=mode;
        
            if (mode == 1){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_0;
				g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;}
            else if (mode == 2){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_1;}
            else if (mode == 3){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_2;}
            else if (mode == 4){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_3;}
            else if (mode == 5){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_4;}
           else if (mode == 6){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_5;}
           else if((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_gprs24X24_5;
                g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;}       
            else 
                return -2;
            break;
        case COLORBT_ICON:
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;
            g_stColoriconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_bt24X24;}
            else if (mode ==2){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_bt_n24X24;
				g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;}
            else if ((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_bt24X24;
                g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;} 
            else 
                return -2;
            break;
        case COLORWIFI_ICON:
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;
            g_stColoriconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi24X24_1;}
            else if (mode == 2){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi24X24_2;}
            else if (mode == 3){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi24X24_3;}
            else if (mode == 4){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi24X24_4;}
			 else if ((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_wifi24X24_4;
                g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;} 
            else 
                return -2;
            break;
        case COLORUSB_ICON:
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;
            g_stColoriconDisp.UseFlag[icon_no]=mode;
            if (mode == 1)
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_usb24X24;
			 else if ((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_usb24X24;
                g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;} 			
            else 
                return -2;
            break;
        case COLORICCARD_ICON:
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;
            g_stColoriconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_ic24X24;}
			else if ((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_ic24X24;
				g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;}			
            else 
                return -2;
            break;
        case COLORLOCK_ICON:
            g_stColoriconDisp.usColor[icon_no]= g_stColorlcdGUI.icon_attr.iconColor;
            g_stColoriconDisp.UseFlag[icon_no]=mode;
            if (mode == 1){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_lock24X24;}
			else if ((mode == 0xFF)||(mode == 0)){
                g_stColoriconDisp.iconBuff[icon_no]=(unsigned char *)gImage_lock24X24;
				g_stColoriconDisp.usColor[icon_no]= COLORICON_GRAY;}		
            else 
                return -2;
            break;
        default:
            return -1;      
        
    }
 
	return 0;
		
}


/*
*Function:		hal_ciconRefresh
*Description:	图标刷新
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ciconRefresh(void)
{

	unsigned char * imageAddr;
	unsigned char i;
	unsigned short w,h;
	int x,y;
	
	//1.查询是否需要刷新
	for(i=0;i<COLORMAX_ICON;i++)
	{
		if(g_stColoriconDisp.UseFlag[i] != g_stColoriconDisp.g_ucIconDispMode[i])
		{
			sysLOG(COLORICON_LOG_LEVEL_4, "i=%d, g_stColoriconDisp.UseFlag[i]=%d, g_stColoriconDisp.g_ucIconDispMode[i]=%d\r\n", i, g_stColoriconDisp.UseFlag[i], g_stColoriconDisp.g_ucIconDispMode[i]);
			//calc the pos

			imageAddr = g_stColoriconDisp.iconBuff[i];
			w = 24;
			h = 24;
			y = 0;
//			x = 128-(i+1)*12; 
//			x = x<0?0:x;
			switch(i)
			{
				case COLORBATTERY_ICON:
					x = g_stLcdConfig.COLORICON_BATTERY_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stColoriconDisp.usColor[i]);
				break;
				case COLORGPRS_ICON:
					x = g_stLcdConfig.COLORICON_GPRS_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stColoriconDisp.usColor[i]);
				break;
				case COLORBT_ICON:
					x = g_stLcdConfig.COLORICON_BT_X;
				break;
				case COLORWIFI_ICON:
					x = g_stLcdConfig.COLORICON_WIFI_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stColoriconDisp.usColor[i]);
				break;
				case COLORUSB_ICON:
					x = g_stLcdConfig.COLORICON_USB_X;
					hal_scrWriteIcon(x, y, imageAddr, w, h, g_stColoriconDisp.usColor[i]);
				break;
				case COLORICCARD_ICON:
					x = g_stLcdConfig.COLORICON_ICCARD_X;
				break;
				case COLORLOCK_ICON:
					x = g_stLcdConfig.COLORICON_LOCK_X;
				break;
				default:

				break;
			}
			
			//set the color
			//hal_setIconColor(g_stColoriconDisp.usColor[i]);
			//send the data to lcd	
			sysLOG(COLORICON_LOG_LEVEL_4, "i=%d, x=%d, y=%d, w=%d, h=%d, g_stColoriconDisp.usColor[i]=%d, imageAddr+20:%x\r\n", i, x, y, w, h, g_stColoriconDisp.usColor[i], *(imageAddr+20));
			
			//hal_scrWriteIcon(x, y, imageAddr, w, h, g_stColoriconDisp.usColor[i]);
			
			
			//update the record info
			g_stColoriconDisp.g_ucIconDispMode[i] = g_stColoriconDisp.UseFlag[i];
		}
	}

}



/*
*Function:		hal_ciconLoop
*Description:	图标Loop调用接口,实时查询变化，并将所对应的图标刷新到buff中
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ciconLoop(void)
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

			hal_scrIconClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, 23);
			for(uint8 i=0; i<(COLORMAX_ICON); i++)
			{
				g_stColoriconDisp.g_ucIconDispMode[i] = 0xFF;
			}
		}
	}
		
	
	g_iColorIconLoopCnt ++;
	if(g_iColorIconLoopCnt >= 10)
	{
		g_iColorIconLoopCnt = 0;
	}
	
/*************刷新RTC图标*************/	

	memset(rtctimebuf, 0, sizeof(rtctimebuf));
	hal_sysGetRTC(&rtctimetmp);
	sprintf(rtctimebuf, "%02d:%02d", rtctimetmp.hour, rtctimetmp.min);
	
	g_stColorlcdGUI.icon_attr.iconColor = BLACK;
	hal_scrIconTextOut(g_stLcdConfig.COLORICON_TIME_X, 0, rtctimebuf, &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont12X24);
	

/******************************************************/	
/*************刷新电量及USB检测图标*************/	
	memcpy(aui8iconModetmp, g_stColoriconDisp.UseFlag, COLORMAX_ICON);
	iRet = hal_pmGetChg();
	if(iRet == 2)//未充电
	{
		hal_ledRun(LEDRED, 0, 0);
		iRet = hal_pmBatGetValue();
		if(iRet >= 0)
		{
			iRet = Hysteresis(iRet, g_iColorIconBatteryTmp, 2, 2, 10, 1);
			iRet = Hysteresis(iRet, g_iColorIconBatteryTmp, 2, 2, 30, 1);
			iRet = Hysteresis(iRet, g_iColorIconBatteryTmp, 2, 2, 55, 1);
			iRet = Hysteresis(iRet, g_iColorIconBatteryTmp, 2, 2, 80, 1);
			g_iColorIconBatteryTmp = iRet;
			
			aui8iconModetmp[COLORUSB_ICON] = 0;
			if(iRet>=0 && iRet<10)
			{
				aui8iconModetmp[COLORBATTERY_ICON] = 5;
			}
			else if(iRet>=10 && iRet<30)
			{
				aui8iconModetmp[COLORBATTERY_ICON] = 1;
			}
			else if(iRet>=30 && iRet<55)
			{
				aui8iconModetmp[COLORBATTERY_ICON] = 2;
			}
			else if(iRet>=55 && iRet<80)
			{
				aui8iconModetmp[COLORBATTERY_ICON] = 3;
			}
			else if(iRet>=80 && iRet<=100)
			{
				aui8iconModetmp[COLORBATTERY_ICON] = 4;
			}
			
		}
		
	}
	else if(iRet == 1)//充电中
	{
		hal_ledRun(LEDRED, 2, 0);
		aui8iconModetmp[COLORUSB_ICON] = 1;
		aui8iconModetmp[COLORBATTERY_ICON] = 6;
	}
	else if(iRet == 4)
	{
		hal_ledRun(LEDRED, 1, 0);
		aui8iconModetmp[COLORUSB_ICON] = 1;
		aui8iconModetmp[COLORBATTERY_ICON] = 4;
	}
	
	hal_ciconSelectIconMode(COLORBATTERY_ICON, aui8iconModetmp[COLORBATTERY_ICON]);
	hal_ciconSelectIconMode(COLORUSB_ICON, aui8iconModetmp[COLORUSB_ICON]);
/******************************************************/
/*************刷新WiFi图标*************/	

	if(OPEN_SUCC == g_stWifiState.cOpenState)
	{
		
		//if(g_iColorIconLoopCnt == 0 || g_iColorIconLoopCnt %2 == 0)
		if(g_iColorIconLoopCnt == 0 || g_iColorIconLoopCnt == 5)
		{
			#if 1
			ST_AP_INFO stAp;
			memset(stAp.cSsid, 0, sizeof(stAp.cSsid));
			memset(stAp.cBssid, 0, sizeof(stAp.cBssid));
			stAp.iChannel = 0;
			stAp.iRssi = 0;
			//if(iRet >= 0)
			{
				iRet = hal_espCheckApAsync(&stAp);//hal_espCheckAp(&stAp);
				if(iRet > 0)
				{
					sysLOG(COLORICON_LOG_LEVEL_4, "<SUCC> hal_espCheckApAsync,iRet=%d,cSsid:%s,cBssid:%s,iChannel=%d,iRssi=%d\r\n", iRet, stAp.cSsid, stAp.cBssid, stAp.iChannel, stAp.iRssi);
					iRet = Hysteresis(stAp.iRssi, g_iColorIconWiFiTmp, 3, 3, -75, 1);
					iRet = Hysteresis(iRet, g_iColorIconWiFiTmp, 3, 3, -65, 1);
					g_iColorIconWiFiTmp = iRet;

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
					
					aui8iconModetmp[COLORWIFI_ICON] = iRet+1;
				}
				else if(iRet = WIFI_NOT_APCONNECT_ERR)
				{
					aui8iconModetmp[COLORWIFI_ICON] = 0;
					sysLOG(COLORICON_LOG_LEVEL_3, "<WARN> hal_espCheckAp,iRet=%d,cSsid:%s,cBssid:%s,iChannel=%d,iRssi=%d\r\n", iRet, stAp.cSsid, stAp.cBssid, stAp.iChannel, stAp.iRssi);
				}
				else

				{
					sysLOG(COLORICON_LOG_LEVEL_3, "<ERR> hal_espCheckAp,iRet=%d,cSsid:%s,cBssid:%s,iChannel=%d,iRssi=%d\r\n", iRet, stAp.cSsid, stAp.cBssid, stAp.iChannel, stAp.iRssi);
				}
				
			}
			#endif
		}
	}
	else

	{
		aui8iconModetmp[COLORWIFI_ICON] = 0;
	
	}
	hal_ciconSelectIconMode(COLORWIFI_ICON, aui8iconModetmp[COLORWIFI_ICON]);


/******************************************************/
/*************刷新2G or 4G图标*************/	

	iRet = hal_wiresockGetGSMorLTE();
	if(iRet == 1)//LTE
	{

		g_stColorlcdGUI.icon_attr.iconColor = COLORICON_GRAY;
		hal_scrIconTextOut(0, 0, "2G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont6X12);
		
		g_stColorlcdGUI.icon_attr.iconColor = BLACK;
		hal_scrIconTextOut(0, 12, "4G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont6X12);

		g_stColorlcdGUI.icon_attr.iconColor = BLACK;
		
	}
	else if(iRet == 2)//GPRS
	{
		
		g_stColorlcdGUI.icon_attr.iconColor = BLACK;
		hal_scrIconTextOut(0, 0, "2G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont6X12);
		
		g_stColorlcdGUI.icon_attr.iconColor = COLORICON_GRAY;
		hal_scrIconTextOut(0, 12, "4G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont6X12);

		g_stColorlcdGUI.icon_attr.iconColor = BLACK;
		
	}
	

/******************************************************/	
/*************刷新GPRS信号图标*************/	

	if(g_iColorIconLoopCnt == 0 || g_iColorIconLoopCnt == 5)
	{
		
		iRet = fibo_get_sim_status(&simstatustmp);
		if(iRet == 0)
		{
			
			if(simstatustmp == 1)
			{
				iRet = hal_wiresockGetCSQ(&rssitmp, &bertmp);
				if(iRet == 0)
				{
					iRet = Hysteresis(rssitmp, g_iColorIconGPRSTmp, 2, 2, 24, 1);
					iRet = Hysteresis(iRet, g_iColorIconGPRSTmp, 2, 2, 18, 1);
					iRet = Hysteresis(iRet, g_iColorIconGPRSTmp, 2, 2, 12, 1);
					iRet = Hysteresis(iRet, g_iColorIconGPRSTmp, 3, 3, 6, 1);
					sprintf(rtctimebuf, "%d,%d,%d", iRet, rssitmp, g_iColorIconGPRSTmp);

					g_iColorIconGPRSTmp = iRet;
						
					if(iRet > 24)
					{
						aui8iconModetmp[COLORGPRS_ICON] = 6;
					}
					else if(iRet <= 24 && iRet > 18)
					{
						aui8iconModetmp[COLORGPRS_ICON] = 5;
					}
					else if(iRet <= 18 && iRet > 12)
					{
						aui8iconModetmp[COLORGPRS_ICON] = 4;
					}
					else if(iRet <= 12 && iRet > 6)
					{
						aui8iconModetmp[COLORGPRS_ICON] = 3;
					}
					else if(iRet <= 6)
					{
						aui8iconModetmp[COLORGPRS_ICON] = 2;
					}
				}
			}
			else

			{
				aui8iconModetmp[COLORGPRS_ICON] = 1;
			}
			hal_ciconSelectIconMode(COLORGPRS_ICON, aui8iconModetmp[COLORGPRS_ICON]);
		}
		
	}
/******************************************************/


	hal_scrMutexCS(SCREEN_LOCK, SCREEN_FIRST);

	hal_ciconRefresh();
	
	hal_scrMutexCS(SCREEN_UNLOCK, SCREEN_FIRST);

	

}

/*
*Function:		hal_ciconInit
*Description:	彩屏图标初始化接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ciconInit(void)
{

	hal_scrIconClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, 23);

	
	for(uint8 i=0; i<(COLORMAX_ICON); i++)
	{
		g_stColoriconDisp.g_ucIconDispMode[i] = 0xFF;
	}
	
	
	g_stColorlcdGUI.icon_attr.iconColor = COLORICON_GRAY;
	hal_scrIconTextOut(0, 0, "2G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont6X12);
	
	g_stColorlcdGUI.icon_attr.iconColor = COLORICON_GRAY;
	hal_scrIconTextOut(0, 12, "4G", &g_stCurPixelIcon.x, &g_stCurPixelIcon.y, &g_stSingleFont6X12);

	g_stColorlcdGUI.icon_attr.iconColor = BLACK;
	
	
	hal_ciconSelectIconMode(COLORBATTERY_ICON, 4);
	hal_ciconSelectIconMode(COLORGPRS_ICON, 6);
	hal_ciconSelectIconMode(COLORBT_ICON, 0);
	hal_ciconSelectIconMode(COLORWIFI_ICON, 0);
	hal_ciconSelectIconMode(COLORUSB_ICON, 1);
	hal_ciconSelectIconMode(COLORICCARD_ICON, 0);
	hal_ciconSelectIconMode(COLORLOCK_ICON, 0);
	

}




/*****************************TEST**************************************/
#if MAINTEST_FLAG

void hal_icionHysteresistest(void)
{
	float iRet = -1;

	sysLOG(COLORICON_LOG_LEVEL_2, "<START1>\r\n");
	/*mode = 0时*/
	iRet = Hysteresis(13, 9, 2, 4, 0, 0);//13
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(13, 9, 2, 5, 0, 0);//9
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9, 13, 4, 2, 0, 0);//9
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9, 13, 5, 2, 0, 0);//13
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(8.7, 9.3, 0.5, 1, 0, 0);//8.7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9.3, 8.7, 1, 0.5, 0, 0);//9.3
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);

	sysLOG(COLORICON_LOG_LEVEL_2, "<START2>\r\n");
	/*mode = 1时*/
	iRet = Hysteresis(8.7, 9.3, 0.5, 0.2, 9, 1);//9.3
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(8.7, 9.3, 0.3, 0.2, 9, 1);//8.7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9.3, 8.7, 0.2, 0.3, 9, 1);//9.3
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(9.3, 8.7, 0.2, 0.5, 9, 1);//8.7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(7, 13, 3, 2, 10, 1);//7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(7, 13, 4, 2, 10, 1);//13
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(13, 7, 2, 3, 10, 1);//13
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(13, 7, 2, 4, 10, 1);//7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
													
	sysLOG(COLORICON_LOG_LEVEL_2, "<START3>\r\n");
	/*mode = 1时*/
	iRet = Hysteresis(-9.3, -8.7, 0.5, 0.2, -9, 1);//-8.7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-9.3, -8.7, 0.3, 0.2, -9, 1);//-9.3
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-8.7, -9.3, 0.2, 0.3, -9, 1);//-8.7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-8.7, -9.3, 0.2, 0.5, -9, 1);//-9.3
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-13, -7, 3, 2, -10, 1);//-13
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-13, -7, 4, 2, -10, 1);//-7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-7, -13, 2, 3, -10, 1);//-7
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(-7, -13, 2, 4, -10, 1);//-13
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
													

	iRet = Hysteresis(81, 0, 2, 2, 10, 1);
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(iRet, 0, 2, 2, 30, 1);
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(iRet, 0, 2, 2, 55, 1);
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);
	iRet = Hysteresis(iRet, 0, 2, 2, 80, 1);
	sysLOG(COLORICON_LOG_LEVEL_2, "Hysteresis iRet=%f\r\n", iRet);


}


#endif








