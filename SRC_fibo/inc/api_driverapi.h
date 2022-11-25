/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_driverapi.h
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

#ifndef _API_DRIVERAPI_H_
#define _API_DRIVERAPI_H_


#include "comm.h"
#include "LC610N_api_ped.h"
#include "hal_seload.h"
#include "hal_fota.h"



#define DRIVERAPI_LOG_LEVEL_0		LOG_LEVEL_0
#define DRIVERAPI_LOG_LEVEL_1		LOG_LEVEL_1
#define DRIVERAPI_LOG_LEVEL_2		LOG_LEVEL_2
#define DRIVERAPI_LOG_LEVEL_3		LOG_LEVEL_3

#define ERR_TIMEPARAM_YEAR			-901
#define ERR_TIMEPARAM_MONTH			-902
#define ERR_TIMEPARAM_DAY			-903
#define ERR_TIMEPARAM_HOUR			-904
#define ERR_TIMEPARAM_MIN			-905
#define ERR_TIMEPARAM_SEC			-906
#define ERR_TIMERDWR_FAILED			-913

#define UART_OK				0		//成功
#define UART_ERR_NOEMPTY	-5500	//发送缓冲区未空(尚余数据待发送)
#define UATR_ERR_INVALID	-5501	//非法通道号
#define UART_ERR_NOTOPEN	-5502	//通道未打开且未与任何物理端口连通
#define UART_ERR_BUF		-5503	//发送缓冲区错误(持续500ms为满状态)
#define UART_ERR_NOTFREE	-5504	//无可用的物理端口
#define UART_ERR_SEND_OVER	-5505	//发送长度超出缓冲区大小
#define UART_ERR_NOCONF		-5506	//
#define UART_ERR_DISCONN	-5507	//
#define UART_ERR_REATTACHED	-5508	//设备从主机拔除后又插上（USB DEV专用）
#define UART_ERR_CLOSED		-5509	//
#define UART_ERR_TIMEOUT	-5510	//数据接收超时
#define UART_ERR_PARAMETER	-5511	//参数错误


typedef enum
{
	LOG_LEVEL_0 = 0,//无条件打印日志
	LOG_LEVEL_1 = 1,//打印realse日志
	LOG_LEVEL_2 = 2,//打印error日志
	LOG_LEVEL_3,//打印warning日志
	LOG_LEVEL_4,//打印debug日志
	LOG_LEVEL_5,//打印info日志
	LOG_LEVEL_6,
	LOG_LEVEL_7,
	LOG_LEVEL_8,
	LOG_LEVEL_9,
	LOG_LEVEL_10,
	
}LOG_LEVEL;


typedef struct RTC_TIME
{
    uint8_t sec;   //< Second,<0-59>
    uint8_t min;   //< Minute,<0-59>
    uint8_t hour;  //< Hour,<0-23>
    uint8_t day;   //< Day,<1-31>
    uint8_t month; //< Month,<1-12>
    uint16_t year; //< Year,<2000-2127>
    uint8_t wDay;  //< Week Day,<1-7>
} RTC_time;



typedef enum{
	KEY_START = 0,
	
	KEY0 = 0x30,
	KEY1 = 0x31,		
	KEY2,			
	KEY3,			
	KEY4,			
	KEY5,			
	KEY6,			
	KEY7,			
	KEY8,			
	KEY9,			
	KEYENTER = 0x11,
	KEYCANCEL,
	KEYCLEAR,
	KEY_FN = 0x01,
	KEYSTAR = 0x25,		
	KEYSHARP = 0x26,

	KEYPOINT = 0x40,
	KEYCLEAR_LONG = 0x41,//清除键长按
	KEYPWR = 0xF0,//电源键短按
	KEYPWROFF = 0xE0,//电源键长按
	KEYF1 = 0x21,
	KEYF2 =	0x22,

	KEYDCASH = 0x42,
	KEYMENU = 0x43,
	KEYPLUS = 0x44,
	
	KEY_END,
}KEY_ID;

typedef enum{
	KEY_NULL = 0,
	KEY_SHORT,
	KEY_LONG,
	
}KEY_VALUE;

typedef enum
{
    P_DEBUG = 0,
    P_APP,
    P_WIFI,
    P_GPRS,
    P_BT = 7,
    P_USB = 10, 
    P_RS232 = 11,
    P_SER_DEFAULT = 0xFF
}SER_PORTNUM_t;


///*
//*Function:		sysLOG_lib
//*Description:	打印log信息
//*Input:			level:log输出等级，*format:打印log内容指针
//*Output:		NULL
//*Hardware:
//*Return:		log输出等级
//*Others:
//*/
//#define sysLOG_lib(level, fmt, ...)		if(level <= g_ui8LogLevel) {L610_LOG(fmt, ##__VA_ARGS__);}


void *sysBaseInit_lib(void);
void sysDelayMs_lib(int ms);
unsigned long long sysGetTicks_lib(void);
int sysSetTimeSE_lib(unsigned char *ucTime);
int sysGetTimeSE_lib(unsigned char *ucTime);
int sysSetTime_lib(unsigned char *pucTime);
int sysGetTime_lib(unsigned char *ucTime);

int sysSetRTC_lib(RTC_time *rtctime);
int sysGetRTC_lib(RTC_time *rtctime);
/*
*Function:		sysSetTimezone_lib
*Description:	设置时区
*Input:			timezone:时区值，单位¼时区
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int sysSetTimezone_lib(int32 timezone);
/*
*Function:		sysGetTimezone_lib
*Description:	读时区
*Input:			*timezone:时区值指针，单位¼时区
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int sysGetTimezone_lib(int32 *timezone);


void sysLOGSet_lib(LOG_LEVEL level);
LOG_LEVEL sysLOGGet_lib(void);
void sysLOG_lib(unsigned char level, const char *format, ...);

//int sysWriteSn_lib (char *pucInfo);
//int sysReadSn_lib (char *pucInfo);
int sysWriteHw_lib (char *pucInfo, unsigned char len);
int sysReadHw_lib (char *pucInfo);
int sysReadHwstring_lib (char *pucInfo);
int sysWriteCid_lib (char *pucInfo, unsigned char len);
int sysReadCid_lib (char *pucInfo);
int sysGetHalInfo_lib(int8 *halinfo);
int sysReadBPVersion_lib(int8 *BPinfo);
void sysGetAppInfoReg_lib(void (*Pgetappinfom)(char inform[512]));



//char ledCtlChg_lib(BOOL value);

unsigned char kbHit_lib(void);
void kbFlush_lib(void);
unsigned char kbGetKey_lib(void);
void kbPwrOffCbReg_lib(void (*keys_callback_pwroff_P)(void));
/*
*@Brief:		设置按键板在按键时是否发声
*@Param IN:		flag:0-不发声;其他-发声
*@Return:		0-成功；<0:失败
*/
int kbSetSound_lib(unsigned char flag);
/*
*@Brief:		读取按键音状态
*@Param IN:		NULL
*@Return:		0-不发声；1-发声
*/
int kbGetSound_lib(void);


int pmSleep_lib(uchar *DownCtrl);
int pmBatteryCheck_lib(void);
char pmGetChg_lib(void);
void pmPowerOff_lib(void);
void pmReboot_lib(void);
void pmChgCtl_lib(BOOL value);

void ttsSetVolume_lib(unsigned char value);
int ttsQueuePlay_lib(char *tts_content, unsigned char *tts_vol, unsigned char *readmode, char encode);
int ttsQueuePlayAmount_lib(char *tts_content, unsigned char *tts_vol, unsigned char *readmode, char encode, int32 *amount);
void ttsQueueClear_lib(void);
int ttsGetSpare_lib(void);
int ttsSetSpeed_lib(int32 speedvalue);
int ttsGetStatus_lib(void);
/*
*Function:		ttsSetLanguage_lib
*Description:	选择TTS语言
*Input:			ttslangtype:0-中文; 1-英文
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int ttsSetLanguage_lib(unsigned char ttslangtype);
/*
*Function:		ttsGetLanguage_lib
*Description:	读取TTS语言
*Input:			NULL
*Output:		*ttslangtype:0-中文; 1-英文
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int ttsGetLanguage_lib(unsigned char *ttslangtype);

/*
*Function:		hal_ttsSetLibPath
*Description:	设置TTS语音库路径
*Input:			*libpathcn：中文语音库路径; *libpathen:英文语音库路径
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int ttsSetLibPath_lib(unsigned char *libpathcn, unsigned char *libpathen);

/*
*Function:		ttsGetLibPath_lib
*Description:	读取TTS语音库路径
*Input:			ttstype:0-中文；1-英文
*Output:		*libpath：读到的语音库路径;
*Hardware:
*Return:		>=0-成功，<0-失败;
*Others:
*/
int ttsGetLibPath_lib(char *libpath, int ttstype);



/*
*@Brief:		升级固件接口
*@Param IN:		flag:要升级的文件类型，TMS_FLAG_DIFFOS-底包差分升级，TMS_FLAG_APP-应用APP升级, TMS_FLAG_VOS-VOS升级, *pcFileName:文件名称（全路径）,*signFileName:签名文件名称，没有签名传NULL
*@Param OUT:	NULL
*@Return:		<0:失败；0：成功
*/
int tmsUpdateFile_lib(enum tms_flag flag, char *pcFileName, char *signFileName);
/*
*@Brief:		下载进度及状态读取回调接口,目前只可以显示下载VOS进度及状态
*@Param IN:		*UpdateFileCB_P:回调接口指针
*				回调接口中：step:下载进度；status:下载状态；schedule:进度百分比；*arg:入参指针
*@Param OUT:	NULL
*@Return:		NULL
*/
void tmsUpdateFileCBReg_lib(void (*UpdateFileCB_P)(LOADSTEP step, int32 status, uint32 schedule, void *arg));

int portOpen_lib(SER_PORTNUM_t emPort, char *Attr);
int portClose_lib(SER_PORTNUM_t emPort);
int portFlushBuf_lib(SER_PORTNUM_t emPort);
int portSends_lib(SER_PORTNUM_t emPort, uchar *str, ushort str_len);
int portRecvs_lib(SER_PORTNUM_t emPort, uchar *pszBuf, ushort usBufLen, ushort usTimeoutMs);

/*
*Function:		dpyInit_lib
*Description:	初始化段码屏
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-初始化成功,其他-失败
*Others:
*/
int dpyInit_lib(void);

/*
*Function:		dpyCls_lib
*Description:	清除屏幕的显示内容
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功,其他-失败
*Others:
*/
int dpyCls_lib(void);

/*
*Function:		dpySetBackLight_lib
*Description:	设置背光亮或灭
*Input:			enable:1:开启背光；其他：关闭背光。
*Output:		NULL
*Hardware:
*Return:		0-成功,其他-失败
*Others:
*/
int dpySetBackLight_lib(int enable);

/*
*Function:		dpyDispAmount_lib
*Description:	显示人民币金额
*Input:			amount:金额值，最大可输入 7 位数字。即 9999999。单位：分。
*Output:		NULL
*Hardware:
*Return:		0-成功，其他-失败
*Others:
*/
int dpyDispAmount_lib(int amount);

/*
*Function:		dpyDispNum_lib
*Description:	显示数字
*Input:			num:数字，最大可输入7位数字只可以是整数。即 9999999
*Output:		NULL
*Hardware:
*Return:		0-成功，其他-失败
*Others:
*/
int dpyDispNum_lib(int num);



#endif



