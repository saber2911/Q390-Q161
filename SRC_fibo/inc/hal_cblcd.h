/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_cblcd.h
** Description:		断码屏相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_CBLCD_H_
#define _HAL_CBLCD_H_

#include "comm.h"

#define CBLCD_LOG_LEVEL_0		LOG_LEVEL_0
#define CBLCD_LOG_LEVEL_1		LOG_LEVEL_1
#define CBLCD_LOG_LEVEL_2		LOG_LEVEL_2
#define CBLCD_LOG_LEVEL_3		LOG_LEVEL_3
#define CBLCD_LOG_LEVEL_4		LOG_LEVEL_4
#define CBLCD_LOG_LEVEL_5		LOG_LEVEL_5



int hal_cblcdBackLightCtl(BOOL value);
int hal_cblcdClr(void);
int hal_cblcdDispAmount(int amount);
int hal_cblcdDispNum(int num);
int hal_cblcdOpen(int backlight);
int hal_cblcdClose(void);


void CblcdTest(void);



#endif

