#ifndef LC610N_API_WIFI_H
#define LC610N_API_WIFI_H

#include "comm.h"

#define APIWIFI_LOG_LEVEL_0		LOG_LEVEL_0
#define APIWIFI_LOG_LEVEL_1		LOG_LEVEL_1
#define APIWIFI_LOG_LEVEL_2		LOG_LEVEL_2
#define APIWIFI_LOG_LEVEL_3		LOG_LEVEL_3
#define APIWIFI_LOG_LEVEL_4		LOG_LEVEL_4
#define APIWIFI_LOG_LEVEL_5		LOG_LEVEL_5


#define NO_OPEN		0
#define OPEN_SUCC	1

#define WIFIUPDATE_1_01_04		10104

#pragma pack(1)

typedef struct
{
	int iDHCPEnable;		/*DHCP使能, 0 -- 关闭 1 -- 开启*/
	char cIp[20]; 			/*静态IP---字符串参数*/
	char cNetMask[20];		/*子网掩码---字符串参数*/
	char cGateWay[20];		/*网关---字符串参数*/
//	char cDnsServer0[20];	/*DNS服务器1 --- 字符串参数*/
//	char cDnsServer1[20];   /*DNS服务器2 --- 字符串参数*/
}ST_WIFI_PARAM;//station模式下的热点WiFi参数


typedef struct
{
	int iEcn;				/*加密方式*/
	char cSsid[64]; 		/*字符串参数,AP的名字*/
	int iRssi;				/*信号强度*/
	char cBssid[20];		/*字符串参数,AP的MAC地址*/
	int iChannel;			/*信道号*/
	int iFreqOffset;		/*AP频偏*/
}ST_AP_LIST;//扫描热点信息

#pragma pack()

typedef enum
{
	eCmdDomainToIP = 0,				/*域名解析为IP*/
	eCmdNetTime,
}ENUM_WIFICtrlCmd;

struct WIFI_STRUCT
{
	int cOpenState;				/**1 --- Open  0 --- Close**/
    int cApClose;
};

typedef enum{
	AP_AIRKISS_ID = 1,
	AP_WEB_ID,
	AP_BT_ID,

}APCTYPE;


//extern uint8 ab[32];
extern struct WIFI_STRUCT g_stWifiState;
//extern uint8 cd[32];
extern uint32 g_ui32ApiWiFiSendMutex;
int wifiInit_lib(void);
int wifiOpen_lib(void);
int wifiClose_lib(void);
int wifiReset_lib(void);


int wifiScan_lib(ST_AP_LIST *pstApList, int iApCount);
int wifiAPConnect_lib(unsigned char *pucSsid, unsigned char *pucPassword);
int wifiAPDisconnect_lib(void);
int wifiSocketCreate_lib(int type);
int wifiSocketClose_lib(int sockid);////
int wifiSSLSocketCreate_lib(void);
int wifiSSLSocketClose_lib(int sockid);////

int wifiCommConnect_lib(int sockid, char *pcTcpUdp,char *pcServeraddr,char *port,int timeout);//
int wifiCommClose_lib(int sockid);//
int wifiCommSendData_lib(int sockid, char *data, int i32Datalen, int timeout, int i32Trytime);//
int wifiWaitData_lib(int sockid, char *pcBuf,int i32Buflen,int timeout);

int wifiTCPConnect_lib(int sockid,char *serveraddr,char *port,int timeout);/////
int wifiTCPClose_lib(int sockid);////
int wifiSend_lib(int sockid,char *data, int datalen, int timeout);/////
int wifiRecv_lib(int sockid, unsigned char *pucdata, unsigned short iLen, unsigned int uiTimeOut);
int wifiSSLConnect_lib(int sockid,char *serveraddr,char *port,int timeout);/////
int wifiSSLClose_lib(int sockid);/////
int wifiSSLSend_lib(int sockid, char *data, int datalen, int timeout);//////
int wifiSSLRecv_lib(int sockid, unsigned char *pucdata, unsigned short iLen, unsigned int uiTimeOut);

//int Wifi_SSLConnect(char *ssl,char *pcServeraddr,char *port,int timeout);


int wifiReadVersion_lib(char *buf,int buflen,int timeout);
int wifiGetLinkStatus_lib(void);


int wifiConfigConnectParam_lib(ST_WIFI_PARAM *pstWifiParam);
int wifiGetConnectParam_lib(ST_WIFI_PARAM *pstWifiParam);
int wifiConfigDNS_lib(char *pcDnsServer0, char *pcDnsServer1);
int wifiGetDNS_lib(char *pcDnsServer0, char *pcDnsServer1);
int wifiSetMac_lib(unsigned char *pcMacBuf);
int wifiGetMac_lib(char *pcMacBuf);
int wifiCheck_lib(ST_AP_INFO *pstApInfo);


int wifiSendCmd_lib(char *pcAtCmd, int iLen);
int wifiWaitResponse_lib(char *pcExpString, char *pcBuffOut, int iSizeOut);

int wifiInit_lib(void);
int wifiCtrl_lib(unsigned int iCmd, void *pArgIn, unsigned int iSizeIn, void *pArgOut, unsigned int iSizeOut);


void Wifi_Test(void);

int api_MenuWifi(void);
void api_wifiTest_Version(void);

int api_wifiTest_Open(void);

int wifiSetTime_lib(void);
int wifiGetApNum_lib(void);
int wifiPing_lib(char *pcIP, int iIpLen);
int wifiGetPingResult_lib(void);

void url_test(void);
int wifiGetUserVersion_lib(char *buf, int buflen);
int wifiGetUpdateState_lib(void);
int wifiUpdate_lib(char mode, char *url);
int wifiReboot_lib(int timeout);
void wifiupdate_test(void);

int wifiGetTamperStatus_lib(int timeout);
int wifiTamper_lib(char mode, int timeout);
void wifitamper_test(void);
int wifiRestore_lib(void);

int wifiAPConnectCheck_lib(void);
int wifiAPConnectType_lib(APCTYPE stConnectType, unsigned int timeout);
int wifiAPConnectQuit_lib(void);


/*
*@Brief:		配置自动休眠
*@Param IN:		0:关闭自动休眠 1:开启自动休眠
*@Param OUT:	NULL
*@Return:		0-成功；其它-失败
*/
int wifiAutoSleep_lib(int mode);
int wifiAPCheck_lib(unsigned char *pucSsid, unsigned char *pucPassword);
int wifiGetHostByName_lib(char *pcDomain, char *pcIP);
int WifiLocalDown_lib(char *pcIP, int i32Len);
int wifiUpgradeSendData_lib(uint8_t *pui8Data, int i32Len, uint8_t ui8Flag);
int wifiWebNetwork_lib(uint8_t *pui8Ssid,  uint8_t *pui8Password, unsigned int ui32Timeout);
int wifiWebNetworkQue_lib(uint8_t ui8CancelFlag);
int wifiWebUpdateQue_lib(char ui8CancelFlag);
int wifiAirkissNetwork_lib(unsigned int ui32Timeout);
int wifiAirkissNetworkQue_lib(uint8_t ui8CancelFlag);
int WifiespDownloadCrt_lib(char cType, char *data, int i32Len);
int WifiespSetSSLConfig_lib(int sockid, uint8_t ui8Mode);
int WifiespQuerySSLConfig_lib(int sockid);

#endif
