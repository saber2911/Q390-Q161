/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_base.c
** Description:		System initialization related interfaces
**
** Version:	1.0, 渠忠磊,2022-02-23
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"
#include "cus_export_table.h"


uint32 g_ui32HwVersion = 0;//硬件版本控制

uint32 g_ui32SemLdATHandle;//外部接口透传AT信号量
uint32 g_ui32Timer2ID = 0;//关机充电时定时接口
uint32 g_ui32Timer3ID = 0;//FOTA升级时在定时器中调用升级接口

uint32 g_ui32SemMenuHandle;//开机进menu之后阻塞执行应用程序的lock
uint32 g_ui32SemSEHandle;//开机进检测是否触发阻塞执行应用程序的lock
uint32 g_ui32MutexLcdHandle;//刷屏防止竞争的锁

struct _LOGOINFOJSON g_stLogoinfoJson;


typedef void (*GetAppInform)(char inform[512]);
GetAppInform g_fcGetAppInfom = NULL;//读取APP版本信息

uint8 g_ui8LcdType = 1;//0-黑白屏(128*96);1-彩屏(320*240)
uint8 g_ui8LcdLogoRefresh = 1;//0-only艾体刷开机logo;1-厂家+艾体刷开机logo

char g_i8TermType = 0;//保存机型，0-Q161

uint8 g_ui8KeyTestEn = 0;//按键测试demo中用到的使能接口

int g_i32SpiFlashStatus = 0;//0-外部flash未挂载成功; 1-外部flash挂载成功


/*
*Function:		hal_sysGetTickms
*Description:	读系统滴答时钟，单位ms
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		滴答时钟，单位ms
*Others:
*/
unsigned long long hal_sysGetTickms(void)
{	
	UINT64 iRet = 0, Ret = 0;
	
	Ret = fibo_getSysTick_ext();
	iRet = Ret/1000;
	return iRet;
	
}

/*
*Function:		hal_sysGetAppInfo
*Description:	读取APP版本相关信息
*Input:			NULL
*Output:		appinfo:输出APP版本相关信息
*Hardware:
*Return:		NULL
*Others:
*/
void hal_sysGetAppInfo(char appinfo[512])
{

	if(g_fcGetAppInfom != NULL)
	{
		
		sysLOG(BASE_LOG_LEVEL_2, "g_fcGetAppInfom != NULL,%s\r\n", appinfo);
		g_fcGetAppInfom(appinfo);
	}
	
}

/*
*Function:		hal_sysGetAppInfoReg
*Description:	注册读取APP版本相关信息接口
*Input:			*Pgetappinfom:注册读取APP接口指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_sysGetAppInfoReg(void (*Pgetappinfom)(char inform[512]))
{
	g_fcGetAppInfom = Pgetappinfom;
	
	sysLOG(BASE_LOG_LEVEL_5, "g_fcGetAppInfom:%d\r\n", g_fcGetAppInfom);
}


/*
*Function:		hal_sysGetHalInfo
*Description:	读取底层库版本号
*Input:			*halinfo:底层库版本号存储指针
*Output:		NULL
*Hardware:
*Return:		读取版本号长度
*Others:
*/
int hal_sysGetHalInfo(int8 *halinfo)
{
	sprintf(halinfo, "\r\nSoftware DRVERSION:%s\r\nCompile Time:%s %s\r\n", DR_VER, __TIME__, __DATE__);
	return strlen(halinfo);
}


/*
*Function:		hal_sysATTrans
*Description:	透传AT指令
*Input:			*sendbuf:发送内容; sendlen:发送内容长度; timeout:超时时间; *cmd:关键字
*Output:		*recvbuf:接收buf
*Hardware:
*Return:		<0:失败；>=0:接收到的长度
*Others:
*/
int hal_sysATTrans(int8 *sendbuf, int sendlen, int8 *recvbuf, int timeout, int8 *cmd)
{
	int iRet = -1;
	int8 *retP = NULL;
	unsigned long long uTime;
	
	iRet = fibo_sem_try_wait(Sem_AT_signal, timeout);
	if(FALSE == iRet)
	{
		sysLOG(AT_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR>fibo_sem_try_wait,iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return -3;
	}
	CBuffFormat(&g_stATCbBuffStruct);
	
	g_ui8ATFlag = 2;
	uTime = hal_sysGetTickms()+timeout;
	iRet = hal_atSend(sendbuf, sendlen);

	while(1)
	{
		retP = MyStrStr(g_stATCbBuffStruct.buff, cmd, g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
		if(retP != NULL)
		{
			break;
		}
		if(hal_sysGetTickms() > uTime)
		{
			break;
		}
		sysDelayMs(100);
	}
	
	g_ui8ATFlag = 0;
	
	if(retP == NULL)//未接收到
	{
		return -2;
		goto exit;
	}
	
	iRet = retP-(g_stATCbBuffStruct.buff+g_stATCbBuffStruct.read_P)+strlen(cmd);
	
	sysLOG(BASE_LOG_LEVEL_5, "iRet:%d, retP:%x, recvbuff:%s\r\n", iRet, retP, g_stATCbBuffStruct.buff+g_stATCbBuffStruct.read_P);
	if(iRet <= 0)
	{
		goto exit;
	}
	memcpy(recvbuf, g_stATCbBuffStruct.buff+g_stATCbBuffStruct.read_P, iRet);
	
exit:
	
	fibo_sem_signal(Sem_AT_signal);
	return iRet;
}

cus_export_tab_t *cus_export_api = NULL;
unsigned int cus_export_api_ver = 0;

/*
*Function:		hal_sysGetBPCompleteVer
*Description:	获取完整的底包版本号
*Input:			NULL
*Output:		*bpver:获取版本号指针
*Hardware:
*Return:		<0:失败；>=0:版本号长度
*Others:
*/
static int hal_sysGetBPCompleteVer(int8 *bpver)
{

	int iRet = -1;
	int8 retP[128];
	int8 *rP1,*rP2;
	uint32 outlen=128,infoLen;
	cus_export_tab_t *cus_export_api_tmp = NULL;

	memset(retP,0,sizeof(retP));
	cus_export_api_tmp = (cus_export_tab_t *)fibo_get_version_info(retP, outlen);
	if(NULL != cus_export_api_tmp)
		cus_export_api = cus_export_api_tmp;

	iRet = 0;
	sysLOG(BASE_LOG_LEVEL_4, "iRet=%d retP:%s\r\n", iRet,retP);
	if(iRet == 0)
	{
		infoLen = strlen(retP);
		rP1 = MyStrStr(retP, "+CGMR:", 0, infoLen);
		if(rP1 != NULL)
		{
			rP2 = MyStrStr(rP1, "\r\n", 0, infoLen);
			if(rP2 != NULL)
			{
				memcpy(bpver,rP1,rP2-rP1);
				iRet = (int)(rP2-rP1);
			}
		}

		sysLOG(BASE_LOG_LEVEL_5, "iRet=%d rp1:%p,rp2:%p\r\n", iRet, rP1, rP2);

	}
	sysLOG(BASE_LOG_LEVEL_4, "iRet=%d ver:%s\r\n", iRet, bpver);

	return iRet;
		
}

/*
*Function:		hal_sysGetBPVersion
*Description:	获取底包版本号
*Input:			NULL
*Output:		*bpver:获取版本号指针
*Hardware:
*Return:		<0:失败；>=0:版本号长度
*Others:
*/
int hal_sysGetBPVersion(int8 *bpver)
{
	int iRet = -1;
	int len = -1;
	char *rP1 = NULL, *rP2 = NULL, *rP3 = NULL;
	char bptmp[256];

	memset(bptmp, 0, sizeof(bptmp));	
	len = hal_sysGetBPCompleteVer(bptmp);
	
	rP1 = MyStrStr(bptmp, ".", 0, 256);
	if(rP1 == NULL)
	{
		iRet = -1;
		goto exit;
	}

	rP2 = MyStrStr(rP1+1, ".", 0, 256-(rP1-bptmp));
	if(rP2 == NULL)
	{
		iRet = -2;
		goto exit;
	}

	rP3 = MyStrStr(rP2+1, ".", 0, 256-(rP2-bptmp));
	if(rP3 == NULL)
	{
		iRet = -3;
		goto exit;
	}

	if(len-(rP3-bptmp+1) < 8)
	{
		iRet = -4;
		goto exit;
	}
	
	memcpy(bpver, rP3+1, 8);
	iRet = 8;
	sysLOG(BASE_LOG_LEVEL_2, "iRet=%d, bptmp=%s, bpver=%s\r\n", iRet, bptmp, bpver);
	
exit:
	return iRet;
}



/*
*NV区中TERM分区总共8个字节，其中数据占5个字节。目前只用到了Byte0,
*Byte0=0:Q360; Byte0=1:Q390; Byte0=2:Q550;
*
*/

/*
*Function:		hal_sysGetNVTerm
*Description:	读取NV区中的设备类型
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败;other:读到的机型码，0-Q161
*Others:
*/
int hal_sysGetNVTerm(void)
{
	int iRet;
	char bufftmp[64];
	char *rP = NULL;
	
	memset(bufftmp, 0, sizeof(bufftmp));
	iRet = hal_nvReadTerm(bufftmp);
	if(iRet <= 0)
		return -1;

	iRet = bufftmp[0];
	if(iRet<0 || iRet>2)
		return -2;
	
	return iRet;
}

/*
*Function:		hal_sysGetTermType
*Description:	获取设备机型
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败;0:成功
*Others:		在设备开机时调用，用来从SE端或NV区中读取机型
*/
int hal_sysGetTermType(void)
{
	int iRet = -1;
	char bufftmp[64];
	char *rP = NULL;
	
	sysLOG(BASE_LOG_LEVEL_5, "<START>\r\n");
	memset(bufftmp, 0, sizeof(bufftmp));
	
	iRet = hal_sysGetNVTerm();
	sysLOG(BASE_LOG_LEVEL_1, "hal_sysGetNVTerm, iRet=%d\r\n", iRet);
	if(iRet >= 0)
	{
		g_i8TermType = iRet;
		iRet = 0;
	}
		
	
	
	return iRet;
}


/*
*Function:		hal_sysReadTermType
*Description:	读取机型值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		机型类型值，0-Q161
*Others:		
*/
int hal_sysReadTermType(void)
{
	return g_i8TermType;
}



/*
*Function:		hal_sysCreateUsrDir
*Description:	开机时，创建用户目录
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_sysCreateUsrDir(void)
{
	int32 ret;
	int8 *dirname0="/app";
	int8 *dirname1="/app/ufs";
	int8 *dirname2="/app/data";

	ret = hal_fileMkdir(dirname0);
	
	if(ret<0)
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<WARN> /app create failed!! ret=%d\r\n", ret); 
	}
	else
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<SUCC> /app create success! ret=%d\r\n", ret); 
	}

	
	ret = hal_fileMkdir(dirname1);

	if(ret<0)
	{
		
		sysLOG(BASE_LOG_LEVEL_4, "<WARN> /app/ufs create failed!! ret=%d\r\n", ret); 
	}
	else
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<SUCC> /app/ufs create success! ret=%d\r\n", ret); 
	}
	
	ret = hal_fileMkdir(dirname2);

	if(ret<0)
	{
		
		sysLOG(BASE_LOG_LEVEL_4, "<WARN> /app/data create failed!! ret=%d\r\n", ret); 
	}
	else
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<SUCC> /app/data create success! ret=%d\r\n", ret); 
	}
}


/*
*Function:		hal_sysCreateAppDir
*Description:	在开机时创建APP用到的文件夹
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void hal_sysCreateAppDir(void)
{
	int32 ret;
	int8 *dirname0="/ext/app";
	int8 *dirname1="/ext/app/data";

	ret = hal_fileMkdir(dirname0);
	
	if(ret<0)
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<WARN> dirname0 create failed!! ret=%d\r\n", ret); 
	}
	else
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<SUCC> dirname0 create success! ret=%d\r\n", ret); 
	}
	
	ret = hal_fileMkdir(dirname1);

	if(ret<0)
	{
		
		sysLOG(BASE_LOG_LEVEL_4, "<WARN> dirname1 create failed!! ret=%d\r\n", ret); 
	}
	else
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<SUCC> dirname1 create success! ret=%d\r\n", ret); 
	}

	ret = hal_fileMkdir(TTS_LIBPATH);

	if(ret<0)
	{
		
		sysLOG(BASE_LOG_LEVEL_4, "<WARN> dirname1 create failed!! ret=%d\r\n", ret); 
	}
	else
	{
	
		sysLOG(BASE_LOG_LEVEL_4, "<SUCC> dirname1 create success! ret=%d\r\n", ret); 
	}
}

/*
*Function:		hal_sysSetSleepMode
*Description:	设置睡眠模式
*Input:			time:多久后进入睡眠，0-退出休眠
*Output:		NULL
*Hardware:
*Return:		0:成功; other:失败
*Others:		
*/
int hal_sysSetSleepMode(uint8 time)
{
	
	int iRet  = -1;
	
	iRet = fibo_setSleepMode(0);
	
	return iRet;
}

/*
*Function:		hal_sysSetTimezone
*Description:	设置时区
*Input:			timezone:时区值，单位¼时区
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_sysSetTimezone(int32 timezone)
{
	int iRet;
	unsigned char rtctimeBCD[16];
	
	iRet = fibo_setRTC_timezone((int8_t)timezone);
	sysLOG(BASE_LOG_LEVEL_1, "fibo_setRTC_timezone, iRet=%d, timezone=%d\r\n", iRet, timezone);

	if(0 == iRet)
	{
		memset(rtctimeBCD, 0, sizeof(rtctimeBCD));
		iRet = sysGetTime_lib(rtctimeBCD);
		if(0 == iRet)
			sysSetTimeSE_lib(rtctimeBCD);
	}
	
	return iRet;
}

/*
*Function:		hal_sysGetTimezone
*Description:	读时区
*Input:			*timezone:时区值指针，单位¼时区
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_sysGetTimezone(int32 *timezone)
{
	int iRet;
	
	iRet = fibo_getRTC_timezone();
	*timezone = iRet;
	iRet = 0;
	sysLOG(BASE_LOG_LEVEL_1, "fibo_setRTC_timezone, iRet=%d, *timezone=%d\r\n", iRet, *timezone);

	return iRet;
}


/*
*Function:		hal_sysSetRTC
*Description:	设置RTC时间
*Input:			*rtctime: RTC_time结构体指针
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_sysSetRTC(RTC_time *rtctime)
{
	int iRet;
	unsigned char rtctimeBCD[16];
	hal_rtc_time_t rtctime_tmp;
	
	rtctime_tmp.sec = rtctime->sec;
	rtctime_tmp.min = rtctime->min;
	rtctime_tmp.hour = rtctime->hour;
	rtctime_tmp.day = rtctime->day;
	rtctime_tmp.month = rtctime->month;
	rtctime_tmp.year = rtctime->year - 2000;
	rtctime_tmp.wDay = rtctime->wDay;
	
	iRet = fibo_setRTC(&rtctime_tmp);

	memset(rtctimeBCD, 0, sizeof(rtctimeBCD));
	iRet = sysGetTime_lib(rtctimeBCD);
	if(0 == iRet)
	{
		sysSetTimeSE_lib(rtctimeBCD);
	}
	
	return iRet;
	
}

/*
*Function:		hal_sysGetRTC
*Description:	读取RTC时间
*Input:			NULL
*Output:		*rtctime:RTC_time结构体指针
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_sysGetRTC(RTC_time *rtctime)
{
	int iRet;
	hal_rtc_time_t rtctime_tmp;
	
	
	
	iRet = fibo_getRTC(&rtctime_tmp);

	if(iRet < 0)
	{
		return iRet;
	}
	rtctime->sec = rtctime_tmp.sec;
	rtctime->min = rtctime_tmp.min;
	rtctime->hour = rtctime_tmp.hour;
	rtctime->day = rtctime_tmp.day;
	rtctime->month = rtctime_tmp.month;
	rtctime->year = rtctime_tmp.year + 2000;
	rtctime->wDay = rtctime_tmp.wDay;

	
	return iRet;

	
}



/*
*Function:		hal_sysSetTime
*Description:	设置系统的日期和时间，星期值将自动算出并设置
*Input:			*pucTime：日期时间参数的指针,格式为YYMMDDhhmmss,参数为BCD码,共6个字节长。(有效时间范围：年(20)00~(20)99,月1~12,日1~31,小时0~24,分钟和秒0~59)
*Output:		NULL
*Hardware:
*Return:		0:成功，其他:日期时间值非法
*Others:
*/
int hal_sysSetTime(unsigned char *pucTime)
{
	int iRet = -1;
	
	RTC_time rtctimetmp;
	rtctimetmp.year = BcdToDec(pucTime, 1)+2000;
	rtctimetmp.month = BcdToDec(pucTime+1, 1);
	rtctimetmp.day = BcdToDec(pucTime+2, 1);
	rtctimetmp.hour = BcdToDec(pucTime+3, 1);
	rtctimetmp.min = BcdToDec(pucTime+4, 1);
	rtctimetmp.sec = BcdToDec(pucTime+5, 1);
	
	if((rtctimetmp.year < 2000 || rtctimetmp.year > 2099) || ((*(pucTime)&0x0F)>9 || ((*(pucTime)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_YEAR;
		goto exit;
	}
	if((rtctimetmp.month < 1 || rtctimetmp.month > 12) || ((*(pucTime+1)&0x0F)>9 || ((*(pucTime+1)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_MONTH;
		goto exit;
	}
	if((rtctimetmp.day < 1 || rtctimetmp.day > 31) || ((*(pucTime+2)&0x0F)>9 || ((*(pucTime+2)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_DAY;
		goto exit;
	}
	if((rtctimetmp.hour < 0 || rtctimetmp.hour > 23) || ((*(pucTime+3)&0x0F)>9 || ((*(pucTime+3)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_HOUR;
		goto exit;
	}
	if((rtctimetmp.min < 0 || rtctimetmp.min > 59) || ((*(pucTime+4)&0x0F)>9 || ((*(pucTime+4)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_MIN;
		goto exit;
	}
	if((rtctimetmp.sec < 0 || rtctimetmp.sec > 59) || ((*(pucTime+5)&0x0F)>9 || ((*(pucTime+5)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_SEC;
		goto exit;
	}
	
	iRet = hal_sysSetRTC(&rtctimetmp);
	if(iRet < 0)
	{
		iRet = ERR_TIMERDWR_FAILED;
	}

exit:
	return iRet;

}

/*
*Function:		hal_sysGetTime
*Description:	读取终端日期和时间
*Input:			NULL
*Output:		*ucTime:读取到的时间指针，以BCD码存放
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_sysGetTime(unsigned char *ucTime)
{

	int iRet = -1;
	
	RTC_time rtctimetmp;
	iRet = hal_sysGetRTC(&rtctimetmp);
	//sysDelayMs(100);
	if(iRet < 0)
	{
		iRet = ERR_TIMERDWR_FAILED;
		goto exit;
	}
	DecToBcd(rtctimetmp.year-2000, ucTime, 1);
	DecToBcd(rtctimetmp.month, ucTime+1, 1);
	DecToBcd(rtctimetmp.day, ucTime+2, 1);
	DecToBcd(rtctimetmp.hour, ucTime+3, 1);
	DecToBcd(rtctimetmp.min, ucTime+4, 1);
	DecToBcd(rtctimetmp.sec, ucTime+5, 1);
	DecToBcd(rtctimetmp.wDay, ucTime+6, 1);

	iRet = 0;

exit:

	return iRet;
	
}

/*
*Function:		hal_sysSetTimeSE
*Description:	设置SE系统的日期和时间，星期值将自动算出并设置
*Input:			*pucTime：日期时间参数的指针,格式为YYMMDDhhmmss,参数为BCD码,共6个字节长。(有效时间范围：年(20)00~(20)99,月1~12,日1~31,小时0~24,分钟和秒0~59)
*Output:		NULL
*Hardware:
*Return:		0:成功，其他:日期时间值非法
*Others:
*/
int hal_sysSetTimeSE(unsigned char *pucTime)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 12;
	int output_len = 0;
	unsigned char i = 0;
	RTC_time rtctimetmp;
	rtctimetmp.year = BcdToDec(pucTime, 1);
	rtctimetmp.month = BcdToDec(pucTime+1, 1);
	rtctimetmp.day = BcdToDec(pucTime+2, 1);
	rtctimetmp.hour = BcdToDec(pucTime+3, 1);
	rtctimetmp.min = BcdToDec(pucTime+4, 1);
	rtctimetmp.sec = BcdToDec(pucTime+5, 1);
	unsigned char ucCmdHead[12] = {0x00, 0xe2, 0x32, 0x00, iCmdLen-6, (iCmdLen -6) >> 8,rtctimetmp.year,rtctimetmp.month,rtctimetmp.day,rtctimetmp.hour,rtctimetmp.min,rtctimetmp.sec};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	if((rtctimetmp.year < 0 || rtctimetmp.year > 99) || ((*(pucTime)&0x0F)>9 || ((*(pucTime)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_YEAR;
		goto RET_END;
	}
	if((rtctimetmp.month < 1 || rtctimetmp.month > 12) || ((*(pucTime+1)&0x0F)>9 || ((*(pucTime+1)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_MONTH;
		goto RET_END;
	}
	if((rtctimetmp.day < 1 || rtctimetmp.day > 31) || ((*(pucTime+2)&0x0F)>9 || ((*(pucTime+2)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_DAY;
		goto RET_END;
	}
	if((rtctimetmp.hour < 0 || rtctimetmp.hour > 23) || ((*(pucTime+3)&0x0F)>9 || ((*(pucTime+3)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_HOUR;
		goto RET_END;
	}
	if((rtctimetmp.min < 0 || rtctimetmp.min > 59) || ((*(pucTime+4)&0x0F)>9 || ((*(pucTime+4)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_MIN;
		goto RET_END;
	}
	if((rtctimetmp.sec < 0 || rtctimetmp.sec > 59) || ((*(pucTime+5)&0x0F)>9 || ((*(pucTime+5)&0xF0)>>4)>9))
	{
		iRet = ERR_TIMEPARAM_SEC;
		goto RET_END;
	}
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
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
	if(0x9000 == iRet) {
		iRet = RET_RF_OK;
	}
	else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
    }
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*Function:		hal_sysGetTimeSE
*Description:	读取SE终端日期和时间
*Input:			NULL
*Output:		*ucTime:读取到的时间指针，以BCD码存放
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_sysGetTimeSE(unsigned char *ucTime)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char i = 0;
	RTC_time rtctimetmp;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x32, 0x01, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
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
	if(0x9000 == iRet) {
	    rtctimetmp.year = retfrm.data[6];
		rtctimetmp.month = retfrm.data[7];
		rtctimetmp.day = retfrm.data[8];
		rtctimetmp.hour = retfrm.data[9];
		rtctimetmp.min = retfrm.data[10];
		rtctimetmp.sec = retfrm.data[11];
		rtctimetmp.wDay = retfrm.data[12];
		DecToBcd(rtctimetmp.year, ucTime, 1);
		DecToBcd(rtctimetmp.month, ucTime+1, 1);
		DecToBcd(rtctimetmp.day, ucTime+2, 1);
		DecToBcd(rtctimetmp.hour, ucTime+3, 1);
		DecToBcd(rtctimetmp.min, ucTime+4, 1);
		DecToBcd(rtctimetmp.sec, ucTime+5, 1);
		DecToBcd(rtctimetmp.wDay, ucTime+6, 1);
		iRet = RET_RF_OK;
	}
	else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
    }
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "RET_END,iRet = %d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_sysGetHwVersion
*Description:	读硬件版本
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0
*Others:		SE复位脚工作常态低电平，拉高sleep在拉低reset SE
*				暂时只做SE复位脚初始，其他硬件版本的检测与处理在后续开发中加入
*/
int hal_sysGetHwVersion(void)
{
	int iRet = -1;
	int32 adc_tmp;
	
	fibo_gpio_mode_set(SE_REBOOT_IO, GpioFunction0);
	fibo_gpio_cfg(SE_REBOOT_IO, GpioCfgOut);
	fibo_gpio_set(SE_REBOOT_IO,FALSE);

	do{
		
		iRet = hal_pmADRead(1, &adc_tmp);
		sysDelayMs(10);
	}while(iRet != 0);
	
	sysLOG(BASE_LOG_LEVEL_1, "hal_GetHwVersion, adc_tmp=%d, iRet=%d\r\n", adc_tmp, iRet);

	if(0<=adc_tmp && adc_tmp<=50)//V1.0硬件
	{
		g_ui32HwVersion = 0;	
	}
	else if(50<adc_tmp && adc_tmp<=220)//V1.1硬件
	{
		g_ui32HwVersion |= 0b1;	
	}
	else if(220<adc_tmp && adc_tmp<=350)//V1.2硬件
	{
		g_ui32HwVersion |= 0b10; 
	}

	iRet = 0;

	return iRet;
}



#define UNLOCKPAD_INPUTOK   0
#define UNLOCKPAD_INPUTERR  -1
#define UNLOCKPAD_TIMEOUT   -2
#define UNLOCKPAD_CANCEL    -3
#define UNLOCKPAD_FN    -4

bool g_blUnLockPeding = FALSE;
bool g_blAppStart = FALSE;//是否开始进入应用流程

int getPWD(char *pwd)
{
    unsigned char ucCmd = 0xFF;
    unsigned char ucPwd[16] = {0};
    unsigned char ucXINHAO[16] = {0};
    int ucPwdLen = 0;
    unsigned long long uiStartTimer;
	sysLOG(BASE_LOG_LEVEL_5, "SelectMode into\r\n");

    hal_keypadFlush();
    uiStartTimer = hal_sysGetTickms()+1000*20; 
    do{
        if(hal_keypadHit() == 0)
        {
            ucCmd = hal_keypadGetKey();
			sysLOG(BASE_LOG_LEVEL_2, "hal_keypadGetKey ucCmd=0x%x\r\n", ucCmd);

            switch(ucCmd)
            {
                case KEYENTER:
                    if(ucPwdLen == 8)
                    {
                        memcpy(pwd, ucPwd, 8);
						return 0;
                    }
       
                break;
                case KEYCANCEL:
                    return UNLOCKPAD_CANCEL;
                break;
                case KEY_FN:
                    return UNLOCKPAD_FN;
                break;
                
                case KEYCLEAR:
                    if(ucPwdLen > 0)
                    {
                        ucPwdLen--;
                        ucXINHAO[ucPwdLen] = 0x00;
                        hal_scrClrLine(5,7);
                        hal_scrPrint(1,5,2,"%s",ucXINHAO);
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
                        hal_scrClrLine(5,7);
                        hal_scrPrint(1,5,2,"%s",ucXINHAO);
                    }
                break;
                default:
                break;
			}
        }
		sysDelayMs(20);
		sysLOG(BASE_LOG_LEVEL_5, "getPWD sysDelayMs\r\n");
    }
	while(uiStartTimer > hal_sysGetTickms());
	sysLOG(LCD_LOG_LEVEL_2, "getPWD TIMEOUT\r\n");
	return UNLOCKPAD_TIMEOUT;
}



int UnlockPad(void)
{
    unsigned char ucCmd = 0xFF;
    unsigned char ucPwdA[16] = {0};
	unsigned char ucPwdB[16] = {0};
    unsigned char ucXINHAO[16] = {0};
	unsigned char ucPwd[16] = {0};
    int ucPwdLen = 0;
    unsigned long long uiStartTimer;
	int iRet = 0;
	sysLOG(LCD_LOG_LEVEL_2, "UnlockPad into\r\n");
    g_blUnLockPeding = TRUE;
    hal_keypadFlush();
    uiStartTimer = hal_sysGetTickms()+1000*60; 
    hal_scrClrLine(5,7);
#if 1
	hal_scrPrintEx(0,3,0x00,"请输入管理员A密码:\n","Input PWD A to UNLOCK\n");
	do{
		
        if(hal_keypadHit() == 0)
        {
            ucCmd = hal_keypadGetKey();
			sysLOG(LCD_LOG_LEVEL_2, "hal_keypadGetKey ucCmd=0x%x\r\n", ucCmd);

            switch(ucCmd)
            {
				case KEYENTER:
					if(ucPwdLen == 8)
					{
						memcpy(ucPwdA, ucPwd, 8);
						if(strcmp(ucPwdA, "28210528") == 0) 
						{
						    return UNLOCKPAD_INPUTOK;
						}
						else
						{
							return UNLOCKPAD_INPUTERR;
						}
					}
	   
				break;
				case KEYCANCEL:
					return UNLOCKPAD_CANCEL;
				break;
				case KEY_FN:
					return UNLOCKPAD_FN;
				break;
				
				case KEYCLEAR:
					if(ucPwdLen > 0)
					{
						ucPwdLen--;
						ucXINHAO[ucPwdLen] = 0x00;
						hal_scrClrLine(5,7);
						hal_scrPrint(1,5,2,"%s",ucXINHAO);
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
						hal_scrClrLine(5,7);
						hal_scrPrint(1,5,2,"%s",ucXINHAO);
					}
				break;
				default:
				break;

			}
        }
		sysDelayMs(20);
		sysLOG(BASE_LOG_LEVEL_5, "UnlockPad sysDelayMs\r\n");
    }
    while(uiStartTimer > hal_sysGetTickms());
    
	sysLOG(LCD_LOG_LEVEL_2, "UnlockPad exit\r\n");

    return UNLOCKPAD_TIMEOUT;
#endif
}

void DeviceReboot(void)
{
   
    hal_pmPwrRST();
}


/*
*Function:		hal_ReadLogoInfoJson
*Description:	解析logoinfo.json文件
*Input:			jsonfilename:logoinfo.json文件名字(带绝对路径);
*Output:		*logoinfojson:LOGOINFOJSON指针
*Hardware:
*Return:		0:succ; <0:failed
*Others:		
*/
int hal_ReadLogoInfoJson(char *jsonfilename, struct _LOGOINFOJSON *logoinfojson)
{
	int iRet = -1;
	char *rP = NULL;
	int filesizetmp = 0;

	Vs_cJSON *jsonroot = NULL;
	Vs_cJSON *jsoncomment_node;
	Vs_cJSON *jsondotMatLcd_node;
	Vs_cJSON *jsoncolorLcd_node;

	Vs_cJSON *jsonchild_node;
	Vs_cJSON *jsongrandchild_node;

	
	iRet = hal_fileExist(jsonfilename);
	if(iRet < 0)
		goto exit;
	
	iRet = hal_fileGetFileSize(jsonfilename);
	if(iRet < 0)
		goto exit;

	filesizetmp = iRet;
	rP = malloc(filesizetmp);
	if(rP == NULL)
		return -1;

	memset(rP, 0, filesizetmp);
	iRet = hal_fileReadPro(jsonfilename, 0, rP, filesizetmp);
	if(iRet != filesizetmp)
	{
		iRet = -2;
		goto exit;
	}

	jsonroot = Vs_cJSON_Parse(rP);
    if(jsonroot == 0)
    {
        sysLOG(BASE_LOG_LEVEL_2, "<ERR>Vs_cJSON_Parse error\r\n");
        iRet = -3;
		goto exit;
    }
    
    
	jsoncomment_node = Vs_cJSON_GetObjectItem(jsonroot, "_comment");
	if(jsoncomment_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem error\r\n");
	}
	Vs_cJSON *commentnode = jsoncomment_node->child;
    int i = 0;
    while(commentnode != 0)
    {
		sysLOG(BASE_LOG_LEVEL_5, "comment name:%d:%s, type:%d\r\n", i, commentnode->valuestring, commentnode->type);
        commentnode = commentnode->next;
		i +=1 ;
    }


	/*******************************Lcd*******************************/
	
	jsondotMatLcd_node = Vs_cJSON_GetObjectItem(jsonroot, "dotMatLcd");
    if(jsondotMatLcd_node == 0){
		sysLOG(BASE_LOG_LEVEL_2, "<ERR>Vs_cJSON_GetObjectItem error\r\n");
        iRet = -4;
		goto exit;
    }

	jsonchild_node = Vs_cJSON_GetObjectItem(jsondotMatLcd_node, "bg_color");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem bg_color not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->lcd_bg_color = jsonchild_node->valueint;
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsondotMatLcd_node, "fg_color");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem fg_color not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->lcd_fg_color = jsonchild_node->valueint;
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsondotMatLcd_node, "bl_pwm");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem bl_pwm not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->lcd_bl_pwm = jsonchild_node->valueint;
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsondotMatLcd_node, "position");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem position not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,type=%d\r\n", jsonchild_node->string, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Array){
			Vs_cJSON *node = jsonchild_node->child;
			if(node != 0)
	        {
	        	logoinfojson->lcd_position_x = node->valueint;
				sysLOG(BASE_LOG_LEVEL_5, "<SUCC>,value=%d,type=%d\r\n", node->valueint, node->type);
	            node = node->next;
	        }
			if(node != 0)
	        {
	        	logoinfojson->lcd_position_y = node->valueint;
				sysLOG(BASE_LOG_LEVEL_5, "<SUCC>,value=%d,type=%d\r\n", node->valueint, node->type);
			}
		
		}
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsondotMatLcd_node, "contrast");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem contrast not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,type=%d\r\n", jsonchild_node->string, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Object){
			jsongrandchild_node = Vs_cJSON_GetObjectItem(jsonchild_node, "uc1617s");
		    if(jsongrandchild_node == 0){
				sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem uc1617s not exited\r\n");
		    }
			else{
				sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsongrandchild_node->string, jsongrandchild_node->valueint, jsongrandchild_node->type);
				if(jsongrandchild_node->type == Vs_cJSON_Number)
					logoinfojson->lcd_contrast_uc1617s = jsongrandchild_node->valueint;
			}

		}

		if(jsonchild_node->type == Vs_cJSON_Object){
			jsongrandchild_node = Vs_cJSON_GetObjectItem(jsonchild_node, "st7571");
		    if(jsongrandchild_node == 0){
				sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem st7571 not exited\r\n");
		    }
			else{
				sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsongrandchild_node->string, jsongrandchild_node->valueint, jsongrandchild_node->type);
				if(jsongrandchild_node->type == Vs_cJSON_Number)
					logoinfojson->lcd_contrast_st7571 = jsongrandchild_node->valueint;
			}

		}
	}


	/*******************************ColorLcd*******************************/
	
	jsoncolorLcd_node = Vs_cJSON_GetObjectItem(jsonroot, "colorLcd");
    if(jsoncolorLcd_node == 0){
		sysLOG(BASE_LOG_LEVEL_2, "<ERR>Vs_cJSON_GetObjectItem error\r\n");
        iRet = -4;
		goto exit;
    }

	jsonchild_node = Vs_cJSON_GetObjectItem(jsoncolorLcd_node, "bg_color");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem bg_color not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->colorlcd_bg_color = jsonchild_node->valueint;
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsoncolorLcd_node, "fg_color");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem fg_color not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->colorlcd_fg_color = jsonchild_node->valueint;
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsoncolorLcd_node, "bl_pwm");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem bl_pwm not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->colorlcd_bl_pwm = jsonchild_node->valueint;
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsoncolorLcd_node, "position");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem position not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,type=%d\r\n", jsonchild_node->string, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Array){
			Vs_cJSON *node = jsonchild_node->child;
			if(node != 0)
	        {
	        	logoinfojson->colorlcd_position_x = node->valueint;
				sysLOG(BASE_LOG_LEVEL_5, "<SUCC>,value=%d,type=%d\r\n", node->valueint, node->type);
	            node = node->next;
	        }
			if(node != 0)
	        {
	        	logoinfojson->colorlcd_position_y = node->valueint;
				sysLOG(BASE_LOG_LEVEL_5, "<SUCC>,value=%d,type=%d\r\n", node->valueint, node->type);
			}
		
		}
	}

	jsonchild_node = Vs_cJSON_GetObjectItem(jsoncolorLcd_node, "rotation");
    if(jsonchild_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem rotation not exited\r\n");
    }
	else{
		sysLOG(BASE_LOG_LEVEL_5, "<SUCC>Vs_cJSON_GetObjectItem name:%s,value=%d,type=%d\r\n", jsonchild_node->string, jsonchild_node->valueint, jsonchild_node->type);
		if(jsonchild_node->type == Vs_cJSON_Number)
			logoinfojson->colorlcd_rotation = jsonchild_node->valueint;
	}

	/*******************************logofile*******************************/

	jsoncomment_node = Vs_cJSON_GetObjectItem(jsonroot, "logofile");
	if(jsoncomment_node == 0){
		sysLOG(BASE_LOG_LEVEL_3, "<WARN>Vs_cJSON_GetObjectItem error\r\n");
	}
	Vs_cJSON *logofilenode = jsoncomment_node;
	if(logofilenode != 0)
	{
		sysLOG(BASE_LOG_LEVEL_5, "%s, type:%d\r\n", logofilenode->valuestring, logofilenode->type);
		memcpy(logoinfojson->logofile, logofilenode->valuestring, strlen(logofilenode->valuestring));
	}

	iRet = 0;
	
exit:
	
	if(NULL != jsonroot)
		Vs_cJSON_Delete(jsonroot);

	if(NULL != rP)
		free(rP);
	
	sysLOG(BASE_LOG_LEVEL_1, "iRet = %d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_LogoInfoInit
*Description:	LogoInfo 初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:succ; <0:failed
*Others:		
*/
int hal_LogoInfoInit(void)
{
	int iRet;
	//step 1:先赋缺省值
	g_stLogoinfoJson.lcd_bg_color = 0;
	g_stLogoinfoJson.lcd_fg_color = 3;
	g_stLogoinfoJson.lcd_bl_pwm = 408;
	g_stLogoinfoJson.lcd_position_x = g_stLcdConfig.LOGODEFAULTPOSITION_X;
	g_stLogoinfoJson.lcd_position_y = g_stLcdConfig.LOGODEFAULTPOSITION_Y;
	g_stLogoinfoJson.lcd_contrast_uc1617s = 20;
	g_stLogoinfoJson.lcd_contrast_st7571 = 30;

	g_stLogoinfoJson.colorlcd_bg_color = WHITE;
	g_stLogoinfoJson.colorlcd_fg_color = BLUE;
	g_stLogoinfoJson.colorlcd_bl_pwm = 408;
	g_stLogoinfoJson.colorlcd_position_x = g_stLcdConfig.COLORLOGODEFAULTPOSITION_X;
	g_stLogoinfoJson.colorlcd_position_y = g_stLcdConfig.COLORLOGODEFAULTPOSITION_Y;

	g_stLogoinfoJson.colorlcd_rotation = 1;

	memset(g_stLogoinfoJson.logofile, 0, sizeof(g_stLogoinfoJson.logofile));
	sprintf(g_stLogoinfoJson.logofile, LOGOFILENAME_DEFAULT);

	//step 2:如果logoinfo.json存在并且解析正确则将json文件中的值作为初始值
	iRet = hal_ReadLogoInfoJson(LOGOINFOJSON_NAME, &g_stLogoinfoJson);
	g_stLogoinfoJson.lcd_fg_color = 3;

	return 0;

}

void DisplayHardwareTriggerType(unsigned int TriggerStatus)
{
	unsigned int Status = 0;

	//Tamper Trigger
	if(TriggerStatus & 0x01)
	{
		hal_scrPrint(0,4,0,"T1= YES");
	}
	else
	{
		hal_scrPrint(0,4,0,"T1= NO ");
	}
	if(TriggerStatus & 0x02)
	{
		hal_scrPrint(0,4,4,"T2= YES");
	}
	else
	{
		hal_scrPrint(0,4,4,"T2= NO ");
	}	
	if(TriggerStatus & 0x10)
	{
		hal_scrPrint(0,5,0,"T3= YES");
	}
	else
	{
		hal_scrPrint(0,5,0,"T3= NO ");
	}
	if(TriggerStatus & 0x20)
	{
		hal_scrPrint(0,5,4,"T4= YES");
	}
	else
	{
		hal_scrPrint(0,5,4,"T4= NO ");
	}

	//Voltage Trigger
	if(TriggerStatus & 0x40)
	{	
		hal_scrPrint(0,6,0,"LV= YES");
	}
	else{
		hal_scrPrint(0,6,0,"LV= NO ");
	}
	if(TriggerStatus & 0x80)
	{	
		hal_scrPrint(0,6,4,"HV= YES");
	}
	else{
		hal_scrPrint(0,6,4,"HV= NO ");
	}

	//Temp Trigger
	if(TriggerStatus & 0x0100)
	{	
		hal_scrPrint(0,7,0,"LT= YES");
	}
	else{
		hal_scrPrint(0,7,0,"LT= NO ");
	}
	if(TriggerStatus & 0x0200)
	{	
		hal_scrPrint(0,7,4,"HT= YES");
	}
	else{
		hal_scrPrint(0,7,4,"HT= NO ");
	}

	//frequency Trigger
	/*if(TriggerStatus & 0x0400)
	{	
		hal_scrPrint(0,8,0,"L_FD3=   YES");
	}
	else{
		hal_scrPrint(0,8,0,"L_FD3=   NO");
	}
	if(TriggerStatus & 0x0800)
	{	
		hal_scrPrint(0,8,4,"H_FD3= YES");
	}
	else{
		hal_scrPrint(0,8,4,"H_FD3=   NO ");
	}*/
}

void hal_RTCsync(void)
{
	int iRet = -1;
	unsigned char rtctimeBCD[16];
	reg_info_t reg_info;

	while(1)
	{
	    fibo_getRegInfo(&reg_info, 0);
		
		sysLOG(BASE_LOG_LEVEL_1, "reg_info.nStatus=%d\r\n", reg_info.nStatus);
		if(reg_info.nStatus==1)
		{
			iRet = cus_export_api->fibo_already_autoTimed();
			sysLOG(BASE_LOG_LEVEL_1, "fibo_already_autoTimed=%d\r\n", iRet);
			if(TRUE == iRet)//已完成基站对时
			{
				iRet = sysGetTime_lib(rtctimeBCD);
				if(0 == iRet)
				{
					sysSetTimeSE_lib(rtctimeBCD);
				}
			}
			else//没有基站对时
			{
				memset(rtctimeBCD, 0, sizeof(rtctimeBCD));
				iRet = sysGetTimeSE_lib(rtctimeBCD);
				if(0 == iRet)
				{
					sysSetTime_lib(rtctimeBCD);
				}
			}
			break;
		}

		sysDelayMs(1000);
	}
	sysLOG(BASE_LOG_LEVEL_1, "iRet=%d\r\n", iRet);
}


/*
*Function:		hal_sysPwrThread
*Description:	电源管理线程
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
static void hal_sysPwrThread(void *param)
{
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
	
	
	while(1)
	{
		
		hal_pmBatChargerCheck(100);
		hal_pmTemperJudge(100, TEMPERHOT_LIMITVALUE);
		hal_ledRedHandle();
		hal_ledBlueHandle();
		hal_ledYellowHandle();
		sysDelayMs(100);
		
	}
}


/*
*Function:		hal_sysSeStatusThread
*Description:	
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void hal_sysSeStatusThread(void *param)
{
	unsigned char ucKey = 0xFF;
	int iSeStatus = 0;
	int iAttempts = 0;
	int iRet = 0;
	unsigned int TriggerStatus = 0;

	sysLOG(LCD_LOG_LEVEL_2, "Device SelfCheck, param 0x%x\r\n", param);
	//sysDelayMs(1000);

	//hal_scrCls();
	//hal_scrPrintEx(1,5,2,"设备自检...", "Device SelfCheck...");
	//sysDelayMs(1000*5);

	while(1)
	{	
		if(((iSeStatus < SELF_CHECK_START) || (iSeStatus > SELF_CHECK_HARD)) && (!g_blUnLockPeding))
		{
			iSeStatus = GetPedState();//check_se_status();
			sysLOG(LCD_LOG_LEVEL_2, "GetPedState, iSeStatus = 0x%x\r\n", iSeStatus);
		}
		else
		{
			break;
		}
		sysDelayMs(1000);
	}	

	if((SELF_CHECK_TAMPER == iSeStatus || SELF_CHECK_HARD == iSeStatus) && (!g_blUnLockPeding))
	{
	    g_blUnLockPeding = TRUE;
	    //jiesuo
		hal_scrCls();
		hal_scrPrintEx(1,2,2,"设备触发，确认键解锁!", "Device trigger, confirm key to unlock...");
		if(Se_GetHardwareTriggerStatus(&TriggerStatus) == RET_RF_OK)
		{
			DisplayHardwareTriggerType(TriggerStatus);
		}
		
		while(1)
		{
			if(hal_keypadHit()==0) 
			{
				ucKey = hal_keypadGetKey();
				if(ucKey == KEYENTER)
				{
					break;
				}
			}
			sysDelayMs(50);
		}

	    hal_scrCls();
		//hal_scrClrLine(3,7);
		hal_scrPrint(1,5,2,"Device LOCK!");
		iRet = Menu_pedFormat();
		hal_scrClrLine(3,7);
        if(iRet == 0)
        {
		    hal_scrPrint(1,5,2,"Device Unlock Success!Reboot...");
        }
		else
		{
			hal_scrPrint(1,5,2,"Device Unlock Failed!Reboot...");
		}
		sysDelayMs(1000);
		DeviceReboot();
		return;
	}
	fibo_sem_signal(g_ui32SemSEHandle);
	sysDelayMs(100);
	fibo_thread_delete();
	sysLOG(LCD_LOG_LEVEL_2, "hal_sysSeStatusThread fibo_thread_delete\r\n");

}


/*
*Function:		hal_sysKeyThread
*Description:	按键线程
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
static void hal_sysKeyThread(void *param)
{
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);

	hal_keypadReg(hal_keypadDeal);
	
	while(1)
	{
		
		hal_keypadHandle();	
		
		hal_pkeyHandle();


//		if(g_ui8KeyTestEn == 1)
//		{
//			keytest();
//		}
		sysDelayMs(100);
		
	}
    
}

/*
*Function:		hal_sysTtsThread
*Description:	TTS播报线程
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
static void hal_sysTtsThread(void *param)
{
	int iRet;
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
	
#if MAINTEST_FLAG
	
	hal_ttsTest();
	
		
	//SN_Hw_test();
	//NV_test();
		
#endif	
	g_stSendPkg.pkg_index = 0xFF;//初始化0xFF,不然第一帧包序为0x00时会误认为是重复帧！
	while(1)
	{
		hal_ttsLoopHandle();
		//DR_Uart0_Handle();
		//DR_WifiwkupLoop();
		//iRet = fibo_sem_try_wait(Sem_Uart2CB_signal, 10);
		//if(iRet == TRUE)//获得信号量
		//{
			//dev_cpydata_IPD();
		//}
		
		hal_scrBackLightHandle();
		hal_secscrBackLightHandle();
		
		sysDelayMs(100);
	}
    //fibo_thread_delete();
}

/*
*Function:		hal_sysIconThread
*Description:	图标线程
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
static void hal_sysIconThread(void *param)
{
	int iRet;
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);



	while(1)
	{
		if(g_ui8LcdType == 0)
		{
			hal_iconLoop();
		}
		else
		{

			hal_ciconLoop();
		}

		sysDelayMs(1000);
		
#if MAINTEST_FLAG

//		wifiGetLinkStatus_lib();

#endif

		
	}
    
}

/*
*Function:		hal_sysPdpOpenThread
*Description:	pdp激活线程
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void hal_sysPdpOpenThread(void *param)
{
	int iRet;
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
	
	while(1)
	{
		hal_RTCsync();
		hal_wiresockPppOpen(NULL, NULL, NULL);
		sysDelayMs(1000);
		fibo_thread_delete();
	}
}

/*
*Function:		hal_sysMenuThread
*Description:	菜单线程
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
static void hal_sysMenuThread(void *param)
{
	int i32Ret = 0;
	char machine[64];
	sysLOG(LCD_LOG_LEVEL_2, "application thread enter, param 0x%x\r\n", param);
	sysDelayMs(100);
	//DownLoadVOS();
	dcepInit();
	sysGetTermType_se(machine);
	sysLOG(SELOAD_LOG_LEVEL_1, "exit %s \r\n",machine);
	SelectMode();
	fibo_sem_signal(g_ui32SemMenuHandle);
	sysDelayMs(100);
	fibo_thread_delete();
	sysLOG(LCD_LOG_LEVEL_2, "SelectMode fibo_thread_delete\r\n");
}

/*
*Function:		hal_sysCallBackDataHandleThread
*Description:	
*Input:			*param:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
static void hal_sysCallBackDataHandleThread(void *param)
{
  	int ret = -1;
	int iRet = -1;
	char msg = 0;
	int back_msg = 0;
	unsigned int TriggerStatus = 0;

	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
	
	while(1)
	{		
		if(g_iUpdateVOSFlag == 0)
		{
			ret = fibo_queue_get(SE_callBack_Queue, &msg, 0);
			fibo_queue_reset(SE_callBack_Queue);
			if(ret == 0)
			{
				ret = checkEvent(&back_msg);
				if(ret>=0)
				{
					sysLOG(BASE_LOG_LEVEL_1, "back_msg=%x\r\n",back_msg);
				}
				if((back_msg & TM_CALLBACK_STATUS) && (!g_blUnLockPeding) && g_blAppStart)//触发
				{
				    g_blUnLockPeding = TRUE;
						
					hal_scrCls();
			       	hal_scrPrintEx(1,2,2,"设备锁定，将重启!", "DEVICE LOCK,Reboot...");
					if(Se_GetHardwareTriggerStatus(&TriggerStatus) == RET_RF_OK);
					{
						DisplayHardwareTriggerType(TriggerStatus);
					}	
#if 1
					for(int i = 8; i >= 0; i--)
					{
						//hal_scrCls();
						hal_scrClrLine(2,2);
						hal_scrPrint(1,2,2, "DEVICE LOCK,restart in %d seconds!", i);
						sysDelayMs(1000);
					}
#endif
					sysDelayMs(1000);
					DeviceReboot();
					break;
				}
			}
		}
		fibo_taskSleep(100);
	}

}

/*
*Function:		hal_sysUniFontIsExit
*Description:	检测unicode字库是否存在
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-不存在，1-存在；
*Others:		在Q161中暂时用不到
*/
int hal_sysUniFontIsExit(void)
{

	return 0;

#if 0
	int iRet = 0;
	int8 bufftmp[32];

	memset(bufftmp, 0, sizeof(bufftmp));
	s_ReadFontLibData(bufftmp, EXT_FLASH_FONT_START, 16);

	if((memcmp(bufftmp, "VST-FONT", 8) == 0) && bufftmp[8] == FONTVERSIONUNICODE)
		iRet = 1;
	else
		iRet = 0;
	
	sysLOG(BASE_LOG_LEVEL_1, "iRet=%d\r\n", iRet);

	return iRet;
#endif
}

/*
*Function:		hal_sysGBFontIsExit
*Description:	检测GB2312字库是否存在
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-不存在，1-存在；
*Others:		
*/
int hal_sysGBFontIsExit(void)
{
	int iRet = 0;
	
	if(1 == hal_fileExist(GB2312FONT12X12FILENAME))
		iRet |= 1;
	if(1 == hal_fileExist(GB2312FONT16X16FILENAME))
		iRet |= (1 << 1);
	if(1 == hal_fileExist(GB2312FONT24X24FILENAME))
		iRet |= (1 << 2);
	if(1 == hal_fileExist(GB2312FONTFILENAME))
		iRet |= (1 << 3);

	sysLOG(BASE_LOG_LEVEL_1, "iRet=%d\r\n", iRet);
	if(iRet != 0)
		iRet = 1;
	else
		iRet = 0;

	return iRet;	

}

int hal_sysWritelcdlogo(void)
{
	int8 *rP = NULL;
	int32 logofilesize = 0;
	

	logofilesize = hal_fileGetFileSize(g_stLogoinfoJson.logofile);
	if(logofilesize > 0)
	{
		rP = malloc(logofilesize);
		if(rP != NULL)
		{
			memset(rP, 0, logofilesize);
			hal_fileReadPro(g_stLogoinfoJson.logofile, 0, rP, logofilesize);
		}
		
	}

	if(rP != NULL)//有logo.bin文件
	{
		hal_scrWriteLogo(g_stLogoinfoJson.lcd_position_x, g_stLogoinfoJson.lcd_position_y, rP);
	}
	else//没有logo.bin文件则刷默认logo图标gImage_New_Aisino_128x32
	{
		hal_scrWriteLogo(g_stLogoinfoJson.lcd_position_x, g_stLogoinfoJson.lcd_position_y, gImage_New_Aisino_128x32);
	}

	if(NULL != rP)
		free(rP);

	return 0;
}

int hal_sysWriteclcdlogo(void)
{

	int iRet = -1;
	int32 logofilesize = 0;
	unsigned int w = 0, h = 0;
	int8 *rP = NULL;
	unsigned char *rPlogo = NULL;

	logofilesize = hal_fileGetFileSize(g_stLogoinfoJson.logofile);
	
	if(logofilesize > 0)//有logo.bin/logo.bmp文件
	{

		rP = malloc(logofilesize);
		if(rP == NULL)
		{
			iRet = -1;
			goto exit;
		}
		memset(rP, 0, logofilesize);
		hal_fileReadPro(g_stLogoinfoJson.logofile, 0, rP, logofilesize);
			
		if(0 == memcmp(g_stLogoinfoJson.logofile, LOGOFILENAME_DEFAULT, sizeof(LOGOFILENAME_DEFAULT)))//刷logo.bin
		{
			
#if LCD_LOGODIRECTION//刷竖向logo

			rPlogo = malloc(logofilesize);
			if(rPlogo == NULL)
			{
				iRet = -1;
				goto exit;
			}
			memset(rPlogo, 0, logofilesize);
			
			hal_scrLogoVertical2Trans(rP, rPlogo, &w, &h);
			hal_scrWriteLogo(g_stLogoinfoJson.colorlcd_position_y, g_stLcdConfig.COLORLCD_PIXHIGH-g_stLogoinfoJson.colorlcd_position_x-h, rPlogo);
#else//刷横向logo
			hal_scrWriteLogo(g_stLogoinfoJson.colorlcd_position_x, g_stLogoinfoJson.colorlcd_position_y, rP);
#endif

		}
		else//刷logo.bmp
		{
		
#if LCD_LOGODIRECTION//刷竖向logo

			BITMAPFILEHEADER filehead_src;
			BITMAPINFOHEADER infohead_src;
		
			memcpy(&filehead_src, rP, sizeof(BITMAPFILEHEADER));
			memcpy(&infohead_src, rP+sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
			w = infohead_src.bImageWidth;
			h = infohead_src.bImageHeight;

			hal_bmpWriteToScr(g_stLogoinfoJson.colorlcd_position_y, g_stLcdConfig.COLORLCD_PIXHIGH-g_stLogoinfoJson.colorlcd_position_x-w, rP, logofilesize, 270);

#else
			hal_bmpWriteToScr(g_stLogoinfoJson.colorlcd_position_x, g_stLogoinfoJson.colorlcd_position_y, rP, logofilesize, 0);
#endif

		}
					
	}
	else//没有logo.bin/logo.bmp文件则刷默认logo图标gImage_New_Aisino_128x32

	{
		hal_scrWriteLogo(g_stLogoinfoJson.colorlcd_position_x, g_stLogoinfoJson.colorlcd_position_y, gImage_New_Aisino_128x32);
	}

exit:

	if(NULL != rP)
		free(rP);
	
	if(NULL != rPlogo)
		free(rPlogo);

	return iRet;
}


/*
*Function:		hal_sysFlashFontInit
*Description:	字库初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void hal_sysFlashFontInit(void)
{
	int iRet;
	int8 bufftmp[32];

	if(g_ui8LcdType == 1)
	{
		hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
	}
	
ReadFontType:

	sysLOG(BASE_LOG_LEVEL_4, "ReadFontType\r\n");
	memset(bufftmp, 0, sizeof(bufftmp));
	iRet = hal_nvReadFontType(bufftmp);
	if(iRet > 0)
	{
		if(FONTFLASH == (uint8)bufftmp[0])//unicode类型字库
		{
			g_strSpiFlashInfo.flash_mountaddr = EXT_FS_FLASH_ADDR_START;
			g_strSpiFlashInfo.flash_mountsize = g_strSpiFlashInfo.flash_size - EXT_FS_FLASH_ADDR_START;
			
			iRet = fibo_ffsmountExtflash(g_strSpiFlashInfo.flash_mountaddr, g_strSpiFlashInfo.flash_mountsize, EXT_FS_DIR, \
				FLASH_SPI_PINSEL, true, false); 
			sysLOG(BASE_LOG_LEVEL_1, "fibo_ffsmountExtflash! startAddr:0x%08x,size:0x%08x,dir:%s, iRet=%d\r\n", \
				g_strSpiFlashInfo.flash_mountaddr, g_strSpiFlashInfo.flash_mountsize, EXT_FS_DIR, iRet);

			if(iRet >= 0)
			{
				g_i32SpiFlashStatus = 1;
			}
			else
			{
				g_i32SpiFlashStatus = 0;
			}
			
			if(1 == hal_sysUniFontIsExit())//判断有字库
			{
				goto Start;
			}
			else
			{
				goto EngMenu;
			}
		}
		else if(FONTFS == (uint8)bufftmp[0])//GB2312类型字库
		{

			g_strSpiFlashInfo.flash_mountaddr = EXT_FS_FLASH_ADDR_START_FONTFS;
			g_strSpiFlashInfo.flash_mountsize = g_strSpiFlashInfo.flash_size - EXT_FS_FLASH_ADDR_START_FONTFS;
			
			/*此处挂载地址和flash大小，在提交服务器时必须修改为要挂载文件系统的flash地址和大小！！！*/
			iRet = fibo_ffsmountExtflash(g_strSpiFlashInfo.flash_mountaddr, g_strSpiFlashInfo.flash_mountsize, EXT_FS_DIR, \
				FLASH_SPI_PINSEL, true, false); 
			sysLOG(BASE_LOG_LEVEL_1, "fibo_ffsmountExtflash! startAddr:0x%08x,size:0x%08x,dir:%s, iRet=%d\r\n", \
				g_strSpiFlashInfo.flash_mountaddr, g_strSpiFlashInfo.flash_mountsize, EXT_FS_DIR, iRet);

			if(iRet >= 0)
			{
				g_i32SpiFlashStatus = 1;
			}
			else
			{
				g_i32SpiFlashStatus = 0;
			}
			
			if(1 == hal_sysGBFontIsExit())//判断有字库
			{
				goto Start;
			}
			else
			{
				goto EngMenu;
			}
		}
		else//NV区中参数不对
		{
			sysLOG(BASE_LOG_LEVEL_1, "<ERR> FontType not available\r\n");
			goto Loop;
		}
	

	}
	else//NV区中没有字库类型标识
	{
		if(1 == hal_sysUniFontIsExit())//判断有unicode字库
		{
			memset(bufftmp, 0, sizeof(bufftmp));
			bufftmp[0] = FONTFLASH;
			hal_nvWriteFontType(bufftmp, 1);//写入NV字库标识
			goto ReadFontType;
		}
		else
		{
			goto Loop;
		}
	}

Loop:

	sysLOG(BASE_LOG_LEVEL_5, "Loop\r\n");
	hal_fontInit(0xFF, GB2312FONT12X12FILENAME);//只加载ASCII字库
	memset(g_enum_font, 0, sizeof(g_enum_font));
    hal_fontEnum(g_enum_font, MAX_FONT_NUMS);
	
	if(g_ui8LcdType == 0)//初始化完字库之后在初始化一遍屏参
	{
		hal_scrSelectFont(&g_stSingleFont6X12, NULL);
	}
	else
	{		
		hal_scrSelectFont(&g_stSingleFont12X24, NULL);
	}
	
	DownLoadAPP();
	
	memset(bufftmp, 0, sizeof(bufftmp));
	iRet = hal_nvReadFontType(bufftmp);
	if(iRet > 0)//判断是否有字库类型标识
	{
		hal_scrCls();
		hal_scrPrint(1,2,2,"Wait a moment please.\n");
		sysDelayMs(1000);
		hal_pmPwrRST();//goto ReadFontType;
	}
	else
	{
		hal_scrCls();
		hal_scrPrint(1,2,2,"Write FontType please!\n");
		sysDelayMs(1000);
		goto Loop;
	}


EngMenu://将菜单设置为英文显示

	sysLOG(BASE_LOG_LEVEL_5, "EngMenu\r\n");
	Menu_SysInfoChoseLanEng();
	
Start://加载字库，启动应用

	sysLOG(BASE_LOG_LEVEL_5, "Start\r\n");
	memset(bufftmp, 0, sizeof(bufftmp));
	iRet = hal_nvReadFontType(bufftmp);
	if(iRet > 0)
	{
		if(FONTFLASH == (uint8)bufftmp[0] || FONTFS == (uint8)bufftmp[0])
			hal_fontSetFontType((uint8 *)bufftmp[0]);
	}
	sysLOG(BASE_LOG_LEVEL_1, "hal_fontGetFontType:%d,(uint8)bufftmp[0]=%d\r\n", hal_fontGetFontType(), (uint8)bufftmp[0]);

	if(FONTFS == hal_fontGetFontType())
		hal_fontLoad();
//	else
//		hal_loadFontLib();
	
Exit:

	sysLOG(BASE_LOG_LEVEL_5, "Exit\r\n");
	hal_scrCls();
	if(g_ui8LcdType == 1)
	{
		hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, g_stLogoinfoJson.colorlcd_fg_color);
	}
	return;

	
}

int hal_SetAtEmmRtc(void)
{
	int iRet = -1;
	char *sendbuf = "AT+CTZR=2\r\n";
	char recvbuf[256];

	memset(recvbuf, 0, sizeof(recvbuf));

	iRet = hal_sysATTrans(sendbuf, strlen(sendbuf), recvbuf, 5000, "OK");
	sysLOG(BASE_LOG_LEVEL_1, "hal_SetAtEmmRtc,iRet=%d\r\n", iRet);

	return iRet;
}

static FIBO_CALLBACK_T g_stSigCallBack = {
    .fibo_signal = hal_wiresockResCallback,
	.at_resp = hal_atResCallback};
extern void fibo_ext_flash_mode_set(uint8_t mode);

void *hal_sysBaseInit(void)
{
	
	int iRet;
	int8 *bufftmpP = NULL;
	uint32 allsize, availsize, maxblocksize;

	g_ui8LogLevel = 2;

	CBuffInit(&g_stATCbBuffStruct, ATCB_BUFF_LEN);
	//iRet = hal_SetAtEmmRtc();
	
#if MAINTEST_FLAG
	sysLogSet(5);
#endif
	hal_cfgParCfgInit();
	hal_pmLoopInit();
	hal_ledRun(LEDRED, 0, 0);
	hal_ledCtl(LEDRED, FALSE);
	
	fibo_gpio_mode_set(UART3_TXD, GpioFunction0);
	fibo_gpio_cfg(UART3_TXD, GpioCfgIn);
	iRet = fibo_gpio_pull_high_resistance(UART3_TXD, true);
	sysLOG(BASE_LOG_LEVEL_1, "fibo_gpio_pull_high_resistance,iRet=%d\r\n", iRet);


	/********************START********************/
	/*此处调用的接口顺序不要改变！*/

	hal_keypadPwrOffReg(keypwrofftest);//在正式走到应用之前，比如菜单项中关机，走这个注册接口。

	g_ui32SemUpdateHandle = fibo_sem_new(1);//此信号量必须在DR_FlashFontInit前面！

	hal_ttsInit();
	hal_keypadFlush();

	fibo_thread_create(hal_sysKeyThread, "hal_sysKeyThread", 1024*4, NULL, OSI_PRIORITY_ABOVE_NORMAL);
	fibo_thread_create(hal_sysTtsThread, "hal_sysTtsThread", 1024*8, NULL, OSI_PRIORITY_ABOVE_NORMAL);

	hal_portUsbInit();
	hal_sysGetTermType();

	fibo_ext_flash_mode_set(FLASH_SPI_MODE);
	hal_flashInit();

	//iRet = hal_flashAllErase(0 ,g_strSpiFlashInfo.flash_size/4096);

	//ex file system mount here, don't use exfilesystem before !!!
	hal_sysFlashFontInit();//此接口位置不要轻易改变！
	sysLOG(BASE_LOG_LEVEL_1, "flash_id=0x%x, flash_size=%d, flash_mountaddr=%d, flash_mountsize=%d\r\n", \
		g_strSpiFlashInfo.flash_id, g_strSpiFlashInfo.flash_size, g_strSpiFlashInfo.flash_mountaddr, g_strSpiFlashInfo.flash_mountsize);

	hal_sysCreateAppDir();
	
	/********************END********************/

	fibo_hal_pmu_setlevel(1, 1);//配置SD相关的IO为3.3V
	
	hal_ttsLibInit();
	
	if(g_ui8LcdType == 0)//初始化完字库之后在初始化一遍屏参
	{
		g_stLcdGUI.RefreshFlag=malloc(g_stLcdConfig.LCD_BLOCKBUFNUM);
		hal_lcdInit();
		if(g_ui32NoChargePwron == 1)//充电中开机后需要清充电图标
		{
			hal_scrClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, g_stLcdConfig.LCD_PIXHIGH-1);
		}
	}
	else
	{
		hal_clcdInit();
		if(g_ui32NoChargePwron == 1)//充电中开机后需要清充电图标
		{
			hal_scrClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
		}
	}
	
	hal_keypadReg(NULL);

    hal_scrGetLanguage();
	
	g_ui32SemMenuHandle = fibo_sem_new(0);
	g_ui32SemSEHandle = fibo_sem_new(0);
	
	if(g_ui8LcdType == 0)
	{
		if(g_i8NormalFlag == 'N')//非正常关机时，在pwrloop中已经刷过logo了，此处不需要再刷一遍
		{
			if(g_ui32NoChargePwron == 1)//充电中开机后需要清充电图标后再刷一遍logo
			{
				hal_scrClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, g_stLcdConfig.LCD_PIXHIGH-1);	
				hal_sysWritelcdlogo();
			}
		}
		hal_iconInit();
	}
	else
	{
		hal_clcdSetAttrib( LCD_ATTR_BACK_COLOR, g_stLogoinfoJson.colorlcd_bg_color);
		hal_clcdSetAttrib( LCD_ATTR_FRONT_COLOR, g_stLogoinfoJson.colorlcd_fg_color);
		if(g_i8NormalFlag == 'N')//非正常关机时，在pwrloop中已经刷过logo了，此处不需要再刷一遍
		{
			if(g_ui32NoChargePwron == 1)//充电中开机后需要清充电图标后再刷一遍logo
			{
				hal_scrClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
				hal_sysWriteclcdlogo();
			}
		}
		hal_ciconInit();
		hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
	}
	
	//wifi多进程互斥锁
	g_ui32ApiWiFiSendMutex = fibo_mutex_create();
	fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);

	g_ui32IPDMutexHandle = fibo_mutex_create();
	//Sem_Uart2CB_signal = fibo_sem_new(0);

	api_blueToothInit();
	
	wifiOpen_lib();

	bufftmpP = malloc(256);
	memset(bufftmpP, 0, 256);
	iRet = hal_sysGetHalInfo(bufftmpP);
	sysLOG(BASE_LOG_LEVEL_1, "hal_sysGetHalInfo iRet=%d:%s\r\n", iRet, bufftmpP);
	free(bufftmpP);
	//hal_iconHysteresistest();

	g_ui32MutexPdpOpenHandle = fibo_mutex_create();
	fibo_mutex_unlock(g_ui32MutexPdpOpenHandle);				
	fibo_thread_create(hal_sysPdpOpenThread, "hal_sysPdpOpenThread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	fibo_thread_create(hal_sysIconThread, "hal_sysIconThread", 1024*4, NULL, OSI_PRIORITY_ABOVE_NORMAL);
	fibo_thread_create(hal_sysPwrThread, "hal_sysPwrThread", 1024*16, NULL, OSI_PRIORITY_ABOVE_NORMAL);
	fibo_thread_create(hal_sysCallBackDataHandleThread, "hal_sysCallBackDataHandleThread", 1024*12, NULL, OSI_PRIORITY_HIGH);
	fibo_thread_create(hal_sysMenuThread, "hal_sysMenuThread", 1024*64, NULL, OSI_PRIORITY_ABOVE_NORMAL);

	fibo_sem_wait(g_ui32SemMenuHandle);
	fibo_sem_free(g_ui32SemMenuHandle);

	fibo_thread_create(hal_sysSeStatusThread, "hal_sysSeStatusThread", 1024*12, NULL, OSI_PRIORITY_ABOVE_NORMAL);
	fibo_sem_wait(g_ui32SemSEHandle);
	fibo_sem_free(g_ui32SemSEHandle);

	g_blAppStart = true;//开始进入应用流程

	hal_keypadPwrOffReg(NULL);

	if(g_ui8LcdType == 0)
	{

		hal_scrCls();
	
	}
	else
	{

		hal_scrClsArea(0, 24, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
		hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);

	}



#if MAINTEST_FLAG

	hal_keypadPwrOffReg(keypwrofftest);
	g_ui8KeyTestEn = 1;

#endif	

#if MAINTEST_FLAG

	hal_lcdDrawLamp(BLUE,TRUE);
	sysDelayMs(3000);
	hal_lcdDrawLamp(YELLOW,TRUE);
	sysDelayMs(3000);
	hal_lcdDrawLamp(GREEN,TRUE);
	sysDelayMs(3000);
	hal_lcdDrawLamp(RED,TRUE);
	sysDelayMs(3000);
	hal_lcdDrawLamp(BLUE,FALSE);
	sysDelayMs(3000);
	hal_ttsSetLanguage(0);
	hal_ttsQueuePlay("你好",NULL,NULL,0);
	sysLOG(BASE_LOG_LEVEL_1, "chinese\r\n");
	sysDelayMs(5000);
	hal_ttsSetLanguage(1);
	hal_ttsQueuePlay("hello",NULL,NULL,0);
	sysLOG(BASE_LOG_LEVEL_1, "english\r\n");
	sysDelayMs(5000);
	//fibo_thread_create(hal_ttsLoopPlayThread, "hal_ttsLoopPlayThread", 1024*4, NULL, OSI_PRIORITY_NORMAL);

	int backcolortmp = 0, textcolortmp = 0;
	backcolortmp = hal_scrGetAttrib(LCD_ATTR_BACK_COLOR);
	textcolortmp = hal_scrGetAttrib(LCD_ATTR_FRONT_COLOR);
	sysLOG(BASE_LOG_LEVEL_1, "backcolortmp=%x, textcolortmp=%x\r\n", backcolortmp, textcolortmp);
	//hal_camSweepTest();
	scrTest();
	//hal_flashTest();

	hal_wiresockSocketTest();

#endif

	#if 0
	int m=6;
	while(m--)
	{
		sysBeep_lib();
		sysDelayMs(200);
		
	}
	#endif

	//iRet = Initial_Decoder();
	sysLOG(BASE_LOG_LEVEL_5, "hal_sysBaseInit END\r\n");
	

	iRet = fibo_get_heapinfo(&allsize, &availsize, &maxblocksize);
	sysLOG(BASE_LOG_LEVEL_1, "fibo_get_heapinfo iRet=%d, allsize=%d, availsize=%d, maxblocksize=%d\r\n", iRet, allsize, availsize, maxblocksize);
	iRet = fibo_file_getFreeSize();
	sysLOG(BASE_LOG_LEVEL_1, "fibo_file_getFreeSize iRet=%d\r\n", iRet);
	iRet = fibo_getbootcause();//0-SoftRST;1-HardwareRST;2-PwrkeyRST;3-插入USB开机
	sysLOG(BASE_LOG_LEVEL_1, "fibo_getbootcause iRet=%d\r\n", iRet);


	//wifiClose_lib();
	//fibo_gpio_set(VBAT3V3EN_GPIO, false);//关闭3V3电源

	//hal_scrCls();
	return (void *)&g_stSigCallBack;

}

/*
extern void main();

void AppMain(void)
{
	main();
}
*/

