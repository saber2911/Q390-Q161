#ifndef _LC610N_SE_BASIC_H_
#define _LC610N_SE_BASIC_H_

#include "comm.h"

#define SE_COMMUNICATION_CTRL		29
#define SE_CB_CTRL					28
#define SE_REBOOT_IO				65

#define SE_CB_QUEUE_LEN			256
#define WAKEUP_CALLBACK_FLAG	0x02
#define KEY_CALLBACK_STATUS     0x20
#define TM_CALLBACK_STATUS      0x04   //触发拉回调
#define PORT_CALLBACK_STATUS    0x40   //通讯串口

#define SELF_CHECK_START								0x2000		//正在初始化
#define SELF_CHECK_USER_ERR								0x2001		//用户密码初始化失败
#define SELF_CHECK_TAMPER								0x2002		//设备已经触发
#define SELF_CHECK_OVER									0x2003		//设备自检已经完成
#define SELF_CHECK_HARD                 				0x2004      //硬件触发


extern uint32 SE_callBack_Queue;

void IO_test_4G_RTS(void);

void config_se_communicationCtrl_io(void);
void config_se_callBack_Ctrl(void);
void config_se_callBack_Ctrl_NoIRQ(void);
int SE_Sleep(uchar *DownCtrl);
void se_reboot(void);
int get_se_cb_status(void);
int SE_awaken(void);
int checkEvent(int* msgType);
void se_callBack_IRQ(void *para);
int Se_GetHardwareTriggerStatus(unsigned int* TriggerStatus);


#endif

