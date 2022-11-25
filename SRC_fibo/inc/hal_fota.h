/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_fota.h
** Description:		升级APP及固件相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_FOTA_H_
#define _HAL_FOTA_H_

#include "comm.h"

#define FOTA_LOG_LEVEL_0 	LOG_LEVEL_0
#define FOTA_LOG_LEVEL_1 	LOG_LEVEL_1
#define FOTA_LOG_LEVEL_2 	LOG_LEVEL_2
#define FOTA_LOG_LEVEL_3 	LOG_LEVEL_3
#define FOTA_LOG_LEVEL_4 	LOG_LEVEL_4
#define FOTA_LOG_LEVEL_5 	LOG_LEVEL_5


enum tms_flag{
 TMS_FLAG_BOOT = 0, 		/*表示 boot 程序*/
 TMS_FLAG_VOS,			 /*表示 VOS 程序*/
 TMS_FLAG_L1, 			/*表示 L1 PK 文件*/
 TMS_FLAG_L2, 			/*表示 L2 PK 文件*/
 TMS_FLAG_FONT, 		/*表示字库文件*/
 TMS_FLAG_MAPP,		 /*表示主控程序*/
 TMS_FLAG_ACQUIRER,	 /*表示应用的根公钥文件*/
 TMS_FLAG_APP_PK,		 /*表示应用公钥文件*/
 TMS_FLAG_APP, 			/*表示应用程序*/
 TMS_FLAG_VOS_NOFILE,	  /*表示以非文件形式存储的 VOS程序*/
 TMS_FLAG_APP_NOFILE,   /*表示以非文件形式存储的 应用程序*/
 TMS_FLAG_DIFFOS,		/*表示底包差分包程序*/
 TMS_FLAG_MAX, 			/*无效*/
};

typedef struct _UPDATEAPP_PARAM
{
	int8 *rP;
	int32 size;
	
}UPDATEAPP_PARAM;

extern struct _UPDATEAPP_PARAM g_stUpdateAppParam;
extern uint32 g_ui32SemUpdateHandle;

int32 hal_fotaAppCheck(int8 *data, uint32 len);
void hal_fotaTimer3Period(void *arg);

int32 hal_fotaUpdate(enum tms_flag flag, char *pcFileName, char *signFileName);

void FirmwareupdateTest(void);


#endif



