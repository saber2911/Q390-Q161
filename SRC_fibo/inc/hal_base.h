/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_base.h
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

#ifndef _HAL_BASE_H_
#define _HAL_BASE_H_

#include "comm.h"
#include "api_driverapi.h"
#include "hal_screen.h"
#include "cus_export_table.h"


#define BASE_LOG_LEVEL_0		LOG_LEVEL_0
#define BASE_LOG_LEVEL_1		LOG_LEVEL_1
#define BASE_LOG_LEVEL_2		LOG_LEVEL_2
#define BASE_LOG_LEVEL_3		LOG_LEVEL_3
#define BASE_LOG_LEVEL_4		LOG_LEVEL_4
#define BASE_LOG_LEVEL_5		LOG_LEVEL_5


#define DR_VER		"V1.01.02"

#define FN_LEN_MAX     60//FILE NAME MAX LENGTH
#define PWRLOOP_IRQ		1

#define MAINTEST_FLAG	0//TEST DEBUG

//#define __CAMERA_CONFIG		0//0-无摄像头,1-有摄像头



typedef enum
{
	GpioFunction0 = 0,
	GpioFunction1 = 1,
	GpioFunction2,
	GpioFunction3,
	GpioFunction4,
	GpioFunction5,
	GpioFunction6,
	GpioFunction7,
	GpioFunction8,
	GpioFunctionM,
	
}GPIO_FUNCTIONS;

typedef enum
{
	GpioCfgIn = 0,
	GpioCfgOut = 1,
	GpioCfgEnd,
}GPIO_CFG_T;

/*-------USER_GPIO----------PIN_INDEX---START--*/
#define VBAT3V3EN_GPIO		1//3V3电平控制

// #define WIFI_PWR_EN_GPIO	42
#define WIFI_WKUP_GPIO		31
#define WIFIRECVWKUP_GPIO	30
#define WIFI_TXD_GPIO		67//需要看下功能复用表//@@@
#define WIFI_RXD_GPIO		68//需要看下功能复用表//@@@


#define AMP_CTRL_GPIO		64 

#define FLASH_SPI_CS_PIN	124//FLASH片选

// #define CHG_ING_GPIO		141 
// #define CHG_EN_GPIO			142

// //暂时没用到//@@@
// #define CAM2V8EN_GPIO		81
// #define CAMLEDEN_GPIO		54

// #define CAMRST_GPIO			72//camera RST

#define LCDA0_GPIO			3//主屏和断码屏数据命令选择
#define LCDSPICSN_GPIO		26//主屏SPI片选

#define UART3_RXD			126
#define UART3_TXD			127

// #define CBLCD_CS_GPIO		33
// #define CBLCD_WR_GPIO		121
// #define CBLCD_DATA_GPIO		119

// #define SEGLCD_BLEN_GPIO	133//副屏的背光
// #define SEGLCDSPICSN_GPIO	13//副屏SPI片选


// #define LED_RED_GPIO		129
// #define LED_BLUE_GPIO		41
// #define LED_YELLOW_GPIO		6

// #define FLASH_SPI_CS_PIN	124

#define SE_TXD_GPIO			139
#define SE_RXD_GPIO			136


/*-------USER_GPIO----------PIN_INDEX---END--*/

#define LOGOINFOJSON_NAME		"/FFS/logoinfo.json"
#define LOGOFILENAME_DEFAULT	"/FFS/logo.bin"

#if 0
#if(LCD_DIRECTION == 0)//横屏

#define LOGODEFAULTPOSITION_X			0
#define LOGODEFAULTPOSITION_Y			32
#define COLORLOGODEFAULTPOSITION_X		100
#define COLORLOGODEFAULTPOSITION_Y		100

#elif(LCD_DIRECTION == 1)//竖屏

#define LOGODEFAULTPOSITION_X			0
#define LOGODEFAULTPOSITION_Y			40
#define COLORLOGODEFAULTPOSITION_X		55
#define COLORLOGODEFAULTPOSITION_Y		128

#endif
#endif


typedef struct _LOGOINFOJSON{

	int lcd_bg_color;
	int lcd_fg_color;
	int lcd_bl_pwm;
	int lcd_position_x;
	int lcd_position_y;
	int lcd_contrast_uc1617s;
	int lcd_contrast_st7571;

	int colorlcd_bg_color;
	int colorlcd_fg_color;
	int colorlcd_bl_pwm;
	int colorlcd_position_x;
	int colorlcd_position_y;
	int colorlcd_rotation;

	char logofile[128];

}LOGOINFOJSON;

extern uint32 g_ui32HwVersion;
extern uint32 g_ui32Timer2ID;
extern uint32 g_ui32Timer3ID;

extern uint32 g_ui32MutexLcdHandle;//刷屏防止竞争的锁
extern uint8 g_ui8LcdType;//0-黑白屏(128*96);1-彩屏(320*240)
extern uint8 g_ui8LcdLogoRefresh;//0-自己刷;1-厂家刷logo
extern struct _LOGOINFOJSON g_stLogoinfoJson;
extern cus_export_tab_t *cus_export_api;
extern unsigned int cus_export_api_ver;
extern int g_i32SpiFlashStatus;


int hal_sysATTrans(int8 *sendbuf, int sendlen, int8 *recvbuf, int timeout, int8 *cmd);

void hal_sysGetAppInfo(char appinfo[512]);
void hal_sysGetAppInfoReg(void (*Pgetappinfom)(char inform[512]));

int hal_sysGetHalInfo(int8 *halinfo);
int hal_sysGetBPVersion(int8 *bpver);
int hal_sysReadTermType(void);

unsigned long long hal_sysGetTickms(void);
void *hal_sysBaseInit(void);
int hal_sysSetSleepMode(uint8 time);
int hal_sysSetTimezone(int32 timezone);
int hal_sysGetTimezone(int32 *timezone);
int hal_sysSetRTC(RTC_time *rtctime);
int hal_sysGetRTC(RTC_time *rtctime);
int hal_sysSetTime(unsigned char *pucTime);
int hal_sysGetTime(unsigned char *ucTime);
int hal_sysSetTimeSE(unsigned char *pucTime);
int hal_sysGetTimeSE(unsigned char *ucTime);
void hal_sysCreateUsrDir(void);
int hal_sysGetHwVersion(void);


void hal_sysPdpOpenThread(void *param);
int hal_sysGetTermType(void);
int hal_ReadLogoInfoJson(char *jsonfilename, struct _LOGOINFOJSON *logoinfojson);
int hal_LogoInfoInit(void);

#endif

