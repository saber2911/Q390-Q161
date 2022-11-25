#ifndef LC610N_HAL_ESP8266_H
#define LC610N_HAL_ESP8266_H

#include "comm.h"


#define HALESP8266_LOG_LEVEL_0		LOG_LEVEL_0
#define HALESP8266_LOG_LEVEL_1		LOG_LEVEL_1
#define HALESP8266_LOG_LEVEL_2		LOG_LEVEL_2
#define HALESP8266_LOG_LEVEL_3		LOG_LEVEL_3
#define HALESP8266_LOG_LEVEL_4		LOG_LEVEL_4
#define HALESP8266_LOG_LEVEL_5		LOG_LEVEL_5


#define NOSDKVERSION                0
#define RTOSSDKVERSION              1

#define  WIFI_NOT_OPEN_ERR				(-6300)		/*没有OPEN*/
#define  WIFI_OPEN_FAIL				    (-6301)		/*OPEN失败*/
#define  WIFI_GET_STATUS_FAIL	    	(-6302)		/*查询网络连接状态失败*/
#define  WIFI_NOT_APCONNECT_ERR			(-6303)		/*未连接AP热点*/
#define  WIFI_APCONNECT_FAIL	    	(-6304)		/*AP连接失败*/
#define  WIFI_NOT_TCPCONNCET			(-6305)		/*未建立TCP连接*/
#define  WIFI_TCPCONNECT_FAIL			(-6306)		/*TCP连接失败*/
#define  WIFI_SCAN_ERR					(-6307)		/*热点扫描失败*/
#define  WIFI_TCPCLOSE_FAIL				(-6308)		/*TCP断开连接失败*/
#define  WIFI_APCLOSE_FAIL				(-6309)		/*AP断开失败*/
#define  WIFI_TCP_CONNECTED				(-6310)		/*TCP连接已经建立*/
#define  WIFI_SENDDATA_FAIL_1			(-6311)		/*数据发送失败1*/
#define  WIFI_SENDDATA_FAIL_2			(-6312)		/*数据发送失败2*/
#define  WIFI_TCP_CONNECT_TIMEOUT		(-6313)		/*TCP连接超时*/
#define  WIFI_SERVER_BUSY				(-6314)		/*服务器忙*/
#define  WIFI_WAITDATA_TIMEOUT			(-6315)		/*等待服务器数据超时*/
#define  WIFI_WAITDATA_INCOMPLETE_1		(-6316)		/*接受服务器数据错误1*/
#define  WIFI_WAITDATA_INCOMPLETE_2		(-6317)		/*接受服务器数据错误2*/
#define  WIFI_WAITDATA_INCOMPLETE_3		(-6318)		/*接受服务器数据错误3*/
#define  WIFI_PRAM_ERR					(-6319)     /*接口函数入参错误*/
#define  WIFI_GETVERSION_ERR			(-6320)	    /*获取固件版本失败*/
#define  WIFI_AP_LINK_ERR				(-6321)		/*网络连接有问题*/

#define  WIFI_AP_CONNECT_TIMEOUT		(-6322)		/*AP连接超时*/
#define  WIFI_AP_CONNECT_PWERR			(-6323)		/*密码错误*/
#define  WIFI_AP_CONNECT_NOTFINDAP		(-6324)		/*找不到目标AP*/
#define  WIFI_GETDBM_ERR				(-6325)		/*获取热点信号强度失败*/
#define  WIFI_GETDBM_RES_ERR			(-6326)		/*获取热点信号强度应答错误*/
#define  WIFI_CHECK_AP_ERR				(-6327)		/*检查模块状态失败*/
#define  WIFI_ENABLE_DHCP_ERR			(-6328)		/*设置DHCP失败*/
#define  WIFI_SET_IP_ERR				(-6329)		/*设置IP失败*/
#define  WIFI_AP_CONNECTED				(-6330)		/*AP已连接*/

#define  WIFI_GETDHCP_ERR				(-6331)		/*获取DHCP状态失败*/
#define  WIFI_NoAP_NOTIP				(-6332)		/*DHCP开启情况下，未连接AP时无法获取IP地址等相关信息*/
#define  WIFI_GETIP_ERR					(-6333)		/*获取IP失败*/
#define  WIFI_GETDNS_ERR				(-6334)		/*获取DNS失败*/
#define  WIFI_DNS_DSIABLE_ERR			(-6335)		/*禁止自定义DNS失败*/
#define  WIFI_DNS_CONFIG_ERR			(-6336)		/*DNS设置失败*/
#define  WIFI_GETMAC_ERR				(-6337)		/*获取本地MAC地址失败*/

#define  WIFI_WAITSTR_NOTEXIST			(-6338)		/*没有接收到期望字符串*/
#define  WIFI_BUFF_OVER_ERR				(-6339)		/*接收缓存溢出*/

#define  WIFI_ATCIPDOMAIN_ERR			(-6340)     /*域名解析失败*/
#define  WIFI_TCP_DISCONNECT			(-6341)		/*TCP连接已断开*/

#define	 WIFI_CIPSNTPCFG_FAIL			(-6342)     /*设置时域和SNTP服务器失败*/
#define  WIFI_CIPSNTPTIME_FAIL			(-6343)		/*查询SNTP时间失败*/

#define  WIFI_RESTORE_FAIL				(-6344)		/*恢复出厂设置失败*/
#define  WIFI_GETAPINFO_FAIL			(-6345)		/*获取AP信息失败*/
#define  WIFI_SEND_CONFLICT_FAIL		(-6356)		//给WiFi发送数据冲突
#define  WIFI_ERRCODE_NOTDEFINE			(-6399)		/*错误码未定义*/

#define  WIFI_UPDATE_ERR				(-6400)		/*WiFi固件更新失败*/
#define  WIFI_GETUPDATESTA_ERR			(-6401)		/*获取WiFi更新状态失败*/
#define  WIFI_REBOOT_ERR				(-6402)		/*wifi重启失败*/
#define  WIFI_GETTAMPERSTA_ERR			(-6403)		/*wifi获取触发状态失败*/
#define  WIFI_TAMPER_ERR				(-6404)		/*设置触发模式失败*/
#define  WIFI_SETMAC_ERR                (-6405)  	/*设置本地MAC地址失败*/
#define  WIFI_UPDATE_SEND_ERR           (-6406)  	/*更新WIFI发送数据失败*/

#define WIFI_SOCKETCREATE_ERR			-6410		//wifi socket create失败
#define WIFI_SSLSOCKETCLOSE_ERR			-6411		
#define WIFI_SOCKE_NOTCREATE_ERR	    -6412		
#define WIFI_WEB_NETWORK_ERR	        -6413	    //开启web配网失败	
#define WIFI_AIRKISS_ERR	            -6414		//开启airkiss配网失败
#define WIFI_WEB_NETWORKQUE_ERR         -6415       //web配网查询状态错误
#define WIFI_AIRKISSQUE_ERR             -6416       //airkiss查询状态错误
#define WIFI_WEB_NETWORKING_ERR         -6417       //web配网前没有关闭TCP或SSL连接
#define WIFI_AIRKISS_NETWORKING_ERR     -6418       //airkiss配网前没有关闭TCP或SSL连接
#define WIFI_UPDATE_SEND_LEN_ERR        -6419       //更新WIFI固件数据包长度大于4096
#define WIFI_IPD_RAM_ERR                -6420       //创建套接字申请内存失败
#define WIFI_SOCKET_CLOSE_PARAA_ERR     -6421       //关闭套接字参数错误
#define  WIFI_LOCAL_UPDATE_FAIL         -6422
#define WIFI_UARTLOG_FAIL               -6423
#define WIFI_WEB_UPDATE_ERR             -6424       //web升级Wifi失败
#define WIFI_CRT_DOWNLOAD_PARA_ERR      -6425       //证书下载参数错误
#define WIFI_CA_CRT_DOWNLOAD_ERR        -6426       //CA证书下载失败
#define WIFI_CLIENT_CRT_DOWNLOAD_ERR    -6427       //客户端证书下载失败
#define WIFI_CLIENT_KEY_DOWNLOAD_ERR    -6428       //客户端密钥下载失败
#define WIFI_MQTT_CA_CRT_DOWNLOAD_ERR   -6429       //MQTT CA证书下载失败
#define WIFI_MQTT_CRT_DOWNLOAD_ERR      -6430       //MQTT证书下载失败
#define WIFI_MQTT_KEY_DOWNLOAD_ERR      -6431       //MQTT密钥下载失败
#define WIFI_SSL_CONFIG_PARA_ERR       -6432       //配置SSL参数错误
#define WIFI_SSL_CONFIG_ERR             -6433       //配置SSL模式错误
#define WIFI_SSL_GET_CONFIG_PARA_ERR    -6434       //获取配置SSL参数错误
#define WIFI_SSL_GET_CONFIG_ERR         -6435       //获取配置SSL模式配置错误

#define WIFI_ATTYPE			1//1-原来的指令，0-新封装的AT指令

#pragma pack(1)

typedef struct
{
	char cSsid[64]; 		/*字符串参数,AP的名字*/
	char cBssid[20];		/*字符串参数,AP的MAC地址*/
	int iChannel;			/*信道号*/
	int iRssi;				/*信号强度*/
}ST_AP_INFO;//已连接AP信息

typedef struct
{
	ST_AP_INFO stApinfo;		/*已连接AP信息*/
	int apErrFlag;
}ST_AP_INFO_ASY;//已连接AP信息


typedef struct
{
	char cBssid[5][20];		/*字符串参数,AP的MAC地址*/
	int iMacCnt;
}ST_AP_MAC;//保存扫描要连接的AP

typedef struct
{
	char ssid[64];//接入点名称
	char pwd[64];//密码
	int chl;//通道号
	int ecn;//加密方式，不支持WEP，0-OPEN,2-WPA_PSK,3-WPA2_PSK,4-WPA_WPA2_PSK
	int max_conn;//允许连接8266 softAP的最多Station数目，取值1-8
	int ssid_hidden;//默认为0，开启广播AP SSID，0-广播SSID，1-不广播SSID
	
}APINFO;//ESP8266 SoftAP当前参数
extern char g_cWifiVersionFlag;
#pragma pack()

void hal_espGpioInit(void);

int hal_wifiSend(char *pcBuff, int iLen);
int ESP_SendAndWaitCmd(char *atcmd,int atcmdLen, char *rcvbuf, int rcvbuflen, int i32Timeout, int trytime, char *cmd);


int ESP_Cmd_Init(void);

int hal_espReset(void);

int ESP_WaitNCmd(char *rcvbuf, int rcvbuflen, int i32Timeout,int argc,...);

int hal_espWifiStatus(void);

int hal_espAPScan(char *pcBuff, unsigned int uiSize);

int hal_espAPConnect(char *ssid, char *password, ST_AP_MAC *pstApMac);
int hal_espAPClose(void);

int hal_espTCPConnect(int i32Sockid, char *pcTcpUdp,char *pcServeraddr,char *pcPort,int i32Timeout);
int hal_espTCPClose(int i32Sockid);

//int ESP_SSLConnect(char *ssl,char *pcServeraddr,char *pcPort,int i32Timeout);

int hal_espSendData(int i32Sockid, char *data, int datalen, int i32Timeout, int trytime);
int hal_espWaitSeverData(int i32Sockid, char *buf,int buflen,int i32Timeout);

int hal_espReadVersion(char *buf,int buflen,int i32Timeout);
int hal_espGetUpdateState(char *buf, int buflen, int i32Timeout)
;
int hal_espUpdate(char mode, char *pcServeraddr, char *url, int i32Timeout)
;
int hal_espReboot(char *buf, int buflen, int i32Timeout)
;

int hal_espGetTamperStatus(char *buf,int buflen,int i32Timeout);
int hal_espTamper(char mode, char *buf, int buflen, int i32Timeout)
;


int hal_wifiGetSingnaldBm(void);
int hal_espCheckAp(ST_AP_INFO *pstApInfo);
int hal_espScanAp(char *pcSsid, ST_AP_MAC *pstApMac);
int hal_espStationSetMac(unsigned char *pucMac);
int hal_espStationGetMac(unsigned char *pucMac);
int hal_espStationSetDHCP(int iEnable);
int hal_espStationGetDHCP(void);
int hal_espStationSetIP(char *pcIP, char *pcGateWay, char *pcNetMask);
int hal_espStationGetIP(char *pcIP, char *pcGateWay, char *pcNetMask);

int hal_espGetIPAndMac(char *pcIPBuff, char *pcMacBuff);

int hal_espConfigDNSEnable(int iEnable);
int hal_espConfigDNS(char *pcDnsServer0, char *pcDnsServer1);
int hal_espGetDns(char *pcDnsServer0, char *pcDnsServer1);


int hal_espAutoConnect(int iEnable);

int hal_wifiWaitString(char *pcWaitStr, char *pcBuff, int iBufSize);

int hal_wifiAtCIPDOMAIN(char *pcDomain, char *pcIP);

//int hal_espUpdateSysTime(char *pcNTP, int iGmTz);
int hal_espUpdateSysTime(void);

int hal_wifiAtRESTORE(void);
int hal_wifiReset(void);
int hal_espGetApNum(void);
int hal_espPing(char *pcIP, int iIpLen);
int hal_getPingResult(void);
int hal_espSleepNotice(void);
int hal_espAPCheck(APINFO *apinfo);


#endif
