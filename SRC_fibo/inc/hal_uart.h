/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_uart.h
** Description:		uart相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_UART_H_
#define _HAL_UART_H_

#include "comm.h"

#define UART_LOG_LEVEL_0		LOG_LEVEL_0
#define UART_LOG_LEVEL_1		LOG_LEVEL_1
#define UART_LOG_LEVEL_2		LOG_LEVEL_2
#define UART_LOG_LEVEL_3		LOG_LEVEL_3
#define UART_LOG_LEVEL_4		LOG_LEVEL_4
#define UART_LOG_LEVEL_5		LOG_LEVEL_5



#define UART1_PORT		0
#define UART2_PORT		1
#define UART3_PORT		2


#define BUFF_LEN_0		2048


void hal_utSEUartInit(uint32_t baudrate);




#endif

