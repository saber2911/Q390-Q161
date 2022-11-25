#include "comm.h"
extern unsigned char ucPortRecvFlag;
uint32 SE_callBack_Queue;//SE拉回调队列，610需发送查询状态指令
unsigned char g_ucSeWakeupCallBackFlag = 0;
unsigned char g_aucSeSleepPara[5] = {0,0,0,0,0};
//拉IO调试程序时使用，需屏蔽
void IO_test_4G_RTS(void)
{
	fibo_gpio_mode_set(11, GpioFunction0);
	fibo_gpio_cfg(11, GpioCfgOut);		
	fibo_gpio_set(11,FALSE);	
}
/* brief  : 和SE通信流控IO初始化
 * param in: NULL
 * retval out: NULL
 * return : NULL
*/
void config_se_communicationCtrl_io(void)
{
	fibo_gpio_mode_set(SE_COMMUNICATION_CTRL, GpioFunction1);
	fibo_gpio_cfg(SE_COMMUNICATION_CTRL, GpioCfgIn);
	fibo_gpio_pull_high_resistance(SE_COMMUNICATION_CTRL, true);
}


/* brief  : 和SE通信流控IO初始化
 * param in: NULL
 * retval out: NULL
 * return : NULL
*/
void config_se_communicationRTS_io(void)
{
	fibo_gpio_mode_set(SE_CB_CTRL, GpioFunction1);
	fibo_gpio_cfg(SE_CB_CTRL, GpioCfgIn);	
	fibo_gpio_pull_high_resistance(SE_CB_CTRL, true);
}

/* brief  : 和SE通信回调初始化(边沿中断方式)，SE拉高回调IO后，LC610需发送查询指令
 * param in: NULL
 * retval out: NULL
 * return : NULL
*/
void config_se_callBack_Ctrl(void)
{
//	fibo_gpio_mode_set(SE_COMMUNICATION_CTRL, GpioFunction1);
//	fibo_gpio_cfg(SE_COMMUNICATION_CTRL, GpioCfgIn);		

	oc_isr_t se_callBack_oc_isr_t = {
		.is_debounce = true,
		.intr_enable = true,  
		.intr_level = false,   
		.intr_falling = false,   
		.inte_rising = true, 
		.callback = se_callBack_IRQ,  
	};
	fibo_gpio_isr_init(SE_COMMUNICATION_CTRL, &se_callBack_oc_isr_t);
}

/* brief  : 和SE通信回调初始化(边沿中断方式)，SE拉高回调IO后，LC610需发送查询指令
 * param in: NULL
 * retval out: NULL
 * return : NULL
*/
void config_se_callBack_Ctrl_NoIRQ(void)
{

//	fibo_gpio_mode_set(SE_COMMUNICATION_CTRL, GpioFunction1);
//	fibo_gpio_cfg(SE_COMMUNICATION_CTRL, GpioCfgIn);		

	oc_isr_t se_callBack_oc_isr_t = {
		.is_debounce = true,
		.intr_enable = false,  
		.intr_level = false,   
		.intr_falling = false,   
		.inte_rising = false, 
		.callback = se_callBack_IRQ,  
	};
	fibo_gpio_isr_init(SE_COMMUNICATION_CTRL, &se_callBack_oc_isr_t);
}


/* brief  : 查询SE回调IO状态
 * param in: NULL
 * retval out: NULL
 * return : ctrl_level 电平状态
*/
int get_se_cb_status(void)
{
	int ret = 0;
	unsigned char ctrl_level = 0;
	ret = fibo_gpio_get(SE_CB_CTRL,&ctrl_level);
	if(ret<0)
	{
		sysLOG(BASE_LOG_LEVEL_1, " ret = %d\r\n",ret);
		return ret;
	}
	return ctrl_level;
}


/* brief  : 复位SE
 * param in: NULL
 * retval out: NULL
 * return : NULL
*/
void se_reboot(void)
{
	
	fibo_gpio_set(SE_REBOOT_IO,TRUE);
	sysDelayMs(20);
	fibo_gpio_set(SE_REBOOT_IO,FALSE);
	
}

/* brief  : 查询SE端状态，IC卡插卡拔卡等
 * param in: NULL
 * retval out: *msgType SE端状态
 * return : <0 error
*/
int checkEvent(int* msgType)
{
    int ret = 0,i;
	uint32 keyidtmp = 0xFF;
    unsigned char cmd_check_event[] = {0x00, 0xa1, 0x21, 0x00, 0x00, 0x00};
    Frame frm, retFrm;
    ret = frameFactory(cmd_check_event, &frm, 0x40, 6, 0x01, 0x00);
    if(ret < 0) 
	{
        return -1;
    }
    ret = transceiveFrame(frm, &retFrm, 1000);
    if(ret < 0) 
	{
        free(frm.data);
        return ret;
    }
    ret = retFrm.data[2]<<8 | retFrm.data[3];
    if(0x9000 == ret && msgType != NULL) 
	{
        *msgType = retFrm.data[9]<<24 | retFrm.data[8]<<16 | retFrm.data[7]<<8 | retFrm.data[6];
		sysLOG(BASE_LOG_LEVEL_1, " *msgType=%x\r\n",*msgType);
		if(*msgType & WAKEUP_CALLBACK_FLAG)
		{
		  is_se_sleepping = 0;
		  g_ucSeWakeupCallBackFlag = 1;
		  sysLOG(API_LOG_LEVEL_2, " WAKEUP_CALLBACK_FLAG\r\n");
		}
		if(*msgType & KEY_CALLBACK_STATUS)
		{
			hal_scrBackLightWakeup();
            for (i = 0; i < retFrm.data[10]; i++)
            {
                keyidtmp = retFrm.data[i + 11];
                ret = fibo_queue_put(g_ui32QueueKeyValue, &keyidtmp, 100);
            }
        }
		if(*msgType & PORT_CALLBACK_STATUS)
		{
		  ucPortRecvFlag = 1;
		  sysLOG(API_LOG_LEVEL_2, " wait recv\r\n");
		}
    }
    free(frm.data);
    free(retFrm.data);

    return ret;
}


void se_callBack_IRQ(void *para)
{
	char msg = 1;
	sysLOG(BASE_LOG_LEVEL_4, "se_callBack_IRQ\r\n");
	fibo_queue_put_isr(SE_callBack_Queue,&msg);
}

/* brief  : 唤醒SE，拉IO或串口发送数据
 * param in: NULL
 * retval out: NULL
 * return : ret 串口数据发送情况
*/
int SE_awaken(void)
{
	sysLOG(BASE_LOG_LEVEL_1, " SE_awaken\r\n");		

    unsigned char cmd_se_awaken[] = {0x55,0x55};
    int ret = sendRowData(cmd_se_awaken, sizeof(cmd_se_awaken));

	//TODO 拉io唤醒SE
	
    is_se_sleepping = 0;
    sysDelayMs(1000);

    return ret;
}

/*
*
*@Brief:		使终端进入休眠状态,以降低功耗
*@Param IN:		DownCtrl[0]：设置对上电状态的 IC 卡是否下电：‘ 0’－不下电,‘ 1’－下电， 目前均支持
				DownCtrl[1]：设置对上电状态的非接触卡是否下电：‘ 0’－不下电,‘ 1’－下电， 目前均支持
				DownCtrl[2]：设置磁条卡上电状态的  卡是否下电：‘ 0’－不下电,‘ 1’－下电， 目前均支持
				DownCtrl[3]：设置SAM卡是否下电：‘ 0’－不下电,‘ 1’－下电， 目前均支持
				DownCtrl[4]：表示是否使能 RTC 唤醒, 0x00:不开启 RTC唤醒功能,0x01~0xff:表示 RTC 唤醒的时间,暂不支持RTC 唤醒
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int SE_Sleep(uchar *DownCtrl)
{
	uchar sendData[5]={0,0,0,0,0};
	int i = 0, j = 0;
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 11;
	unsigned int paraLen = strlen(DownCtrl);

	if(paraLen<=5)
	{
		for(i=0; i<paraLen; i++)
		{
			sendData[i]=DownCtrl[i];
			g_aucSeSleepPara[i] = DownCtrl[i];
		}
	}
	for(j=0; j<4; j++)
	{
		if((sendData[j] != '1') && (sendData[j] != '0'))
		{
			sendData[j]='1';
			g_aucSeSleepPara[i] = '1';
		}
	}
	unsigned char ucCmdHead[11] = {0x00, 0xE3, 0x00, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, sendData[0], sendData[1], sendData[2],sendData[3], sendData[4]};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(BASE_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
	fibo_free(frm.data);

	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = RET_RF_OK;
		is_se_sleepping = 1;
	}
	else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
    }
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(BASE_LOG_LEVEL_1, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/****************查询状态*********************
#define SELF_CHECK_START								0x2000		//正在初始化
#define SELF_CHECK_USER_ERR								0x2001		//用户密码初始化失败
#define SELF_CHECK_TAMPER								0x2002		//设备已经触发
#define SELF_CHECK_OVER									0x2003		//设备自检已经完成
#define SELF_CHECK_HARD                 				0x2004      //硬件触发
*/
int check_se_status()
{
    int ret = 0;
    unsigned char cmd_get_status[] = {0x00, 0xa1, 0x02, 0x00, 0x00, 0x00};
    Frame frm, retfrm;
    ret = frameFactory(cmd_get_status, &frm, 0x40, 6, 0x01, 0x00);
    if(ret < 0)
        return ret;
    ret = transceiveFrame(frm, &retfrm, 500);
    free(frm.data);
    if(ret <0)
        return ret;
    ret=retfrm.data[2]<<8 | retfrm.data[3];
    free(retfrm.data);
	sysLOG(BASE_LOG_LEVEL_1, " ret = 0x%x, ret = %d\r\n", ret, ret);	
    return ret;
}

/*
*@Brief:		获取硬件触发寄存器值 
*@Param IN:		无				
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int Se_GetHardwareTriggerStatus(unsigned int* TriggerStatus)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xA6, 0x01, 0x04, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(BASE_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		(*TriggerStatus) = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		iRet = RET_RF_OK;
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(BASE_LOG_LEVEL_1, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

