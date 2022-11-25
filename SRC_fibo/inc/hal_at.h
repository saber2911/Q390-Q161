/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_at.h
** Description:		模组AT指令相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_AT_H_
#define _HAL_AT_H_

#include "comm.h"

#define AT_LOG_LEVEL_0		LOG_LEVEL_0
#define AT_LOG_LEVEL_1		LOG_LEVEL_1
#define AT_LOG_LEVEL_2		LOG_LEVEL_2
#define AT_LOG_LEVEL_3		LOG_LEVEL_3
#define AT_LOG_LEVEL_4		LOG_LEVEL_4
#define AT_LOG_LEVEL_5		LOG_LEVEL_5



#define ATCB_BUFF_LEN		2048

extern uint8 g_ui8ATFlag;
extern uint32 Sem_AT_signal;
extern struct _BUFF_STRUCT g_stATCbBuffStruct;

void hal_atResCallback(INT8 *buf, UINT16 len);
int hal_atSend(int8 *buf, uint16 len);


#endif


