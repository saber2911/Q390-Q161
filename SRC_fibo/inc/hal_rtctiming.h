/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_rtctiming.h
** Description:		RTC时钟转换相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/


#ifndef _HAL_RTCTIMING_H_
#define _HAL_RTCTIMING_H_

#include "comm.h"

#define RTCT_LOG_LEVEL_0		LOG_LEVEL_0
#define RTCT_LOG_LEVEL_1		LOG_LEVEL_1
#define RTCT_LOG_LEVEL_2		LOG_LEVEL_2
#define RTCT_LOG_LEVEL_3		LOG_LEVEL_3
#define RTCT_LOG_LEVEL_4		LOG_LEVEL_4
#define RTCT_LOG_LEVEL_5		LOG_LEVEL_5




int hal_rtctSec2Rtc(struct timeval *timeval_t, RTC_time *rtc_time, void *args);
int hal_rtctRtc2Sec(struct timeval *timeval_t, RTC_time *rtc_time, void *args);



#endif

