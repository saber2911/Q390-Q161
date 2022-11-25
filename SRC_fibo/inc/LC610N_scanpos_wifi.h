#ifndef L610_CLDSD_WIFI_H
#define L610_CLDSD_WIFI_H

#include "comm.h"


#define WIFI_LOG_LEVEL_0		LOG_LEVEL_0
#define WIFI_LOG_LEVEL_1		LOG_LEVEL_1
#define WIFI_LOG_LEVEL_2		LOG_LEVEL_2
#define WIFI_LOG_LEVEL_3		LOG_LEVEL_3
#define WIFI_LOG_LEVEL_4		LOG_LEVEL_4
#define WIFI_LOG_LEVEL_5		LOG_LEVEL_5

#define WIFI_PORT		UART1_PORT
#define WIFI_BAUDSPEED  921600 //460800//2000000//

#define WIFIWKUP_HOLDTIME		5000

#define WIFI_ENABLE 1
#define GPS_ENABLE	2
#define UARTBUFF_LEN		1024*30
#define IPDUARTBUFF_LEN		1024*20
#define CMDUARTBUFF_LEN		1024*10
#define AP_AIRKISS	1
#define	AP_WEB		2
#define AP_BT		3
struct UART_S{
	volatile uint32 write_P;
	volatile uint32 read_P;
	int8 uart_buff[UARTBUFF_LEN];
	volatile uint32 IPD_zero_write_P;
	volatile uint32 IPD_zero_read_P;
	int8 *IPD_zero_uart_buff;
	volatile uint32 IPD_one_write_P;
	volatile uint32 IPD_one_read_P;
	int8 *IPD_one_uart_buff;
	volatile uint32 IPD_two_write_P;
	volatile uint32 IPD_two_read_P;
	int8 *IPD_two_uart_buff;
	volatile uint32 IPD_three_write_P;
	volatile uint32 IPD_three_read_P;
	int8 *IPD_three_uart_buff;
	volatile uint32 IPD_four_write_P;
	volatile uint32 IPD_four_read_P;
	int8 *IPD_four_uart_buff;
	volatile uint32 CMD_write_P;
	volatile uint32 CMD_read_P;
	int8 CMD_uart_buff[CMDUARTBUFF_LEN];
	volatile uint32 uart_buff_count;
	volatile uint32 IPD_zero_uart_buff_len;
	volatile uint32 IPD_one_uart_buff_len;
	volatile uint32 IPD_two_uart_buff_len;
	volatile uint32 IPD_three_uart_buff_len;
	volatile uint32 IPD_four_uart_buff_len;
	volatile uint32 uart_buff_len;
	volatile uint8 IPD_holdflag;//+IPD有可能分两帧下来，所以要hold一次，不然就有可能copy到 CMD_buf里面 //
	volatile uint32 CMD_cpy_num;//
	//uint32 uartIPD_templen;//暂存第一次获得+IPD时的的数据长度
	volatile uint32 invailddatalen;//无效的数据记录
	volatile uint8 wifibusy_flag;//保证发送只有一个在调用；0-闲时；1-忙时
};

struct WIFI_GPS//wifi_busying 的cmd 和data分开，因为调用ESP_SendAndWaitCmd时回复">"会把wifi_gps.wifi_busying清零，使得gps切换成功！
{
	uint8 wifi_busying;//0-WiFi闲时；1-WiFi发送cmd忙时；2-WiFi发送data忙时，WiFi忙时不可切换GPS
	uint8 wifi_gps_state;//1-wifi通信状态，2-gps通信状态
};

struct WIFI_CONNECT
{
	ST_AP_INFO	stAp;
	uint32		apconnect_count;
	uint8		apconnect_type_flag;//正在配网标志位
	uint8		apconnected_flag;//wifi已连接
	uint32		apconnect_time;//等待连接时间
	uint32		apconnect_timeout;//超时时间
	uint8		apconnect_timeout_flag;//超时标志位

	ST_AP_INFO	checkstAp;
	uint32		apconnect_checkcount;
	uint8		apconnect_checkflag;//上电检测wifi状态标志位
	uint32		apconnect_checktime;//等待连接时间
	uint32		apconnect_checktimeout;//超时时间
	uint8		apconnect_checktimeout_flag;//超时标志位
};

struct CLOSED_S{
	uint32 read_P;
	uint32 write_P;
	uint32 num;
};
extern uint32 g_ui32IPDMutexHandle;
extern volatile uint8 g_ui8WifiConnectClosed;
extern uint8 g_ui8CldsdClosemodeFlag;  
extern struct UART_S g_stWifiUart;
extern struct WIFI_CONNECT g_stWifiConnect;

int32 DR_Wifi_Pwr_EN(BOOL pwr_v);
void DR_Wifi_Pwr_init(void);


void Wifi_send(char *data, UINT32 dataLen);
void Wifi_uart_init(void);
void Wifi_init(uint32_t baudrate);
BOOL Wifi_wkup(BOOL wkup_V);

char* myStrStr(char *pcSrc, char *pcDes, uint32 readP, uint32 writeP);

int dev_wifiWaitString(char *cmd);
uint32 dev_wifiReadString(char *data,uint32 datalen,uint32 len);
int dev_cpydata_IPD(void);
int dev_wifiReadData(int i32Sockid, char *data, uint32 datalen,uint32 i32Timeout);
int8 Wifi_Set_MACAddr(char *data);
void wifi_setmacaddr_test(void);
void apitestrecv(int i32Sockid);
char  HexToAscii(unsigned char    data_hex);
//int Search_StringBInStringA(uint8 *A,uint8 *B);
int Search_StringBInStringA(char *pcStrA, char *pcStrB);
int AP_connect_check(void);
int APconnect_type(uint8 connect_type,uint32 i32Timeout);
int APconnect_quit(void);
int APconnect_check(void);
int APstatus_Check(void);
void wifi_test_demo(void);
void dev_read_CLOSED_test(void);

int dev_wifiRead_CLOSED(void);
int server_connect_test(void);
int WiFiSSL_ConnectTest(void);


void DR_BTPwrInit(void);
int8 DR_BTPwrCtl(BOOL Pwrvalue);
//void DR_WifiwkupLoop(void);

void DR_IPDmutex_lock(UINT32 mutex_id);
void DR_IPDmutex_unlock(UINT32 mutex_id);

#endif


