
/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_at.c
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

#include "comm.h"



uint8 g_ui8ATFlag = 0;//调用AT标志位，0-无调用；1-外部串口有调用;2-内部AT有调用
uint32 Sem_AT_signal;//内部接口调用AT回调信号量

struct _BUFF_STRUCT g_stATCbBuffStruct;//接口调用AT指令返回的结果


/*
*Function:		hal_atResCallback
*Description:	虚拟AT接收回调接口
*Input:			*buf:AT返回的数据;len:AT返回的数据长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_atResCallback(INT8 *buf, UINT16 len)
{
    sysLOG(AT_LOG_LEVEL_4, "hal_atResCallback, g_ui8ATFlag=%d, len=%d, buf:%s\r\n", g_ui8ATFlag, len, buf);
	
	if(g_ui8ATFlag == 2)//内部调用AT
	{
	
		CBuffWrite(&g_stATCbBuffStruct, buf, len);
	}
}


/*
*Function:		hal_atSend
*Description:	虚拟AT发送
*Input:			*buf:AT内容指针；len:AT内容长度
*Output:		NULL
*Hardware:
*Return:		<0:发送失败；>=0:发送成功的字节数
*Others:
*/
int hal_atSend(int8 *buf, uint16 len)
{
	int iRet = -1;
	
	sysLOG(AT_LOG_LEVEL_4, "hal_atSend, buf:%s, len=%d\r\n", buf, len);
	iRet = fibo_at_send((unsigned char *)buf, len);
	return iRet;
}




