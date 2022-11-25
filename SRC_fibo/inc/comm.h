/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		comm.h
** Description:		通用接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _COMM_H_
#define _COMM_H_

#if 0
typedef char	int8;
typedef short	int16;
typedef int		int32;

typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
#endif
#define TRUE	true
#define	FALSE	false

#pragma pack(1) 
typedef struct{ 
	unsigned char Cmd[4]; 
	unsigned short Lc; 
	unsigned char DataIn[512] ; 
	unsigned short Le; 
}APDU_SEND_LIB; 
 
typedef struct{ 
	unsigned short LenOut; 
	unsigned char DataOut[512]; 
	unsigned char SWA; 
	unsigned char SWB; 
}APDU_RESP_LIB;
#pragma pack()


#include "fibo_opencpu.h"
#include "lwip/def.h"
#include "osi_mem.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sci_types.h"
#include <math.h>
#include "hal_base.h"
#include "hal_uart.h"
#include "hal_camera.h"
#include "hal_at.h"
#include "hal_fota.h"
#include "hal_load.h"
#include "hal_wiressl.h"
#include "hal_wiresocket.h"
#include "LC610N_hal_esp8266.h"
#include "LC610N_api_wifi.h"
#include "LC610N_scanpos_wifi.h"
#include "hal_cfg.h"
#include "hal_tts.h"
#include "hal_file.h"
#include "hal_nv.h"
#include "hal_pwr.h"
#include "hal_keys.h"
#include "hal_led.h"
#include "hal_audio.h"
#include "hal_seload.h"
#include "hal_guiicon.h"
#include "hal_pwrkey.h"
#include "hal_icon.h"
#include "hal_screen.h"
#include "hal_lcd.h"
#include "hal_colorlcd.h"
#include "hal_coloricon.h"
#include "hal_rtctiming.h"
#include "hal_VscJSON.h"
#include "hal_bmp.h"
#include "hal_security.h"
#include "hal_cblcd.h"
#include "LC610N_ringbuffer.h"
#include "hal_usb.h"
#include "hal_font.h"
#include "hal_seg.h"
#include "hal_secscreen.h"

#include "api_driverapi.h"
#include "api_screen.h"
#include "api_file.h"
#include "api_wiresocket.h"

#include "drv_lcd.h"
#include "rsaeuro.h"
#include "nn.h"
#include "rsa.h"
#include "sha160.h"
#include "md5.h"

#include "LC610N_se_communication.h"
#include "LC610N_se_basic.h"

#include "LC610N_api_gmsm.h"
#include "LC610N_api_ped.h"
#include "LC610N_api_calc.h"
#include "LC610N_api_sys.h"
#include "LC610N_api_picc.h"
#include "LC610N_api_icc.h"
#include "LC610N_scanpos_menu.h"
#include "hal_spiflash.h"
#include "formatStr.h"
#include "LC610N_api_gps.h"
#include "LC610N_api_ble.h"
#include "LC610N_api_dcep.h"
#include "LC610N_hal_dcep.h"
#include "api_secscreen.h"
#include "NMEA0183.h"
#include "gpio.h"
/*日志编译选择，0-对应log等级不参与编译,1-对应log等级参与编译*/
#define LOGLEVEL_R		1
#define LOGLEVEL_E		0
#define LOGLEVEL_W		0
#define LOGLEVEL_D		0
#define LOGLEVEL_I		0




#define COMM_LOG_LEVEL_0		LOG_LEVEL_0
#define COMM_LOG_LEVEL_1		LOG_LEVEL_1
#define COMM_LOG_LEVEL_2		LOG_LEVEL_2
#define COMM_LOG_LEVEL_3		LOG_LEVEL_3
#define COMM_LOG_LEVEL_4		LOG_LEVEL_4
#define COMM_LOG_LEVEL_5		LOG_LEVEL_5




#define	COMMERR_LOAD_UNSUPPORTEDCMD		-5000//不支持的指令
#define	COMMERR_LOAD_UNKNOWNSTRARCODE	-5001//不可识别的起始字符
#define COMMERR_LOAD_UNKNOWNCMDTYPE		-5003//不可识别的指令类别
#define COMMERR_LOAD_UNKNOWNCMD			-5004//不可识别的指令
#define	COMMERR_LOAD_LRCERR				-5005//LRC校验失败
#define COMMERR_LOAD_INCOMPLETEDATA		-5006//数据未收完


#define COMMWAR_LOAD_REPEATPKGINDEX		-5201//重复帧

#define filename(x) strrchr(x,'/')? strrchr(x,'/')+1:x




typedef struct _BUFF_STRUCT{

	uint32 write_P;//写指针
	uint32 read_P;//读指针
	int8 *buff;//buff地址
	uint32 bufflen;//buff长度
	uint32 count;//buff缓存长度
	volatile uint32 invailddatalen;//无效的数据记录
}BUFF_STRUCT;



extern volatile uint8 g_ui8LogLevel;
extern char syslogBuffDebug[4096];

int Vsnprintf(char *str, size_t size, const char *format, ...);


#define sysDelayMs(msec)				fibo_taskSleep(msec)

#define L610_LOG(fmt, ...)				__OSI_PRINTF(OSI_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

#define sysLOG(level, fmt, ...)			if(level <= g_ui8LogLevel){\
											if(level == 0){\
												memset(syslogBuffDebug, 0, 4096);\
												Vsnprintf((char *)syslogBuffDebug, 4095, "#L610#[%s]-%s-<%d>:"fmt"", filename(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__);\
												L610_LOG("%s", syslogBuffDebug);\
											}\
											else if(level == 1 && LOGLEVEL_R > 0){\
												memset(syslogBuffDebug, 0, 4096);\
												Vsnprintf((char *)syslogBuffDebug, 4095, "#L610#[%s]-%s-<%d>:"fmt"", filename(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__);\
												L610_LOG("%s", syslogBuffDebug);\
											}\
											else if(level == 2 && LOGLEVEL_E > 0){\
												memset(syslogBuffDebug, 0, 4096);\
												Vsnprintf((char *)syslogBuffDebug, 4095, "#L610#[%s]-%s-<%d>:"fmt"", filename(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__);\
												L610_LOG("%s", syslogBuffDebug);\
											}\
											else if(level == 3 && LOGLEVEL_W > 0){\
												memset(syslogBuffDebug, 0, 4096);\
												Vsnprintf((char *)syslogBuffDebug, 4095, "#L610#[%s]-%s-<%d>:"fmt"", filename(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__);\
												L610_LOG("%s", syslogBuffDebug);\
											}\
											else if(level == 4 && LOGLEVEL_D > 0){\
												memset(syslogBuffDebug, 0, 4096);\
												Vsnprintf((char *)syslogBuffDebug, 4095, "#L610#[%s]-%s-<%d>:"fmt"", filename(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__);\
												L610_LOG("%s", syslogBuffDebug);\
											}\
											else if(level >= 5 && LOGLEVEL_I > 0){\
												memset(syslogBuffDebug, 0, 4096);\
												Vsnprintf((char *)syslogBuffDebug, 4095, "#L610#[%s]-%s-<%d>:"fmt"", filename(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__);\
												L610_LOG("%s", syslogBuffDebug);\
											}\
										}


void sysLogSet(LOG_LEVEL level);
LOG_LEVEL sysLogGet(void);
//void sysLOG(uint8 level, const char *fmt, ...);


unsigned long HexToDec(const unsigned char *hex, int length);
int DecToHex(int dec, unsigned char *hex, int length);
unsigned long  BcdToDec(const unsigned char *bcd, int length);
int DecToBcd(int Dec, unsigned char *Bcd, int length);


unsigned int HexToStr(char *inchar, unsigned int len, char *outtxt);
unsigned int StrToHex(char *str,char *hex);
BYTE CalLRC(BYTE *pStr, DWORD lLength);
char* MyStrStr(char *pcSrc, char *pcDes, uint32 readP, uint32 writeP);
void CBuffFormat(BUFF_STRUCT *buff_struct);
int CBuffInit(BUFF_STRUCT *buff_struct, uint32 bufflen);
int CBuffGetWrittenLen(BUFF_STRUCT *buff_struct);
void CBuffClose(BUFF_STRUCT *buff_struct);
int32 CBuffRead(BUFF_STRUCT *buff_struct, int8 *data, uint32 len);
int32 CBuffWrite(BUFF_STRUCT *buff_struct, int8 *data, uint32 len);
int CBuffReadStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len);
int CBuffClean(BUFF_STRUCT *buff_struct, uint32 len);
int32 CBuffCopy(BUFF_STRUCT *buff_struct, int8 *data, uint32 len);
int CBuffCopyStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len);

int32 CBuffFiFoWrite(BUFF_STRUCT *buff_struct, int8 *data, uint32 len);
int CBuffFiFoReadStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len);
int CBuffFiFoCopyStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len);
int GetNumFromAscii(char *buff, uint32 offset, char *endchar, unsigned char lenth, unsigned char mode);
float Hysteresis(float activityV, float lastV, float deviationDown, float deviationUp, float criticalV, uint8 mode);
void BuffOrder(int32 *buf, uint32 len);
void Delayss(int ss);




#endif




