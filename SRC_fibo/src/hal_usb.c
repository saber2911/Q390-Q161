/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_usb.c
** Description:		USB相关接口
**
** Version:	1.0, 渠忠磊,2022-03-09
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"

static atDevice_t *UsbHandle = NULL;
uint8 DownLoadVOSFlag = 0; //0:当前没有下载VOS；1：正在下载VOS
unsigned long long g_ullStartTime = 0;
unsigned long long g_ullEndTime = 0;
struct _BUFF_STRUCT g_stUsbBuffStr;//USB数据缓存结构体
static int32 *g_i32UsbThreadHandle = NULL;/*USB数据处理线程*/
static uint32 g_ui32MutexUsbHandle;//USB锁
unsigned char first_find_Q = 0;
uint8 ui8USBStatus = 0;//0-closed;1-opened
extern unsigned int se_comm_mutex;
#define USBBUFFLEN			1024*10

uint32 usbdev_daemon_sem = 0;//usb device daemon sem,indicate usb dev insert or out

typedef enum
{
    DRV_SERIAL_EVENT_BROKEN = (1 << 3),      ///< usb port broken
    DRV_SERIAL_EVENT_READY = (1 << 4),       ///< usb port ready
    DRV_SERIAL_EVENT_OPEN = (1 << 5),        ///< open by host
    DRV_SERIAL_EVENT_CLOSE = (1 << 6),       ///< close by host
}usbEvent;


/*
*Function:		hal_portUsbEvent
*Description:	USB事件通知接口
*Input:			arg:USB事件类型
*Output:		NULL
*Hardware:
*Return:		<0:失败;0:成功
*Others:
*/
int hal_portUsbEvent(int arg)
{
	sysLOG(PORT_LOG_LEVEL_1, " usbdev report msg arg=0x%x\r\n",arg);	

	osiEvent_t extkb_event = {0};
	
	switch (arg)
	{
		case DRV_SERIAL_EVENT_BROKEN://usb remove
			sysLOG(PORT_LOG_LEVEL_4, " the usbdev had broken!\r\n");
		break;
		case DRV_SERIAL_EVENT_READY://usb insert
			sysLOG(PORT_LOG_LEVEL_4, " the usbdev had ready!\r\n");
		break;
		case DRV_SERIAL_EVENT_OPEN://upper computer open usb port
			sysLOG(PORT_LOG_LEVEL_4, " the usbdev had opened!\r\n");
			fibo_sem_signal(usbdev_daemon_sem);
		break;
		case DRV_SERIAL_EVENT_CLOSE://upper computer close usb port
			sysLOG(PORT_LOG_LEVEL_4, " the usbdev had closed!\r\n");	
		break;
		default:
			sysLOG(PORT_LOG_LEVEL_4, " usbdev unkonw state!\r\n");  
		break;
	}
	
	extkb_event.id = arg;
	osiEventTrySend(g_i32UsbThreadHandle, &extkb_event, OSI_WAIT_FOREVER);
	
	return 0;
}

/*
*Function:		hal_portUsbSEHandle
*Description:	USB透传升级VOS的函数句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败;>=0:成功
*Others:
*/
int hal_portUsbSEHandle(void)
{
	int iRet = 0;

	unsigned int avlid_len = 0;
	unsigned int read_len = 0;
	unsigned long long uTime;
	unsigned char *rP = NULL;
	rP = (unsigned char*)malloc(USBBUFFLEN);
	if(rP == NULL)
	{
		iRet = -1;
		goto exit;
	}
	memset(rP, 0, USBBUFFLEN);
	
	if(CBuffGetWrittenLen(&g_stUsbBuffStr) > 0)
	{
		avlid_len = CBuffGetWrittenLen(&g_stUsbBuffStr);
		sysDelayMs(2);
		if((CBuffGetWrittenLen(&g_stUsbBuffStr) - avlid_len) == 0)
		{
			memset(rP, 0, USBBUFFLEN);
			read_len = hal_portUsbRecv(rP, avlid_len, 1);
			if((0 == first_find_Q)&&(1 == read_len))//判断第一次接收到上位机的Q，复位SE进入boot
			{
				if('Q' == *rP)
				{
					fibo_mutex_lock(se_comm_mutex);//lock
					config_se_callBack_Ctrl_NoIRQ();
					hal_utSEUartInit(DOWNLOAD_RATE);
					sysDelayMs(2);
					DownLoadVOSFlag = 1;
			
					first_find_Q = 1;
					uTime = hal_sysGetTickms() + 1000;
					se_reboot();
					sysLOG(PORT_LOG_LEVEL_1, " reboot se\r\n");
					while(1)
					{
						iRet = fibo_hal_uart_put(SE_COMMUNICATION_PORT, rP, read_len);
						if(get_ringBuffer_valid(&se_rev_ringbuff) > 0)
							break;
						g_ullStartTime = hal_sysGetTickms();

						if(g_ullStartTime > uTime)
							break;
						
						sysDelayMs(5);
					}
				}
			}
			iRet = fibo_hal_uart_put(SE_COMMUNICATION_PORT, rP, read_len);
	        if(iRet < 0 || iRet != read_len)
	        {
				sysLOG(PORT_LOG_LEVEL_2, "send to usb error,iRet=%d\r\n",iRet);		        
	        }
		}
		
	}

	if(get_ringBuffer_valid(&se_rev_ringbuff) > 0)//接收SE数据转发给USB
	{	
		g_ullStartTime = hal_sysGetTickms();	
		avlid_len = get_ringBuffer_valid(&se_rev_ringbuff);
		sysDelayMs(2);
		if((get_ringBuffer_valid(&se_rev_ringbuff) - avlid_len) == 0)//判断是否接收完成
		{				
			bzero(rP, USBBUFFLEN);
			read_len = ring_buffer_read(&se_rev_ringbuff, rP, avlid_len);
			sysLOG(PORT_LOG_LEVEL_4, "se_uart_read_len = %d\r\n", read_len);
			iRet = hal_portUsbSend(rP, read_len);
			if(iRet < 0 || iRet != read_len)
			{
				sysLOG(PORT_LOG_LEVEL_2, "send to se error,iRet=%d\r\n", iRet);
			}				
		}		
	}
	
exit:

	if(rP != NULL)
	{
		free(rP);
	}
	return iRet;
	
}


/*
*Function:		hal_portUsbSECommitHandle
*Description:	USB透传SE指令的函数句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败;>=0:成功
*Others:
*/
int hal_portUsbSECommitHandle(void)
{
	int iRet = 0;

	unsigned int avlid_len = 0;
	unsigned int read_len = 0;
	unsigned char *rP = NULL;
	rP = (unsigned char*)malloc(USBBUFFLEN);
	if(rP == NULL)
	{
		iRet = -1;
		goto exit;
	}
	memset(rP, 0, USBBUFFLEN);

	if(CBuffGetWrittenLen(&g_stUsbBuffStr) > 0)
	{
		avlid_len = CBuffGetWrittenLen(&g_stUsbBuffStr);
		sysDelayMs(2);
		if((CBuffGetWrittenLen(&g_stUsbBuffStr) - avlid_len) == 0)
		{
			memset(rP, 0, USBBUFFLEN);
			read_len = hal_portUsbRecv(rP, avlid_len, 1);
			iRet = fibo_hal_uart_put(SE_COMMUNICATION_PORT, rP, read_len);
	        if(iRet < 0 || iRet != read_len)
	        {
				sysLOG(PORT_LOG_LEVEL_2, "send to usb error,iRet=%d\r\n",iRet);		        
	        }
		}
		
	}

	if(get_ringBuffer_valid(&se_rev_ringbuff) > 0)//接收SE数据转发给USB
	{		
		avlid_len = get_ringBuffer_valid(&se_rev_ringbuff);
		sysDelayMs(2);
		if((get_ringBuffer_valid(&se_rev_ringbuff) - avlid_len) == 0)//判断是否接收完成
		{				
			bzero(rP, USBBUFFLEN);
			read_len = ring_buffer_read(&se_rev_ringbuff, rP, avlid_len);
			sysLOG(PORT_LOG_LEVEL_4, "se_uart_read_len = %d\r\n", read_len);
			iRet = hal_portUsbSend(rP, read_len);
			if(iRet < 0 || iRet != read_len)
			{
				sysLOG(PORT_LOG_LEVEL_2, "send to se error,iRet=%d\r\n", iRet);
			}				
		}		
	}

exit:

	if(rP != NULL)
	{
		free(rP);
	}
	return iRet;
	
}

/*
*Function:		hal_portUsbWiFiHandle
*Description:	USB透传升级WiFi的函数句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败;>=0:成功
*Others:
*/
int hal_portUsbWiFiHandle(void)
{
	int ret = 0, i=0;
	
	unsigned int avlid_len = 0;
	unsigned int read_len = 0;	
	unsigned char write_buffer[4200];
	int RxFlashFlag = 0;
	int iLen = 0;
	unsigned char lrc = 0;
	unsigned char *rP = NULL;
	
	rP = (unsigned char*)malloc(USBBUFFLEN);
	if(rP == NULL)
	{
		ret = -1;
		goto exit;
	}
	memset(rP, 0, USBBUFFLEN);
	
START:
	if(CBuffGetWrittenLen(&g_stUsbBuffStr) > 0)//接收USB数据转发给SE
	{
		
		read_len =  CBuffRead(&g_stUsbBuffStr, rP, 1);
		sysLOG(PORT_LOG_LEVEL_5, " rP[0]=%d %d %d %d %d\r\n",rP[0],rP[1],rP[2],rP[3],rP[4]);	
		if(rP[0] == SHAKE_REQUEST)//判断第一次接收到上位机的Q，复位SE进入boot
		{
			write_buffer[0] = SHAKE_REPLY;
			ret = hal_portUsbSend(write_buffer, 1);
			if(ret < 0 || ret != read_len)
			{
				sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
			}
		}
		else if(rP[0] == CMD_POS_NAME)
		{
            write_buffer[0] = CMD_POS_NAME_REPLY;
			ret = hal_portUsbSend(write_buffer, 1);
			if(ret < 0 || ret != read_len)
			{
				sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
			}
		}
		else if(rP[0] == CMD_TRASHY)
		{
			goto START;
		}
		else if(rP[0] != STX)
		{
			goto START;
		}
		else if(rP[0] == STX)
		{
			lrc = 0;
			
			while(avlid_len<3)
			{
				avlid_len = CBuffGetWrittenLen(&g_stUsbBuffStr);
				sysDelayMs(1);
			}
			CBuffRead(&g_stUsbBuffStr, rP+1, 3);

			iLen = (rP[2]<<8) + rP[3];

			while(avlid_len < iLen+1)
			{
				avlid_len = CBuffGetWrittenLen(&g_stUsbBuffStr);
				sysDelayMs(2);
			}

			CBuffRead(&g_stUsbBuffStr, rP+4, iLen+1);

			for(i=0; i<iLen+3; i++)
			{
				lrc ^= rP[i+1];
			}
			sysLOG(PORT_LOG_LEVEL_4, " lrc0=%d lrc1=%d\r\n",lrc,rP[iLen+4]);
			if(rP[iLen+4] != lrc)
			{
				return ;
			}
			else{
				if(rP[1] == 0xe4)
				{
					write_buffer[0] = ACK;
					ret = hal_portUsbSend(write_buffer, 1);
					if(ret < 0 || ret != read_len)
					{
						sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
					}
				}
			}

			lrc = 0;
			if(rP[1]==0xe4)
			{
				sysLOG(PORT_LOG_LEVEL_4, " sendiLen=%d\r\n",iLen-2);	
				wifiUpgradeSendData_lib(rP+6, iLen-2, 0);
				write_buffer[0] = 0x02;
				write_buffer[1] = 0xe3;
				write_buffer[2] = 0x00;
				write_buffer[3] = 0x00;
				for(i=0; i<3; i++)
				{
					lrc ^= write_buffer[i+1];
				}
				write_buffer[4] = lrc;
				ret = hal_portUsbSend(write_buffer, 5);
				if(ret < 0 || ret != read_len)
				{
					sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
				}
				RxFlashFlag = 0;
			}
			else if((rP[1]==0xe6) && (RxFlashFlag == 0))
			{
				sysDelayMs(200);
				sysLOG(PORT_LOG_LEVEL_4, " sendiLen=%d\r\n",iLen-2);	
				wifiUpgradeSendData_lib(rP+6, 0, 1);
				write_buffer[0] = 0x02;
				write_buffer[1] = 0xe6;
				write_buffer[2] = 0x00;
				write_buffer[3] = 0x02;
				write_buffer[4] = 0x01;
				write_buffer[5] = 0x00;
				for(i=0; i<5; i++)
				{
					lrc ^= write_buffer[i+1];
				}
				write_buffer[6] = lrc;
				ret = hal_portUsbSend(write_buffer, 7);
				if(ret < 0 || ret != read_len)
				{
					sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
				}
				RxFlashFlag = 1;
			}
			else if((rP[1]==0xe6) && (RxFlashFlag == 1))
			{
				sysDelayMs(200);
				sysLOG(PORT_LOG_LEVEL_4, " sendiLen=%d\r\n",iLen-2);	
				write_buffer[0] = 0x02;
				write_buffer[1] = 0xe6;
				write_buffer[2] = 0x00;
				write_buffer[3] = 0x02;
				write_buffer[4] = 0x02;
				write_buffer[5] = 0x64;
				for(i=0; i<5; i++)
				{
					lrc ^= write_buffer[i+1];
				}
				write_buffer[6] = lrc;
				ret = hal_portUsbSend(write_buffer, 7);
				if(ret < 0 || ret != read_len)
				{
					sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
				}
				RxFlashFlag = 2;
			}
			else if((rP[1]==0xe6) && (RxFlashFlag == 2))
			{
				sysDelayMs(200);
				sysLOG(PORT_LOG_LEVEL_4, " sendiLen=%d\r\n",iLen-2);	
				write_buffer[0] = 0x02;
				write_buffer[1] = 0xe6;
				write_buffer[2] = 0x00;
				write_buffer[3] = 0x02;
				write_buffer[4] = 0x03;
				write_buffer[5] = 0x64;
				for(i=0; i<5; i++)
				{
					lrc ^= write_buffer[i+1];
				}
				write_buffer[6] = lrc;
				ret = hal_portUsbSend(write_buffer, 7);
				if(ret < 0 || ret != read_len)
				{
					sysLOG(PORT_LOG_LEVEL_2, " send to se error,ret=%d\r\n",ret);		        
				}
				RxFlashFlag = 0;
			}

		}
		else{
			sysLOG(PORT_LOG_LEVEL_2, "ERROR rP[0]=%d\r\n",rP[0]);	
		}
	

	}

	exit:

	if(rP != NULL)
	{
		free(rP);
	}
	return ret;
    
}



/*
*Function:		hal_portUsbRecvCallback
*Description:	USB接收数据回调
*Input:			
*Output:		
*Hardware:
*Return:		NULL
*Others:
*/
void hal_portUsbRecvCallback(atDevice_t *th, void *buf, size_t size, void *arg)
{
	
	CBuffWrite(&g_stUsbBuffStr, buf, size);
	sysLOG(PORT_LOG_LEVEL_4, " usbdev rev len:%d\r\n",size);
}


/*
*Function:		hal_portUsbThread
*Description:	USB线程
*Input:			*param:入参指针
*Output:		
*Hardware:
*Return:		NULL
*Others:
*/
int g_usbHasBeenOpened = 0;//0:upper computer close comport 1:upper computer open comport;
//int g_open_port = 0;//usb device open by application

static void hal_portUsbThread(void *param)
{
	int iRet;
	osiThread_t *threadTmp = osiThreadCurrent();

	UsbHandle = FIBO_usbDevice_init(DRV_NAME_USRL_COM6, hal_portUsbRecvCallback);
	FIBO_usbDevice_State_report(UsbHandle, (Report_UsbDev_Event)hal_portUsbEvent);
	sysLOG_lib(PORT_LOG_LEVEL_1, "the usbdev init again wait usb insert!\r\n");
	g_usbHasBeenOpened = 0;


	iRet = CBuffInit(&g_stUsbBuffStr, USBBUFFLEN);
	if(iRet < 0)
	{
		sysLOG_lib(PORT_LOG_LEVEL_1, "<ERR> g_stUsbBuffStr init failed!\n");
		//return iRet;
	}
		
    while(1)
	{

		osiEvent_t event = {0};
        osiEventTryWait(threadTmp, &event, OSI_WAIT_FOREVER);
        switch(event.id)
        {
			case DRV_SERIAL_EVENT_BROKEN:

				FIBO_usbDevice_Deinit(UsbHandle);
				UsbHandle = FIBO_usbDevice_init(DRV_NAME_USRL_COM6, hal_portUsbRecvCallback);
				FIBO_usbDevice_State_report(UsbHandle, (Report_UsbDev_Event)hal_portUsbEvent);
				sysLOG_lib(PORT_LOG_LEVEL_1, "the usbdev init again wait usb insert!\r\n");
				g_usbHasBeenOpened = 0;
				fibo_sem_wait(usbdev_daemon_sem);  //wait usb insert							
			break;				
			
			case DRV_SERIAL_EVENT_READY://usb insert
				g_usbHasBeenOpened = 0;
				sysLOG_lib(PORT_LOG_LEVEL_1, "the usbdev had ready!\r\n");
			break;
			case DRV_SERIAL_EVENT_OPEN://upper computer open usb port
				g_usbHasBeenOpened = 1;
				sysLOG_lib(PORT_LOG_LEVEL_1, "the usbdev had opened!\r\n");
			break;
			case DRV_SERIAL_EVENT_CLOSE://upper computer close usb port
				g_usbHasBeenOpened = 0;
				sysLOG_lib(PORT_LOG_LEVEL_1, "the usbdev had closed!\r\n");
			break;
			
			default:
				sysLOG(PORT_LOG_LEVEL_1, "event.id=0x%x\r\n", event.id);
			break;

		}
    }

exit:

	sysLOG(PORT_LOG_LEVEL_1, "The Thread is exited\n");

	osiThreadExit();
}

/*
*Function:		hal_portUsbInit
*Description:	初始化USB
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_portUsbInit(void)
{
	int iRet = -1;

	//g_ui32MutexUsbHandle = fibo_mutex_create();
	//fibo_mutex_unlock(g_ui32MutexUsbHandle);

	usbdev_daemon_sem = fibo_sem_new(0);//creat a daemon semaphore
	
	g_i32UsbThreadHandle = osiThreadCreate("hal_portUsbThread", hal_portUsbThread, NULL, OSI_PRIORITY_NORMAL, 1024 * 8, 30);
	if (g_i32UsbThreadHandle == NULL)
	{
		sysLOG(PORT_LOG_LEVEL_1, "<ERR> osiThreadCreate USB Thread failed!\n");
		iRet = -2;
	   goto err;
	}
	iRet = 0;
	goto exit;

err:
	
	//fibo_mutex_delete(g_ui32MutexUsbHandle);

exit:
	
	return iRet;
}

/*
*Function:		hal_portUsbFlushBuff
*Description:	复位通信口，清除usb口缓冲区数据
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_portUsbFlushBuff(void)
{
	
	CBuffFormat(&g_stUsbBuffStr);
	return 0;
}


/*
*Function:		hal_portUsbOpen
*Description:	打开USB
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败,端口被占用
*Others:
*/
int hal_portUsbOpen(void)
{
	int iRet;

	
	/*iRet = fibo_mutex_try_lock(g_ui32MutexUsbHandle, 100);
	if(iRet == 0)
	{
		iRet = CBuffInit(&g_stUsbBuffStr, USBBUFFLEN);
		if(iRet < 0)
		{
			sysLOG(PORT_LOG_LEVEL_2, "<ERR> g_stUsbBuffStr init failed!\n");
			return iRet;
		}
		hal_portUsbFlushBuff();
		
		UsbHandle = FIBO_usbDevice_init(DRV_NAME_USRL_COM6, hal_portUsbRecvCallback);
		FIBO_usbDevice_State_report(UsbHandle, (Report_UsbDev_Event)hal_portUsbEvent);
		return 0;
	}
	else
	{
		return -1;
	}*/
	hal_portUsbFlushBuff();
	
	return 0;
}

/*
*Function:		hal_portUsbClose
*Description:	关闭USB
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_portUsbClose(void)
{
	int iRet;

	/*fibo_mutex_unlock(g_ui32MutexUsbHandle);
	FIBO_usbDevice_Deinit(UsbHandle);*/
	//DR_Close_Buff(&g_stUsbBuffStr);

	return 0;
}


/*
*Function:		hal_portUsbSend
*Description:	usb发送数据接口
*Input:			*data:发送数据指针; len:发送数据长度
*Output:		
*Hardware:
*Return:		>0-发送的字节数;0-失败
*Others:
*/
int hal_portUsbSend(char *data, int len)
{
	if(g_usbHasBeenOpened == 0)
	{
		return 0;
	}
		
	return FIBO_usbDevice_send(UsbHandle, data, len);
}


/*
*Function:		hal_portUsbRecv
*Description:	usb接收数据接口
*Input:			len:期望接收长度;timeout:超时时间,单位ms
*Output:		*data:读取数据存储指针
*Hardware:
*Return:		<0:失败；>=0:成功读取的字节数
*Others:
*/
int hal_portUsbRecv(char *data, int len, uint32 timeout)
{
	int iRet = -1;
	unsigned long long uTime = 0;
	int recvdlen = 0;
	
	uTime = hal_sysGetTickms() + timeout;
	
	while(1)
	{
		iRet = CBuffRead(&g_stUsbBuffStr, data+recvdlen, len-recvdlen);
		if(iRet < 0)
		{
			return iRet;
		}
		if(iRet == 0)
		{
			sysDelayMs(10);
		}
				
		recvdlen += iRet;
		if(recvdlen >= len)
		{
			return recvdlen;
		}
		if(hal_sysGetTickms() > uTime)
		{
			return recvdlen;
		}
	}
	
	
}


/*
*Function:		hal_portOpen
*Description:	打开指定的通信口
*Input:			emPort:通道号，10-usb
*				*Attr:通讯速率和格式串,如果是USB转串，不需要进行该操作。传NULL即可
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_portOpen(SER_PORTNUM_t emPort, char *Attr)
{
	int iRet;
	if(emPort == P_RS232)
	{
		return portOpenEx(emPort, Attr);
	}
	else if(emPort == P_USB)
	{
		if(ui8USBStatus == 1)
		{
			return UART_OK;
		}
		iRet = hal_portUsbOpen();
		if(iRet < 0)
			return iRet;
		ui8USBStatus = 1;
		return iRet;
	}
	else
	{
		return UATR_ERR_INVALID;
	}
	
	
}


/*
*Function:		hal_portClose
*Description:	关闭指定的通信口
*Input:			emPort:通道号，10-usb
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_portClose(SER_PORTNUM_t emPort)
{
	int iRet;
	
	if(emPort == P_RS232)
	{
		return portCloseEx(emPort);
	}
	else if(emPort == P_USB)
	{
		if(ui8USBStatus == 0)
		{
			return UART_OK;
		}
		iRet = hal_portUsbClose();
		if(iRet < 0)
			return iRet;
		ui8USBStatus = 0;
		return iRet;
	}
	else
	{
		return UATR_ERR_INVALID;
	}
	
}


/*
*Function:		hal_portFlushBuf
*Description:	复位通讯口,该函数将清除串口接收缓冲区中的所有数据
*Input:			emPort:通道号，10-usb
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_portFlushBuf(SER_PORTNUM_t emPort)
{
	if(emPort == P_RS232)
	{
		return portFlushBufEx(emPort);
	}
	else if(emPort == P_USB)
	{
		if(ui8USBStatus == 0)
		{
			return UART_ERR_NOTOPEN;
		}
		return hal_portUsbFlushBuff();
	}
	else
	{
		return UATR_ERR_INVALID;
	}
	
}


/*
*Function:		hal_portSends
*Description:	使用指定的通讯口发送若干字节的数据
*Input:			emPort:通道号，10-usb;*str:发送数据指针;str_len:发送数据长度
*Output:		NULL
*Hardware:
*Return:		>=0-成功发送的字节数;<0-失败
*Others:
*/
int hal_portSends(SER_PORTNUM_t emPort, uchar *str, ushort str_len)
{
	int iRet = -1;

	if(str_len == 0)
	{
		return UART_OK;
	}
	
	if(emPort == P_RS232)
	{
		return portSendsEx(emPort, str, str_len);
	}
	if(emPort == P_USB)
	{
		if(ui8USBStatus == 0)
		{
			return UART_ERR_NOTOPEN;
		}
		iRet = hal_portUsbSend(str, str_len);
		if(iRet <= 0)
		{
			return UART_ERR_BUF;
		}
		return iRet;
	}
	else
	{
		return UATR_ERR_INVALID;
	}
	
	
}


/*
*Function:		hal_portRecvs
*Description:	在给定的时限内，最多接收期望长度的数据
*Input:			emPort:通道号，10-usb
*				usBufLen:期望接收的字节数
*				usTimeoutMs:接收超时时间，单位ms,
*Output:		*pszBuf:接收缓冲区指针
*Hardware:
*Return:		>=0-成功发送的字节数;<0-失败
*Others:
*/
int hal_portRecvs(SER_PORTNUM_t emPort, uchar *pszBuf, ushort usBufLen, ushort usTimeoutMs)
{
	int iRet = -1;

	if(usBufLen == 0)
	{
		return UART_OK;
	}
	
	if(emPort == P_RS232)
	{
		return portRecvsEx(emPort, pszBuf, usBufLen, usTimeoutMs);
	}
	else if(emPort == P_USB)
	{
		if(ui8USBStatus == 0)
		{
			return UART_ERR_NOTOPEN;
		}
		iRet = hal_portUsbRecv((char *)pszBuf, usBufLen, usTimeoutMs);
		if(iRet < 0)
		{
			return UART_ERR_TIMEOUT;
		}
		return iRet;
	}
	else
	{
		return UATR_ERR_INVALID;
	}
	
	
}


/*********************************TEST*********************************/

#if MAINTEST_FLAG


void hal_portUSBTest(void)
{
	int iRet;
	uint8 buftmp[1024];
	sysLOG(PORT_LOG_LEVEL_1, "start usb test\n");
	iRet = hal_portClose(P_USB);
	sysLOG(PORT_LOG_LEVEL_1, "hal_portClose,iRet=%d\n", iRet);
	sysDelayMs(1000);
	iRet = hal_portOpen(P_USB, NULL);
	sysLOG(PORT_LOG_LEVEL_1, "hal_portOpen,iRet=%d\n", iRet);
	if(iRet < 0)
		return;

	while(1)
	{
		memset(buftmp, 0, sizeof(buftmp));
		iRet = hal_portRecvs(P_USB, buftmp, sizeof(buftmp), 1000);
		sysLOG(PORT_LOG_LEVEL_1, "hal_portRecvs,iRet=%d\n", iRet);
		if(iRet > 0)
		{
			iRet = hal_portSends(P_USB, buftmp, iRet);
			sysLOG(PORT_LOG_LEVEL_1, "hal_portSends,iRet=%d\n", iRet);
			if(iRet < 0)
				return;
		}
		else if(iRet < 0)
			return;
		
		sysDelayMs(10);
	}

}


#endif





