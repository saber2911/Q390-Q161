/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_rtctiming.c
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


#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>


/*
*Function:		hal_rtctYearCommLeap
*Description:	计算平闰年
*Input:			year:年份
*Output:		NULL
*Hardware:
*Return:		0-平年;1-闰年
*Others:
*/
int hal_rtctYearCommLeap(uint32 year)
{
    int a,y;
	
    y = year;
    if(y%4 == 0)
    {    
        if(y%100 == 0)
        	if(y%400 == 0)
             	a = 1;//世纪闰年
        	else
              	a = 0;//平年
        else
             a = 1;//普通闰年
    }
    else
        a = 0;//平年
    
    return a;
}


/*
*Function:		hal_rtctSettimeofday
*Description:	设置时间，传参总秒数
*Input:			*timeval_t:timeval指针;*args:形参
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_rtctSettimeofday(struct timeval *timeval_t, void *args)
{
	int iRet = -1;
	uint32 yeartmp = 1970;
	uint8 monthtmp = 1;
	uint8 daytmp = 1;
	uint8 hourtmp = 0;
	uint8 mintmp = 0;
	uint8 sectmp = 0;
	RTC_time rtc_time;
	time_t sec_temp = 0;
	time_t	sec_t = 0;
	suseconds_t usec_t = 0;

	long days = 0;
	uint8 weekday = 0;

	while(1)
	{
		iRet = hal_rtctYearCommLeap(yeartmp);
		if(iRet == 0)sec_temp += 31536000;
		else sec_temp += 31622400;
		
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		yeartmp ++;

	}
	rtc_time.year = yeartmp;

	iRet = hal_rtctYearCommLeap(rtc_time.year);
	sec_temp = sec_t;
	for(monthtmp=1; monthtmp<=12; monthtmp++)
	{
		
		switch(monthtmp)
		{
			
			case 1:
				sec_temp += 31*86400;
			break;
			case 2:
				sec_temp += (28+iRet)*86400;
			break;
			case 3:
				sec_temp += (31)*86400;
			break;
			case 4:
				sec_temp += (30)*86400;
			break;
			case 5:
				sec_temp += (31)*86400;
			break;
			case 6:
				sec_temp += (30)*86400;
			break;
			case 7:
				sec_temp += (31)*86400;
			break;
			case 8:
				sec_temp += (31)*86400;
			break;
			case 9:
				sec_temp += (30)*86400;
			break;
			case 10:
				sec_temp += (31)*86400;
			break;
			case 11:
				sec_temp += (30)*86400;
			break;
			case 12:
				sec_temp += (31)*86400;
			break;
			
		}
		
		
		rtc_time.month = monthtmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	for(daytmp=1; daytmp<=31; daytmp++)
	{
		sec_temp += 86400;
		
		
		rtc_time.day = daytmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	for(hourtmp=0; hourtmp<=23; hourtmp++)
	{
		sec_temp += 3600;

		
		rtc_time.hour = hourtmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	for(mintmp=0; mintmp<=59; mintmp++)
	{
		sec_temp += 60;

		
		rtc_time.min = mintmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	sectmp = timeval_t->tv_sec - sec_t;
	rtc_time.sec = sectmp;

	days = timeval_t->tv_sec/86400;
	if(timeval_t->tv_sec%86400 != 0)days += 1;
	weekday = (days+3)%7;//1970-1-1是周四,+3天是周日，周日余数为0
	if(weekday == 0)weekday = 7;
	rtc_time.wDay = weekday;
	
	iRet = hal_sysSetRTC(&rtc_time);
	
	return iRet;
}


/*
*Function:		hal_rtctGettimeofday
*Description:	读取时间，总秒数
*Input:			*args:形参
*Output:		*timeval_t:timeval指针;
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_rtctGettimeofday(struct timeval *timeval_t, void *args)
{
	int iRet = -1;
	uint32 yeartmp = 1970;
	RTC_time rtc_time;
	time_t	sec_t = 0;
	suseconds_t usec_t = 0;

	iRet = hal_sysGetRTC(&rtc_time);
	sysLOG(RTCT_LOG_LEVEL_5, "%d-%d-%d %02d:%02d:%02d\n", rtc_time.year, rtc_time.month, rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);
	if(iRet < 0)
	{
		return iRet;
	}
	
	do{
		iRet = hal_rtctYearCommLeap(yeartmp);
		if(iRet == 0)sec_t += 31536000;
		else sec_t += 31622400;

		yeartmp ++;

	}while(yeartmp < rtc_time.year);

	iRet = hal_rtctYearCommLeap(rtc_time.year);
	
	switch(rtc_time.month)
	{
		case 1:

		break;
		case 2:
			sec_t += 31*86400;
		break;
		case 3:
			sec_t += (31+28+iRet)*86400;
		break;
		case 4:
			sec_t += (31+28+iRet+31)*86400;
		break;
		case 5:
			sec_t += (31+28+iRet+31+30)*86400;
		break;
		case 6:
			sec_t += (31+28+iRet+31+30+31)*86400;
		break;
		case 7:
			sec_t += (31+28+iRet+31+30+31+30)*86400;
		break;
		case 8:
			sec_t += (31+28+iRet+31+30+31+30+31)*86400;
		break;
		case 9:
			sec_t += (31+28+iRet+31+30+31+30+31+31)*86400;
		break;
		case 10:
			sec_t += (31+28+iRet+31+30+31+30+31+31+30)*86400;
		break;
		case 11:
			sec_t += (31+28+iRet+31+30+31+30+31+31+30+31)*86400;
		break;
		case 12:
			sec_t += (31+28+iRet+31+30+31+30+31+31+30+31+30)*86400;
		break;
		
	}

	sec_t += (rtc_time.day-1)*86400;
	sec_t += rtc_time.hour * 3600;
	sec_t += rtc_time.min * 60;
	sec_t += rtc_time.sec;

	timeval_t->tv_sec = sec_t;
	timeval_t->tv_usec = sec_t *1000;

	return 0;
}


/*
*Function:		hal_rtctSec2Rtc
*Description:	由hal_rtctSettimeofday演变而来，处理timeval_t的数据为RTC_time格式
*Input:			*timeval_t-timeval指针; 
*Output:		*rtc_time-RTC_time指针
*Hardware:
*Return:		0-成功，其他值-失败
*Others:
*/
int hal_rtctSec2Rtc(struct timeval *timeval_t, RTC_time *rtc_time, void *args)
{
	int iRet = -1;
	uint32 yeartmp = 1970;
	uint8 monthtmp = 1;
	uint8 daytmp = 1;
	uint8 hourtmp = 0;
	uint8 mintmp = 0;
	uint8 sectmp = 0;
	time_t sec_temp = 0;
	time_t	sec_t = 0;
	suseconds_t usec_t = 0;

	long days = 0;
	uint8 weekday = 0;

	//sysLOG(RTCT_LOG_LEVEL_5, "timeval_t->tv_sec=%ld\n", timeval_t->tv_sec);

	while(1)
	{
		iRet = hal_rtctYearCommLeap(yeartmp);
		if(iRet == 0)sec_temp += 31536000;
		else sec_temp += 31622400;
		
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		yeartmp ++;

	}
	rtc_time->year = yeartmp;

	iRet = hal_rtctYearCommLeap(rtc_time->year);
	sec_temp = sec_t;
	for(monthtmp=1; monthtmp<=12; monthtmp++)
	{
		
		switch(monthtmp)
		{
			
			case 1:
				sec_temp += 31*86400;
			break;
			case 2:
				sec_temp += (28+iRet)*86400;
			break;
			case 3:
				sec_temp += (31)*86400;
			break;
			case 4:
				sec_temp += (30)*86400;
			break;
			case 5:
				sec_temp += (31)*86400;
			break;
			case 6:
				sec_temp += (30)*86400;
			break;
			case 7:
				sec_temp += (31)*86400;
			break;
			case 8:
				sec_temp += (31)*86400;
			break;
			case 9:
				sec_temp += (30)*86400;
			break;
			case 10:
				sec_temp += (31)*86400;
			break;
			case 11:
				sec_temp += (30)*86400;
			break;
			case 12:
				sec_temp += (31)*86400;
			break;
			
		}
		
		
		rtc_time->month = monthtmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	for(daytmp=1; daytmp<=31; daytmp++)
	{
		sec_temp += 86400;
		
		
		rtc_time->day = daytmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	for(hourtmp=0; hourtmp<=23; hourtmp++)
	{
		sec_temp += 3600;

		
		rtc_time->hour = hourtmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	for(mintmp=0; mintmp<=59; mintmp++)
	{
		sec_temp += 60;

		
		rtc_time->min = mintmp;
		if(timeval_t->tv_sec < sec_temp)
		{
			sec_temp = sec_t;
			break;
		}
		sec_t = sec_temp;
		
	}

	sectmp = timeval_t->tv_sec - sec_t;
	rtc_time->sec = sectmp;

	days = timeval_t->tv_sec/86400;
	if(timeval_t->tv_sec%86400 != 0)days += 1;
	weekday = (days+3)%7;//1970-1-1是周四,+3天是周日，周日余数为0
	if(weekday == 0)weekday = 7;
	rtc_time->wDay = weekday;
	//sysLOG(RTCT_LOG_LEVEL_5, "%d-%d-%d %02d:%02d:%02d %d\n", rtc_time->year, rtc_time->month, rtc_time->day, rtc_time->hour, rtc_time->min, rtc_time->sec, rtc_time->wDay);
	iRet = 0;
	
	return iRet;
}



/*
*Function:		hal_rtctRtc2Sec
*Description:	由hal_rtctGettimeofday演变而来，处理RTC_time格式为timeval_t格式
*Input:			*rtc_time-RTC_time指针
*Output:		*timeval_t-timeval指针; 
*Hardware:
*Return:		0-成功，其他值-失败
*Others:
*/
int hal_rtctRtc2Sec(struct timeval *timeval_t, RTC_time *rtc_time, void *args)
{
	int iRet = -1;
	uint32 yeartmp = 1970;
	time_t	sec_t = 0;
	suseconds_t usec_t = 0;

	//sysLOG(RTCT_LOG_LEVEL_5, "%d-%d-%d %02d:%02d:%02d %d\n", rtc_time->year, rtc_time->month, rtc_time->day, rtc_time->hour, rtc_time->min, rtc_time->sec, rtc_time->wDay);
	
	do{
		iRet = hal_rtctYearCommLeap(yeartmp);
		if(iRet == 0)sec_t += 31536000;
		else sec_t += 31622400;

		yeartmp ++;

	}while(yeartmp < rtc_time->year);

	iRet = hal_rtctYearCommLeap(rtc_time->year);
	
	switch(rtc_time->month)
	{
		case 1:

		break;
		case 2:
			sec_t += 31*86400;
		break;
		case 3:
			sec_t += (31+28+iRet)*86400;
		break;
		case 4:
			sec_t += (31+28+iRet+31)*86400;
		break;
		case 5:
			sec_t += (31+28+iRet+31+30)*86400;
		break;
		case 6:
			sec_t += (31+28+iRet+31+30+31)*86400;
		break;
		case 7:
			sec_t += (31+28+iRet+31+30+31+30)*86400;
		break;
		case 8:
			sec_t += (31+28+iRet+31+30+31+30+31)*86400;
		break;
		case 9:
			sec_t += (31+28+iRet+31+30+31+30+31+31)*86400;
		break;
		case 10:
			sec_t += (31+28+iRet+31+30+31+30+31+31+30)*86400;
		break;
		case 11:
			sec_t += (31+28+iRet+31+30+31+30+31+31+30+31)*86400;
		break;
		case 12:
			sec_t += (31+28+iRet+31+30+31+30+31+31+30+31+30)*86400;
		break;
		
	}

	sec_t += (rtc_time->day-1)*86400;
	sec_t += rtc_time->hour * 3600;
	sec_t += rtc_time->min * 60;
	sec_t += rtc_time->sec;

	timeval_t->tv_sec = sec_t;
	timeval_t->tv_usec = sec_t *1000;
	//sysLOG(RTCT_LOG_LEVEL_5, "timeval_t->tv_sec=%ld\n", timeval_t->tv_sec);

	return 0;
}







