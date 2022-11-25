/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_driverapi.c
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
#include "api_driverapi.h"
#include "LC610N_api_uart.h"
#include "hal_fota.h"


/*
*Function:		sysBaseInit_lib
*Description:	初始化接口，在app enter里面调用即可
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		返回值是模块信号以及AT指令的回调，必须在enter接口返回
*Others:
*/
void *sysBaseInit_lib(void)
{
	return hal_sysBaseInit();
}


/*
*Function:		sysDelayMs_lib
*Description:	延时接口
*Input:			ms:延时时间，单位ms
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void sysDelayMs_lib(int ms)
{
	sysDelayMs(ms);
}


/*
*Function:		sysGetTicks_lib
*Description:	读系统滴答时钟，单位ms
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		滴答时钟，单位ms
*Others:
*/
unsigned long long sysGetTicks_lib(void)
{
	return hal_sysGetTickms();
}

/*
*Function:		sysSetTime_lib
*Description:	设置系统的日期和时间，星期值将自动算出并设置
*Input:			*pucTime：日期时间参数的指针,格式为YYMMDDhhmmss,参数为BCD码,共6个字节长。(有效时间范围：年(20)00~(20)99,月1~12,日1~31,小时0~24,分钟和秒0~59)
*Output:		NULL
*Hardware:
*Return:		0:成功，其他:日期时间值非法
*Others:
*/
int sysSetTime_lib(unsigned char *pucTime)
{

	return hal_sysSetTime(pucTime);

}


/*
*Function:		sysGetTime_lib
*Description:	读取终端日期和时间
*Input:			NULL
*Output:		*ucTime:读取到的时间指针，以BCD码存放
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int sysGetTime_lib(unsigned char *ucTime)
{

	return hal_sysGetTime(ucTime);
	
}

/*
*Function:		sysSetTimeSE_lib
*Description:	设置SE系统的日期和时间，星期值将自动算出并设置
*Input:			*pucTime：日期时间参数的指针,格式为YYMMDDhhmmss,参数为BCD码,共6个字节长。(有效时间范围：年(20)00~(20)99,月1~12,日1~31,小时0~24,分钟和秒0~59)
*Output:		NULL
*Hardware:
*Return:		0:成功，其他:日期时间值非法
*Others:
*/
int sysSetTimeSE_lib(unsigned char *pucTime)
{
	return hal_sysSetTimeSE(pucTime);
}

/*
*Function:		sysGetTimeSE_lib
*Description:	读取SE终端日期和时间
*Input:			NULL
*Output:		*ucTime:读取到的时间指针，以BCD码存放
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int sysGetTimeSE_lib(unsigned char *ucTime)
{
	return hal_sysGetTimeSE(ucTime);
}


/*
*Function:		sysSetRTC_lib
*Description:	设置RTC时间
*Input:			*rtctime: RTC_time结构体指针
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int sysSetRTC_lib(RTC_time *rtctime)
{

	return hal_sysSetRTC(rtctime);
	
}


/*
*Function:		sysGetRTC_lib
*Description:	读取RTC时间
*Input:			NULL
*Output:		*rtctime:RTC_time结构体指针
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int sysGetRTC_lib(RTC_time *rtctime)
{

	return hal_sysGetRTC(rtctime);
	
}

/*
*Function:		sysSetTimezone_lib
*Description:	设置时区
*Input:			timezone:时区值，单位¼时区
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int sysSetTimezone_lib(int32 timezone)
{
	int iRet;
	
	iRet = hal_sysSetTimezone(timezone);

	return iRet;
}

/*
*Function:		sysGetTimezone_lib
*Description:	读时区
*Input:			*timezone:时区值指针，单位¼时区
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int sysGetTimezone_lib(int32 *timezone)
{
	int iRet;
	
	iRet = hal_sysGetTimezone(timezone);

	return iRet;
}



/*
*Function:		sysLOGSet_lib
*Description:	设置log等级
*Input:			level: log输出等级
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void sysLOGSet_lib(LOG_LEVEL level)
{
	sysLogSet(level);
}

/*
*Function:		sysLOGGet_lib
*Description:	获取log等级
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		log输出等级
*Others:
*/
LOG_LEVEL sysLOGGet_lib(void)
{
	return sysLogGet();
}

int8 syslogBuff[4096] = {0};

/*
*Function:		sysLOG_lib
*Description:	打印log信息
*Input:			level:log输出等级，*format:打印log内容指针
*Output:		NULL
*Hardware:
*Return:		log输出等级
*Others:
*/
void sysLOG_lib(unsigned char level, const char *format, ...)
{
	va_list vp;

	if(level <= g_ui8LogLevel)
	{		
		memset(syslogBuff, 0, sizeof(syslogBuff));
		va_start(vp, format);
		vsnprintf((int8 *)syslogBuff,4095, format, vp);
		va_end(vp);
		L610_LOG("%s", syslogBuff);

	}
		
}


/*
*Function:		sysWriteHw_lib
*Description:	写入终端硬件版本信息
*Input:			*pucInfo:指向存放版本信息的指针，len:写入字节数
*Output:		NULL
*Hardware:
*Return:		>0:写入字节数；<=0:写入失败
*Others:
*/
int sysWriteHw_lib(char *pucInfo, unsigned char len)
{
	int ret;
				
	ret = hal_nvWriteHwVersion(pucInfo, len);
			
	return ret;
}


/*
*Function:		sysReadHw_lib
*Description:	读取终端硬件标识
*Input:			NULL
*Output:		*pucInfo:存放硬件版本号的地址指针，需要预先分配16字节的空间
*Hardware:
*Return:		>0：读到的字节数；<=0:读取失败
*Others:
*/
int sysReadHw_lib(char *pucInfo)
{
	int ret;
	
	ret = hal_nvReadHwVersion(pucInfo);

	return ret;
}


/*
*Function:		sysReadHwstring_lib
*Description:	读取终端硬件版本号字符串形式
*Input:			NULL
*Output:		*pucInfo:存放硬件版本号的地址指针，需要预先分配16字节的空间
*Hardware:
*Return:		>0：读到的字节数；<=0:读取失败
*Others:
*/
int sysReadHwstring_lib(char *pucInfo)
{
	int ret;
	
	ret = hal_nvReadHwVersionString(pucInfo);

	return ret;
}


/*
*Function:		sysWriteCid_lib
*Description:	写入客户标识信息
*Input:			*pucInfo:指向存放客户标识的指针，len:写入字节数
*Output:		NULL
*Hardware:
*Return:		>0:写入字节数；<=0:写入失败
*Others:
*/
int sysWriteCid_lib(char *pucInfo, unsigned char len)
{
	int ret;
				
	ret = hal_nvWriteCustomerID(pucInfo, len);
			
	return ret;
}


/*
*Function:		sysReadCid_lib
*Description:	读取客户标识ID
*Input:			NULL
*Output:		*pucInfo:存放客户标识的地址指针，需要预先分配16字节的空间
*Hardware:
*Return:		>0:读到的字节数；<=0:读取失败
*Others:
*/
int sysReadCid_lib(char *pucInfo)
{
	int ret;
	
	ret = hal_nvReadCustomerID(pucInfo);

	return ret;
}


/*
*Function:		sysGetHalInfo_lib
*Description:	读底层库版本号
*Input:			NULL
*Output:		*halinfo:底层库版本号存储指针
*Hardware:
*Return:		<0:失败；>=0:成功，版本号长度
*Others:
*/
int sysGetHalInfo_lib(int8 *halinfo)
{
	return hal_sysGetHalInfo(halinfo);
}


/*
*Function:		sysReadBPVersion_lib
*Description:	读底包版本号
*Input:			NULL
*Output:		*BPinfo:底包版本号存储指针
*Hardware:
*Return:		<0:失败；>=0:成功，版本号长度
*Others:
*/
int sysReadBPVersion_lib(int8 *BPinfo)
{
	return hal_sysGetBPVersion(BPinfo);
}


/*
*Function:		sysGetAppInfoReg_lib
*Description:	注册读取应用版本信息的接口
*Input:			*Pgetappinfom:读取应用版本信息接口指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void sysGetAppInfoReg_lib(void (*Pgetappinfom)(char inform[512]))
{
	hal_sysGetAppInfoReg(Pgetappinfom);
}


/*
*Function:		kbHit_lib
*Description:	检测键盘缓冲区中是否有尚未取走的按键值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:缓冲区有按键值；非0:缓冲区没有键值
*Others:
*/
unsigned char kbHit_lib(void)
{
	
	return hal_keypadHit();
}


/*
*Function:		kbFlush_lib
*Description:	键盘队列清空
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void kbFlush_lib(void)
{
	hal_keypadFlush();
}


/*
*Function:		kbGetKey_lib
*Description:	读取一个键盘队列中最早放入的一个键值
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		读到的键值，没有键值则反0xFF
*Others:
*/
unsigned char kbGetKey_lib(void)
{
	uint8 iRet;
	iRet = hal_keypadGetKey();
	return iRet;
	
}


/*
*Function:		kbSetSound_lib
*Description:	设置按键板在按键时是否发声
*Input:			flag:0-不发声;其他-发声
*Output:		NULL
*Hardware:
*Return:		0-成功；<0:失败
*Others:
*/
int kbSetSound_lib(unsigned char flag)
{
	return hal_keypadSetSound(flag);
}


/*
*Function:		kbGetSound_lib
*Description:	读取按键音状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-不发声；1-发声
*Others:
*/
int kbGetSound_lib(void)
{
	return hal_keypadGetSound();
}


/*
*Function:		kbGetString_lib
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
int kbGetString_lib(unsigned char *pucStr, unsigned char ucMode, unsigned char ucMinLen, unsigned char ucMaxLen, unsigned short usTimeOutSec)
{
	return hal_kbGetString(pucStr, ucMode, ucMinLen, ucMaxLen, usTimeOutSec);
}


/*
*Function:		kbLock_lib
*Description:	设置按键锁
*Input:			mode:0-按键不会锁；1-按键立即锁定；2-30秒后自动锁定
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void kbLock_lib(int mode)
{
	
}


/*
*Function:		kbCheck_lib
*Description:	获取当前键盘状态信息
*Input:			iCmd:按键状态，0-按键锁定状态；1-按键缓冲区内的键值数；2-按键音状态
*Output:		NULL
*Hardware:
*Return:		<0:无效；
*				>=0:当iCmd=0时，0-按键未锁，1-按键已锁
*					当iCmd=1时，>=0-键值数
*					当iCmd=2时，0-不发声，1-发声
*Others:
*/
int kbCheck_lib(int iCmd)
{

}


/*
*Function:		kbLightOn_lib
*Description:	设置按键背光模式
*Input:			mode:背光模式，0-长灭模式，1-定时模式，2-长亮模式
*				countms:背光亮的时间，最大值为65535，以10ms为单位
*Output:		NULL
*Hardware:
*Return:		0-成功；其他-失败
*Others:
*/
unsigned char kbLightOn_lib(uint8_t mode, uint32_t countms)
{
	return 1;
}


/*
*Function:		kbLightOff_lib
*Description:	关闭按键背光
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void kbLightOff_lib(void)
{
	
}


/*
*Function:		kbPwrOffCbReg_lib
*Description:	按键关机回调函数注册接口
*Input:			*keys_callback_P:按键关机回调接口指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void kbPwrOffCbReg_lib(void (*keys_callback_pwroff_P)(void))
{
	hal_keypadPwrOffReg(keys_callback_pwroff_P);
}


/*
*Function:		pmSleep_lib
*Description:	使终端进入休眠状态,以降低功耗
*Input:			DownCtrl:		输入参数,指向下电控制串(需以‘\0’结尾)。
*				DownCtrl[0]:	设置对上电状态的 IC 卡是否下电：‘0’－不下电,‘1’－下电，目前均支持
*				DownCtrl[1]:	设置对上电状态的非接触卡是否下电：‘0’－不下电,‘1’－下电，目前均支持
*				DownCtrl[2]:	表示是否使能 RTC 唤醒, 0x00:不开启 RTC唤醒功能,0x01~0xff:表示 RTC 唤醒的时间，单位为分钟若输入参数为 NULL,则默认为均下电 ,RTC 唤醒功能不开启。暂不支持RTC 唤醒
*				DownCtrl[3]:	表示WIFI是否下电；0-不下电 1-下电
*Output:		NULL
*Hardware:
*Return:		0-succ; other-failed
*Others:
*/
int pmSleep_lib(uchar *DownCtrl)
{
	return hal_pmSleep(DownCtrl);
}


/*
*Function:		pmBatteryCheck_lib
*Description:	阻塞式获取电池电量（百分比制：0-100）
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		BM_ERR_CHARGING:充电中；0-100：电量百分比
*Others:
*/
int pmBatteryCheck_lib(void)
{
	int ret;
	ret = hal_pmBatGetValue();
	return ret;
}


/*
*Function:		pmGetChg_lib
*Description:	获得USB充电状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:未知; 1:充电中; 2:未接USB; 3:接USB但未充电; 4:满电
*Others:
*/
char pmGetChg_lib(void)
{
	return hal_pmGetChg();
}


/*
*Function:		pmPowerOff_lib
*Description:	设备关机
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void pmPowerOff_lib(void)
{
	hal_pmPwrOFF();
}


/*
*Function:		pmReboot_lib
*Description:	设备重启
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void pmReboot_lib(void)
{
	hal_pmPwrRST();
}


/*
*Function:		pmChgCtl_lib
*Description:	充电控制使能接口
*Input:			value:TRUE-使能；FALSE-失能
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void pmChgCtl_lib(BOOL value)
{
	hal_pmChargerCtl(value);
}


/*
*Function:		ttsSetVolume_lib
*Description:	tts设置音量
*Input:			value:音量值1-5
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void ttsSetVolume_lib(unsigned char value)
{
	hal_ttsSetVolume(value);
}


/*
*Function:		ttsQueuePlay_lib
*Description:	TTS队列播报接口
*Input:			*tts_content:播报内容指针, *tts_vol:播报音量指针，不设置则为NULL，*readmode:读数字方式指针，0-自动判断；1-按号码方式；2-按数值方式，NULL-不设置
*				encode:播报内容编码格式，0:UTF-8; 1:GB2312; 2:UNICODE
*Output:		NULL
*Hardware:
*Return:		0:成功；-1:待播报队列已满，失败
*Others:
*/
int ttsQueuePlay_lib(char *tts_content, unsigned char *tts_vol, unsigned char *readmode, char encode)
{
	return hal_ttsQueuePlay(tts_content, tts_vol, readmode, encode);
}


/*
*Function:		ttsQueuePlayAmount_lib
*Description:	TTS队列播报接口,带断码屏显示
*Input:			*tts_content:播报内容指针, *tts_vol:播报音量指针，不设置则为NULL，*readmode:读数字方式指针，0-自动判断；1-按号码方式；2-按数值方式，NULL-不设置
*				encode:播报内容编码格式，0:UTF-8; 1:GB2312; 2:UNICODE，*amount:断码屏在播报这一笔时同步显示的金额，单位分，不显示金额传NULL
*Output:		NULL
*Hardware:
*Return:		0:成功；-1:待播报队列已满，失败
*Others:
*/
int ttsQueuePlayAmount_lib(char *tts_content, unsigned char *tts_vol, unsigned char *readmode, char encode, int32 *amount)
{
	return hal_ttsQueuePlayAmount(tts_content, tts_vol, readmode, encode, amount);
}


/*
*Function:		ttsQueueClear_lib
*Description:	清播报队列
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void ttsQueueClear_lib(void)
{
	hal_ttsQueueClear();
}


/*
*Function:		ttsGetSpare_lib
*Description:	获取TTS队列剩余空间
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		-1:获取失败; -2:队列播报关闭; >=0剩余空间个数;
*Others:
*/
int ttsGetSpare_lib(void)
{
	return hal_ttsGetSpare();
}


/*
*Function:		ttsSetSpeed_lib
*Description:	tts设置语速，默认语速值为3000
*Input:			speedvalue:-32768~32767
*Output:		NULL
*Hardware:
*Return:		0-成功，其他-失败
*Others:
*/
int ttsSetSpeed_lib(int32 speedvalue)
{
	int iRet = -1;
	
	iRet = fibo_tts_voice_speed(speedvalue);
	return iRet;
}


/*
*Function:		ttsGetStatus_lib
*Description:	获取TTS播报状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-空闲，1-忙;
*Others:
*/
int ttsGetStatus_lib(void)
{

	return fibo_tts_is_playing();
}


/*
*Function:		ttsSetLanguage_lib
*Description:	选择TTS语言
*Input:			ttslangtype:0-中文; 1-英文
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int ttsSetLanguage_lib(unsigned char ttslangtype)
{
	return hal_ttsSetLanguage(ttslangtype);
}

/*
*Function:		ttsGetLanguage_lib
*Description:	读取TTS语言
*Input:			NULL
*Output:		*ttslangtype:0-中文; 1-英文
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int ttsGetLanguage_lib(unsigned char *ttslangtype)
{
	return hal_ttsGetLanguage(ttslangtype);
}

/*
*Function:		hal_ttsSetLibPath
*Description:	设置TTS语音库路径
*Input:			*libpathcn：中文语音库路径; *libpathen:英文语音库路径
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int ttsSetLibPath_lib(unsigned char *libpathcn, unsigned char *libpathen)
{
	return hal_ttsSetLibPath(libpathcn, libpathen);
}

/*
*Function:		ttsGetLibPath_lib
*Description:	读取TTS语音库路径
*Input:			ttstype:0-中文；1-英文
*Output:		*libpath：读到的语音库路径;
*Hardware:
*Return:		>=0-成功，<0-失败;
*Others:
*/
int ttsGetLibPath_lib(char *libpath, int ttstype)
{
	return hal_ttsGetLibPath(libpath, ttstype);
}


/*
*Function:		tmsUpdateFile_lib
*Description:	升级固件接口
*Input:			flag:要升级的文件类型，TMS_FLAG_DIFFOS-底包差分升级，TMS_FLAG_APP-应用APP升级, TMS_FLAG_VOS-VOS升级,
*				*pcFileName:文件名称（全路径）,*signFileName:签名文件名称，没有签名传NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败；0：成功
*Others:
*/
int tmsUpdateFile_lib(enum tms_flag flag, char *pcFileName, char *signFileName)
{
	return hal_fotaUpdate(flag, pcFileName, signFileName);
}


/*
*Function:		tmsUpdateFileCBReg_lib
*Description:	下载进度及状态读取回调接口,目前只可以显示下载VOS进度及状态
*Input:			*UpdateFileCB_P:回调接口指针
*				回调接口中：step:下载进度；status:下载状态；schedule:进度百分比；*arg:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void tmsUpdateFileCBReg_lib(void (*UpdateFileCB_P)(LOADSTEP step, int32 status, uint32 schedule, void *arg))
{

	hal_seldUpdateVOSCbReg(UpdateFileCB_P);
	
}




/*
*Function:		portOpen_lib
*Description:	打开指定的通信口
*Input:			emPort:通道号，10-usb
*				*Attr:通讯速率和格式串,如果是USB转串，不需要进行该操作。传NULL即可
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int portOpen_lib(SER_PORTNUM_t emPort, char *Attr)
{
	return hal_portOpen(emPort, Attr);
}


/*
*Function:		portClose_lib
*Description:	关闭指定的通信口
*Input:			emPort:通道号，10-usb
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int portClose_lib(SER_PORTNUM_t emPort)
{
	return hal_portClose(emPort);
}


/*
*Function:		portFlushBuf_lib
*Description:	复位通讯口,该函数将清除串口接收缓冲区中的所有数据
*Input:			emPort:通道号，10-usb
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int portFlushBuf_lib(SER_PORTNUM_t emPort)
{
	return hal_portFlushBuf(emPort);
}


/*
*Function:		portSends_lib
*Description:	使用指定的通讯口发送若干字节的数据
*Input:			emPort:通道号，10-usb;*str:发送数据指针;str_len:发送数据长度
*Output:		NULL
*Hardware:
*Return:		>=0-成功发送的字节数;<0-失败
*Others:
*/
int portSends_lib(SER_PORTNUM_t emPort, uchar *str, ushort str_len)
{
	return hal_portSends(emPort, str, str_len);
}


/*
*Function:		portRecvs_lib
*Description:	在给定的时限内，最多接收期望长度的数据
*Input:			emPort:通道号，10-usb
*				usBufLen:期望接收的字节数
*				usTimeoutMs:接收超时时间，单位ms,
*Output:		*pszBuf:接收缓冲区指针
*Hardware:
*Return:		>=0-成功发送的字节数;<0-失败
*Others:
*/
int portRecvs_lib(SER_PORTNUM_t emPort, uchar *pszBuf, ushort usBufLen, ushort usTimeoutMs)
{
	return hal_portRecvs(emPort, pszBuf, usBufLen, usTimeoutMs);
}


/*
*Function:		dpyInit_lib
*Description:	初始化段码屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-初始化成功,其他-失败
*Others:
*/
int dpyInit_lib(void)
{
	return hal_cblcdOpen(FALSE);
}


/*
*Function:		dpyCls_lib
*Description:	清除屏幕的显示内容
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功,其他-失败
*Others:
*/
int dpyCls_lib(void)
{
	return hal_cblcdClr();
}


/*
*Function:		dpySetBackLight_lib
*Description:	设置背光亮或灭
*Input:			enable:1:开启背光；其他：关闭背光。
*Output:		NULL
*Hardware:
*Return:		0-成功,其他-失败
*Others:
*/
int dpySetBackLight_lib(int enable)
{
	int iRet = -1;
	if(enable == 1)
	{
		iRet = hal_cblcdBackLightCtl(TRUE);
	}
	else

	{
		iRet = hal_cblcdBackLightCtl(FALSE);
	}
	return iRet;
}


/*
*Function:		dpyDispAmount_lib
*Description:	显示人民币金额
*Input:			amount:金额值，最大可输入 7 位数字。即 9999999。单位：分。
*Output:		NULL
*Hardware:
*Return:		0-成功，其他-失败
*Others:
*/
int dpyDispAmount_lib(int amount)
{
	return hal_cblcdDispAmount(amount);
}


/*
*Function:		dpyDispNum_lib
*Description:	显示数字
*Input:			num:数字，最大可输入7位数字只可以是整数。即 9999999
*Output:		NULL
*Hardware:
*Return:		0-成功，其他-失败
*Others:
*/
int dpyDispNum_lib(int num)
{
	return hal_cblcdDispNum(num);
}


