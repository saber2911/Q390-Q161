/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_cfg.h
** Description:		配置文件相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_CFG_H_
#define _HAL_CFG_H_
#define USER_CFG_FILE_NAME "/app/ufs/cfg_file.ini"  //根据usb下载得出路径

#include "comm.h"

#define CFG_LOG_LEVEL_0			LOG_LEVEL_0
#define CFG_LOG_LEVEL_1			LOG_LEVEL_1
#define CFG_LOG_LEVEL_2			LOG_LEVEL_2



typedef struct _PDPCFG_STR{
	uint8 apn[64];//apn
	uint8 username[64];//username
	uint8 password[64];//password
	
	
}PDPCFG_STR;

typedef struct _CONFIG_INI{
	char GPS[8];				//GPS
	char LED[8];				//LED
	char TricolorLED[8];		//00:黑白屏,01:彩屏
	char CBLCD[8];				//00:有断码屏,01:点阵屏
	char SPIFLASH[8];			//00:无flash,01:有flash
	char LCDDIRC[8];			//00:横屏,01:竖屏
	char CAMERA[8]; 			//00:无摄像头,01有摄像头
	char KEYBOARD[8];			//00:AP键盘, 01:SE键盘, "NULL":无键盘
	char TERMINAL_NAME[8];		//机器类型  Q360 / Q161
}CONFIG_INI;

extern PDPCFG_STR g_stPdpCfg;
extern struct _LOGOINFOJSON g_stLogoinfoJson;

int hal_cfgReadCfgFile(int8 *cfgname);

int hal_cfgParCfgInit(void);
int hal_cfgWriteNtpEn(int *value);
int hal_cfgReadNtpEn(int *value);

int hal_cfgWritePdpCfg(PDPCFG_STR *pdpcfg, char *apn, char *username, char *password);
int hal_cfgReadPdpCfg(PDPCFG_STR *pdpcfg);

int hal_cfgWriteTTSCfg(char *ttspath_cn, char *ttspath_en, int *ttstype);
int hal_cfgReadTTSType(int *ttstype);
int hal_cfgReadTTSPath(char *ttspath, int ttstype);

void usercfgtest(void);

void hal_config_Init(void);
void hal_throw_error(void);
void hal_reFresh_Cfg(void);
#endif

