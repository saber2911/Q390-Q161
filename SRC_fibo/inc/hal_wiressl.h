/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_wiressl.h
** Description:		4G SSL相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_WIRESSL_H_
#define _HAL_WIRESSL_H_


#include "comm.h"

#define SSL_LOG_LEVEL_0		LOG_LEVEL_0
#define SSL_LOG_LEVEL_1		LOG_LEVEL_1
#define SSL_LOG_LEVEL_2		LOG_LEVEL_2
#define SSL_LOG_LEVEL_3		LOG_LEVEL_3
#define SSL_LOG_LEVEL_4		LOG_LEVEL_4
#define SSL_LOG_LEVEL_5		LOG_LEVEL_5


extern uint8 pssltmp[256];
int hal_wiresslSetSslVer(unsigned char ucVer);
void hal_wiresslDefault(void);
int hal_wiresslSendCertFile (unsigned char ucType, unsigned char *pucData, int iLen);
int hal_wiresslSetMode(unsigned char  ucMode);
int hal_wiresslSocketCreate(void);
int hal_wiresslSocketClose(int sockid);
int hal_wiresslConnect(int sockid, char *pucDestIP, char *pucDestPort, int timeout);

int hal_wiresslSend(int sockid, unsigned char *pucData, unsigned int uiLen);
int hal_wiresslRecv(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut);
int hal_wiresslGetErrcode(void);

void hal_wiresslTest(void);

#endif

