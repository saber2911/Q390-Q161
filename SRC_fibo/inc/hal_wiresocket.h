/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_wiresocket.h
** Description:		4G 通信及socket相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_WIRESOCKET_H_
#define _HAL_WIRESOCKET_H_

#include "comm.h"

#define SOCKET_LOG_LEVEL_0		LOG_LEVEL_0
#define SOCKET_LOG_LEVEL_1		LOG_LEVEL_1
#define SOCKET_LOG_LEVEL_2		LOG_LEVEL_2
#define SOCKET_LOG_LEVEL_3		LOG_LEVEL_3
#define SOCKET_LOG_LEVEL_4		LOG_LEVEL_4
#define SOCKET_LOG_LEVEL_5		LOG_LEVEL_5


#define SOCKET_SSL_OK					0//成功
#define ERR_FUNC_PARAM_INVALID			-6100//参数无效
#define ERR_NOT_OPEN					-6101//未执行OPEN
#define ERR_NOT_TCPINIT					-6102//未执行TCPINIT
#define ERR_NOT_TCPCONNECT				-6103//未执行TCPCONNECT
#define ERR_TCP_NOT_CONNECT				-6104//未建立TCP连接
#define ERR_TIMEOUT						-6105//等待超时，没有接收到任何数据
#define ERR_NO_CARD						-6106//没有SIM卡
#define ERR_POWER_FAIL					-6107//模块上电异常
#define ERR_TCP_CONNECT_NOT_OK			-6108//TCP没有连接
#define ERR_CRAD_TYPE					-6109//卡槽内插入的不是SIM卡

#define ERR_AT_CHECK_FAIL				-6110//AT命令通信异常
#define ERR_AT_CGMM_FAIL				-6111//获取模块型号失败
#define ERR_TYPE_UNKNOWN				-6112//模块型号未知
#define ERR_AT_IPR_115200_FAIL			-6113//设置115200波特率失败
#define ERR_AT_IPR_921600_FAIL			-6114//设置921600波特率失败
#define ERR_2G_AT_QIDEACT_FAIL			-6115//关闭GPRS PDP场景失败（去激活失败）
#define ERR_2G_AT_QIFGCNT_FAIL			-6116//配置前置场景失败
#define ERR_2G_AT_QICSGP_FAIL			-6117//配置连接方式失败
#define ERR_AT_CPIN_FAIL				-6118//查询SIM卡状态失败
#define ERR_2G_AT_CGREG_FAIL			-6119//查询GPRS网络注册状态失败
#define ERR_2G_AT_QIOPEN_FAIL			-6120//建立TCP连接指令格式错误
#define ERR_2G_TCP_CONNECTED			-6121//TCP连接已经存在
#define ERR_TCP_CONNECT_FAIL			-6122//TCP连接失败
#define ERR_2G_TCP_CONNECTING			-6123//TCP正在连接
#define ERR_2G_AT_QICLOSE_FAIL			-6124//断开TCP连接失败
#define ERR_2G_AT_QISEND_FAIL			-6125//指定待发送数据长度失败
#define ERR_2G_SEND_DATA_FAIL			-6126//发送数据失败
#define ERR_2G_AT_QIHEAD_FAIL			-6127//设置数据显示头信息失败
#define ERR_2G_IPHEAD_WRONG				-6128//帧头接收错误
#define ERR_2G_DATA_LOST				-6129//接收异常，帧数据丢失
#define ERR_2G_ATI_FAIL					-6130//获取模块信息失败
#define ERR_2G_AT_GSN_FAIL				-6131//获取IMEI失败
#define ERR_2G_CSQ_UNKNOWN				-6132//信号强度未知
#define ERR_2G_AT_CSQ_FAIL				-6133//查询信号强度失败
#define ERR_2G_AT_CFUN_FAIL				-6134//设置功能模式失败
#define ERR_2G_AT_QSCLK_FAIL			-6135//休眠模式设定失败
#define ERR_2G_AT_QVBATT_FAIL			-6136//设置电压阈值失败
#define ERR_2G_AT_CMEE_FAIL				-6137//启用错误码上报失败
#define ERR_2G_AT_QIREGAPP_FAIL			-6138//启动任务失败
#define ERR_2G_AT_QIACT_FAIL			-6139//激活移动场景失败
#define ERR_2G_AT_QISTAT_FAIL			-6140//查询当前连接状态
#define ERR_2G_TCP_CONNCET_CLOSE		-6141//TCP连接关闭
#define ERR_AT_CIMI_FAIL				-6142//查询国际移动台设备标识(IMSI)失败
#define ERR_2G_AT_QENG_FAIL_1			-6143//开启工程模式失败
#define ERR_2G_AT_QENG_FAIL_2			-6144//查询小区信息失败
#define ERR_2G_AT_QENG_FAIL_3			-6145//小区信息响应接收错误
#define ERR_2G_AT_QENG_FAIL_4			-6146//关闭工程模式失败
#define ERR_2G_AT_QINDI_FAIL			-6147//配置缓存接收失败
#define ERR_2G_AT_QIRD_FAIL				-6148//读取数据失败
#define ERR_2G_TCP_CONNECT_STATUS_ERR	-6149//建立TCP连接时 模块状态异常
#define ERR_2G_TCP_CONNECT_LOST			-6150//TCP连接已断开，需重连
#define ERR_2G_AT_QNITZ_FAIL			-6151//启用时间同步失败
#define ERR_2G_AT_CCLK_FAIL				-6152//获取模块时间失败
#define ERR_MODULE_NOT_FUC				-6153//当前模块暂不支持此功能
#define ERR_2G_AT_QIDNSGIP_FAIL			-6154//域名解析失败
#define ERR_2G_AT_CTZU_FAIL				-6155//同步网络时间以及RTC时间同步失败
#define ERR_ATS0_FAIL					-6156//自动应答前振铃次数设定失败
#define ERR_2G_AT_QIDNSIP_FAIL			-6157//配置IP连接或域名连接失败
#define ERR_AT_QLTS_NOT_SUPPORT			-6158//当地运营商不支持基站同步时间
		
		
#define ERR_4G_AT_CPIN_FAIL				-6201//查询SIM卡状态失败
#define ERR_4G_AT_CGATT_FAIL			-6202//查询GPRS附着状态失败
#define ERR_4G_AT_CIPQSEND_FAIL			-6203//设置快发模式失败
#define ERR_4G_AT_CIPHEAD_FAIL			-6204//设置接收显示IP头失败
#define ERR_4G_AT_CIPSHUT_FAIL			-6205//关闭移动场景失败
#define ERR_4G_AT_CSTT_FAIL				-6206//启动任务并设置接入点APN、用户名、密码失败
#define ERR_4G_AT_CIICR_FAIL			-6207//激活移动场景失败
#define ERR_4G_TCP_CONNECTED			-6208//TCP连接已经存在
#define ERR_4G_TCP_CONNECT_FAIL			-6209//建立TCP连接失败
#define ERR_4G_TCP_CONNECTING			-6210//正在建立TCP连接
#define ERR_4G_AT_CIPCLOSE_FAIL			-6211//关闭TCP连接失败
#define ERR_4G_AT_CIPSEND_FAIL			-6212//指定带发送数据长度失败
#define ERR_4G_SEND_DATA_FAIL			-6213//发送数据失败
#define ERR_4G_IPHEAD_WRONG				-6214//数据帧头接收失败
#define ERR_4G_DATA_LOST				-6215//接收异常，帧数据丢失
#define ERR_4G_AT_CSQ_FAIL				-6216//查询信号强度失败
#define ERR_4G_CSQ_UNKNOWN				-6217//信号强度未知
#define ERR_4G_AT_CIFSR_FAIL			-6218//查询本地IP失败
#define ERR_ATE0_FAIL					-6219//关闭指令回显失败
#define ERR_4G_AT_CGMR_FAIL				-6220//查询固件版本失败
#define ERR_4G_AT_CGSN_FAIL				-6221//获取IMEI失败
#define ERR_4G_AT_CFUN_FAIL				-6222//功能模式设定失败
#define ERR_4G_AT_CIPSTATUS_FAIL		-6223//查询当前连接状态失败
#define ERR_4G_TCP_CONNCET_CLOSE		-6224//TCP连接断开
#define ERR_4G_TCP_CLOSING				-6225//TCP正在断开连接
#define ERR_4G_AT_CGREG_FAIL			-6226//查询当前GPRS注册状态
#define ERR_4G_AT_QISTATE_FAIL			-6227//查询模块连接状态失败
#define ERR_4G_AT_CEREG_FAIL			-6228//查询网络注册失败
#define ERR_4G_AT_QICSGP_FAIL			-6229//设置APN失败
#define ERR_4G_AT_QISEND_FAIL			-6230//设置发送数据指令失败
#define ERR_4G_AT_QIRD_FAIL				-6231//确认接收数据长度失败
#define ERR_4G_READ_DATA_FAIl			-6232//读取数据失败
#define ERR_4G_AT_CIPRXGET_FAIL			-6233//手动指令获取数据失败
#define ERR_4G_AT_CIPRXGET_4_FAIL		-6234//查询未读数据长度失败
#define ERR_4G_AT_CIPRXGET_2_FAIL		-6235//读取数据失败
#define ERR_4G_AT_CIPMODE_FAIL			-6236//选择应用模式失败
#define ERR_AT_QISWTMD_FAIL				-6237//设置接收模式失败
#define ERR_4G_AT_CNTP_FAIL				-6238//同步网络时间失败
#define ERR_4G_AT_QENG_1_FAIL			-6239//查询服务小区信息失败
#define ERR_4G_AT_QENG_2_FAIL			-6240//查询相邻小区信息失败
#define ERR_4G_AT_QNTP_FAIL				-6241//连接NTP服务器失败
#define ERR_4G_AT_QLTS_FAIL				-6242//查询同步时间失败
#define ERR_4G_AT_QNWINFO_FAIL			-6243//查询网络信息失败
#define ERR_4G_AT_BUFFOVER				-6244//传参buff太小，溢出
#define ERR_4G_AT_SOCKCREATE_FAIL		-6245//创建socket失败
#define ERR_4G_AT_SOCKCOLOSE_FAIL		-6246//关闭socket失败
#define ERR_4G_AT_CHECK_FAIL			-6247//查询失败
#define ERR_4G_AT_SSLSOCK_FAIL			-6248//SSL socket 失败
#define ERR_4G_AT_SOCKCHECKERR_FAIL		-6249//读取socketerr原因失败
#define ERR_4G_AT_NOREG_FAIL			-6250//网络未注册
typedef enum{

	CID1 = 1,
	CID2 = 2,
	CID3,
	CID4,
	CID5,
	CID6,
	CID7,
	CIDMAX,
	
}CID_NUM;

typedef struct CELLINFO_STR{
	int32 MCC;
	int32 MNC;
	int32 LAC;
	uint32 CellID;
	int32 RxDbm;
}CellInfo_Str;

typedef struct CELLINFOALL_STR{

	int8 cellinfonum;
	CellInfo_Str cellinfo_str[16];

}CellInfoAll_Str;

extern uint32 g_ui32MutexPdpOpenHandle;
extern struct timeval g_stSocketTimeval;
extern fd_set g_SocketReadfd;
extern fd_set g_SocketErrfd;
extern fd_set g_SocketWritefd;

int hal_GetBpVersion(void);
int hal_JudgeBpVersion(void);

int hal_wiresockNtp(char *pucIP, unsigned int uiPort, uint32 timeout);
int hal_wiresockGetDNS(char *pri_dns, char *sec_dns);
int hal_wiresockSetDNS(const char *pri_dns, const char *sec_dns);

int hal_wirePdpWriteParam(char *apn, char *username, char *password);
int hal_wiresockPppOpenex(unsigned char *pucApn, unsigned char *pucUserName, unsigned char *pucPassword, unsigned int timeout);
int hal_wiresockPppOpen(unsigned char *pucApn, unsigned char *pucUserName, unsigned char *pucPassword);
int hal_wiresockCheckPppDial(void);
int hal_wiresockSocketCreate(int nProtocol);
int hal_wiresockSocketClose(int sockid);
int hal_wiresockTcpConnect(int sockid, char *pucIP, char *pucPort, int timeout);
int hal_wiresockSend(int sockid, unsigned char *pucData, unsigned int uiLen);
int hal_wiresockRecv(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut);
int hal_wiresockGetIMSIAndIMEI(unsigned char *pucIMSI, unsigned char *pucIMEI);
int hal_wiresockGetCSQ(int *rssi, int *ber);
int hal_wiresockGetSingnal(void);
int hal_wiresockGetCCID(unsigned char *ccid);
int hal_wiresockGetSimStatus(unsigned char *status);
int hal_wiresockGetRegInfo(unsigned char *status);
int hal_wiresockGetPdpStatus(char cid, unsigned char *status);
int hal_wiresockPppClose(void);

void hal_wiresockSetNoBlock(int sockfd);
void hal_wiresockSetBlock(int sockfd);

int hal_wiresockGetGSMorLTE(void);
/*
*@Brief:		设置注册的是4G还是2G网络,掉电不保存,切换网络后必须重新pdpOpen
*@Param IN:		mode:0-Auto,LTE优先;1-LTE only;2-GSM only
*@Param OUT:	NULL
*@Return:		<0:失败；>=0:成功
*/
int hal_wiresockSetGSMorLTE(int mode);
/*
*@Brief:		设置注册的是4G还是2G网络,掉电保存,切换网络后必须重新pdpOpen
*@Param IN:		mode:0-Auto,LTE优先;1-LTE only;2-GSM only
*@Param OUT:	NULL
*@Return:		<0:失败；>=0:成功
*/
int hal_wiresockSetGSMorLTEex(int mode);


//int hal_wiresockGetCellInfo(opencpu_cellinfo_t *cellinfo);
int hal_wiresockGetCellInfo(      CellInfoAll_Str *cellinfoall_info, uint32 timeout);
/*
*@Brief:		获取注网IP
*@Param IN:		iIPlen= pcIP的内存空间大小
*@Param OUT:	pcIP：IP值，iIPlen返回IP长度
*@Return:		0:成功；<0：失败(-1 未注网，-2 pcIP空间不够；
*/
int hal_wiresockGetIP(unsigned char *pcIP, int *iIPlen);
int hal_wiresockGetHostByName(char *Domainname, char *addr);

void hal_wiresockResCallback(GAPP_SIGNAL_ID_T sig, va_list arg);
void hal_wiresockSocketTest(void);

int hal_wirelessSetNtpEn(int *value);
int hal_wirelessGetNtpEn(void);


#endif



