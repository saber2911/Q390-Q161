#ifndef LC610N_API_SYS_H
#define LC610N_API_SYS_H


#include "comm.h"


//基本系统的错误代码范围为-0001~ -0999。 
#define ERR_INVALID_PARAM -1 //数据缓冲不存在 
#define ERR_NOSUCHDATA -2 //数据不存在 
#define ERR_FORMAT -3 //数据格式不对 
#define ERR_LENGTH -4 //数据长度不对 
#define ERR_TIME_CNTS -5 //传入时间错误 
#define TIMER_OVERTIME -6 //定时器超时 
#define ERR_PARA_INVALID -7 //参数不合法 
#define ERR_USB_STATE -8 //USB 已接入，无法休眠 
#define ERR_APILoadFail -10 //接口加载不正确 
#define TIME_FORMAT_ERR          -900 //格式错误 
#define TIME_YEAR_ERR            -901 //年错误 
#define TIME_MONTH_ERR           -902 //月错误 
#define TIME_DAY_ERR             -903 //日错误 
#define TIME_HOUR_ERR            -904 //小时错误 
#define TIME_MINUTE_ERR          -905 //分钟错误 
#define TIME_SECOND_ERR          -906 //秒错误 
#define TIME_WEEK_ERR            -907 //周错误 
#define TIME_SET_ERR             -908 //设置错误 
#define TIME_GET_ERR             -909 //获取错误 
#define TIME_ERR_OVERFLOW  -914 //时间溢出 
#define ERR_GET_BATT_TEMP        -915  //获取电池温度失败
#define ERR_CUR_NO_BATT          -916 //未插电池

void sysBeep_lib(void);
int sysBeepF_lib(unsigned char ucMode, unsigned short usDlyTime);
#if 0
/*
*@Brief:		设置系统的日期和时间,星期值将自动算出并设置
*@Param IN:		*pucTime[输入] 日期时间参数的指针,格式为 YYMMDDhhmmss,参数为 BCD 码,共 6 个字节长。(有效时间范围：年(20)00~(20)99,月 1~12,日 1~31,小时 0~24,分钟和秒 0~59) 
*@Param OUT:	null 
*@Return:		0:成功; <0:失败
*/
int hal_sysSetTime(unsigned char* pucTime);
/*
*@Brief:		读取终端日期和时间
*@Param IN:		NULL
*@Param OUT:	ucTime[输出] 存放日期时间值的指针,以 BCD 码形式存放： time[0] 年份 time[1] 月份 time[2] 日 time[3] 小时 time[4] 分钟 time[5] 秒钟 time[6] 星期 
*@Return:		0:成功; <0:失败
*/
int hal_sysGetTime (unsigned char *ucTime);

/*
*@Brief:		延时 ms 毫秒
*@Param IN:		ms[输入] 毫秒数 
*@Param OUT:	
*@Return:		0:成功; <0:失败
*/
void sysDelayMs(int ms);
#endif

/*
*@Brief:		读取终端序列号
*@Param IN:		uiIdMask[输入] Idmask 取值如下: 0x0055FFAA ：读取序列号 0xAAFF5500：读取扩展序列号 0xFFAA0055：读取客制信息 
*@Param OUT:	pucVerInfo[ 输 出] 版本信息  
*@Return:		0:成功; <0:失败
*/
int sysReadSn_lib(unsigned int uiIdMask, unsigned char* pucInfo);

/*
*@Brief:		读取终端TUSN序列号
*@Param IN:		uiIdMask[输入] Idmask 取值如下: 0x0055FFAA ：读取序列号 0xAAFF5500：读取扩展序列号 0xFFAA0055：读取客制信息 
*@Param OUT:	pucInfo[输出] 用于存放产品序列号的缓冲区地址,需要预先分配 64 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysReadTUSN_lib(unsigned int uiIdMask, unsigned char* pucInfo);


/*
*@Brief:		读取终端的版本信息
*@Param IN:		uiId[输入] 0  boot 版本 1  vos 版本 2  硬件配置版本 3  tms 版本 4  Lib 版本 5  HVN 和 FVN 版本 
*@Param OUT:	pucInfo[输出] 用于存放产品序列号的缓冲区地址,需要预先分配 64 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysReadVerInfo_lib(unsigned int uiId, unsigned char* pucVerInfo);

void sysGetRandom_lib (unsigned int lenth, unsigned char *pucRandom);

/*
*@Brief:		读取终端配置信息,信息缓冲区应不少于30个字节
*@Param IN:		无
*@Param OUT:	out_info [输出]	
				out_info[0]	是否支持国密
				out_info[1]	是否支持蜂鸣器
				out_info[2]	是否支持闪烁灯
				out_info[3]	是否有触屏
				out_info[4]	是否支持磁条卡
				out_info[5]	是否支持接触IC卡
				out_info[6]	是否支持非接卡
				out_info[7]	是否支持蓝牙
				out_info[8]	是否支持断码屏
				out_info[9]	PSAM卡配置信息      0－不支持，－卡槽1，2－卡槽2，3－卡槽1、2
				out_info[10]	LAN(TCP/IP)模块配置信息
				out_info[11]	GPS模块配置信息
				out_info[12]	4G/2G模块配置信息        0－不支持，1－4G/2G，2－4G，3－2G
				out_info[13]	WI-FI模块配置信息
				out_info[14]	是否支持显示屏
				out_info[15]	是否支持打印机（1-热敏、2-针打、0-无）
				out_info[16]-[29]	保留
*@Return:		>=0:返回终端信息有效字节长度; <0:失败
*/
int sysGetTermInfo_lib (unsigned char *out_info);

/*
*@Brief:		从SE读取终端机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型，如：Q360,需要预先分配 20 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysGetTermType_se(char *out_type);

/*
*@Brief:		读取终端机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型，如：Q360,需要预先分配 20 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysGetTermType_lib(char *out_type);

/*
*@Brief:		读取按键适配的内部机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型，如：Q360,需要预先分配 20 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysGetKeyboardType_lib(char *out_type);



/*
*@Brief:		读取终端机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型完整信息。 
*@Return:		0:成功; <0:失败
*/
int sysGetTermTypeImperInfo_lib(char *out_type);

/*
*@Brief:		获取内存堆空间	
*@Param IN:		null 
*@Param OUT:	size[输出]:内存池总大小;avail_size[输出]:当前可用内存大小;max_block_size[输出]:实际可分配最大块大小;
*@Return:		0:成功; <0:失败
*/
int sysGetHeapInfo_lib(uint32_t * size,uint32_t * avail_size,uint32_t * max_block_size);

/*
*@Brief:		读取国密版本号
*@Param IN:		null 
*@Param OUT:	pcGmVersion[输出] 国密版本号。
*@Return:		>:成功,版本号字节; <0:失败
*/
int sysReadGmVersion_lib(char *pcGmVersion);

/*
*@Brief:		获取设备温度
*@Param IN:		null 
*@Param OUT:	pscTemp[输出] 温度,1字节  -20℃ < pscTemp < 80℃
*@Return:		=0:成功; <0:失败
*/
int sysGetBattChargTemp_lib(signed char *pscTemp);
#endif
