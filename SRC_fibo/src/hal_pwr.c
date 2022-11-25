/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_pwr.c
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

#include "comm.h"
#include "dev_cblcd.h"

extern unsigned char gpsPowerUpFlag;
extern unsigned char g_ucSeWakeupCallBackFlag;
struct _BAT_STR g_stBatStr;
struct _BAT_QUICK g_stBatQuick;

volatile uint8 g_ui8SystemStatus = 1;//系统状态标志位：0-关机; 1-充电循环中; 2-正常启动;


static volatile uint8 g_ui8VBatFullRefresh = 0;//刷新满电电压值标志位
static volatile uint8 g_ui8ChgDetIrqEn=0;//CHG_DET中断使能标志位
 
static volatile uint8 g_ui8ChgCount = 0;//单单charger1和charger2缓存容易出现问题，比如charger1=1，charger2=0,0b01,
					//插入线之后应该charger1=0，charger2=1,0b10,但是由于两个变量变化不同步，所以需要在对bat_s.charger_v做一缓存，

static int32 g_i32BatAdcBuf[4]={0};
static volatile int32 g_i32BatAdcVal = 0;
static volatile uint8 g_ui8BatAdcCnt = 0;

static volatile uint8 g_ui8ChgTmp = 0;

static uint32 g_ui32ChgFullCnt = 0;
static uint32 g_ui32ChgNoFullCnt = 0;
static uint8 g_ui8ChgValueLast = 1;

static uint8 g_ui8VfullCheckFlagQ = 0;
static unsigned long long g_ui32VfullTimeQ = 0;

static int g_iIconSuspend = 0;//0-icon run 1-suspend icon display

static int g_iBigBatIconTmp = 0;
static int g_iTimer3_1000ms = 0;
int g_iBackLightCnt = 1;//关机充电时背光点亮时间计时

uint32 g_ui32NoChargePwron = 0;//1-充电中开机;0-其他情况下开机
int8 g_i8NormalFlag=0;



#define BATPARAM_NAME		"/app/ufs/BATParam.txt"


/*
*Function:		hal_pmADInit
*Description:	AD初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmADInit(void)
{
	fibo_hal_adc_init();
}


/*
*Function:		hal_pmADRead
*Description:	读某个通道的AD采样值
*Input:			channel:通道,adc0-channel2 adc1-channel3 adc2-channel1
*Output:		*data:读到的值输出指针
*Hardware:
*Return:		0-成功;other-失败
*Others:		
*/
int hal_pmADRead(hal_adc_channel_t channel, uint32 *data)
{
	int32 iRet;
	iRet = fibo_hal_adc_get_data_polling(channel, data);
	if(iRet < 0)
	{	
		sysLOG(PWR_LOG_LEVEL_2, "<ERR> fibo_hal_adc_get_data_polling failed! iRet=%d\r\n", iRet);
		return iRet;
	}
	
	return iRet;
	
}

uint32 TemperHy_count = 0;
uint32 TemperHot_Flag = 0;//过热标志，0-正常，1-过热
uint32 TemperTimercount = 0;


/*
*@brief:电池充电温度控制，温度高于TemperLimit值时断开充电，否则正常充电
*@param1:count:DR_TemperJudge此接口为多少毫秒被调用一次，TemperLimit:截止充电温度值，单位摄氏度
*@return:0-成功，other-失败
*/
int hal_pmTemperJudge(uint32 count, uint32 TemperLimit)
{
	int iRet = -1;
	char temperT = 0;

	if(1 == fibo_get_Usbisinsert())//插入USB线
	{
		TemperTimercount ++;
		if((TemperTimercount*count) > TEMPERHOT_LOOPTIME)
		{
			TemperTimercount = 0;
			iRet = sysGetBattChargTemp_lib(&temperT);
			sysLOG(PWR_LOG_LEVEL_4, "sysGetBattChargTemp_lib,iRet=%d,temperT=%d\r\n", iRet, temperT);
			if(iRet < 0)
				return -1;
			
			if(TemperHot_Flag == 0)//正常使能充电状态下
			{
				if(temperT > TemperLimit)//温度过高
				{
					TemperHy_count ++;
					if(TemperHy_count > TEMPERHOT_CNT)
					{
						TemperHot_Flag = 1;
						TemperHy_count = 0;
						hal_pmChargerCtl(FALSE);//温度过热，断开充电
						sysLOG(PWR_LOG_LEVEL_1, "<WARN> COLSE CHARGE! TemperHy_count=%d,DR_GetTemperR:%d\r\n", TemperHy_count, iRet);
					}
				}
				else
				{
					TemperHy_count = 0;
				}
			}
			else//过热断开充电状态下
			{
				if(temperT < TemperLimit)//温度正常
				{
					TemperHy_count ++;
					if(TemperHy_count > TEMPERHOT_CNT)
					{
						sysLOG(PWR_LOG_LEVEL_1, "<WARN> OPEN CHARGE! TemperHy_count=%d,DR_GetTemperR:%d\r\n", TemperHy_count, iRet);
						TemperHot_Flag = 0;
						TemperHy_count = 0;
						hal_pmChargerCtl(TRUE);//温度恢复正常，使能充电
					}
				}
				else
				{
					TemperHy_count = 0;
				}
			}
		}
	}
	else//拔出USB线
	{
		TemperTimercount = 0;
		TemperHot_Flag = 0;
		TemperHy_count = 0;
		hal_pmChargerCtl(TRUE);//温度恢复正常，使能充电
	}

	return 0;
}


/*
*Function:		hal_pmBatFiFo
*Description:	电池电量FIFO操作
*Input:			vbatalue:读取到的一次电池电量
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmBatFiFo(uint32 vbatalue)
{
	if(g_stBatStr.bat_checkflag>=1)
	{
		g_stBatStr.bat_buf[g_stBatStr.bat_buf_WP]=vbatalue;
		if(g_stBatStr.bat_buf_WP==g_stBatStr.bat_buf_RP && g_stBatStr.bat_buf_n!=0)
		{
			g_stBatStr.bat_buf_RP++;
			if(g_stBatStr.bat_buf_RP==BATV_NUM)g_stBatStr.bat_buf_RP=0;
		}
		g_stBatStr.bat_buf_WP++;
		if(g_stBatStr.bat_buf_WP==(BATV_NUM))g_stBatStr.bat_buf_WP=0;
		
		if(g_stBatStr.bat_buf_n<BATV_NUM)
		{
			g_stBatStr.bat_buf_n++;
		}
		
		sysLOG(PWR_LOG_LEVEL_4, "bat_buf_WP:%d,bat_buf_RP:%d,bat_buf_n:%d\r\n",g_stBatStr.bat_buf_WP,g_stBatStr.bat_buf_RP,g_stBatStr.bat_buf_n);
	}
	else
	{
		if(g_stBatStr.bat_buf_n>0)
		{
			g_stBatStr.bat_buf_n--;
			g_stBatStr.bat_buf_RP++;
			if(g_stBatStr.bat_buf_RP==BATV_NUM)g_stBatStr.bat_buf_RP=0;

		}
		
		sysLOG(PWR_LOG_LEVEL_4, "bat_buf_WP:%d,bat_buf_RP:%d,bat_buf_n:%d\r\n",g_stBatStr.bat_buf_WP,g_stBatStr.bat_buf_RP,g_stBatStr.bat_buf_n);
	}
}


/*
*Function:		hal_pmCollectBatAdcV
*Description:	采集电池电压
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmCollectBatAdcV(void)
{
	int32 adc_bat = 0;

	if(g_i32BatAdcVal == 0)
	{
		
		while(1)
		{
			do
			{
				hal_pmADRead(3, &adc_bat);
				adc_bat=adc_bat*11/10;	
				//sysDelayMs(5);
			}while (adc_bat < 3000 || adc_bat > 5000);
			
			g_i32BatAdcBuf[g_ui8BatAdcCnt] = adc_bat;
			g_ui8BatAdcCnt ++;
			
			if(g_ui8BatAdcCnt == 3)
			{
			
				BuffOrder(g_i32BatAdcBuf, 3);		
				g_i32BatAdcVal = g_i32BatAdcBuf[1];
				g_ui8BatAdcCnt = 0;
				memset(g_i32BatAdcBuf, 0, sizeof(g_i32BatAdcBuf));
				break;
			}
		}
		
	}
	else
	{
		do
		{
			hal_pmADRead(3, &adc_bat);
			adc_bat=adc_bat*11/10;
			//sysDelayMs(5);
		}while (adc_bat < 3000 || adc_bat > 5000);
		
		g_i32BatAdcBuf[g_ui8BatAdcCnt] = adc_bat;
		g_ui8BatAdcCnt ++;
		
		if(g_ui8BatAdcCnt == 3)
		{
		
			BuffOrder(g_i32BatAdcBuf, 3);		
			g_i32BatAdcVal = g_i32BatAdcBuf[1];
			g_ui8BatAdcCnt = 0;
			memset(g_i32BatAdcBuf, 0, sizeof(g_i32BatAdcBuf));
		}
		
	}
	
}


/*
*Function:		hal_pmGetBatAdcV
*Description:	读取采集到的电池电压
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		采集到的电池电压
*Others:
*/
static int32 hal_pmGetBatAdcV(void)
{
	return g_i32BatAdcVal;
}


/*
*Function:		hal_pmBatCheck
*Description:	电池电量检测
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		计算后的电池电量值，<=0:失败
*Others:
*/
static int32 hal_pmBatCheck(void)
{
	uint8 charger_v2=0;
	uint32 adc_bat = 0;
	int32 sum_adc=0;
	uint32 i;
	uint32 bat_buf_rP_temp=0;
	bat_buf_rP_temp=g_stBatStr.bat_buf_RP;
	
	adc_bat=hal_pmGetBatAdcV();
	
	if(adc_bat > 3000 && adc_bat < 5000)
	{
	
		sysLOG(PWR_LOG_LEVEL_4, "The Bat V is %d mV\r\n", adc_bat);
		charger_v2 = fibo_get_Usbisinsert();
		if(charger_v2 == 0 && g_ui8ChgTmp <= 1)//只有未插入充电线才更新last_batV
		{
			g_stBatQuick.last_batV = adc_bat;
			sysLOG(PWR_LOG_LEVEL_3, "g_ui8ChgTmp:%d, g_stBatStr.charger_v:%d\r\n", g_ui8ChgTmp, g_stBatStr.charger_v);
		}
		hal_pmBatFiFo(adc_bat);
		if(g_stBatStr.bat_buf_n>0)
		{
			for(i=0;i<g_stBatStr.bat_buf_n;i++)
			{
				sum_adc+=g_stBatStr.bat_buf[bat_buf_rP_temp];//小于BATV_NUM的时候，是从地址0开始，等于BATV_NUM则全部需要计算
				bat_buf_rP_temp++;
				if(bat_buf_rP_temp==BATV_NUM)bat_buf_rP_temp=0;
			}
			g_stBatStr.bat_value=sum_adc/g_stBatStr.bat_buf_n;
			
			return g_stBatStr.bat_value;
		}
		else
		{
			return 0;
		}
	}
	else
	{
	
		sysLOG(PWR_LOG_LEVEL_2, "<ERR> The Bat V is %d mV\r\n", adc_bat);
		return -1;
	}
}


/*
*Function:		hal_pmBatHysteresis
*Description:	迟滞接口
*Input:			HysteresisV:输入浮点型电量百分比；lastV:输入上一次保持的电量百分比
*Output:		NULL
*Hardware:
*Return:		返回迟滞后的百分比
*Others:
*/
static int32 hal_pmBatHysteresis(float HysteresisV,int32 lastV)
{
	float lastvtemp;
	int32 batvtemp;
	lastvtemp=lastV;
	
	if(HysteresisV<(lastvtemp-0.5))batvtemp=HysteresisV;
	else if(HysteresisV>(lastvtemp+1+0.5))batvtemp=HysteresisV;
	else batvtemp=lastV;
	return batvtemp;
}


/*
*Function:		hal_pmBatGetValue
*Description:	阻塞式获取电池电量（百分比制：0-100），0%-100%对应的电压为：BAT_ZERO-g_stBatQuick.VbatFull
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		BM_ERR_CHARGING:充电中；0-100：电量百分比
*Others:
*/
int32 hal_pmBatGetValue(void)
{
	float bat_tempv;
	int32 bat_quantity=0;
	unsigned long long uTime=0;
	
	uTime=hal_sysGetTickms()+5000;
		
	while(1)
	{
	
		if(g_stBatStr.bat_checkflag==0)//充电中
		{
			return BM_ERR_CHARGING;
		}
		else if(g_stBatStr.bat_checkflag==1 || g_stBatStr.bat_checkflag==3)//刚拔线状态3和第一次检测电量为等待
		{
			sysDelayMs(50);
			if(hal_sysGetTickms()>uTime)
			{
				return BM_ERR_CHARGING;
			}
		}
		else if(g_stBatStr.bat_checkflag==2)
		{
			if(g_stBatStr.bat_buf_n < (BATV_NUM/2))//等待buf填满在读取电量，不然ad取值不准，容易获取不准确的电量
			{
				sysDelayMs(50);
				if(hal_sysGetTickms()>uTime)break;
			}
			else


			{
				break;
			}
		}
		
	}

	if(g_stBatStr.bat_value >= g_stBatQuick.VbatFull)
	{
		bat_quantity = 100;	
		sysLOG(PWR_LOG_LEVEL_4, " g_stBatStr.bat_checkflag:%d, g_stBatStr.bat_value:%d,the bat_quantity:%d,g_stBatQuick.VbatFull:%d\r\n", g_stBatStr.bat_checkflag, g_stBatStr.bat_value, bat_quantity, g_stBatQuick.VbatFull);
	}
	else if(g_stBatStr.bat_value > BAT_ZERO && g_stBatStr.bat_value < g_stBatQuick.VbatFull)
	{
		bat_tempv = (float)(g_stBatStr.bat_value - BAT_ZERO) * 100 / (g_stBatQuick.VbatFull - BAT_ZERO);
		if(bat_quantity != 0)bat_quantity = (hal_pmBatHysteresis(bat_tempv, bat_quantity));//非第一次
		else bat_quantity = bat_tempv;//第一次
		
		sysLOG(PWR_LOG_LEVEL_4, " g_stBatStr.bat_checkflag:%d, g_stBatStr.bat_value:%d,the bat_quantity:%d,g_stBatQuick.VbatFull:%d\r\n", g_stBatStr.bat_checkflag, g_stBatStr.bat_value, bat_quantity, g_stBatQuick.VbatFull);
	}
	else
	{
		if(g_stBatStr.bat_value == 0)//长时间充电，队列里面已经清空，拔线后还没来得及填buf就去读电量存在异常，直接判为充电中
		{
			bat_quantity= BM_ERR_CHARGING;
		}
		else//低电量
		{
			bat_quantity= 0;
		}
		sysLOG(PWR_LOG_LEVEL_4, " the bat_quantity:%d, g_stBatStr.bat_value:%d\r\n", bat_quantity, g_stBatStr.bat_value);
	}
	if(bat_quantity == 0)
	{
		bat_quantity = 1;//如果电量为0%，返回1%即可
	}
	return bat_quantity;
}


/*
*Function:		hal_pmChgDetIrq
*Description:	充电线插入中断相应接口
*Input:			*param:入参
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmChgDetIrq(void *param)
{
	uint8 level;
	if(g_ui8ChgDetIrqEn==1)
	{
		level = fibo_get_Usbisinsert();
		if(TRUE == level)
		{			
			//sysLOG(PWR_LOG_LEVEL_2, "CHG_DET:TRUE\r\n"); 	
		}
		else
		{
			//sysLOG(PWR_LOG_LEVEL_2, "CHG_DET:FALSE\r\n"); 	
		}
	}
   
}


/*
*Function:		hal_pmQChgWriteParam
*Description:	写快充参数
*Input:			*param_buff:写入的数据;param_len:写入的数据长度;offset:写入数据在文件中的偏移量
*Output:		NULL
*Hardware:
*Return:		0-失败；>0-写入字节数
*Others:
*/
static int8 hal_pmQChgWriteParam(int8 *param_buff, uint8 param_len, uint8 offset)
{
	int32 fd;
	int32 iRet,cret;
	int8 QCHGParam_name[FN_LEN_MAX];

	memset(QCHGParam_name, 0, sizeof(QCHGParam_name));
	sprintf(QCHGParam_name, "/app/ufs/QCHGParam.txt");
	fd = hal_fileOpen(QCHGParam_name, FS_O_RDWR|FS_O_CREAT);
	if(fd < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Open File Fail, fd=%d\r\n", fd);		
		return FALSE;
	}

	iRet = hal_fileSeek(fd, offset, FS_SEEK_SET);
	if(iRet < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	g_stBatQuick.last_iChargeTime = g_stBatQuick.iChargeTime;
	cret = hal_fileWrite(fd, param_buff, param_len);
	if(cret != param_len)
	{
		
		sysLOG(PWR_LOG_LEVEL_4, "Write File Fail %d\r\n", cret);		
		return FALSE;
	}

	iRet = hal_fileClose(fd);
	if(iRet != 0)
	{
		sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", iRet);		
		return FALSE;
	}

	return cret;
	
}


/*
*Function:		hal_pmQChgClrParam
*Description:	清除快充参数
*Input:			param_len:清除的数据长度
*Output:		NULL
*Hardware:
*Return:		0-失败；>0-清除字节数
*Others:
*/
static int8 hal_pmQChgClrParam(uint8 param_len)
{
	int32 fd;
	int32 iRet,cret;
	int8 QCHGParam_name[FN_LEN_MAX];
	int8 fileofQCHGParam[param_len];	
	
	memset(fileofQCHGParam, 0, sizeof(fileofQCHGParam));

	memset(QCHGParam_name, 0, sizeof(QCHGParam_name));
	sprintf(QCHGParam_name, "/app/ufs/QCHGParam.txt");
	fd = hal_fileOpen(QCHGParam_name, FS_O_RDWR|FS_O_CREAT);
	if(fd < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Open File Fail, fd=%d\r\n", fd); 	
		return FALSE;
	}

	iRet = hal_fileSeek(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	
	cret = hal_fileWrite(fd, fileofQCHGParam, param_len);
	if(cret != param_len)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Write File Fail %d\r\n", cret);		
		return FALSE;
	}

	iRet = hal_fileClose(fd);
	if(iRet != 0)
	{	
		sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", iRet);		
		return FALSE;
	}

	return cret;
	
}


/*
*Function:		hal_pmQChgReadParam
*Description:	读取快充参数
*Input:			param_len:需要读取的数据长度;offset:读取文件中的偏移量
*Output:		*param_buff:读取参数存储指针
*Hardware:
*Return:		0-失败,>0-读到的字节数
*Others:
*/
int8 hal_pmQChgReadParam(int8 *param_buff, uint8 param_len, uint8 offset)
{
	int32 fd;
	int32 iRet, ret;
	uint32 filesizeofQCHGParam;
	
	int8 QCHGParam_name[FN_LEN_MAX];
	
	memset(QCHGParam_name, 0, sizeof(QCHGParam_name));
	sprintf(QCHGParam_name, "/app/ufs/QCHGParam.txt");

	fd = hal_fileOpen(QCHGParam_name, FS_O_RDONLY);
	if(fd < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Open File Fail, fd=%d\r\n", fd); 	
		return FALSE;
	}

	iRet = hal_fileSeek(fd, offset, FS_SEEK_SET);
	if(iRet < 0)
	{	
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	filesizeofQCHGParam = hal_fileGetFileSize(QCHGParam_name);
	if(filesizeofQCHGParam < 0)
	{
		
		sysLOG(PWR_LOG_LEVEL_4, "getfilesize Fail %d\r\n", filesizeofQCHGParam);
		return FALSE;
	}
	iRet = hal_fileSeek(fd, offset, FS_SEEK_SET);
	if(iRet < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	
	iRet = hal_fileRead(fd, param_buff, param_len);
	if(iRet<0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Read File Fail %d\r\n", iRet);		
		return FALSE;
	}
	else if(iRet==0)
	{
		iRet = hal_fileClose(fd);
		if(iRet != 0)
		{			
			sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", iRet);		
			return FALSE;
		}
		
		sysLOG(PWR_LOG_LEVEL_3, "/app/ufs/QCHGParam.txt read NULL\r\n");		
		return FALSE;
	}
	else
	{
		ret = hal_fileClose(fd);
		if(ret != 0)
		{		
			sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", ret);		
			return FALSE;
		}
		
		sysLOG(PWR_LOG_LEVEL_3, "/app/ufs/QCHGParam.txt read OK!,iRet:%d\r\n", iRet);	
		return iRet;
	}
	
}


/*
*Function:		hal_pmWriteBatParam
*Description:	写电池参数
*Input:			*param_buff:写入的数据;param_len:写入的数据长度;offset:写入数据在文件中的偏移量
*Output:		NULL
*Hardware:
*Return:		0-失败；>0-写入字节数
*Others:
*/
static int hal_pmWriteBatParam(int8 *param_buff, uint8 param_len, uint8 offset)
{
	int32 fd;
	int32 iRet,cret;
	int8 BATParam_name[FN_LEN_MAX];

	memset(BATParam_name, 0, sizeof(BATParam_name));
	sprintf(BATParam_name, BATPARAM_NAME);
	fd = hal_fileOpen(BATParam_name, FS_O_RDWR|FS_O_CREAT);
	if(fd < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Open File Fail, fd=%d\r\n", fd);		
		return FALSE;
	}

	iRet = hal_fileSeek(fd, offset, FS_SEEK_SET);
	if(iRet < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	cret = hal_fileWrite(fd, param_buff, param_len);
	if(cret != param_len)
	{
		
		sysLOG(PWR_LOG_LEVEL_4, "Write File Fail %d\r\n", cret);		
		return FALSE;
	}

	iRet = hal_fileClose(fd);
	if(iRet != 0)
	{
		sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", iRet);		
		return FALSE;
	}

	return cret;
	
}


/*
*Function:		hal_pmClrBatParam
*Description:	清除电池参数
*Input:			param_len:清除的数据长度
*Output:		NULL
*Hardware:
*Return:		0-失败；>0-清除字节数
*Others:
*/
static int8 hal_pmClrBatParam(uint8 param_len)
{
	int32 fd;
	int32 iRet,cret;
	int8 BATParam_name[FN_LEN_MAX];
	int8 fileofBATParam[param_len];	
	
	memset(fileofBATParam, 0, sizeof(fileofBATParam));

	memset(BATParam_name, 0, sizeof(BATParam_name));
	sprintf(BATParam_name, BATPARAM_NAME);
	fd = hal_fileOpen(BATParam_name, FS_O_RDWR|FS_O_CREAT);
	if(fd < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Open File Fail, fd=%d\r\n", fd); 	
		return FALSE;
	}

	iRet = hal_fileSeek(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	
	cret = hal_fileWrite(fd, fileofBATParam, param_len);
	if(cret != param_len)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Write File Fail %d\r\n", cret);		
		return FALSE;
	}

	iRet = hal_fileClose(fd);
	if(iRet != 0)
	{	
		sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", iRet);		
		return FALSE;
	}

	return cret;
	
}


/*
*Function:		hal_pmReadBatParam
*Description:	读取电池参数
*Input:			param_len:需要读取的数据长度;offset:读取文件中的偏移量
*Output:		*param_buff:读取参数存储指针
*Hardware:
*Return:		0-失败,>0-读到的字节数
*Others:
*/
static int8 hal_pmReadBatParam(int8 *param_buff, uint8 param_len, uint8 offset)
{
	int32 fd;
	int32 iRet, ret;
	uint32 filesizeofBatParam;
	
	int8 BATParam_name[FN_LEN_MAX];
	
	memset(BATParam_name, 0, sizeof(BATParam_name));
	sprintf(BATParam_name, BATPARAM_NAME);

	filesizeofBatParam = hal_fileGetFileSize(BATParam_name);
	if(filesizeofBatParam < 0)
	{
		
		sysLOG(PWR_LOG_LEVEL_4, "getfilesize Fail %d\r\n", filesizeofBatParam);
		return FALSE;
	}

	fd = hal_fileOpen(BATParam_name, FS_O_RDONLY);
	if(fd < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Open File Fail, fd=%d\r\n", fd); 	
		return FALSE;
	}

	iRet = hal_fileSeek(fd, offset, FS_SEEK_SET);
	if(iRet < 0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Seek File Fail %d\r\n", iRet);		
		return FALSE;
	}
	
	iRet = hal_fileRead(fd, param_buff, param_len);
	if(iRet<0)
	{		
		sysLOG(PWR_LOG_LEVEL_4, "Read File Fail %d\r\n", iRet);		
		return FALSE;
	}
	else if(iRet==0)
	{
		iRet = hal_fileClose(fd);
		if(iRet != 0)
		{			
			sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", iRet);		
			return FALSE;
		}
		
		sysLOG(PWR_LOG_LEVEL_3, "/app/ufs/QCHGParam.txt read NULL\r\n");		
		return FALSE;
	}
	else
	{
		ret = hal_fileClose(fd);
		if(ret != 0)
		{		
			sysLOG(PWR_LOG_LEVEL_4, "Close File Fail %d\r\n", ret);		
			return FALSE;
		}
		
		sysLOG(PWR_LOG_LEVEL_3, "/app/ufs/QCHGParam.txt read OK!,iRet:%d\r\n", iRet);	
		return iRet;
	}
	
}


/*
*Function:		hal_pmChargerCtl
*Description:	充电控制使能接口
*Input:			value:TRUE-使能；FALSE-失能
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pmChargerCtl(BOOL value)
{	
	switch(value)
	{
		case TRUE:
			fibo_gpio_set(CHG_EN_GPIO, FALSE);
		break;

		case FALSE:

			fibo_gpio_set(CHG_EN_GPIO, TRUE);
		break;

		default:

		break;
	
	}
	
}


/*
*Function:		hal_pmChargerInit
*Description:	充电初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		CHG_ING,高电平表示未充电，低电平表示充电中
*				CHG_DET,检测microusb充电口是否有电，高电平有电，低电平没电
*/
static void hal_pmChargerInit(void)
{
	hal_pmADInit();
	
	/*初始化IO*/
	fibo_gpio_mode_set(CHG_EN_GPIO, GpioFunction0);
	fibo_gpio_cfg(CHG_EN_GPIO, GpioCfgOut);
	hal_pmChargerCtl(TRUE);
	
	fibo_gpio_mode_set(CHG_ING_GPIO, GpioFunction0);
	fibo_gpio_cfg(CHG_ING_GPIO, GpioCfgIn);	
		
	g_stBatStr.charger_v = 1;
	g_stBatStr.charger_read = 0;
	g_stBatStr.bat_checkflag=2;//初始化之后直接快速填充buf
#if PWRLOOP_IRQ	
	oc_isr_t DR_ChgD_oc_isr_t = {
		.is_debounce = true,
		.intr_enable = true,  
		.intr_level = false,   
		.intr_falling = true,   
		.inte_rising = true, 
		.callback = hal_pmChgDetIrq,  
	};
#endif

	g_ui8ChgDetIrqEn=1;

	
#if QUICK_CHARGE
	
		if(4 == hal_pmQChgReadParam(&g_stBatQuick.VbatFull, 4, 12))
		{
			if(g_stBatQuick.VbatFull < BAT_HUND || g_stBatQuick.VbatFull > 5000)
			{
				g_stBatQuick.VbatFull = BAT_HUND;
			}
		}
		else
		{
			g_stBatQuick.VbatFull = BAT_HUND;
		}
		
		g_stBatQuick.last_batV = 0;
		g_stBatQuick.last_iChargeTime = 0;
		
		
		g_stBatQuick.iNeedTime = NEEDTIME_MAX; 
		g_stBatQuick.iFreeTime = 0;
		g_stBatQuick.iFreeTime_once = 100;
		g_stBatQuick.iChargeTime_add = 0;
		g_stBatQuick.iChargeTime = 0;
		g_stBatQuick.iChargeTime_all = 0;
		g_stBatQuick.iCHargeTime_once = 0;
		g_stBatQuick.iChargeTime_offset = 0;
		g_stBatQuick.free_flag = 0;//开机后如果没充电则开始计算iFreeTime_once
		g_stBatQuick.push_flag = 0;
		g_stBatQuick.chargedone = 0;
		g_stBatQuick.chargedone_last =0;
		g_stBatQuick.writeparam_count = 0;
		g_stBatQuick.count = 100;
#else
		g_ui8VBatFullRefresh = 0;
		if(4 == hal_pmReadBatParam(&g_stBatQuick.VbatFull, 4, 0))
		{
			if(g_stBatQuick.VbatFull < BAT_HUND || g_stBatQuick.VbatFull > 5000)
			{
				g_stBatQuick.VbatFull = BAT_HUND;
			}
		}
		else
		{
			g_stBatQuick.VbatFull = BAT_HUND;
		}

		sysLOG(PWR_LOG_LEVEL_1, "g_ui8VBatFullRefresh:%d,VbatFull=%d\r\n", g_ui8VBatFullRefresh, g_stBatQuick.VbatFull); 
		
	
#endif
	
}



/*
*Function:		hal_pmChargerCheck
*Description:	充电检测
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0b01-未充电，0b10-正在充电中,0b11-已充满
*Others:		
*/
static uint8 hal_pmChargerCheck(void)
{

	uint8 charger_v1=0,charger_v2=0;
	
#if QUICK_CHARGE
	charger_v1 = g_stBatQuick.chargedone;
#else
	fibo_gpio_get(CHG_ING_GPIO, &charger_v1);

#endif
	charger_v2 = fibo_get_Usbisinsert();

	if(charger_v2 == 1 && TemperHot_Flag == 1)//插入USB线,并且充电过热
	{
		g_stBatStr.charger_v = 2;
		return g_stBatStr.charger_v;
	}

	
	g_ui8ChgTmp = charger_v1 | (charger_v2 << 1);
	
	if((g_stBatStr.charger_v != g_ui8ChgTmp) && (g_ui8ChgTmp == g_ui8ChgValueLast))//最新值与之前的不一样，并且与上一次一样，才能count++
	{
		
		sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.charger_v:%d, g_ui8ChgTmp:%d, g_ui8ChgValueLast:%d\r\n", g_stBatStr.charger_v, g_ui8ChgTmp, g_ui8ChgValueLast); 
		g_stBatStr.bat_checkflag = 3;//usb插入或者拔出都要做个等待，因为需要10*100ms才能改变状态，如果不等待读进去的可能是插着usb线的电压值！
		g_ui8ChgCount ++;
		if(g_ui8ChgCount > CHARGER_TREMBLE)//开机100ms采样一次，关机充电10ms采样一次，连续CHARGER_TREMBLE次不一样才能改变状态
		{
			if(g_ui8ChgTmp == 3)
			{
				if(g_ui32ChgFullCnt < (60+10))//防止溢出
				{
					g_ui32ChgFullCnt ++;
				}
				g_ui32ChgNoFullCnt = 0;
				if(g_ui32ChgFullCnt > 60)//如果判为充满，需要连续60次（1min）才能确认充满状态
				{
					g_stBatStr.charger_v = g_ui8ChgTmp;
					g_ui8VBatFullRefresh = 1;//确认已充满，拔线后可以刷新VbatFull
				}
				else
				{
					g_stBatStr.charger_v = 2;
				}
							
				sysLOG(PWR_LOG_LEVEL_4, "g_ui8ChgCount > %d, g_ui32ChgFullCnt:%d, g_ui32ChgNoFullCnt:%d, g_stBatStr.charger_v=%d, g_ui8ChgTmp:%d,g_ui8VBatFullRefresh=%d\r\n", CHARGER_TREMBLE, g_ui32ChgFullCnt, g_ui32ChgNoFullCnt, g_stBatStr.charger_v, g_ui8ChgTmp, g_ui8VBatFullRefresh); 
			}
			else

			{
				if(g_ui32ChgNoFullCnt < 10)//防止溢出
				{
					g_ui32ChgNoFullCnt++;
				}
				if(g_ui32ChgNoFullCnt > 1)
				{
					g_ui32ChgFullCnt = 0;
				}	
				g_stBatStr.charger_v = g_ui8ChgTmp;
			}
			
			g_ui8ChgCount = 0;
			g_stBatStr.charger_read = 1;//开机之后，如果插着充电线，则需要等待到bat_s.charger_v的值更新了才能获取充电状态。
			sysLOG(PWR_LOG_LEVEL_1, "g_ui8ChgCount > %d, g_stBatStr.charger_v=%d\r\n", CHARGER_TREMBLE, g_stBatStr.charger_v); 
		}
	}
	else
	{
		if(g_ui8ChgCount != 0)
		{	
			
			sysLOG(PWR_LOG_LEVEL_4, "g_ui8ChgCount=%d\r\n", g_ui8ChgCount); 
			g_ui8ChgCount = 0;
		}
		if(g_ui8ChgTmp == g_ui8ChgValueLast)//开机之后，如果没有插充电线，读取值会和上一次值charger_v_last一样，也可以获取充电状态
		{
			g_stBatStr.charger_read = 1;
		}
	}
	g_ui8ChgValueLast = g_ui8ChgTmp;//记录上一次的状态
	
	if(g_stBatStr.charger_v>1)//usb插入
	{
		g_stBatStr.bat_lowpwrflag=0;
		g_stBatStr.bat_checkflag=0;
		g_stBatStr.bat_checkwait_cnt=0;
	}
	else//只用电池才检测
	{
		if(g_stBatStr.bat_checkflag==0)
		{
			g_stBatStr.bat_checkflag=3;
			//g_stBatStr.charger_v=0;
			sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==0\r\n"); 
		}
		else if(g_stBatStr.bat_checkflag==3)
		{
		
			sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==3, g_stBatStr.bat_checkwait_cnt:%d\r\n", g_stBatStr.bat_checkwait_cnt); 
			g_stBatStr.bat_checkwait_cnt++;
			if(g_stBatStr.bat_checkwait_cnt>READBAT_DELAY)//等待2s再去做check
			{
				g_stBatStr.bat_checkwait_cnt=0;
				g_stBatStr.bat_count=0;
				//g_stBatStr.charger_v=0;
				g_stBatStr.bat_checkflag=1;
				
				sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==1\r\n"); 
			}
			
		}
		
	}
	return g_stBatStr.charger_v;
	
}


/*
*Function:		hal_pmGetChargerValue
*Description:	获得USB充电状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0b0x-未插充电线，0b10-正在充电中,0b11-已插充电线但未充电即已充满, -2:超时，获取失败
*Others:
*/
int8 hal_pmGetChargerValue(void)
{
	unsigned long long uTime=0;
	
	uTime=hal_sysGetTickms()+2000;
	while(1)
	{
		if(g_stBatStr.charger_read == 1)
		{
			break;
		}
		sysDelayMs(10);
		if(hal_sysGetTickms() > uTime)
		{
			return -2;
		}
	}
	return g_stBatStr.charger_v;
}

/*
*Function:		hal_pmGetChg
*Description:	获得USB充电状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:未知; 1:充电中; 2:未接USB; 3:接USB但未充电; 4:满电
*Others:
*/
char hal_pmGetChg(void)
{
	char iRet = 0;
	
	iRet = hal_pmGetChargerValue();//0b0x-未插充电线，0b10-正在充电中,0b11-已插充电线但未充电即已充满

	if(iRet == 0 || iRet == 1)
	{
		return 2;
	}
	else if(iRet == 2)
	{
		return 1;
	}
	else if(iRet == 3)
	{
		return 4;
	}
	else


	{
		return 0;
	}
}

/*
*Function:		hal_pmQNeedTime
*Description:	读取需要充电的时间
*Input:			lastV:上一次的充电时间值
*Output:		NULL
*Hardware:
*Return:		需要充电的时间值
*Others:
*/
static uint32 hal_pmQNeedTime(uint32 lastV)
{
	if(lastV >= 4000)
	{
		return (NEEDTIME_MAX - 20);
	}
	else if(lastV >= 3900)
	{
		return (NEEDTIME_MAX - 8);
	}
	else if(lastV >= 3800)
	{
		return (NEEDTIME_MAX - 5);
	}
	else if(lastV >= 3700)
	{
		return (NEEDTIME_MAX - 2);
	}
	else if(lastV <= 3420)
	{
		return (NEEDTIME_MAX + NEEDTIME_ADD_MAX);
	}
	else
	{
		return NEEDTIME_MAX;
	}
}


/*
*Function:		hal_pmQClrParam
*Description:	换电池测试时，需要不充电的情况下清除下保存的参数，不然之前的充电时间会参与计算
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-失败；>0-写入字节数
*Others:
*/
int hal_pmQClrParam(void)
{
	int iRet = -1;


	g_stBatQuick.iFreeTime_once = 100;
	
	g_stBatQuick.free_flag = 2;//回到初始化的值

	g_stBatQuick.iChargeTime = 0;
	g_stBatQuick.last_iChargeTime = 0;
	g_stBatQuick.iNeedTime = NEEDTIME_MAX;
	iRet = hal_pmQChgWriteParam((char *)&g_stBatQuick.last_batV, 12, 0);//拔线超过30min，则更新下文件系统中的值，防止再次插线开机时，last_iChargeTime参与计算。
	sysLOG(PWR_LOG_LEVEL_4, "hal_pmQChgWriteParam, iRet:%d\r\n", iRet);

	return iRet;
}

/*
*Function:		hal_pmQVfullCheck
*Description:	快充模式中电池满电检测
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-失败；>0-写入字节数
*Others:
*/
static void hal_pmQVfullCheck(void)
{
	if(g_stBatStr.bat_checkflag == 2)//已拔线，并且可正常检测电压
	{
		if(g_stBatStr.bat_value > g_stBatQuick.VbatFull)//电池电压>满电电压
		{
			if(g_ui8VfullCheckFlagQ == 0)
			{
				g_ui32VfullTimeQ = hal_sysGetTickms();
				g_ui8VfullCheckFlagQ = 1;
				sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_value:%d, g_stBatQuick.VbatFull:%d, g_ui8VfullCheckFlagQ:%d, g_ui32VfullTimeQ:%lld\r\n", g_stBatStr.bat_value, g_stBatQuick.VbatFull, g_ui8VfullCheckFlagQ, g_ui32VfullTimeQ); 
			}
			else

			{
				if(hal_sysGetTickms() > (g_ui32VfullTimeQ+VFULLTIMEDELAY))
				{
					g_stBatQuick.VbatFull = g_stBatStr.bat_value;
					g_ui8VfullCheckFlagQ = 0;
					sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_value:%d, g_stBatQuick.VbatFull:%d, g_ui8VfullCheckFlagQ:%d\r\n", g_stBatStr.bat_value, g_stBatQuick.VbatFull, g_ui8VfullCheckFlagQ); 
				}
			}
			
		}
		else//电压在VFULLTIMEDELAY时间内跌落下来
		{
			g_ui8VfullCheckFlagQ = 0;
			sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_value:%d, g_stBatQuick.VbatFull:%d, g_ui8VfullCheckFlagQ:%d\r\n", g_stBatStr.bat_value, g_stBatQuick.VbatFull, g_ui8VfullCheckFlagQ); 
			
		}
	}
	else if(g_stBatStr.bat_checkflag == 0)//插入USB线
	{
		g_ui8VfullCheckFlagQ = 0;
		sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_value:%d, g_stBatQuick.VbatFull:%d, g_ui8VfullCheckFlagQ:%d\r\n", g_stBatStr.bat_value, g_stBatQuick.VbatFull, g_ui8VfullCheckFlagQ);
	}
	
}

/*
*Function:		hal_pmQChargerCheck
*Description:	快充模式中充电检测
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmQChargerCheck(void)
{
	uint8 charger_v2=0;
	g_stBatQuick.count++;
	if(g_stBatQuick.count >= 100)
	{
		g_stBatQuick.count = 0;

		charger_v2 = fibo_get_Usbisinsert();
		if(charger_v2 == 1)//插入充电线
		{
			if(g_stBatQuick.iFreeTime_once > 30)//插入线，但中途拔出充电线超过30min
			{
				g_stBatQuick.last_iChargeTime = 0;
				g_stBatQuick.iNeedTime = NEEDTIME_MAX;
				g_stBatQuick.iFreeTime = 0;
				g_stBatQuick.iFreeTime_once = 0;
				g_stBatQuick.iChargeTime_add = 0;
				g_stBatQuick.iChargeTime = 0;
				g_stBatQuick.iChargeTime_all = 0;
				g_stBatQuick.iCHargeTime_once = 0;
				g_stBatQuick.iChargeTime_offset = 0;
				g_stBatQuick.chargedone = 0;
				g_stBatQuick.chargedone_last = 0;
				g_stBatQuick.writeparam_count = 0;
				g_stBatQuick.push_flag = 0;
				g_stBatQuick.free_flag = 0;
				
				if(g_stBatQuick.last_batV == 0)//开机前已插入USB
				{
					
					if(12 == hal_pmQChgReadParam((char *)&g_stBatQuick.last_batV, 12, 0))//文件读取成功
					{
					
						if(g_stBatQuick.last_batV == 0)//读成功，但未存储数据,
						{
							g_stBatQuick.last_iChargeTime = 0;
							g_stBatQuick.iNeedTime = NEEDTIME_MAX;
							
						}
						else//读成功并且有存储的数据
						{
							g_stBatQuick.iNeedTime = hal_pmQNeedTime(g_stBatQuick.last_batV);
						}
						
						g_stBatQuick.iChargeTime_offset = g_stBatQuick.last_iChargeTime;
						g_stBatQuick.iChargeTime_all +=g_stBatQuick.iChargeTime_offset;
					}
					else//文件不存在
					{
						g_stBatQuick.last_batV = 0;
						g_stBatQuick.last_iChargeTime = 0;
						g_stBatQuick.iNeedTime = NEEDTIME_MAX;
					}
					
					sysLOG(PWR_LOG_LEVEL_2, "charged before Boot,last_batV:%d,last_iChargeTime:%d,iNeedTime:%d\r\n", g_stBatQuick.last_batV, g_stBatQuick.last_iChargeTime, g_stBatQuick.iNeedTime); 
				}
				else//开机后插入USB
				{

					g_stBatQuick.iNeedTime = hal_pmQNeedTime(g_stBatQuick.last_batV);
				}
	
				
			}
			else//插入线，但中途拔出线未超过30min
			{
				g_stBatQuick.chargedone_last = 0;
				if(g_stBatQuick.free_flag != 0)
				{
					g_stBatQuick.free_flag = 0;
					g_stBatQuick.iFreeTime += g_stBatQuick.iFreeTime_once;
					if(g_stBatQuick.iNeedTime < (NEEDTIME_MAX + NEEDTIME_ADD_MAX))
					{
						if(g_stBatQuick.iChargeTime_add < 5)
						{
							g_stBatQuick.iChargeTime_add += (g_stBatQuick.iFreeTime_once/10) + 1;
							g_stBatQuick.iNeedTime += (g_stBatQuick.iFreeTime_once/10) + 1;
						}
						
					}
					
					sysLOG(PWR_LOG_LEVEL_2, "charged but time<30min iNeedTime:%d,iFreeTime:%d\r\n", g_stBatQuick.iNeedTime, g_stBatQuick.iFreeTime); 
				}
				
			}
			//g_stBatQuick.iChargeTime开始计时
			if(g_stBatQuick.push_flag == 0)
			{
				g_stBatQuick.push_flag = 1;
				g_stBatQuick.pushusb_uTime = hal_sysGetTickms();
				g_stBatQuick.writeparam_count = 0;
				
				sysLOG(PWR_LOG_LEVEL_2, "pushusb_uTime start, last_batV:%d, pushusb_uTime:%lld,iNeedTime:%d\r\n", g_stBatQuick.last_batV, g_stBatQuick.pushusb_uTime, g_stBatQuick.iNeedTime); 
			}

			
			g_stBatQuick.iChargeTime = g_stBatQuick.iChargeTime_all + (hal_sysGetTickms() - g_stBatQuick.pushusb_uTime)/(1000*60);
			
			sysLOG(PWR_LOG_LEVEL_4, "hal_sysGetTickms:%d, pushusb_uTime:%lld, iFreeTime:%d\r\n", hal_sysGetTickms(), g_stBatQuick.pushusb_uTime, g_stBatQuick.iFreeTime); 

			
			
			if((hal_sysGetTickms() - g_stBatQuick.pushusb_uTime)/(1000*60) > (5*g_stBatQuick.writeparam_count))
			{
				
				sysLOG(PWR_LOG_LEVEL_4, "hal_pmQChgWriteParam\r\n"); 
				hal_pmQChgWriteParam((char *)&g_stBatQuick.last_batV, 12, 0);
				g_stBatQuick.writeparam_count +=1;
			}
			
			if(g_stBatQuick.iChargeTime >= g_stBatQuick.iNeedTime)//充满
			{
				g_stBatQuick.chargedone = 1;
				g_stBatQuick.chargedone_last = 1;				
				sysLOG(PWR_LOG_LEVEL_2, "g_stBatQuick.chargedone:%d\r\n", g_stBatQuick.chargedone); 
			}
		}
		else//拔出充电线
		{
			g_stBatQuick.chargedone = 0;
			
			if(g_stBatQuick.push_flag != 0)
			{
				g_stBatQuick.push_flag = 0;
				g_stBatQuick.writeparam_count = 0;
				g_stBatQuick.iCHargeTime_once = (hal_sysGetTickms() - g_stBatQuick.pushusb_uTime)/(1000*60);
				g_stBatQuick.iChargeTime_all +=g_stBatQuick.iCHargeTime_once;
			}
			//g_stBatQuick.iFreeTime开始计时
			if(g_stBatQuick.free_flag == 0)//检测到第一次拔出线
			{
				g_stBatQuick.free_flag = 1;
				g_stBatQuick.freeusb_uTime = hal_sysGetTickms();
				
				sysLOG(PWR_LOG_LEVEL_4, "freeusb_uTime start,free_flag:%d, freeusb_uTime:%lld\r\n", g_stBatQuick.free_flag, g_stBatQuick.freeusb_uTime); 
			}
			else if(g_stBatQuick.free_flag == 1)//插入并拔出线
			{
				g_stBatQuick.iFreeTime_once = (hal_sysGetTickms() - g_stBatQuick.freeusb_uTime)/(1000*60);
				if((g_stBatQuick.iFreeTime_once) > 30)//拔线超过30min
				{
					g_stBatQuick.free_flag = 2;//回到初始化的值

					g_stBatQuick.iChargeTime = 0;
					g_stBatQuick.last_iChargeTime = 0;
					g_stBatQuick.iNeedTime = NEEDTIME_MAX;
					hal_pmQChgWriteParam((char *)&g_stBatQuick.last_batV, 12, 0);//拔线超过30min，则更新下文件系统中的值，防止再次插线开机时，last_iChargeTime参与计算。
				}
				
				sysLOG(PWR_LOG_LEVEL_4, "g_stBatQuick.iFreeTime_once:%d\r\n", g_stBatQuick.iFreeTime_once); 
			}
		}
		
		sysLOG(PWR_LOG_LEVEL_5, "iChargeTime:%d,iNeedTime:%d,iChargeTime_offset:%d,free_flag:%d,push_flag:%d, iCHargeTime_once:%d,iChargeTime_all:%d\r\n", g_stBatQuick.iChargeTime, g_stBatQuick.iNeedTime, g_stBatQuick.iChargeTime_offset, g_stBatQuick.free_flag, g_stBatQuick.push_flag, g_stBatQuick.iCHargeTime_once, g_stBatQuick.iChargeTime_all); 

		hal_pmQVfullCheck();
	}
}

/*
*Function:		hal_pmBatLowCheck
*Description:	电池低电量检测
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmBatLowCheck(void)
{
	static uint32 batlowcnt = 0;
	static uint32 batlowflag = 0;
	int iRet = 0;
	int lcdtmp = 0;
	
	batlowcnt ++;
	if(batlowcnt > 100)//10秒检测一次
	{
		batlowcnt = 0;
		if(g_stBatStr.bat_checkflag == 2 && g_stBatStr.bat_buf_n == BATV_NUM)//已拔线且非第一次检测&&缓存中的值已填满
		{
			
			iRet = hal_pmBatGetValue();
			if(iRet < 3 && iRet >=0)//未充电且电量小于3%
			{
				batlowflag ++;
				if(batlowflag > 3)//连续3次电量小于3%
				{
					batlowflag = 0;
					sysLOG(PWR_LOG_LEVEL_1, "<WARN> batlow start poweroff,hal_pmBatGetValue,iRet:%d,batlowflag=%d\r\n", iRet, batlowflag);
					hal_scrCls();
					lcdtmp = hal_scrGetAttrib(LCD_ATTR_FONTTYPE);
					hal_scrSetAttrib(LCD_ATTR_FONTTYPE, 0);
					hal_scrPrint(0, 2, 0b10000010,"电量过低正在关机!");
					hal_scrSetAttrib(LCD_ATTR_FONTTYPE, lcdtmp);
					hal_ttsQueuePlay("电量过低正在关机", NULL, NULL, 0);
					sysDelayMs(5000);
					hal_pmPwrOFF();
				}
				sysLOG(PWR_LOG_LEVEL_3, "<WRAN> hal_pmBatGetValue,iRet:%d,batlowflag=%d\r\n", iRet, batlowflag);
				
			}
			else
			{
				batlowcnt = 0;
				batlowflag = 0;
			}
		
		}
		else
		{
			batlowcnt = 0;
			batlowflag = 0;
		}
	}
	
}


/*
*Function:		hal_pmBatChargerCheck
*Description:	电池以及充电检测一次接口
*Input:			count:多久检测一次，计数器值;
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pmBatChargerCheck(uint32 count)
{
	int32 ret;
	hal_pmCollectBatAdcV();
	hal_pmChargerCheck();
#if QUICK_CHARGE	
	hal_pmQChargerCheck();
#endif
#if BATLOWPWROFF
	hal_pmBatLowCheck();
#endif

	g_stBatStr.bat_count++;
	if(g_stBatStr.bat_checkflag==0)//插入USB线
	{
		if(g_stBatStr.bat_count>=count)//10s查一次
		{
			g_stBatStr.bat_count=0;
			
			hal_pmBatCheck();
//			if(g_stBatStr.bat_lowpwrflag==0)
//			{
//				if(g_stBatStr.bat_value<BATLOWPWR)
//				{
//					g_stBatStr.bat_lowpwrflag=1;//低电量标志位					
//				}
//			}
		}
	}
	else if(g_stBatStr.bat_checkflag==1 || g_stBatStr.bat_checkflag==2)//未插充电线且不是等待状态3
	{
		if(g_stBatStr.bat_buf_n<BATV_NUM)
		{
			if(g_stBatStr.bat_checkflag==2)//非第一次检测
			{
				if(g_stBatStr.bat_count>=1)//0.1s查一次
				{
					g_stBatStr.bat_count=0;
					
					hal_pmBatCheck();
					if(g_stBatStr.bat_lowpwrflag==0)
					{
						if(g_stBatStr.bat_value<BATLOWPWR)
						{
							g_stBatStr.bat_lowpwrflag=1;//低电量标志位
						
						}
					}
					
				}
			}
			else if(g_stBatStr.bat_checkflag==1)//第一次检测，为了避免刚拔线瞬间电压偏高
			{
				if(g_stBatStr.bat_count>=1)//0.1s查一次
				{
					g_stBatStr.bat_count=0;
					
					while(1)
					{
						ret = hal_pmBatCheck();
						if(ret > 0)break;
						sysDelayMs(10);
					}
					if(g_stBatStr.bat_lowpwrflag==0)
					{
						if(g_stBatStr.bat_value<BATLOWPWR)
						{
							g_stBatStr.bat_lowpwrflag=1;//低电量标志位
						
						}
					}
#if QUICK_CHARGE					
					if(g_stBatQuick.chargedone_last == 1)//充电时间满足则更新
					{
						if((ret - BAT_HUNDOFFSET) > BAT_HUND)//(拔线后的电量值-100mV)>4000mV
						{
							g_stBatQuick.VbatFull = ret - BAT_HUNDOFFSET;
						}
						else//(拔线后的电量值-100mV)<4000mV,则降为4000mV
						{
							g_stBatQuick.VbatFull = BAT_HUND;
						}
						hal_pmQChgWriteParam(&g_stBatQuick.VbatFull, 4, 12);
						
						sysLOG(PWR_LOG_LEVEL_2, "Write g_stBatQuick.VbatFull,:%d\r\n", g_stBatQuick.VbatFull); 
					}
					
					sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==1, ret:%d, chargedone_last:%d\r\n", ret, g_stBatQuick.chargedone_last); 
#else
					if(g_ui8VBatFullRefresh == 1)//已充满则更新
					{
						g_ui8VBatFullRefresh = 0;
						if((ret - BAT_HUNDOFFSET) > BAT_HUND)//(拔线后的电量值-100mV)>4000mV
						{
							g_stBatQuick.VbatFull = ret - BAT_HUNDOFFSET;
						}
						else//(拔线后的电量值-100mV)<4000mV,则降为4000mV
						{
							g_stBatQuick.VbatFull = BAT_HUND;
						}
						hal_pmWriteBatParam(&g_stBatQuick.VbatFull, 4, 0);
						
						sysLOG(PWR_LOG_LEVEL_2, "Write g_stBatQuick.VbatFull:%d,g_ui8VBatFullRefresh=%d\r\n", g_stBatQuick.VbatFull, g_ui8VBatFullRefresh); 
					}
					
					sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==1, ret:%d\r\n", ret); 
#endif
					if(g_stBatStr.bat_buf_n >= (BATV_NUM/2))//等待bat_buf_n队列满了在改变bat_checkflag的值，防止拔线后第一次读取到的电量和后面读取到的不一致
					{
						g_stBatStr.bat_checkflag = 2;
					}
				}
			}
		}
#if 1		
		else
		{
			if(g_stBatStr.bat_checkflag==1)
			{
				if(g_stBatStr.bat_count>=1)//0.1s查一次
				{
					g_stBatStr.bat_count=0;
					
					
					g_stBatStr.bat_checkflag=2;//防止插上就拔掉USB线的时候，bat_s.bat_buf_n个数还没减少一个，直接走这里，但是bat_checkflag=1；导致读电量时一直阻塞
					while(1)
					{
						ret = hal_pmBatCheck();
						if(ret > 0)break;
						sysDelayMs(10);
					}
					if(g_stBatStr.bat_lowpwrflag==0)
					{
						if(g_stBatStr.bat_value<BATLOWPWR)
						{
							g_stBatStr.bat_lowpwrflag=1;//低电量标志位
						
						}
					}
#if QUICK_CHARGE					
					if(g_stBatQuick.chargedone_last == 1)//充电时间满足则更新
					{
						if((ret - BAT_HUNDOFFSET) > BAT_HUND)//(拔线后的电量值-100mV)>4000mV
						{
							g_stBatQuick.VbatFull = ret - BAT_HUNDOFFSET;
						}
						else//(拔线后的电量值-100mV)<4000mV,则降为4000mV
						{
							g_stBatQuick.VbatFull = BAT_HUND;
						}
						hal_pmQChgWriteParam(&g_stBatQuick.VbatFull, 4, 12);
						
						sysLOG(PWR_LOG_LEVEL_2, "Write g_stBatQuick.VbatFull,:%d\r\n", g_stBatQuick.VbatFull); 
					}
					
					sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==1, ret:%d, chargedone_last:%d\r\n", ret, g_stBatQuick.chargedone_last); 
#else
					if(g_ui8VBatFullRefresh == 1)//已充满则更新
					{
						g_ui8VBatFullRefresh = 0;
						if((ret - BAT_HUNDOFFSET) > BAT_HUND)//(拔线后的电量值-100mV)>4000mV
						{
							g_stBatQuick.VbatFull = ret - BAT_HUNDOFFSET;
						}
						else//(拔线后的电量值-100mV)<4000mV,则降为4000mV
						{
							g_stBatQuick.VbatFull = BAT_HUND;
						}
						hal_pmWriteBatParam(&g_stBatQuick.VbatFull, 4, 0);
						
						sysLOG(PWR_LOG_LEVEL_2, "Write g_stBatQuick.VbatFull:%d,g_ui8VBatFullRefresh=%d\r\n", g_stBatQuick.VbatFull, g_ui8VBatFullRefresh); 
					}
					
					sysLOG(PWR_LOG_LEVEL_4, "g_stBatStr.bat_checkflag==1, ret:%d\r\n", ret);
#endif
				}
			}
			else if(g_stBatStr.bat_checkflag==2)
			{
				if(g_stBatStr.bat_count>=count)//10s查一次
				{
					g_stBatStr.bat_count=0;
					hal_pmBatCheck();
					if(g_stBatStr.bat_lowpwrflag==0)
					{
						if(g_stBatStr.bat_value<BATLOWPWR)
						{
							g_stBatStr.bat_lowpwrflag=1;//低电量标志位
						
						}
					}
				}
			}
		}
#endif		
	}
	
}


/*
*Function:		hal_pmReadNormalOFF
*Description:	读重启标志位
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-失败；'R'-重启；'N'-不自动重启
*Others:
*/
static int8 hal_pmReadNormalOFF(void)
{
	int32 fd;
	int32 iRet;
	
	int8 fileofNorflag[20];
	int8 Norflagname[32];

	memset(Norflagname, 0, sizeof(Norflagname));
	sprintf(Norflagname, "/app/ufs/Norflag.txt");

	/*打开文件*/
	fd = hal_fileOpen(Norflagname, FS_O_RDONLY);
	if(fd < 0)
	{
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> hal_fileOpen fd=%d\r\n", fd);
		return FALSE;
	}
	
	iRet = hal_fileSeek(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> Seek File Fail %d\r\n", iRet);
		
		return FALSE;
	}
	
	
	memset(fileofNorflag,0,sizeof(fileofNorflag));
	iRet = hal_fileRead(fd, fileofNorflag,1);
	if(iRet<0)
	{		
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> Read File Fail %d\r\n", iRet);
		return FALSE;
	}
	else if(iRet==0)
	{
		iRet = hal_fileClose(fd);
		if(iRet != 0)
		{			
			sysLOG(PWR_LOG_LEVEL_5, "<ERR> Close File Fail %d\r\n", iRet);
			return FALSE;
		}
		
		sysLOG(PWR_LOG_LEVEL_4, "<ERR> /app/ufs/Norflag.txt read NULL\r\n");
		return FALSE;
	}
	else
	{
		iRet = hal_fileClose(fd);
		if(iRet != 0)
		{
		
			sysLOG(PWR_LOG_LEVEL_5, "<ERR> Close File Fail %d\r\n", iRet);
			return FALSE;
		}
		
		sysLOG(PWR_LOG_LEVEL_4, "<SUCC> /app/ufs/Norflag.txt read OK!\r\n");
		return fileofNorflag[0];
	}
	
}


/*
*Function:		hal_pmWriteNormalOFF
*Description:	写正常关机标志位
*Input:			*buff:写入内容指针
*Output:		NULL
*Hardware:
*Return:		1-成功；0失败
*Others:
*/
static int8 hal_pmWriteNormalOFF(int8 *buff)
{
	int32 fd;
	int32 iRet,cret;
	int8 Norflagname[32];

	memset(Norflagname, 0, sizeof(Norflagname));
	sprintf(Norflagname, "/app/ufs/Norflag.txt");

	fd = hal_fileOpen(Norflagname, FS_O_RDWR | FS_O_CREAT);
	if(fd < 0)
	{	
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> Open File Fail %d\r\n", fd);
		return FALSE;
	}
	
	iRet = hal_fileSeek(fd, 0, FS_SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> Seek File Fail %d\r\n", iRet);
		return FALSE;
	}
	
	cret = hal_fileWrite(fd, buff, 1);
	if(cret != (1))
	{
	
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> Write File Fail %d\r\n", cret);
		return FALSE;
	}

	iRet = hal_fileClose(fd);
	if(iRet != 0)
	{
	
		sysLOG(PWR_LOG_LEVEL_5, "<ERR> Close File Fail %d\r\n", iRet);
		return FALSE;
	}

	return cret;
	
	
	
}


/*
*Function:		hal_pmPwrOFF
*Description:	关机
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pmPwrOFF(void)
{
	int32 iRet;
	int ucKey = 0;

	g_stTTS_s.tts_enable = 0;//关闭tts播报队列

	iRet = hal_pmWriteNormalOFF("N");
	sysLOG(PWR_LOG_LEVEL_4, "PWR OFF ,hal_pmWriteNormalOFF iRet:%d\r\n", iRet);
#if QUICK_CHARGE
	hal_pmQChgWriteParam((char *)&g_stBatQuick.last_batV, 12, 0);
#endif
	
	if(gpsPowerUpFlag == 1)
	{
		iRet = gpsClose_lib();
		sysDelayMs(1000);

		if(iRet != 0)
		{
			iRet = gpsClose_lib();

			if(iRet != 0)
			{

				hal_scrCls();

			    hal_scrPrint(0, 3, 2, "GPS Close Fail!");

				hal_scrPrint(0, 4, 2, "Whether to turn it off?");

				hal_scrPrint(0, 6, 2, "Select KEYENTER/KEYCANCEL");

				while(1)
				{

					if(hal_keypadHit()==0) 
					{

						ucKey = hal_keypadGetKey();

						if(ucKey == KEYCANCEL)
						{

							return;

						}

						if(ucKey == KEYENTER)
						{

							hal_scrCls();

							break;

						}

					}

				}

			}

			sysDelayMs(1000);

		}
	}
	
	while(1)
	{
		
		if(g_stTTS_s.tts_ordercnt == 0)
		{
			sysLOG(PWR_LOG_LEVEL_2, "g_stTTS_s.tts_ordercnt:%d\r\n", g_stTTS_s.tts_ordercnt);
			break;
		}
		sysDelayMs(100);
	}

	if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)//合力泰屏幕消除关机竖线
	{
		hal_scrCls();
	 	fibo_lcd_write_reg_cus(0xae, NULL, 0); 
	 	sysDelayMs(100);
	}
	fibo_gpio_set(VBAT3V3EN_GPIO, false);//关闭3V3电源
	
	while(1)//播报完毕再关机
	{
		iRet = fibo_tts_is_playing();
		sysLOG(PWR_LOG_LEVEL_4, "while(1), fibo_tts_is_playing():%d\r\n", iRet);
		if(iRet == FALSE)
		{
		
			sysLOG(PWR_LOG_LEVEL_2, "fibo_tts_is_playing() == FALSE\r\n");
			break;
		}
		sysDelayMs(100);		
		//sysLOG(PWR_LOG_LEVEL_2, "fibo_tts_is_playing\r\n");
	}
	
	
	while(1)//按键释放再关机
	{
		
		if(g_ui8KeyPwroffFree == 0)
		{
			sysLOG(PWR_LOG_LEVEL_2, "g_ui8KeyPwroffFree:%d\r\n", g_ui8KeyPwroffFree);
			break;
		}
		sysDelayMs(100);
	}
	
	if(hal_pmGetChargerValue() >= 2)//插入充电线,重启
	{
		
		sysLOG(PWR_LOG_LEVEL_1, "PWR OFF fibo_softReset\r\n");		
		fibo_softReset();		
	}
	else
//未插入充电线，关机
	{
		
		sysLOG(PWR_LOG_LEVEL_1, "PWR OFF fibo_softPowerOff\r\n");

		fibo_softPowerOff();

	}

	
}


/*
*Function:		hal_pmPwrRST
*Description:	重启
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pmPwrRST(void)
{
	int32 iRet;
	g_stTTS_s.tts_enable = 0;//关闭tts播报队列
	sysLOG(PWR_LOG_LEVEL_1, "PWR RST \r\n");
	while(1)
	{
		
		if(g_stTTS_s.tts_ordercnt == 0)
		{
			sysLOG(PWR_LOG_LEVEL_2, "g_stTTS_s.tts_ordercnt:%d\r\n", g_stTTS_s.tts_ordercnt);
			break;
		}
		sysDelayMs(100);
	}
	
	if(g_ui32LcdDevID == HLTLCD_2_4_UC1617S_ID)//合力泰屏幕消除关机竖线
	{
		hal_scrCls();
	 	fibo_lcd_write_reg_cus(0xae, NULL, 0); 
	 	sysDelayMs(100);
	}
	while(1)//播报完毕再关机
	{
		iRet = fibo_tts_is_playing();
		if(iRet == FALSE)
		{
		
			sysLOG(PWR_LOG_LEVEL_2, "fibo_tts_is_playing() == FALSE\r\n");
			break;
		}
		sysDelayMs(100);		
	}
	
	fibo_softReset();
	
}

/*
*Function:		hal_pmSleep
*Description:	使终端进入休眠状态,以降低功耗
*Input:			DownCtrl:		输入参数,指向下电控制串(需以‘\0’结尾)。
*				DownCtrl[0]:	设置对上电状态的 IC 卡是否下电：‘0’－不下电,‘1’－下电，目前均支持
*				DownCtrl[1]:	设置对上电状态的非接触卡是否下电：‘0’－不下电,‘1’－下电，目前均支持
*				DownCtrl[2]:	表示是否使能 RTC 唤醒, 0x00:不开启 RTC唤醒功能,0x01~0xff:表示 RTC 唤醒的时间，单位为分钟若输入参数为 NULL,则默认为均下电 ,RTC 唤醒功能不开启。暂不支持RTC 唤醒
*				DownCtrl[3]:	表示WIFI是否下电；0-不下电 1-下电
*Output:		NULL
*Hardware:
*Return:		0-succ; other-failed
*Others:
*/
int hal_pmSleep(uchar *DownCtrl)
{
	
	int iRet = -1;
	uint8 keytmp = 0;
	char defaultCtrl[5]={"1111\x0"};
	unsigned char *popData;
	int datalen;
	ushort backLightVal,backmode,wifiStatus;
	g_ui8PwrkeyFlag = 0;
	unsigned char InfoTmp[64];
	int Ret = 0;
//	ushort secscr_backLightVal,secscr_backmode;

	iRet = fibo_get_Usbisinsert();
	sysLOG(PWR_LOG_LEVEL_2, "fibo_get_Usbisinsert == %d\r\n", iRet);
	if(iRet == 1)//usb插入
	{
		iRet = -1;
		goto exit;
	}
	//关闭断码屏背光，背光功耗25mA左右
	dpySetBackLight_lib(0);
	//g_iIconSuspend = 1;
	hal_keypadFlush();
	wifiStatus = g_stWifiState.cOpenState;
	//wifi关闭 SE休眠
	if(DownCtrl == NULL)
	{
		iRet = SE_Sleep(defaultCtrl); 
		if(wifiStatus == OPEN_SUCC)
		{
			wifiClose_lib();
		}
	}		
	else
	{
		iRet = SE_Sleep(DownCtrl);
		if(DownCtrl[3] == '1')
		{
			if(wifiStatus == OPEN_SUCC)
			{
				wifiClose_lib();
			}
		}
	}
	if(iRet < 0)
	{
		//goto exit;
		//失败的情况下考虑是否对SE强制下电
	}
	//iRet = fibo_setSleepMode(1);
	//if(iRet < 0)
	{
		//goto exit;
	}

	//缓存屏幕内容
	//datalen = hal_scrGetPopLen();
	//popData = (unsigned char*) fibo_malloc(datalen);
	//hal_scrPopDot(popData,datalen);

	//hal_scrCls();
	//sysDelayMs(1000);

	backLightVal = hal_scrGetBackLightTime();
	backmode = hal_scrGetAttrib(LCD_ATTR_BACKLIGHT_MODE);
	hal_scrSetBackLightMode(0,0);//关闭背光

//	secscr_backLightVal = hal_secscrGetBackLightTime();
//	secscr_backmode = hal_secscrGetAttrib(LCD_ATTR_BACKLIGHT_MODE);
//	hal_secscrSetBackLightMode(0,0);//关闭背光
	hal_secscrClose();//休眠时，直接关闭就可以

	//屏幕休眠
	//iRet = fibo_lcd_Sleep(1);
	
	while(1)
	{
		//keytmp = g_ui8PwrkeyFlag;
		
		if(hal_keypadHit() == 0)
		{
			//iRet = fibo_setSleepMode(0);
			//iRet = fibo_lcd_Sleep(0);
			//hal_scrInit();
			SE_awaken();
			hal_keypadFlush();
			iRet = 0;
			break;
		}
		iRet = fibo_get_Usbisinsert();
		if(iRet == 1)//usb插入
		{
			iRet = -1;
			SE_awaken();
			break;
		}			
		if(g_ucSeWakeupCallBackFlag == 1)
		{
			sysLOG(PWR_LOG_LEVEL_1, "g_ucSeWakeupCallBackFlag SE_awaken\r\n");
			hal_keypadFlush();
			g_ucSeWakeupCallBackFlag = 0;
			break;
		}			
		sysDelayMs(100);
	}
	sysLOG(PWR_LOG_LEVEL_0, "Exit SE_awaken\r\n");
	Ret = 0;
	while(1)
	{
		memset(InfoTmp, 0x00, sizeof(InfoTmp));
		if(sysReadVerInfo_lib(2, InfoTmp) == 0)
		{
			sysLOG(PWR_LOG_LEVEL_0, "sysReadVerInfo_lib return 0, Ret=%d\r\n", Ret);
			break;
		}
		else
		{
			if(Ret >= 2)
			{
				se_reboot();
				Ret = 0;
				sysLOG(PWR_LOG_LEVEL_0, "se_reboot\r\n");
				sysDelayMs(3000);
				break;
			}
		}
		sysDelayMs(50);
		Ret++;
		
	}
	
	//恢复屏幕内容
	sysDelayMs(10);
	//hal_scrPushDot(popData,datalen);
	hal_scrSetBackLightMode(backmode,backLightVal);//开启背光
//	hal_secscrSetBackLightMode(secscr_backmode,secscr_backLightVal);//开启背光
	if(DownCtrl == NULL)
	{
		if(wifiStatus == OPEN_SUCC)
		{
			wifiOpen_lib();
		}
	}		
	else
	{
		if(DownCtrl[3] == '1')
		{
			if(wifiStatus == OPEN_SUCC)
			{
				wifiOpen_lib();
			}
		}
	}

exit:
		//g_iIconSuspend = 0;
	sysLOG(PWR_LOG_LEVEL_2, "hal_pmSleep iRet=%d\r\n", iRet);
	return iRet;
	
}


/*
*Function:		hal_pmTimer2Period
*Description:	定时器3接口
*Input:			*arg:入参
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmTimer2Period(void *arg)
{
	
	if(g_ui8SystemStatus == 1)//充电循环中才可以进行指示灯控制等，正式开机会被开机流程接管指示灯以及电压检测
	{
		hal_pmBatChargerCheck(100);
		hal_pmTemperJudge(100, TEMPERHOT_LIMITVALUE);

		hal_ledRedHandle();
		if(hal_pmGetChargerValue()==2)//充电中
		{
			hal_ledRun(LEDRED, 2, 0);
		}
		else if(hal_pmGetChargerValue()==3)//充满
		{
			hal_ledRun(LEDRED, 1, 0);
		}
		else
		{
			hal_ledRun(LEDRED, 0, 0);		
		}
		
		
		g_iTimer3_1000ms++;
		if(g_iTimer3_1000ms >= 10)
		{
			g_iTimer3_1000ms = 0;
			if(g_iBackLightCnt > 0)
			{
				g_iBackLightCnt++;
			}
			if(g_iBackLightCnt > 10)
			{
				g_iBackLightCnt = 0;
				hal_scrSetBackLightMode(0, 10);
			}
			g_iBigBatIconTmp ++;
			if(g_iBigBatIconTmp > 4)
			{
				g_iBigBatIconTmp = 0;
			}
			
			if(hal_pmGetChargerValue()==2)//充电中
			{
				g_ui32NoChargePwron = 1;
				if(g_ui8LcdType == 0)
				{
					hal_scrWriteBigBatIcon(g_stLcdConfig.BIGBATTION_X, g_stLcdConfig.BIGBATTION_Y, g_iBigBatIconTmp);
				}
				else

				{
					hal_scrWriteBigBatIcon(g_stLcdConfig.COLORBIGBATTION_X, g_stLcdConfig.COLORBIGBATTION_Y, g_iBigBatIconTmp);
				}
			}
			else if(hal_pmGetChargerValue()==3)//充满
			{
				g_ui32NoChargePwron = 1;
				if(g_ui8LcdType == 0)
				{
					hal_scrWriteBigBatIcon(g_stLcdConfig.BIGBATTION_X, g_stLcdConfig.BIGBATTION_Y, 4);
				}
				else

				{
					hal_scrWriteBigBatIcon(g_stLcdConfig.COLORBIGBATTION_X, g_stLcdConfig.COLORBIGBATTION_Y, 4);
				}
				
			}
			
		}
	}

	
}


/*
*Function:		hal_pmGetChargerV
*Description:	判断是否充电
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0b0x-未充电，0b10-正在充电中,0b11-已充满
*Others:
*/
static uint8 hal_pmGetChargerV(void)
{
	int32 iRet;
	uint8 charger_v1=0,charger_v2=0;
	uint8 g_ui8ChgTmp = 0;
	
	iRet = fibo_gpio_get(CHG_ING_GPIO, &charger_v1);
	charger_v2 = fibo_get_Usbisinsert();
	
	g_ui8ChgTmp = charger_v1 | (charger_v2 << 1);
	return g_ui8ChgTmp;

}


/*下电情景：1，未充电情况下的开机键非长按；2，充电中途拔线且没有按下开机键*/

/*
*Function:		hal_pmPwrOffAssign1
*Description:	下电情景1，有正常关机标志位，上电伊始，未充电情况下的开机键非长按
*Input:			pwrkeyjdgV:是否有开机键长按事件，0-没有长按开机键;1-有长按开机键
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmPwrOffAssign1(int32 pwrkeyjdgV)
{
	int32 iRet;
	
	iRet = pwrkeyjdgV;
	if(iRet == 0)//非长按开机键
	{
		
		iRet = hal_pmGetChargerV();
		if(iRet <= 1)//未插充电线
		{
			
			sysLOG(PWR_LOG_LEVEL_1, "hal_pmGetChargerV,iRet:%d\r\n", iRet); 
						
			fibo_softPowerOff();

		}
	}
}


/*
*Function:		hal_pmPwrOffAssign2
*Description:	下电情景2，有正常关机标志位，充电中途拔线且没有按下开机键，或者未充电开机键非长按
*Input:			pwrkeyjdgV:是否有开机键长按事件，0-没有长按开机键;1-有长按开机键
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pmPwrOffAssign2(int32 pwrkeyjdgV)
{
	int32 iRet;
	uint8 IO_value = 0;
	
	iRet = hal_pmGetChargerV();

	if(iRet <= 1)//未充电
	{
		if(IO_value == 0 && g_ui8SystemStatus <= 1)//开机键未按下且未检测到长按
		{
					
			if(pwrkeyjdgV == 0)//不是按键启动
			{

				fibo_softPowerOff();

			}
		}
	}
	
}

/*
*Function:		hal_pm3V3Init
*Description:	3.3V电源初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_pm3V3Init(void)
{
	
	fibo_gpio_mode_set(VBAT3V3EN_GPIO, GpioFunction2);
	fibo_gpio_cfg(VBAT3V3EN_GPIO, GpioCfgOut);
	fibo_gpio_set(VBAT3V3EN_GPIO, false);
	
}


/*
*Function:		hal_pmLoopInit
*Description:	开机检测以及部分初始化接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_pmLoopInit(void)
{
	int32 iRet;
	int32 firstjudge = 0;
	uint8 iotmp = 0;
	uint32 uTime;
	uint8 charger_v2 = 0;
	uint16 PwrOnCause = 0;
	uint8 i=0;
	sysLOG(PWR_LOG_LEVEL_1, "\r\n"); 
	Sem_AT_signal = fibo_sem_new(1);
	hal_config_Init();
	fibo_watchdog_enable(60);
	//fibo_watchdog_disable();
	hal_GetBpVersion();
	hal_sysGetHwVersion();

	keypad_info_t Pwrkey_info;
	for(i=0; i<20; i++)
	{
		if(g_ui8LcdLogoRefresh == 1)
		{
			if(firstjudge != 1)
			{
				if(fibo_getbootcause() == 2)//0-softReset;1-RSTPinReset;2-PwrkeyReset;3-USBchargeReset
				{
					firstjudge = 1;
					sysLOG(PWR_LOG_LEVEL_4, "fibo_getbootcause() == 2\r\n"); 
					break;
				}
			}
		}
		Pwrkey_info = fibo_get_pwrkeypad_status();
		if(Pwrkey_info.press_or_release == 1)
		{
			firstjudge = 1;
			sysLOG(PWR_LOG_LEVEL_4, "Pwrkey_info.press_or_release == 1\r\n");
			break;
		}
		sysDelayMs(10);
	}
	sysLOG(PWR_LOG_LEVEL_1, "i=%d,fibo_get_pwrkeypad_status key_id=%d,press_or_release=%d,long_or_short_press=%d,fibo_getbootcause=%d\r\n", i, Pwrkey_info.key_id, Pwrkey_info.press_or_release, Pwrkey_info.long_or_short_press, fibo_getbootcause()); 

	fibo_lpg_switch(0);
	hal_pmChargerInit();
	hal_sysCreateUsrDir();
	hal_keypadInit();
	hal_pm3V3Init();
	dev_cblcdInit();
	hal_ledInit();
	fibo_gpio_set(VBAT3V3EN_GPIO, TRUE);//开启3V3电源
	
	g_ui32MutexLcdHandle = fibo_mutex_create();
	fibo_mutex_unlock(g_ui32MutexLcdHandle);
	
	for(uint32 i=0; i<50; i++)
	{
		hal_pmBatChargerCheck(1000);//定时器反应慢，关机充电时充电灯延时点亮，所以需要做个循环计算充电状态
	}
	hal_LogoInfoInit();
	
	if(g_ui8LcdLogoRefresh == 1 && hal_fileExist(g_stLogoinfoJson.logofile) == 1)//厂家初始化LCD屏则直接将PWM打开即可
	{
		g_stLcdGUI.BackLightEN = 1;
		g_stColorlcdGUI.BackLightEN = 1;
		hal_scrGpioInit((g_stLogoinfoJson.colorlcd_bl_pwm)/4/2.55);
		
	}
	else//厂家未初始化LCD和刷logo，需要自己清屏
	{
		hal_scrGpioInit(0);
	}
	g_ui8LcdType = hal_clcdInitfirst();//hal_scrInit();
	if(g_ui8LcdType == 0)
	{
		g_ui8LcdType = 0;
		hal_lcdInit();
		
		if(g_ui8LcdLogoRefresh == 0)//厂家未初始化LCD和刷logo，需要自己清屏
		{
			hal_scrClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, g_stLcdConfig.LCD_PIXHIGH-1);				
		}
		
	}
	else

	{
		g_ui8LcdType = 1;
		hal_clcdInit();
		if(g_ui8LcdLogoRefresh == 0)//厂家未初始化LCD和刷logo，需要自己清屏
		{

			hal_scrClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
		}
		
	}
	
	g_i8NormalFlag=hal_pmReadNormalOFF();
	if(g_i8NormalFlag != 'N')//非正常关机，直接启动即可
	{
		if(hal_fileExist(g_stLogoinfoJson.logofile) != 1)//如果logo文件不存在则艾体刷logo
		{
			if(g_ui8LcdType == 0)
			{
				
				hal_scrClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, g_stLcdConfig.LCD_PIXHIGH-1);
				hal_scrWriteLogo(g_stLogoinfoJson.lcd_position_x, g_stLogoinfoJson.lcd_position_y, gImage_New_Aisino_128x32);
				
			}
			else
			{
				hal_clcdSetAttrib( LCD_ATTR_BACK_COLOR, g_stLogoinfoJson.colorlcd_bg_color);
				hal_clcdSetAttrib( LCD_ATTR_FRONT_COLOR, g_stLogoinfoJson.colorlcd_fg_color);
				
				hal_scrClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
		
				hal_scrWriteLogo(g_stLogoinfoJson.colorlcd_position_x, g_stLogoinfoJson.colorlcd_position_y, gImage_New_Aisino_128x32);

			}
		}
		
		iRet = fibo_timer_free(g_ui32Timer2ID);//开机并释放电源键后，就不需要走这个DR_KeyJudge_Pwr了，释放掉即可			
		sysLOG(PWR_LOG_LEVEL_2, "fibo_timer_free, iRet = %d\r\n", iRet); 
		//DR_KeyCB_Reg_Pwr(NULL);//正常启动之后就不需要这个回调了
		g_stKeyTable.key_s_pwr.key_value = 0;
		g_ui8KeyPwronFree = 0;
		g_ui8SystemStatus = 2;//启动开机流程
	}
	else//正常关机情况下
	{
		hal_clcdSetAttrib( LCD_ATTR_FRONT_COLOR, BLACK);
		if(firstjudge == 1)//确认到是长按键开机
		{
			g_ui8SystemStatus = 2;//启动开机流程
			if(hal_fileExist(g_stLogoinfoJson.logofile) != 1)//如果logo文件不存在则艾体刷logo
			{
				if(g_ui8LcdType == 0)
				{
					
					hal_scrClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, g_stLcdConfig.LCD_PIXHIGH-1);
					hal_scrWriteLogo(g_stLogoinfoJson.lcd_position_x, g_stLogoinfoJson.lcd_position_y, gImage_New_Aisino_128x32);
					
				}
				else
				{
					hal_clcdSetAttrib( LCD_ATTR_BACK_COLOR, g_stLogoinfoJson.colorlcd_bg_color);
					hal_clcdSetAttrib( LCD_ATTR_FRONT_COLOR, g_stLogoinfoJson.colorlcd_fg_color);
					
					hal_scrClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
					
					hal_scrWriteLogo(g_stLogoinfoJson.colorlcd_position_x, g_stLogoinfoJson.colorlcd_position_y, gImage_New_Aisino_128x32);				
				}
			}
			 
		}
		else
		{
			if(g_ui8LcdLogoRefresh == 1)//厂家有刷logo，关机充电模式需要清掉logo
			{
				if(g_ui8LcdType == 0)
				{
					hal_scrClsArea(0, 0, g_stLcdConfig.LCD_PIXWIDTH-1, g_stLcdConfig.LCD_PIXHIGH-1);
				}
				else

				{
					hal_scrClsArea(0, 0, g_stLcdConfig.COLORLCD_PIXWIDTH-1, g_stLcdConfig.COLORLCD_PIXHIGH-1);
				}
			}
			hal_pmPwrOffAssign1(firstjudge);
		}
		g_ui32Timer2ID = fibo_timer_period_new(100, hal_pmTimer2Period, NULL);
	}
	
	hal_pkeyInit();
	DR_Wifi_Pwr_init();/*wifi模块处于下电状态,并把串口TX RX配置为普通IO*/
	
	//DR_KeyCB_Reg_Pwr(hal_keypadPwrkeyDeal);
	hal_keypadReg(hal_pkeyDeal);
	
	
	sysLOG(PWR_LOG_LEVEL_3, "hal_scrSetBackLightValue g_stColorlcdGUI.BackLight=%d,g_stLcdGUI.BackLight=%d\r\n", g_stColorlcdGUI.BackLight, g_stLcdGUI.BackLight);
	hal_scrSetBackLightValue((g_stLogoinfoJson.colorlcd_bl_pwm)/4/2.55);

	hal_segInit();

	if (g_iCamera_exist==1)
	{
		hal_camInit();
	}

	scanpos_se_comm_init();
	
	while(1)
	{
		hal_keypadHandle();
		hal_pkeyHandle();
		if(g_ui8SystemStatus <= 1)
		{
			hal_pmPwrOffAssign2(firstjudge);
			
		}
		else
//正常开机
		{
			/*程序起来后再清Write_NormalOFFflag*/
			iRet = hal_pmWriteNormalOFF("\0");		
			sysLOG(PWR_LOG_LEVEL_1, "hal_pmWriteNormalOFF iRet = %d\r\n", iRet); 
			//fibo_gpio_get(KEY_PWR_GPIO, &iotmp);//需要适配到新的开机键检测
			if(iotmp == 0)//确认到按键已经释放，则释放关机下的定时器检测
			{
				iRet = fibo_timer_free(g_ui32Timer2ID);//开机并释放电源键后，就不需要走这个DR_KeyJudge_Pwr了，释放掉即可			
				sysLOG(KEYS_LOG_LEVEL_2, "fibo_timer_free, iRet = %d\r\n", iRet); 
				//DR_KeyCB_Reg_Pwr(NULL);//正常启动之后就不需要这个回调了
				g_stKeyTable.key_s_pwr.key_value = 0;
				g_ui8KeyPwronFree = 0;
			}			
			//sysLOG(PWR_LOG_LEVEL_2, "KEY_PWR_GPIO, iotmp:%d\r\n", iotmp);
			break;
		}
		
		sysDelayMs(100);
	}
}



