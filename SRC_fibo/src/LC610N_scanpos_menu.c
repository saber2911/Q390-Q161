#include "comm.h"
#include "LC610N_scanpos_menu.h"
#include "LC610N_api_gps.h"
#include "LC610N_unlock.h"
#include "logo.h"

extern unsigned int se_comm_mutex;
extern unsigned long long g_ullStartTime;
extern unsigned long long g_ullEndTime;

unsigned int g_uiMaxLine = 6;//最大行数
extern uint8 DownLoadVOSFlag;
#define LCD_MAX_LINE    	(g_uiMaxLine)

typedef struct stMenuNode
{
	char* pszChnMsg;
	char*pszEngMsg;
	void (*pFun)();
}STMENUNODE,*PSTMENUNODE;

unsigned char  AsciiToHex(unsigned char *ucBuffer);
int hal_scrSetMenuTitle(const char *str)
{
	char sbuffer[256];
	if (str == NULL)
	{
		return -1;
	}
	scrDrawRectBlock_lib(0, 24, 319, 47, 0x01);
	sprintf(sbuffer,"%s\n",str);
	scrPrint_lib(1,1,2,sbuffer);
    return 0;
}

int Menu_uiSetMenuTitle(STMENUNODE *pstMenu)
{
	if (pstMenu == NULL)
	{
		return -1;
	}

	if (LAN_CHN == g_ui8ScrLanguage)
	{
		hal_scrSetMenuTitle(pstMenu->pszChnMsg);
	}
	else
	{
		hal_scrSetMenuTitle(pstMenu->pszEngMsg);
	}
	return 0;
}
int hal_scrSetMenuItem(unsigned char row, const char *str)//0 icon,1 title
{	
	char sbuffer[256];
	if (str == NULL)
	{
		return -1;
	}
	//row += 1;
	if(row < 0 || row > LCD_MAX_LINE)
	{
		return -2;
	}
	sprintf(sbuffer,"%s\n",str);
	scrPrint_lib(1, row+2, 0, sbuffer);
    return 0;
}

int Menu_uiSetMenuItem(unsigned char row, unsigned char btnline, STMENUNODE *pstMenu)
{
	char sbuffer[256];
	if (pstMenu == NULL)
	{
		return -1;
	}

	if (LAN_CHN == g_ui8ScrLanguage)
	{
		sprintf(sbuffer,"%d-%s\n",btnline,pstMenu->pszChnMsg);
	}
	else
	{
		sprintf(sbuffer,"%d-%s\n",btnline,pstMenu->pszEngMsg);
	}

	hal_scrSetMenuItem(row,sbuffer);
	return 0;
}

void NoFun(void)
{
	return;
}
void mLogset()
{
	unsigned char ucKey;
	int level;
	scrCls_lib();
	scrPrint_lib(0,1,0,"请选择日志等级:");
	scrPrint_lib(0,2,0,"pls select log level");
	scrPrint_lib(0,3,0,"press 0~4");
	ucKey = kbGetKey_lib();

	while(1)
	{
		ucKey = kbGetKey_lib();
		if(ucKey == 0xFF)
		{					
			continue;
		}
		if(ucKey == KEYCANCEL)	
		{
			scrCls_lib();
			scrPrint_lib(0,2,0,"用户取消");
			sysDelayMs(2000);
			return;
		}
		if((ucKey == KEY0)
			||(ucKey == KEY1)
			||(ucKey == KEY2)
			||(ucKey == KEY3))
		{
			scrPrint_lib(0,4,0,"log level：%c!!",ucKey);
			level = ucKey - '0';
			sysLOGSet_lib(level);
			sysDelayMs(2000);
			return;

		}			

	}

}

void mCLcdCacheTest()
{
	uint8_t * popData;
	int datalen = 320*(240-24)*2;
	popData = (unsigned char*) fibo_malloc(datalen);
	scrCls_lib();
	//1.将屏幕填充满特定的颜色
	//scrDrawRectBlock_lib(0,24,319,239,BLUE);
	//scrPrint_lib(0,1,0,"this is old screen data");
	scrPrint_lib(0,2,0,"广州市天河区建工路5号4楼");
	scrPrint_lib(0,3,0,"房就餐人数");
	//scrPrint_lib(0,4,0,"44444444");
	//scrPrint_lib(0,5,0,"55555555");
	//scrPrint_lib(0,6,0,"66666666");
	//scrPrint_lib(0,7,0,"77777777");	
	//2.将缓存数据读出
	scrPopDot_lib(popData,datalen);
	//scrPrint_lib(0,2,0,"data pop wait key");
	hal_keypadWaitOneKey();	
	//3.更新屏幕缓存数据
	if (g_iType_machine == 0)
	{
		scrDrawRectBlock_lib(0,24,319,239,GREEN);
	}
	else if (g_iType_machine == 1)
	{
		scrDrawRectBlock_lib(0,24,239,319,GREEN);
	}
	
	scrPrint_lib(0,3,0,"lcd has refresh wait key");
	hal_keypadWaitOneKey();	
	//4.push旧的内容到屏幕并刷新屏幕
	scrPushDot_lib(popData,datalen);
	sysDelayMs_lib(2000);	
	scrPrint_lib(0,3,0,"lcd has refresh wait key exit");
	hal_keypadWaitOneKey();

}

void mGLcdCacheTest()
{
	uint8_t * popData;
	int datalen = 128*(96-12)/4;
	popData = (unsigned char*) fibo_malloc(datalen);
	
	//1.将屏幕填充满特定的颜色
	scrPrint_lib(0,1,0,"this is old screen data");
	scrPrint_lib(0,2,0,"22222222");
	scrPrint_lib(0,3,0,"33333333");
	scrPrint_lib(0,4,0,"44444444");
	scrPrint_lib(0,5,0,"55555555");
	scrPrint_lib(0,6,0,"66666666");
	scrPrint_lib(0,7,0,"77777777");
	//scrPrint_lib(0,4,0,"33333333");
	//2.将缓存数据读出
	scrPopDot_lib(popData,datalen);
	scrPrint_lib(0,3,0,"data pop wait key");
	hal_keypadWaitOneKey();	
	//3.更新屏幕缓存数据
	scrCls_lib();
	scrPrint_lib(0,3,0,"lcd has refresh wait key");
	hal_keypadWaitOneKey();	
	//4.push旧的内容到屏幕并刷新屏幕
	scrPushDot_lib(popData,datalen);
	sysDelayMs_lib(2000);	
	scrPrint_lib(0,3,0,"lcd has refresh wait key exit");
	hal_keypadWaitOneKey();

}

void mFontCodeTest()
{
	scrCls_lib();
	scrPrint_lib(0,1,0,"lcd font test!!");

	sysLOG_lib(LCD_LOG_LEVEL_1, "[%s] -%s- Line=%d: mFontCodeTest g_stUniMultiFont12X12  \r\n", filename(__FILE__), __FUNCTION__, __LINE__);

	if(FONTFS == hal_fontGetFontType())
		hal_scrSelectFont(NULL, &g_stGBMultiFont12X12);
//	else
//		hal_scrSelectFont(NULL, &g_stUniMultiFont12X12);
	scrPrint_lib(0,2,0,"十二号字展示");
	sysDelayMs(500);
	sysLOG_lib(LCD_LOG_LEVEL_1, "[%s] -%s- Line=%d: mFontCodeTest g_stUniMultiFont16X16  \r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	if(FONTFS == hal_fontGetFontType())
		hal_scrSelectFont(NULL, &g_stGBMultiFont16X16);
//	else
//		hal_scrSelectFont(NULL, &g_stUniMultiFont16X16);
	scrPrint_lib(0,3,0,"十六号字展示");
	sysDelayMs(500);
	sysLOG_lib(LCD_LOG_LEVEL_1, "[%s] -%s- Line=%d: mFontCodeTest g_stUniMultiFont24X24  \r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	if(FONTFS == hal_fontGetFontType())
		hal_scrSelectFont(NULL, &g_stGBMultiFont24X24);
//	else
//		hal_scrSelectFont(NULL, &g_stUniMultiFont24X24);
	scrPrint_lib(0,4,0,"二十四号字展示");
	
	hal_scrSelectFont(&g_stSingleFont6X12, NULL);
	scrPrint_lib(0,5,0,"Hello World!\r\n");
	sysDelayMs(500);	
	hal_scrSelectFont(&g_stSingleFont8X16, NULL);
	scrPrint_lib(0,6,0,"Hello World!\r\n");
	sysDelayMs(500);
	hal_scrSelectFont(&g_stSingleFont8X16, NULL);
	scrPrint_lib(0,7,0,"Hello World!\r\n");
	sysDelayMs(500);
	if(FONTFS == hal_fontGetFontType())
		hal_scrSelectFont(&g_stSingleFont12X24, &g_stGBMultiFont24X24);
//	else
//		hal_scrSelectFont(&g_stSingleFont12X24, &g_stUniMultiFont24X24);
	//sysDelayMs(500);

}
void Menu_ModChkScr()
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
            { "屏幕测试", "LCD ", NoFun },
            { "屏幕颜色", "LCD COLOR", NoFun}, 
            { "缓存测试", "LCD CACHE", mCLcdCacheTest}, 
            //{ "缓存文字", "LCD str CACHE", mGLcdCacheTest}, 
            { "字库文字", "FONT&CODE", mFontCodeTest}, 
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;

}

void McCheckRF()
{

	int siRet;
	uchar ucKey, ucCardType,szSerialInfo[10],szCID[8],szOther[300];
	unsigned long long uiTimerCount;

    scrCls_lib();
    scrPrint_Ex(0, 1, 0x80|2, "RF阅读器","RF READER");

    sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: McCheckRF \r\n", filename(__FILE__), __FUNCTION__, __LINE__);

	if((siRet = piccOpen_lib()) != RET_RF_OK)
	{
		goto out;
	}
    
	scrClrLine_lib(2,3);
	scrPrint_Ex(0, 2, 0x02, "请刷卡!","PLS SWIPED CARD!");

	memset(szSerialInfo,0,10);
	memset(szCID,0,8);
	memset(szOther,0,300);

    uiTimerCount = sysGetTicks_lib();  
	while ((siRet = piccDetect_lib(0,&ucCardType,szSerialInfo,szOther)) != 0x00 )
	{
		if(0x00== (siRet = piccDetect_lib('m',&ucCardType,szSerialInfo,szOther)))
			break;
		if(sysGetTicks_lib() > uiTimerCount + 30000)
		{
			scrPrint_Ex(0,3,0x02,"读卡超时","TIME OUT");
			piccClose_lib();
            goto out;
		}
        ucKey = kbGetKey_lib();
        if(ucKey == KEYCANCEL)	
        {
            piccClose_lib();
            goto out;
        }
	}
	
	if (siRet != 0x00)
	{
		piccClose_lib();
		goto out;
	}
	scrClrLine_lib(2,6);
	piccClose_lib();
	if(ucCardType == 'M')
	{
		scrPrint_Ex(0,3,0x00|4,"M 卡测试PICC完成!","M CARD TEST PICC OK!");	
	}
	else if(ucCardType == 'B')
	{
		scrPrint_Ex(0,3,0x00|4,"B 卡测试PICC完成!","B CARD TEST PICC OK!");	
	}
	else
	{
		scrPrint_Ex(0,3,0x00|4,"A 卡测试PICC完成!","A CARD TEST PICC OK!");	
	}
	sysDelayMs(2000);
    out:
        scrClrLine_lib(1,6);
        
}

void MRFPiccISOTest()
{

	unsigned char  ucMode = 0;
	unsigned char* pucCardType[200] = {0};
	unsigned char* pucSerialInfo[200] = {0};
	unsigned char pucOther[200] = {0};
	int iRet ;

	APDU_SEND_LIB ApduSend;
	APDU_RESP_LIB ApduResp;
    scrCls_lib();
    scrPrint_Ex(0, 1, 0x80|2, "RF阅读器","RF READER");
	
	piccOpen_lib();

	iRet = piccDetect_lib(ucMode, pucCardType, pucSerialInfo, pucOther);
    sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:  piccDetect_lib,iRet = %d, pucCardType=%s, pucSerialInfo=%s, pucOther=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet, pucCardType, pucSerialInfo, pucOther);
    if(iRet == 0)
	{

		scrPrint_lib(0, 2, 0x80|2, "piccdetect succ type:%c",pucCardType);
		
	    piccLight_lib(0x03, 1);
		ApduSend.Cmd[0] = 0x00;
		ApduSend.Cmd[1] = 0xa4;
		ApduSend.Cmd[2] = 0x04;
		ApduSend.Cmd[3] = 0x00;
		memset(ApduSend.DataIn, 0, sizeof(ApduSend.DataIn));
		strcpy((char*)ApduSend.DataIn, "1PAY.SYS.DDF01");
		memset(ApduResp.DataOut, 0, sizeof(ApduResp.DataOut));
		ApduSend.Lc = 14;
		ApduSend.Le = 256;
		iRet = piccIsoCommand_lib(&ApduSend, &ApduResp);
		if(iRet == 0)
		{
			scrPrint_lib(0, 3, 0x80|2, "ppse sw1:%x,sw2:%x",ApduResp.SWA,ApduResp.SWB);
		}
		else
		{
			scrPrint_lib(0, 3, 0x80|2, "ppse err iRet:%d",iRet);
		}
		sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:piccIsoCommand_lib, iRet = %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	}
	else
	{
	    piccLight_lib(0x01, 1);
	}

	
	piccClose_lib();

	piccLight_lib(0x03, 2);
	piccLight_lib(0x01, 2);
	sysDelayMs(2000);

}
void CheckPoll(void)
{
	unsigned char ucKey = 0xFF;
	int siRet;
	unsigned char ucCardType, szSerialInfo[10], szCID[8], szOther[300];
	int number = 0,num = 0;
	char display[15] = { 0 };

    scrCls_lib();
   
    hal_scrSetMenuTitle("Check Poll");

    if ((siRet = piccOpen_lib()) != RET_RF_OK)
    {
        scrPrint_Ex(0,1,0x00|2,"非接模块打开错误","piccOpen Faild");
        sysDelayMs(3000);
        goto out;
    }
   scrPrint_Ex(0,1,0x00|2,"请刷卡","PLS SWIPE CARD!");

    memset(szSerialInfo, 0, 10);
    memset(szCID, 0, 8);
    memset(szOther, 0, 300);

    while (1)
    {
        piccOpen_lib();
        sysDelayMs(5);
        siRet = piccDetect_lib(1, &ucCardType, szSerialInfo, szOther);
        if (!siRet)
        {
            memset(display, 0x00, sizeof(display));
            sprintf(display, "%c, %d", ucCardType, number++);
            scrPrint_Ex(0,2,0x00|2, display, display);
        }
        else
        {
            memset(display, 0x00, sizeof(display));
            sprintf(display, "%d, %d", siRet, num++);
            scrPrint_Ex(0,3,0x00|2, display, display);
        }
        piccClose_lib();
        sysDelayMs(5);
		ucKey = kbGetKey_lib();
        if(ucKey == KEYCANCEL)
        {
            piccClose_lib();
            goto out;
        }
    }

out:
	sysDelayMs(1000);
	return;
}
void mPiccIdcard()
{
	
		int siRet;
		uchar ucKey, ucCardType,szSerialInfo[10],szCID[8],szOther[300];
		unsigned long long uiTimerCount;
		APDU_SEND_LIB ApduSend;
		APDU_RESP_LIB ApduResp;
		scrCls_lib();
		scrPrint_Ex(0, 1, 0x80|2, "       RF阅读器      ","RF READER");
	
		sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: mPiccIdcard \r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	
		if((siRet = piccOpen_lib()) != RET_RF_OK)
		{
			scrPrint_lib(0, 3, 0x02, "open fail");
			sysDelayMs(1000);
			return;
		}
		
		scrClrLine_lib(2,3);
		scrPrint_Ex(0, 2, 0x02, "请刷卡!","PLS SWIPED CARD!");
	
		memset(szSerialInfo,0,10);
		memset(szCID,0,8);
		memset(szOther,0,300);
	
		uiTimerCount = sysGetTicks_lib();  
		while (1 )
		{
			piccReset_lib();
			//scrClrLine_lib(1,6);
			siRet = piccDetect_lib('D',&ucCardType,szSerialInfo,szOther);
			if(0x00 == siRet)
			{
				scrClrLine_lib(3,4);
				scrPrint_Ex(0, 3, 0x02, "寻到D","detect id card!");
				sysDelayMs(1000);
			}
			else
			{
				scrClrLine_lib(3,4);
				sysDelayMs(100);
				scrPrint_lib(0, 3, 0x02, "fail :%d",siRet);			
			}
			memcpy(ApduSend.Cmd,"\x00\xA4\x00\x00",4);
			ApduSend.Lc = 02;
			memcpy(ApduSend.DataIn,"\x60\x02",2);
			ApduSend.Le = 0;
			
			scrClrLine_lib(3,4);
			scrPrint_lib(0, 3, 0x02 ,"apdu:00A4");
		 	siRet = piccIsoCommand_lib(&ApduSend, &ApduResp);
			if(0x00 == siRet)
			{
				scrClrLine_lib(4,5);
				scrPrint_lib(0, 4, 0x02 ,"SWA:%X,%X",ApduResp.SWA,ApduResp.SWB);
				if(ApduResp.LenOut == 0)
					scrPrint_lib(0, 5, 0x02 ,"L:%X",ApduResp.LenOut);
				else
					scrPrint_lib(0, 5, 0x02 ,"L:%X,D:%X%X%X%X",ApduResp.LenOut,ApduResp.DataOut[0],ApduResp.DataOut[1],ApduResp.DataOut[2],ApduResp.DataOut[3]);
				sysDelayMs(1000);
			}
			else
			{
				scrClrLine_lib(4,5);
				sysDelayMs(100);
				scrPrint_lib(0, 4, 0x02, "APDU fail :%d",siRet);			
			}		
			memcpy(ApduSend.Cmd,"\x00\x36\x00\x00",4);
			ApduSend.Lc = 0x00;
			//memcpy(ApduSend.DataIn,"\x60\x02",2);
			ApduSend.Le = 0x08;	
			scrClrLine_lib(3,5);
			scrPrint_lib(0, 3, 0x02 ,"apdu:0036");			
		 	siRet = piccIsoCommand_lib(&ApduSend, &ApduResp);
			if(0x00 == siRet)
			{
				scrClrLine_lib(4,5);
				scrPrint_lib(0, 4, 0x02 ,"SWA:%X,%X",ApduResp.SWA,ApduResp.SWB);
				scrPrint_lib(0, 5, 0x02 ,"L:%X,D:%X%X%X%X",ApduResp.LenOut,ApduResp.DataOut[0],ApduResp.DataOut[1],ApduResp.DataOut[2],ApduResp.DataOut[3]);
				sysDelayMs(1000);
			}
			else
			{
				scrClrLine_lib(4,5);
				sysDelayMs(100);
				scrPrint_lib(0, 4, 0x02, "APDU fail :%d",siRet);			
			}	

			ucKey = kbGetKey_lib();
			if(ucKey == KEYCANCEL)	
			{
				piccClose_lib();
				break;
			}
		}

		sysDelayMs(1000);

		scrClrLine_lib(1,6);


}
void Menu_ModChkPICC(void)
{
	int iSize = 0;
	unsigned char i = 0;
	STMENUNODE astMenu[] = {
            { "非接测试",				"RF Test",       		NoFun    },
//            { "所有类型卡", 				"ALL TYPE CARD", 		McCheckRF    },
//            { "发送ppse",   			"PPSE",        			MRFPiccISOTest   },
            //{ "测试B卡",          "TYPE B",        NoFun   },
            //{ "测试M卡",          "TYPE M",        NoFun   },
            { "测试冲突", 				"TEST COL",      		CheckPoll    },
//            { "测试身份证",   			"TYPE D",   	     	mPiccIdcard},
    };

    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
	return;
}


static void Menu_ScanPreview(void)
{
	int iRet;
	uint8 scanbufftmp[1024];
	int numtmp;
	sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: Menu_Scan enter,\r\n", filename(__FILE__), __FUNCTION__, __LINE__);

	memset(scanbufftmp, 0, sizeof(scanbufftmp));
	
#if 1
	while(1)
	{

		if(kbGetKey_lib() == KEYCANCEL)
		{
			break;
		}

		if(g_ui8LcdType == 0)
		{
			scrCls_lib();
			scrTextOut_lib(0, 24, "scan START!\r\n");
		}
		else


		{

			//hal_scrSelectFont(&g_stSingleFont6X12, NULL);
			//hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
			scrCls_lib();
			scrPrintf_lib("scan START!");
		}		
		sysDelayMs(500);
		g_ui8CameraPreviewEn = 1;
		iRet = scanOpen_lib();
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:scanOpen_lib: %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		scanStart_lib();
		scrCls_lib();
		numtmp = 1000;
		if(iRet == 0)
		{
			do
			{
				numtmp --;
				if(kbGetKey_lib() == KEYCANCEL)
				{
					break;
				}
				iRet = scanCheck_lib();
				if(iRet == 1)
				{
					hal_camScanPreviewByScr();
					memset(scanbufftmp, 0, sizeof(scanbufftmp));
					iRet = scanRead_lib(scanbufftmp, sizeof(scanbufftmp));
					sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:scanRead_lib, iRet:%d, scanbufftmp:%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet, scanbufftmp);
					if(iRet > 0)
					{
						g_ui8CameraPreviewEn = 0;
						if(g_ui8LcdType == 0)
						{
							scrTextOut_lib(0, 24, "scan SUCC, scanbufftmp:\r\n");
							scrTextOut_lib(0, 36, scanbufftmp);
						}
						else

						{
							hal_scrClsArea(0, 24,g_stLcdConfig.COLORLCD_PIXWIDTH-1,g_stLcdConfig.COLORLCD_PIXHIGH-1);
							//hal_clcdRefresh();
							//hal_scrSelectFont(&g_stSingleFont6X12, NULL);
							//hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
							hal_scrClrLine(1, 2);
							scrPrint_lib(0, 1, 0x80, "scanbufftmp:\r\n%s", scanbufftmp);
						}
						sysDelayMs(200);

						break;
					}
				}
				else

				{
					sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:scanRead_lib, iRet:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				}
				sysDelayMs(5);
			}while(numtmp > 0);
			scanClose_lib();
			g_ui8CameraPreviewEn =0;
			if(g_ui8LcdType == 1)
			{
				hal_scrClsArea(0, 24,g_stLcdConfig.COLORLCD_PIXWIDTH-1,g_stLcdConfig.COLORLCD_PIXHIGH-1);
			}
		}
		sysDelayMs(200);
		
	}
#endif
	sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:sweep code osiThreadExit\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
    //fibo_thread_delete();
}


static void Menu_Scan(void)
{
	int iRet;
	uint8 scanbufftmp[1024];
	int numtmp;
    unsigned int succ = 0,err= 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	
	memset(scanbufftmp, 0, sizeof(scanbufftmp));
    scrCls_lib();
    scrPrint_lib(0, 1, 0x02, "Scan...");	
    scrPrint_lib(0, 2, 0,"press any key stop");

	sysDelayMs(500);

	iRet = scanOpen_lib();
	scanStart_lib();
	scrClrLine_lib(3,5);
	numtmp = 0;
	if(iRet == 0)
	{
	    uTimeBegin = hal_sysGetTickms();
		while(1)
		{
			numtmp++;
			if(0x00 == kbHit_lib())
    		{
    			break;
    		}
			iRet = scanCheck_lib();
			if(iRet == 1)
			{
				memset(scanbufftmp, 0, sizeof(scanbufftmp));
				iRet = scanRead_lib(scanbufftmp, sizeof(scanbufftmp));
				if(iRet > 0)
				{
					//scrPrint_lib(0, 2, 0x02, "%s",scanbufftmp);
   					sysBeep_lib();
				    succ++;
				}
				else
				{
				    //scrClrLine_lib(2, 5);
				    err++;
				}
				if(numtmp % 10 == 0)
				{
				    //audioFilePlay_lib("/ext/app/data/scanok.wav");
				    scrClrLine_lib(4,5);
				    uTimeEnd = hal_sysGetTickms();
					scrPrint_lib(0, 3, 0x00, "succ=%d,err=%d",succ,err);	
					scrPrint_lib(0, 4, 0x02, "%s",scanbufftmp);	
					scrPrint_lib(0, 5, 0x00, "time(s)=%d", (uTimeEnd-uTimeBegin) / 1000);	
				}
			}
			//sysDelayMs(5);
		}//while(numtmp > 0);
		scanClose_lib();
	}
	hal_keypadWaitOneKey();
	return; 
}

#if 0
static void DR_ThreadEntry_ScanPlay(void *param)
{
    audioFilePlayPath_lib("/ext/app/data/scaning.mp3");
	//audioFilePlayPath_lib("/ext/app/data/scanok.mp3");
    fibo_thread_delete();
}

static void Menu_ScanOK(void)
{
	int iRet;
	uint8 scanbufftmp[1024];
	int numtmp;
    unsigned int succ = 0,err= 0;
	int iStop = 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	
	memset(scanbufftmp, 0, sizeof(scanbufftmp));
    scrCls_lib();
    scrPrint_lib(0, 1, 0x02, "Scan...");	
    scrPrint_lib(0, 2, 0,"press any key stop");

	sysDelayMs(500);

	iRet = scanOpen_lib();
	scanStart_lib();
	scrClrLine_lib(3,5);
	numtmp = 0;
	if(iRet == 0)
	{
	    uTimeBegin = hal_sysGetTickms();
		while(1)
		{
			numtmp++;
			if(0x00 == kbHit_lib())
    		{
    			break;
    		}
			iRet = scanCheck_lib();
			if(iRet == 1)
			{
				memset(scanbufftmp, 0, sizeof(scanbufftmp));
				iRet = scanRead_lib(scanbufftmp, sizeof(scanbufftmp));
				if(iRet > 0)
				{
                    //sysBeep_lib();
                    fibo_thread_create(DR_ThreadEntry_ScanPlay, "DR_ThreadEntry_ScanPlay", 1024*8, NULL, OSI_PRIORITY_ABOVE_NORMAL);
				    succ++;
					if(iStop > 0)  iStop--;
				}
				else
				{
				    err++;
#if 1
					iStop++;
				    if(iStop >= 5)
				    {
						audioStop_lib();
						iStop = 0;
				    }
#endif
				}
				if(numtmp % 10 == 0)
				{
				    //if(iRet > 0) sysBeep_lib();
				    scrClrLine_lib(4,5);
				    uTimeEnd = hal_sysGetTickms();
					scrPrint_lib(0, 3, 0x00, "succ=%d,err=%d",succ,err);	
					scrPrint_lib(0, 4, 0x02, "%s",scanbufftmp);	
					scrPrint_lib(0, 5, 0x00, "time(s)=%d", (uTimeEnd-uTimeBegin) / 1000);	
				}
			}
			//sysDelayMs(5);
		}//while(numtmp > 0);
		scanClose_lib();
	}
	hal_keypadWaitOneKey();
	return; 
}
#endif


void Menu_ModLed()
{
	unsigned char ucKey = 0xFF;
	scrCls_lib();
	//Menu_uiSetTile("LED测试", "LED TEST");
	//hal_scrSelectFont(&g_stSingleFont6X12, NULL);
	//hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
	//hal_scrClrLine(1, 2);
	scrPrint_lib(0, 3, 2, "LED TEST...");
	scrPrint_lib(0, 4, 2,  "[Cancel]:EXIT");
	scrPrint_lib(0, 5, 2,  "[ANYKEY]:RETRY");
	kbFlush_lib();
	while (1)
	{
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "PICC BULE LED ON");
		piccLight_lib(PICCLEDBLUE, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "PICC YELLOW LED ON");
		piccLight_lib(PICCLEDBLUE, 0);
		piccLight_lib(PICCLEDYELLOW, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "PICC GREEN LED ON");
		piccLight_lib(PICCLEDYELLOW, 0);
		piccLight_lib(PICCLEDGREEN, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "PICC RED LED ON");
		piccLight_lib(PICCLEDGREEN, 0);
		piccLight_lib(PICCLEDRED, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "RED LED ON");
		piccLight_lib(PICCLEDRED, 0);
		piccLight_lib(LEDRED, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "BLUE LED ON");
		piccLight_lib(LEDRED, 0);
		piccLight_lib(LEDBLUE, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "YELLOW LED ON");
		piccLight_lib(LEDBLUE, 0);
		piccLight_lib(LEDYELLOW, 1);
		sysDelayMs_lib(1000);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2, "ALL LED ON");
		//piccLight_lib(PICCLEDRED, 0);
		//piccLight_lib(0xff, 1);
		piccLight_lib(PICCLEDBLUE, 1);
		piccLight_lib(PICCLEDYELLOW, 1);
		piccLight_lib(PICCLEDGREEN, 1);
		piccLight_lib(PICCLEDRED, 1);
		piccLight_lib(LEDRED, 1);
		piccLight_lib(LEDBLUE, 1);
		piccLight_lib(LEDYELLOW, 1);
		sysDelayMs_lib(1000);
		//piccLight_lib(0xff, 0);
		piccLight_lib(PICCLEDBLUE, 0);
		piccLight_lib(PICCLEDYELLOW, 0);
		piccLight_lib(PICCLEDGREEN, 0);
		piccLight_lib(PICCLEDRED, 0);
		piccLight_lib(LEDRED, 0);
		piccLight_lib(LEDBLUE, 0);
		piccLight_lib(LEDYELLOW, 0);
		scrClrLine_lib(2, 3);
		scrPrint_lib(0, 2, 2,  "ALL LED OFF");
		while(1)
		{
				ucKey = kbGetKey_lib();
				if(ucKey == 0xFF)
				{					
					continue;
				}
				if (ucKey == KEYCANCEL)
				{
					piccLight_lib(0xff, 0);
					return;
				}
				break;
		}
	}

}
void McCheckKBTest(void)
{
	unsigned char uckey = 0xff;
	char display[5] = { 0 };
	scrCls_lib();
	//hal_scrSelectFont(&g_stSingleFont6X12, NULL);
	//hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
	//hal_scrClrLine(1, 2);
	scrPrint_lib(0, 1, 2, "KEY TEST...");
	//api_scrClsEcpIcon(LCD_COLOR_WHITE);
	//Menu_uiSetTile("键码测试", "KEY TEST");
	scrPrint_lib(0, 3, 2, "[Cancel]:EXIT");
	while (1)
	{
		uckey = kbGetKey_lib();
		if (uckey == 0xff) continue;
		if (uckey == KEYCANCEL)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "Cancel");
			sysDelayMs_lib(100);
			return;
		}
		if (uckey >= '0' && uckey <= '9')
		{
			sprintf(display, "%c", uckey);
			//ttsQueuePlay_lib(display, NULL, NULL, 0);
			//if(fibo_tts_is_playing() == TRUE)
				//fibo_tts_stop();
			//hal_ttsPlay(display, 0, 0);
			sysBeepF_lib(uckey%7,50);
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, display);
			continue;
		}
		if (uckey == KEYENTER)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "OK");
			continue;
		}
		if (uckey == KEYCLEAR)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "DELETE");
			continue;
		}
		if(uckey == KEYSTAR)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "*");
			continue;
		}
		if(uckey == KEYSHARP)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "#");
			continue;
		}
		if(uckey == KEY_FN)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "Function Key");
			continue;
		}
		if(uckey == KEYF1)
		{

			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "KEYF1");
			continue;

		}
		if(uckey == KEYF2)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "KEYF2");
			continue;

		}
		if(uckey == KEYPOINT)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "POINT");
			continue;

		}
		if(uckey == KEYDCASH)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "DCASH");
			continue;

		}
		if(uckey == KEYMENU)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "MENU");
			continue;

		}
		if(uckey == KEYPLUS)
		{
			scrClrLine_lib(2, 3);
			scrPrint_lib(0, 2, 2, "+");
			continue;

		}
		//sysDelayMs(20);
		
	}
}

void McCheckKBStringTest(void)
{
	unsigned char uckey = 0xff;
	char display[5] = { 0 };
	unsigned char outStrtmp[128];

	memset(outStrtmp, 0, sizeof(outStrtmp));
	
	scrCls_lib();
	
	scrPrint_lib(0, 1, 2, "KEY TEST...");
	
	scrPrint_lib(0, 3, 2, "[Cancel]:EXIT\n");
	
	while (1)
	{
		kbGetString_lib(outStrtmp, 0b00100001, 0, 128, 20);
		kbGetString_lib(outStrtmp, 0b00101001, 0, 128, 20);
		kbGetString_lib(outStrtmp, 0b00100101, 0, 128, 20);
		kbGetString_lib(outStrtmp, 0b00100000, 0, 128, 20);
		kbGetString_lib(outStrtmp, 0b00111001, 0, 128, 20);
		kbGetString_lib(outStrtmp, 0b00000011, 0, 128, 20);
		kbGetString_lib(outStrtmp, 0b00000000, 0, 128, 20);
		sysLOG_lib(LCD_LOG_LEVEL_1, "[%s] -%s- Line=%d: \r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		break;//sysDelayMs_lib(100);
		
	}
}


void Menu_CrdMsg(void)
{
	int iRet = 0;
	unsigned char ucKey = 0;
	GPS_Coordinates s_GPSData;
	char Lon_area[2]={0,0};
	char Lat_area[2]={0,0};
	char bufffer[15];
	char DirectionBuf[2];

	scrCls_lib();
	//sysDelayMs_lib(500);
	scrPrint_lib(0, 2, 2, "GPS Crd TEST");
	kbFlush_lib();
	iRet = gpsPowerUp_lib();

	if(iRet != 0)
	{

		scrClrLine_lib(4, 5);

		scrPrint_lib(0, 4, 2, "GPS Power Up Fail");

		while(1)
		{
			if(kbHit_lib()==0) 
			{

				ucKey = kbGetKey_lib();

				if(ucKey != 0xFF)
				{
					return;
				}
			}

			sysDelayMs_lib(50);
		}

	}



	scrClrLine_lib(4, 5);

	scrPrint_lib(0, 4, 2, "GPS Power Up ...");

	sysDelayMs_lib(2200);

	while(gpsQueryCrtState_lib() != 0)
	{
			scrClrLine_lib(4, 5);

			scrPrint_lib(0, 4, 2, "Query Power Up Fail");

			while(1)
			{
				if(kbHit_lib()==0) 
				{
					ucKey = kbGetKey_lib();

					if(ucKey != 0xFF)
					{
						gpsClose_lib();

						return;
					}

				}

				sysDelayMs_lib(50);
			}
	}



	scrClrLine_lib(4, 5);
	scrPrint_lib(0, 4, 0, "Longitude:");
	scrPrint_lib(0, 7, 0, "Latitude:");
	sysDelayMs_lib(500);
	//sysDelayMs_lib(2000);

	while(1)
	{
		scrCls_lib();

		scrPrint_lib(0, 2, 2, "GPS TEST");

		iRet = getGPSCoordinates_lib(&s_GPSData);

		if(iRet == 0)
		{
			scrPrint_lib(0, 4, 0, "Longitude: %09f",s_GPSData.Lon);
			scrPrint_lib(0, 7, 0, "Latitude:  %09f",s_GPSData.Lat);
		}
		else{
			scrPrint_lib(0, 5, 2, "GPS No Data!");
		}

	    sysDelayMs_lib(1000);
		if(kbHit_lib()==0)
		{
		ucKey = kbGetKey_lib();

		if (ucKey == KEYCANCEL)
		{
			iRet = gpsClose_lib();

			if(iRet != 0)
			{
				scrCls_lib();

				scrPrint_lib(0, 2, 2, "GPS TEST");

			    scrPrint_lib(0, 4, 2, "GPS Close Fail");

				while(1)
				{

					if(kbHit_lib()==0) 
					{
						ucKey = kbGetKey_lib();

						if(ucKey != 0xFF)
						{
							return;
						}

					}

					sysDelayMs_lib(50);

				}

			}
			return;
		} 
		}
	}
}



void Menu_ELVMsg(void)
{
	int iRet = 0;
	unsigned char QiutFlag = 0;
	unsigned char ucKey = 0;

	scrCls_lib();
	GPSGSAData GpsGSVData;
	memset(&GpsGSVData, 0x00, sizeof(GPSGSAData));
	//sysDelayMs_lib(500);
	scrPrint_lib(0, 2, 2, "GPS Elv TEST");
	kbFlush_lib();
	iRet = gpsPowerUp_lib();

	if(iRet != 0)
	{
		scrClrLine_lib(4, 5);
		scrPrint_lib(0, 4, 2, "GPS Power Up Fail");

		while(1)
		{
			if(kbHit_lib()==0) 
			{
				ucKey = kbGetKey_lib();

				if(ucKey != 0xFF)
				{
					return;
				}
			}
			sysDelayMs_lib(50);
		}
	}

	scrClrLine_lib(4, 5);
	scrPrint_lib(0, 4, 2, "GPS Power Up ...");
	sysDelayMs_lib(2200);

	while(gpsQueryCrtState_lib() != 0)
	{
			scrClrLine_lib(4, 5);
			scrPrint_lib(0, 4, 2, "Query Power Up Fail");
			while(1)
			{
				if(kbHit_lib()==0) 
				{
					ucKey = kbGetKey_lib();
					if(ucKey != 0xFF)
					{
						gpsClose_lib();
						return;
					}
				}

				sysDelayMs_lib(50);
			}
	}

	scrClrLine_lib(4, 5);
	sysDelayMs_lib(500);
	scrPrint_lib(0, 3, 2, "Select 0 ~ 9 to view");

	while(1)
	{
       iRet = GetGSV_lib(&GpsGSVData);
	   if(iRet==0)
	   {
		   while(1)
		   {
			   if(kbHit_lib()==0) 
			   {
				scrCls_lib();
				scrPrint_lib(0, 2, 2, "GPS Elv TEST");
				scrPrint_lib(0, 3, 2, "Select 0 ~ 9 to view");
				scrPrint_lib(0, 4, 0, "GPSNum=%d", GpsGSVData.GPNoSv);
				scrPrint_lib(0, 4, 4, "GBNum=%d", GpsGSVData.GBNoSv);

				ucKey = kbGetKey_lib();
				switch(ucKey)
				{
					case KEY0:
					//scrClrLine_lib(4,8);

					scrPrint_lib(0, 5, 0, "sv1=%d", GpsGSVData.sv[0]);

					scrPrint_lib(0, 5, 4, "sv2=%d", GpsGSVData.sv[1]);

					scrPrint_lib(0, 6, 0, "elv1=%d", GpsGSVData.elv[0]);

					scrPrint_lib(0, 6, 4, "elv2=%d", GpsGSVData.elv[1]);

					scrPrint_lib(0, 7, 0, "az1=%d", GpsGSVData.az[0]);

					scrPrint_lib(0, 7, 4, "az2=%d", GpsGSVData.az[1]);
					break;
					
					case KEY1:
					//scrClrLine_lib(4,8);
					scrPrint_lib(0, 5, 0, "sv3=%d", GpsGSVData.sv[2]);

					scrPrint_lib(0, 5, 4, "sv4=%d", GpsGSVData.sv[3]);

					scrPrint_lib(0, 6, 0, "elv3=%d", GpsGSVData.elv[2]);

					scrPrint_lib(0, 6, 4, "elv4=%d", GpsGSVData.elv[3]);

					scrPrint_lib(0, 7, 0, "az3=%d", GpsGSVData.az[2]);

					scrPrint_lib(0, 7, 4, "az4=%d", GpsGSVData.az[3]);

					break;

					case KEY2:
					//scrClrLine_lib(4,8);
					scrPrint_lib(0, 5, 0, "sv5=%d", GpsGSVData.sv[4]);

					scrPrint_lib(0, 5, 4, "sv6=%d", GpsGSVData.sv[5]);

					scrPrint_lib(0, 6, 0, "elv5=%d", GpsGSVData.elv[4]);

					scrPrint_lib(0, 6, 4, "elv6=%d", GpsGSVData.elv[5]);

					scrPrint_lib(0, 7, 0, "az5=%d", GpsGSVData.az[4]);

					scrPrint_lib(0, 7, 4, "az6=%d", GpsGSVData.az[5]);

					break;

					case KEY3:
					//scrClrLine_lib(4,8);
					scrPrint_lib(0, 5, 0, "sv7=%d", GpsGSVData.sv[6]);

					scrPrint_lib(0, 5, 4, "sv8=%d", GpsGSVData.sv[7]);

					scrPrint_lib(0, 6, 0, "elv7=%d", GpsGSVData.elv[6]);

					scrPrint_lib(0, 6, 4, "elv8=%d", GpsGSVData.elv[7]);

					scrPrint_lib(0, 7, 0, "az7=%d", GpsGSVData.az[6]);

					scrPrint_lib(0, 7, 4, "az8=%d", GpsGSVData.az[7]);

					break;

					case KEY4:
					//scrClrLine_lib(4,8);
					scrPrint_lib(0, 5, 0, "sv9=%d", GpsGSVData.sv[8]);

					scrPrint_lib(0, 5, 4, "sv10=%d", GpsGSVData.sv[9]);

					scrPrint_lib(0, 6, 0, "elv9=%d", GpsGSVData.elv[8]);

					scrPrint_lib(0, 6, 4, "elv10=%d", GpsGSVData.elv[9]);

					scrPrint_lib(0, 7, 0, "az9=%d", GpsGSVData.az[8]);

					scrPrint_lib(0, 7, 4, "az10=%d", GpsGSVData.az[9]);

					break;

					case KEY5:
					//scrClrLine_lib(4,8);
					scrPrint_lib(0, 5, 0, "sv11=%d", GpsGSVData.sv[10]);

					scrPrint_lib(0, 5, 4, "sv12=%d", GpsGSVData.sv[11]);

					scrPrint_lib(0, 6, 0, "elv11=%d", GpsGSVData.elv[10]);

					scrPrint_lib(0, 6, 4, "elv12=%d", GpsGSVData.elv[11]);

					scrPrint_lib(0, 7, 0, "az11=%d", GpsGSVData.az[10]);

					scrPrint_lib(0, 7, 4, "az12=%d", GpsGSVData.az[11]);

					break;

					case KEY6:
					scrPrint_lib(0, 5, 0, "sv13=%d", GpsGSVData.sv[12]);

					scrPrint_lib(0, 5, 4, "sv14=%d", GpsGSVData.sv[13]);

					scrPrint_lib(0, 6, 0, "elv13=%d", GpsGSVData.elv[12]);

					scrPrint_lib(0, 6, 4, "elv14=%d", GpsGSVData.elv[13]);

					scrPrint_lib(0, 7, 0, "az13=%d", GpsGSVData.az[12]);

					scrPrint_lib(0, 7, 4, "az14=%d", GpsGSVData.az[13]);

					break;

					case KEY7:

					scrPrint_lib(0, 5, 0, "sv15=%d", GpsGSVData.sv[14]);

					scrPrint_lib(0, 5, 4, "sv16=%d", GpsGSVData.sv[15]);

					scrPrint_lib(0, 6, 0, "elv15=%d", GpsGSVData.elv[14]);

					scrPrint_lib(0, 6, 4, "elv16=%d", GpsGSVData.elv[15]);

					scrPrint_lib(0, 7, 0, "az15=%d", GpsGSVData.az[14]);

					scrPrint_lib(0, 7, 4, "az16=%d", GpsGSVData.az[15]);

					break;

					case KEY8:

					scrPrint_lib(0, 5, 0, "sv17=%d", GpsGSVData.sv[16]);

					scrPrint_lib(0, 5, 4, "sv18=%d", GpsGSVData.sv[17]);

					scrPrint_lib(0, 6, 0, "elv17=%d", GpsGSVData.elv[16]);

					scrPrint_lib(0, 6, 4, "elv18=%d", GpsGSVData.elv[17]);

					scrPrint_lib(0, 7, 0, "az17=%d", GpsGSVData.az[16]);

					scrPrint_lib(0, 7, 4, "az18=%d", GpsGSVData.az[17]);

					break;

					case KEY9:

					scrPrint_lib(0, 5, 0, "sv19=%d", GpsGSVData.sv[18]);

					scrPrint_lib(0, 5, 4, "sv20=%d", GpsGSVData.sv[19]);

					scrPrint_lib(0, 6, 0, "elv19=%d", GpsGSVData.elv[18]);

					scrPrint_lib(0, 6, 4, "elv20=%d", GpsGSVData.elv[19]);

					scrPrint_lib(0, 7, 0, "az19=%d", GpsGSVData.az[18]);

					scrPrint_lib(0, 7, 4, "az20=%d", GpsGSVData.az[19]);

					break;

					case KEYCANCEL:
					gpsClose_lib();
					return ;
					//break;
					default:
					QiutFlag = 1;
					break;
				}
				kbFlush_lib();
			  }

			  if(QiutFlag == 1)
			  {
				  scrCls_lib();

					scrPrint_lib(0, 2, 2, "GPS Elv TEST");

					scrPrint_lib(0, 3, 2, "Select 0 ~ 9 to view");

				  break;

			  }

			  sysDelayMs_lib(50);

		   }

	   }
	   else{
		    scrCls_lib();

			scrPrint_lib(0, 2, 2, "GPS Elv TEST");

			scrPrint_lib(0, 3, 2, "Select 0 ~ 9 to view");

		    scrPrint_lib(0, 5, 2, "Get GPS elv fail!");

	   }

		sysDelayMs_lib(1000);

	   if((kbHit_lib()==0) && (QiutFlag == 0))
	   {
		   ucKey = kbGetKey_lib();

		   if(ucKey == KEYCANCEL)
		   {
			   iRet = gpsClose_lib();

				if(iRet != 0)
				{
					scrCls_lib();

					scrPrint_lib(0, 2, 2, "GPS TEST");

					scrPrint_lib(0, 4, 2, "GPS Close Fail");

					while(1)
					{

						if(kbHit_lib()==0) 
						{

							ucKey = kbGetKey_lib();

							if(ucKey != 0xFF)
							{
								return;
							}

						}

						sysDelayMs_lib(50);

					}

				}
			   return;

		   }

		   kbFlush_lib();

	   }

	   QiutFlag = 0;

	   scrClrLine_lib(5,6);
	}
}

void Menu_GetGPSMsg(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
			{ "基础项", "BasicFunc", NoFun },
            { "坐标", "CrdMsg", Menu_CrdMsg},
			{ "仰角", "ELVMsg", Menu_ELVMsg},	
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    //SetMiddleSizeFont();
    Menu_ShowStandardMenu(astMenu, iSize);
    //SetLargeSizeFont();
    return;
}
void Menu_ClientLCD(void)
{
	char TestCounter=1;
	unsigned char ucKey = 0;
	int Amount = 9999999; 
	scrCls_lib();
	scrPrint_lib(0, 2, 2, "Client LCD TEST");
	if(dpyInit_lib()!=0)
	{
		scrPrint_lib(0, 4, 2, "Init fail!");
		sysDelayMs(1000);
		return;
	}
	while(1)
	{	
		scrCls_lib();
	    scrPrint_lib(0, 2, 2, "Client LCD TEST");
		if((TestCounter%3) == 1)
		{
			if(dpySetBackLight_lib(0)!=0)
			{
				scrPrint_lib(0, 4, 2, "Back Light off fail!");
			}
			else{
				scrPrint_lib(0, 4, 2, "Back light off successfully!");
			}
			sysDelayMs(1000);
			scrClrLine_lib(4, 6);
			if(dpySetBackLight_lib(1)!=0)
			{
				scrPrint_lib(0, 4, 2, "Back Light on fail!");
			}
			else{
				scrPrint_lib(0, 4, 2, "Back Light on successfully!");
			}
			sysDelayMs(1000);
		}
		else if((TestCounter%3) == 2)
		{
			if(dpyDispAmount_lib(Amount)==0)
			{
				scrPrint_lib(0, 4, 0, "Amount=%d",Amount);
			    Amount--;
				if(Amount==0)
				{
					Amount=9999999;
				}
			}
			else{
				scrPrint_lib(0, 4, 2, "Show amount fail!");
			}
			sysDelayMs(1000);
		}
		else{
			if(dpyCls_lib()==0)
			{
				scrPrint_lib(0, 4, 2, "Clear Screen");
			}
			else{
				scrPrint_lib(0, 4, 2, "Clear Screen fail!");
			}
			sysDelayMs(1000);
		}
		TestCounter++;
		if(TestCounter > 9999999)
		TestCounter=1;
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			dpySetBackLight_lib(0);
			return;
		} 
	}
}


void Menu_HighPowerWaste()
{
	unsigned char ucKey = 0xFF;
	int iRet = 0;
	int isockid = 0;
	unsigned char out_info[30] = {0};
	uchar ucCardType,szSerialInfo[10],szCID[8],szOther[300];
	char cSendBuff[1024];
	char cRecvBuff[1024];
	ST_AP_LIST stAplist[30];
	memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	
	scrCls_lib();
	scrPrint_Ex(0, 1, 2, "高功耗测试", "High Power TEST...");
	scrPrint_lib(0, 2, 2,   "[Cancel]:EXIT");
	//scrPrint_lib(0, 5, 2,  "[ANYKEY]:RETRY");
	kbFlush_lib();
	//读配置文件
	//sysGetTermInfo_lib (out_info);
	while (1)
	{
        //打开扫码
        scrClrLine_lib(3,3);
		scrPrint_lib(0, 3, 0x02, "scan Opening ..."); 
		if (g_iCamera_exist==1)
		{
			iRet = scanOpen_lib();
			scanStart_lib();
		}

		scrClrLine_lib(3,3);
		if(iRet ==0)
		{
			scrPrint_lib(0, 3, 0x02, "scan Open OK"); 
		}
		else
		{
			scrPrint_lib(0, 3, 0x02, "scan Open faild"); 
		}
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			break;
		}
		//打开非接
		scrClrLine_lib(4,4);
		scrPrint_lib(0, 4, 0x02, "picc Opening ..."); 
        iRet = piccOpen_lib();
        piccDetect_lib(0,&ucCardType,szSerialInfo,szOther);
		scrClrLine_lib(4,4);
		if(iRet ==0)
		{
			scrPrint_lib(0, 4, 0x02, "picc Open OK"); 
		}
		else
		{
			scrPrint_lib(0, 4, 0x02, "picc Open faild"); 
		}
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			break;
		}
		//打开wifi
		scrClrLine_lib(5,5);
		scrPrint_lib(0, 5, 0x02, "wifi opening ..."); 
		iRet = wifiOpen_lib();
		wifiScan_lib(stAplist, 30);
		scrClrLine_lib(5,6);
 		if(iRet == 0)
		{
			scrPrint_lib(0, 5, 0x02, "wifi open OK"); 
		}
		else
		{
			scrPrint_lib(0, 5, 0x02, "wifi open faild"); 
		}
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			break;
		}
		//打开4G
		scrClrLine_lib(6,6);
		scrPrint_lib(0, 6, 0x02, "4G Connecting ..."); 
		iRet = wirelessPppOpen_lib(NULL, NULL, NULL);
		scrClrLine_lib(6,6);
		if(iRet ==0)
		{
			iRet =	wirelessCheckPppDial_lib();
			if(iRet ==0)
			{
				isockid = wirelessSocketCreate_lib(0);
				if(isockid > 0)
				{
					iRet = wirelessTcpConnect_lib(isockid, "103.235.231.21", "3000", 1000);
					if(iRet == 0)
					{
						scrPrint_lib(0, 6, 0x02, "4G Connect OK"); 
						iRet = wirelessSend_lib(isockid,cSendBuff, 1024);
						if(iRet == 1024)
						{
						    scrClrLine_lib(6,6);
						    scrPrint_lib(0, 6, 0x02, "4G Send OK"); 
						}
						memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
						iRet = wirelessRecv_lib(isockid, cRecvBuff, 1024, 1000);
						if(0 == memcmp(cSendBuff, cRecvBuff, 1024))
						{
						    scrClrLine_lib(6,6);
						    scrPrint_lib(0, 6, 0x02, "4G Recve OK"); 
						}
					}
				}
				else
				{
					scrPrint_lib(0, 6, 0x02, "4G Connect3 faild"); 
				}
			}
			else
			{
				scrPrint_lib(0, 6, 0x02, "4G Connect2 faild"); 
			}
		}
		else
		{
			scrPrint_lib(0, 6, 0x02, "4G Connect1 faild"); 
		}
		//
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			break;
		}
	}
	
	if (g_iCamera_exist==1)
	{
		scanClose_lib();
	}

	piccClose_lib();
	wifiClose_lib();
	wirelessSocketClose_lib(isockid);
	wirelessPppClose_lib();
}

void Menu_LowPowerWaste()
{
	unsigned char ucKey = 0xFF;
	scrCls_lib();
	scrPrint_Ex(0, 3, 2, "低功耗测试","Low Power TEST...");
	scrPrint_lib(0, 4, 2,  "[Cancel]:EXIT");
	scrPrint_lib(0, 5, 2,  "[ANYKEY]:RETRY");
	kbFlush_lib();
	while (1)
	{
		scrClrLine_lib(2, 3);
		scrPrint_Ex(0, 2, 2, "开发中...", "developing...");
		ucKey = kbGetKey_lib();

		if (ucKey == KEYCANCEL)
		{
			return;
		}
	}
}

void Menu_SE_SleepMode(void)
{
	unsigned char ucKey = 0;
	unsigned char DownCtrl[5]={'1','1','1','1',0x00};
	unsigned char ModeFlag = 0;

	scrCls_lib();
	scrPrint_lib(0, 1, 2, "Sleep Mode TEST");
	scrPrint_lib(0, 3, 2, "[SleepMode]:KEYENTER");
	scrPrint_lib(0, 4, 2, "[QuitSleep]:KEYCANCEL");

	while(1)
	{
				ucKey = kbGetKey_lib();
				if(ucKey == 0xFF)
				{					
					continue;
				}

				if (ucKey == KEYCANCEL)
				{
					if(ModeFlag==1)
					{
						SE_awaken();
					}
					return;
				}

				if(ModeFlag == 0)
				{
					if(ucKey == KEYENTER)
					{
						scrClrLine_lib(5, 6);
						if(SE_Sleep(DownCtrl)==0)
					    {
							scrPrint_lib(0, 5, 2, "SE set sleep mode!");
						}
						else
						{
							scrPrint_lib(0, 5, 0, "SE set sleep mode fail!");
						}	
						ModeFlag = 1;
					}	
				}
				sysDelayMs_lib(50);
	}
}

void Menu_SleepMode()
{
	//1.restore lcd
	unsigned char keyval;
	uint8_t * popData;
	int datalen = 128*(96-12)/4;

	scrCls_lib();
	scrPrint_lib(0, 1, 0,"press key to sleep");

	hal_keypadWaitOneKey();	



	while(1)
	{
		if(kbHit_lib()==0) 
		{
			keyval = kbGetKey_lib();
			if(keyval == 0xFF)
			{
				sysDelayMs(100);
				continue;
			}
			switch(keyval)
			{
				case '1':
					scrClrLine_lib(3, 3);
					scrPrint_lib(0, 3, 0,"close wifi!");
					wifiClose_lib();
				break;
				case '2':
					scrClrLine_lib(3, 3);
					scrPrint_lib(0, 3, 0,"dypClose");
					dpyInit_lib();
					dpySetBackLight_lib(0);
				break;
				case '3':
					scrClrLine_lib(3, 3);
					SE_Sleep("1111\x00");
					scrPrint_lib(0, 3, 0,"se sleep!!");
					sysDelayMs(2000);
				break;
				case '4':
					scrClrLine_lib(3, 4);
					scrPrint_lib(0, 4, 0,"-----!!");
					sysDelayMs(2000);	
					pmSleep_lib(NULL);
					scrPrint_lib(0, 4, 0,"exit!");
					sysDelayMs(2000);

				break;
				case KEYCANCEL:
					scrClrLine_lib(3, 4);
					//SE_Sleep("1111\x00");
					scrPrint_lib(0, 3, 0,"CANCLE!!");
					sysDelayMs(2000);
					goto sleep;
				break;
				
				default:
					break;
			}
		}
	
		sysDelayMs(100);
	}

	
sleep:

	#if 0
	scrClrLine_lib(3, 4);
	scrPrint_lib(0, 4, 0,"-----!!");
	sysDelayMs(2000);	
	
	popData = (unsigned char*) fibo_malloc(datalen);
	scrPopDot_lib(popData,datalen);
	//2.sleep
	pmSleep_lib(NULL);
	//3.resume lcd
	scrPushDot_lib(popData,datalen);

	scrPrint_lib(0, 3, 0,"wakeup");
	
	hal_keypadWaitOneKey();	

	scrPrint_lib(0, 4, 0,"exit!");
	sysDelayMs(2000);
	#else
	scrPrint_lib(0, 4, 0,"user cancel       !");
	sysDelayMs(2000);

	#endif
}

void Menu_PowerWaste(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
            { "功耗", "power waste", NoFun },
            { "SE高功耗", "SE High Power",Menu_HighPowerWaste},               
            { "SE低功耗", "SE Low Power",Menu_LowPowerWaste},  
			{ "SE睡眠模式", "SE_SleepMode", Menu_SE_SleepMode},
			{ "整机睡眠模式", "AP SleepMode", Menu_SleepMode},
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;
}


void Menu_Scanpress()
{
	int iRet;
	uint8 scanbufftmp[1024];
	int numtmp;
    unsigned int succ = 0,err= 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	int value = 2,ccount = 0;
		
	memset(scanbufftmp, 0, sizeof(scanbufftmp));
    scrCls_lib();
    scrPrint_lib(0, 1, 0x02, "Scan...");	
    scrPrint_lib(0, 2, 0,"press any key stop");

	sysDelayMs(500);


	numtmp = 0;

	    uTimeBegin = hal_sysGetTickms();
		while(1)
		{

			scrClrLine_lib(3,5);
			//scanClose_lib();
			//scanClose_lib();
			
			iRet = scanOpen_lib();
			sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:scanOpen_lib iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			if(iRet != 0)
			{
				scrPrint_lib(0, 3, 0x00, "scanOpen fail=%d",iRet);

				hal_keypadWaitOneKey();

			}
			numtmp++;
			if(0x00 == kbHit_lib())
    		{
    			break;
    		}
			ccount = 0;
			while(1)
			{
				iRet = scanCheck_lib();
				sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:scanCheck_lib iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				if(iRet == 1)
					break;
				//sysDelayMs(100);
				//ccount++;
				//if(ccount > 50)
				//	break;
			}
			if(iRet == 1)
			{
				memset(scanbufftmp, 0, sizeof(scanbufftmp));
				iRet = scanRead_lib(scanbufftmp, sizeof(scanbufftmp));
				sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d:scanRead_lib iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				if(iRet > 0)
				{
					//scrPrint_lib(0, 2, 0x02, "%s",scanbufftmp);
   					//sysBeep_lib();
				    succ++;
					//hal_ttsQueuePlay("扫码成功", &value, NULL, 0);
					//sysDelayMs(2000);
				}
				else
				{
				    //scrClrLine_lib(2, 5);
				    //hal_ttsQueuePlay("扫码失败", &value, NULL, 0);
					//sysDelayMs(2000);
				    err++;
				}
				if(numtmp % 10 == 0)
				{
				    //audioFilePlay_lib("/ext/app/data/scanok.wav");
				    scrClrLine_lib(4,5);
				    uTimeEnd = hal_sysGetTickms();
					scrPrint_lib(0, 3, 0x00, "succ=%d,err=%d",succ,err);	
					scrPrint_lib(0, 4, 0x02, "%s",scanbufftmp);	
					scrPrint_lib(0, 5, 0x00, "time(s)=%d", (uTimeEnd-uTimeBegin) / 1000);	
					sysDelayMs(500);
				}
			}
			
			scanClose_lib();

			
		}//while(numtmp > 0);
		
	hal_keypadWaitOneKey();
	return; 

}
void Menu_ScanFunc(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
            { "扫码", "ScanFunc", NoFun },
            { "解码测试", "Scan", Menu_Scan}, 
            //{ "解码测试+声音", "Scan+OK", Menu_ScanOK}, 
            { "扫码预览", "ScanPreview", Menu_ScanPreview}, 
            //{ "开关压力", "openClosePress", Menu_Scanpress}, 
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;
}


extern INT32 fibo_ffsmountExtflash(		UINT32 uladdr_start, UINT32 ulsize,char *dir,
									UINT8 spi_pin_sel,bool format_on_fail,
									bool force_format);



void Menu_exFileSysmount()
{
	int iRet;
	scrCls_lib();
	scrPrint_lib(0,1,0,"mount file press");
	hal_keypadWaitOneKey();

	/*iRet = hal_flashAllErase(4*1024*1024, 256);
	scrPrint_lib(0,1,0,"hal_flashAllErase iret:%d",iRet);
	hal_keypadWaitOneKey();*/
	iRet = fibo_file_getFreeSize();
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_getFreeSize lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	

	iRet = fibo_file_getFreeSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_getFreeSize_ex lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_getTotalSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_getTotalSize_ex lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	
	iRet = fibo_ffsmountExtflash(g_strSpiFlashInfo.flash_mountaddr, g_strSpiFlashInfo.flash_mountsize, EXT_FS_DIR, 
		FLASH_SPI_PINSEL, true, false);
	scrPrint_lib(0,2,0,"1iRet = %d",iRet);
	hal_keypadWaitOneKey();

	

	iRet = fibo_file_getFreeSize();
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_getFreeSize lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_getFreeSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_getFreeSize_ex lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_getTotalSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_getTotalSize_ex lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	//scrPrint_lib(0,3,0,"len = %d     ",iRet);
	hal_keypadWaitOneKey();	
	
	iRet = fibo_file_open("/ext/text", O_CREAT|O_RDWR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fileOpen_lib iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	if(iRet < 0)
	{
		iRet = FILE_NOEXIST;
	}
	scrPrint_lib(0,2,0,"iRet = %d     ",iRet);
	hal_keypadWaitOneKey();		
	
	iRet = fibo_file_close(iRet);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_close iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
}

void filesys_dirtest()
{
	int fd;
	int iRet;
	
	uint8_t data[32],data_r[32];

	
	//1.在确认已经创建了文件系统的前提下 
	//写入文件
	fd = fibo_file_open("/ext/text", O_RDWR);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fileOpen_lib fd=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, fd);
	if(fd < 0)
	{
		fd = FILE_NOEXIST;
	}
	iRet = fibo_file_getFreeSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:before write lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_getTotalSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:before write lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_seek(fd,0,FS_SEEK_END);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_seek iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	
	memset(data,0x31,sizeof(data));
	iRet = fibo_file_write(fd,data,32);
	if(iRet<0 )
	{
		sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_write iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		goto exit;
	}

	iRet = fibo_file_close(fd);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_close iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	iRet = fibo_file_getFreeSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:after write lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_getTotalSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:after write lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	fd = fibo_file_open("/ext/text", O_RDWR);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fileOpen_lib O_RDWR fd=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, fd);
	if(fd < 0)
	{
		fd = FILE_NOEXIST;
	}	
	memset(data_r,0,sizeof(data_r));
	iRet = fibo_file_read(fd,data_r,32);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_read iRet=%d data:%x \r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet,data_r[0]);
	if(iRet<0 )
	{
		goto exit;
	}	

	iRet = fibo_file_close(fd);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_close iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	if(0 == memcmp(data,data_r,32))
	{
		sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:read write is same\r\n", filename(__FILE__), __FUNCTION__, __LINE__);

	}

	//再次获取 看是否存在对应路径
	fd = fibo_file_open("/ext/text", O_RDWR);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fileOpen_lib confirm fd=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, fd);
	if(fd < 0)
	{
		fd = FILE_NOEXIST;
	}
	memset(data_r,0,sizeof(data_r));
	iRet = fibo_file_read(fd,data_r,32);	
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_read iRet=%d data:%x \r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet,data_r[0]);
	if(iRet<0 )
	{
		goto exit;
	}	

exit:
	iRet = fibo_file_close(fd);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_close iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);	

}
void deleteTestFile()
{
	int iRet;
	iRet = fileRemove_lib("/FileSysTest.txt");	
	sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: fileRemove_lib iRet=%d, size:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet, fileInGetFileSysFreeSize_lib());
	iRet = fileRemove_lib("/app/ufs/UserAPP.bin");	
	sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: fileRemove_lib iRet=%d, size:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet, fileInGetFileSysFreeSize_lib());

	int i ;

	char sendbuf[32];
	
	
	
	/*for(i=1;i<1;i++)
	{
		memset(sendbuf,0,sizeof(sendbuf));
		sprintf(sendbuf,"/%d.txt",i);
		iRet = fileRemove_lib(sendbuf);
		
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: fileRemove_lib iRet=%d, size:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet, fileInGetFileSysFreeSize_lib());
	}*/
}
//
void filesyserase()
{
	int iRet;
	
	scrCls_lib();	
	scrPrint_lib(1,3,0,"filesyserase begin......");
	iRet = hal_flashAllErase(0 ,g_strSpiFlashInfo.flash_size/4096);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:hal_flashAllErase iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);	
	
	scrCls_lib();
	//scrPrint_lib(0,2,0,"filesyserase OK: %d",iRet);
	if(iRet == 0)
	{
		scrPrint_lib(1,3,0,"filesyserase OK");
	}
	else
	{
		scrPrint_lib(1,3,0,"filesyserase failed");
	}
	
	hal_keypadWaitOneKey();	

}
const unsigned char filezip[] = {
		0x50, 0x4B, 0x03, 0x04, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4C, 0x04, 0x53, 0x6F, 0xE9, 
		0xA7, 0x9E, 0x0C, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x66, 0x69, 
		0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x61, 0x69, 0x20, 0x74, 0x69, 0x20, 0x77, 0x65, 0x69, 0x20, 
		0x65, 0x72, 0x50, 0x4B, 0x01, 0x02, 0x3F, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x4C, 
		0x04, 0x53, 0x6F, 0xE9, 0xA7, 0x9E, 0x0C, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x08, 0x00, 
		0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x66, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x0A, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x01, 0x00, 0x18, 0x00, 0xFA, 0xC9, 0xE2, 0xFA, 0xD0, 0x88, 0xD7, 0x01, 0xFA, 0xC9, 0xE2, 0xFA, 
		0xD0, 0x88, 0xD7, 0x01, 0x09, 0xED, 0x5A, 0xF0, 0xD0, 0x88, 0xD7, 0x01, 0x50, 0x4B, 0x05, 0x06, 
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 
		0x00, 0x00	
};
void exFileZipTest()
{
	int fd;
	int iRet;
	unsigned char data[64];

	iRet = fibo_file_delete("/ext/zipFile.zip");
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_delete iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	iRet = fibo_file_delete("/ext/zipmk/file.txt");
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_delete iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	//1.在确认已经创建了文件系统的前提下 
	//写入文件
	fd = fibo_file_open("/ext/zipFile.zip", O_RDWR|O_CREAT);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fileOpen_lib fd=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, fd);
	if(fd < 0)
	{
		fd = FILE_NOEXIST;
	}
	iRet = fibo_file_getFreeSize_ex(EXT_FS_DIR);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:before write lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	//iRet = fibo_file_getTotalSize_ex(EXT_FS_DIR);
	//sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:before write lenth=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	iRet = fibo_file_seek(fd,0,FS_SEEK_SET);
	sysLOG_lib(LOAD_LOG_LEVEL_2, "[%s] -%s- Line=%d:fibo_file_seek iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	
	iRet = fibo_file_write(fd,filezip,sizeof(filezip));
	if(iRet<0 )
	{
		sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_write iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		goto exit;
	}

	iRet = fibo_file_close(fd);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_close iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	
	iRet = fibo_file_unzip("/ext/zipFile.zip","/ext/zipmk/");
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_unzip iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	if(iRet != 0)
	{
		goto exit;
	}

	fd = fibo_file_open("/ext/file.txt", O_RDWR);
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fileOpen_lib fd=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, fd);
	if(fd < 0)
	{
		fd = FILE_NOEXIST;
	}
	memset(data,0,sizeof(data));
	iRet = fibo_file_read(fd,data,sizeof(data));
	
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_read iRet=%d filecontent:%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet,data);
	if(iRet<0 )
	{	
		goto exit;
	}
	iRet = fibo_file_close(fd);

	iRet = fibo_file_delete("/ext/zipmk/file.txt");
	sysLOG_lib(LOAD_LOG_LEVEL_1, "[%s] -%s- Line=%d:fibo_file_delete iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);

	return 0;
	
	exit:
		return -1;
		

}
//const char datatmp[180*1024]={0x31};
void MaxfileWrite()
{
	int fd;
	int iRet;
	char filename[32];
	int i;
	unsigned char  datatowrite[1024];
	scrCls_lib();
	scrPrint_lib(0, 1, 2, "press to writefile");

	//memcpy(datatowrite,datatmp,1024);
	
	memset(filename,0,sizeof(filename));
	strcpy(filename,"/interFstest.bin");
	iRet = fileRemove_lib(filename);
	sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:fileRemove_lib iRet= %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	fd = fileOpen_lib(filename,O_CREAT|O_RDWR);
	if(fd < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> open File:%s Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, filename,iRet);
		return FALSE;
	}
	iRet = fileSeek_lib(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Seek File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}

	for(i=0;i<950;i++)
	{
		memset(datatowrite,i%256,sizeof(datatowrite));
		iRet = fileWrite_lib(fd, datatowrite, 1024);
		if(iRet != (1024))
		{
			sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Write File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 2, 2, "write:%s fail",filename);
			hal_keypadWaitOneKey();
			iRet = fileClose_lib(fd);
			iRet = fileRemove_lib(filename);
			sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:fileRemove_lib iRet= %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 5, 2, "delete:%s iret:%d",filename,iRet);		
			return FALSE;
		}
		scrPrint_lib(0, 2, 2, "write data size:%d",i*1024);
		sysDelayMs(20);
	}
	iRet = fileClose_lib(fd);
	if(iRet != 0)
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Close File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	scrPrint_lib(0, 5, 2, "write:%s succ",filename);	
	hal_keypadWaitOneKey();
	iRet = fileRemove_lib(filename);
	sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:fileRemove_lib iRet= %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	scrPrint_lib(0, 5, 2, "delete:%s iret:%d",filename,iRet);

	hal_keypadWaitOneKey();

}


void fileSysGet()
{
	int iRet;
	long fileSize = 0,total;
	
	scrCls_lib();
	scrPrint_lib(0,1,0,"fileSystemSize");
	hal_keypadWaitOneKey();	
	fileSize = fileGetFileSysFreeSize_lib();
	scrPrint_lib(0,2,0,"exFree:%d",fileSize);
	total = fibo_file_getTotalSize_ex("/ext");
	scrPrint_lib(0,3,0,"exTotal:%d",total);

	fileSize = fileInGetFileSysFreeSize_lib();
	scrPrint_lib(0,4,0,"inFree:%d",fileSize);
	total = fibo_file_getTotalSize();
	scrPrint_lib(0,5,0,"exFree:%d",total);

	hal_keypadWaitOneKey();


}
void Menu_exFileSys(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
            { "filesys", "filesys", NoFun },
            { "空间查询", "fileSize", fileSysGet},
			//{ "MaxfileWrite", "MaxfileWrite", MaxfileWrite },
            //{ "文件系统创建", "fileSysCreat", Menu_exFileSysmount },
            //{ "文件读写", "fileRW", filesys_dirtest },
            //{ "文件系统擦除", "filesyserase", filesyserase },
            //{ "删除测试文件", "delTestFile", deleteTestFile },
            //{ "zip解压测试", "zipFileTest", exFileZipTest},
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    //SetMiddleSizeFont();
    Menu_ShowStandardMenu(astMenu, iSize);
    //SetLargeSizeFont();
    return;
}



void Menu_GetTime(void)
{
	unsigned char ucKey = 0xFF;
	unsigned char aucTime[6] = {0};
    unsigned char timeAsc[16]={0};
	STMENUNODE astMenu[] = {
				{ "显示时间", "GET TIME", NoFun },
			};
	scrCls_lib();
	Menu_uiSetMenuTitle(&astMenu[0]);
	scrPrint_Ex(0,2,0x00,"当前时间:\n","Current Time\n");
	kbFlush_lib();
  	while(1)
	{  
    	sysGetTimeSE_lib(aucTime);
        timeAsc[0] = BcdToDec(&aucTime[0] , 1);
        timeAsc[1] = BcdToDec(&aucTime[1] , 1);
        timeAsc[2] = BcdToDec(&aucTime[2] , 1);
        timeAsc[3] = BcdToDec(&aucTime[3] , 1);
        timeAsc[4] = BcdToDec(&aucTime[4] , 1);
        timeAsc[5] = BcdToDec(&aucTime[5] , 1);
		scrPrint_lib(0,3,0x00,"20%02d-%02d-%02d %02d:%02d:%02d\n",
			timeAsc[0],timeAsc[1],timeAsc[2],timeAsc[3],timeAsc[4],timeAsc[5]);
	    sysDelayMs_lib(100);
	    ucKey = kbGetKey_lib();
	    if(ucKey == KEYCANCEL)
	    {
	        break;
	    }
	}
	return;
}
void Menu_SetTime(void)
{
	unsigned char ucKey = 0xFF,i=0;
	unsigned char aucTime[6] = {0};
	unsigned char str[36] = {0};
	unsigned char aucBcdTime[36] = {0},timeAsc[16]={0};
    int iRet = 0;
	STMENUNODE astMenu[] = {
				{ "设置时间", "SET TIME", NoFun },
			};
	scrCls_lib();
	Menu_uiSetMenuTitle(&astMenu[0]);
	kbFlush_lib();
	sysGetTimeSE_lib(aucTime);
	timeAsc[0] = BcdToDec(&aucTime[0] , 1);
	timeAsc[1] = BcdToDec(&aucTime[1] , 1);
	timeAsc[2] = BcdToDec(&aucTime[2] , 1);
	timeAsc[3] = BcdToDec(&aucTime[3] , 1);
	timeAsc[4] = BcdToDec(&aucTime[4] , 1);
    timeAsc[5] = BcdToDec(&aucTime[5] , 1);
	scrPrint_Ex(0,2,0x00,"当前时间:\n","Current Time\n");
	scrPrint_lib(0,4,0x00,"20%02d-%02d-%02d %02d:%02d:%02d\n",
		timeAsc[0],timeAsc[1],timeAsc[2],timeAsc[3],timeAsc[4],timeAsc[5]);
	scrPrint_lib(0,5,0x00,"YYMMDDHHMMSS:\n");
	iRet = kbGetString_lib(str,0x64, 1, 12, 120);
    aucBcdTime[0] = AsciiToHex(str+0);
    aucBcdTime[1] = AsciiToHex(str+2);
    aucBcdTime[2] = AsciiToHex(str+4);
    aucBcdTime[3] = AsciiToHex(str+6);
    aucBcdTime[4] = AsciiToHex(str+8);
    aucBcdTime[5] = AsciiToHex(str+10);
	iRet = sysSetTimeSE_lib(aucBcdTime);
	sysGetTimeSE_lib(aucTime);
	timeAsc[0] = BcdToDec(&aucTime[0] , 1);
	timeAsc[1] = BcdToDec(&aucTime[1] , 1);
	timeAsc[2] = BcdToDec(&aucTime[2] , 1);
	timeAsc[3] = BcdToDec(&aucTime[3] , 1);
	timeAsc[4] = BcdToDec(&aucTime[4] , 1);
    timeAsc[5] = BcdToDec(&aucTime[5] , 1);
	scrClrLine_lib(2,4);
	scrPrint_Ex(0,2,0x00,"当前时间:\n","Current Time\n");
	scrPrint_lib(0,4,0x00,"20%02d-%02d-%02d %02d:%02d:%02d\n",timeAsc[0],timeAsc[1],timeAsc[2],timeAsc[3],timeAsc[4],timeAsc[5]);
	while(1)
	{
	    ucKey = kbGetKey_lib();
	    if(ucKey == KEYCANCEL)
	    {
	        break;          
	    }
		sysDelayMs_lib(100);
	}
	return;
}
void Menu_SeTime(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
			{ "基础项", "BasicFunc", NoFun },
            { "获取时间", "GetTime", Menu_GetTime},
			{ "设置时间", "SetTime", Menu_SetTime},	
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;
}

void Menu_ModBEEP()
{
	uint8_t keyVal;
	unsigned char Sendbuf[256];
	kbFlush_lib();
	scrCls_lib();
	
	scrPrint_lib(0, 1, 0, "1 beep 2 send 3beepSend");
	
	portOpen_lib(11, "9600,8,n,1");

	while(1)
	{
		keyVal = kbGetKey_lib();
		if (keyVal == KEYCANCEL)
		{
			return;
		}
		if (keyVal == KEY1)
		{	
			
			scrCls_lib();
			scrPrint_lib(0, 2, 0, "beep test");
			sysBeep_lib();
			
		}
		if (keyVal == KEY2)
		{			
			scrCls_lib();
			//Menu_ChkPort();
			scrPrint_lib(0, 2, 0, "uart send 256B");
			memset(Sendbuf,'a',sizeof(Sendbuf));
			portSends_lib(11, Sendbuf, 256);	
		}
		if (keyVal == KEY3)
		{			
			scrCls_lib();
			scrPrint_lib(0, 2, 0, "buzzering...");
			sysBeepF_lib(1,10000);
			sysDelayMs(2000);	
			scrPrint_lib(0, 2, 0, "buzzer   end");
			scrPrint_lib(0, 3, 0, "uart send");
			memset(Sendbuf,'b',sizeof(Sendbuf));
			portSends_lib(11, Sendbuf, 256);
		}

	}


}


void Menu_ModUpdateVOS(void)
{
	uint8_t keyVal;
	unsigned char Sendbuf[256];
	kbFlush_lib();
	scrCls_lib();
	
	scrPrint_lib(0, 1, 0, "start test update VOS");
	scrPrint_lib(0, 2, 0, "please push KEYENTER to start");

	while(1)
	{
		keyVal = kbGetKey_lib();
		if (keyVal == KEYCANCEL)
		{
			return;
		}
		if (keyVal == KEYENTER)
		{	
			
			scrCls_lib();
			scrPrint_lib(0, 2, 0, "update VOS...");
			Test_SEload_UpdateVOS();
			scrCls_lib();
			scrPrint_lib(0, 2, 0, "update VOS done...");
		}

		sysDelayMs(100);

	}


}

void Menu_basicFunc(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
            { "基础项", "BasicFunc", NoFun },
            { "显示", "Screen", Menu_ModChkScr },
            //{ "触屏", "Touch Screen", Menu_ModChkTouScr },  
            { "指示灯", "LED",Menu_ModLed},               
            { "键盘", "Keypad", McCheckKBTest },
            //{ "键盘字符输入", "Keypad Char Input", McCheckKBStringTest },
            { "蜂鸣器", "BEEP", Menu_ModBEEP},           
            { "扫码", "Scan", Menu_ScanFunc}, 
            //{ "背光", "Backlight", Menu_ModLight},
            //{ "时钟", "Time", Menu_ModChkTick },
            //{"对比度", "Contrast", Menu_ModContrast},
			{ "GPS", "GPS", Menu_GetGPSMsg},	
			{ "段码屛", "ClientLCD", Menu_ClientLCD},	
			{ "功耗", "power waste", Menu_PowerWaste},
			//{ "外部文件", "exFileSys", Menu_exFileSys},
			//{ "dumptest", "dumptest", dumptestStack},
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    //SetMiddleSizeFont();
    Menu_ShowStandardMenu(astMenu, iSize);
    //SetLargeSizeFont();
    return;
}

void Menu_ChkIC(void)
{
	unsigned char ret = 0;
	unsigned char ucAtr[32];
	unsigned char ucKey = 0;
	unsigned char out_info[30] = {0};
	ST_APDU_REQ ApduSend;
    ST_APDU_RSP ApduRecv;
	scrCls_lib();
	scrPrint_lib(0, 1, 2, "IC card TEST");
	scrPrint_lib(0, 3, 0, "IC card Init ...");

	sysGetTermInfo_lib (out_info);
	ret = out_info[5];
	if((iccInit(0) == 0) && (ret >= 1))
	{
		sysDelayMs_lib(500);
		scrClrLine_lib(3, 5);
		scrPrint_lib(0, 3, 0, "IC card Init completed");
		sysDelayMs_lib(2000);
		scrClrLine_lib(3, 5);
		scrPrint_lib(0, 3, 0, "Please insert the card");
	}
	else{
		scrClrLine_lib(3, 5);
		scrPrint_lib(0, 3, 2, "IC card Init fail");
	}
	while(1)
	{
		if(iccGetPresent_lib(0)==0)
		{
			scrClrLine_lib(3, 5);
			scrPrint_lib(0, 3, 0, "IC card detected");
			sysDelayMs_lib(5000);
			if(iccPowerUp_lib(0,ucAtr)==0)
			{
				scrCls_lib();
				scrPrint_lib(0, 1, 2, "IC card TEST");
				scrPrint_lib(0, 3, 0, "IC card Power on completed");
				scrPrint_lib(0, 5, 0, "ATR= %02x%02x%02x%02x%02x%02x%02x",ucAtr[0],ucAtr[1],ucAtr[2],ucAtr[3],ucAtr[4],ucAtr[5],ucAtr[6]);
				sysDelayMs_lib(5000);
				memset(&ApduSend, 0, sizeof(ApduSend));
    			memset(&ApduRecv, 0, sizeof(ApduRecv));
    			memcpy(ApduSend.cmd, "\x00\xa4\x04\x00", 4 );
    			ApduSend.le = 256;
    			ApduSend.lc = 14;
    			memcpy(ApduSend.data_in, "1PAY.SYS.DDF01", 14 );
				if(iccIsoCommand_lib(0, &ApduSend, &ApduRecv) == 0)
				{
					scrCls_lib();
				     scrPrint_lib(0, 1, 2, "IC card TEST");
					scrPrint_lib(0, 3, 0, "IC card Using the normal");
					sysDelayMs_lib(2000);
				   scrPrint_lib(0, 5, 0, "recv= %02x %02x",ApduRecv.swa,ApduRecv.swb);
				}
				else
				{
					scrPrint_lib(0, 3, 0, "IC card Using fail");
				}
				sysDelayMs_lib(5000);
				scrCls_lib();
				if(iccClose_lib(0) == 0)
				{
					scrClrLine_lib(3, 7);
					scrPrint_lib(0, 3, 0, "IC card Power off");
				}
				else{
					scrClrLine_lib(3, 7);
					scrPrint_lib(0, 3, 0, "IC card Power off fail");
				}
			}
			else{
					scrClrLine_lib(3, 5);
					scrPrint_lib(0, 3, 0, "IC card Power on fail");
			}
		}
		sysDelayMs_lib(200);
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			piccLight_lib(0xff, 0);
			return;
		}		
	}
}

bool blWifiConnect = FALSE;
void MenuWifi_Scan_ConnectAp(void)
{
	int iRet = -1;
	ST_AP_LIST stAplist[30];
	int i = 0;
	int j = 0;
	unsigned char ucKey = 0xFF;
	unsigned char aucPassWord[32];
	int iLoopOut = 0;
	blWifiConnect = FALSE;
	scrPrint_lib(0, 2, 0x02, "Wifi Scan ...");
	while(1)	
	{
		scrClrLine_lib(3,7);
        iRet = wifiOpen_lib();
		if(iRet < 0)
		{
		    wifiReset_lib();
			iRet = wifiOpen_lib();
			if(iRet < 0)
			{
				scrPrint_lib(0, 7, 0x02, "Wifi open err %d", iRet);
				hal_keypadWaitOneKey();
				return ;
			}
		}
	    iRet = wifiScan_lib(stAplist, 30);
		if(iRet < 0)
		{
		    wifiReset_lib();
			wifiOpen_lib();
			iRet = wifiScan_lib(stAplist, 30);
			if(iRet < 0)
			{
				scrPrint_lib(0, 7, 0x02, "Wifi Scan err %d", iRet);
				hal_keypadWaitOneKey();
				return ;
			}
		}
		
		iLoopOut = 1;
	    while(1)
	    {
			if(iLoopOut != 1)
			{
				break;
			}
			
	        for(i = 0; i < 6; i++)
	        {
				scrPrint_lib(0, 2+i, 0x00, "%d.%-16.16s  %ddBm", i+1, stAplist[j+i].cSsid, stAplist[j+i].iRssi);
	            if((j+i+1) >= iRet)
	            {
	                break;
	            }
	        }
            iLoopOut = 2;
	        while(1)
	        {
				if(iLoopOut != 2)
				{
					break;
				}
							
	            ucKey = kbGetKey_lib();
	            
	            if(KEYENTER == ucKey)
	            {
	                if(j >= (iRet-6))
	                {
	                    j = 0;
	                }
	                else
	                {
	                    j++;
	                }
	                scrClrLine_lib(2,7);
					iLoopOut = 1;
	                break;
	            }

				if(KEYCANCEL == ucKey)
	            {		
	                return ;
	            }

				if(ucKey >= '1' && ucKey <= ('0'+ i+1))
	            {
	                scrClrLine_lib(2,7);
	                
	                scrPrint_lib(0, 2, 0x02, "%s", stAplist[j+ ucKey -'0'- 1].cSsid);

					memset(aucPassWord, 0x00, sizeof(aucPassWord));
					if(0 == memcmp(stAplist[j+ ucKey -'0'- 1].cSsid, "JW-2.4G", strlen("JW-2.4G")))
					{
		                memcpy(aucPassWord+1, "jiewen-2017", strlen("jiewen-2017"));					
					}
					else if(0 == memcmp(stAplist[j+ ucKey -'0'- 1].cSsid, "VANSTONE-RD", strlen("VANSTONE-RD")))
					{
		                memcpy(aucPassWord+1, "van12345", strlen("van12345"));					
					}
					else
					{
						scrPrint_lib(0, 3, 0x02, "Need Password?");
						while(1)
						{
							iRet = kbGetKey_lib();
							if(iRet == KEYCANCEL)
							{
								break;
							}
							
							if(iRet == KEYENTER)
							{
								break;
							}
						}

						if(iRet == KEYENTER)
						{
							scrClrLine_lib(3,4);
							scrPrint_lib(0, 3, 0x02, "Please Input Password:\n");
							//scrGotoxyLine(0, 4);
			                while(1)
			                {
			                    iRet = kbGetString_lib(aucPassWord, 0x25, 1, sizeof(aucPassWord), 300);
			                    if(0 == iRet)
			                    {
			                        break;
			                    }
			                    else if(KEYCANCEL == iRet)
			                    {
	//								api_wifiClose();
									return;
			                    }                    
			                }							
						}
					}
		
	                scrClrLine_lib(3,4);
	                scrPrint_lib(0, 4, 0x02, "Connect ...");
	                
	                iRet = wifiAPConnect_lib(stAplist[j+ ucKey -'0'- 1].cSsid, (char*)aucPassWord);
	                if(0 == iRet)
	                {
	                    scrClrLine_lib(2,7);
	                    scrPrint_lib(0, 4, 0x02, "%s Connect OK", stAplist[j+ ucKey -'0'- 1].cSsid);
						blWifiConnect = TRUE;
						hal_keypadWaitOneKey();
						return ;
	                }
					else
					{
	                    scrClrLine_lib(2,7);
	                    scrPrint_lib(0, 4, 0x02, "%s Connect Fail", stAplist[j+ ucKey -'0'- 1].cSsid);

						iLoopOut = 0;
						hal_keypadWaitOneKey();
					}
				}
			}
        }
    }
}

void MenuWifi_TCP(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1024];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	uint32 uTimeS;
	char ii = 0;
	
	memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }
/*建立TCP连接*/
	scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
	scrPrint_lib(0, 3, 0x02, "Port:3000");
	scrPrint_lib(0, 4, 0x02, "Connect...."); 	
	isockid = wifiSocketCreate_lib(0);
	sysLOG_lib(1, "[%s] -%s- <%d>:wifiSocketCreate isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
	if(isockid >= 0)
	{
		iRet = wifiTCPConnect_lib(isockid, "103.235.231.21", "3000", 8000);
		if(iRet < 0)
		{
			if(iRet == WIFI_TCP_CONNECTED)
			{
				wifiTCPClose_lib(isockid);
			}
			scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();	
			wifiSocketClose_lib(isockid);
			return ;
		}
		scrClrLine_lib(4,5);
		scrPrint_lib(0, 4, 0x02, "Connect OK"); 
	}
	else
	{
		scrClrLine_lib(4,7);
		switch(isockid)
		{
			case WIFI_NOT_OPEN_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
			break;
			
			case WIFI_SOCKETCREATE_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
			break;
			default:
				scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
				break;
		}
		
		scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
		hal_keypadWaitOneKey();
		return;	
	}

	uTimeBegin = hal_sysGetTickms();
    /*循环发送、接收数据*/
	while(1)
	{
		//ii++;
		scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
		    break;
		}
		
		//if(3 == iRet)
		{
			iSum += 1;
            sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
			iRet = wifiSend_lib(isockid,cSendBuff, 1024, 1000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
			if(iRet == 0)
			{
				iSuccT += 1;
			}
			else if(iRet == WIFI_AP_LINK_ERR)
			{
			    scrClrLine_lib(4,4);
			    scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
			    scrClrLine_lib(6,7);
				break;
			}
			else
			{
			    iErr += 1;
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
				continue;
			}
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiRecv_lib(isockid, cRecvBuff, 1024, 10000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 1024)
			{
				if(0 == memcmp(cSendBuff, cRecvBuff, 1024))
				{
					iSuccR += 1;
				}
			}
			//scrPrint_lib(0, 6, 0x00, "S:%d T:%d R:%d", iSum, iSuccT, iSuccR);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);

		}
		//sysDelayMs(1000);
	}
	uTimeEnd = hal_sysGetTickms();
    //wifiTCPClose_lib(isockid);
	wifiSocketClose_lib(isockid);
	uTimeS = (uTimeEnd-uTimeBegin) / 1000;
	//scrPrint_lib(0, 7, 0x02, "time:%d, AnyKey Exit", (uTimeEnd-uTimeBegin) / 1000);
	//scrPrint_lib(0, 7, 0x02, "Rate(b/s):%d, AnyKey Exit", (iSuccT+iSuccR)*1024 * 8 / uTimeS);
	scrPrint_lib(0, 6, 0x02, "Rate(KB/s):%d, AnyKey", (iSuccT+iSuccR) / uTimeS);
	hal_keypadWaitOneKey();
}


void MenuWifi_socketTrans(void)
{
	int iRet = -1;
	int i,times=0;
	char cSendBuff[1024];
	char cRecvBuff[1024];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	unsigned long long uTimeS;
	char senddata;
	int lenth;
	
	memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }
	while(1)
	{
		times++;
		scrPrint_lib(0, 1, 0x02, "transTimes=%d",times);
		scrClrLine_lib(2,7);
		
		//1.查询AP热点连接信息
		iRet = wifiGetLinkStatus_lib();
		if((iRet == 5) || (iRet < 0))
		{	
		
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiGetLinkStatus_lib iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		    scrPrint_lib(0, 3, 0x02, "LinkStatus=%d   ",iRet);
			hal_keypadWaitOneKey();	
			return;	
		}

		//2.进入循环      创建socket--创建tcp连接 -- 
		/*建立TCP连接*/
		scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
		scrPrint_lib(0, 3, 0x02, "Port:3000");
		scrPrint_lib(0, 4, 0x02, "Connect...."); 	
		isockid = wifiSocketCreate_lib(0);
		sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSocketCreate isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
		if(isockid >= 0)
		{
			iRet = wifiTCPConnect_lib(isockid, "103.235.231.21", "3000", 8000);
			if(iRet < 0)
			{
				if(iRet == WIFI_TCP_CONNECTED)
				{
					wifiTCPClose_lib(isockid);
				}
				scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
				scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
				hal_keypadWaitOneKey();	
				wifiSocketClose_lib(isockid);
				continue;
			}
			scrClrLine_lib(4,5);
			scrPrint_lib(0, 4, 0x02, "Connect OK"); 
		}
		else
		{
			scrClrLine_lib(4,7);
			switch(isockid)
			{
				case WIFI_NOT_OPEN_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
				break;
				
				case WIFI_SOCKETCREATE_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
				break;
				default:
					scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
					break;
			}
			
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();
			return;	
		}
		i = 0;
		uTimeBegin = hal_sysGetTickms();
	    /*循环发送、接收数据*/
		while(i<3)
		{
			scrClrLine_lib(6,7);
			i++;
			senddata = '0'+i;
			memset(cSendBuff,senddata,sizeof(cSendBuff));

			iSum += 1;
	        sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
			iRet = wifiSend_lib(isockid,cSendBuff, 1024, 1000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
			if(iRet == 0)
			{
				iSuccT += 1;
			}
			else if(iRet == WIFI_AP_LINK_ERR)
			{
			    scrClrLine_lib(4,4);
			    scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
			    scrClrLine_lib(6,7);
				break;
			}
			else
			{
			    iErr += 1;
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
				sysDelayMs(1000);
				scrPrint_lib(0, 8, 0x02, "AnyKey Exit");
				hal_keypadWaitOneKey();				
				//goto exit;
			}
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiRecv_lib(isockid, cRecvBuff, 1024, 5000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 1024)
			{
				if(0 == memcmp(cSendBuff, cRecvBuff, 1024))
				{
					iSuccR += 1;
				}
			}
			sysDelayMs(20);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
		}
exit:
		uTimeEnd = hal_sysGetTickms();
	    //wifiTCPClose_lib(isockid);
		wifiSocketClose_lib(isockid);
		uTimeS = (uTimeEnd-uTimeBegin);
		scrPrint_lib(0, 7, 0x02, "loop uTimeS(ms):%d", uTimeS);
		sysDelayMs(1000);
	}
}

void MenuWifi_socketBoundaryTestDataOne(void)
{
	int iRet = -1;
	int i=0,times=0;
	char cSendBuff[1024];
	char cRecvBuff[1025];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	char senddata;
	int lenth;
	uint32 uartcb_templen;
    uint8 headbuf[] = "Recv 1024 bytes SEND OK+IPD,0,200:";

	uint8 uartcb_tempdata[] = "Recv 1024 bytes SEND OK+IPD,0,200:33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333";
	memset(cSendBuff, '3', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	//memcpy(uartcb_tempdata, headbuf, 36);
	//memcpy(uartcb_tempdata+36, cSendBuff, sizeof(cSendBuff));
	//memset(&uartcb_tempdata[36], '3', 1024);
	//uartcb_tempdata[1060] = 0;
	scrClrLine_lib(2,7);
	uint8 uartcb_printbuf[1080];
	memset(uartcb_printbuf, 0x00, 1080);
	while(1)
	{
		times++;
		scrPrint_lib(0, 1, 0x02, "transTimes=%d",times);
		scrClrLine_lib(2,7);

		//2.进入循环      创建socket--创建tcp连接 -- 
		/*建立TCP连接*/
		scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
		scrPrint_lib(0, 3, 0x02, "Port:3000");
		scrPrint_lib(0, 4, 0x02, "Connect...."); 	
		isockid = wifiSocketCreate_lib(0);
		i=0;
		while(i<3)
		{
			scrClrLine_lib(6,7);
			i++;
			senddata = '0'+i;
			memset(cSendBuff,senddata,sizeof(cSendBuff));

			iSum += 1;
	        sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
           
		    g_stWifiUart.read_P = 1024*30 - 20;
			g_stWifiUart.write_P = 1024*30 - 20;
			g_stWifiUart.uart_buff_count = 0;
			uartcb_templen = 235;
			memcpy(&g_stWifiUart.uart_buff[g_stWifiUart.write_P], uartcb_tempdata,UARTBUFF_LEN-g_stWifiUart.write_P);
		    memcpy(&g_stWifiUart.uart_buff[0],(const void *)(uartcb_tempdata+(UARTBUFF_LEN-g_stWifiUart.write_P)),uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
			memcpy(&uartcb_printbuf[0],&g_stWifiUart.uart_buff[g_stWifiUart.write_P],UARTBUFF_LEN-g_stWifiUart.write_P);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>UARTBUFF_LEN-g_stWifiUart.write_P=%d uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P) = %d uartcb_printbuf=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__,UARTBUFF_LEN-g_stWifiUart.write_P, uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P),uartcb_printbuf);
		    memcpy(&uartcb_printbuf[UARTBUFF_LEN-g_stWifiUart.write_P],&g_stWifiUart.uart_buff[0],uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>uartcb_printbuf=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, uartcb_printbuf);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>uartcb_tempdata=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, uartcb_tempdata);
			g_stWifiUart.write_P = uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P);
            g_stWifiUart.uart_buff_count += uartcb_templen;
			
            dev_cpydata_IPD();
			
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiRecv_lib(isockid, cRecvBuff, 200, 5000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d %s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet,cRecvBuff);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 200)
			{
				if(0 == memcmp(uartcb_tempdata+34, cRecvBuff, 200))
				{
					iSuccR += 1;
				}
			}
			sysDelayMs(20);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
		}
//exit:
		uTimeEnd = hal_sysGetTickms();
	    //wifiTCPClose_lib(isockid);
		wifiSocketClose_lib(isockid);
		uTimeS = (uTimeEnd-uTimeBegin);
		scrPrint_lib(0, 7, 0x02, "loop uTimeS(ms):%d", uTimeS);
		sysDelayMs(1000);
	}
}

void MenuWifi_socketBoundaryTestDataTwo(void)
{
	int iRet = -1;
	int i=0,times=0;
	char cSendBuff[1024];
	char cRecvBuff[1025];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	char senddata;
	int lenth;
	uint32 uartcb_templen;
    uint8 headbuf[] = "Recv 1024 bytes SEND OK+IPD,0,200:";

	uint8 uartcb_tempdata[] = "Recv 1024 bytes SEND OK+IPD,0,200:33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333";
	memset(cSendBuff, '3', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	//memcpy(uartcb_tempdata, headbuf, 36);
	//memcpy(uartcb_tempdata+36, cSendBuff, sizeof(cSendBuff));
	//memset(&uartcb_tempdata[36], '3', 1024);
	//uartcb_tempdata[1060] = 0;
	scrClrLine_lib(2,7);
	uint8 uartcb_printbuf[1080];
	memset(uartcb_printbuf, 0x00, 1080);
	while(1)
	{
		times++;
		scrPrint_lib(0, 1, 0x02, "transTimes=%d",times);
		scrClrLine_lib(2,7);

		//2.进入循环      创建socket--创建tcp连接 -- 
		/*建立TCP连接*/
		scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
		scrPrint_lib(0, 3, 0x02, "Port:3000");
		scrPrint_lib(0, 4, 0x02, "Connect...."); 	
		isockid = wifiSocketCreate_lib(0);
		i=0;
		while(i<3)
		{
			scrClrLine_lib(6,7);
			i++;
			senddata = '0'+i;
			memset(cSendBuff,senddata,sizeof(cSendBuff));

			iSum += 1;
	        sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
           
		    g_stWifiUart.read_P = 1024*30 - 25;
			g_stWifiUart.write_P = 1024*30 - 25;
			g_stWifiUart.uart_buff_count = 0;
			uartcb_templen = 234;
			memcpy(&g_stWifiUart.uart_buff[g_stWifiUart.write_P], uartcb_tempdata,UARTBUFF_LEN-g_stWifiUart.write_P);
		    memcpy(&g_stWifiUart.uart_buff[0],(const void *)(uartcb_tempdata+(UARTBUFF_LEN-g_stWifiUart.write_P)),uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
			memcpy(&uartcb_printbuf[0],&g_stWifiUart.uart_buff[g_stWifiUart.write_P],UARTBUFF_LEN-g_stWifiUart.write_P);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>UARTBUFF_LEN-g_stWifiUart.write_P=%d uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P) = %d uartcb_printbuf=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__,UARTBUFF_LEN-g_stWifiUart.write_P, uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P),uartcb_printbuf);
		    memcpy(&uartcb_printbuf[UARTBUFF_LEN-g_stWifiUart.write_P],&g_stWifiUart.uart_buff[0],uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>uartcb_printbuf=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, uartcb_printbuf);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>uartcb_tempdata=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, uartcb_tempdata);
			g_stWifiUart.write_P = uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P);
            g_stWifiUart.uart_buff_count += uartcb_templen;
			
            dev_cpydata_IPD();
			
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiRecv_lib(isockid, cRecvBuff, 200, 5000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d %s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet,cRecvBuff);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 200)
			{
				if(0 == memcmp(uartcb_tempdata+34, cRecvBuff, 200))
				{
					iSuccR += 1;
				}
			}
			sysDelayMs(20);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
		}
//exit:
		uTimeEnd = hal_sysGetTickms();
	    //wifiTCPClose_lib(isockid);
		wifiSocketClose_lib(isockid);
		uTimeS = (uTimeEnd-uTimeBegin);
		scrPrint_lib(0, 7, 0x02, "loop uTimeS(ms):%d", uTimeS);
		sysDelayMs(1000);
	}
}

void MenuWifi_socketBoundaryTestDataThr(void)
{
	int iRet = -1;
	int i=0,times=0;
	char cSendBuff[1024];
	char cRecvBuff[1025];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	char senddata;
	int lenth;
	uint32 uartcb_templen;
    uint8 headbuf[] = "Recv 1024 bytes SEND OK+IPD,0,200:";

	uint8 uartcb_tempdata[] = "Recv 1024 bytes SEND OK+IPD,0,200:33333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333";
	memset(cSendBuff, '3', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	//memcpy(uartcb_tempdata, headbuf, 36);
	//memcpy(uartcb_tempdata+36, cSendBuff, sizeof(cSendBuff));
	//memset(&uartcb_tempdata[36], '3', 1024);
	//uartcb_tempdata[1060] = 0;
	scrClrLine_lib(2,7);
	uint8 uartcb_printbuf[1080];
	memset(uartcb_printbuf, 0x00, 1080);
	while(1)
	{
		times++;
		scrPrint_lib(0, 1, 0x02, "transTimes=%d",times);
		scrClrLine_lib(2,7);

		//2.进入循环      创建socket--创建tcp连接 -- 
		/*建立TCP连接*/
		scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
		scrPrint_lib(0, 3, 0x02, "Port:3000");
		scrPrint_lib(0, 4, 0x02, "Connect...."); 	
		isockid = wifiSocketCreate_lib(0);
		i=0;
		while(i<3)
		{
			scrClrLine_lib(6,7);
			i++;
			senddata = '0'+i;
			memset(cSendBuff,senddata,sizeof(cSendBuff));

			iSum += 1;
	        sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
           
		    g_stWifiUart.read_P = 1024*30 - 40;
			g_stWifiUart.write_P = 1024*30 - 40;
			g_stWifiUart.uart_buff_count = 0;
			uartcb_templen = 234;
			memcpy(&g_stWifiUart.uart_buff[g_stWifiUart.write_P], uartcb_tempdata,UARTBUFF_LEN-g_stWifiUart.write_P);
		    memcpy(&g_stWifiUart.uart_buff[0],(const void *)(uartcb_tempdata+(UARTBUFF_LEN-g_stWifiUart.write_P)),uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
			memcpy(&uartcb_printbuf[0],&g_stWifiUart.uart_buff[g_stWifiUart.write_P],UARTBUFF_LEN-g_stWifiUart.write_P);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>UARTBUFF_LEN-g_stWifiUart.write_P=%d uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P) = %d uartcb_printbuf=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__,UARTBUFF_LEN-g_stWifiUart.write_P, uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P),uartcb_printbuf);
		    memcpy(&uartcb_printbuf[UARTBUFF_LEN-g_stWifiUart.write_P],&g_stWifiUart.uart_buff[0],uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>uartcb_printbuf=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, uartcb_printbuf);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>uartcb_tempdata=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, uartcb_tempdata);
			g_stWifiUart.write_P = uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P);
            g_stWifiUart.uart_buff_count += uartcb_templen;
			
            dev_cpydata_IPD();
			
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiRecv_lib(isockid, cRecvBuff, 200, 5000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d %s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet,cRecvBuff);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 200)
			{
				if(0 == memcmp(uartcb_tempdata+34, cRecvBuff, 200))
				{
					iSuccR += 1;
				}
			}
			sysDelayMs(20);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
		}
//exit:
		uTimeEnd = hal_sysGetTickms();
	    //wifiTCPClose_lib(isockid);
		wifiSocketClose_lib(isockid);
		uTimeS = (uTimeEnd-uTimeBegin);
		scrPrint_lib(0, 7, 0x02, "loop uTimeS(ms):%d", uTimeS);
		sysDelayMs(1000);
	}
}

void MenuWifi_scanAp(void)
{
	int iRet = -1;
	int i;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 succ;
	uint32 err;
	uint32 uTimeS;
	ST_AP_LIST stApList[32];

	scrClrLine_lib(2,7);
    if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }
	uTimeS = 0;
	succ=0;
	err=0;
	while(1)
	{
		//1.选择开启搜索热点
		scrClrLine_lib(2,7);
		scrPrint_lib(0, 1, 0x02, "scanAp uTimeS:%d",uTimeS);
		uTimeS++;
		iRet = wifiScan_lib(stApList, 32);
		if(iRet < 0)
		{
			scrPrint_lib(0, 2, 0x03, "scanErr:%d",iRet);
			kbFlush_lib();
			while(0xFF == kbGetKey_lib())
			{
				sysDelayMs(1000);
			}
			err++;
			//continue;
		}
		
		succ++;
		//2.显示
		for(i=0;i<5;i++)
		{
			scrPrint_lib(0, 2+i, 0x00, "%d.:%s",i,stApList[i].cSsid);
		}
		sysDelayMs(10);
		//4.wait stop
		if(0xFF != kbGetKey_lib())
		{
			while(1)
			{
				kbFlush_lib();
				while(0xFF == kbGetKey_lib())
				{
					uTimeS = 0;
					sysDelayMs(1000);
				}
			}
		}

	}

}



void MenuWifi_ssl(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1024];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	unsigned long long uTimeS;
	
	memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }
/*建立TCP连接*/
	scrPrint_lib(0, 2, 0x02, "wifiSSLSocketCreate_lib");
	scrPrint_lib(0, 3, 0x02, "Port:443");
	scrPrint_lib(0, 4, 0x02, "Connect....");	
	
	isockid = wifiSSLSocketCreate_lib();
	sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSSLSocketCreate_lib isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
	if(isockid >= 0)
	{
		iRet = wifiSSLConnect_lib(isockid, "gw.open.icbc.com.cn", "443", 8000);
		if(iRet < 0)
		{
			if(iRet == WIFI_TCP_CONNECTED)
			{
				wifiTCPClose_lib(isockid);
			}
			scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();	
			wifiSocketClose_lib(isockid);
			return ;
		}
		scrClrLine_lib(4,5);
		scrPrint_lib(0, 4, 0x02, "Connect OK"); 
	}
	else
	{
		scrClrLine_lib(4,7);
		switch(isockid)
		{
			case WIFI_NOT_OPEN_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
			break;
			
			case WIFI_SOCKETCREATE_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
			break;
			default:
				scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
				break;
		}
		
		scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
		hal_keypadWaitOneKey();
		return;	
	}


	scrClrLine_lib(5,6);
	scrPrint_lib(0, 5, 0x02, "anykey senddata"); 
	hal_keypadWaitOneKey();
	scrClrLine_lib(5,6);
	
	uTimeBegin = hal_sysGetTickms();
    /*循环发送、接收数据*/
	while(1)
	{
		scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
		    break;
		}
		
		//if(3 == iRet)
		{
			iSum += 1;
            sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
			iRet = wifiSSLSend_lib(isockid,cSendBuff, 1024, 1000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
			if(iRet == 0)
			{
				iSuccT += 1;
			}
			else if(iRet == WIFI_AP_LINK_ERR)
			{
			    scrClrLine_lib(4,4);
			    scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
			    scrClrLine_lib(6,7);
				break;
			}
			else
			{
			    iErr += 1;
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
				continue;
			}
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiSSLRecv_lib(isockid, cRecvBuff, 1024, 10000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 1024)
			{
				if(0 == memcmp(cSendBuff, cRecvBuff, 1024))
				{
					iSuccR += 1;
				}
			}
			//scrPrint_lib(0, 6, 0x00, "S:%d T:%d R:%d", iSum, iSuccT, iSuccR);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);

		}

	}
	uTimeEnd = hal_sysGetTickms();
    //wifiSSLClose_lib(isockid);
	wifiSSLSocketClose_lib(isockid);
	uTimeS = (uTimeEnd-uTimeBegin) / 1000;
	//scrPrint_lib(0, 7, 0x02, "time:%d, AnyKey Exit", (uTimeEnd-uTimeBegin) / 1000);
	//scrPrint_lib(0, 7, 0x02, "Rate(b/s):%d, AnyKey Exit", (iSuccT+iSuccR)*1024 * 8 / uTimeS);
	scrPrint_lib(0, 6, 0x02, "Rate(KB/s):%d, AnyKey Exit", (iSuccT+iSuccR) / uTimeS);
	hal_keypadWaitOneKey();
}

void Menu_ModChkCom4G(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1024];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	int recvdlen = 0;
	unsigned long long uTimeBegin;
	unsigned long long uTimeEnd;
	unsigned long long uTimeS;
	
	memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
	scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
	scrPrint_lib(0, 3, 0x02, "Port:3000");
	scrPrint_lib(0, 4, 0x02, "Connect...."); 
	iRet = wirelessPppOpen_lib(NULL, NULL, NULL);
	if(iRet != 0)
	{
		sysLOG_lib(SOCKET_LOG_LEVEL_1, "[%s] -%s- Line=%d:<ERR> wirelessPppOpen_lib, iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		scrPrint_lib(0, 7, 0x02, "wirelessPppOpen err");
		hal_keypadWaitOneKey();
		return;	
	}
	sysDelayMs(100);

	/*******CHECK********/ 
	iRet =	wirelessCheckPppDial_lib();
	if(iRet != 0)
	{
	
		sysLOG_lib(SOCKET_LOG_LEVEL_1, "[%s] -%s- Line=%d:<ERR>  wirelessCheckPppDial_lib, iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		scrPrint_lib(0, 6, 0x02, "wirelessCheckPppDial err");
		hal_keypadWaitOneKey();
		wirelessPppClose_lib();
		return;	
	}

/*建立TCP连接*/
	isockid = wirelessSocketCreate_lib(0);
	sysLOG_lib(SOCKET_LOG_LEVEL_3, "[%s] -%s- <%d>:wirelessSocketCreate isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
	if(isockid > 0)
	{
		iRet = wirelessTcpConnect_lib(isockid, "103.235.231.21", "3000", 1000);
		if(iRet < 0)
		{
			scrPrint_lib(0, 5, 0x00, "wireless Connect Err: %d", iRet);
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();	
			wirelessSocketClose_lib(isockid);
			wirelessPppClose_lib();
			return ;
		}
		scrClrLine_lib(4,5);
		scrPrint_lib(0, 4, 0x02, "Connect OK"); 
	}
	else
	{
		scrClrLine_lib(3,7);
		switch(isockid)
		{
			case WIFI_NOT_OPEN_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
			break;
			
			case WIFI_SOCKETCREATE_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
			break;
			default:
				scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
				break;
		}
		
		scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
		hal_keypadWaitOneKey();
		wirelessPppClose_lib();
		return;	
	}

    uTimeBegin = hal_sysGetTickms();
    /*循环发送、接收数据*/
	while(1)
	{
		iSum += 1;
        sysLOG_lib(SOCKET_LOG_LEVEL_3, "[%s] -%s- <%d>:wirelessSend_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		iRet = wirelessSend_lib(isockid,cSendBuff, 1024);
		sysLOG_lib(SOCKET_LOG_LEVEL_3, "[%s] -%s- <%d>:wirelessSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		scrPrint_lib(0, 6, 0x00, "wirelessSend iRet:%d", iRet);
		if(iRet == 1024)
		{
			iSuccT += 1;
		}
		else
		{
			scrPrint_lib(0, 7, 0x02, "AnyKey Exit");
		    hal_keypadWaitOneKey();
			break;
		}
		
		memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
		recvdlen = 0;
		sysLOG_lib(SOCKET_LOG_LEVEL_3, "[%s] -%s- <%d>:wirelessRecv begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		do{
			iRet = wirelessRecv_lib(isockid, cRecvBuff, 1024, 1000);
			recvdlen += iRet;
			sysLOG_lib(SOCKET_LOG_LEVEL_3, "[%s] -%s- Line=%d:<SUCC> wirelessRecv, iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			sysDelayMs(10);
		}while(recvdlen < 1024);
		sysLOG_lib(SOCKET_LOG_LEVEL_3, "[%s] -%s- <%d>:wirelessRecv end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		scrPrint_lib(0, 7, 0x00, "wirelessRecv iRet:%d", iRet);
		if(recvdlen == 1024)
		{
			if(0 == memcmp(cSendBuff, cRecvBuff, 1024))
			{
				iSuccR += 1;
			}
		}
		//scrPrint_lib(0, 6, 0x00, "S:%d T:%d R:%d", iSum, iSuccT, iSuccR);
		scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);

        scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
			break;
		}
	}
	uTimeEnd = hal_sysGetTickms();
	wirelessSocketClose_lib(isockid);
	wirelessPppClose_lib();
	uTimeS = (uTimeEnd-uTimeBegin) / 1000;
	//scrPrint_lib(0, 7, 0x02, "time:%d, AnyKey Exit", (uTimeEnd-uTimeBegin) / 1000);
	//scrPrint_lib(0, 7, 0x02, "Rate(b/s):%d, AnyKey Exit", (iSuccT+iSuccR)*1024 * 8 / uTimeS);
	scrPrint_lib(0, 6, 0x02, "Rate(KB/s):%d, AnyKey Exit", (iSuccT+iSuccR) / uTimeS);
	hal_keypadWaitOneKey();
}

void MenuWifi_GetMAC(void)
{
	int iRet = 0;
	unsigned char ucKey = 0;
	char GetMacBuf[50];
	scrCls_lib();
	kbFlush_lib();

	scrPrint_lib(0, 1, 2, "Get MAC");
	memset(GetMacBuf, 0, sizeof(GetMacBuf));
	iRet = wifiGetMac_lib(GetMacBuf);

	if(iRet == 0)
	{
		scrClrLine_lib(3,4);
		scrPrint_lib(2, 3, 0x00, "MAC: %s \r\n", GetMacBuf);

		while(1)
		{

			if(kbHit_lib()==0)                           //按任意键进入
			{
				ucKey = kbGetKey_lib();

				if(ucKey != 0xff)                   //取消键退出
				{	
					return;
				}
			}
		}

	}
	else{
		scrClrLine_lib(3,4);
		scrPrint_lib(0, 3, 2, "Get MAC fail");

		while(1)
		{

			if(kbHit_lib()==0)                           //按任意键进入
			{
				ucKey = kbGetKey_lib();

				if(ucKey != 0xff)                   //取消键退出
				{	
					return;
				}

			}

		}

	}

}



void MenuWifi_SetMAC(void)
{
	int iRet = 0;
	unsigned char ucKey = 0;
	char GetMacBuf[50];
	char SetMacBuf[50];
	char MacAddress[50] = {0};//"48:3f:da:ab:f9:66";
	char MacAddressCounter = 0;
	char charModeFlag = 0;
	char charRptKey2Flag = 0;
	char charRptKey3Flag = 0;
	char charKey2number = 0;
	char charKey3number = 0;
    unsigned long long time2_last = 0;
	unsigned long long time2_this = 0;	
	unsigned long long time3_last = 0;
	unsigned long long time3_this = 0;

	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "Set MAC");

	while(1)
	{
		scrClrLine_lib(3,4);
		scrClrLine_lib(4,5);
		scrClrLine_lib(5,6);
		//scrClrLine_lib(6,7);
		scrPrint_lib(0, 2, 2, "please input MAC");
		scrPrint_lib(2, 4, 0x00, "  MAC=");
		scrPrint_Ex(0,7,0x02,"功能键切换输入模式\n","FunKeyToggleInputMode\n");

		MacAddressCounter = 0;
		charModeFlag = 0;
		memset(MacAddress, 0x00, 50);

		charRptKey2Flag = 0;
		time2_this = 0;
		charKey2number = 0;
		charRptKey3Flag = 0;
		time3_this = 0;
		charKey3number = 0;

		while(1)
		{

			if(MacAddressCounter == 16)
			{

				if(charRptKey2Flag == 1)
				{

					time2_this = hal_sysGetTickms();

					if((time2_this - time2_last)>=1000)
					{
						break;
					}
				}

				if(charRptKey3Flag == 1)
				{

					time3_this = hal_sysGetTickms();
					if((time3_this - time3_last)>=1000)
					{
						break;
					}

				}

			}

			if(kbHit_lib()==0)                           //按任意键进入
			{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)                   //取消键退出
					{	
						return;
					}

					if((ucKey != KEY2)&&(charRptKey2Flag == 1))
					{

						scrClrLine_lib(4,5);
						scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
						MacAddressCounter++;

						if(MacAddressCounter >= 17)
						{
							break;
						}

						if(((MacAddressCounter+1))%3 == 0)
						{
							MacAddress[MacAddressCounter]= 0x3a;
							scrClrLine_lib(4,5);
							scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
							MacAddressCounter++;

							if(MacAddressCounter >= 17)
							{
								break;
							}
						}
						charRptKey2Flag = 0;
					}

					if(((ucKey != KEY3)&&(charRptKey3Flag == 1)))
					{
						scrClrLine_lib(4,5);
						scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
						MacAddressCounter++;

						if(MacAddressCounter >= 17)
						{
							break;
						}

						if(((MacAddressCounter+1))%3 == 0)
						{

							MacAddress[MacAddressCounter]= 0x3a;
							scrClrLine_lib(4,5);
							scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
							MacAddressCounter++;
							if(MacAddressCounter >= 17)
							{
								break;
							}
						}

						charRptKey3Flag = 0;
					}

					//输入模式切换
					if(ucKey == KEY_FN)
					{

						if(charModeFlag == 0)
						{
							charModeFlag = 1;
						}
						else{

							charModeFlag = 0;
						}
					}

					//数字输入模式
					if((charModeFlag == 0) && (0x30 <= ucKey) && (ucKey <= 0x39))
					{	
						MacAddress[MacAddressCounter] = ucKey;
						MacAddressCounter++;
						scrClrLine_lib(4,5);
						scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
						//scrPrint_lib(2, 6, 0x00, "Counter1:%d \r\n", MacAddressCounter);
						if(MacAddressCounter >= 17)
						{
							break;
						}

						if(((MacAddressCounter+1))%3 == 0)
						{
							MacAddress[MacAddressCounter]= 0x3a;
							MacAddressCounter++;
							scrClrLine_lib(4,5);
							scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
							//scrPrint_lib(2, 6, 0x00, "Counter2:%d \r\n", MacAddressCounter);

							if(MacAddressCounter >= 17)
							{
								break;
							}

						}

					}

				//字符输入模式

				if(charModeFlag == 1)
				{

					if(ucKey==KEY2)
					{

						time2_this = hal_sysGetTickms();

						if(charRptKey2Flag == 1)
						{

								if((time2_this - time2_last) < 1000)
								{
									charKey2number++;

									if(charKey2number == 3)
									{
										charKey2number = 0;
									}

									switch(charKey2number)
									{

										case 0:
											MacAddress[MacAddressCounter] = 'c';
											scrClrLine_lib(4,5);
											scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
											break;

										case 1:
											MacAddress[MacAddressCounter] = 'a';
											scrClrLine_lib(4,5);
											scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
											break;

										case 2:
											MacAddress[MacAddressCounter] = 'b';
											scrClrLine_lib(4,5);
											scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
											break;

										default: break;
									}

								}
								else{
									scrClrLine_lib(4,5);
									scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
									MacAddressCounter++;

									if(MacAddressCounter >= 17)
									{
										break;
									}

									if(((MacAddressCounter+1))%3 == 0)
									{
										MacAddress[MacAddressCounter]= 0x3a;
										scrClrLine_lib(4,5);
										scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
										MacAddressCounter++;

										if(MacAddressCounter >= 17)
										{
											break;
										}
									}
									MacAddress[MacAddressCounter] = 'a';
									scrClrLine_lib(4,5);
									scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
									charKey2number=1;
									time2_last = 0;
									charRptKey2Flag = 0;
								}
						}
						else{

								MacAddress[MacAddressCounter] = 'a';
								scrClrLine_lib(4,5);
								scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
								charKey2number=1;
								time2_last = 0;
								charRptKey2Flag = 1;
						}
						time2_last = time2_this;
					}
					else{

						charKey2number=0;
						time2_last = 0;
						charRptKey2Flag = 0;
					}

					if(ucKey==KEY3)
					{
						time3_this = hal_sysGetTickms();
						if(charRptKey3Flag == 1)
						{
								if((time3_this - time3_last) < 1000)
								{
									charKey3number++;
									if(charKey3number == 3)
									{
										charKey3number = 0;
									}

									switch(charKey3number)
									{
										case 0:
											MacAddress[MacAddressCounter] = 'f';
											scrClrLine_lib(4,5);
											scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
											break;

										case 1:
											MacAddress[MacAddressCounter] = 'd';
											scrClrLine_lib(4,5);
											scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
											break;

										case 2:
											MacAddress[MacAddressCounter] = 'e';
											scrClrLine_lib(4,5);
											scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
											break;
										default: break;
									}
								}
								else{
									scrClrLine_lib(4,5);
									scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
									MacAddressCounter++;
									if(MacAddressCounter >= 17)
									{
										break;
									}
									if(((MacAddressCounter+1))%3 == 0)
									{
										MacAddress[MacAddressCounter]= 0x3a;
										scrClrLine_lib(4,5);
										scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
										MacAddressCounter++;
										if(MacAddressCounter >= 17)
										{
											break;
										}
									}
									MacAddress[MacAddressCounter] = 'd';
									scrClrLine_lib(4,5);
									scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
									charKey3number=1;
									time3_last = 0;
									charRptKey3Flag = 0;
								}
						}
						else{
								MacAddress[MacAddressCounter] = 'd';
								scrClrLine_lib(4,5);
								scrPrint_lib(2, 4, 0x00, "  MAC= %s \r\n", MacAddress);
								charKey3number=1;
								time3_last = 0;
								charRptKey3Flag = 1;
						}
						time3_last = time3_this;

					}
					else{
						charKey3number=0;
						time3_last = 0;
						charRptKey3Flag = 0;
					}
				}
			}
		}

		scrClrLine_lib(2,3);
		scrClrLine_lib(4,5);
		scrPrint_lib(2, 4, 0x00, "SetMAC=%s \r\n", MacAddress);
		//scrClrLine_lib(5,6);
		//scrClrLine_lib(6,7);
				/*scrPrint_lib(2, 5, 0x00, "InMAC=%s \r\n", MacAddress);
				while(1)
				{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();

						if(ucKey == KEYCANCEL)                   //取消键退出
						{	
							return;
						}
						else if(ucKey != 0xff)
						{
							break;
						}
					}
				}*/

		iRet = wifiSetMac_lib(MacAddress);
		if(iRet != 0)
		{
			scrClrLine_lib(2,3);
			scrClrLine_lib(3,4);
			scrClrLine_lib(4,5);
			scrPrint_lib(0, 3, 2, "SetMacFail %d\r\n",iRet);

			while(1)
			{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();

					if(ucKey == KEYCANCEL)                   //取消键退出
					{	
						return;
					}
					else if(ucKey != 0xff)
					{
						break;
					}
				}
			}
		}
			memset(GetMacBuf, 0, sizeof(GetMacBuf));
			iRet = wifiGetMac_lib(GetMacBuf);

			if(iRet == 0)
			{
				scrClrLine_lib(5,6);
				scrPrint_lib(2, 5, 0x00, "GetMAC=%s \r\n", GetMacBuf);
			}
			else{
				scrClrLine_lib(5,6);
				scrPrint_lib(2, 5, 0x00, "GetMACFail");
			}

			while(1)
			{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)                   //取消键退出
					{	
						return;
					}
					else if(ucKey != 0xff)
					{
						break;
					}
				}
			}
	}
}
void wifi_deinituart()
{
//	fibo_gpio_mode_set(WIFI_TXD_GPIO, GpioFunction0);
//	fibo_gpio_mode_set(WIFI_RXD_GPIO, GpioFunction0);
//
//	fibo_gpio_cfg(WIFI_TXD_GPIO, GpioCfgIn);
//	fibo_gpio_cfg(WIFI_RXD_GPIO, GpioCfgIn);

	scrCls_lib();
	scrPrint_lib(0, 2, 2, "deinitwifi");
	
	hal_keypadWaitOneKey();	

}

#if __MENU_DEBUG

extern const unsigned char client_ca[0x588];
extern const unsigned char client_cert[0x1208];
extern const unsigned char client_key[0x698];

void Menu_DownloadCertificate(void)
{
	char ucKey = 0;
	char dataBuf[4096];
	char connectMode[4] = "SSL\0";
	char connectAddress[100] = "P0U4OGQLRM.wx-mqtt2.tencentdevices.com\0";
	char portBuf[20];
	int port = 8883;
	char psk[]="D1883DF73206224F9692AD768F02E7A1\0";
	char psk_id[]={ "P0U4OGQLRMTOS_TEST_TEST00000046\0"};
	//char psk[100]; 
	int i = 0;
	int iRet = 0; 
	int isockid = 0;
	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "Download Certificate");
	scrClrLine_lib(4,5);
	memset(portBuf, 0x00, sizeof(portBuf));
	//memset(psk, 0x00, 100);
	//memcpy(psk, psk_key, 17);
	sprintf(portBuf,"%d",port);
	if(g_cWifiVersionFlag == 1)
	{
		scrPrint_lib(0, 4, 0, "1.CA crt     2.client crt");
		scrPrint_lib(0, 4, 0, "3.client key");
		while(1)
		{
			if(kbHit_lib()==0)                           //按任意键进入
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)
				{
					kbFlush_lib();
					return;
				}
				else if(ucKey == KEY1)
				{
					scrClrLine_lib(4,5);
					scrPrint_lib(0, 4, 2, "CA cert download");
					scrClrLine_lib(6,6);
					memset(dataBuf,0x55,4096);
					
					iRet = WifiespDownloadCrt_lib(0, client_ca, sizeof(client_ca));
					if(iRet < 0)
					{
                        scrPrint_lib(0, 6, 2, "CA cert download fail");
                    }
					else{
						scrPrint_lib(0,6,2,"CA cert download OK");
					}
					goto exit;
                }
				else if(ucKey == KEY2)
				{
					scrClrLine_lib(4,5);
					scrPrint_lib(0, 4, 2, "Client cert download");
					scrClrLine_lib(6,6);
					memset(dataBuf,0x55,4096);
					
					iRet = WifiespDownloadCrt_lib(1, client_cert, sizeof(client_cert));
					if(iRet < 0)
					{
                        scrPrint_lib(0, 6, 2, "Client cert download fail");
                    }
					else{
						scrPrint_lib(0,6,2,"Client cert download OK");
					}
					goto exit;
				}
				else if(ucKey == KEY3)
				{
					scrClrLine_lib(4,5);
					scrPrint_lib(0, 4, 2, "Client key download");
					scrClrLine_lib(6,6);
					memset(dataBuf,0x55,4096);
					
					iRet = WifiespDownloadCrt_lib(2, client_key, sizeof(client_key));
					if(iRet < 0)
					{
                        scrPrint_lib(0, 6, 2, "Client key download fail");
                    }
					else{
						scrPrint_lib(0,6,2,"Client key download OK");
					}
					goto exit;
				}
			}
			sysDelayMs(50);
		}
	}
	else{
		while(1)
		{
			if(kbHit_lib()==0)                           //按任意键进入
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)
				{
					kbFlush_lib();
					return;
				}
				/*else if(ucKey == KEY1)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 2, "ca_cert");
					scrClrLine_lib(6,6);
					memset(dataBuf,0x55,4096);
					for(i=0; i<4096; i++)
					{
						dataBuf[i]=(i%5 + '0');
					}
					//iRet = hal_espDownloadCertificate(0, acesp_ca_cert, 0x57c);
					if(iRet < 0)
					{
						scrPrint_lib(0,6,2,"Download fail");
					}
					scrPrint_lib(0,6,2,"Download ca_cert OK");
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 2, "1.ca_cert 2.cert_private_key");
				}
				else if(ucKey == KEY2)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 2, "cert_private_key");
					scrClrLine_lib(6,6);
					memset(dataBuf,0x55,4096);
					for(i=0; i<4096; i++)
					{
						dataBuf[i]=(i%5 + '0');
					}
					//iRet = hal_espDownloadCertificate(0, dataBuf, 4096);
					if(iRet < 0)
					{
						scrPrint_lib(0,6,2,"Download fail");
					}
					scrPrint_lib(0,6,2,"Download cert_private_key OK");
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 2, "1.ca_cert 2.cert_private_key");
				}*/
				/*else if(ucKey == KEY3)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 2, "connect to Server");
					scrClrLine_lib(6,6);
					isockid = wifiSSLSocketCreate_lib();
					iRet = hal_espDownloadPSK(isockid, psk_id, psk);
					if(iRet < 0)
					{
						scrPrint_lib(0, 6, 2, "send PSK Fail");
					}
					
					iRet = wifiCommConnect_lib(isockid, connectMode, connectAddress, portBuf, 5000);
					if(iRet < 0)
					{
						scrPrint_lib(0, 6, 2, "connect to Server Fail");
					}
					else{
						scrPrint_lib(0, 6, 2, "connect to Server OK");
					}
				}*/
			}
			sysDelayMs(50);
		}
	}
	
exit:
    while (1)
    {
        if (kbHit_lib() == 0) //按任意键进入
        {
            ucKey = kbGetKey_lib();
            if (ucKey == KEYCANCEL)
            {
                kbFlush_lib();
                return;
            }
        }
        sysDelayMs(50);
    }
}

void MenuWifi_sslCrt(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1024];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	int i =0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	//char sendData[] = "105B00044D5154540482025800344D73776970652D536F756E64626F782D37353766396661352D383563332D343438312D393837332D38306466333035396334323300193F53444B3D507974686F6E2656657273696F6E3D312E342E39\0";
	char sendData[7]={1,2,3,4,5,6,0};
	memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }
/*建立TCP连接*/
	scrPrint_lib(0, 2, 0x02, "wifiSSLSocketCreate_lib");
	scrPrint_lib(0, 3, 0x02, "Port:443");
	scrPrint_lib(0, 4, 0x02, "Connect....");
	hal_espQuerySSLConfig(); 	
	
	isockid = wifiSSLSocketCreate_lib();
	sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSSLSocketCreate_lib isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
	if(isockid >= 0)
	{
		scrClrLine_lib(4,4);
		for(i=0; i<5; i++)
		{
			WifiespSetSSLConfig_lib(i, 3);
			iRet = WifiespQuerySSLConfig_lib(i);
			scrPrint_lib(0, 4, 0x02, "iRet=%d",iRet);
			sysDelayMs(2000);
		}
		scrClrLine_lib(4,4);
		iRet = wifiSSLConnect_lib(isockid, "192.168.1.106", "8739", 8000);
		if(iRet < 0)
		{
			if(iRet == WIFI_TCP_CONNECTED)
			{
				wifiTCPClose_lib(isockid);
			}
			scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();	
			wifiSocketClose_lib(isockid);
			return ;
		}
		scrClrLine_lib(4,5);
		scrPrint_lib(0, 4, 0x02, "Connect OK"); 
	}
	else
	{
		scrClrLine_lib(4,7);
		switch(isockid)
		{
			case WIFI_NOT_OPEN_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
			break;
			
			case WIFI_SOCKETCREATE_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
			break;
			default:
				scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
				break;
		}
		
		scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
		hal_keypadWaitOneKey();
		return;	
	}
	scrClrLine_lib(5,6);

    /*循环发送、接收数据*/
	while(1)
	{
		scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
		    break;
		}
		
		{
			iSum += 1;
            sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum);
			iRet = wifiSSLSend_lib(isockid,sendData, sizeof(sendData), 1000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
			if(iRet == 0)
			{
				iSuccT += 1;
			}
			else if(iRet == WIFI_AP_LINK_ERR)
			{
			    scrClrLine_lib(4,4);
			    scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
			    scrClrLine_lib(6,7);
				break;
			}
			else
			{
			    iErr += 1;
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
				continue;
			}
			
			/*memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib begin\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = wifiSSLRecv_lib(isockid, cRecvBuff, 1024, 10000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 1024)
			{
				if(0 == memcmp(cSendBuff, cRecvBuff, 1024))
				{
					iSuccR += 1;
				}
			}*/
			sysDelayMs(1000);
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
		}

	}
	for(i=0; i<5; i++)
    {
        WifiespSetSSLConfig_lib(i, 0);
        iRet = WifiespQuerySSLConfig_lib(i);
        //scrPrint_lib(0, 4, 0x02, "iRet=%d", iRet);
    }
    wifiSSLSocketClose_lib(isockid);
	hal_keypadWaitOneKey();
}

void MenuWifi_wifiUpdate(void)
{

	unsigned char ucKey = 0xFF;
	int iRet;
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: MenuWifi_wifiUpdate\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	scrCls_lib();
	scrPrint_lib(1,2,2,"DownLoad WIFI\r\n...\r\n");
    
	if(g_cWifiVersionFlag == 1)
	{
		scrPrint_lib(1,4,2,"Please restart POS after PC tool shows success.");
		hal_utSEUartInit(DOWNLOAD_RATE);

		hal_portUsbOpen();

		kbFlush_lib();
		while(1)
		{  
			sysDelayMs_lib(10);

			hal_portUsbWiFiHandle();
			
			ucKey = kbGetKey_lib();
			
			// 数字键选定功能
			if(ucKey >= '1' && ucKey <= '0')
			{
			
			}

			if(ucKey == KEYCANCEL)
			{                   // 按键: 取消键
				sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:ucKey == KEYCANCEL\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
				hal_portUsbClose();
				hal_utSEUartInit(DEFAULT_RATE);
				break;
			}
		}
	}
	else
	{
		scrPrint_lib(1,4,2,"Wifi version not supported!");
		while(1)
		{
			ucKey = kbGetKey_lib();
			if(ucKey == KEYCANCEL)
			{
				return;
			}
			sysDelayMs(50);
		}
	}

}
#endif

void MenuWifi_webNetwork(void)
{
	int iRet = 0;
	unsigned char ucKey = 0;
	char flag = 0;
	scrCls_lib();
	scrPrint_lib(1,2,2,"WIFI WebNetwork Start...\r\n");
	blWifiConnect = FALSE;
	if(g_cWifiVersionFlag == 1)
	{
		iRet = wifiAPConnectType_lib(2,200);
		if(iRet < 0)
		{
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- Line=%d:<ERR> ESP_Cmd_Init iRet:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(1,4,2,"WIFI WebNetwork Fail!\r\n");
		}
		else{
			scrPrint_lib(1,4,2,"WebNetwork State: 4\r\n");
			flag = 1;
		}

		if(flag == 1)
		{
			while(1)
			{	
				iRet = wifiAPConnectCheck_lib();
				if(iRet == 2)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(1,4,2,"WIFI WebNetwork OK!\r\n");
					blWifiConnect = TRUE;
					break;
				}
				else if(iRet == 3)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(1,4,2,"WIFI WebNetwork timeout!\r\n");
					break;
				}
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)
				{
					wifiAPConnectQuit_lib();
					return;
				}
				scrClrLine_lib(4,4);
				scrPrint_lib(1,4,2,"WebNetwork State: %d\r\n", iRet);
				sysDelayMs(50);
			}
		}
	}
	else{
		scrPrint_lib(1,4,2,"Wifi version not supported!");
	}

	while(1)
	{
		ucKey = kbGetKey_lib();
		if(ucKey == KEYCANCEL)
		{
			return;
		}
		sysDelayMs(50);
	}
}

void MenuWifi_airkess(void)
{
	int iRet = 0;
	char flag = 0;
	unsigned char ucKey = 0;
	scrCls_lib();
	scrPrint_lib(1,2,2,"WIFI airkissNetwork Start...\r\n");
	blWifiConnect = FALSE;
	if(g_cWifiVersionFlag == 1)
	{
		iRet = wifiAPConnectType_lib(1,200);
		if(iRet < 0)
		{
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- Line=%d:<ERR> ESP_Cmd_Init iRet:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(1,4,2,"WIFI airkissNetwork Fail!\r\n");
		}
		else{
			scrPrint_lib(1,4,2,"airkissNetwork State: 4!\r\n");
			flag = 1;
		}	
		if(flag == 1)
		{
			while(1)
			{	
				iRet = wifiAPConnectCheck_lib();
				if(iRet == 2)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(1,4,2,"WIFI airkissNetwork OK!\r\n");
					blWifiConnect = TRUE;
					break;
				}
				else if(iRet == 3)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(1,4,2,"WIFI WebNetwork timeout!\r\n");
					break;
				}
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)
				{
					wifiAPConnectQuit_lib();
					return;
				}
				scrClrLine_lib(4,4);
				scrPrint_lib(1,4,2,"airkissNetwork State: %d\r\n", iRet);
				sysDelayMs(50);
			}
		}
	}
	else{
		scrPrint_lib(1,4,2,"Wifi version not supported!");
	}

	while(1)
	{	
		ucKey = kbGetKey_lib();
		if(ucKey == KEYCANCEL)
		{
			return;
		}
		sysDelayMs(50);
	}
}

void MenuWifi_RESTORE(void)
{
	int iRet = 0;
	unsigned char ucKey = 0;
	scrCls_lib();
	scrPrint_lib(1,2,2,"WIFI RESTORE...\r\n");

	iRet = wifiRestore_lib();
	if(iRet < 0)
	{
		scrPrint_lib(1,4,2,"WIFI RESTORE Fail!\r\n");
	}
	else if(iRet == 0){
		//sysDelayMs(2000);
		//wifiInit_lib();
		scrPrint_lib(1,4,2,"WIFI RESTORE OK!\r\n");
	}
	
	while(1)
	{	
		ucKey = kbGetKey_lib();
		if(ucKey == KEYCANCEL)
		{
			return;
		}
		sysDelayMs(50);
	}
}

void MenuWifi_TCPMultiConnection(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1025];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	char i = 0;
	char ii = 0;
	
	//memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cSendBuff, 'A', sizeof(cSendBuff)/4);
	memset(cSendBuff+sizeof(cSendBuff)/4, 'B', sizeof(cSendBuff)/4);
	memset(cSendBuff+(sizeof(cSendBuff)/4)*2, 'C', sizeof(cSendBuff)/4);
	memset(cSendBuff+(sizeof(cSendBuff)/4)*3, 'D', sizeof(cSendBuff)/4);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    /*if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }*/
/*建立TCP连接*/
	scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
	scrPrint_lib(0, 3, 0x02, "Port:3000");
	scrPrint_lib(0, 4, 0x02, "Connect...."); 	

	for(i=0; i<5; i++)
	{
		isockid = wifiSocketCreate_lib(0);
		sysLOG_lib(1, "[%s] -%s- <%d>:wifiSocketCreate isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
		if(isockid >= 0)
		{
			iRet = wifiTCPConnect_lib(isockid, "192.168.43.30", "3000", 8000);
			if(iRet < 0)
			{
				sysLOG_lib(1, "[%s] -%s- <%d>:ERROR wifiTCPConnect isockid=%d iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid, iRet);
				if(iRet == WIFI_TCP_CONNECTED)
				{
					wifiTCPClose_lib(isockid);
				}
				scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
				scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
				hal_keypadWaitOneKey();	
				for(i=0; i<=isockid; i++)
				{
					wifiSocketClose_lib(i);
				}
				return ;
			}
			scrClrLine_lib(4,5);
			scrPrint_lib(0, 4, 0x02, "Connect OK"); 
		}
		else
		{
			scrClrLine_lib(4,7);
			switch(isockid)
			{
				case WIFI_NOT_OPEN_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
				break;
				
				case WIFI_SOCKETCREATE_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
				break;
				default:
					scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
					break;
			}
			
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");

			hal_keypadWaitOneKey();
			for(i=0; i<=isockid; i++)
			{
				wifiSocketClose_lib(i);
			}
			return;	
		}
	}
	

	uTimeBegin = hal_sysGetTickms();
    /*循环发送、接收数据*/
	while(1)
	{
		//ii++;
		scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
		    break;
		}
		
		for(i=0; i<5; i++)
		{
			iSum += 1;
            sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum,i);
			iRet = wifiSend_lib(i,cSendBuff, 1024-1, 1000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
			if(iRet == 0)
			{
				iSuccT += 1;
			}
			else if(iRet == WIFI_AP_LINK_ERR)
			{
			    scrClrLine_lib(4,4);
			    scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
			    scrClrLine_lib(6,7);
				break;
			}
			else
			{
			    iErr += 1;
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
				continue;
			}
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, i);
			iRet = wifiRecv_lib(i, cRecvBuff, 1024-1, 10000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 1024-1)
			{
				sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib cRecvBuff=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, cRecvBuff);
				if(0 == memcmp(cSendBuff, cRecvBuff, 1024-1))
				{
					iSuccR += 1;
					sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib iSuccR=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSuccR);
				}
				else{
					sysBeep_lib();
					while(1)
					{	
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							goto exit;
							//return;
						}
						sysDelayMs(50);
					}
				}
			}
			else{
				sysBeep_lib();
				while(1)
				{	
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
					{
						goto exit;
						//return;
					}
					sysDelayMs(50);
				}
			}
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);

		}
		sysDelayMs(20);
	}
exit:
	uTimeEnd = hal_sysGetTickms();
    //wifiTCPClose_lib(isockid);

    for (i = 0; i < 5; i++)
    {
        wifiSocketClose_lib(i);
    }

    uTimeS = (uTimeEnd-uTimeBegin) / 1000;
	scrPrint_lib(0, 6, 0x02, "Rate(KB/s):%d, AnyKey", (iSuccT+iSuccR) / uTimeS);
	hal_keypadWaitOneKey();
}

void MenuWifi_MallocResource(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1025];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	uint32 allsize=0, availsize=0, maxblocksize=0;
	char i = 0;
	char ii = 0;
	
	//memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cSendBuff, 'A', sizeof(cSendBuff)/4);
	memset(cSendBuff+sizeof(cSendBuff)/4, 'B', sizeof(cSendBuff)/4);
	memset(cSendBuff+(sizeof(cSendBuff)/4)*2, 'C', sizeof(cSendBuff)/4);
	memset(cSendBuff+(sizeof(cSendBuff)/4)*3, 'D', sizeof(cSendBuff)/4);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    /*if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }*/
/*建立TCP连接*/
	scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
	scrPrint_lib(0, 3, 0x02, "Port:3000");
	scrPrint_lib(0, 4, 0x02, "Connect...."); 	
Start:
	for(i=0; i<5; i++)
	{
		isockid = wifiSocketCreate_lib(0);
		sysLOG_lib(1, "[%s] -%s- <%d>:wifiSocketCreate isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
		if(isockid >= 0)
		{
			iRet = wifiTCPConnect_lib(isockid, "192.168.43.30", "3000", 8000);
			if(iRet < 0)
			{
				sysLOG_lib(1, "[%s] -%s- <%d>:ERROR wifiTCPConnect isockid=%d iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid, iRet);
				if(iRet == WIFI_TCP_CONNECTED)
				{
					wifiTCPClose_lib(isockid);
				}
				scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
				scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
				hal_keypadWaitOneKey();	
				for(i=0; i<=isockid; i++)
				{
					wifiSocketClose_lib(i);
				}
				return ;
			}
			scrClrLine_lib(4,5);
			scrPrint_lib(0, 4, 0x02, "Connect OK"); 
		}
		else
		{
			scrClrLine_lib(4,7);
			switch(isockid)
			{
				case WIFI_NOT_OPEN_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
				break;
				
				case WIFI_SOCKETCREATE_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
				break;
				default:
					scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
					break;
			}
			
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");

			hal_keypadWaitOneKey();
			for(i=0; i<=isockid; i++)
			{
				wifiSocketClose_lib(i);
			}
			return;	
		}
	}
	iRet = fibo_get_heapinfo(&allsize, &availsize, &maxblocksize);
	sysLOG_lib(1, "[%s] -%s- <%d>:allsize=%d availsize=%d maxblocksize=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, allsize, availsize, maxblocksize);
	uTimeBegin = hal_sysGetTickms();
	ii = 0;
    /*循环发送、接收数据*/
	while(ii < 5)
	{
		ii++;
		scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
            ucKey = kbGetKey_lib();
            if (ucKey == KEYCANCEL)
            {
                for (i = 0; i < 5; i++)
                {
                    wifiSocketClose_lib(i);
                }
                return;
            }
			else if(ucKey == KEY0)
			{
				return;
			}
        }
		
		for(i=0; i<5; i++)
		{
			iSum += 1;
            sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum,i);
			iRet = wifiSend_lib(i,cSendBuff, 1024-1, 1000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
			if(iRet == 0)
			{
				iSuccT += 1;
			}
			else if(iRet == WIFI_AP_LINK_ERR)
			{
			    scrClrLine_lib(4,4);
			    scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
			    scrClrLine_lib(6,7);
				break;
			}
			else
			{
			    iErr += 1;
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
				continue;
			}
			
			memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, i);
			iRet = wifiRecv_lib(i, cRecvBuff, 1024-1, 10000);
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
			if(iRet == 1024-1)
			{
				sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib cRecvBuff=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, cRecvBuff);
				if(0 == memcmp(cSendBuff, cRecvBuff, 1024-1))
				{
					iSuccR += 1;
					sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib iSuccR=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSuccR);
				}
				else{
					sysBeep_lib();
					while(1)
					{	
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							goto exit;
							//return;
						}
						sysDelayMs(50);
					}
				}
			}
			else{
				sysBeep_lib();
				while(1)
				{	
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
					{
						goto exit;
						//return;
					}
					sysDelayMs(50);
				}
			}
			scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);

		}
		sysDelayMs(20);
	}
exit:
	uTimeEnd = hal_sysGetTickms();
    
	for (i = 0; i < 5; i++)
    {
        wifiSocketClose_lib(i);
    }
    /*wifiInit_lib();
    sysDelayMs(10000);*/
    goto Start;

    uTimeS = (uTimeEnd-uTimeBegin) / 1000;
	scrPrint_lib(0, 6, 0x02, "Rate(KB/s):%d, AnyKey", (iSuccT+iSuccR) / uTimeS);
	hal_keypadWaitOneKey();
}
#if 0
void MenuWifi_TCPAndSSL(void)
{
	int iRet = -1;
	char cSendBuff[1024];
	char cRecvBuff[1025];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	int iErr = 0;
	unsigned char ucKey = 0xFF;
	int isockid = 0;
	int sslisockid = 0;
	uint32 uTimeBegin;
	uint32 uTimeEnd;
	uint32 uTimeS;
	char i = 0;
	char ii = 0;
	
	//memset(cSendBuff, 'A', sizeof(cSendBuff));
	memset(cSendBuff, 'A', sizeof(cSendBuff)/4);
	memset(cSendBuff+sizeof(cSendBuff)/4, 'B', sizeof(cSendBuff)/4);
	memset(cSendBuff+(sizeof(cSendBuff)/4)*2, 'C', sizeof(cSendBuff)/4);
	memset(cSendBuff+(sizeof(cSendBuff)/4)*3, 'D', sizeof(cSendBuff)/4);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	scrClrLine_lib(2,7);
    /*if(!blWifiConnect)
    {
        scrPrint_lib(0, 3, 0x02, "Please Choose a Network");
		hal_keypadWaitOneKey();	
		return;
    }*/
/*建立TCP连接*/
	scrPrint_lib(0, 2, 0x02, "IP:103.235.231.21");
	scrPrint_lib(0, 3, 0x02, "Port:3000");
	scrPrint_lib(0, 4, 0x02, "Connect...."); 	

	for(i=0; i<4; i++)
	{
		isockid = wifiSocketCreate_lib(0);
		sysLOG_lib(1, "[%s] -%s- <%d>:wifiSocketCreate isockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid);
		if(isockid >= 0)
		{
			iRet = wifiTCPConnect_lib(isockid, "103.235.231.21", "3000", 8000);
			if(iRet < 0)
			{
				sysLOG_lib(1, "[%s] -%s- <%d>:ERROR wifiTCPConnect isockid=%d iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, isockid, iRet);
				if(iRet == WIFI_TCP_CONNECTED)
				{
					wifiTCPClose_lib(isockid);
				}
				scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
				scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
				hal_keypadWaitOneKey();	
				wifiSocketClose_lib(isockid);
				for(i=0; i<=isockid; i++)
				{
					wifiSocketClose_lib(i);
				}
				return ;
			}
			scrClrLine_lib(4,5);
			scrPrint_lib(0, 4, 0x02, "Connect OK"); 
		}
		else
		{
			scrClrLine_lib(4,7);
			switch(isockid)
			{
				case WIFI_NOT_OPEN_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
				break;
				
				case WIFI_SOCKETCREATE_ERR:
					scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
				break;
				default:
					scrPrint_lib(0, 4, 0x00, "Err: %d", isockid);
					break;
			}
			
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();
			for(i=0; i<=isockid; i++)
            {
                wifiSocketClose_lib(i);
            }
            return;	
		}
	}
	
	sslisockid = wifiSSLSocketCreate_lib();
	sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSSLSocketCreate_lib sslisockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, sslisockid);
	if(sslisockid >= 0)
	{
		iRet = wifiSSLConnect_lib(sslisockid, "gw.open.icbc.com.cn", "443", 8000);
		if(iRet < 0)
		{
			if(iRet == WIFI_TCP_CONNECTED)
			{
				wifiTCPClose_lib(sslisockid);
			}
			scrPrint_lib(0, 5, 0x00, "TCP Connect Err: %d", iRet);
			scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
			hal_keypadWaitOneKey();	
			wifiSocketClose_lib(sslisockid);
			return ;
		}
		scrClrLine_lib(4,5);
		scrPrint_lib(0, 4, 0x02, "Connect OK"); 
	}
	else
	{
		scrClrLine_lib(4,7);
		switch(sslisockid)
		{
			case WIFI_NOT_OPEN_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_NOT_OPEN_ERR");
			break;
			
			case WIFI_SOCKETCREATE_ERR:
				scrPrint_lib(0, 4, 0x00, "WIFI_SOCKETCREATE_ERR");
			break;
			default:
				scrPrint_lib(0, 4, 0x00, "Err: %d", sslisockid);
				break;
		}
		
		scrPrint_lib(0, 6, 0x02, "AnyKey Exit");
		hal_keypadWaitOneKey();
		return;	
	}

	uTimeBegin = hal_sysGetTickms();
    /*循环发送、接收数据*/
	while(1)
	{
		//ii++;
		scrClrLine_lib(6,7);
		if(0 == kbHit_lib())
		{
		    break;
		}
		
		for(i=0; i<5; i++)
		{
			if(i<4)
			{
				iSum += 1;
				sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum,i);
				iRet = wifiSend_lib(i,cSendBuff, 1024-i, 1000);
				sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
				if(iRet == 0)
				{
					iSuccT += 1;
				}
				else if(iRet == WIFI_AP_LINK_ERR)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
					scrClrLine_lib(6,7);
					break;
				}
				else
				{
					iErr += 1;
					scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
					continue;
				}
				
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib begin %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, i);
				iRet = wifiRecv_lib(i, cRecvBuff, 1024-i, 10000);
				sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
				if(iRet == 1024-i)
				{
					sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib cRecvBuff=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, cRecvBuff);
					if(0 == memcmp(cSendBuff, cRecvBuff, 1024-i))
					{
						iSuccR += 1;
						sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib iSuccR=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSuccR);
					}
					else{
						sysBeep_lib();
						while(1)
						{	
							ucKey = kbGetKey_lib();
							if(ucKey == KEYCANCEL)
							{
								goto exit;
							}
							sysDelayMs(50);
						}
					}
				}
				else{
					sysBeep_lib();
					while(1)
					{	
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							goto exit;
						}
						sysDelayMs(50);
					}
				}
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
			}
			else{
				iSum += 1;
				sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib begin iSum=%d sslisockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSum,sslisockid);
				iRet = wifiSSLSend_lib(sslisockid,cSendBuff, 1019, 1000);
				sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiSend_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				scrPrint_lib(0, 6, 0x00, "wifiSend iRet:%d", iRet);
				if(iRet == 0)
				{
					iSuccT += 1;
				}
				else if(iRet == WIFI_AP_LINK_ERR)
				{
					scrClrLine_lib(4,4);
					scrPrint_lib(0, 4, 0x00, "WIFI_AP_LINK_ERR iRet:%d", iRet);
					scrClrLine_lib(6,7);
					break;
				}
				else
				{
					iErr += 1;
					scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
					continue;
				}
				
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib begin sslisockid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,sslisockid);
				iRet = wifiSSLRecv_lib(sslisockid, cRecvBuff, 1019, 10000);
				sysLOG_lib(APIWIFI_LOG_LEVEL_2, "[%s] -%s- <%d>:wifiRecv_lib end iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
				scrPrint_lib(0, 7, 0x00, "wifiRecv iRet:%d", iRet);
				if(iRet == 1019)
				{
					sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib cRecvBuff=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, cRecvBuff);
					if(0 == memcmp(cSendBuff, cRecvBuff, 1019))
					{
						iSuccR += 1;
						sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- <%d>:wifiRecv_lib iSuccR=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSuccR);
					}
					else{
						sysBeep_lib();
						while(1)
						{	
							ucKey = kbGetKey_lib();
							if(ucKey == KEYCANCEL)
							{
								goto exit;
							}
							sysDelayMs(50);
						}
					}
				}
				else{
					sysBeep_lib();
					while(1)
					{	
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							goto exit;
						}
						sysDelayMs(50);
					}
				}
				scrPrint_lib(0, 5, 0x00, "S:%d T:%d R:%d E:%d", iSum, iSuccT, iSuccR, iErr);
			}
			

		}
		sysDelayMs(20);
	}
	uTimeEnd = hal_sysGetTickms();
exit: 
	for(i=0; i<4; i++)
	{
		wifiSocketClose_lib(i);
	}
	wifiSSLSocketClose_lib(sslisockid);
	uTimeS = (uTimeEnd-uTimeBegin) / 1000;
	scrPrint_lib(0, 6, 0x02, "Rate(KB/s):%d, AnyKey", (iSuccT+iSuccR) / uTimeS);
	hal_keypadWaitOneKey();
}

int hal_espSetWifiMode(char cMode);
int hal_espQueWifiMode(void);

void MenuWifi_QueStatus(void)
{
	int iRet = 0;
	unsigned char ucKey = 0;
	scrCls_lib();
	scrPrint_lib(1,2,2,"Que Status...\r\n");

	while(1)
	{
		ucKey = kbGetKey_lib();
		if(ucKey == KEY1)
		{
			iRet = wifiGetLinkStatus_lib();
		}
		else if(ucKey == KEY2)
		{
			iRet = hal_espQueWifiMode();
		}
		else if(ucKey == KEY3)
		{
			hal_espSetWifiMode(1);
		}
		else if(ucKey == KEY4)
		{
			hal_espSetWifiMode(2);
		}
        else if(ucKey == KEYCANCEL)
        {
           return;
        }
        sysDelayMs(50);
    }
	
}
#endif
void Menu_wifiLocalUpdate(void)
{
	unsigned char ucKey = 0xFF;
	int iRet = -1;
	unsigned char ucCmd = 0;
    unsigned char IP[20] = {0};
    int Len = 0;
	unsigned char key = 0;
    int type = 0;

	scrCls_lib();
	kbFlush_lib();

	scrPrint_Ex(0x00, 1, 0x02, "本地升级", "Local Update");
	if(g_cWifiVersionFlag == 1)
	{
		iRet = wifiAPConnectType_lib(2,300000);
		if(iRet < 0)
		{
			sysLOG_lib(APIWIFI_LOG_LEVEL_1, "[%s] -%s- Line=%d:<ERR> ESP_Cmd_Init iRet:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			scrPrint_lib(1,4,2,"WIFI UPdate Fail!\r\n");
		}

		while(1)
		{
			if(0 == kbHit_lib())
            {
                ucKey = kbGetKey_lib();
                if (ucKey == KEYCANCEL)
                {
					wifiWebUpdateQue_lib(1);
                    return;
                }
            }

            iRet = wifiWebUpdateQue_lib(0);
			if(iRet == 1)
			{
				scrPrint_lib(2, 4, 2, "Wait Update!");
			}
			else if(iRet == 2)
			{
				scrClrLine_lib(4,4);
				scrPrint_lib(2, 4, 2, "Start receiving data!");
			}
			else if(iRet == 3)
			{
				scrClrLine_lib(4,4);
				scrPrint_lib(2, 4, 2, "Local Update OK!");
				while(1)
                {
                    ucKey = kbGetKey_lib();
                    if (ucKey == KEYCANCEL)
                    {
                        return;
                    }
                    sysDelayMs(50);
                }
            }
			else if(iRet < 0)
			{
				scrClrLine_lib(4,4);
				scrPrint_lib(2, 4, 2, "Local Update Fail!");
				while(1)
                {
                    ucKey = kbGetKey_lib();
                    if (ucKey == KEYCANCEL)
                    {
                        return;
                    }
                    sysDelayMs(50);
                }
            }
			sysDelayMs(50);
		}
	}
	else{
		memset(IP, 0x00, sizeof(IP));
		scrPrint_Ex(0x00, 3, 0x02, "请输入IP", "Pls Input IP");
		scrPrint_lib(1,5,0,"IP=%s",IP);
		while(1)
		{
			if(kbHit_lib() == 0)
			{
				ucCmd = kbGetKey_lib();
				switch(ucCmd)
				{
					case KEYENTER:
						scrClrLine_lib(5,7);
						if((Len > 10) && (Len < 16))
						{
							
							iRet = WifiLocalDown_lib(IP, Len);
							if(iRet < 0)
							{
								scrClrLine_lib(4,4);
								scrPrint_lib(2, 4, 0x00, "Local Update Fail!");
							}
							else{
								scrClrLine_lib(4,4);
								scrPrint_lib(2, 4, 0x00, "Local Update OK!");
							}
							while(1)
							{
								ucKey = kbGetKey_lib();
								if(ucKey == KEYCANCEL)
								{
									return;
								}
								sysDelayMs(50);
							}

						}
						else{
							scrPrint_lib(0, 5, 2, "Input IP ERROR");
						}
						
					break;
					case KEYCANCEL:
						return ;
					break;
					case KEY_FN:
						//return UNLOCKPAD_FN;
					break;
					case KEYPOINT:
						if(Len < 16)
						{
							IP[Len++] = '.';
							scrClrLine_lib(5,7);
							scrPrint_lib(1,5,0,"IP=%s",IP);
						}
					break;
					case KEYCLEAR:
						IP[Len-1] = 0x00;
						Len--;
						scrClrLine_lib(5,7);
						scrPrint_lib(1,5,0,"IP=%s",IP);
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
						if(Len < 16)
						{
							IP[Len++] = ucCmd;
							scrClrLine_lib(5,7);
							scrPrint_lib(1,5,0,"IP=%s",IP);
						}
					break;
					default:
					break;

				}
			}
			sysDelayMs(20);
		}
	}
	
}
#if 0
void Menu_wifiTestCMD(void)
{
	unsigned char ucKey = 0xFF;
	int iRet = -1;
	char ssid[50];
	char pass[20];
	char pcDnsServer0[50];
	char pcDnsServer1[50];
    char pcIP[50], pcGateWay[50], pcNetMask[50];
	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "CMD TEST");

	while(1)
    {
        ucKey = kbGetKey_lib();
        if (ucKey == KEYCANCEL)
        {
            return;
        }
		else if(ucKey == KEY0)
		{
			memset(ssid, 0x00, sizeof(ssid));
			memset(pass, 0x00, sizeof(pass));
			scrClrLine_lib(5,6);
			wifiAPCheck_lib(ssid, pass);
			scrPrint_lib(1,5,0,"ssid=%s",ssid);
			scrPrint_lib(1,6,0,"pass=%s",pass);
		}
		else if(ucKey == KEY1)
		{
			scrClrLine_lib(5,6);
			iRet = hal_espConfigDNSEnable(0);
		}
		else if(ucKey == KEY2)
		{
			scrClrLine_lib(5,6);
			iRet = hal_espConfigDNSEnable(1);
		}
		else if(ucKey == KEY3)
		{
			scrClrLine_lib(5,6);
			wifiConfigDNS_lib("208.67.222.222", "114.114.114.114");
		}
		else if(ucKey == KEY4)
		{
			scrClrLine_lib(5,6);
			memset(pcDnsServer0, 0x00, sizeof(pcDnsServer0));
			memset(pcDnsServer1, 0x00, sizeof(pcDnsServer1));
			wifiGetDNS_lib(pcDnsServer0, pcDnsServer1);
			scrPrint_lib(1,5,0,"DNS1=%s",pcDnsServer0);
			scrPrint_lib(1,6,0,"DNS2=%s",pcDnsServer1);
		}
        else if(ucKey == KEY5)
        {
            hal_espStationSetDHCP(0);//关闭自动获取
        }
        else if(ucKey == KEY6)
        {
            hal_espStationSetDHCP(1);//开启自动获取
        }
        else if(ucKey == KEY7)
        {
            hal_espStationGetDHCP();
        }
        else if(ucKey == KEY8)
        {
            hal_espStationSetIP("192.168.1.100", "192.168.1.1", "255.255.255.0");
        }
        else if(ucKey == KEY9)
        {
            scrClrLine_lib(4,6);
            memset(pcIP, 0, sizeof(pcIP));
            memset(pcGateWay, 0, sizeof(pcGateWay));
            memset(pcNetMask, 0, sizeof(pcNetMask));
            hal_espStationGetIP(pcIP, pcGateWay, pcNetMask);
            scrPrint_lib(1,4,0,"IP=%s",pcIP);
            scrPrint_lib(1,5,0,"GateWay=%s",pcGateWay);
            scrPrint_lib(1,6,0,"NetMask=%s",pcNetMask);
        }
        sysDelayMs(50);
    }
   
}
#endif
void Menu_addFun(void)
{
	int iSize = 0;
	
	STMENUNODE astMenu[] = {
				{ "WIFI测试", "WIFI TEST", NoFun },
				//{ "多连接",   "MultiConnection", MenuWifi_TCPMultiConnection},
				//{ "多连接+SSL", "TCP+SSL", MenuWifi_TCPAndSSL},
				//{ "版本升级", "wifiUpdate", MenuWifi_wifiUpdate},
				{ "Web配网", "webNetwork", MenuWifi_webNetwork},
				{ "智能配网", "airkessNetwork", MenuWifi_airkess},
				{ "恢复设置", "WifiRESTORE ",MenuWifi_RESTORE},
				//{ "使用内存", "useOfMemory", MenuWifi_MallocResource},
				{ "本地升级", "localUpdate",   Menu_wifiLocalUpdate},
				//{ "下载证书", "DownloadCertificate", Menu_DownloadCertificate},
				//{ "状态查询", "StatusQue ", MenuWifi_QueStatus},
				//{ "wifiSocket", "wifiSocket", MenuWifi_socketBoundaryTestDataTwo},
				//{ "证书测试", "CertTest", MenuWifi_sslCrt},
				//{ "测试指令", "testCMD", Menu_wifiTestCMD},
			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;    
}

void Menu_ModChkComWIFI(void)
{
	int iSize = 0;
	
	STMENUNODE astMenu[] = {
				{ "WIFI测试", "WIFI TEST", NoFun },
				{ "选取网络    ", "Choose a network", MenuWifi_Scan_ConnectAp},
				{ "TCP通讯测试 ", "TCP             ", MenuWifi_TCP},
//			    { "wifi版本升级", "wifi_update ", wifiupdate_test},
//			    { "wifiSSL通讯", "wifi_SSL ", MenuWifi_ssl},
				{ "wifiscan", "wifiscan ",MenuWifi_scanAp},
//				{ "wifisocketTrans", "wifisocketTrans ",MenuWifi_socketTrans},
//				{ "设置MAC", "wifi_SetMAC ", MenuWifi_SetMAC},
				{ "查看MAC", "wifi_GetMAC ", MenuWifi_GetMAC},
				{ "增加功能", "addFun",       Menu_addFun},

				//{ "wifi串口失能", "wifi_deinitUart ", wifi_deinituart}		
				//{ "打开WIFI", "WIFI OPEN", api_wifiTest_Open },
				//{ "热点扫描", "AP SCAN  ", api_wifiTest_Connect},
			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;    
}

void Menu_ChkPort(void)
{
	unsigned char ucKey = 0;
	int iRet = 0;
	unsigned char i=0, j = 0;
	unsigned char sendFlag  = 0;
	unsigned char QuitPrintFlag = 0;
	unsigned char SendBuf[16];
	unsigned char RecvBuf[256];

	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "PORT TEST");
	scrPrint_lib(0,3,2,"select the baud rate");
	scrPrint_lib(0,4,0,"1.9600");
	scrPrint_lib(0,5,0,"2.38400");
	scrPrint_lib(0,6,0,"3.115200");
	scrPrint_lib(0,4,4,"4.460800");
	scrPrint_lib(0,5,4,"5.512000");
	scrPrint_lib(0,6,4,"6.921600");
		while(1)
		{
			if(kbHit_lib()==0)                           //按任意键进入
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)                   //取消键退出
				{		
					kbFlush_lib();
					return;
				}
				else if(ucKey == KEY1)
				{
					iRet = portOpen_lib(11, "9600,8,n,1");
				}
				else if(ucKey == KEY2)
				{
					iRet = portOpen_lib(11, "38400,8,n,1");
				}
				else if(ucKey == KEY3)
				{
					iRet = portOpen_lib(11, "115200,8,n,1");
				}
				else if(ucKey == KEY4)
				{
					iRet = portOpen_lib(11, "460800,8,n,1");
				}
				else if(ucKey == KEY5)
				{
					iRet = portOpen_lib(11, "512000,8,n,1");
				}
				else if(ucKey == KEY6)
				{
					iRet = portOpen_lib(11, "921600,8,n,1");
				}
				else{
					continue;
				}
				
				if(iRet != 0)
				{
					scrClrLine_lib(3,4);
					scrClrLine_lib(4,5);
					scrClrLine_lib(5,6);
					scrClrLine_lib(6,7);
					scrPrint_lib(0,4,2,"Open port fail");
					while(1)
					{
						if(kbHit_lib()==0)                           //按任意键进入
						{
							ucKey = kbGetKey_lib();
							//if(ucKey == KEYCANCEL)                   //取消键退出
							{		
								kbFlush_lib();
								return;
							}
						}
						sysDelayMs(50);
					}
				}
				break;
			}
			sysDelayMs(50);
		}

	while(1)
	{
		scrClrLine_lib(3,4);
		scrClrLine_lib(4,5);
		scrClrLine_lib(5,6);
		scrClrLine_lib(6,7);

		scrPrint_lib(0,3,2,"1.send 2.recv");

		while(1)
		{	
			if(kbHit_lib()==0)                           //按任意键进入
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)                   //取消键退出
				{		
					portClose_lib(11);
					kbFlush_lib();
					return;
				}
				if(ucKey == KEY2)                   //取消键退出
				{		
					kbFlush_lib();
					break;
				}
				if(ucKey != KEY1)                   //取消键退出
				{		
					kbFlush_lib();
					continue;
				}

				for(j=0; j<8; j++)
				{
					SendBuf[2*j]=0x55;
					SendBuf[2*j+1]=0xAA;
				}
				iRet = portSends_lib(11, SendBuf, 16);
			    if(iRet != 0)
				{
					scrClrLine_lib(4,5);
					scrPrint_lib(0,4,2,"Send data fail");
					while(1)
			    	{
						if(kbHit_lib()==0)                           //按任意键进入
						{
							ucKey = kbGetKey_lib();
							//if(ucKey == KEYCANCEL)                   //取消键退出
							{		
								portClose_lib(11);
								kbFlush_lib();
								return;
							}
						}
						sysDelayMs(50);
			    	}
				}

				scrClrLine_lib(5,6);
				scrPrint_lib(0,5,2,"Send data 16 bytes");
			}
			sysDelayMs(50);
		}

		iRet = portFlushBuf_lib(11);
		if(iRet != 0)
		{
				scrClrLine_lib(4,5);
				scrPrint_lib(0,4,2,"Clear flush fail");
				while(1)
			   	{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						{		
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
					}
					sysDelayMs(50);
					//sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: Clear flush fail\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			    }
		}

		scrClrLine_lib(4,5);
		scrClrLine_lib(3,4);
		kbFlush_lib();
		scrPrint_lib(0,3,2,"1.send 2.recv");

		while(1)
		{	
			if(kbHit_lib()==0)
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)                   //取消键退出
				{		
					portClose_lib(11);
					kbFlush_lib();
					return;
				}
				sysDelayMs(50);
				
				if(ucKey == KEY1)                
				{		
					kbFlush_lib();
					break;
				}
			}
				memset(RecvBuf, 0x00, sizeof(RecvBuf));
				iRet = portRecvs_lib(11, RecvBuf,256,2000);
				sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portRecvs iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet);
				if(iRet < 0)
				{
					iRet = 0;
#if 0
					scrClrLine_lib(4,5);
					scrClrLine_lib(5,6);
					scrClrLine_lib(6,7);
					scrPrint_lib(0,4,2,"Recvs data fail");
					while(1)
			    	{
						if(kbHit_lib()==0)                           //按任意键进入
						{
							sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: Recvs ucKey1%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,ucKey);
							ucKey = kbGetKey_lib();
							sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: Recvs ucKey2%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,ucKey);
							{		
								kbFlush_lib();
								sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: Recvs\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
								return;
							}
						}
						//sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: Recvs\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
						sysDelayMs(50);
					}
#endif
				}

				scrClrLine_lib(4,5);
				scrClrLine_lib(5,6);
				scrClrLine_lib(6,7);
				scrPrint_lib(0, 5, 0x00, "received bytes=%d",iRet);
				if(iRet > 0)
				{
					for(j=0; j<iRet; j++)
					{
						RecvBuf[j] -= 0x30;
					}
					scrPrint_lib(0, 6, 0, "RecvBuf %d %d %d %d %d %d %d %d", RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[3],RecvBuf[4],RecvBuf[5],RecvBuf[6],RecvBuf[7]);
				}
				
				//scrPrint_lib(0,4,2,"Recvs data complete");
				sysLOG_lib(BASE_LOG_LEVEL_1, "RecvBuf %d %d %d %d %d %d %d %d", RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[3],RecvBuf[4],RecvBuf[5],RecvBuf[6],RecvBuf[7]);
				//scrPrint_lib(0, 4, 0, "RecvBuf=%x%x%x%x%x%x",RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[3],RecvBuf[4],RecvBuf[5]);
				if(iRet > 0) 
				{
					kbFlush_lib();
					scrClrLine_lib(3,4);
					scrPrint_lib(0,3,2,"1.send 2.recv");
					while(1)
					{
						if(kbHit_lib()==0)
						{
							ucKey = kbGetKey_lib();
							if(ucKey == KEYCANCEL)                   //取消键退出
							{		
								portClose_lib(11);
								kbFlush_lib();
								return;
							}
							else if(ucKey == KEY1)
							{
								kbFlush_lib();
								sendFlag = 1;
								break;
							}
							else if(ucKey == KEY2)
							{
								kbFlush_lib();
								scrClrLine_lib(3,4);
								scrClrLine_lib(5,6);
								scrClrLine_lib(6,7);
								scrPrint_lib(0,3,2,"1.send 2.recv");
								break;
							}
						}
						sysDelayMs(50);
					}
					if(sendFlag == 1)
					{
						sendFlag = 0;
						break;
					}
				}
			sysDelayMs(50);
			//sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: Clear flush and recv data\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		}	
	}

}

void Menu_BLEserver()
{
	int iRet = 0;
	unsigned char ucKey = 0;
	uint8_t *data,*makeStr;
	int recLen,total;
	scrCls_lib();
	kbFlush_lib();

	scrPrint_lib(0, 1, 0, "BLE begin");
	//iRet = api_blueToothInit();
	iRet = btOpen_lib();
	scrPrint_lib(0, 2, 0, "BLE btOpen_lib:%d",iRet);
	
	data = (uint8_t *)malloc(256);
	memset(data,0,256);
	makeStr = (uint8_t *)malloc(256);
	memset(makeStr,0,256);
	
	scrPrint_lib(0, 2, 0, "pls open APP connect");
	char mac[18];
	memset(mac,0,sizeof(mac));
	hal_btGetMac(mac,17);
	scrPrint_lib(0, 3, 0, "MAC:%s",mac);

	while(1)
	{
		
		recLen = btRecv_lib(data,20,10);
		if(total >= 20)
		{
			memset(makeStr, 0, sizeof(makeStr));
			sprintf(makeStr,"NOTIFY:%s",data);
			btSend_lib(makeStr, total+4);
			OSI_PRINTFI("AUTO_BLE btRecv_lib total:%d, last str data:%s" ,total, data);
			scrClrLine_lib(3, 6);
			scrPrint_lib(0, 4, 0, "R<-:%s",data);
			scrPrint_lib(0, 6, 0, "W->:%s",makeStr);
			total = 0;
			memset(data,0,256);
		}
		else if(recLen > 0)
		{
			total+=recLen;
		}
		else
		{
			OSI_PRINTFI("AUTO_BLE btRecv_lib waitting data total=%d",total);
		}				
		sysDelayMs(100);
		
		ucKey = kbGetKey_lib();
		if(ucKey == KEYCANCEL)
			break;
	}
	free(data);
	free(makeStr);
	btClose_lib();

}
void Menu_USBQueCurMode(void)
{
	unsigned char ucKey = 0;
	INT32 usbModeVal = 0;

	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "Get USB Mode");

	usbModeVal = fibo_get_usbmode();
	if(usbModeVal == 31)
	{
		scrPrint_lib(0, 4, 2, "Current USB 0 to 5 On");
	}
	else
	{
		scrPrint_lib(0, 4, 2, "Current USB 0 to 5 Off");
	}
	
	//scrPrint_lib(0,4,2,"USB Current Mode=%d",usbModeVal);
	while(1)
	{
		if(kbHit_lib()==0)                           //按任意键进入=
		{
			ucKey = kbGetKey_lib();
			if(ucKey == KEYCANCEL)                   //取消键退出
			{		
				kbFlush_lib();
				return;
			}
		}
		sysDelayMs(100);
	}
}

void Menu_USBSetMode(void)
{
	unsigned char ucKey = 0;
	INT32 usbModeVal = 0;

	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "Set USB Mode");

	usbModeVal = fibo_get_usbmode();
	if(usbModeVal == 31)
	{
		//scrPrint_lib(0, 3, 2, "Current USB 0 to 4 On");
		scrPrint_lib(0, 4, 2, "Enter Close USB 0 to 5");
		while(1)
		{
			if(kbHit_lib()==0)                           //按任意键进入=
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)                   //取消键退出
				{		
					kbFlush_lib();
					return;
				}
				else if(ucKey == KEYENTER)
				{
					fibo_set_usbmode(34);
					scrPrint_lib(0, 6, 2, "Sudo Reboot...");
					sysDelayMs(2000);
					DeviceReboot();
				}
			}
			sysDelayMs(100);
		}
	}
	else if(usbModeVal == 34)
	{
		//scrPrint_lib(0, 3, 2, "Current USB 0 to 4 Off");
		scrPrint_lib(0, 4, 2, "Enter Open USB 0 to 5");
		while(1)
		{
			if(kbHit_lib()==0)                           //按任意键进入=
			{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)                   //取消键退出
				{		
					kbFlush_lib();
					return;
				}
				else if(ucKey == KEYENTER)
				{
					fibo_set_usbmode(31);
					scrPrint_lib(0, 6, 2, "Sudo Reboot...");
					sysDelayMs(2000);
					DeviceReboot();
				}
			}
			sysDelayMs(100);
		}
	}	
}

void Menu_USBZeroAndFiveOpen(void)
{
	unsigned char ucKey = 0;
	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "Open All Ports");
	scrPrint_lib(0, 4, 2, "Enter Open USB 0 to 5");
	while(1)
	{
		if(kbHit_lib()==0)                           //按任意键进入=
		{
			ucKey = kbGetKey_lib();
			if(ucKey == KEYCANCEL)                   //取消键退出
			{		
				kbFlush_lib();
				return;
			}
			else if(ucKey == KEYENTER)
			{
				fibo_atDevicePortOpen();
				scrPrint_lib(0, 5, 2, "Open USB 0 to 5 complete");
			}
		}
		sysDelayMs(100);
	}
}

void Menu_USBZeroAndFiveClose(void)
{
	unsigned char ucKey = 0;
	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "Open Single Port");
	scrPrint_lib(0, 4, 2, "Enter Close USB 0 to 5");
	while(1)
	{
		if(kbHit_lib()==0)                           //按任意键进入=
		{
			ucKey = kbGetKey_lib();
			if(ucKey == KEYCANCEL)                   //取消键退出
			{		
				kbFlush_lib();
				return;
			}
			else if(ucKey == KEYENTER)
			{
				fibo_atDevicePortClose();
				scrPrint_lib(0, 5, 2, "Close USB 0 to 5 complete");
			}
		}
		sysDelayMs(100);
	}
}
void Menu_COMSend(void)
{
	unsigned char ucKey = 0;
	int iRet = 0;
	unsigned char i=0, j = 0;
	unsigned char sendFlag  = 0;
	unsigned char QuitPrintFlag = 0;
	unsigned char SendBuf[512];
	unsigned char RecvBuf[512];
	unsigned char RecvBufCmp[512];
	unsigned int counter = 0;
	unsigned int errorCounter = 0;
	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "PORT Clent TEST");
	memset(SendBuf, 0x00, sizeof(SendBuf));
	memset(RecvBuf, 0x00, sizeof(RecvBuf));
	memset(RecvBufCmp, 0x56, sizeof(RecvBufCmp));
   iRet = portOpen_lib(11, "115200,8,n,1");
   if(iRet != 0)
   {
	   scrClrLine_lib(4,7);
	   scrPrint_lib(0,4,2,"Open port fail");
	   while(1)
	   {
		   if(kbHit_lib()==0)							//按任意键进入
		   {
			   ucKey = kbGetKey_lib();
			   //if(ucKey == KEYCANCEL) 				  //取消键退出
			   {	   
				   kbFlush_lib();
				   return FALSE;
			   }
		   }
		   sysDelayMs(50);
	   }
   }
   
   iRet = portFlushBuf_lib(11);
   if(iRet != 0)
   {
		scrClrLine_lib(4,5);
		scrPrint_lib(0,4,2,"Clear flush fail");
		while(1)
		{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
					}
					sysDelayMs(50);
		}
	}

   SendBuf[0] = 0x10;
   SendBuf[1] = 0x05;
   SendBuf[2] = 0x01;
   SendBuf[3] = SendBuf[1] + SendBuf[2];
   SendBuf[4] = 0x16;
   scrClrLine_lib(4,5);
   scrPrint_lib(0,4,2,"Waiting handshake...");
   while(1)
   {
	    iRet = portSends_lib(11, SendBuf, 5);
		if(iRet != 0)
		{
			scrClrLine_lib(4,5);
			scrPrint_lib(0,4,2,"Send data fail");
			while(1)
			{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
				}
				sysDelayMs(50);
			}
		}

		iRet = portRecvs_lib(11, RecvBuf,5,2000);
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portRecvs iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet);
		if(iRet < 0)
		{
			scrClrLine_lib(4,5);
			scrPrint_lib(0,4,2,"Recvs data fail");
			while(1)
			{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
				}
				sysDelayMs(50);
			}
		}
		scrClrLine_lib(5,6);
		scrPrint_lib(0, 5, 2, "iRet=[%d]:%x %x %x %x %x", iRet, RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[3],RecvBuf[4]);
		if((RecvBuf[0] == 0x10) && (RecvBuf[1] == 0x55) && (RecvBuf[2] == 0x00))
		{
			if((RecvBuf[3] == RecvBuf[1] + RecvBuf[2]) && (RecvBuf[4] == 0x16))
			{
				SendBuf[0] = 0x10;
				SendBuf[1] = 0x06;
				SendBuf[2] = 0x01;
				SendBuf[3] = SendBuf[1] + SendBuf[2];
				SendBuf[4] = 0x16;
				iRet = portSends_lib(11, SendBuf, 5);
				if(iRet != 0)
				{
					scrClrLine_lib(4,5);
					scrPrint_lib(0,4,2,"Send data fail");
					while(1)
					{
						if(kbHit_lib()==0)                           //按任意键进入
						{
							ucKey = kbGetKey_lib();
							if(ucKey == KEYCANCEL)
								{
									portClose_lib(11);
									kbFlush_lib();
									return;
								}
						}
						sysDelayMs(50);
					}
				}
				break;
			}
		}

		//while(1)
		{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
					{
						portClose_lib(11);
						kbFlush_lib();
						return;
					}
					
				}
				sysDelayMs(50);
		}
		sysDelayMs(50);
   }
	sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port01\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
   iRet = portFlushBuf_lib(11);
   if(iRet != 0)
   {
		scrClrLine_lib(4,5);
		scrPrint_lib(0,4,2,"Clear flush fail");
		while(1)
		{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
					}
					sysDelayMs(50);
		}
	}
	sysDelayMs(1000);
	scrClrLine_lib(5,6);
  sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port02\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
   while(1)
   {
	    memset(SendBuf, 0x55, sizeof(SendBuf));
	    iRet = portSends_lib(11, SendBuf, 256);
		if(iRet != 0)
		{
			scrClrLine_lib(4,5);
			scrPrint_lib(0,4,2,"Send data fail");
			while(1)
			{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
				}
				sysDelayMs(50);
			}
		}
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port03\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		memset(RecvBuf, 0x00, sizeof(RecvBuf));
		//while(1)
		{
			//sysDelayMs(3000);
			iRet = portRecvs_lib(11, RecvBuf,256,5000);
			sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portRecvs iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet);
			if(iRet < 0)
			{
				scrClrLine_lib(4,5);
				scrPrint_lib(0,4,2,"Recvs data fail");
				while(1)
				{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
					}
					sysDelayMs(50);
				}
			}
			sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port04\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			 /*if(kbHit_lib()==0)
			{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)					 //取消键退出
					{		
						portClose_lib(11);
						kbFlush_lib();
						return;
					}
			}*/
		
		}
		
		 sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port04\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		 if(memcmp(RecvBuf, RecvBufCmp, 256) == 0)
			{
				counter++;
	     	 scrClrLine_lib(4,4);
			 scrPrint_lib(0,4,2,"Test OK Num=%d",counter);
			  scrClrLine_lib(5,5);
			  scrPrint_lib(0,5,0,"Test Fail Num=%d",  errorCounter);
				scrClrLine_lib(6,6);
				scrPrint_lib(0, 6, 2, "iRet=[%d]:%x %x %x %x %x", iRet, RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[254],RecvBuf[255]);
			}
			else{
				 errorCounter++;
			  scrClrLine_lib(4,4);
			  scrPrint_lib(0,4,2,"Test OK Num=%d",counter);
			  scrClrLine_lib(5,5);
			  scrPrint_lib(0,5,0,"Test Fail Num=%d",  errorCounter);
				scrClrLine_lib(6,6);
				scrPrint_lib(0, 6, 2, "iRet=[%d]:%x %x %x %x %x", iRet, RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[254],RecvBuf[255]);
			}
			/*scrClrLine_lib(5,5);
				counter++;
				scrPrint_lib(0,5,2,"Test counter=%d",counter);*/
		iRet = portFlushBuf_lib(11);
				if(iRet != 0)
				{
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,2,"Clear flush fail");
						while(1)
						{
									if(kbHit_lib()==0)                           //按任意键进入
									{
										ucKey = kbGetKey_lib();
										if(ucKey == KEYCANCEL)
										{
											portClose_lib(11);
											kbFlush_lib();
											return;
										}
									}
									sysDelayMs(50);
						}
					}
		 if(kbHit_lib()==0)
		{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)					 //取消键退出
				{		
					portClose_lib(11);
					kbFlush_lib();
					return;
				}
		}
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port06\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		 sysDelayMs(50);
   }
}

void Menu_COMRev(void)
{
	unsigned char ucKey = 0;
	int iRet = 0;
	unsigned char i=0, j = 0;
	unsigned char sendFlag  = 0;
	unsigned char QuitPrintFlag = 0;
	unsigned char SendBuf[512];
	unsigned char RecvBuf[512];
	unsigned char RecvBufCmp[512];
	unsigned int counter = 0;
	unsigned int errorCounter = 0;
	scrCls_lib();
	kbFlush_lib();
	scrPrint_lib(0, 1, 2, "PORT SERVER TEST");
	memset(SendBuf, 0x00, sizeof(SendBuf));
	memset(RecvBuf, 0x00, sizeof(RecvBuf));
	memset(RecvBufCmp, 0x55, sizeof(RecvBufCmp));
   iRet = portOpen_lib(11, "115200,8,n,1");
   if(iRet != 0)
   {
	   scrClrLine_lib(4,7);
	   scrPrint_lib(0,4,2,"Open port fail");
	   while(1)
	   {
		   if(kbHit_lib()==0)							//按任意键进入
		   {
			   ucKey = kbGetKey_lib();
			   if(ucKey == KEYCANCEL)
						{
							kbFlush_lib();
							return;
						}
		   }
		   sysDelayMs(50);
	   }
   }
   
   iRet = portFlushBuf_lib(11);
   if(iRet != 0)
   {
		scrClrLine_lib(4,5);
		scrPrint_lib(0,4,2,"Clear flush fail");
		while(1)
		{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
					}
					sysDelayMs(50);
		}
	}

   scrClrLine_lib(4,5);
   scrPrint_lib(0,4,2,"Waiting handshake...");
   while(1)
   {
		iRet = portRecvs_lib(11, RecvBuf,5,3000);
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portRecvs iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet);
		if(iRet < 0)
		{
			scrClrLine_lib(4,5);
			scrPrint_lib(0,4,2,"Recvs data fail");
			while(1)
			{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
				}
				sysDelayMs(50);
			}
		}
		scrClrLine_lib(5,6);
		scrPrint_lib(0, 5, 2, "iRet=[%d]:%x %x %x %x %x", iRet, RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[3],RecvBuf[4]);
		if((RecvBuf[0] == 0x10) && ((RecvBuf[1] == 0x05) || (RecvBuf[1] == 0x06)) && (RecvBuf[2] == 0x01))
		{
			if((RecvBuf[3] == RecvBuf[1] + RecvBuf[2]) && (RecvBuf[4] == 0x16))
			{
				if(RecvBuf[1] == 0x06)
					break;
				SendBuf[0] = 0x10;
				SendBuf[1] = 0x55;
				SendBuf[2] = 0x00;
				SendBuf[3] = SendBuf[1] + SendBuf[2];
				SendBuf[4] = 0x16;
				iRet = portSends_lib(11, SendBuf, 5);
				if(iRet != 0)
				{
					scrClrLine_lib(4,5);
					scrPrint_lib(0,4,2,"Send data fail");
					while(1)
					{
						if(kbHit_lib()==0)                           //按任意键进入
						{
							ucKey = kbGetKey_lib();
							if(ucKey == KEYCANCEL)
							{
								portClose_lib(11);
								kbFlush_lib();
								return;
							}
						}
						sysDelayMs(50);
					}
				}
			}
		}

		//while(1)
		{
				if(kbHit_lib()==0)                           //按任意键进入
				{
					ucKey = kbGetKey_lib();
					if(ucKey == KEYCANCEL)
					{
						portClose_lib(11);
						kbFlush_lib();
						return;
					}
					
				}
				//sysDelayMs(50);
		}
		sysDelayMs(50);
   }
  // sysDelayMs(400);
   iRet = portFlushBuf_lib(11);
				if(iRet != 0)
				{
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,2,"Clear flush fail");
						while(1)
						{
									if(kbHit_lib()==0)                           //按任意键进入
									{
										ucKey = kbGetKey_lib();
										if(ucKey == KEYCANCEL)
										{
											portClose_lib(11);
											kbFlush_lib();
											return;
										}
									}
									sysDelayMs(50);
						}
					}
	scrClrLine_lib(5,6);
  sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port01\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
  while(1)
  {
	//while(1)
	{
			memset(RecvBuf, 0x00, sizeof(RecvBuf));
			iRet = portRecvs_lib(11, RecvBuf,256,5000);
			sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portRecvs iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet);
			if(iRet < 0)
			{
				scrClrLine_lib(4,5);
				scrPrint_lib(0,4,2,"Recvs data fail");
				while(1)
				{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
						{
							portClose_lib(11);
							kbFlush_lib();
							return;
						}
						
					}
					sysDelayMs(50);
				}
			}
			/*sysDelayMs(50);
			if(iRet == 512)
			{
				break;
			}*/
	}
   sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port02\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
   if(memcmp(RecvBuf, RecvBufCmp, 256) == 0)
   {
	   		counter++;
	     	 scrClrLine_lib(4,4);
			 scrPrint_lib(0,4,2,"Test OK Num=%d",counter);
			  scrClrLine_lib(5,5);
			  scrPrint_lib(0,5,0,"Test Fail Num=%d",  errorCounter);
			 scrClrLine_lib(6,6);
			 scrPrint_lib(0, 6, 2, "iRet=[%d]:%x %x %x %x %x", iRet, RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[254],RecvBuf[255]);
	}
	else{
		       errorCounter++;
			    scrClrLine_lib(4,4);
			 scrPrint_lib(0,4,2,"Test OK Num=%d",counter);
			  scrClrLine_lib(5,5);
			  scrPrint_lib(0,5,0,"Test Fail Num=%d",  errorCounter);	
			  scrClrLine_lib(6,6);
				scrPrint_lib(0, 6, 2, "iRet=[%d]:%x %x %x %x %x", iRet, RecvBuf[0],RecvBuf[1],RecvBuf[2],RecvBuf[254],RecvBuf[255]);	 
				/*while(1)
				{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
							{
								portClose_lib(11);
								kbFlush_lib();
								return;
							}
					}
					sysDelayMs(50);
				}*/
	}
	        memset(SendBuf, RecvBuf[0]+1, sizeof(SendBuf));
			iRet = portSends_lib(11, SendBuf, 256);
			if(iRet != 0)
			{
				scrClrLine_lib(4,5);
				scrPrint_lib(0,4,2,"Send data fail");
				while(1)
				{
					if(kbHit_lib()==0)                           //按任意键进入
					{
						ucKey = kbGetKey_lib();
						if(ucKey == KEYCANCEL)
							{
								portClose_lib(11);
								kbFlush_lib();
								return;
							}
					}
					sysDelayMs(50);
				}
			}
			
	iRet = portFlushBuf_lib(11);
				if(iRet != 0)
				{
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,2,"Clear flush fail");
						while(1)
						{
									if(kbHit_lib()==0)                           //按任意键进入
									{
										ucKey = kbGetKey_lib();
										if(ucKey == KEYCANCEL)
										{
											portClose_lib(11);
											kbFlush_lib();
											return;
										}
									}
									sysDelayMs(50);
						}
					}
        
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port03\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		 if(kbHit_lib()==0)
		{
				ucKey = kbGetKey_lib();
				if(ucKey == KEYCANCEL)					 //取消键退出
				{		
					portClose_lib(11);
					kbFlush_lib();
					return;
				}
		}
		sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: port04\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		 sysDelayMs(50);
  }
}

void Menu_COMTest(void)
{
	int iSize = 0;

	STMENUNODE astMenu[] = {
				{ "串口测试", "PORT TEST", NoFun },
				{ "串口发送", "COM Send", Menu_COMSend},
				{ "串口接收", "COM Rev", Menu_COMRev},
			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;  
}


void Menu_ModChkCom(void)
{
	int iSize = 0;
	
	STMENUNODE astMenu[] = {
				{ "通信测试", "COMMNICATION TEST", NoFun },
				//{ "串口测试", "UART",        Menu_ModChkComUart},
				//{ "USB口测试","USB CDC", Menu_ModChkComCDC   },
                //{ "以太网",   "ETHERNET",Menu_ModEthernet},
				//{ "GPRS",               "GPRS   ", api_MenuWireless },
				//{ "拨号测试",           "DIAL   ", Menu_ModChkComDial },
				//{ "串口发送AT指令测试", "UART AT", Menu_ModChkComAT   },
                { "WIFI",               "WIFI   ", Menu_ModChkComWIFI},
                { "4G",                 "4G     ", Menu_ModChkCom4G},
//				{ "通讯端口", "port", Menu_ChkPort},
				{"串口测试",  "COMTest", Menu_COMTest},
				{ "蓝牙BLE", "BLE", Menu_BLEserver},
				//{ "USB开关", "USB_on-off", Menu_USBonoff},
//				{ "升级VOS", "Update VOS", Menu_ModUpdateVOS},

			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;
}

void Menu_ModChkIntMsr()
{
	unsigned char ucKey = 0xFF;
	scrCls_lib();
	scrPrint_Ex(0, 3, 2, "磁卡测试","MSR TEST...");
	scrPrint_lib(0, 4, 2,  "[Cancel]:EXIT");
	scrPrint_lib(0, 5, 2,  "[ANYKEY]:RETRY");
	kbFlush_lib();
	while (1)
	{
		scrClrLine_lib(2, 3);
		scrPrint_Ex(0, 2, 2, "开发中...", "developing...");
		ucKey = kbGetKey_lib();

		if (ucKey == KEYCANCEL)
		{
			return;
		}
	}
}
#if 0
int Menu_ModChkSamCard(unsigned int uiSamSlotNum)
{
	int iRet,i;
    uint8_t ATRData[5],ATROutPut[65];
    APDU_SEND_LIB ApduSend;
    APDU_RESP_LIB ApduRecv;

    
	memset(ATRData,0,sizeof(ATRData));
	memset(ATROutPut,0,sizeof(ATROutPut));
	scrClrLine_lib(1, 5);
	scrPrint_lib(0,1,0,"SAM SLOT:%d",uiSamSlotNum);
	    
    //api_samHardwareInit();

    iRet = iccPowerUp_lib(uiSamSlotNum,ATROutPut);
    if(0x00 == iRet)
    {
        scrPrint_lib(0,2,0,"ATR:");
        for(i=0;i<ATROutPut[0];i++)
        {
            scrPrintf_lib("%x",ATROutPut[i]);
        }
    }
    else
    {			
        scrPrint_lib(0,2,0,"Reset Fail%d",iRet);
    }

    memset(&ApduSend, 0, sizeof(ApduSend));
    memset(&ApduRecv, 0, sizeof(ApduRecv));

    memcpy(ApduSend.Command, "\x00\xa4\x04\x00", 4 );
    ApduSend.Le = 256;
    ApduSend.Lc = 14;
    memcpy( ApduSend.DataIn, "1PAY.SYS.DDF01", 14 );

    iRet = iccIsoCommand_lib(uiSamSlotNum,&ApduSend, &ApduRecv);
    if(iRet == 0)
    {
        scrPrint_lib(0,4,0,"APDU:%x,%x  ",ApduRecv.SWA,ApduRecv.SWB);
        for(i=0;i<ApduRecv.LenOut;i++)
        {
            scrPrintf_lib("%x",ApduRecv.DataOut[i]);
        }
    }
    else
    {			
        scrPrint_lib(0,4,0,"APDU:%d",iRet);
    }
    hal_keypadWaitOneKey();
    iccClose_lib(uiSamSlotNum);
    
    return iRet;
}
#endif

#if 1

void Menu_ModChkSAM(void)
{
	const char *MenuChina[] = {" [取消] . 重测"," [确认] . 继续"};
	const char *MenuEnglish[] = {" [X] . Again"," [o] . Continue"};
	char TitleChina[] = "PSAM 卡命令测试";
	char TitleEnglish[] = "PSAM Command Test";
	uchar Count = 0,BuffAtr[40];
	int Ret = 0;
	uchar ATR[65];
	
	APDU_SEND_LIB ApduSend;
	APDU_RESP_LIB ApduRecv;

	memset(ATR,0,sizeof(ATR));
	memset(BuffAtr  , 0, sizeof(BuffAtr));
	memset(ApduSend.DataIn,0,sizeof(ApduSend.DataIn));
	memset(ApduRecv.DataOut,0,sizeof(ApduRecv.DataOut));

	ApduSend.Cmd[0]=0x00;
	ApduSend.Cmd[1]=0xa4;
	ApduSend.Cmd[2]=0x04;
	ApduSend.Cmd[3]=0x00;
	ApduSend.Lc=strlen("1PAY.SYS.DDF01");
	ApduSend.Le=0;
	strcpy(ApduSend.DataIn,"1PAY.SYS.DDF01");
		
	scrCls_lib();
	
	scrPrint_Ex(0, 1, 2,TitleChina,TitleEnglish);

	while(1)
	{
		uchar G_SamNum = 0;
		scrClrLine_lib(2, 4);
		
		if((kbHit_lib()==0x00)&&(kbGetKey_lib()==KEYCANCEL))
			break;
		Ret = iccPowerUp_lib(0x01,ATR);
		if(Ret == 0)
		{
			Ret = iccIsoCommand_lib(0x01,&ApduSend,&ApduRecv);
			if(Ret<0)
			{
				scrPrint_lib(0, 3, 2, "卡座命令失败%d", Ret);
				continue;
			}
			else
			{
				scrPrint_Ex(0, 3, 2, "卡座命令成功","Slot Send Succ");
				scrPrint_lib(0, 4, 2, "SWA:%02x SWB:%02x",ApduRecv.SWA,ApduRecv.SWB);
				//Count ++;
			}
		}
		else
		{   
		    scrPrint_lib(0, 3, 2, "上电失败:%d",Ret); 
		}
		iccClose_lib(0x01);
		sysDelayMs_lib(2000);
		return;
	}
}

#else
void Menu_ModChkSAM(void)
{
	unsigned char ucKey = 0xFF;
	uint8_t slot = 1;
	    
	STMENUNODE astMenu[] = {
					{ "SAM卡测试", "SAM CARD TEST", NoFun },
					//{ "基本通讯流程", "BASIC COMM", NoFun },
					//{ "插卡检测", "DEC", NoFun },	
					//{ "通讯流程不下电", "COMM NO CLOSE", NoFun },
					//{ "下电", "CLOSE", NoFun },				
				};
	scrCls_lib();
	Menu_uiSetMenuTitle(&astMenu[0]);

	kbFlush_lib();
    while(1)
    {
		ucKey = kbGetKey_lib();
        if(ucKey == KEYENTER)
		{					// 按键: 确认键
		    if(slot != 2)
                slot = 1;
            else
                slot = 2;
			//Menu_ModChkSamCard(slot);
            slot++;
		}
		if(ucKey == KEYCANCEL)
		{					// 按键: 取消键
			break;
		}
    }
	return;
}
#endif

void Menu_cardFunc()
{
    int iSize = 0;
                       
    STMENUNODE astMenu[] = {
            { "卡类",  "Card test",  NoFun},
//            { "IC卡", "IC Card", Menu_ChkIC },
//            { "SAM", "SAM Card", Menu_ModChkSAM },       
//            { "磁卡", "MSR", Menu_ModChkIntMsr },
            { "PICC", "PICC Card", Menu_ModChkPICC },
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;

}
static unsigned char aasc_to_bcd(unsigned char ucAsc)
{
	unsigned char ucBcd = 0;

	if ((ucAsc >= '0') && (ucAsc <= '9'))
		ucBcd = ucAsc - '0';
	else if ((ucAsc >= 'A') && (ucAsc <= 'F'))
		ucBcd = ucAsc - 'A' + 10;
	else if ((ucAsc >= 'a') && (ucAsc <= 'f'))
		ucBcd = ucAsc - 'a' + 10;
	else if ((ucAsc > 0x39) && (ucAsc <= 0x3f))
		ucBcd = ucAsc - '0';
	else 
 		ucBcd = 0x0f;
	
	return ucBcd;
}

void AscToBcd(unsigned char  * sBcdBuf,     char  * sAscBuf, int iAscLen)
{
	int i, j;

	if((sBcdBuf == NULL) || (sAscBuf == NULL) || (iAscLen < 0))
		return;
	
	j = 0;

	for (i = 0; i < (iAscLen + 1) / 2; i++) 
	{
		sBcdBuf[i] = aasc_to_bcd(sAscBuf[j++]) << 4;
		sBcdBuf[i] |= (j >= iAscLen) ? 0x00 : aasc_to_bcd(sAscBuf[j++]);
	}
}


void GMSmAllTest(void)
{
	unsigned char ucKey = 0xff;
	int ret=0;
	scrCls_lib();
	scrPrint_Ex(0, 0, 2, "国密测试", "Test GM");
	unsigned char InputData[40],PulKey[200],PrvKey[200], OutData[200], user_id[20], msg[100], sign[200], Preresult[200], SmKey[100], vector[100];
	int DataLen = 16,OutPutLen = 0;
	int iRet = 0;
	gmSm2Init_lib(NULL);
	while (1)
	{
		memset(InputData,	0x00,sizeof(InputData));
		memset(PrvKey,	0x00, sizeof(PrvKey));
		memset(PulKey,	0x00, sizeof(PulKey));
		memset(OutData, 0x00, sizeof(OutData));
		memset(user_id, 0x00, sizeof(user_id));
		memset(msg, 0x00, sizeof(msg));
		memset(sign,	0x00, sizeof(sign));
	
		AscToBcd(InputData,"11111111111111111111111111111111",32);

		memset(OutData, 0x00, sizeof(OutData));
		iRet = gmSm2ExportPk_lib(2, OutData);
		memcpy(PulKey, OutData, 32);
		if(iRet != 0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM2导出公钥失败", "Sm2ExportPubk faild");
			sysDelayMs(150);
			break;
		}
		memset(OutData, 0x00, sizeof(OutData));
		iRet = gmSm2ExportPk_lib(3, OutData);
		memcpy(PulKey+32, OutData, 32);
	    if(iRet == 0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM2导出公钥成功", "Sm2ExportPubk Succeed");
			sysDelayMs(150);
		}
		else
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM2导出公钥失败", "Sm2ExportPubk faild");
			sysDelayMs(150);
			break;
		}
		memset(OutData, 0x00, sizeof(OutData));
		iRet = gmSm2ExportPk_lib(1, OutData);
		memcpy(PrvKey, PulKey, 64);
		memcpy(PrvKey + 64, OutData, 32);
	    if(iRet == 0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM2导出私钥成功", "Sm2ExportPrikey Succeed");
			sysDelayMs(150);
		}
		else
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM2导出私钥失败", "Sm2ExportPrikey faild");
			sysDelayMs(150);
			break;
		}
		iRet = gmSm2_lib(PulKey, 64, InputData, 16, OutData, &OutPutLen, 1);
		if(iRet == 0)
		{
			gmSm2_lib(PrvKey, 96, OutData, OutPutLen, OutData, &OutPutLen, 0);
			if(memcmp(OutData,InputData,16)==0)
			{
				scrClrLine_lib(4, 4);
				scrPrint_Ex(0, 4, 2, "SM2加解密成功", "GM SM2 Succeed");
				sysDelayMs(150);
			}
		}
	
		memcpy(user_id, "\x11",1);
		memcpy(msg, "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x12",32);
	
		iRet = gmSm2Sign_lib(user_id,  1, PulKey, PrvKey + 64, msg, 32, sign);
		iRet = gmSm2Verify_lib(user_id,  1, PulKey, sign, msg, 32);
		if(iRet == 0)
		{
			scrClrLine_lib(4, 4);
		    scrPrint_Ex(0, 4, 2, "SM2签名验签成功", "GM SM2 S&V Succeed");
			sysDelayMs(150);
		}
		else
			break;
	
		memset(InputData,	0x00,sizeof(InputData));
		memset(OutData, 0x00, sizeof(OutData));
		AscToBcd(InputData,"F1BC1DFF9B4C9268D5FD31FF72FD07040527267CC33F7BCFCAE366848F8FA7EF53F4324E2C375D6D3C49F8071F1729227D7D994DEE32293C8D6FFA6A10F73CAC", 128);	
		AscToBcd(Preresult,"700A86D08F86618902A10FBE2821888ECAF7632932DE0C72E9CADAE9AE901ACA",64);
		iRet = gmSm3_lib(InputData, 64, OutData);
		if(memcmp(OutData,Preresult,32)==0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM3算法成功", "GM SM3 Succeed");
			sysDelayMs(150);
		}
		else
			break;
	
		memset(InputData,	0x00,sizeof(InputData));
		memset(OutData, 0x00, sizeof(OutData));
		memset(SmKey,	0x00, sizeof(SmKey));
		memcpy(InputData,"\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef",16);
		memcpy(SmKey,"\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef",16);
		memcpy(Preresult,"\xF5\x7A\x93\x36\xB5\x07\xC3\x1C\xDD\xD3\x9A\x66\xAA\x99\xF6\xC2",16);

		iRet = gmSm4_lib(InputData, 16, OutData, SmKey, NULL, 1); 
		if(memcmp(OutData,Preresult,16)==0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM4加密成功", "GM SM4 Succeed");
			sysDelayMs(150);
		}
		else
			break;

		//0x00 ECB 解密
		iRet = gmSm4_lib(OutData, 16, OutData, SmKey, NULL, 0); 
		if(memcmp(OutData,InputData,16)==0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM4 ECB解密成功", "GM SM4 ECB Succeed");
			sysDelayMs(150);
		}
		else
			break;
	
		memset(InputData,	0x00,sizeof(InputData));
		memset(OutData, 0x00, sizeof(OutData));
		memset(SmKey,	0x00, sizeof(SmKey));
		memset(vector,	0x00, sizeof(vector));
	
		AscToBcd(InputData,"A3FBE4D1D76E6804EBC297FA25AAE007",32);
		AscToBcd(SmKey,"FE19B6CF10F034A70CE2FBE59B2F090E", 32);
		AscToBcd(Preresult,"9D90BCD83744333251330F0D281680C6",32);
		AscToBcd(vector,"62E521F2572F77EA5B0BBC1A36B42016",32);
	
		//0x03 CBC 加密
		iRet = gmSm4_lib(InputData, 16, OutData, SmKey, vector, 3); 
		if(memcmp(OutData,Preresult,16)==0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM4 CBC加密成功", "GM SM4 CBC Succeed");
			sysDelayMs(150);
		}
		else
			break;
		//0x02 CBC 解密
		iRet = gmSm4_lib(OutData, 16, OutData, SmKey, vector, 2); 
		if(memcmp(OutData,InputData,16)==0)
		{
			scrClrLine_lib(4, 4);
			scrPrint_Ex(0, 4, 2, "SM4 CBC解密成功", "GM SM4 CBC Succeed");
			sysDelayMs(150);
		}
		else
			break;

		sysDelayMs(150);
		scrClrLine_lib(3, 4);
		scrPrint_Ex(0, 4, 2, "国密芯片检测成功", "GM Test Succeed");
		while (1)
		{
			ucKey = kbGetKey_lib();
			if (ucKey == KEYCANCEL)
			{
				//api_gmClose();
				return;
			}
		}
	}
	scrClrLine_lib(3, 4);
	scrPrint_Ex(0, 4, 2, "国密芯片检测失败", "GM Test faild");
	while (1)
	{
		ucKey = kbGetKey_lib();
		if (ucKey == KEYCANCEL)
		{
			//api_gmClose();
			return;
		}
	}
}
#define GM_TEST_LEN 2*1024
void GMSm4Test(void)
{
	unsigned char ucKey = 0xff;
	int ret=0;
	unsigned char smkey[16];
	unsigned long long startTime,endtime;
	scrCls_lib();
	scrPrint_Ex(0, 1, 2, "国密sm4测试", "sm4 test");

	unsigned char *inputData = (uint8_t *)fibo_malloc(GM_TEST_LEN);
	unsigned char *outData = (uint8_t *)fibo_malloc(GM_TEST_LEN);
	while(1)
	{
		memset(inputData,0x11,GM_TEST_LEN);
		memset(outData,0x0,GM_TEST_LEN);
		memset(smkey,0x11,16);

		startTime = sysGetTicks_lib();
		ret = gmSm4_lib(inputData, GM_TEST_LEN/2, outData, smkey, NULL,1);			
		endtime = sysGetTicks_lib() - startTime;
		scrPrint_lib(0,2,0,"time1 is:%d ret=%d",endtime,ret);

		ret = gmSm4_lib(inputData+GM_TEST_LEN/2, GM_TEST_LEN/2, outData, smkey, NULL,1);	
		endtime = sysGetTicks_lib() - startTime;
		
		
		scrPrint_lib(0,3,0,"time2 is:%d ret=%d",endtime,ret);	
		
		scrPrint_lib(0,4,0,"[ok]-retest|[X]-exit");
		while (1)
		{
			ucKey = kbGetKey_lib();
			if (ucKey == KEYCANCEL)
			{
				fibo_free(inputData);
				fibo_free(outData);
				return;
			}
			if(ucKey == KEYENTER)
			{
				scrCls_lib();
				break;
			}
			sysDelayMs(200);
		}
		sysDelayMs(200);
	}
}

void Menu_ModChkGM()
{
    int iSize = 0;
                       
    STMENUNODE astMenu[] = {
            { "国密测试",  "GM test",      NoFun},
            { "SM All test", "SM ALL test", GMSmAllTest },
            { "SM4 test", "SM4 test", GMSm4Test },       

        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;

}


#define UNLOCKPAD_INPUTOK   0
#define UNLOCKPAD_INPUTERR  -1
#define UNLOCKPAD_TIMEOUT   -2
#define UNLOCKPAD_CANCEL    -3
#define UNLOCKPAD_FN    -4

void Menu_ModChkPed()
{
	int iSize = 0;
	
	STMENUNODE astMenu[] = {
				{ "安全服务类", "ped test", NoFun },
                //{ "文件系统", "FILE SYS", Menu_ModFILE},				
                { "国密", "GM", Menu_ModChkGM },
                //{ "解锁", "unlock", unlockPassWord },
			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;

}

#if 0

const char Test_Rootcert[]=
"-----BEGIN CERTIFICATE-----\r\n"
"MIIBYjCCAQmgAwIBAgIJALucIOuX/YsKMAoGCCqBHM9VAYN1MC4xCzAJBgNVBAYT\r\n"
"AkNOMRAwDgYDVQQIDAdCRUlKSU5HMQ0wCwYDVQQKDARDQ1RDMB4XDTIxMDUyNzA3\r\n"
"MjkxOVoXDTIzMDUyNzA3MjkxOVowLjELMAkGA1UEBhMCQ04xEDAOBgNVBAgMB0JF\r\n"
"SUpJTkcxDTALBgNVBAoMBENDVEMwWTATBgcqhkjOPQIBBggqgRzPVQGCLQNCAAS9\r\n"
"j3lAct5NmGJB32DinbJoswP3aFrS/OZf5IC7vqpMtumP9Y+35iiWjAXXuBat8CUt\r\n"
"MS9rZkcWuEQ3Z6SK/7qDoxAwDjAMBgNVHRMEBTADAQH/MAoGCCqBHM9VAYN1A0cA\r\n"
"MEQCIGUhLKuIoxCyg3Jk2v3nAZd4SAty7RSgr/lusKvZY1bSAiA0qIHX77rBcVUJ\r\n"
"/ofIEeAN2yu8Jz12KZxoIVjvEysKiw==\r\n"
"-----END CERTIFICATE-----\r\n";

const char baidu_ca_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIKLjCCCRagAwIBAgIMclh4Nm6fVugdQYhIMA0GCSqGSIb3DQEBCwUAMGYxCzAJ\r\n"
"BgNVBAYTAkJFMRkwFwYDVQQKExBHbG9iYWxTaWduIG52LXNhMTwwOgYDVQQDEzNH\r\n"
"bG9iYWxTaWduIE9yZ2FuaXphdGlvbiBWYWxpZGF0aW9uIENBIC0gU0hBMjU2IC0g\r\n"
"RzIwHhcNMjAwNDAyMDcwNDU4WhcNMjEwNzI2MDUzMTAyWjCBpzELMAkGA1UEBhMC\r\n"
"Q04xEDAOBgNVBAgTB2JlaWppbmcxEDAOBgNVBAcTB2JlaWppbmcxJTAjBgNVBAsT\r\n"
"HHNlcnZpY2Ugb3BlcmF0aW9uIGRlcGFydG1lbnQxOTA3BgNVBAoTMEJlaWppbmcg\r\n"
"QmFpZHUgTmV0Y29tIFNjaWVuY2UgVGVjaG5vbG9neSBDby4sIEx0ZDESMBAGA1UE\r\n"
"AxMJYmFpZHUuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwamw\r\n"
"rkca0lfrHRUfblyy5PgLINvqAN8p/6RriSZLnyMv7FewirhGQCp+vNxaRZdPrUEO\r\n"
"vCCGSwxdVSFH4jE8V6fsmUfrRw1y18gWVHXv00URD0vOYHpGXCh0ro4bvthwZnuo\r\n"
"k0ko0qN2lFXefCfyD/eYDK2G2sau/Z/w2YEympfjIe4EkpbkeBHlxBAOEDF6Speg\r\n"
"68ebxNqJN6nDN9dWsX9Sx9kmCtavOBaxbftzebFoeQOQ64h7jEiRmFGlB5SGpXhG\r\n"
"eY9Ym+k1Wafxe1cxCpDPJM4NJOeSsmrp5pY3Crh8hy900lzoSwpfZhinQYbPJqYI\r\n"
"jqVJF5JTs5Glz1OwMQIDAQABo4IGmDCCBpQwDgYDVR0PAQH/BAQDAgWgMIGgBggr\r\n"
"BgEFBQcBAQSBkzCBkDBNBggrBgEFBQcwAoZBaHR0cDovL3NlY3VyZS5nbG9iYWxz\r\n"
"aWduLmNvbS9jYWNlcnQvZ3Nvcmdhbml6YXRpb252YWxzaGEyZzJyMS5jcnQwPwYI\r\n"
"KwYBBQUHMAGGM2h0dHA6Ly9vY3NwMi5nbG9iYWxzaWduLmNvbS9nc29yZ2FuaXph\r\n"
"dGlvbnZhbHNoYTJnMjBWBgNVHSAETzBNMEEGCSsGAQQBoDIBFDA0MDIGCCsGAQUF\r\n"
"BwIBFiZodHRwczovL3d3dy5nbG9iYWxzaWduLmNvbS9yZXBvc2l0b3J5LzAIBgZn\r\n"
"gQwBAgIwCQYDVR0TBAIwADBJBgNVHR8EQjBAMD6gPKA6hjhodHRwOi8vY3JsLmds\r\n"
"b2JhbHNpZ24uY29tL2dzL2dzb3JnYW5pemF0aW9udmFsc2hhMmcyLmNybDCCA04G\r\n"
"A1UdEQSCA0UwggNBggliYWlkdS5jb22CDGJhaWZ1YmFvLmNvbYIMd3d3LmJhaWR1\r\n"
"LmNughB3d3cuYmFpZHUuY29tLmNugg9tY3QueS5udW9taS5jb22CC2Fwb2xsby5h\r\n"
"dXRvggZkd3ouY26CCyouYmFpZHUuY29tgg4qLmJhaWZ1YmFvLmNvbYIRKi5iYWlk\r\n"
"dXN0YXRpYy5jb22CDiouYmRzdGF0aWMuY29tggsqLmJkaW1nLmNvbYIMKi5oYW8x\r\n"
"MjMuY29tggsqLm51b21pLmNvbYINKi5jaHVhbmtlLmNvbYINKi50cnVzdGdvLmNv\r\n"
"bYIPKi5iY2UuYmFpZHUuY29tghAqLmV5dW4uYmFpZHUuY29tgg8qLm1hcC5iYWlk\r\n"
"dS5jb22CDyoubWJkLmJhaWR1LmNvbYIRKi5mYW55aS5iYWlkdS5jb22CDiouYmFp\r\n"
"ZHViY2UuY29tggwqLm1pcGNkbi5jb22CECoubmV3cy5iYWlkdS5jb22CDiouYmFp\r\n"
"ZHVwY3MuY29tggwqLmFpcGFnZS5jb22CCyouYWlwYWdlLmNugg0qLmJjZWhvc3Qu\r\n"
"Y29tghAqLnNhZmUuYmFpZHUuY29tgg4qLmltLmJhaWR1LmNvbYISKi5iYWlkdWNv\r\n"
"bnRlbnQuY29tggsqLmRsbmVsLmNvbYILKi5kbG5lbC5vcmeCEiouZHVlcm9zLmJh\r\n"
"aWR1LmNvbYIOKi5zdS5iYWlkdS5jb22CCCouOTEuY29tghIqLmhhbzEyMy5iYWlk\r\n"
"dS5jb22CDSouYXBvbGxvLmF1dG+CEioueHVlc2h1LmJhaWR1LmNvbYIRKi5iai5i\r\n"
"YWlkdWJjZS5jb22CESouZ3ouYmFpZHViY2UuY29tgg4qLnNtYXJ0YXBwcy5jboIN\r\n"
"Ki5iZHRqcmN2LmNvbYIMKi5oYW8yMjIuY29tggwqLmhhb2thbi5jb22CDyoucGFl\r\n"
"LmJhaWR1LmNvbYIRKi52ZC5iZHN0YXRpYy5jb22CEmNsaWNrLmhtLmJhaWR1LmNv\r\n"
"bYIQbG9nLmhtLmJhaWR1LmNvbYIQY20ucG9zLmJhaWR1LmNvbYIQd24ucG9zLmJh\r\n"
"aWR1LmNvbYIUdXBkYXRlLnBhbi5iYWlkdS5jb20wHQYDVR0lBBYwFAYIKwYBBQUH\r\n"
"AwEGCCsGAQUFBwMCMB8GA1UdIwQYMBaAFJbeYfG9HBYpUxzAzH07gwBA5hp8MB0G\r\n"
"A1UdDgQWBBSeyXnX6VurihbMMo7GmeafIEI1hzCCAX4GCisGAQQB1nkCBAIEggFu\r\n"
"BIIBagFoAHYAXNxDkv7mq0VEsV6a1FbmEDf71fpH3KFzlLJe5vbHDsoAAAFxObU8\r\n"
"ugAABAMARzBFAiBphmgxIbNZXaPWiUqXRWYLaRST38KecoekKIof5fXmsgIhAMkZ\r\n"
"tF8XyKCu/nZll1e9vIlKbW8RrUr/74HpmScVRRsBAHYAb1N2rDHwMRnYmQCkURX/\r\n"
"dxUcEdkCwQApBo2yCJo32RMAAAFxObU85AAABAMARzBFAiBURWwwTgXZ+9IV3mhm\r\n"
"E0EOzbg901DLRszbLIpafDY/XgIhALsvEGqbBVrpGxhKoTVlz7+GWom8SrfUeHcn\r\n"
"4+9Dn7xGAHYA9lyUL9F3MCIUVBgIMJRWjuNNExkzv98MLyALzE7xZOMAAAFxObU8\r\n"
"qwAABAMARzBFAiBFBYPxKEdhlf6bqbwxQY7tskgdoFulPxPmdrzS5tNpPwIhAKnK\r\n"
"qwzch98lINQYzLAV52+C8GXZPXFZNfhfpM4tQ6xbMA0GCSqGSIb3DQEBCwUAA4IB\r\n"
"AQC83ALQ2d6MxeLZ/k3vutEiizRCWYSSMYLVCrxANdsGshNuyM8B8V/A57c0Nzqo\r\n"
"CPKfMtX5IICfv9P/bUecdtHL8cfx24MzN+U/GKcA4r3a/k8pRVeHeF9ThQ2zo1xj\r\n"
"k/7gJl75koztdqNfOeYiBTbFMnPQzVGqyMMfqKxbJrfZlGAIgYHT9bd6T985IVgz\r\n"
"tRVjAoy4IurZenTsWkG7PafJ4kAh6jQaSu1zYEbHljuZ5PXlkhPO9DwW1WIPug6Z\r\n"
"rlylLTTYmlW3WETOATi70HYsZN6NACuZ4t1hEO3AsF7lqjdA2HwTN10FX2HuaUvf\r\n"
"5OzP+PKupV9VKw8x8mQKU6vr\r\n"
"-----END CERTIFICATE-----\r\n";

const char xj_ca_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIGBTCCBO2gAwIBAgIKFjaGEAAAAAALczANBgkqhkiG9w0BAQUFADBOMRMwEQYK\r\n"
"CZImiZPyLGQBGRYDbG9jMRIwEAYKCZImiZPyLGQBGRYCeGoxEjAQBgoJkiaJk/Is\r\n"
"ZAEZFgJpczEPMA0GA1UEAxMGWEpJU0NBMB4XDTEzMDUzMTAxMjgxNVoXDTE0MDUz\r\n"
"MTAxMjgxNVowYDETMBEGCgmSJomT8ixkARkWA2xvYzESMBAGCgmSJomT8ixkARkW\r\n"
"AnhqMRIwEAYKCZImiZPyLGQBGRYCaXMxEjAQBgNVBAsMCWRlcGFydF9pczENMAsG\r\n"
"A1UEAxMEdGVzdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANSAExd3\r\n"
"MXZdFrmadqVKILY89b76b1+T4UbkcQvUVdRp3x6Ai5Q7/NUdbALDaPEnCFdWYCqv\r\n"
"bDryGLJPPsVWeL0zWoaPBuSUHejEkUq5vrFSHDVIy+iQDq6DiGrh6Z7zjVmIGCR5\r\n"
"WeTUtsXdhGMKjKjeWqPBOY1tUT6myLeF26qYSfrp9A292gj2LpHAVxJT43Qbr5zW\r\n"
"HukPb1GvXnAz6ODL+U8vkBSqo3FzvT+I/tmUFK921dboImJcJynJY6PzYczGEQ1y\r\n"
"Emhf+umlLOPcW3xBvzU7G12F8dvwEhdRaiwpsSktnMapccvqN/nPFFNT5j45zYNf\r\n"
"M8jZujSSRFfWppUCAwEAAaOCAtEwggLNMAsGA1UdDwQEAwIFoDAdBgNVHQ4EFgQU\r\n"
"LVst51oE+PH1f+nC+hWVNkj9zaAwKwYJKwYBBAGCNxQCBB4eHABTAG0AYQByAHQA\r\n"
"YwBhAHIAZABMAG8AZwBvAG4wHwYDVR0jBBgwFoAUK5Oa9Rplhjex4sWsBreGGmRM\r\n"
"it4wgfcGA1UdHwSB7zCB7DCB6aCB5qCB44aBr2xkYXA6Ly8vQ049WEpJU0NBLENO\r\n"
"PXNlcnZlcjAxLENOPUNEUCxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1T\r\n"
"ZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPWlzLERDPXhqLERDPWxvYz9jZXJ0\r\n"
"aWZpY2F0ZVJldm9jYXRpb25MaXN0P2Jhc2U/b2JqZWN0Q2xhc3M9Y1JMRGlzdHJp\r\n"
"YnV0aW9uUG9pbnSGL2h0dHA6Ly9zZXJ2ZXIwMS5pcy54ai5sb2MvQ2VydEVucm9s\r\n"
"bC9YSklTQ0EuY3JsMIIBCQYIKwYBBQUHAQEEgfwwgfkwgaYGCCsGAQUFBzAChoGZ\r\n"
"bGRhcDovLy9DTj1YSklTQ0EsQ049QUlBLENOPVB1YmxpYyUyMEtleSUyMFNlcnZp\r\n"
"Y2VzLENOPVNlcnZpY2VzLENOPUNvbmZpZ3VyYXRpb24sREM9aXMsREM9eGosREM9\r\n"
"bG9jP2NBQ2VydGlmaWNhdGU/YmFzZT9vYmplY3RDbGFzcz1jZXJ0aWZpY2F0aW9u\r\n"
"QXV0aG9yaXR5ME4GCCsGAQUFBzAChkJodHRwOi8vc2VydmVyMDEuaXMueGoubG9j\r\n"
"L0NlcnRFbnJvbGwvc2VydmVyMDEuaXMueGoubG9jX1hKSVNDQS5jcnQwHwYDVR0l\r\n"
"BBgwFgYIKwYBBQUHAwIGCisGAQQBgjcUAgIwKQYDVR0RBCIwIKAeBgorBgEEAYI3\r\n"
"FAIDoBAMDnRlc3RAaXMueGoubG9jMA0GCSqGSIb3DQEBBQUAA4IBAQA3jE83C0wT\r\n"
"ZDXgfzk182KbT7+1SEfYCIwzDqkc+kAlgTqizWKzGWDEf1QiXA/RRM1FIR5O+4jU\r\n"
"8CrOJwegpQZnCMXuhCoyXpadNwC6uhSSeXyt9TW2XsUQq/JBpA80oHsJxBrOAuo2\r\n"
"PjSBNYJR7et/+gqpjTMXTGoqkVM9rb++ugS1N+VkzEBvcdq2A5nYzxzdt7wELa7A\r\n"
"JkES4HG7X4yreZ6yIfpmCtq56QJfQyXH/5RoNQmekpkTEGIhUVuH5hIyeEKoSX+m\r\n"
"ghd5XOeBDH7u+5lbi8eKsYrV/wp6PRNwCtFVD3b2qsU85ujXHsqhL3Bq9KsHuW+c\r\n"
"byxfykt5oI//\r\n"
"-----END CERTIFICATE-----\r\n";

#include "mbedtls\x509_crt.h"
const char Test_Cacert[]=
"-----BEGIN CERTIFICATE-----\r\n"
"MIIBxTCCAWygAwIBAgIIUAAAAUFVNzIwCgYIKoEcz1UBg3UwPDELMAkGA1UEBhMC\r\n"
"Q04xEjAQBgNVBAoMCUlDQkNfdGVzdDEZMBcGA1UEAwwQSUNCQ19ERUNQX3Rlc3RD\r\n"
"QTAeFw0yMTA3MDcwODU0MjNaFw0yNDA3MDYwODU1MTNaMDoxEDAOBgNVBAoMB1ZB\r\n"
"U1RPTkUxCzAJBgNVBAYTAkNOMRkwFwYDVQQDDBAwMDIxNjQyNzY4OTA1MDE0MFkw\r\n"
"EwYHKoZIzj0CAQYIKoEcz1UBgi0DQgAE7Rf278oSXg7DtKEoBQuXQLupvRmwcLkD\r\n"
"4asYmQmPyRn+6lLG1uR0MvadubT+LMSgZZkgtzVfIVxCOiO6wT3oMKNaMFgwCQYD\r\n"
"VR0TBAIwADALBgNVHQ8EBAMCBsAwHwYDVR0jBBgwFoAUbRZRFAMv28HYH01/BhAF\r\n"
"sC4DQ88wHQYDVR0OBBYEFBXEzycINEkCMnnwFQ9jm/R73RVJMAoGCCqBHM9VAYN1\r\n"
"A0cAMEQCIA97gIegZjc0zhQtdaRQEtEflL9Z8/iDqnMTsNjEa06GAiAHdYQ6P151\r\n"
"KteI4kl8gKebKjSwmcjQ1iBoRwfzVQYe7g==\r\n"
"-----END CERTIFICATE-----\r\n";

int CertParseTest(void)
{
	int iRet=0;
	scrCls_lib();
	scrPrint_Ex(0, 1, 2, "证书解析", "CertParseTest");

	mbedtls_x509_crt Cacert;
	char buff[4096] = {0};
	
	mbedtls_x509_crt_init(&Cacert);
	iRet = mbedtls_x509_crt_parse(&Cacert, (unsigned char*)Test_Cacert, sizeof(Test_Cacert));
	if(iRet != 0)
	{
	    sysLOG_lib(API_LOG_LEVEL_1, "[%s] -%s- Line=%d:  mbedtls_x509_crt_parse err %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	    goto RET_END;
	}
    sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:  version = %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, Cacert.version);
	iRet = mbedtls_x509_crt_info(buff, sizeof(buff)-1, "    ", &Cacert);
	if(iRet < 0)
	{
	    sysLOG_lib(API_LOG_LEVEL_1, "[%s] -%s- Line=%d:  mbedtls_x509_crt_info err %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
	    goto RET_END;
	}
	sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:  buff = %s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, buff);

	iRet = TRUE;
RET_END:
    mbedtls_x509_crt_free(&Cacert);
	return iRet;
	
}

void Menu_ModChkCert()
{
    int iSize = 0;
                       
    STMENUNODE astMenu[] = {
            { "证书测试",  "Cert test",      NoFun},
            { "证书解析", "CertParseTest", CertParseTest },

        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;

}
#endif
void Menu_ModChkDCEP()
{
	int iSize = 0;
	
	STMENUNODE astMenu[] = {
				{ "数字货币", "DCEP test", NoFun },
                //{ "证书", "Cert", Menu_ModChkCert },
                //{ "解锁", "unlock", unlockPassWord },
				//{"接口测试", "FUN Test", DCEPWalltest},
			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;

}

void logofile_write()
{
	int fd;
	int iRet;
	char filename[32];
	scrCls_lib();
	scrPrint_lib(0, 1, 2, "press to writefile");


	memset(filename,0,sizeof(filename));
	strcpy(filename,"/FFS/update_00.bin");
	fd = fileOpen_lib(filename,O_CREAT|O_RDWR);
	if(fd < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> open File:%s Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, filename,iRet);
		return FALSE;
	}
	iRet = fileSeek_lib(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Seek File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	
	iRet = fileWrite_lib(fd, update_00, 518);
	if(iRet != (518))
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Write File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}

	iRet = fileClose_lib(fd);
	if(iRet != 0)
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Close File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	scrPrint_lib(0, 2, 2, "write:%s succ",filename);


	memset(filename,0,sizeof(filename));
	strcpy(filename,"/FFS/update_01.bin");
	fd = fileOpen_lib(filename,O_CREAT|O_RDWR);
	if(fd < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> open File:%s Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, filename,iRet);
		return FALSE;
	}
	iRet = fileSeek_lib(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Seek File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	
	iRet = fileWrite_lib(fd, update_01, 518);
	if(iRet != (518))
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Write File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}

	iRet = fileClose_lib(fd);
	if(iRet != 0)
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Close File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	scrPrint_lib(0, 3, 2, "write:%s succ",filename);

	memset(filename,0,sizeof(filename));
	strcpy(filename,"/FFS/update_02.bin");
	fd = fileOpen_lib(filename,O_CREAT|O_RDWR);
	if(fd < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> open File:%s Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, filename,iRet);
		return FALSE;
	}
	iRet = fileSeek_lib(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Seek File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	
	iRet = fileWrite_lib(fd, update_02, 518);
	if(iRet != (518))
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Write File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}

	iRet = fileClose_lib(fd);
	if(iRet != 0)
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Close File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	scrPrint_lib(0, 4, 2, "write:%s succ",filename);

	memset(filename,0,sizeof(filename));
	strcpy(filename,"/FFS/update_03.bin");
	fd = fileOpen_lib(filename,O_CREAT|O_RDWR);
	if(fd < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> open File:%s Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, filename,iRet);
		return FALSE;
	}
	iRet = fileSeek_lib(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Seek File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	
	iRet = fileWrite_lib(fd, update_03, 518);
	if(iRet != (518))
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Write File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}

	iRet = fileClose_lib(fd);
	if(iRet != 0)
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Close File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	scrPrint_lib(0, 5, 2, "write:%s succ",filename);

	memset(filename,0,sizeof(filename));
	strcpy(filename,"/FFS/update_04.bin");
	fd = fileOpen_lib(filename,O_CREAT|O_RDWR);
	if(fd < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> open File:%s Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, filename,iRet);
		return FALSE;
	}
	iRet = fileSeek_lib(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Seek File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	
	iRet = fileWrite_lib(fd, update_04, 518);
	if(iRet != (518))
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Write File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}

	iRet = fileClose_lib(fd);
	if(iRet != 0)
	{
	
		sysLOG_lib(PWR_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR> Close File Fail %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return FALSE;
	}
	scrPrint_lib(0, 6, 2, "write:%s succ",filename);
	
	hal_keypadWaitOneKey();	
	hal_scrWriteLogo(0,32,(uint8_t *)update_00);
	hal_keypadWaitOneKey();	
	hal_scrWriteLogo(0,32,(uint8_t *)update_01);
	hal_keypadWaitOneKey();	
	hal_scrWriteLogo(0,32,(uint8_t *)update_02);
	hal_keypadWaitOneKey();	
	hal_scrWriteLogo(0,32,(uint8_t *)update_03);
	hal_keypadWaitOneKey();	
	hal_scrWriteLogo(0,32,(uint8_t *)update_04);
	hal_keypadWaitOneKey();	

	
}

void Menu_ModupdateT()
{
	int iSize = 0;
	
	STMENUNODE astMenu[] = {
				{ "安全服务类", "ped test", NoFun },
              	
                { "升级进度文件写入", "infoimage write", logofile_write },
                //{ "MaxfileWrite", "MaxfileWrite", MaxfileWrite },
                //{ "解锁", "unlock", unlockPassWord },
			};
	
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;

}

void Menu_writeTusn()
{
	int iRet;
	int len;
	char tusn[]={"TUSN0000140600054000080"};
	unsigned char data[32];
	unsigned char keyVal;
	scrCls_lib();
	scrPrint_lib(0,1,0,"写入tusn测试\n");	
	len = strlen(tusn);

	while(1)
	{
		keyVal = kbGetKey_lib();
		if(keyVal ==  KEYENTER)
		{
			scrClrLine_lib(2,4);
			tusn[len-1]+=1;
			scrPrint_lib(0,2,0,"w:%s",tusn);
			iRet = sysWriteTUSN_lib(tusn,sizeof(tusn));
			scrPrint_lib(0,3,0,"iRet = %d",iRet);
			memset(data,0,sizeof(data));
			iRet = sysReadTUSN_lib(0x0055FFAA,data);
			scrPrint_lib(0,4,0,"r:%s",data);
			
		}
		if(keyVal == KEYCANCEL)
			return;
		sysDelayMs(1000);
	}
	
	

}
void Menu_ModuleCheck()
{
    int iSize = 0; 
    sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_ModuleCheck into\r\n", filename(__FILE__), __FUNCTION__, __LINE__);	   
	STMENUNODE astMenu[] = {
			{ "模块检测", "TEST MODULE", NoFun },
			{ "基础类",   "Basic test",  Menu_basicFunc},
			{ "卡类",     "Card test",  Menu_cardFunc},
			{ "通讯类",   "Communicate test",  Menu_ModChkCom},
            //{ "test write","test write",  Menu_writeTusn},
            //{ "升级测试", "update test",  Menu_ModupdateT},
            //{ "数字货币","DCEP test",  Menu_ModChkDCEP},
	};            
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_LocalDownload iSize=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSize);
    Menu_ShowStandardMenu(astMenu, iSize);
    sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_LocalDownload\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
    return;
}

void DownLoadVOS(void)
{
	unsigned char ucKey = 0xFF;
	int iRet;
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: DownLoadVOS\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	scrCls_lib();
	scrPrint_lib(1,2,2,"DownLoad VOS\r\n...\r\n");
	scrPrint_lib(1,4,2,"Please restart POS after PC tool shows success.");
	g_ullStartTime = 0;
	g_ullEndTime = 0;
	//fibo_mutex_lock(se_comm_mutex);//lock
	
	hal_portUsbOpen();
	first_find_Q = 0;

    kbFlush_lib();
	while(1)
	{  
    	 
    	
	    sysDelayMs_lib(10);
		hal_portUsbSEHandle();
        
	    ucKey = kbGetKey_lib();
		
	    if(ucKey == KEYCANCEL)
	    {                   // 按键: 取消键
	    	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:ucKey == KEYCANCEL\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			hal_portUsbClose();
			//hal_utSEUartInit(DEFAULT_RATE);
			//config_se_callBack_Ctrl();
			//fibo_mutex_unlock(se_comm_mutex);//unlock
			//DownLoadVOSFlag = 0;
	        break;
	    }

		if(DownLoadVOSFlag == 1)
		{
			g_ullEndTime = hal_sysGetTickms();
		}
		
		if((g_ullEndTime - g_ullStartTime) > 1500)
	    {                   // 按键: 取消键
	    	//sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:ucKey == KEYCANCEL\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			se_reboot();
			hal_utSEUartInit(DEFAULT_RATE);
			config_se_callBack_Ctrl();
			fibo_mutex_unlock(se_comm_mutex);//unlock
			hal_portUsbClose();
			DownLoadVOSFlag = 0;
			sysDelayMs_lib(2500);
			break;
	        //break;
	    }
	}
}
void commitVOS(void)
{
	unsigned char ucKey = 0xFF;
	int iRet;
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: commitVOS\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	scrCls_lib();
	scrPrint_lib(1,2,2,"DownLoad VOS or SN\r\n...\r\n");
	scrPrint_lib(1,4,2,"Please restart POS after PC tool shows success.");

	hal_utSEUartInit(DEFAULT_RATE);//DOWNLOAD_RATE);

	hal_portUsbOpen();
	
	kbFlush_lib();
	while(1)
	{  
    	 
    	
	    sysDelayMs_lib(10);
        hal_portUsbSECommitHandle();
		
	    ucKey = kbGetKey_lib();
	    
	    // 
	    if(ucKey >= '1' && ucKey <= '0')
	    {
	    
	    }

	    if(ucKey == KEYCANCEL)
	    {                   // 
	    	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:ucKey == KEYCANCEL\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			hal_portUsbClose();
			hal_utSEUartInit(DEFAULT_RATE);
	        break;
	    }
	}
}

void DownLoadAPP(void)
{
	unsigned char ucKey = 0xFF;
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: DownLoadAPP\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	scrCls_lib();
	scrPrint_lib(1,2,2,"DownLoad APP/FONT/TUNS\r\n...\r\n");
	scrPrint_lib(1,4,2,"Wait a moment please.");
	hal_portUsbOpen();
	kbFlush_lib();

	clear_LoadStep();
	
  	while(1)
	{  
    	 
    	
	    sysDelayMs_lib(10);
		
		hal_ldProcessHandle();
        
	    ucKey = kbGetKey_lib();
	    
	    // 数字键选定功能
	    if(ucKey >= '1' && ucKey <= '0')
	    {
	    
	    }

	    if(ucKey == KEYCANCEL)
	    {                   // 按键: 取消键
	    	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:ucKey == KEYCANCEL\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	    	hal_portUsbClose();
	        break;
	    }
	}
	return;
}

int Menu_ShowStandardMenu(STMENUNODE astMenu[], int iSize)
{
	int iLoop = 0;
    int iTotLine = 0, iCurPage = 0;
    int iTotPage = 0, iCurPageMaxItem = 0;
    unsigned char ucKey = 0xFF;
	unsigned char ucItemNum=0x00;
	unsigned char ucStartline = 0x00;
	unsigned char ucMenuItemMaxLine = 0x00;
	unsigned char ucMenuTitleLine = 0x00;

	iTotLine = iSize - 1;//remove title
	if(iTotLine == 0)
	{
		//return 0;
	}
	ucMenuItemMaxLine = g_uiMaxLine -1;

	
	iTotPage = (iTotLine - 1) / ucMenuItemMaxLine + 1;
    sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_ShowStandardMenu iTotPage=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iTotPage);
	kbFlush_lib();
	while(1)
	{
        scrCls_lib();
        Menu_uiSetMenuTitle(&astMenu[0]);
        ucStartline = 0x01;
        ucMenuTitleLine = 0x00;     
		if(iTotLine - iCurPage*ucMenuItemMaxLine <= ucMenuItemMaxLine)
		{
			iCurPageMaxItem = iTotLine - iCurPage*ucMenuItemMaxLine;
		}
		else
		{
			iCurPageMaxItem  = ucMenuItemMaxLine;
		}
		for(iLoop = 1; iLoop <= iCurPageMaxItem; iLoop++)
		{
			ucItemNum++;
			Menu_uiSetMenuItem(iLoop - ucMenuTitleLine , ucItemNum, &astMenu[iCurPage*ucMenuItemMaxLine + iLoop]);
		}
		sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_ShowStandardMenu while(2)\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		while(2)
		{
			ucKey = kbGetKey_lib();
			
			// 数字键选定功能
			if(ucKey >= '1' && ucKey <= '0' + iTotLine)
			{
				astMenu[ucKey - '1' + 1].pFun();
				kbFlush_lib();
				ucItemNum-=iCurPageMaxItem;
				break;
			}

			if(iTotPage >= 2 && ucKey == KEYENTER)
			{					// 按键: 确认键
				iCurPage += 1;
				
				if(iCurPage >= iTotPage)
				{
					ucItemNum = 0;
					iCurPage = 0;
				}
					
				break;
			}

			if(ucKey == KEYCANCEL)
			{					// 按键: 取消键
				return KEYCANCEL;
			}
		}
	}

	return 0;
}



//==================二级主菜单======================//

void Menu_LocalDownload(void)
{
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_LocalDownload into\r\n", filename(__FILE__), __FUNCTION__, __LINE__);

#if 1
    int iSize = 0;
	unsigned char ucKey = 0xFF;
	
    STMENUNODE astMenu[] = {
        { "本地下载", "Local Download"  , NoFun },
        { "下载VOS",   "Download VOS"  ,    DownLoadVOS},
		{ "下载应用/字库/TUSN", "DownLoad APP/FONT/TUSN",       DownLoadAPP},
		//{ "se",   "commit se"  ,    commitVOS},
    };
	//SetMiddleSizeFont();
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_LocalDownload iSize=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iSize);
	Menu_ShowStandardMenu(astMenu, iSize);
    //SetLargeSizeFont();
#endif
    sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d: Menu_LocalDownload\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
	return;
}

int Menu_ShowHiddenMenu(STMENUNODE astMenu[], int iSize,int hidden_size)
{
	int iLoop = 0;
    int iTotLine = 0, iCurPage = 0;
    int iTotPage = 0, iCurPageMaxItem = 0;
    unsigned char ucKey = 0xFF;
	//STR_PASSWD  STR_PassWdInput = { 0, { 0 } };
	unsigned char ucItemNum=0x00;
	unsigned char ucStartline = 0x00;
	unsigned char ucMenuItemMaxLine = 0x00;
	unsigned char ucMenuTitleLine = 0x00;

	iTotLine = iSize - 1-hidden_size;//remove title
	if(iTotLine == 0)
	{
		//return 0;
	}

	ucMenuItemMaxLine = g_uiMaxLine -1;
	//if(g_ucLcdType == 0x01)
	//{
	//	g_uiMaxLine = 2;
	//	ucMenuItemMaxLine = g_uiMaxLine;
	//}
	
	iTotPage = (iTotLine - 1) / ucMenuItemMaxLine + 1;

	kbFlush_lib();
	while(1)
	{
		scrCls_lib();
		//if(g_ucLcdType == 0x01)
		{
		//	ucStartline = 0x00;
		//	ucMenuTitleLine = 0x01;
		}
		//else
		{
			Menu_uiSetMenuTitle(&astMenu[0]);
			ucStartline = 0x01;
            ucMenuTitleLine = 0x00;
		}

		if(iTotLine - iCurPage*ucMenuItemMaxLine <= ucMenuItemMaxLine)
		{
			iCurPageMaxItem = iTotLine - iCurPage*ucMenuItemMaxLine;
		}
		else
		{
			iCurPageMaxItem  = ucMenuItemMaxLine;
		}
		for(iLoop = 1; iLoop <= iCurPageMaxItem; iLoop++)
		{
			ucItemNum++;
			Menu_uiSetMenuItem(iLoop - ucMenuTitleLine , ucItemNum, &astMenu[iCurPage*ucMenuItemMaxLine + iLoop]);
		}
		
		while(2)
		{
			ucKey = kbGetKey_lib();
			
			// 数字键选定功能
			if(ucKey >= '1' && ucKey <= '0' + iTotLine)
			{
				astMenu[/*iCurPage*MENU_ITEM_MAX_LINE +*/ ucKey - '1' + 1].pFun();
				kbFlush_lib();
				ucItemNum-=iCurPageMaxItem;
				break;
			}

			/*if(ucKey == KEYCLEAR)
				break;	*/		// 按键: 清除键
			if(ucKey == KEYENTER)//if(iTotPage >= 2 && ucKey == KEYENTER)
			{
                if(iTotPage >= 2)// 按键: 确认键
				{
                    iCurPage += 1;
				    //ucMenuTitleLine = 0;
				    if(iCurPage >= iTotPage)
				    {
					    iCurPage = 0;
					    ucItemNum = 0;
					    //ucMenuTitleLine = 1;
				    }
                }
                else
                {
				    iCurPage = 0;
					ucItemNum = 0;                    
                }
#if 0					
				if(0 == cmpPasswd(&STR_PassWdInput, MENU_PASSWD))
                {
                	ucItemNum = 0;//-=iCurPageMaxItem;
                    iTotLine = iSize - 1;
					iTotPage = (iTotLine - 1) / ucMenuItemMaxLine + 1;
					iCurPage = 0;
					iCurPageMaxItem = 0;
                }
#endif
				break;
			}

			if(ucKey == KEYCANCEL)
			{					// 按键: 取消键
				return KEYCANCEL;
			}
			//revPasswd(ucKey, &STR_PassWdInput);
		}
	}

	return 0;
}
unsigned char  AsciiToHex(unsigned char *ucBuffer)
{
     unsigned char Temp[2]={0};
	 unsigned char ucTemp = 0x00;

	 if((ucBuffer[0] >= '0') && (ucBuffer[0] <= '9'))
	 {
           Temp[0] = ucBuffer[0] - 0x30;
	 }
     else if((ucBuffer[0] >= 'a') && (ucBuffer[0] <= 'z'))
	 {
		   Temp[0] = ucBuffer[0] - 0x61 + 0x0a;
	 }
	 else if((ucBuffer[0] >= 'A') && (ucBuffer[0] <= 'Z'))
	 {
           Temp[0] = ucBuffer[0] - 0x41 + 0x0a;
	 }

     if((ucBuffer[1] >= '0') && (ucBuffer[1] <= '9'))
	 {
           Temp[1] = ucBuffer[1] - 0x30;
	 }
     else if((ucBuffer[1] >= 'a') && (ucBuffer[1] <= 'z'))
	 {
		   Temp[1] = ucBuffer[1] - 0x61 + 0x0a;
	 }
	 else if((ucBuffer[1] >= 'A') && (ucBuffer[1] <= 'Z'))
	 {
           Temp[1] = ucBuffer[1] - 0x41 + 0x0a;
	 }

     ucTemp = (Temp[0] << 4) + Temp[1];
	 return (ucTemp);
}
unsigned char bcd_to_byte(unsigned char ucBcd)
{
	return (((ucBcd >> 4) & 0x0f) * 10 + (ucBcd & 0x0f));
}

long BcdToLong(unsigned char  *sBcdBuf, int iBcdLen)
{
	long 	lValue = 0;
	
	if ((iBcdLen <= 0) || (sBcdBuf == NULL))
		return 0;
		
	while (iBcdLen-- > 0)
		lValue = lValue * 100 + bcd_to_byte(*sBcdBuf++);
	return lValue;
}

void Menu_SysInfoSetTime(void)
{
	unsigned char ucKey = 0xFF,i=0;
	unsigned char aucTime[6] = {0};
	unsigned char str[36] = {0};
	unsigned char aucBcdTime[36] = {0},timeAsc[16]={0};
    int iRet = 0;
	
	STMENUNODE astMenu[] = {
				{ "设置时间", "SET TIME", NoFun },
			};
	scrCls_lib();

	Menu_uiSetMenuTitle(&astMenu[0]);
	kbFlush_lib();
	
	sysGetTime_lib(aucTime);
			

	timeAsc[0] = BcdToLong(&aucTime[0] , 1);
	timeAsc[1] = BcdToLong(&aucTime[1] , 1);
	timeAsc[2] = BcdToLong(&aucTime[2] , 1);
	timeAsc[3] = BcdToLong(&aucTime[3] , 1);
	timeAsc[4] = BcdToLong(&aucTime[4] , 1);
    timeAsc[5] = BcdToLong(&aucTime[5] , 1);

	scrPrint_Ex(0,2,0x00,"当前时间:\n","Current Time\n");
	scrPrint_lib(0,4,0x00,"20%02d-%02d-%02d %02d:%02d:%02d\n",
		timeAsc[0],timeAsc[1],timeAsc[2],timeAsc[3],timeAsc[4],timeAsc[5]);

	//scrClrLine_lib(2,3);
	scrPrint_lib(0,5,0x00,"YYMMDDHHMMSS:\n");
	iRet = kbGetString_lib(str,0x64, 1, 12, 120);
    aucBcdTime[0] = AsciiToHex(str+0);
    aucBcdTime[1] = AsciiToHex(str+2);
    aucBcdTime[2] = AsciiToHex(str+4);
    aucBcdTime[3] = AsciiToHex(str+6);
    aucBcdTime[4] = AsciiToHex(str+8);
    aucBcdTime[5] = AsciiToHex(str+10);
	
	iRet = sysSetTime_lib(aucBcdTime);
	
	sysGetTime_lib(aucTime);
	timeAsc[0] = BcdToLong(&aucTime[0] , 1);
	timeAsc[1] = BcdToLong(&aucTime[1] , 1);
	timeAsc[2] = BcdToLong(&aucTime[2] , 1);
	timeAsc[3] = BcdToLong(&aucTime[3] , 1);
	timeAsc[4] = BcdToLong(&aucTime[4] , 1);
    timeAsc[5] = BcdToLong(&aucTime[5] , 1);
	//api_kbWaitOneKey();
	scrClrLine_lib(2,4);
	scrPrint_Ex(0,2,0x00,"当前时间:\n","Current Time\n");
	scrPrint_lib(0,4,0x00,"20%02d-%02d-%02d %02d:%02d:%02d\n",timeAsc[0],timeAsc[1],timeAsc[2],timeAsc[3],timeAsc[4],timeAsc[5]);

	
	while(1)
	{
	    ucKey = kbGetKey_lib();

	    if(ucKey == KEYCANCEL)
	    {
	        break;          
	    }
	}
    
	return;
}

void Menu_SysInfoGetTime(void)
{
	unsigned char ucKey = 0xFF;
	unsigned char aucTime[6] = {0};
    unsigned char timeAsc[16]={0};
	
	STMENUNODE astMenu[] = {
				{ "显示时间", "GET TIME", NoFun },
			};
	scrCls_lib();

	Menu_uiSetMenuTitle(&astMenu[0]);
	scrPrint_Ex(0,2,0x00,"当前时间:\n","Current Time\n");

	kbFlush_lib();

  	while(1)
	{  
    	sysGetTime_lib(aucTime);
        timeAsc[0] = BcdToLong(&aucTime[0] , 1);
        timeAsc[1] = BcdToLong(&aucTime[1] , 1);
        timeAsc[2] = BcdToLong(&aucTime[2] , 1);
        timeAsc[3] = BcdToLong(&aucTime[3] , 1);
        timeAsc[4] = BcdToLong(&aucTime[4] , 1);
        timeAsc[5] = BcdToLong(&aucTime[5] , 1);
		scrPrint_lib(0,3,0x00,"20%02d-%02d-%02d %02d:%02d:%02d\n",
			timeAsc[0],timeAsc[1],timeAsc[2],timeAsc[3],timeAsc[4],timeAsc[5]);
	    sysDelayMs_lib(100);
	    ucKey = kbGetKey_lib();

	    if(ucKey == KEYCANCEL)
	    {
	        break;
	    }

	}
    
	return;
}

static void MenuBasicInfo(int type)
{
#define SHOW_GMV "V1.01"
#define SHOW_FVN "1.00"
#define SHOW_HVN "1.00"

    char strSN[32], strESN[64];
    unsigned char pszTmp[32];
    char cont_buf[21], TermName[16], Area;
	unsigned char cfg[10];
    unsigned char pn[32];
    int ret, ver_len = 21;
	char wifiVerBuf[256];
	char pcGmVersion[22];
   signed char Temp =0;
    memset(TermName, 0x00, sizeof(TermName));
	memset(cfg, 0x00, sizeof(cfg));

    //readCfgInfo("TERMINAL_NAME", TermName);
  
    memset(cont_buf, 0, sizeof(cont_buf));
    if(type == 1)
    {
        memset(strSN, 0, sizeof(strSN));
        memset(strESN, 0, sizeof(strESN));

	    sysReadSn_lib(0x0055FFAA, (unsigned char *)strSN);

		//strcpy((char *)pszTmp, PED_SW_VERSION);
	  
        //hal_scrPrint(0, 0, 0x02, "%s PED%s\r\n", TermName, pszTmp);
        if(strSN[0])
        {
			scrPrint_lib(2, 2, 0x00, "SN: %s \r\n", strSN);
        }
        else
        {
			scrPrint_lib(2, 2, 0x00, "SN: NULL \r\n");		
        }
		memset(strSN, 0, sizeof(strSN));

	    sysReadTUSN_lib(0x0055FFAA, (unsigned char *)strSN);
		if(strSN[0])
		{
			scrPrint_lib(2, 4, 0x00, "TUSN: %s \r\n", strSN);
		}
		else
		{
			scrPrint_lib(2, 4 ,0x00, "TUSN: NULL \r\n");		
		}
		
		memset(strSN, 0, sizeof(strSN));

	    hal_nvReadCustomerID((unsigned char *)strSN);
		if(strSN[0])
		{
			scrPrint_lib(2, 6, 0x00, "CID: %s \r\n", strSN);
		}
		else
		{
			scrPrint_lib(2, 6 ,0x00, "CID: NULL \r\n");		
		}
		scrPrint_lib(2, 7, 0x00, "press continue..");
    }
    else if(type == 2)
    {
        strcpy((char *)pszTmp, DR_VER);
		
        scrPrint_lib(2, 3, 0x00, "APP Lib:%s\r\n", pszTmp);
		memset(pszTmp, 0x00, sizeof(pszTmp));
		sysReadVerInfo_lib(2,pszTmp);
		scrPrintf_lib(" MAIN: %s\r\n", pszTmp);

		memset(pszTmp, 0x00, sizeof(pszTmp));
		sysReadBPVersion_lib(pszTmp);
		scrPrintf_lib(" BP: %s\r\n", pszTmp);

		scrPrintf_lib("\r\n");
		scrPrintf_lib(" press continue..\r\n");

    }
    else if(type == 3)
    {
		memset(pszTmp, 0x00, sizeof(pszTmp));
		sysReadVerInfo_lib(0, pszTmp);

		scrPrint_lib(2, 2, 0x00, "BOOT: %s\r\n", pszTmp);

		memset(pszTmp, 0x00, sizeof(pszTmp));
		sysReadVerInfo_lib(1,pszTmp);
	    scrPrint_lib(2, 3, 0x00, "VOS : %s\r\n", pszTmp);
		
		memset(pszTmp, 0x00, sizeof(pszTmp));
		sysReadBPVersion_lib(pszTmp);
		scrPrint_lib(2, 5, 0x00, "BP: %s\r\n", pszTmp);

		memset(wifiVerBuf, 0x00, sizeof(wifiVerBuf));
		ret = wifiGetUserVersion_lib(wifiVerBuf,sizeof(wifiVerBuf));
		if(ret > 0)
		{
			scrPrint_lib(2, 6, 0x00, "WIFI: %s", wifiVerBuf);
			scrPrint_lib(2, 7, 0x00, "press continue..");	
		}
		else 
		{
			scrPrint_lib(2, 6, 0x00, "press continue..");
		}
		
    }
	else if(type == 4)
    {
		memset(pcGmVersion, 0x00, sizeof(pcGmVersion));
		ret = sysReadGmVersion_lib(pcGmVersion);
		if(ret>0)
		{
			scrPrint_lib(2, 3, 0x00, "%s\r\n", pcGmVersion);
		}
		else
		{
			scrPrint_lib(2, 3, 0x00, "GMV : %s\r\n", SHOW_GMV);
		}
	    
		scrPrint_lib(2, 4, 0x00, "FVN : %s\r\n", SHOW_FVN);
		scrPrint_lib(2, 5, 0x00, "HVN : %s\r\n", SHOW_HVN);
		
		//scrPrint_lib(2, 7, 0x00, "press continue..");
		//memset(pcGmVersion, 0x00, sizeof(pcGmVersion));
        sysGetBattChargTemp_lib(&Temp);
        // i32Val = pcGmVersion[1];
        // i32Val += pcGmVersion[2]<<8;

        scrPrint_lib(2, 6, 0x00, "%d\r\n", Temp);
        // scrPrint_lib(2, 7, 0x00, "press continue..");
    }

    return;
}

static void MenuConfigInfo(void)
{
	char con_buf0[32] = {0};
    char con_buf1[32] = {0};
    char con_buf2[32] = {0};
	char wifiVerBuf[256];

	unsigned char out_info[30] = {0};
	sysGetTermInfo_lib (out_info);
	
    unsigned char ret = out_info[3];
    if(ret > 0)
    {
        memcpy(con_buf0, "TP: [Y]  ", 10);
    }
    else
    {
        memcpy(con_buf0, "TP: [N]  ", 10);
    }
    ret = 0;

    ret = out_info[4];
    if(ret > 0)
    {
        memcpy(con_buf1, "MAG: [Y]  ", 10);
    }
    else
    {
        memcpy(con_buf1, "MAG: [N]  ", 10);
    }
    ret = 0;
    ret = out_info[5];
    if(ret > 0)
    {
        memcpy(&con_buf1[11], "ICC: [Y]", 8);
    }
    else
    {
        memcpy(&con_buf1[11], "ICC: [N]", 8);
    }
    ret = 0;
    ret = out_info[6];
    if(ret > 0)
    {
        memcpy(con_buf2, "RF : [Y]  ", 10);
    }
    else
    {
        memcpy(con_buf2, "RF : [N]  ", 10);
    }
    ret = 0;
    ret = out_info[7];
    if(ret > 0)
    {
        memcpy(&con_buf2[11], "BT : [Y]", 8);
    }
    else
    {
        memcpy(&con_buf2[11], "BT : [N]", 8);
    }

	char out_type[20] = {0};
	ret = sysGetTermTypeImperInfo_lib(out_type);
	if(ret == 0)
	{
		scrPrint_lib(2, 2, 0x00, "Name:%s", out_type);
	}
	
	scrPrint_lib(2, 3, 0x00, "%s", con_buf0);
	scrPrint_lib(2, 4, 0x00, "%s", con_buf1);
	scrPrint_lib(2, 5, 0x00, "%s", con_buf2);
	
	memset(out_type, 0x00, 20);
	sysGetTermType_lib (out_type);
    scrPrint_lib(2, 6, 0x00, "POS Type: %s", out_type);
	
	//memset(out_type, 0x00, 20);
	//sysReadCid_lib(out_type);
	//scrPrint_lib(2, 7, 0x00, "CID: %s", out_type);

    return;
}

void Menu_SysInfoShowVer(void)
{
	int step_flag = 1;
	int step_flag_last = 0;
    unsigned char ucKey;
    int page_num = 6;

    unsigned char ucLen  = 0;//
    
    scrCls_lib();
	
    while(1)
    {
    	if(step_flag_last != step_flag)
    	{
	        switch(step_flag)
	        {
	        case 1:
	            MenuBasicInfo(1);
	            break;
	        case 2:
	            MenuBasicInfo(2);
	            break;
			case 3:
	            MenuBasicInfo(3);
	            break;

	        case 4:
	            MenuConfigInfo();
	            break;
			case 5:
	            MenuBasicInfo(4);
	            break;
	        default:
	            break;

	        }
			step_flag_last = step_flag;
    	}

        ucKey = kbGetKey_lib();

        if(ucKey == KEYENTER)
        {
        	step_flag++;       
            if(step_flag <= (page_num - 1))
            {
				scrCls_lib();
                
            }
            else
            {
				scrCls_lib();
                step_flag = 1;
            }
        } 
              
        if(ucKey == KEYCANCEL)
        {
            break;
        }

        if(ucKey == KEYCLEAR)//
        {
            continue;
        }

 		sysDelayMs(50);
    }
}

/*
*@Brief:		格式化 PED 
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int Menu_pedFormat(void)
{
	unsigned char ucKey = 0xFF;
	int iRet = PED_RET_ERROR;
	
	scrCls_lib();
	kbFlush_lib();

	scrPrint_Ex(0x00, 1, 0x02, "设备锁定", "DEVICE LOCK");

	//scrPrint_Ex(0x00, 3, 0x02, "1.密码解锁", "1.Password Unlock");
	scrPrint_Ex(0x00, 4, 0x02, "1.联机解锁", "1.Online	Unlock");
	
	while(1)
	{
		ucKey = kbGetKey_lib();
		
		if('1' == ucKey)
		{
			/*iRet = hal_unlockPassWord();
			if(UNLOCKPAD_INPUTOK != iRet)
			{
				scrPrint_lib(1,3,0,"PassWord error");
                return -1;
			}
			else
			{
				break;
			}
		}
		else if('2' == ucKey)
		{*/
			iRet = hal_unlockOnline();
			if(0 != iRet)
			{
				return -2;
			}
			else
			{
				break;
			}
		}
		else if(ucKey == KEYCANCEL)
		{
			return 0;
		} 
		else
		{
			;
		}
		sysDelayMs(20);
	}


	scrCls_lib();
	scrPrint_lib(1,2,0,"pedFormat begin......");
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xA6, 0x01, 0x01, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
    sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:  ucCmd = %s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, caShow);
	fibo_free(caShow);

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: frameFactory,iRet = %d, iRet=0x%x\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet, iRet);
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 20000);
	fibo_free(frm.data);
	if(iRet <0) {
		sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: transceiveFrame,iRet = %d, iRet=0x%x\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet, iRet);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: iRet = %d, iRet=0x%x\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet, iRet);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9340)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d: RET_END,iRet = %d, iRet=0x%x\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet, iRet);
	if(iRet == 0)
	{
	    //se_reboot();
		scrPrint_lib(1,4, 0x00,"pedFormat ok", "pedFormat ok");
	}
	else
	{
		scrPrint_lib(1, 3, 0x00, "pedFormat failed");
		scrPrintf_lib(" iRet=%d\r\n", iRet);
	}
	sysDelayMs(2000);
	//scrPrint_lib(1,5,0x00,"Press the cancel key to exit");
	//scrPrint_lib(1,5,0x00,"Please restart POS!");
#if 0
	while(1)
	{
		ucKey = kbGetKey_lib();
		if(ucKey == KEYCANCEL)
		{					// 按键: 取消键

			return KEYCANCEL;
		}           
    }
#endif
	return iRet;
}

void Menu_SysInfoChoseLanChn(void)
{
	scrSetLanguage_lib(1);
}

void Menu_SysInfoChoseLanEng(void)
{
	scrSetLanguage_lib(0);
}

void Menu_SysInfoChoseLan(void)
{
	int iSize = 0;

	STMENUNODE astMenu[] = {
				{ "语言设置", "SET LANGUAGE", NoFun },
				{ "中文", "CHINESE", Menu_SysInfoChoseLanChn },
				{ "ENGLISH", "ENGLISH", Menu_SysInfoChoseLanEng },
		};

	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;
}
#define ADMIN_PWD      "23459876"
int adminInputPwd(void)
{
	int ucPwdLen = 0;
	unsigned char ucCmd = 0xFF;
	unsigned char ucPwd[16] = {0};
	unsigned char ucXINHAO[16] = {0};
	unsigned char aucPassWord[9];
	scrCls_lib();
	kbFlush_lib();
	scrPrint_Ex(0x00, 3, 0x02, "请输入密码", "Pls Input Password");
	while(1)
    {
		if(kbHit_lib() == 0)
        {
            ucCmd = kbGetKey_lib();
            switch(ucCmd)
            {
				case KEYENTER:
					if(ucPwdLen == 8)
					{
					    memcpy(aucPassWord, ucPwd, 8); 
						if(memcmp(aucPassWord, ADMIN_PWD, 8) == 0)
						{
							scrCls_lib();
							scrPrint_Ex(0x00, 2, 0x02, "验证成功", "PASSWORD OK");
							sysDelayMs(1000);
								return UNLOCKPAD_INPUTOK;
						}
						else
						{
							scrCls_lib();
							scrPrint_Ex(0x00, 2, 0x02, "密码错误", "PASSWORD ERROR");
							sysDelayMs(2000);	
							return UNLOCKPAD_INPUTERR;
						}	
					}
	   
				break;
				case KEYCANCEL:
					return UNLOCKPAD_CANCEL;
				break;
				case KEY_FN:
					//return UNLOCKPAD_FN;
				break;
				
				case KEYCLEAR:
					if(ucPwdLen > 0)
					{
						ucPwdLen--;
						ucXINHAO[ucPwdLen] = 0x00;
						scrClrLine_lib(5,7);
						scrPrint_lib(1,5,2,"%s",ucXINHAO);
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
					if(ucPwdLen < 8)
					{
						ucXINHAO[ucPwdLen] = '*';
						ucPwd[ucPwdLen++] = ucCmd;
						scrClrLine_lib(5,7);
						scrPrint_lib(1,5,2,"%s",ucXINHAO);
					}
				break;
				default:
				break;

			}
        }
		sysDelayMs(20);
    }	
}
void getFileSystemInfo(void)
{
	int iRet;
	long fileSize = 0,total;
	
	scrCls_lib();
	scrPrint_lib(0,1,2,"fileSystemSize");
	//hal_keypadWaitOneKey(); 
	fileSize = fileGetFileSysFreeSize_lib();
	scrPrint_lib(0,3,0,"exFree : %d",fileSize);
	total = fibo_file_getTotalSize_ex("/ext");
	scrPrint_lib(0,4,0,"exTotal: %d",total);

	fileSize = fileInGetFileSysFreeSize_lib();
	scrPrint_lib(0,5,0,"inFree : %d",fileSize);
	total = fibo_file_getTotalSize();
	scrPrint_lib(0,6,0,"inTotal: %d",total);

	hal_keypadWaitOneKey();
}
void FormatFileSystem(void)
{
	filesyserase();
}
void fileSystem(void)
{
	int iSize = 0;
	STMENUNODE astMenu[] = {
				{ "文件系统", "FILE SYSTEM", NoFun },
				{ "获取信息", "Get Info", getFileSystemInfo },
				{ "格式化",   "FormatFileSystem",  FormatFileSystem },
		};

	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;
}

void setUSB(void)
{
	int iSize = 0; 
    STMENUNODE astMenu[] = {
			{ "基础项", "SET USB", NoFun },
            { "获取模式", "GetMode", Menu_USBQueCurMode},
			{ "设置模式", "SetMode", Menu_USBSetMode},
			//{ "开启所有端口", "OpenAllPorts", Menu_USBZeroAndFiveOpen},
			//{ "开启单一端口", "OpenSinglePort", Menu_USBZeroAndFiveClose},		
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    //SetMiddleSizeFont();
    Menu_ShowStandardMenu(astMenu, iSize);
    //SetLargeSizeFont();
    return;
}

void adminSet(void)
{
	int iRet = 0;
	int iSize = 0;
	iRet = adminInputPwd();
	if(iRet != 0)
	{
		return ;
	}

	STMENUNODE astMenu[] = {
				{ "管理员设置", "ADMIN SET", NoFun },
				{ "文件系统", "File System", fileSystem },
				{ "设置USB", "Set USB", setUSB },
		};

	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowStandardMenu(astMenu, iSize);
	return;
}

void Menu_GetKeypadSound(void)
{
	unsigned char ucKey;
	int level;
	scrCls_lib();
	scrPrint_lib(0,1,0,"按键音:%d", kbGetSound_lib());
	scrPrint_lib(0,2,0,"KeyPadSound:%d", kbGetSound_lib());
	ucKey = kbGetKey_lib();

	while(1)
	{
		ucKey = kbGetKey_lib();
		if(ucKey == 0xFF)
		{					
			continue;
		}
		if(ucKey == KEYCANCEL)	
		{
			scrCls_lib();
			return;
		}		

	}

}

void Menu_SetKeypadSound(void)
{
	unsigned char ucKey;
	int level;
	scrCls_lib();
	scrPrint_lib(0,1,0,"设置按键音:");
	scrPrint_lib(0,2,0,"Pls Set KeyPadSound:");
	scrPrint_lib(0,3,0,"press 0 or 1");
	ucKey = kbGetKey_lib();

	while(1)
	{
		ucKey = kbGetKey_lib();
		if(ucKey == 0xFF)
		{					
			continue;
		}
		if(ucKey == KEYCANCEL)	
		{
			scrCls_lib();
			return;
		}		
		if(ucKey == KEY0)
		{
			kbSetSound_lib(0);
			return;
		}
		else if(ucKey == KEY1)
		{
			kbSetSound_lib(1);
			return;
		}
		
	}

}


void KayPadSound(void)
{
    int iSize = 0; 
    STMENUNODE astMenu[] = {
			{ "基础项", "BasicFunc", NoFun },
            { "读取按键音设置", "GetKeypadSound", Menu_GetKeypadSound},
			{ "设置按键音", "SetKeypadSound", Menu_SetKeypadSound},
        };            
    iSize = sizeof(astMenu) / sizeof(STMENUNODE);
    Menu_ShowStandardMenu(astMenu, iSize);
    return;
}


void Menu_SystemSetting(void)
{
	int iSize = 0;

	STMENUNODE astMenu[] = {
				{ "系统设置", 		"System Setting ", NoFun },
				{ "设置时间 ", 		"Set Time       ", Menu_SysInfoSetTime },
				{ "显示时间 ", 		"Get Time       ", Menu_SysInfoGetTime },
				{ "版本信息 ", 		"Show Version   ", Menu_SysInfoShowVer },
				{ "语言选择 ", 		"Set Language   ", Menu_SysInfoChoseLan },
				//{ "主频设置 ",	  "MainFreq Setting", Menu_ModSetMainFreq},
				//{ "键盘设置 ",   "KeyBoard Setting", Menu_ModSetKeypad},
				{ "格式化PED ",  "Format PED ", Menu_pedFormat },
				{ "设置Log等级 ",  "SETLogLevel", mLogset },
				{ "Se时间设置", "SeTimeSet", Menu_SeTime},
				{ "管理员设置 ",   "admin Set", adminSet},
				{ "按键音 ",   "KeyPad Sound", KayPadSound},
//#ifdef  FUN_DEBUG_TEST              
                //{ "格式化EXFLASH ",  "Format Exflash ", Menu_FormatExflash },
				//{ "重建文件系统  ",  "Recover Filesys", Menu_RecoverFilesys },
//#endif               
		};
	//SetMiddleSizeFont();
	iSize = sizeof(astMenu) / sizeof(STMENUNODE);
	Menu_ShowHiddenMenu(astMenu, iSize,0);
	//SetLargeSizeFont();
	return;
}

//extern void fileStressTest();
//==================一级主菜单======================//
int MainMenu(void)
{
    int iSize = 0xFF;
	int iRet = 0;
	int iLoop = 0;
    int iTotLine = 0, iCurPage = 0;
    int iTotPage = 0, iCurPageMaxItem = 0;
    unsigned char ucKey = 0xFF;
	unsigned char ucItemNum=0x00;
	unsigned char ucStartline = 0x00;
	unsigned char ucMenuItemMaxLine = 0x00;
	unsigned char ucMenuTitleLine = 0x00;
    
    STMENUNODE astMenu[] = {
            { "主菜单",      "HOME MENU", NoFun },
            { "本地下载",    "Local Download", Menu_LocalDownload },
            //{ "密钥下载", "Key Download", Menu_KeyDownload },
            { "系统设置",    "System Setting", Menu_SystemSetting },
            { "模块测试",    "Test Module   ", Menu_ModuleCheck },
//            { "flash测试",    "Test flash   ", fileStressTest },
          };

REMAIN:
    ucItemNum=0x00;
    ucStartline = 0x00;
    ucMenuItemMaxLine = 0x00;
	ucMenuTitleLine = 0x00;
    iLoop = 0;
    iTotLine = 0;
    iCurPage = 0;
    iTotPage = 0;
    iCurPageMaxItem = 0;

	iSize = sizeof(astMenu) / sizeof(STMENUNODE);

	iTotLine = iSize - 1;//remove title
	if(iTotLine == 0)
	{
		//return 0;
	}
	ucMenuItemMaxLine = g_uiMaxLine -1;
    

	iTotPage = (iTotLine - 1) / ucMenuItemMaxLine + 1;

	kbFlush_lib();  

    while(1)
    {
        scrCls_lib();
		//if(g_ucLcdType == 0x01)
		//{
		//	ucStartline = 0x00;
		//	ucMenuTitleLine = 0x01;
		//}
		//else
		{
			Menu_uiSetMenuTitle(&astMenu[0]);
			ucStartline = 0x01;

            ucMenuTitleLine = 0x00;
		}
		
		if(iTotLine - iCurPage*ucMenuItemMaxLine <= ucMenuItemMaxLine)
		{
			iCurPageMaxItem = iTotLine - iCurPage*ucMenuItemMaxLine;
		}
		else
		{
			iCurPageMaxItem  = ucMenuItemMaxLine;
		}
        
		for(iLoop = 1; iLoop <= iCurPageMaxItem; iLoop++)
		{
			ucItemNum++;
			Menu_uiSetMenuItem(iLoop - ucMenuTitleLine , ucItemNum, &astMenu[iCurPage*ucMenuItemMaxLine + iLoop]);
		}
        
        while(2)
        {
            //iRet = hal_downLoadFile(10);
            //if(iRet)
            //{
            //    goto REMAIN;
            //}
            
            ucKey = kbGetKey_lib();
			
			// 数字键选定功能
			if(ucKey >= '1' && ucKey <= '0' + iTotLine)
			{
				astMenu[ucKey - '1' + 1].pFun();
				kbFlush_lib();
				ucItemNum-=iCurPageMaxItem;
				break;
			}

			if(iTotPage >= 2 && ucKey == KEYENTER)
			{					// 按键: 确认键
				iCurPage += 1;
				//ucMenuTitleLine = 0;
				if(iCurPage >= iTotPage)
				{
					ucItemNum = 0;
					iCurPage = 0;
					//ucMenuTitleLine = 1;
				}
					
				break;
			}

			if(ucKey == KEYCANCEL)
			{					// 按键: 取消键
                //if(g_ucLcdType == 0x01) /*黑白屏*/
                //{
                    //ScrFontSet(0);
                //}
				return KEYCANCEL;
			}           
        }
    }
    
    //if(g_ucLcdType == 0x01) /*黑白屏*/
    //{
        //ScrFontSet(0);
    //}
    
	return iRet;
}

void SelectMode(void)
{
    unsigned char ucCmd = 0xFF;
    unsigned long long uiStartTimer;
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:SelectMode into\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
    uiStartTimer = sysGetTicks_lib()+1000;    
    do{
        if(kbHit_lib() == 0)
        {
            ucCmd = kbGetKey_lib();
			sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:kbGetKey_lib ucCmd=0x%x\r\n", filename(__FILE__), __FUNCTION__, __LINE__, ucCmd);
            if(ucCmd == KEYENTER)
            {
            	iMenuFlag = 1;
               ucCmd = MainMenu();
			   scrCls_lib();
			}
        }
		sysDelayMs(20);
		sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:SelectMode sysDelayMs  iMenuFlag = %d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iMenuFlag);
    }
    while(uiStartTimer > sysGetTicks_lib());
	sysLOG_lib(LCD_LOG_LEVEL_2, "[%s] -%s- Line=%d:SelectMode exit\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
}