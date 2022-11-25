/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_pwr.h
** Description:		电源管理相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_PWR_H_
#define _HAL_PWR_H_

#include "comm.h"

#define PWR_LOG_LEVEL_0			LOG_LEVEL_0
#define PWR_LOG_LEVEL_1			LOG_LEVEL_1
#define PWR_LOG_LEVEL_2			LOG_LEVEL_2
#define PWR_LOG_LEVEL_3			LOG_LEVEL_3
#define PWR_LOG_LEVEL_4			LOG_LEVEL_4
#define PWR_LOG_LEVEL_5			LOG_LEVEL_5


#define BM_ERR_NO_BAT 			-7501 //无电池
#define BM_ERR_CHARGING 		-7502 //充电中

#define BATLOWPWR	3500//低电量提醒电压值
#define BATV_NUM	32//64
#define BAT_ZERO	3500//电量0%时的电压
#define BAT_HUND	4090//电量100%时的电压
#define	READBAT_DELAY	10//延时读取电池电压值,单位100ms,拔线后确认充电状态需要(CHARGER_TREMBLE*100)秒，所以，延时1秒再去读取电量实际上是从拔线后延时了2s。
#define	NEEDTIME_MAX	55//快充时所需最长时间
#define	NEEDTIME_ADD_MAX	10//电池亏电时，最多增加的充电时间
#define BAT_HUNDOFFSET	50//充电后的电压-BAT_HUNDOFFSET，作为100%时的电压
#define QUICK_CHARGE	0//快充机制使能标志位，0-失能，1-使能
#define CHARGER_TREMBLE		10//充电抖动误差值，
#define VFULLTIMEDELAY		30*60*1000//100%电量最大持续时间
#define BATLOWPWROFF		1//低电量关机使能标志位

#define TEMPERHOT_LIMITVALUE 		50+5//断开充电温度值，单位1摄氏度,高温截止温度为50摄氏度，考虑NTC误差+5摄氏度
#define TEMPERHOT_LOOPTIME			10000//间隔10000ms一次检测温度
#define TEMPERHOT_CNT				10//间隔TEMPERHOT_LOOPTIME检测一次温度，持续TEMPERHOT_CNT次后断开充电


typedef struct _BAT_STR
{
	uint8 bat_checkflag;//电池检测标志位（USB未插入）0-usb线插入，1-第一次电量检测；2-非第一次电量检测；3-拔掉USB线后等待电量检测状态
	uint32 bat_buf[BATV_NUM];
	uint32 bat_buf_WP;//电池电量数组写指针
	uint32 bat_buf_RP;//电池电量数组读指针
	uint32 bat_buf_n;//电池电量存储个数
	uint32 bat_value;//电池电量
	uint32 bat_count;
	uint32 bat_checkwait_cnt;
	uint8 bat_lowpwrflag;//低电量标志位
	uint8 charger_v;//充电状态值
	uint8 charger_read;//充电线状态获取成功

}BAT_S;

typedef struct _BAT_QUICK
{
	uint32	last_batV;//记录最近一次的电池电压
	uint32	last_iChargeTime;//重启前已经充电累计时长	
	uint32	iNeedTime;//需要延时多久结束充电
	uint32	VbatFull;//动态可变满电电压值
	
	uint32	iFreeTime;//拔线累计时长
	uint32	iFreeTime_once;//一次拔线的时长
	uint32	iChargeTime_add;//中途拔线导致的增加时长
	uint32	iChargeTime;//最终充电时长
	uint32	iChargeTime_all;//充电累计时长
	uint32	iCHargeTime_once;//一次充电时长
	uint32	iChargeTime_offset;//充电累计时长偏移量
	uint8	free_flag;//拔出线标志位，0-未拔出；1-拔出；2-初始化状态
	uint8	push_flag;//插入线标志位，0-未插入；1-插入
	unsigned long long	freeusb_uTime;//拔出USB线那一刻的systick
	unsigned long long	pushusb_uTime;//插入USB线那一刻的systick
	int8	chargedone;//充满标志位
	int8	chargedone_last;//最近一次充电时间满足标志位
	uint32	writeparam_count;//写参数5min计数
	uint16	count;
	
}BAT_QUICK;


extern volatile uint8 g_ui8SystemStatus;
extern struct _BAT_STR g_stBatStr;
extern int g_iBackLightCnt;
extern uint32 g_ui32NoChargePwron;//1-充电中开机;0-其他情况下开机
extern int8 g_i8NormalFlag;

int hal_pmADRead(hal_adc_channel_t channel, uint32 *data);
int hal_pmTemperJudge(uint32 count, uint32 TemperLimit);
void hal_pmChargerCtl(BOOL value);
int32 hal_pmBatGetValue(void);
int8 hal_pmGetChargerValue(void);
char hal_pmGetChg(void);
void hal_pmBatChargerCheck(uint32 count);

void hal_pmPwrOFF(void);
void hal_pmPwrRST(void);
void hal_pmLoopInit(void);
int hal_pmSleep(uchar *DownCtrl);

int hal_pmQClrParam(void);


#endif


