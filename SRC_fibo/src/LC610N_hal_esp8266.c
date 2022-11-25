#include "comm.h"


char g_cWifiVersionFlag = 0;       //0:no SDK；1：RTOS SDK
char g_cWifiWebNetworkState = 4; 
char g_cWifiAirkissNetState = 4; 
char g_cWaitWebUpdateState = 0;

uint64_t g_ui64AirkissNetworkTime = 0;
uint64_t g_ui64WebNetworkTime = 0;
uint8_t g_ui8Lrc = 0;

int hal_wifiSend(char *pcBuff, int iLen)
{
	if((NULL == pcBuff) || (0 == iLen))
	{
		return -1;
	}
	
	Wifi_wkup(TRUE);

	//dev_wifiClearRecv();
	Wifi_send(pcBuff,iLen);
	Wifi_wkup(FALSE);
	return 0;
}

int hal_wifiWaitString(char *pcWaitStr, char *pcBuff, int iBufSize)
{
	int i32Ret1 = -1;
	int i32Ret2 = -1;
		
	i32Ret1 = dev_wifiWaitString(pcWaitStr);

	i32Ret2 = dev_wifiReadString(pcBuff, iBufSize, i32Ret1);
	if(i32Ret2 < 0)
	{
		return -6339;
	}

	return i32Ret1;
}

/*
* Description: 发送指令，等待制定回复，带超时
* Input:        atcmd:AT指令，rcvbuf:接收缓存，cmd:关键字，cmdlen:关键字长度，timeout:单次超时时间 
*               trytime:重试次数
* Output:N
* Return:      -1:超时退出，>0:包含关键字的数据包长度。WIFI_SEND_CONFLICT_FAIL-sendCmd忙中，-3-回复的数据有错，WIFI_SERVER_BUSY-WiFi忙，稍后再发送
*/


int ESP_SendAndWaitCmd(char *atcmd,int atcmdLen, char *rcvbuf, int rcvbuflen, int i32Timeout, int trytime, char *acCmd)
{
    int i = 0;
	int32 i32Ret = 0;
	unsigned int ui32Time = 0;
	int i32Ret1 = -1;
	
	fibo_mutex_try_lock(g_ui32ApiWiFiSendMutex, 5000);
	
	if(g_stWifiUart.wifibusy_flag==0) 
	{
		g_stWifiUart.wifibusy_flag=1;
	}
	else 
	{
		ui32Time = hal_sysGetTickms() + i32Timeout;
		while(1)
		{
			sysDelayMs(10);
			if(g_stWifiUart.wifibusy_flag==0)
			{
				g_stWifiUart.wifibusy_flag=1;
				
				break;
			}
			if(hal_sysGetTickms() >= ui32Time)
			{
				sysLOG(HALESP8266_LOG_LEVEL_0, "wifibusy_flag:%d\r\n", g_stWifiUart.wifibusy_flag);
				i32Ret1 = WIFI_SEND_CONFLICT_FAIL;
				goto exit;
			}
		}
		
	}
	//Wifi_wkup(TRUE);
	//sysDelayMs(10);
    for (; i < trytime; i++)
    {
		g_stWifiUart.CMD_read_P=g_stWifiUart.CMD_write_P=0;
		memset(g_stWifiUart.CMD_uart_buff,0,sizeof(g_stWifiUart.CMD_uart_buff));
		g_ui8CldsdClosemodeFlag=0;
		//g_ui8WifiConnectClosed=0;
		
		if(g_stWifiUart.wifibusy_flag == 1)Wifi_send(atcmd,atcmdLen);
		//sysLOG(1,"send&wait acCmd once is ok---------------");
		
		/*等待接收*/
		ui32Time = hal_sysGetTickms() + i32Timeout;
		//sysLOG(1,"ui32Time%d\r\n",ui32Time);
		while(1)
		{
			//sysLOG(1,"ESP_SendAndWaitCmd!!!!!!!!!");
			//
			i32Ret = dev_wifiWaitString(acCmd);
			if(i32Ret >= 0)
			{
				i32Ret += strlen(acCmd);
				i32Ret = dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret);
				
				sysLOG(HALESP8266_LOG_LEVEL_4, "acCmd send ok,trytime:%d, i32Ret :%d,recv is %s\r\n", trytime, i32Ret, rcvbuf);
				//Wifi_wkup(FALSE);
				i32Ret1 = i32Ret;
				
				goto exit;
			}
			
			if(hal_sysGetTickms() >= ui32Time)
			{
				if(dev_wifiWaitString("busy p...") >= 0)
				{
					g_stWifiUart.wifibusy_flag = 2;//如果是busy，则重试的时候不发送只接收即可，
				}
				//发生超时以后，看下缓存buf里面有什么内容
				else
				{
					/*i32Ret = hal_espSetBaud();
					if(i32Ret == 0)
					{
						sysLOG(HALESP8266_LOG_LEVEL_1, "hal_espSetBaud, i32Ret=%d\r\n", i32Ret);
					}*/
				}				
				sysLOG(HALESP8266_LOG_LEVEL_0, "acCmd hal_sysGetTickms i32Timeout  = %d!\r\n",i32Timeout);
				
				break;
			}	
			sysDelayMs(2);
		}		
    }
	if(dev_wifiWaitString("busy p...") >= 0)
	{
		
		//Wifi_wkup(FALSE);
		i32Ret1 = WIFI_SERVER_BUSY;
		sysLOG(API_LOG_LEVEL_0, "  transceiveFrame iRet= %d\r\n", i32Ret1);
		goto exit;
	}
	if(dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret) < 0)
	{
		
		//Wifi_wkup(FALSE);
		i32Ret1 = -3;
		sysLOG(API_LOG_LEVEL_0, "  transceiveFrame iRet= %d\r\n", i32Ret1);
		goto exit;
	}
	sysLOG(API_LOG_LEVEL_0, "  transceiveFrame iRet= %d\r\n", i32Ret);
	//Wifi_wkup(FALSE);
    i32Ret1 = -1;
	goto exit;

exit:
	
	g_stWifiUart.wifibusy_flag=0;
	fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
	return i32Ret1;
}

int ESP_SendAndWaitCmdwithErr(char *atcmd,int atcmdLen, char *rcvbuf, int rcvbuflen, int i32Timeout, int trytime, char *acCmd,char *cmd2)
{
    int i = 0;
	int32 i32Ret = 0;
	unsigned int ui32Time = 0;
	int i32Ret1 = -1;
	
	fibo_mutex_try_lock(g_ui32ApiWiFiSendMutex, 5000);

	
	if(g_stWifiUart.wifibusy_flag==0) 
	{
		g_stWifiUart.wifibusy_flag=1;
	}
	else 
	{
		ui32Time = hal_sysGetTickms() + i32Timeout;
		while(1)
		{
			sysDelayMs(10);
			if(g_stWifiUart.wifibusy_flag==0)
			{
				g_stWifiUart.wifibusy_flag=1;
				break;
			}
			if(hal_sysGetTickms() >= ui32Time)
			{
				sysLOG(HALESP8266_LOG_LEVEL_2, "wifibusy_flag:%d\r\n", g_stWifiUart.wifibusy_flag);
				i32Ret1 = WIFI_SEND_CONFLICT_FAIL;
				goto exit;
			}
		}
		
	}
	//Wifi_wkup(TRUE);
	//sysDelayMs(10);
    for (; i < trytime; i++)
    {
		g_stWifiUart.CMD_read_P=g_stWifiUart.CMD_write_P=0;
		memset(g_stWifiUart.CMD_uart_buff,0,sizeof(g_stWifiUart.CMD_uart_buff));
		g_ui8CldsdClosemodeFlag=0;
		//g_ui8WifiConnectClosed=0;
		
		if(g_stWifiUart.wifibusy_flag == 1)Wifi_send(atcmd,atcmdLen);
		
		/*等待接收*/
		ui32Time = hal_sysGetTickms() + i32Timeout;
		while(1)
		{
			sysDelayMs(2);
			i32Ret = dev_wifiWaitString(acCmd);
			if(i32Ret >= 0)
			{
				i32Ret += strlen(acCmd);
				i32Ret = dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_4, "acCmd send ok,trytime:%d, i32Ret :%d,recv is %s\r\n", trytime, i32Ret, rcvbuf);
				i32Ret1 = i32Ret;
				goto exit;
			}
			if(cmd2 != NULL)
			{
				i32Ret = dev_wifiWaitString(cmd2);
				if(i32Ret >= 0)
				{
					i32Ret += strlen(cmd2);
					i32Ret = dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret);
					i32Ret1 = WIFI_TCPCLOSE_FAIL;
					sysLOG(HALESP8266_LOG_LEVEL_2, "acCmd send ok <ERROR>, WIFI_TCPCLOSE_FAIL trytime:%d, i32Ret :%d,recv is %s\r\n", trytime, i32Ret, rcvbuf);
					
					goto exit;
				}

			}
			
			if(hal_sysGetTickms() >= ui32Time)
			{
				if(dev_wifiWaitString("busy p...") >= 0)
				{
					g_stWifiUart.wifibusy_flag = 2;//如果是busy，则重试的时候不发送只接收即可，
				}
				sysLOG(HALESP8266_LOG_LEVEL_3, "acCmd hal_sysGetTickms i32Timeout  = %d!\r\n",i32Timeout);
				break;
			}			
		}		
    }
	if(dev_wifiWaitString("busy p...") >= 0)
	{
		
		//Wifi_wkup(FALSE);
		i32Ret1 = WIFI_SERVER_BUSY;
		goto exit;
	}
	if(dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret) < 0)
	{
		
		//Wifi_wkup(FALSE);
		i32Ret1 = -3;
		goto exit;
	}
	
	//Wifi_wkup(FALSE);
    i32Ret1 = -1;
	goto exit;

exit:
	
	g_stWifiUart.wifibusy_flag=0;
	fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
	return i32Ret1;
}

#if 0
int ESP_SendAndWaitCmdNoLock(char *atcmd,int atcmdLen, char *rcvbuf, int rcvbuflen, int i32Timeout, int trytime, char *acCmd)
{
    int i = 0;
	int32 i32Ret = 0;
	unsigned int ui32Time = 0;
	int i32Ret1 = -1;	
		
    for (; i < trytime; i++)
    {
		g_stWifiUart.CMD_read_P=g_stWifiUart.CMD_write_P=0;
		memset(g_stWifiUart.CMD_uart_buff,0,sizeof(g_stWifiUart.CMD_uart_buff));

		g_ui8CldsdClosemodeFlag=0;
		
		Wifi_send(atcmd,atcmdLen);
		/*等待接收*/
		ui32Time = hal_sysGetTickms() + i32Timeout;
		//sysLOG(1,"ui32Time%d\r\n",ui32Time);
		while(1)
		{
			//sysLOG(1,"ESP_SendAndWaitCmd!!!!!!!!!");
			sysDelayMs(2);
			i32Ret = dev_wifiWaitString(acCmd);
			if(i32Ret >= 0)
			{
				i32Ret += strlen(acCmd);
				i32Ret = dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret);
				
				sysLOG(HALESP8266_LOG_LEVEL_1, "acCmd send ok,trytime:%d, i32Ret :%d,recv is %s\r\n", trytime, i32Ret, rcvbuf);
				i32Ret1 = i32Ret;
				goto exit;
			}
			
			if(hal_sysGetTickms() >= ui32Time)
			{
				if(dev_wifiWaitString("busy p...") >= 0)
				{
					//g_stWifiUart.wifibusy_flag = 2;//如果是busy，则重试的时候不发送只接收即可，
				}
				sysLOG(HALESP8266_LOG_LEVEL_1, "acCmd hal_sysGetTickms i32Timeout  = %d!!!!!!!!\r\n",ui32Time);
				break;
			}			
		}		
    }
	if(dev_wifiWaitString("busy p...") >= 0)
	{
		//Wifi_wkup(FALSE);
		i32Ret1 = WIFI_SERVER_BUSY;
		goto exit;
	}
	if(dev_wifiReadString(rcvbuf, rcvbuflen, i32Ret) < 0)
	{
		
		//Wifi_wkup(FALSE);
		i32Ret1 = -3;
		goto exit;
	}
	//Wifi_wkup(FALSE);
    i32Ret1 = -1;
	goto exit;

exit:	
	return i32Ret1;
}
#endif

#if WIFI_ATTYPE

/*
* Description: 发送数据接口
* Input:        data:待发送数据内容，datalen:待发送数据长度,单次最长数据包暂定2048
*               i32Timeout:单次超时时间 
*               trytime:重试次数
* Output:N
* Return:      -1:发送失败，>=0:发送成功。
*/
int hal_espSendData(int i32Sockid, char *data, int datalen, int i32Timeout, int trytime)
{
    //int ret = 0;
    int i = 0;
	int j = 0;
    char acBuf[100];
    char acCmd[20]; //固定字符11个字节，结尾需要添加\r\n作为结束符
	unsigned int ui32Time = 0;
	
	
    if((datalen == 0) || (datalen > 2048))
    {
        return WIFI_PRAM_ERR;									/*参数错误*/
    }

	memset(acCmd, 0x00, sizeof(acCmd));
	
	sprintf(&acCmd[i],"AT+CIPSEND=%d,%d",i32Sockid, datalen);
	i=strlen(acCmd);

	acCmd[i++] = '\r';
	acCmd[i++] = '\n';
	
	Wifi_wkup(TRUE);

    for(; j < trytime; j++)
    {
		memset(acBuf, 0x00, sizeof(acBuf));

		if(ESP_SendAndWaitCmd(acCmd, i, acBuf, 40, i32Timeout, 1,  ">") < 0)
		{
			if((dev_wifiWaitString("CLOSED") >= 0) || (dev_wifiWaitString("link is not valid") >= 0) || (dev_wifiWaitString("ERROR") >= 0) )
			{
				Wifi_wkup(FALSE);
				sysLOG(HALESP8266_LOG_LEVEL_2, "<ERR> WIFI_AP_LINK_ERR\r\n");
				return WIFI_AP_LINK_ERR;		/*网络断开连接 --- 热点有问题*/
			}
			//Wifi_wkup(FALSE);
			//return WIFI_SENDDATA_FAIL_1;     /*发送数据，没有收到'>'*/
		}
		memset(acBuf, 0x00, sizeof(acBuf));
		if(ESP_SendAndWaitCmd(data, datalen, acBuf, 100, i32Timeout*4, 1,  "SEND OK\r\n") < 0)
		{
			if(Search_StringBInStringA(acBuf, "Recv") >= 0)
			{
				ui32Time = hal_sysGetTickms() + 10000;
				while(1)
				{
					if(dev_wifiWaitString( "SEND OK") >= 0)
					{
						Wifi_wkup(FALSE);
						sysLOG(HALESP8266_LOG_LEVEL_1, "<SUCC> SEND OK\r\n");
						return 0;
					}

					if(dev_wifiWaitString( "SEND FAIL") >= 0)
					{
						Wifi_wkup(FALSE);
						sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> SEND FAIL\r\n");
						return WIFI_SENDDATA_FAIL_2;
					}
					
					if(dev_wifiWaitString( "busy s...") >= 0)
					{
						Wifi_wkup(FALSE);
						sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> busy s...\r\n");
						return WIFI_SERVER_BUSY;
					}	

					if(dev_wifiWaitString( "CLOSED") >= 0)
					{
						Wifi_wkup(FALSE);
						sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> CLOSED\r\n");
						return WIFI_AP_LINK_ERR;
					}

					if(dev_wifiWaitString( "ERROR") >= 0)
					{
						Wifi_wkup(FALSE);
						sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> ERROR\r\n");
						return WIFI_AP_LINK_ERR;
					}
					
					if(hal_sysGetTickms() >= ui32Time)
		    		{
		    			Wifi_wkup(FALSE);
						sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> i32Timeout\r\n");
						return WIFI_SENDDATA_FAIL_2;
		    		}
					//sysDelayMs(10);
				}				
			}
			Wifi_wkup(FALSE);
			sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> WIFI_SENDDATA_FAIL_2\r\n");
			return WIFI_SENDDATA_FAIL_2;
		}
		Wifi_wkup(FALSE);
		sysLOG(HALESP8266_LOG_LEVEL_1, "<SUCC>\r\n");
		return 0;
    }
	Wifi_wkup(FALSE);
	sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> WIFI_SENDDATA_FAIL_2\r\n");
    return WIFI_SENDDATA_FAIL_2;
}
#endif

//复位延时，等待约1秒
void rst_delay(void)
{
    int n = 1000;
	int i = 0;

    while (n-- != 0)
    {
        for (i = 0; i < 24000; i++) ;
    }
}

/*
* Description: 芯片复位
* Input:        
* Output:N
* Return:      -1:发送失败，>=0:发送成功。
*/
int hal_espReset(void)
{
    char acBuf[2048];
	int i32Ret = 0;
	int i32Limit = 0;
	Wifi_wkup(TRUE);

	while(1)
	{
		memset(acBuf, 0x00, sizeof(acBuf));
		
		i32Ret = ESP_SendAndWaitCmd("AT+RST\r\n", strlen("AT+RST\r\n"), acBuf, sizeof(acBuf), 5000, 1, "ready");
		if(i32Ret >= 0)
		{
			break;
		}
		
		i32Limit++;
		if(i32Limit >= 3)
		{
			Wifi_wkup(FALSE);
			return -2;
		}
	}
	Wifi_wkup(FALSE);
	return 0;
}

void hal_espGpioInit(void)
{
	//ESP_GPIO_Init();
}

/*
* Description: ESP8266开机固定指令发送:在线检测，关闭回显，Station模式
* Input: N
* Output:N
* Return: <0:初始化失败
*/
int ESP_Cmd_Init(void)
{
    unsigned char acBuf[100];
//	unsigned char aucMac[64];
	
    int iRet = -1;
	int i32Limit = 0;
	
	Wifi_wkup(TRUE);


	while(1)
	{
	/*测试AT启动*/	
		sysLOG(HALESP8266_LOG_LEVEL_4, "ESP_Cmd_Init is opened!!\r\n");
		memset(acBuf, 0, sizeof(acBuf));

	    iRet = ESP_SendAndWaitCmd("AT\r\n", strlen("AT\r\n"), (char *)acBuf, 100, 500, 1, "\r\nOK\r\n");
		if(iRet < 0)
		{
			i32Limit++;
			if(i32Limit >= 3)
			{
				Wifi_wkup(FALSE);
				return iRet;
			}
		}

		break;
	}

	wifiChangeBrand();
	
	Wifi_wkup(FALSE);
    return 0;
}

#if WIFI_ATTYPE

/*
* Description: 查询当前网络连接状态
* Input: N
* Output:N
* Return: <0:通信错误，无响应。 
*         2:已连接AP，获得IP地址。 3:已建立TCP或UDP传输 
*         4:断开网络连接          5:未连接AP 
*/
int hal_espWifiStatus(void)
{
    char acBuf[100];
    int ret = -1;
	int i = 3;
	int statmp = 0;

	Wifi_wkup(TRUE);

	while(i--)//AT+CIPSTATUS只可以查到2或者5状态
	{
		memset(acBuf, 0, sizeof(acBuf));
	
		if(ESP_SendAndWaitCmd("AT+CIPSTATUS\r\n", 14, acBuf, 100, 500, 1, "OK\r\n") >= 0)
		{
			ret = dev_wifiWaitString("STATUS:");
			if(ret >= 0)
			{
				memset(acBuf, 0, sizeof(acBuf));

				dev_wifiReadString(acBuf, 100, ret);
				sysLOG(HALESP8266_LOG_LEVEL_4, "DWR STATUS is %s\r\n", acBuf);
				ret = Search_StringBInStringA(acBuf,"STATUS:");
				if(ret >= 0)
				{
					sysLOG(HALESP8266_LOG_LEVEL_1, "get STATUS ret is %d\r\n", acBuf[ret + 7] - '0');
					
					statmp = (acBuf[ret + 7] - '0');
					if(((acBuf[ret + 7]) == '0') || ((acBuf[ret + 7]) == '1'))
				    {
						statmp = 2;
					}
					if(statmp == 2)
					{
						break;
					}
				}				
			}
		}
	}
	if(statmp == 2)//查到是2再去调用AT+VANSTATUS去查询，
	{
		i = 3;
		while(i--)
	    {
			memset(acBuf, 0, sizeof(acBuf));
		
			if(ESP_SendAndWaitCmd("AT+CIPSTATUS\r\n", 14, acBuf, 100, 500, 1, "OK\r\n") >= 0)
			{
				ret = dev_wifiWaitString("STATUS:");
				if(ret >= 0)
				{
					memset(acBuf, 0, sizeof(acBuf));

					dev_wifiReadString(acBuf, 100, ret);
					sysLOG(HALESP8266_LOG_LEVEL_4, "DWR STATUS is %s\r\n", acBuf);
					ret = Search_StringBInStringA(acBuf,"STATUS:");
					if(ret >= 0)
					{
						sysLOG(HALESP8266_LOG_LEVEL_1, "get STATUS ret is %d\r\n", acBuf[ret + 7] - '0');
						Wifi_wkup(FALSE);
						if(((acBuf[ret + 7]) == '0') || ((acBuf[ret + 7]) == '1'))
						{
							return 5;
						}
						return (acBuf[ret + 7] - '0');
					}				
				}
			}
	    }
	}
	else
	{
		Wifi_wkup(FALSE);
		return statmp;
	}
	Wifi_wkup(FALSE);
    return WIFI_GET_STATUS_FAIL;
}
#endif

/*
* Description: 扫描WIFI热点
* Input: N
* Output:N
* Return: <0:初始化失败
*/
int hal_espAPScan(char *pcBuff, unsigned int uiSize)
{
    int ret = -1;
	unsigned int ui32Time=0;
	Wifi_wkup(TRUE);

	unsigned char acBuf[100];

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd("AT+CWLAPOPT=1,63\r\n", strlen("AT+CWLAPOPT=1,63\r\n"), (char *)acBuf, 100, 500, 3, "\r\nOK\r\n");
	if (ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}
	//memset(acBuf, 0x00, sizeof(acBuf));
    ret = ESP_SendAndWaitCmd("AT+CWLAP\r\n", strlen("AT+CWLAP\r\n"), pcBuff, uiSize, 5000, 1, "\r\nOK\r\n");
    if(ret < 0)
    {
    	sysLOG(HALESP8266_LOG_LEVEL_2, "hal_espAPScan ret is %d\r\n", ret);
		Wifi_wkup(FALSE);
        return ret;
    }
	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 连接wifi热点
* Input:       ssid:热点名称(最大长度32字符)，password:联机密码(最大长度64字节)，均以\0结尾
* Output:N
* Return:      0:连接成功，-1:连接失败
*/
int hal_espAPConnect(char *ssid, char *password, ST_AP_MAC *pstApMac)    //AT+CWJAP_DEF="JW-2.4G","jiewen-2017"
{
    char at_cmd[128];
    char acBuf[100];
    //char *ptr;
    int i = 0;
    int i32Ret = 0;
	int j = 0,m = 0;
	unsigned int ui32Time = 0;
	char cSsidLen = 0;
	int i32Ret1 = -1;
	Wifi_wkup(TRUE);
	fibo_mutex_try_lock(g_ui32ApiWiFiSendMutex, 5000);
    memset(at_cmd, 0, sizeof(at_cmd));
	memcpy(at_cmd, "AT+CWJAP=\"", 10);
	i = 10;
    while (*ssid != '\0')
    {
        at_cmd[i++] = *ssid++;
        cSsidLen++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';
    at_cmd[i++] = '\"';

    while (*password != '\0')
    {
        at_cmd[i++] = *password++;
    }
    at_cmd[i++] = '\"';

/*加入MAC*/

	for(j = 0; ; j++)
	{		
		if(j < pstApMac->iMacCnt)
	    {
			at_cmd[i] = ',';
			at_cmd[i+1] = '\"'; 
			memcpy(at_cmd+i+2, pstApMac->cBssid[j], 17);
	    	at_cmd[i+19] = '\"';
	    	at_cmd[i+20] = '\r';
	    	at_cmd[i+21] = '\n';

			m = (i + 22);
		}
		else
		{
			if((pstApMac->iMacCnt == 0) && (j == pstApMac->iMacCnt))
			{
				at_cmd[i] = '\r';
	    		at_cmd[i+1] = '\n';
			
				m = (i + 2);
			}
			else
			{
				break;
			}
		}
		
		fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
		hal_espAPClose();

		if(g_stWifiUart.wifibusy_flag==0)g_stWifiUart.wifibusy_flag=1;
		else 
		{
			ui32Time = hal_sysGetTickms() + 5000;
			while(1)
			{
				sysDelayMs(10);
				if(g_stWifiUart.wifibusy_flag==0)
				{
					g_stWifiUart.wifibusy_flag=1;
					break;
				}
				if(hal_sysGetTickms() >= ui32Time)
				{
					sysLOG(HALESP8266_LOG_LEVEL_1, "wifibusy_flag:%d\r\n", g_stWifiUart.wifibusy_flag);
					Wifi_wkup(FALSE);
					i32Ret1 = WIFI_SEND_CONFLICT_FAIL;
					goto exit;
				}
			}
			
		}
		DR_IPDmutex_lock(g_ui32IPDMutexHandle);
		wifiRevBuffInit();
		DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
		g_ui8CldsdClosemodeFlag=0;
		g_ui8WifiConnectClosed=0;
		
		if(g_stWifiUart.wifibusy_flag == 1)Wifi_send(at_cmd, m);

		ui32Time = hal_sysGetTickms() + 30*1000;
		while(1)
		{
			i32Ret = dev_wifiWaitString("\r\nOK\r\n");
	        if(i32Ret >= 0)
	        {
	            Wifi_wkup(FALSE);
	        	i32Ret1 = 0;
				goto exit;
	        }
			
	        i32Ret = dev_wifiWaitString("WIFI GOT IP");
	        if(i32Ret >= 0)
	        {
	            Wifi_wkup(FALSE);
	        	i32Ret1 = 0;
				//goto exit;
	        }
	        
			i32Ret = dev_wifiWaitString("+CWJAP:");
			if(i32Ret >= 0)
			{
				dev_wifiReadString(acBuf, sizeof(acBuf), i32Ret);
				
				switch(acBuf[i32Ret+7])
				{						
					case '1':		/*连接超时*/
						Wifi_wkup(FALSE);
						i32Ret1 = -1;
						goto exit;
					case '2':		/*密码错误*/
						Wifi_wkup(FALSE);
						i32Ret1 = -2;
						goto exit;
					case '3':		/*找不到目标AP*/
						Wifi_wkup(FALSE);
						i32Ret1 = -3;
						goto exit;
					case '4':		/*连接失败*/
					default:
						Wifi_wkup(FALSE);
						i32Ret1 = -4;
						goto exit;
				}
			}
			i32Ret = dev_wifiWaitString("FAIL");
	        if(i32Ret >= 0)
	        {
	            Wifi_wkup(FALSE);
	        	i32Ret1 = -5;
				goto exit;
	        }

			if(dev_wifiWaitString("busy p...") >= 0)
			{
				
				Wifi_wkup(FALSE);
				i32Ret1 = WIFI_SERVER_BUSY;
				goto exit;
			}

			if(hal_sysGetTickms() >= ui32Time)
			{
				if(g_stWifiUart.CMD_write_P == 0)
					g_stWifiUart.wifibusy_flag = 2;
				else
				{
					/*i32Ret = hal_espSetBaud();
					if(i32Ret == 0)
					{
						sysLOG(HALESP8266_LOG_LEVEL_1, "hal_espSetBaud, ret=%d\r\n", i32Ret);
					}*/
				}
				break;
			}
			//sysDelayMs(50);
		}
	}		
	

	Wifi_wkup(FALSE);
    i32Ret1 = -4;
	goto exit;

exit:

	g_stWifiUart.wifibusy_flag=0;
	fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
	return i32Ret1;

}

/*
* Description: 断开热点连接
* Input:       
* Output:N
* Return:      0:断开成功，-1:断开失败
*/
int hal_espAPClose(void)
{
    char acBuf[100];
	int i32Ret = 0;
	
	memset(acBuf, 0x00, sizeof(acBuf));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+CWQAP\r\n", strlen("AT+CWQAP\r\n"), acBuf, sizeof(acBuf), 5000, 1, "\r\nWIFI DISCONNECT\r\n");
	//i32Ret = ESP_SendAndWaitCmdNoLock("AT+CWQAP\r\n", strlen("AT+CWQAP\r\n"), acBuf, sizeof(acBuf), 5000, 1, "\r\nWIFI DISCONNECT\r\n");
	if(i32Ret < 0)
	{
		i32Ret = Search_StringBInStringA(acBuf, "\r\nOK\r\n");
		if(i32Ret >= 0)
		{
			Wifi_wkup(FALSE);
			return 0;
		}
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	Wifi_wkup(FALSE);
    return 0;
}


/*
* Description: 查询已配置的AP参数
* Input:       
* Output:N
* Return:      0:成功，<0:失败
*/
int hal_espAPCheck(APINFO *apinfo)
{
    char acBuf[256];
	int i32Ret = 0;
	char *rP = NULL;
	char *rP1 = NULL;
	int readedlen = 0;
	char buftmp[32];
	memset(buftmp, 0, sizeof(buftmp));
	memset(acBuf, 0x00, sizeof(acBuf));
	Wifi_wkup(TRUE);

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CWSAP_DEF?\r\n", strlen("AT+CWSAP_DEF?\r\n"), acBuf, sizeof(acBuf), 5000, 1, "OK");
		if(i32Ret < 0)
		{
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		rP = myStrStr(acBuf, "+CWSAP_DEF:", 0, sizeof(acBuf));
		readedlen = strlen("+CWSAP_DEF:");
	}
	else{
		i32Ret = ESP_SendAndWaitCmd("AT+CWSAP?\r\n", strlen("AT+CWSAP?\r\n"), acBuf, sizeof(acBuf), 5000, 1, "OK");
		if(i32Ret < 0)
		{
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		rP = myStrStr(acBuf, "+CWSAP:", 0, sizeof(acBuf));
		readedlen = strlen("+CWSAP:");
	}
	
	if(rP != NULL)
	{
		rP1 = myStrStr(rP+readedlen, ",", 0, sizeof(acBuf)-readedlen);
		if(rP1 == NULL)
		{
			i32Ret = -1;
			goto exit;
		}
		memcpy(apinfo->ssid, rP+readedlen, rP1-rP-readedlen);
		readedlen = rP1-rP+1;

		rP1 = myStrStr(rP+readedlen, ",", 0, sizeof(acBuf)-readedlen);
		if(rP1 == NULL)
		{
			i32Ret = -2;
			goto exit;
		}
		memcpy(apinfo->pwd, rP+readedlen, rP1-rP-readedlen);
		readedlen = rP1-rP+1;

		rP1 = myStrStr(rP+readedlen, ",", 0, sizeof(acBuf)-readedlen);
		if(rP1 == NULL)
		{
			i32Ret = -3;
			goto exit;
		}
		memcpy(buftmp, rP+readedlen, rP1-rP-readedlen);
		apinfo->chl = GetNumFromAscii(buftmp, 0, "\0", rP1-rP-readedlen, 1);
		readedlen = rP1-rP+1;

		rP1 = myStrStr(rP+readedlen, ",", 0, sizeof(acBuf)-readedlen);
		if(rP1 == NULL)
		{
			i32Ret = -4;
			goto exit;
		}
		memcpy(buftmp, rP+readedlen, rP1-rP-readedlen);
		apinfo->ecn = GetNumFromAscii(buftmp, 0, "\0", rP1-rP-readedlen, 1);
		readedlen = rP1-rP+1;

		rP1 = myStrStr(rP+readedlen, ",", 0, sizeof(acBuf)-readedlen);
		if(rP1 == NULL)
		{
			i32Ret = -5;
			goto exit;
		}
		memcpy(buftmp, rP+readedlen, rP1-rP-readedlen);
		apinfo->max_conn = GetNumFromAscii(buftmp, 0, "\0", rP1-rP-readedlen, 1);
		readedlen = rP1-rP+1;

		memcpy(buftmp, rP+readedlen, 1);
		apinfo->ssid_hidden = GetNumFromAscii(buftmp, 0, "\0", 1, 1);
//		readedlen = rP1-rP+1;
	}
	
    i32Ret = 0;

exit:
	
	Wifi_wkup(FALSE);

	return i32Ret;
}
#if WIFI_ATTYPE
/*
* Description: 连接TCP
* Input:       pcTcpUdp:连接方式("TCP"/"UDP")    pcServeraddr:服务器地址("192.168.1.1"/"www.baidu.com.cn") 
*              pcPort:端口号("10000")         AT+CIPSTART="TCP","192.168.72.144",6000
* Output:N
* Return:      1:已连接过该服务器，0:连接成功，-1:连接超时，-2:系统忙，
*/
int hal_espTCPConnect(int i32Sockid, char *pcTcpUdp, char *pcServeraddr, char *pcPort, int i32Timeout)
{
    char at_cmd[128];
//    char acBuf[100];
    int i = 0;
    int j = 0;
	unsigned int ui32Time = 0;
	int32 i32Ret;
	ST_WIFI_PARAM stWifiParam;
	int i32Ret1 = -1;
	Wifi_wkup(TRUE);
	fibo_mutex_try_lock(g_ui32ApiWiFiSendMutex, 5000);
    memset(at_cmd, 0, sizeof(at_cmd));
	sprintf(at_cmd,"AT+CIPSTART=%d,\"", i32Sockid);
	i = strlen(at_cmd);

    while (*pcTcpUdp != '\0')
    {
        at_cmd[i++] = *pcTcpUdp++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';
    at_cmd[i++] = '\"';

    while (*pcServeraddr != '\0')
    {
        at_cmd[i++] = *pcServeraddr++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';

    while (*pcPort != '\0')
    {
        at_cmd[i++] = *pcPort++;
    }
    at_cmd[i++] = '\r';
    at_cmd[i++] = '\n';
#if 0	
	memset(acBuf, 0, 100);
	if(ESP_SendAndWaitCmd(at_cmd, i, acBuf, 100, i32Timeout, 1, "CONNECT\r\n\r\nOK\r\n") >= 0)
	{
		return 0;
	}
#else
	if(g_stWifiUart.wifibusy_flag==0)g_stWifiUart.wifibusy_flag=1;
	else 
	{
		ui32Time = hal_sysGetTickms() + i32Timeout;
		while(1)
		{
			sysDelayMs(10);
			if(g_stWifiUart.wifibusy_flag==0)
			{
				g_stWifiUart.wifibusy_flag=1;
				break;
			}
			if(hal_sysGetTickms() >= ui32Time)
			{
				sysLOG(HALESP8266_LOG_LEVEL_1, "wifibusy_flag:%d\r\n", g_stWifiUart.wifibusy_flag);
				Wifi_wkup(FALSE);
				i32Ret1 = WIFI_SEND_CONFLICT_FAIL;
				goto exit;
			}
		}
		
	}
	//Wifi_wkup(TRUE);

	for(; j < 3; j++)
	{
		DR_IPDmutex_lock(g_ui32IPDMutexHandle);
		//wifiRevBuffInit();
		g_stWifiUart.CMD_read_P=g_stWifiUart.CMD_write_P=0;
	    memset(g_stWifiUart.CMD_uart_buff,0,sizeof(g_stWifiUart.CMD_uart_buff));
		if(i32Sockid == 0)
		{
			g_stWifiUart.IPD_zero_read_P=g_stWifiUart.IPD_zero_write_P=0; 
			g_stWifiUart.IPD_zero_uart_buff_len=0;
			memset(g_stWifiUart.IPD_zero_uart_buff, 0x00, sizeof(g_stWifiUart.IPD_zero_uart_buff));
		}
        else if (i32Sockid == 1)
        {
            g_stWifiUart.IPD_one_read_P = g_stWifiUart.IPD_one_write_P = 0;
            g_stWifiUart.IPD_one_uart_buff_len = 0;
            memset(g_stWifiUart.IPD_one_uart_buff, 0x00, sizeof(g_stWifiUart.IPD_one_uart_buff));
        }
        else if (i32Sockid == 2)
        {
            g_stWifiUart.IPD_two_read_P = g_stWifiUart.IPD_two_write_P = 0;
            g_stWifiUart.IPD_two_uart_buff_len = 0;
            memset(g_stWifiUart.IPD_two_uart_buff, 0x00, sizeof(g_stWifiUart.IPD_two_uart_buff));
        }
        else if (i32Sockid == 3)
        {
            g_stWifiUart.IPD_three_read_P = g_stWifiUart.IPD_three_write_P = 0;
            g_stWifiUart.IPD_three_uart_buff_len = 0;
            memset(g_stWifiUart.IPD_three_uart_buff, 0x00, sizeof(g_stWifiUart.IPD_three_uart_buff));
        }
        else if (i32Sockid == 4)
        {
            g_stWifiUart.IPD_four_read_P = g_stWifiUart.IPD_four_write_P = 0;
            g_stWifiUart.IPD_four_uart_buff_len = 0;
            memset(g_stWifiUart.IPD_four_uart_buff, 0x00, sizeof(g_stWifiUart.IPD_four_uart_buff));
        }

		DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
		g_ui8CldsdClosemodeFlag=0;
		g_ui8WifiConnectClosed=0;
		
		if(g_stWifiUart.wifibusy_flag == 1)Wifi_send(at_cmd,i);

		/*等待接收*/
		ui32Time = hal_sysGetTickms() + i32Timeout;
		while(1)
		{
			if(dev_wifiWaitString("CONNECT\r\n\r\nOK\r\n") >= 0)
			{
				Wifi_wkup(FALSE);
				i32Ret1 = 0;
				goto exit;
			}

			if(dev_wifiWaitString("CLOSED") >= 0)
			{
				g_stWifiUart.wifibusy_flag = 1;
				sysLOG(HALESP8266_LOG_LEVEL_1, "CONNECTServerClosed\r\n");			
				break;
			}
			
			if(dev_wifiWaitString("DNS Fail") >= 0 && j==0)
			{
				g_stWifiUart.wifibusy_flag=0;
				//查IP，查不到则在循环一次在查不到就结束，查到继续往下走
				memset(stWifiParam.cIp, 0, sizeof(stWifiParam.cIp));
				memset(stWifiParam.cGateWay, 0, sizeof(stWifiParam.cGateWay));
				memset(stWifiParam.cNetMask, 0, sizeof(stWifiParam.cNetMask));
				i32Ret = hal_espStationGetIP(stWifiParam.cIp, stWifiParam.cGateWay, stWifiParam.cNetMask);
				if(i32Ret < 0)
				{
					g_stWifiUart.wifibusy_flag=0;					
					sysLOG(HALESP8266_LOG_LEVEL_2, "<ERR> hal_espStationGetIP i32Ret:%d\r\n", i32Ret);
					break;
				}
				else
				{
					sysLOG(HALESP8266_LOG_LEVEL_3, "<WARN> hal_espStationGetIP i32Ret:%d, cIp:%s, cGateWay:%s, cNetMask:%s\r\n", i32Ret, stWifiParam.cIp, stWifiParam.cGateWay, stWifiParam.cNetMask);
				}
				
			}

			if(hal_sysGetTickms() >= ui32Time)
			{
				j = 3;
				if(g_stWifiUart.CMD_write_P == 0)g_stWifiUart.wifibusy_flag = 2;//如果没有收到数据，则重试的时候不发送只接收即可，
				else
				{
					/*i32Ret = hal_espSetBaud();
					if(i32Ret == 0)
					{
						sysLOG(HALESP8266_LOG_LEVEL_1, "hal_espSetBaud, ret=%d\r\n", i32Ret);
					}*/
				}	
				sysLOG(HALESP8266_LOG_LEVEL_3, "ui32Time=%d %d\r\n", ui32Time, hal_sysGetTickms());			
				break;
			}
			//sysDelayMs(10);
		}
	}	
#endif
	if(dev_wifiWaitString("ALREADY CONNECTED") >= 0)
	{
		Wifi_wkup(FALSE);
		i32Ret1 = WIFI_TCP_CONNECTED;
		goto exit;
	}

	if(dev_wifiWaitString("busy p...") >= 0)
	{
		Wifi_wkup(FALSE);
		i32Ret1 = WIFI_SERVER_BUSY;
		goto exit;
	}
	Wifi_wkup(FALSE);
	i32Ret1 = WIFI_TCPCONNECT_FAIL;
	goto exit;

exit:
	g_stWifiUart.wifibusy_flag=0;
	fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
	return i32Ret1;
}
#endif

#if WIFI_ATTYPE
/*
* Description: 断开服务器连接
* Input:       N
* Output:N
* Return:      -1:断开失败，0:断开成功
*/
int hal_espTCPClose(int i32Sockid)
{
    char acBuf[100];
	char acCmd[100];
	int i32Ret;

	memset(acBuf, 0x00, sizeof(acBuf));
	memset(acCmd, 0x00, sizeof(acCmd));
	Wifi_wkup(TRUE);

	sprintf(acCmd,"AT+CIPCLOSE=%d\r\n", i32Sockid);
	i32Ret=ESP_SendAndWaitCmdwithErr(acCmd, strlen(acCmd), acBuf, 100, 2000, 2, "OK","ERROR");
	if(i32Ret>=0)
	{
		Wifi_wkup(FALSE);
		return 0;
	}
	else
	{
		Wifi_wkup(FALSE);
		return -1;
	}
}

#else

/*
* Description: 断开服务器连接
* Input:       N
* Output:N
* Return:      -1:断开失败，0:断开成功
*/
int hal_espTCPClose(void)
{
    char acBuf[100];
	int i32Ret;

	memset(acBuf, 0x00, sizeof(acBuf));
	Wifi_wkup(TRUE);

	
	i32Ret=ESP_SendAndWaitCmd("AT+VANCLOSE\r\n", 13, acBuf, 100, 2000, 2, "OK");
	if(i32Ret>=0)
	{
		Wifi_wkup(FALSE);
		return 0;
	}
	else
	{
		Wifi_wkup(FALSE);
		return -1;
	}
}
#endif

#if 0
#if WIFI_ATTYPE

/*
* Description: 连接SSL
* Input:       ssl:连接方式("SSL")    pcServeraddr:服务器地址("192.168.1.1"/"www.baidu.com.cn") 
*              pcPort:端口号("10000")         AT+CIPSTART="SSL","192.168.72.144",6000
* Output:N
* Return:      1:已连接过该服务器，0:连接成功，-1:连接超时，-2:系统忙，
*/
int ESP_SSLConnect(char *ssl,char *pcServeraddr,char *pcPort,int i32Timeout)
{
	char at_cmd[128];
    char acBuf[100];
    int i=0;
    int ret = 0;
    int j;
    int k;
    int t;
	Wifi_wkup(TRUE);
    memset(at_cmd, 0, sizeof(at_cmd));

    at_cmd[i++] = 'A';
    at_cmd[i++] = 'T';
    at_cmd[i++] = '+';
    at_cmd[i++] = 'C';
    at_cmd[i++] = 'I';
    at_cmd[i++] = 'P';
    at_cmd[i++] = 'S';
    at_cmd[i++] = 'T';
    at_cmd[i++] = 'A';
    at_cmd[i++] = 'R';
    at_cmd[i++] = 'T';
    at_cmd[i++] = '=';
    at_cmd[i++] = '\"';

    while (*ssl != '\0')
    {
        at_cmd[i++] = *ssl++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';
    at_cmd[i++] = '\"';

    while (*pcServeraddr != '\0')
    {
        at_cmd[i++] = *pcServeraddr++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';

    while (*pcPort != '\0')
    {
        at_cmd[i++] = *pcPort++;
    }
    at_cmd[i++] = '\r';
    at_cmd[i++] = '\n';
		
//   ret = ESP_SendAndWaitCmd("AT+CIPSSLSIZE=4096\r\n", strlen("AT+CIPSSLSIZE=4096\r\n"), acBuf, sizeof(acBuf), 500, 3, "OK");
    if(ret == 0)
			sysLOG(HALESP8266_LOG_LEVEL_1, "SSL缓存长度修改成功\r\n");
    else
			sysLOG(HALESP8266_LOG_LEVEL_1, "SSL缓存长度修改失败\r\n");
    
    memset(acBuf, 0, 100);
    
    for(t=0;t<10;t++)
    {
        for(j=1000;j>=0;j--)
                for(k=12000;k>0;k--);

//		ret = ESP_SendAndWaitNCmd(at_cmd, i, acBuf, sizeof(acBuf), i32Timeout, 1, "CONNECT", "ALREADY CONNECTED", "busy p...");
        if (ret == 0)
        {
                ret = 0;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL连接成功\r\n");
                break;					
        }
        else if (ret == 1)
        {
                ret = 1;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL已连接\r\n");
                break;
        }
        else if (ret == 2)
        {
                ret = -2;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL忙\r\n");
        }
        else if (ret == -1)
        {
                ret = -1;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL连接超时\r\n");
        }
    }
	Wifi_wkup(FALSE);
    return 0;
}
#else
/*
* Description: 连接SSL
* Input:       ssl:连接方式("SSL")    pcServeraddr:服务器地址("192.168.1.1"/"www.baidu.com.cn") 
*              pcPort:端口号("10000")         AT+VANSTART="SSL","192.168.72.144",6000
* Output:N
* Return:      1:已连接过该服务器，0:连接成功，-1:连接超时，-2:系统忙，
*/
int ESP_SSLConnect(char *ssl,char *pcServeraddr,char *pcPort,int i32Timeout)
{
	char at_cmd[128];
    char acBuf[100];
    int i=0;
    int ret = 0;
    int j;
    int k;
    int t;
	Wifi_wkup(TRUE);
    memset(at_cmd, 0, sizeof(at_cmd));

    at_cmd[i++] = 'A';
    at_cmd[i++] = 'T';
    at_cmd[i++] = '+';
    at_cmd[i++] = 'V';
    at_cmd[i++] = 'A';
    at_cmd[i++] = 'N';
    at_cmd[i++] = 'S';
    at_cmd[i++] = 'T';
    at_cmd[i++] = 'A';
    at_cmd[i++] = 'R';
    at_cmd[i++] = 'T';
    at_cmd[i++] = '=';
    at_cmd[i++] = '\"';

    while (*ssl != '\0')
    {
        at_cmd[i++] = *ssl++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';
    at_cmd[i++] = '\"';

    while (*pcServeraddr != '\0')
    {
        at_cmd[i++] = *pcServeraddr++;
    }
    at_cmd[i++] = '\"';
    at_cmd[i++] = ',';

    while (*pcPort != '\0')
    {
        at_cmd[i++] = *pcPort++;
    }
    at_cmd[i++] = '\r';
    at_cmd[i++] = '\n';
		
 //   ret = ESP_SendAndWaitCmd("AT+CIPSSLSIZE=4096\r\n", strlen("AT+CIPSSLSIZE=4096\r\n"), acBuf, sizeof(acBuf), 500, 3, "OK");
    if(ret == 0)
			sysLOG(HALESP8266_LOG_LEVEL_1, "SSL缓存长度修改成功\r\n");
    else
			sysLOG(HALESP8266_LOG_LEVEL_1, "SSL缓存长度修改失败\r\n");
    
    memset(acBuf, 0, 100);
    
    for(t=0;t<10;t++)
    {
        for(j=1000;j>=0;j--)
                for(k=12000;k>0;k--);

//		ret = ESP_SendAndWaitNCmd(at_cmd, i, acBuf, sizeof(acBuf), i32Timeout, 1, "CONNECT", "ALREADY CONNECTED", "busy p...");
        if (ret == 0)
        {
                ret = 0;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL连接成功\r\n");
                break;					
        }
        else if (ret == 1)
        {
                ret = 1;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL已连接\r\n");
                break;
        }
        else if (ret == 2)
        {
                ret = -2;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL忙\r\n");
        }
        else if (ret == -1)
        {
                ret = -1;
				sysLOG(HALESP8266_LOG_LEVEL_1, "SSL连接超时\r\n");
        }
    }
	Wifi_wkup(FALSE);
    return 0;
}
#endif	
#endif
/*
* Description: 等待服务器的数据
* Input:       acBuf:接收缓存(应比实际期望的最长数据) 
*              buflen:缓存长度
*              i32Timeout:超时时间
* Output:N
* Return:      -1:超时，>0:数据包解析得到的长度，该长度可能大于缓存长度 
*              当实际返回长度值大于缓存长度-14时，可能会产生部分数据丢失的情况。
*              所以当接收成功后务必通过长度或者数据自校验来验证是否接收完整。
*/
int hal_espWaitSeverData(int i32Sockid, char *acBuf,int buflen,int i32Timeout)
{
	
	return dev_wifiReadData(i32Sockid, acBuf,buflen,i32Timeout);
}

/*
* Description: 读取模块软件版本信息
* Input:       acBuf:接收缓存(应比实际期望的最长数据+10字节) 
*              buflen:缓存长度
*              i32Timeout:超时时间
* Output:N
* Return:      -1:超时，>0:数据包解析得到的长度。
*/
int hal_espReadVersion(char *acBuf,int buflen,int i32Timeout)
{
	int i32Ret = -1;

    memset(acBuf, 0, sizeof((char *)acBuf));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+GMR\r\n", strlen("AT+GMR\r\n"), acBuf, buflen, i32Timeout, 3, "OK");
	Wifi_wkup(FALSE);
	return i32Ret;
}

int hal_espGetUpdateState(char *acBuf, int buflen, int i32Timeout)

{
	int i32Ret = -1;

	memset(acBuf, 0, sizeof((char *)acBuf));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+CIUPDATE?\r\n", strlen("AT+CIUPDATE?\r\n"), acBuf, buflen, i32Timeout, 3, "OK");
	Wifi_wkup(FALSE);
	return i32Ret;
}

/*
*@brief:更新WiFi固件
*@param1:0-serveraddr为域名；1-serveraddr为IP;param2:服务器的域名地址或者IP;param3:路径;param4:超时时间
*@return:>0-成功;<0-失败
*/
int hal_espUpdate(char mode, char *pcServeraddr, char *url, int i32Timeout)

{
	int i32Ret = -1;
	int8 *rP=NULL;
	char recvbuf[128];
	char sendbuf[256];
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf,"AT+CIUPDATE=0,%d,\"%s\",\"%s\"\r\n", mode, pcServeraddr, url);
	//sysLOG(HALESP8266_LOG_LEVEL_1, "sendbuf:%s\r\n", sendbuf);
	Wifi_wkup(TRUE);
	
	i32Ret = ESP_SendAndWaitCmd(sendbuf, strlen(sendbuf), recvbuf, sizeof(recvbuf), i32Timeout, 1, "OK");
	if(i32Ret < 0)
	{
		rP = myStrStr(g_stWifiUart.CMD_uart_buff, "System Upgrade", g_stWifiUart.CMD_read_P, g_stWifiUart.CMD_write_P);
		if(rP == NULL)
		{
			Wifi_wkup(FALSE);
			return WIFI_UPDATE_ERR;
		}
		else
		{
			Wifi_wkup(FALSE);
			return 1;
		}
	}
	Wifi_wkup(FALSE);
	return i32Ret;
}

int hal_espReboot(char *acBuf, int buflen, int i32Timeout)

{
	int i32Ret = -1;

	memset(acBuf, 0, sizeof((char *)acBuf));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+REBOOT\r\n", strlen("AT+REBOOT\r\n"), acBuf, buflen, i32Timeout, 1, "OK");
	Wifi_wkup(FALSE);
	return i32Ret;
}

int hal_espGetTamperStatus(char *acBuf,int buflen,int i32Timeout)
{
	int i32Ret = -1;

    memset(acBuf, 0, sizeof((char *)acBuf));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+TAMPER?\r\n", strlen("AT+TAMPER?\r\n"), acBuf, buflen, i32Timeout, 3, "OK");
	Wifi_wkup(FALSE);
	return i32Ret;
}

int hal_espTamper(char mode, char *acBuf, int buflen, int i32Timeout)

{
	int i32Ret = -1;
	uint8 sendbuf[128];
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "AT+TAMPER=%d\r\n", mode);
	memset(acBuf, 0, sizeof(acBuf));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd(sendbuf, strlen(sendbuf), acBuf, buflen, i32Timeout, 1, "OK");
	Wifi_wkup(FALSE);
	return i32Ret;
}


int hal_espDownloadPSK(int i32Sockid, IN char *PSK, IN char *PSK_id)
{
    char cBuf[1000*5];
	char cSendBuff[96];
    int i32Ret = -1;
	int address = 0;
	int i = 0;
	int j = 0, lrc =0;
	int iSsidLen = 0;
	char *pcStrH = NULL;
	char *pcStrT = NULL;
	char *pcStrOK = NULL;
	char *pcStrPra = NULL;

	if((PSK_id == NULL) || (PSK == NULL))
	{
		return (-1);
	}
	
	Wifi_wkup(TRUE);
	sysLOG(BASE_LOG_LEVEL_4, " Download Cert01\r\n");
    memset(cBuf, 0, sizeof(cBuf));
	memset(cSendBuff, 0, sizeof(cSendBuff));

	sprintf(cSendBuff,"AT+CIPSSLCCONF=%d,0\r\n", i32Sockid);
	i=strlen(cSendBuff);

	i32Ret = ESP_SendAndWaitCmd(cSendBuff, i, cBuf, sizeof(cBuf), 5000, 3, "OK");
    if(i32Ret == 0)
	{
		sysLOG(HALESP8266_LOG_LEVEL_4, "SSL缓存长度修改成功\r\n");
	}else{
		sysLOG(HALESP8266_LOG_LEVEL_4, "SSL缓存长度修改失败\r\n");
	}

	memset(cBuf, 0, sizeof(cBuf));
	memset(cSendBuff, 0, sizeof(cSendBuff));

	sprintf(cSendBuff,"AT+CIPSSLCPSK=%d,\"%s\",\"%s\"\r\n", i32Sockid, PSK, PSK_id);
	i=strlen(cSendBuff);
    i32Ret = ESP_SendAndWaitCmd(cSendBuff, i, cBuf, sizeof(cBuf), 5000, 1, "\r\nOK\r\n");
    if(i32Ret < 0)
    {		
		sysLOG(BASE_LOG_LEVEL_2, " Download Cert i32Ret=%d\r\n",i32Ret);
    	Wifi_wkup(FALSE);
        return i32Ret;
    }
	
	sysLOG(BASE_LOG_LEVEL_4, " Download lrc= %d\r\n",lrc);

	sysLOG(BASE_LOG_LEVEL_4, " Download Cert04\r\n");
	Wifi_wkup(FALSE);
	return 0;
}

/*
* Description: 联机更新WIFI模块的固件，该接口返回时间比较长，中途请勿断电。
* Input:       N
* Output:N
* Return:      -1:错误，>0:更新成功
*/
int ESP_FW_Upadata(void)
{
    return 0;
}

/*
* Description: 设置WIFI主机名称
* Input:       N
* Output:N
* Return:      -1:错误，>=0:更新成功
*/
int ESP_SetName(char *name)
{
    char at_cmd[128];
    char acBuf[100];
    int i = 14;
    int ret = 0;
	Wifi_wkup(TRUE);

    memset(at_cmd, 0, sizeof(at_cmd));

    memcpy(at_cmd,"AT+CWHOSTNAME=",14);
    
    while (*name != '\0')
    {
        at_cmd[i++] = *name;
        name++;
    }
	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd(at_cmd, strlen(at_cmd), acBuf,sizeof(acBuf), 500, 2, "OK");
    if (ret < 0)
    {
    	Wifi_wkup(FALSE);
        return -1;
    }
	Wifi_wkup(FALSE);
    return 0;
}



/*
* Description: 查询Station已连接的AP信息
* Input:
* Output:	pstApInfo 
* Return:      
*/
ST_AP_INFO_ASY g_stApAsyInfo;

//
int hal_espCheckApAsync(ST_AP_INFO *pstApInfo)
{

	sysLOG(HALESP8266_LOG_LEVEL_4, "\r\n");
		
	if(pstApInfo != NULL)
	{
		memcpy(pstApInfo,(uint8_t *)&(g_stApAsyInfo.stApinfo),sizeof(ST_AP_INFO));
	}

	if(g_stApAsyInfo.apErrFlag < 0)
		return g_stApAsyInfo.apErrFlag;

	if(g_stApAsyInfo.stApinfo.iRssi < (-75))
	{
		return 1;
	}
	else if((g_stApAsyInfo.stApinfo.iRssi >= (-75)) && (g_stApAsyInfo.stApinfo.iRssi < (-65)))
	{
		return 2;
	}
	else 
	{
		return 3;
	}

}

int ESP_CheckAp(char versionFlag, char* atcmd, int cmdLen, ST_AP_INFO *pstApInfo)
{
	char cBuff[100];
	char *pcStrH = NULL;
	char *pcStrT = NULL;
	int iSsidLen = 0;
	int j = 0;

	memset(cBuff, 0x00, sizeof(cBuff));
	Wifi_wkup(TRUE);

	    if(ESP_SendAndWaitCmd(atcmd, cmdLen, cBuff, 100, 1000, 1, "\r\nOK\r\n") < 0)
		{
			Wifi_wkup(FALSE);
			return WIFI_CHECK_AP_ERR;
		}
		sysLOG(HALESP8266_LOG_LEVEL_4, "\r\n");
		if(g_cWifiVersionFlag == NOSDKVERSION)
		{
			pcStrH = strstr(cBuff, "+CWJAP_DEF:");
			if(pcStrH != NULL)
			{
				pcStrH += 11;
			}
		}
		else{
			pcStrH = strstr(cBuff, "+CWJAP:");
			if(pcStrH != NULL)
			{
				pcStrH += 7;
			}	
		}
		
		if(pcStrH != NULL)
		{
			sysLOG(HALESP8266_LOG_LEVEL_4, "pcStrH != NULL\r\n");
			if(pcStrH[0] == '\"')
			{
				
				pcStrH++;

				pcStrT = strstr(&pcStrH[0], "\"");
	/*取<ssid>*/			
				memset(pstApInfo->cSsid, 0x00, 64);

				iSsidLen = (pcStrT - pcStrH);
				if(iSsidLen > 64)
				{
					memcpy(pstApInfo->cSsid, &pcStrH[0], 64);
				}
				else
				{
					memcpy(pstApInfo->cSsid, &pcStrH[0], iSsidLen);
				}

				pcStrH = (pcStrT+3);
				
	/*取<mac>*/			
				pcStrT = strstr(&pcStrH[0], "\"");
				memset(pstApInfo->cBssid, 0x00, 20);
				memcpy(pstApInfo->cBssid, &pcStrH[0], (pcStrT - pcStrH));
				
				pcStrH = (pcStrT+2);
				
	/*取<channel>*/			
				pstApInfo->iChannel = 0;
				pcStrT = strstr(&pcStrH[0], ",");
				for (j = 0; j < (pcStrT - pcStrH); j++)
				{
					pstApInfo->iChannel *= 10;
					pstApInfo->iChannel += (pcStrH[j] - '0');
				}
				
				pcStrH = (pcStrT+2);
				
	/*取<rssi>*/			
				pstApInfo->iRssi = 0;
				pcStrT = strstr(&pcStrH[0], ",");
				for (j = 0; j < (pcStrT - pcStrH); j++)
				{
					pstApInfo->iRssi *= 10;
					pstApInfo->iRssi += (pcStrH[j] - '0');
				}
				pstApInfo->iRssi *= (-1);
				
				//sysLOG(2,"***********pstApInfo->iRssi:%d************\r\n",pstApInfo->iRssi);
				//checkApFlag++;
				//memcpy((uint8_t *)&gstApInfo,pstApInfo,sizeof(ST_AP_INFO));
				if(pstApInfo->iRssi < (-75))
				{
					Wifi_wkup(FALSE);
					return 1;
				}
				else if((pstApInfo->iRssi >= (-75)) && (pstApInfo->iRssi < (-65)))
				{
					Wifi_wkup(FALSE);
					return 2;
				}
				else 
				{
					Wifi_wkup(FALSE);
					return 3;
				}
			}
		}
		sysLOG(HALESP8266_LOG_LEVEL_4, "\r\n");
		if(Search_StringBInStringA(cBuff, "No AP\r\n") >= 0)		/*当前未连接AP*/
		{
			Wifi_wkup(FALSE);
			return WIFI_NOT_APCONNECT_ERR;
		}	
		sysLOG(HALESP8266_LOG_LEVEL_4, "\r\n");	
		Wifi_wkup(FALSE);
		return WIFI_ERRCODE_NOTDEFINE;
}

int hal_espCheckAp(ST_AP_INFO *pstApInfo)
{
	int i32Ret = 0;

	#if 0
	if(checkApFlag > 5)
	{
		memcpy(pstApInfo,(uint8_t *)&gstApInfo,sizeof(ST_AP_INFO));
		sysLOG(HALESP8266_LOG_LEVEL_1, "hal_espCheckAp checkflag:%d, ssid:%s\r\n",checkApFlag,gstApInfo.cSsid);
		return 3;
	}
	#endif

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		return ESP_CheckAp(0, "AT+CWJAP_DEF?\r\n", 15, pstApInfo);
	}
	else{
		return ESP_CheckAp(1, "AT+CWJAP?\r\n", 11, pstApInfo);
	}

	
}

/*
* Description: 指定SSID，查找符合条件的AP
* Input:	pcSsid		想要查找的SSID
* Output:	pstApMac 
* Return:      
*/

int hal_espScanAp(IN char *pcSsid, OUT ST_AP_MAC *pstApMac)
{
    char cBuf[100*5];
	char cSendBuff[96];
    int i32Ret = -1;
	int i = 0;
	int iSsidLen = 0;
	char *pcStrH = NULL;
	char *pcStrT = NULL;
	char *pcStrOK = NULL;
	char *pcStrPra = NULL;

	if(pcSsid == NULL)
	{
		return (-1);
	}
	Wifi_wkup(TRUE);

    memset(cBuf, 0, sizeof(cBuf));
	memset(cSendBuff, 0, sizeof(cSendBuff));

	memcpy(cSendBuff, "AT+CWLAP=\"", 10);
	i = 10;
    while (*pcSsid != '\0')
    {
        cSendBuff[i++] = *pcSsid++;
		iSsidLen++;
    }
	cSendBuff[i++] = '\"';
	cSendBuff[i++] = '\r';
    cSendBuff[i++] = '\n';
	
    i32Ret = ESP_SendAndWaitCmd(cSendBuff, i, cBuf, sizeof(cBuf), 5000, 1, "\r\nOK\r\n");
    if(i32Ret < 0)
    {		
    	Wifi_wkup(FALSE);
        return i32Ret;
    }

	pcStrOK = strstr(cBuf, "\r\nOK\r\n");
	pcStrPra = &cBuf[0];

	pcStrH = strstr(&pcStrPra[0], "+CWLAP:");		
	pcStrT = strstr(&pcStrPra[0], ")");
	if((pcStrH == NULL) && (pcStrT == NULL))
	{
		Wifi_wkup(FALSE);
		return -3;	
	}
	
	while (1)
	{
		pcStrH = strstr(&pcStrPra[0], "+CWLAP:");		
		pcStrT = strstr(&pcStrPra[0], ")");
		if((pcStrH != NULL) && (pcStrT != NULL) && (pcStrH < pcStrOK))
		{
			pcStrH += 10;													/*跳过<ecn>*/

			if((pcStrH[0] == '\"')&&(pcStrH[0] == pcStrH[1]))
			{
				pcStrPra = strstr(&pcStrH[0], ")");
				pcStrPra += 3;			
			}
			else
			{
				pcStrH++;
				pcStrPra = strstr(&pcStrH[0], "\"");					   /*跳过<ssid>*/		
				pcStrH = (pcStrPra+3);
				
				pcStrPra = strstr(&pcStrH[0], ",");						   /*跳过<rssi>*/
				pcStrH = (pcStrPra+2);
					
				/*获取<MAC>*/
				pcStrPra = strstr(&pcStrH[0], "\"");
				memset(pstApMac->cBssid[pstApMac->iMacCnt], 0x00, 20);
				memcpy(pstApMac->cBssid[pstApMac->iMacCnt], &pcStrH[0], (pcStrPra - pcStrH));
				pcStrH = (pcStrPra+2);
				
				pstApMac->iMacCnt++;

				if(pstApMac->iMacCnt >= 5)
				{
					Wifi_wkup(FALSE);
					return pstApMac->iMacCnt;
				}
	
				pcStrPra = strstr(&pcStrH[0], ")");
				pcStrPra += 3;
			}
		}
		else
		{
			Wifi_wkup(FALSE);
			return pstApMac->iMacCnt;
		}
	}
	Wifi_wkup(FALSE);
	return 0;
}


/*
* Description: 设置Station的MAC地址
* Input:	
* Output:	
* Return:      
*/
int hal_espStationSetMac(unsigned char *pucMac)
{
	char at_cmd[128];
    char acBuf[100];
    int i = 0;
    int ret = 0;

	Wifi_wkup(TRUE);
	memset(acBuf, 0, sizeof(acBuf));
	memset(at_cmd, 0x00, sizeof(at_cmd));

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		i = 17;
		memset(at_cmd, 0, sizeof(at_cmd));
		memcpy(at_cmd,"AT+CIPSTAMAC_DEF=",i);
		at_cmd[i++] = 0x22;
		memcpy(at_cmd+i,pucMac,17);
		i += 17;
		at_cmd[i++] = 0x22;
		memcpy(at_cmd+i,"\r\n",2);
	}
    else{
		i = 13;
		memset(at_cmd, 0, sizeof(at_cmd));
		memcpy(at_cmd,"AT+CIPSTAMAC=",i);
		at_cmd[i++] = 0x22;
		memcpy(at_cmd+i,pucMac,17);
		i += 17;
		at_cmd[i++] = 0x22;
		memcpy(at_cmd+i,"\r\n",2);
	}
	
	//ret = ESP_SendAndWaitCmd(at_cmd, strlen(at_cmd), acBuf,sizeof(acBuf), 1000, 2, "OK");
	ret = ESP_SendAndWaitCmd(at_cmd, i+2, acBuf,sizeof(acBuf), 1000, 2, "OK");

    if (ret < 0)
    {
		scrClrLine_lib(5,6);
		scrPrint_lib(0, 5, 2, "sendMacFail %d\r\n",ret);
		//sysLOG(BASE_LOG_LEVEL_1, " transceiveFrame end ret = %d\r\n",ret);
    	Wifi_wkup(FALSE);
        return -1;
    }
	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 获取Station的MAC地址
* Input:	
* Output:	
* Return:      
*/
int hal_espStationGetMacDef(unsigned char *pucMac)
{
	int i32Ret = -1;
	char cRecvBuff[100];

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CIPSTAMAC_DEF?\r\n", strlen("AT+CIPSTAMAC_DEF?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		i32Ret = Search_StringBInStringA(cRecvBuff, "+CIPSTAMAC_DEF:");
		if(i32Ret >= 0)
		{
			memcpy(pucMac, cRecvBuff+i32Ret+16, 17);
			Wifi_wkup(FALSE);
			return 0;
		}
	}
	else{
		i32Ret = ESP_SendAndWaitCmd("AT+CIPSTAMAC?\r\n", strlen("AT+CIPSTAMAC?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		i32Ret = Search_StringBInStringA(cRecvBuff, "+CIPSTAMAC:");
		if(i32Ret >= 0)
		{
			memcpy(pucMac, cRecvBuff+i32Ret+12, 17);
			Wifi_wkup(FALSE);
			return 0;
		}
	}
    
	Wifi_wkup(FALSE);
	return -1;
}
/*
* Description: 获取Station的CUR MAC地址
* Input:	
* Output:	
* Return:      
*/
int hal_espStationGetMac(unsigned char *pucMac)
{
	int i32Ret = -1;
	char cRecvBuff[100];

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CIPSTAMAC_CUR?\r\n", strlen("AT+CIPSTAMAC_CUR?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		i32Ret = Search_StringBInStringA(cRecvBuff, "+CIPSTAMAC_CUR:");
		if(i32Ret >= 0)
		{
			memcpy(pucMac, cRecvBuff+i32Ret+16, 17);
			Wifi_wkup(FALSE);
			return 0;
		}
	}
	else{
		i32Ret = ESP_SendAndWaitCmd("AT+CIPSTAMAC?\r\n", strlen("AT+CIPSTAMAC?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		i32Ret = Search_StringBInStringA(cRecvBuff, "+CIPSTAMAC:");
		if(i32Ret >= 0)
		{
			memcpy(pucMac, cRecvBuff+i32Ret+12, 17);
			Wifi_wkup(FALSE);
			return 0;
		}
	}
    
	Wifi_wkup(FALSE);
	return -1;
}



/*
* Description: 设置Station的DHCP
* Input:	
* Output:	
* Return:      
*/
int hal_espStationSetDHCP(int iEnable)
{
	int i32Ret = -1;
	char cRecvBuff[100];
	
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);

	if(0 == iEnable)			/*关闭DHCP*/
	{
		if(g_cWifiVersionFlag == NOSDKVERSION)
		{
			i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP_DEF=1,0\r\n", strlen("AT+CWDHCP_DEF=1,0\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{
				Wifi_wkup(FALSE);
				return -1;
			}
		}
		else{
			i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP=0,1\r\n", strlen("AT+CWDHCP=0,1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{
				Wifi_wkup(FALSE);
				return -1;
			}
		}	
	}
	else						/*开启DHCP*/
	{
		if(g_cWifiVersionFlag == NOSDKVERSION)
		{
			i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP_DEF=1,1\r\n", strlen("AT+CWDHCP_DEF=1,1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{
				Wifi_wkup(FALSE);
				return -1;
			}
		}
		else{
			i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP=1,1\r\n", strlen("AT+CWDHCP=1,1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{
				Wifi_wkup(FALSE);
				return -1;
			}
		}		
	}
	Wifi_wkup(FALSE);
	return 0;
}

/*
* Description: 查询Station的DHCP
* Input:	
* Output:	
* Return:      0  --- Station DHCP关闭
			   1  --- Station DHCO开启
*/

int hal_espStationGetDHCP(void)
{
	int i32Ret = -1;
	char cRecvBuff[100];
	
/*获取DHCP使能*/ /*初始状态下:DHCP开启*/
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP_DEF?\r\n", strlen("AT+CWDHCP_DEF?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		
		i32Ret = Search_StringBInStringA(cRecvBuff, "+CWDHCP_DEF:");
		if(i32Ret < 0)
		{
			Wifi_wkup(FALSE);
			return -1;
		}
		Wifi_wkup(FALSE);
		return (((cRecvBuff[i32Ret + 12] - '0') >> 1) & 0x01);
	}
	else{
		i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP?\r\n", strlen("AT+CWDHCP?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		
		i32Ret = Search_StringBInStringA(cRecvBuff, "+CWDHCP:");
		if(i32Ret < 0)
		{
			Wifi_wkup(FALSE);
			return -1;
		}
		Wifi_wkup(FALSE);
		return (((cRecvBuff[i32Ret + 8] - '0') >> 1) & 0x01);
	}	
	
}

/*
* Description: 设置Station的IP 网关 子网掩码
* Input:	
* Output:	
* Return:      
*/

int hal_espStationSetIP(char *pcIP, char *pcGateWay, char *pcNetMask)
{
	int i32Ret = -1;
	char cSendbuff[200];
	char cRecvBuff[200];
	//char *pstrH = NULL;
	//char *pstrT = NULL;
	int i = 0;

/*设置静态IP、子网掩码、网关*/ 
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	memset(cSendbuff, 0x00, sizeof(cSendbuff));

	if(0 == memcmp(pcIP, "0.0.0.0", strlen("0.0.0.0")))
	{
		return 0;
	}

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		memcpy(cSendbuff, "AT+CIPSTA_DEF=", strlen("AT+CIPSTA_DEF="));
		i = strlen("AT+CIPSTA_DEF=");
	}
	else{
		memcpy(cSendbuff, "AT+CIPSTA=", strlen("AT+CIPSTA="));
		i = strlen("AT+CIPSTA=");
	}
	
	cSendbuff[i++] = '\"';
	memcpy(cSendbuff+i, pcIP, strlen((char*)pcIP));
	i += strlen((char*)pcIP);
	cSendbuff[i++] = '\"';

	if((0 != memcmp(pcGateWay, "0.0.0.0", strlen("0.0.0.0")))||(0 != memcmp(pcNetMask, "0.0.0.0", strlen("0.0.0.0"))))
	{
		cSendbuff[i++] = ',';

		if(0 != memcmp(pcGateWay, "0.0.0.0", strlen("0.0.0.0")))
		{
			cSendbuff[i++] = '\"';
			memcpy(cSendbuff+i, pcGateWay, strlen((char*)pcGateWay));
			i += strlen((char*)pcGateWay);
			cSendbuff[i++] = '\"';
		}

		if(0 != memcmp(pcNetMask, "0.0.0.0", strlen("0.0.0.0")))
		{
			cSendbuff[i++] = ',';
			cSendbuff[i++] = '\"';
			memcpy(cSendbuff+i, pcNetMask, strlen((char*)pcNetMask));
			i += strlen((char*)pcNetMask);
			cSendbuff[i++] = '\"';
		}
	}
		
	cSendbuff[i++] = '\r';
	cSendbuff[i++] = '\n';

	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd(cSendbuff, i, cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{	
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	Wifi_wkup(FALSE);
	return 0;
}

/*
* Description: 获取Station的IP 网关 子网掩码
* Input:	
* Output:	
* Return:      
*/

int hal_espStationGetIP(char *pcIP, char *pcGateWay, char *pcNetMask)
{
	int i32Ret = -1;
	char cRecvBuff[200];
	char *pstrH = NULL;
	char *pstrT = NULL;

	Wifi_wkup(TRUE);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CIPSTA?\r\n", strlen("AT+CIPSTA?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{	
		Wifi_wkup(FALSE);
		return i32Ret;
	}

	pstrH = strstr(cRecvBuff, "+CIPSTA:ip:\"");
	if(pstrH != NULL)
	{
		pstrH += strlen("+CIPSTA:ip:\"");

		pstrT = strstr(pstrH, "\"\r\n");

		memcpy(pcIP, &pstrH[0], (pstrT - pstrH));
	}
	else
	{
		Wifi_wkup(FALSE);
		return -2;
	}

	pstrH = strstr(cRecvBuff, "+CIPSTA:gateway:\"");
	if(pstrH != NULL)
	{
		pstrH += strlen("+CIPSTA:gateway:\"");

		pstrT = strstr(pstrH, "\"\r\n");

		memcpy(pcGateWay, &pstrH[0], (pstrT - pstrH));
	}
	else
	{
		Wifi_wkup(FALSE);
		return -3;
	}

	pstrH = strstr(cRecvBuff, "+CIPSTA:netmask:\"");
	if(pstrH != NULL)
	{
		pstrH += strlen("+CIPSTA:netmask:\"");

		pstrT = strstr(pstrH, "\"\r\n");

		memcpy(pcNetMask, &pstrH[0], (pstrT - pstrH));
	}
	else
	{
		Wifi_wkup(FALSE);
		return -4;
	}
	Wifi_wkup(FALSE);
	return 0;
}

/*
* Description: 使能DHCP后，调用此接口来获取本地IP,动态IP
* Input:	
* Output:	
* Return:      
*/
int hal_espGetIPAndMac(char *pcIPBuff, char *pcMacBuff)
{
	int i32Ret = -1;
	char cRecvBuff[200];
	char *pStrH = NULL;
	char *pStrT = NULL;

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+CIFSR\r\n", strlen("AT+CIFSR\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	
	if(pcIPBuff != NULL)
	{
		pStrH = strstr(cRecvBuff, "+CIFSR:STAIP,\"");
		if(pStrH == NULL)
		{
			Wifi_wkup(FALSE);
			return -1;
		}
		pStrH += strlen("+CIFSR:STAIP,\"");
		
		pStrT = strstr(cRecvBuff, "\"\r\n");

		memcpy(pcIPBuff, &pStrH[0], (pStrT-pStrH));
	}
	
	if(pcMacBuff != NULL)
	{
		pStrH = strstr(&pStrH[0], "+CIFSR:STAMAC,\"");
		if(pStrH == NULL)
		{
			Wifi_wkup(FALSE);
			return -1;
		}

		pStrH += strlen("+CIFSR:STAMAC,\"");
		
		pStrT = strstr(&pStrH[0], "\"\r\n");

		memcpy(pcMacBuff, &pStrH[0], (pStrT-pStrH));
	}

	Wifi_wkup(FALSE);
	return 0;
}

/*
* Description: 自定义DNS使能
* Input:	
* Output:	
* Return:      
*/
int hal_espConfigDNSEnable(int iEnable)
{
	int i32Ret = -1;
	char cRecvBuff[200];
	
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);


	if(0 == iEnable)								/*自动*/
	{
		if(g_cWifiVersionFlag == NOSDKVERSION)
		{
			i32Ret = ESP_SendAndWaitCmd("AT+CIPDNS_DEF=0\r\n", strlen("AT+CIPDNS_DEF=0\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{		
				Wifi_wkup(FALSE);
				return i32Ret;
			}	
		}
		else{
			i32Ret = ESP_SendAndWaitCmd("AT+CIPDNS=0\r\n", strlen("AT+CIPDNS=0\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{		
				Wifi_wkup(FALSE);
				return i32Ret;
			}	
		}
			
	}
	else											/*手动*/
	{
		if(g_cWifiVersionFlag == NOSDKVERSION)
		{
			i32Ret = ESP_SendAndWaitCmd("AT+CIPDNS_DEF=1\r\n", strlen("AT+CIPDNS_DEF=1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{		
				Wifi_wkup(FALSE);
				return i32Ret;
			}
		}
		else{
			i32Ret = ESP_SendAndWaitCmd("AT+CIPDNS=1\r\n", strlen("AT+CIPDNS=1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
			if(i32Ret < 0)
			{		
				Wifi_wkup(FALSE);
				return i32Ret;
			}
		}
				
	}
	Wifi_wkup(FALSE);
	return 0;
}

/*
* Description: 自定义DNS服务器
* Input:	
* Output:	
* Return:      
*/
int hal_espConfigDNS(char *pcDnsServer0, char *pcDnsServer1)
{
	int i32Ret = -1;
	char cSendbuff[100];
	char cRecvBuff[100];
	int i = 0;

	if(pcDnsServer0 == NULL)
	{
		return -1;
	}
	
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	memset(cSendbuff, 0x00, sizeof(cSendbuff));

	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		memcpy(cSendbuff, "AT+CIPDNS_DEF=1", strlen("AT+CIPDNS_DEF=1"));			/*使能自定义DNS服务器*/
		i = strlen("AT+CIPDNS_DEF=1");
	}
	else{
		memcpy(cSendbuff, "AT+CIPDNS=1", strlen("AT+CIPDNS=1"));			/*使能自定义DNS服务器*/
		i = strlen("AT+CIPDNS=1");
	}
	
	
	if((NULL != pcDnsServer0) || ((NULL != pcDnsServer1)))
	{
		cSendbuff[i++] = ',';
		if(NULL != pcDnsServer0)
		{
			cSendbuff[i++] = '\"';
			memcpy(cSendbuff+i, pcDnsServer0, strlen(pcDnsServer0));
			i += strlen(pcDnsServer0);
			cSendbuff[i++] = '\"';	
		}

		if(NULL != pcDnsServer1)
		{
			cSendbuff[i++] = ',';
			cSendbuff[i++] = '\"';
			memcpy(cSendbuff+i, pcDnsServer1, strlen(pcDnsServer1));
			i += strlen(pcDnsServer1);
			cSendbuff[i++] = '\"';	
		}
	}

	cSendbuff[i++] = '\r';
	cSendbuff[i++] = '\n';

	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd(cSendbuff, i, cRecvBuff, sizeof(cRecvBuff), 1000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		Wifi_wkup(FALSE);
		return i32Ret;
	}		

	Wifi_wkup(FALSE);
	return 0;
}

int hal_espGetDns(char *pcDnsServer0, char *pcDnsServer1)
{
	int i32Ret = -1;
	char cRecvBuff[200];
	char *pStrH = NULL;
	char *pStrT = NULL;

	if((NULL == pcDnsServer0) || (NULL == pcDnsServer1))
	{
		return -1;
	}
	Wifi_wkup(TRUE);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CIPDNS_DEF?\r\n", strlen("AT+CIPDNS_DEF?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}

		pStrH = strstr(cRecvBuff, "+CIPDNS_DEF:");
		if(NULL == pStrH)
		{
			memcpy(pcDnsServer0, "0.0.0.0", strlen("0.0.0.0"));
			memcpy(pcDnsServer1, "0.0.0.0", strlen("0.0.0.0"));
			Wifi_wkup(FALSE);
			return 0;
		}

		pStrT = strstr(cRecvBuff, "\r\n");
		memcpy(pcDnsServer0, pStrH+12, (pStrT-(pStrH+12)));

		pStrH += 12;

		pStrH = strstr(pStrT+2, "+CIPDNS_DEF:");
		if(NULL == pStrH)
		{
			memcpy(pcDnsServer1, "0.0.0.0", strlen("0.0.0.0"));
			Wifi_wkup(FALSE);
			return 0;
		}

		pStrT = strstr(pStrH, "\r\n");
		memcpy(pcDnsServer1, pStrH+12, (pStrT-(pStrH+12)));
	}
	else{
		i32Ret = ESP_SendAndWaitCmd("AT+CIPDNS?\r\n", strlen("AT+CIPDNS?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
		
		pStrH = strstr(cRecvBuff, ",\"");
		if(NULL == pStrH)
		{
			memcpy(pcDnsServer0, "0.0.0.0", strlen("0.0.0.0"));
			memcpy(pcDnsServer1, "0.0.0.0", strlen("0.0.0.0"));
			Wifi_wkup(FALSE);
			return 0;
		}
		pStrT = strstr(pStrH, "\",");
		memcpy(pcDnsServer0, pStrH+2, (pStrT-(pStrH+2)));
		sysLOG(HALESP8266_LOG_LEVEL_4, "  pcDnsServer0 = %s\r\n", pcDnsServer0);
		pStrH = strstr(pStrT, ",\"");
		if(NULL == pStrH)
		{
			memcpy(pcDnsServer1, "0.0.0.0", strlen("0.0.0.0"));
			Wifi_wkup(FALSE);
			return 0;
		}
		pStrT = strstr(pStrH, "\"\r\n");
		memcpy(pcDnsServer1, pStrH+2, (pStrT-(pStrH+2)));
		sysLOG(HALESP8266_LOG_LEVEL_4, "  pcDnsServer1 = %s\r\n", pcDnsServer1);
	}
	
	Wifi_wkup(FALSE);
	return 0;
}

int hal_espAutoConnect(int iEnable)
{
	int i32Ret = -1;
	char cRecvBuff[100];

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	Wifi_wkup(TRUE);

	if(0 == iEnable)
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CWAUTOCONN=0\r\n", strlen("AT+CWAUTOCONN=0\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}
	}
	else
	{
		i32Ret = ESP_SendAndWaitCmd("AT+CWAUTOCONN=1\r\n", strlen("AT+CWAUTOCONN=1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
		if(i32Ret < 0)
		{		
			Wifi_wkup(FALSE);
			return i32Ret;
		}		
	}
	Wifi_wkup(FALSE);
	return 0;
}

int hal_wifiAtCIPDOMAIN(char *pcDomain, char *pcIP)
{
	char cSendBuff[128];
	char cRecvBuff[128];
	char *pStrH = NULL;
	int i = 0;
	int i32Ret = -1;

	memset(cSendBuff, 0x00, sizeof(cSendBuff));
	
	memcpy(cSendBuff, "AT+CIPDOMAIN=", strlen("AT+CIPDOMAIN="));
	i = strlen("AT+CIPDOMAIN=");
	cSendBuff[i++] = '\"';
	memcpy(cSendBuff+i, pcDomain, strlen(pcDomain));
	i += strlen(pcDomain);
	cSendBuff[i++] = '\"';
	cSendBuff[i++] = '\r';
	cSendBuff[i++] = '\n';

	Wifi_wkup(TRUE);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd(cSendBuff, i, cRecvBuff, sizeof(cRecvBuff), 10000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{	
		Wifi_wkup(FALSE);
		return i32Ret;
	}

	pStrH = strstr(cRecvBuff, "+CIPDOMAIN:");
	if(pStrH == NULL)
	{
		Wifi_wkup(FALSE);
		return -2;
	}

	pStrH += strlen("+CIPDOMAIN:");
	
	i = 0;
	while(*pStrH)
	{
		if((pStrH[0] == '\r') || (pStrH[0] == '\n'))
		{
			break;
		}

		pcIP[i] = pStrH[0];

		pStrH++;
		i++;
	}
	Wifi_wkup(FALSE);
	return 0;
}

#if 0
int hal_espUpdateSysTime(char *pcNTP, int iGmTz)
{
	char cSendBuff[128];
	char cRecvBuff[128];
	char cMonthStr[][3] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	unsigned char ucDateTime[7];
	int i = 0;
	char *pStrH = NULL;
	int i32Ret = -1;
	int iSendlen = 0;
	
	memset(cSendBuff, 0x00, sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	memset(ucDateTime, 0x00, sizeof(ucDateTime));
	
	iSendlen = strlen("AT+CIPSNTPCFG=1,");
	memcpy(cSendBuff, "AT+CIPSNTPCFG=1,", iSendlen);

	if(iGmTz < 0)
	{
		cSendBuff[iSendlen++] = '-';

		iGmTz *= (-1);
	}

	if(iGmTz >= 10)
	{
		cSendBuff[iSendlen++] = '1';
	}

	cSendBuff[iSendlen++] = iGmTz%10+0x30;
	cSendBuff[iSendlen++] = ',';

	cSendBuff[iSendlen++] = '\"';
	memcpy(cSendBuff+iSendlen, pcNTP, strlen(pcNTP));
	iSendlen += strlen(pcNTP);
	cSendBuff[iSendlen++] = '\"';
	
	cSendBuff[iSendlen++] = '\r';
	cSendBuff[iSendlen++] = '\n';	
	
	i32Ret = ESP_SendAndWaitCmd(cSendBuff, iSendlen, cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		return WIFI_CIPSNTPCFG_FAIL;
	}

	sysDelayMs(2000);

	i32Ret = ESP_SendAndWaitCmd("AT+CIPSNTPTIME?\r\n", strlen("AT+CIPSNTPTIME?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		return WIFI_CIPSNTPTIME_FAIL;
	}

	pStrH = strstr(cRecvBuff, "+CIPSNTPTIME:");
	if(pStrH == NULL)
	{
		return WIFI_CIPSNTPTIME_FAIL;
	}

	pStrH += strlen("+CIPSNTPTIME:");

	pStrH += 4;	/*跳过周*/

/*月份*/
	for(i = 0; i < 12; i++)
	{
		if(memcmp(pStrH, &cMonthStr[i], 3) == 0)
		{
			if((i+1)>=10)
			{
				ucDateTime[1] = 0x10 + (i+1)%10;
			}
			else
			{
				ucDateTime[1] = (i+1);
			}

			break;
		}
	}

	if(i == 12)
	{
		return WIFI_CIPSNTPTIME_FAIL;
	}

	pStrH += 4;	/*跳过月*/
/*日期*/
	ucDateTime[2] = ((pStrH[0]-0x30)*0x10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过日期*/
/*小时*/
	ucDateTime[3] = ((pStrH[0]-0x30)*0x10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过小时*/
/*分钟*/
	ucDateTime[4] = ((pStrH[0]-0x30)*0x10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过分钟*/
/*秒*/
	ucDateTime[5] = ((pStrH[0]-0x30)*0x10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过秒*/
/*年份*/
	ucDateTime[0] = ((pStrH[2]-0x30)*0x10) + (pStrH[3]-0x30);

	hal_setDateTime(ucDateTime, iGmTz);//0x19 0x05 0x24 0x10 0x18 0x33
	
	return 0;
}
#else
int hal_espUpdateSysTime(void)
{
	char cRecvBuff[128];
	char cMonthStr[][3] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	unsigned char ucDateTime[7];
	int i = 0;
	char *pStrH = NULL;
	int i32Ret = -1;
	hal_rtc_time_t rtc_time;

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	memset(ucDateTime, 0x00, sizeof(ucDateTime));
	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+CIPSNTPTIME?\r\n", strlen("AT+CIPSNTPTIME?\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{	
		Wifi_wkup(FALSE);
		return WIFI_CIPSNTPTIME_FAIL;
	}

	pStrH = strstr(cRecvBuff, "+CIPSNTPTIME:");
	if(pStrH == NULL)
	{
		Wifi_wkup(FALSE);
		return WIFI_CIPSNTPTIME_FAIL;
	}

	pStrH += strlen("+CIPSNTPTIME:");

	pStrH += 4;	/*跳过周*/

/*月份*/
	for(i = 0; i < 12; i++)
	{
		if(memcmp(pStrH, &cMonthStr[i], 3) == 0)
		{
			rtc_time.month = (i+1);
			break;
		}
	}

	if(i == 12)
	{
		Wifi_wkup(FALSE);
		return WIFI_CIPSNTPTIME_FAIL;
	}

	pStrH += 4;	/*跳过月*/
/*日期*/
	rtc_time.day = ((pStrH[0]-0x30)*10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过日期*/
/*小时*/
	rtc_time.hour = ((pStrH[0]-0x30)*10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过小时*/
/*分钟*/
	rtc_time.min = ((pStrH[0]-0x30)*10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过分钟*/
/*秒*/
	rtc_time.sec = ((pStrH[0]-0x30)*10) + (pStrH[1]-0x30);

	pStrH += 3;	/*跳过秒*/
/*年份*/
	rtc_time.year = ((pStrH[0]-0x30)*1000) + ((pStrH[1]-0x30)*100) + ((pStrH[2]-0x30)*10) + (pStrH[3]-0x30);


	 fibo_setRTC(&rtc_time);

	Wifi_wkup(FALSE);
	
	return 0;
}
#endif


int hal_wifiAtRESTORE(void)
{
	//char cSendBuff[128];
	char cRecvBuff[128];
	//char *pStrH = NULL;

	int i32Ret = -1;

	Wifi_wkup(TRUE);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+RESTORE\r\n", strlen("AT+RESTORE\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		Wifi_wkup(FALSE);
		return WIFI_RESTORE_FAIL;
	}
	sysDelayMs(2000);
	Wifi_wkup(FALSE);
	return 0;
}

int hal_wifiReset(void)
{
	DR_Wifi_Pwr_EN(FALSE);//dev_espPowerOff();

	sysDelayMs(500);

	return 0;
}


int hal_espGetApNum(void)
{
	char cRecvBuff[128];

	//int i = 0;
	char *pStrH = NULL;
	int i32Ret = -1;

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));

	Wifi_wkup(TRUE);

	i32Ret = ESP_SendAndWaitCmd("AT+GETAPINFO\r\n", strlen("AT+GETAPINFO\r\n"), cRecvBuff, sizeof(cRecvBuff), 1000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{	
		Wifi_wkup(FALSE);
		return WIFI_GETAPINFO_FAIL;
	}

	pStrH = strstr(cRecvBuff, "+ApNum: ");
	if(pStrH == NULL)
	{
		Wifi_wkup(FALSE);
		return WIFI_GETAPINFO_FAIL;
	}

	pStrH += strlen("+ApNum: ");

	i32Ret = pStrH[0] - 0x30;
	Wifi_wkup(FALSE);
	if(i32Ret == 0)
	{
		return 0;
	}
	else if(i32Ret == 1)
	{
		return 1;
	}
	
	return WIFI_GETAPINFO_FAIL;
}


UINT32 s_u32PingTime = 0;

int hal_espPing(char *pcIP, int iIpLen)
{
	char cSendBuff[128];


	sysLOG(HALESP8266_LOG_LEVEL_1, "\r\n");

	memset(cSendBuff, 0x00, sizeof(cSendBuff));
	
	Wifi_wkup(TRUE);
	fibo_mutex_try_lock(g_ui32ApiWiFiSendMutex, 5000);

	memcpy(cSendBuff, "AT+PING=\"", 9);
	memcpy(cSendBuff+9, pcIP, iIpLen);
	cSendBuff[9+iIpLen] ='\"';
	cSendBuff[10+iIpLen] ='\r';
	cSendBuff[11+iIpLen] ='\n';

	if(g_stWifiUart.wifibusy_flag==0)g_stWifiUart.wifibusy_flag=1;
	else 
	{
		sysLOG(HALESP8266_LOG_LEVEL_1, "wifibusy_flag:%d\r\n", g_stWifiUart.wifibusy_flag);
		Wifi_wkup(FALSE);
		fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
		return WIFI_SEND_CONFLICT_FAIL;
	}
	g_stWifiUart.CMD_read_P=g_stWifiUart.CMD_write_P=0;
	memset(g_stWifiUart.CMD_uart_buff,0,sizeof(g_stWifiUart.CMD_uart_buff));

	Wifi_send(cSendBuff, 12+iIpLen);

	s_u32PingTime = hal_sysGetTickms() + 10*1000;

	Wifi_wkup(FALSE);

	return 0;
}


int hal_getPingResult(void)
{
	int i32Ret = 0;
	int i32Ret1 = -1;

	i32Ret = dev_wifiWaitString("\r\nOK\r\n");
	if(i32Ret >= 0)
	{
		sysLOG(HALESP8266_LOG_LEVEL_1, "hal_getPingResult 0\r\n");
		i32Ret1 = 0;
		goto exit;
	}

	i32Ret = dev_wifiWaitString("+i32Timeout\r\n\r\nERROR\r\n");
	if(i32Ret >= 0)
	{
		sysLOG(HALESP8266_LOG_LEVEL_1, "hal_getPingResult 1\r\n");
		i32Ret1 = 1;
		goto exit;
	}
	
	if(hal_sysGetTickms() > s_u32PingTime)
	{
		sysLOG(HALESP8266_LOG_LEVEL_1, "Ping TimeOut\r\n");
		i32Ret1 = 1;
		goto exit;
	}
	//g_stWifiUart.wifibusy_flag=0;
	i32Ret1 = 2;
	goto exit;

exit:
	g_stWifiUart.wifibusy_flag = 0;
	fibo_mutex_unlock(g_ui32ApiWiFiSendMutex);
	return i32Ret1;
}


/*
* Description: 4G模块休眠通知WiFi的指令
* Input:       N
* Output:N
* Return:      -1:失败，0:成功
*/
int hal_espSleepNotice(void)
{
    #if WIFI_ATTYPE
	sysLOG(HALESP8266_LOG_LEVEL_1, "<SUCC>\r\n");
	return 0;
	#else
    char acBuf[100];
	int i32Ret;

	memset(acBuf, 0x00, sizeof(acBuf));
	Wifi_wkup(TRUE);

	
	i32Ret=ESP_SendAndWaitCmd("AT+VANSLEEP\r\n", strlen("AT+VANSLEEP\r\n"), acBuf, 100, 500, 2, "OK");
	if(i32Ret>=0)
	{
		Wifi_wkup(FALSE);
		sysLOG(HALESP8266_LOG_LEVEL_1, "<SUCC>\r\n");
		return 0;
	}
	else
	{
		Wifi_wkup(FALSE);
		sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR>\r\n");
		return -1;
	}
	#endif
}

/* Description: 启用/禁用多连接模式
* Input: N
* Output:N
* Return: <0:初始化失败
*/
int hal_espSetCIPMUX(char cMode)
{
    int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	Wifi_wkup(TRUE);

	unsigned char acBuf[100];

	memset(acCmd, 0x00, sizeof(acCmd));
	memcpy(acCmd, "AT+CIPMUX=", strlen("AT+CIPMUX="));
	i = strlen("AT+CIPMUX=");
	
	sprintf(&acCmd[i],"%d",cMode);
	i=strlen(acCmd);
	acCmd[i++] = '\r';
	acCmd[i++] = '\n';

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd(acCmd, i, (char *)acBuf, 100, 1000, 3, "\r\nOK\r\n");
	if(ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}
	
	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 设置Wi-Fi模式
* Input: 
* Output:N
* Return: <0:初始化失败
*/
int hal_espSetWifiMode(char cMode)
{
    int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	Wifi_wkup(TRUE);

	unsigned char acBuf[100];

	memset(acCmd, 0x00, sizeof(acCmd));
	memcpy(acCmd, "AT+CWMODE=", strlen("AT+CWMODE="));
	i = strlen("AT+CWMODE=");
	
	sprintf(&acCmd[i],"%d",cMode);
	i=strlen(acCmd);
	acCmd[i++] = '\r';
	acCmd[i++] = '\n';

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd(acCmd, i, (char *)acBuf, 100, 1000, 3, "\r\nOK\r\n");
	if(ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}
	
	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 查询Wi-Fi模式
* Input: 
* Output:N
* Return: <0:初始化失败
*/
int hal_espQueWifiMode(void)
{
    int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	Wifi_wkup(TRUE);

	unsigned char acBuf[100];

	memset(acCmd, 0x00, sizeof(acCmd));
	memcpy(acCmd, "AT+CWMODE?\r\n", strlen("AT+CWMODE?\r\n"));
	i = strlen("AT+CWMODE?\r\n");

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd(acCmd, i, (char *)acBuf, 100, 1000, 3, "\r\nOK\r\n");
	if(ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}
	
	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 启用WIFI睡眠模式
* Input: N
* Output:N
* Return: <0:初始化失败
*/
int hal_espSetSleep(void)
{
    int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	Wifi_wkup(TRUE);

	unsigned char acBuf[100];

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd("AT+SLEEPWKCFG=2,13,1\r\n", strlen("AT+SLEEPWKCFG=2,13,1\r\n"), (char *)acBuf, 100, 1000, 3, "\r\nOK\r\n");
	if (ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd("AT+SLEEP=2", strlen("AT+SLEEP=2"), (char *)acBuf, 100, 1000, 3, "\r\nOK\r\n");
	if (ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}
	
	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 查询SSL客户端配置
* Input:       i32Sockid:套接字ID；
* Output:N
* Return: <0:初始化失败
*/
int hal_espQuerySSLConfig(int i32Sockid)
{
    int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	
	char *pStrH=NULL;
	char *pStrT=NULL;
	unsigned char acBuf[128];

	Wifi_wkup(TRUE);
	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd("AT+CIPSSLCCONF?\r\n", strlen("AT+CIPSSLCCONF?\r\n"), (char *)acBuf, 128, 1000, 3, "\r\nOK\r\n");
	if (ret < 0)
	{
		Wifi_wkup(FALSE);
		return WIFI_SSL_GET_CONFIG_PARA_ERR;
	}
	
	pStrT = acBuf;
	for(i=0; i<5; i++)
	{
		pStrH = strstr(pStrT, "+CIPSSLCCONF:");
		if(pStrH == NULL)
		{
			Wifi_wkup(FALSE);
			return WIFI_SSL_GET_CONFIG_PARA_ERR;
		}
		pStrH += strlen("+CIPSSLCCONF:");
		if((pStrH[0] - '0') == i32Sockid)
		{
			Wifi_wkup(FALSE);
			return pStrH[2] - '0';
		}
		pStrT = pStrH;
	}

	Wifi_wkup(FALSE);
    return 0;
}

/*
* Description: 设置SSL客户端配置
* Input:       i32Sockid:套接字ID；mode:0:不认证；1：提供客户端证书；2：验证服务器；3：双向认证
* Output:N
* Return: <0:初始化失败 0：配置成功
*/
int hal_espSetSSLConfig(int i32Sockid, int mode)
{
    int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	Wifi_wkup(TRUE);

	unsigned char acBuf[100];
	memset(acCmd, 0x00, sizeof(acCmd));
	if(mode == 0)
	{
		sprintf(acCmd,"AT+CIPSSLCCONF=%d,%d\r\n", i32Sockid, mode);
	}
	else{
		sprintf(acCmd,"AT+CIPSSLCCONF=%d,%d,0,0\r\n", i32Sockid, mode);
	}
	i=strlen(acCmd);

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd(acCmd, i, (char *)acBuf, 100, 2000, 3, "\r\nOK\r\n");
	if (ret < 0)
	{
		Wifi_wkup(FALSE);
		return WIFI_SSL_CONFIG_ERR;
	}
	
	Wifi_wkup(FALSE);
    return 0;
}

int hal_wifiUpgradeSendData(uint8_t *data, int len, uint8_t flag)
{
	int ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	unsigned char acBuf[100];
	Wifi_wkup(TRUE);
	
	for(i=0; i<len; i++)
	{
		g_ui8Lrc ^= data[i];
	}
	
	memset(acCmd, 0x00, sizeof(acCmd));
	sprintf(acCmd,"AT+USERUPDATE=%d,%d,%d\r\n", len, g_ui8Lrc, flag);
	i=strlen(acCmd);
	if(flag == 1)
	{
		g_ui8Lrc = 0;
	}

	memset(acBuf, 0, sizeof(acBuf));
	ret = ESP_SendAndWaitCmd(acCmd, i, (char *)acBuf, 100, 10000, 3, "\r\nOK\r\n");
	if (ret < 0)
	{
		Wifi_wkup(FALSE);
		return ret;
	}
    sysLOG(HALESP8266_LOG_LEVEL_4, " sendLen=%d lrc=%d flag=%d\r\n", len, g_ui8Lrc, flag);
    if (len != 0)
    {
		memset(acBuf, 0, sizeof(acBuf));
		if(ESP_SendAndWaitCmd(data, len, acBuf, 100, 6000, 1,  "\r\nOK\r\n") < 0)
		{
			sysLOG(HALESP8266_LOG_LEVEL_2, " sendLen=%d lrc=%d flag=%d\r\n", len, g_ui8Lrc, flag);
			Wifi_wkup(FALSE);
			return ret;
		}
	}
	
	if(flag == 1)
	{
		sysLOG(HALESP8266_LOG_LEVEL_4, " sendLen=%d lrc=%d flag=%d\r\n", len, g_ui8Lrc, flag);
		sysDelayMs(500);
		wifiInit_lib();
	}
	Wifi_wkup(FALSE);
	return 0;
}

int hal_wifiWebNetwork(uint8_t *ssid,  uint8_t *password, unsigned int i32Timeout)
{
	char cRecvBuff[300];
	//char *pStrH = NULL;
	char sendbuf[128];
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char flag = 0;
	ST_AP_INFO *pstAp = &g_stApAsyInfo.stApinfo;
	Wifi_wkup(TRUE);
	g_cWifiWebNetworkState = 4;
	g_ui64WebNetworkTime = hal_sysGetTickms() + i32Timeout*1000;
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	/*i32Ret = ESP_SendAndWaitCmd("AT+RESTORE\r\n", strlen("AT+RESTORE\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		Wifi_wkup(FALSE);
		return WIFI_RESTORE_FAIL;
	}
	sysDelayMs(2000);*/
	/*i32Ret = ESP_SendAndWaitCmd("AT+CMD?\r\n", strlen("AT+CMD?\r\n"), (char *)cRecvBuff, 100, 10000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+SYSLOG=1\r\n", strlen("AT+SYSLOG=1\r\n"), cRecvBuff, sizeof(cRecvBuff), 3000, 1, "\r\nOK\r\n");
	if(i32Ret < 0)
	{		
		Wifi_wkup(FALSE);
		return WIFI_RESTORE_FAIL;
	}
	sysDelayMs(2000);
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));*/
	
	i32Ret = ESP_SendAndWaitCmd("AT+CWQAP\r\n", strlen("AT+CWQAP\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	memset(pstAp, 0x00, sizeof(ST_AP_INFO));
	g_stApAsyInfo.apErrFlag = WIFI_NOT_APCONNECT_ERR;
	
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWMODE=3\r\n", strlen("AT+CWMODE=3\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	//sysLOG(HALESP8266_LOG_LEVEL_2, "<ERR> AT+CWMODE i32Ret:%d\r\n", i32Ret);
	/*memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWMODE?\r\n", strlen("AT+CWMODE?\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	sysLOG(HALESP8266_LOG_LEVEL_1, "<ERR> AT+CWMODE i32Ret:%d\r\n", i32Ret);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP?\r\n", strlen("AT+CWDHCP?\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}*/

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWDHCP=0,2\r\n", strlen("AT+CWDHCP=0,2\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CIPAP=\"192.168.10.1\",\"192.168.10.1\",\"255.255.255.0\"\r\n", strlen("AT+CIPAP=\"192.168.10.1\",\"192.168.10.1\",\"255.255.255.0\"\r\n"), (char *)cRecvBuff, 100, 10000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	//sysLOG(HALESP8266_LOG_LEVEL_2, "<ERR> AT+CWMODE i32Ret:%d\r\n", i32Ret);

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	memset(sendbuf, 0x00, sizeof(sendbuf));
	//sprintf(sendbuf,"AT+CWSAP=\"%s\",\"%s\",11,3,3\r\n", ssid, password);
	sprintf(sendbuf,"AT+CWSAP=\"%s\",\"\",11,0,3\r\n", ssid);
	//sysLOG(HALESP8266_LOG_LEVEL_4, "<ERR> sendbuf:%s\r\n", sendbuf);
	i32Ret = ESP_SendAndWaitCmd(sendbuf, strlen(sendbuf), cRecvBuff, 100, 3000, 1, "OK");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CIPMUX=1\r\n", strlen("AT+CIPMUX=1\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}

	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+WEBSERVER=1,80,50\r\n", strlen("AT+WEBSERVER=1,80,25\r\n"), (char *)cRecvBuff, 100, 10000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}

    //Wifi_wkup(FALSE);
	//g_cWifiWebNetworkState = 4;
	g_cWaitWebUpdateState = 1;
	
	return 0;
}

int hal_wifiWebNetworkClose(void)
{
	char cRecvBuff[300];
	
	char sendbuf[128];
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char flag = 0;

	Wifi_wkup(TRUE);
    
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+WEBSERVER=0\r\n", strlen("AT+WEBSERVER=0\r\n"), (char *)cRecvBuff, 100, 5000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}

	sysDelayMs(3000);
    memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
    i32Ret = ESP_SendAndWaitCmd("AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
    if(i32Ret < 0)
    {
        Wifi_wkup(FALSE);
        return i32Ret;
    }
    Wifi_wkup(FALSE);
	return 0;
}

int hal_wifiWebNetworkQue(char cancelFlag)
{
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char cnt = 0;
	char cRecvBuff[300];
	ST_AP_INFO *pstAp = &g_stApAsyInfo.stApinfo;
	while(cnt < 2)
	{
		if(g_cWifiWebNetworkState == 4)
		{
			i32Ret = dev_wifiWaitString("+WEBSERVERRSP:1");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("+WEBSERVERRSP:1\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				//ui32Time = hal_sysGetTickms() + i32Timeout;
				g_cWifiWebNetworkState = 0;
				sysLOG(HALESP8266_LOG_LEVEL_1, " WEBSERVERRSP:1 i32Ret:%d\r\n", i32Ret);
			}
		}
		else if(g_cWifiWebNetworkState == 0)
		{
			i32Ret = dev_wifiWaitString("WIFI CONNECTED");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("WIFI CONNECTED\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				g_cWifiWebNetworkState++;
				sysLOG(HALESP8266_LOG_LEVEL_1, " WIFI CONNECTED i32Ret:%d\r\n", i32Ret);
			}
		}
		else if(g_cWifiWebNetworkState == 1)
		{
			i32Ret = dev_wifiWaitString("WIFI GOT IP");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("WIFI GOT IP\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				//g_cWifiWebNetworkState++;
				sysLOG(HALESP8266_LOG_LEVEL_1, " WIFI GOT IP i32Ret:%d\r\n", i32Ret);
			}

			i32Ret = dev_wifiWaitString("+WEBSERVERRSP:2");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("+WEBSERVERRSP:2\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_1, " WEBSERVERRSP:2 i32Ret:%d\r\n", i32Ret);
				sysDelayMs(500);
				g_cWifiWebNetworkState=4;
				hal_wifiWebNetworkClose();
				memset(pstAp, 0x00, sizeof(ST_AP_INFO));
				g_stApAsyInfo.apErrFlag = hal_espCheckAp(pstAp);
				return 2;
			}
		}
		/*else if(g_cWifiWebNetworkState == 2)
		{
			 i32Ret = dev_wifiWaitString("+WEBSERVERRSP:2");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("+WEBSERVERRSP:2\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_1, " WEBSERVERRSP:2 i32Ret:%d\r\n", i32Ret);
				sysDelayMs(500);
				g_cWifiWebNetworkState++;
				hal_wifiWebNetworkClose();
				break;
			}
		}*/
		cnt++;

		if(cancelFlag == 1)
		{
			g_cWifiWebNetworkState = 4;
            //wifiInit_lib();
			hal_wifiWebNetworkClose();
			//sysDelayMs(1000);
			return 0;
		}
		
		if(hal_sysGetTickms() >= g_ui64WebNetworkTime)
		{
			g_cWifiWebNetworkState = 4;
			hal_wifiWebNetworkClose();
			return 3;
		}

		sysDelayMs(50);
	}
	return g_cWifiWebNetworkState;
}

int hal_wifiWebUpdateQue(char cancelFlag)
{
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char cnt = 0;
	char ucKey = 0;
	char cRecvBuff[300];

	while(cnt < 2)
	{
		//sysLOG(HALESP8266_LOG_LEVEL_1, " g_cWaitWebUpdateState:%d\r\n", g_cWaitWebUpdateState);
		if(g_cWaitWebUpdateState == 1)
		{                              
			i32Ret = dev_wifiWaitString("+WEBSERVERRSP:3");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("+WEBSERVERRSP:3\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				//ui32Time = hal_sysGetTickms() + i32Timeout;
				g_cWaitWebUpdateState++;
				sysLOG(HALESP8266_LOG_LEVEL_1, " WEBSERVERRSP:3 i32Ret:%d\r\n", i32Ret);
			}
		}
		else if(g_cWaitWebUpdateState == 2)
		{
			i32Ret = dev_wifiWaitString("+WEBSERVERRSP:4");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("+WEBSERVERRSP:4\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_1, " WEBSERVERRSP:4 i32Ret:%d\r\n", i32Ret);
				g_cWaitWebUpdateState++;
				hal_wifiWebNetworkClose();
				break;
			}

			i32Ret = dev_wifiWaitString("+WEBSERVERRSP:5");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("+WEBSERVERRSP:5\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_1, " WEBSERVERRSP:5 i32Ret:%d\r\n", i32Ret);
				g_cWaitWebUpdateState = -1;
				hal_wifiWebNetworkClose();
				break;
			}
		}
		cnt++;

		if(cancelFlag == 1)
		{
			g_cWaitWebUpdateState = 0;
            //wifiInit_lib();
			hal_wifiWebNetworkClose();
			//sysDelayMs(1000);
			return 0;
		}
		sysDelayMs(50);
	}
	return g_cWaitWebUpdateState;
}

int hal_wifiAirkissNetwork(unsigned int i32Timeout)
{
	char cRecvBuff[300];
	
	char sendbuf[128];
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char flag = 0;
	ST_AP_INFO *pstAp = &g_stApAsyInfo.stApinfo;
	Wifi_wkup(TRUE);
	//g_cWifiAirkissNetState = 0;
	
	g_ui64AirkissNetworkTime = hal_sysGetTickms() + i32Timeout*1000;
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWQAP\r\n", strlen("AT+CWQAP\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		sysLOG(API_LOG_LEVEL_0, "  wifiAirkissNetwork=%d\r\n",i32Ret);
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	
	memset(pstAp, 0x00, sizeof(ST_AP_INFO));
	g_stApAsyInfo.apErrFlag = WIFI_NOT_APCONNECT_ERR;
	
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWMODE=1\r\n", strlen("AT+CWMODE=1\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		sysLOG(API_LOG_LEVEL_0, "  wifiAirkissNetwork=%d\r\n",i32Ret);
		Wifi_wkup(FALSE);
		return i32Ret;
	}
    
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWSTARTSMART=2\r\n", strlen("AT+CWSTARTSMART=2\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		sysLOG(API_LOG_LEVEL_0, "  wifiAirkissNetwork=%d\r\n",i32Ret);
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	//Wifi_wkup(FALSE);
	g_cWifiAirkissNetState = 4;
	//sysDelayMs(300);
	return 0;
}

int hal_wifiAirkissNetworkClose(void)
{
	char cRecvBuff[300];
	
	char sendbuf[128];
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char flag = 0;

	Wifi_wkup(TRUE);
    
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	i32Ret = ESP_SendAndWaitCmd("AT+CWSTOPSMART\r\n", strlen("AT+CWSTOPSMART\r\n"), (char *)cRecvBuff, 100, 1000, 3, "\r\nOK\r\n");
	if(i32Ret < 0)
	{
		Wifi_wkup(FALSE);
		return i32Ret;
	}
	Wifi_wkup(FALSE);
	return 0;
}

int hal_wifiAirkissNetworkQue(char cancelFlag)
{
	int i = 0;
	int i32Ret = -1;
	unsigned int ui32Time=0;
	char flag = 0;
	char cRecvBuff[300];
	char cnt = 0;
	ST_AP_INFO *pstAp = &g_stApAsyInfo.stApinfo;
	while(cnt < 2)
	{
		if(g_cWifiAirkissNetState == 4)
		{
			i32Ret = dev_wifiWaitString("Smart get wifi info");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("Smart get wifi info\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				//ui32Time = hal_sysGetTickms() + i32Timeout;
				g_cWifiAirkissNetState = 0;
				sysLOG(HALESP8266_LOG_LEVEL_1, " Smart get wifi info i32Ret:%d\r\n", i32Ret);
			}
		}
		else if(g_cWifiAirkissNetState == 0)
		{
			i32Ret = dev_wifiWaitString("WIFI CONNECTED");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("WIFI CONNECTED\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				g_cWifiAirkissNetState++;
				sysLOG(HALESP8266_LOG_LEVEL_1, " WIFI CONNECTED i32Ret:%d\r\n", i32Ret);
			}
		}
		else if(g_cWifiAirkissNetState == 1)
		{
			i32Ret = dev_wifiWaitString("WIFI GOT IP");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("WIFI GOT IP\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				//g_cWifiAirkissNetState++;
				sysLOG(HALESP8266_LOG_LEVEL_1, " WIFI GOT IP i32Ret:%d\r\n", i32Ret);
			}

			i32Ret = dev_wifiWaitString("smartconfig connected wifi");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("smartconfig connected wifi\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_1, " smartconfig connected wifi i32Ret:%d\r\n", i32Ret);
				g_cWifiAirkissNetState = 4;
				sysDelayMs(6000);
				i32Ret = hal_wifiAirkissNetworkClose();
				if(i32Ret < 0)
				{
					return i32Ret;
				}
				memset(pstAp, 0x00, sizeof(ST_AP_INFO));
				g_stApAsyInfo.apErrFlag = hal_espCheckAp(pstAp);
				return 2;
			}
		}
		/*else if(g_cWifiAirkissNetState == 4)
		{
			i32Ret = dev_wifiWaitString("smartconfig connected wifi");
			if(i32Ret >= 0)
			{
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				i32Ret += strlen("smartconfig connected wifi\r\n");
				dev_wifiReadString(cRecvBuff, sizeof(cRecvBuff), i32Ret);
				sysLOG(HALESP8266_LOG_LEVEL_1, " smartconfig connected wifi i32Ret:%d\r\n", i32Ret);
				g_cWifiAirkissNetState = 4;
				sysDelayMs(6000);
				i32Ret = hal_wifiAirkissNetworkClose();
				if(i32Ret < 0)
				{
					return i32Ret;
				}
				break;
			}
		}*/
		
		if(cancelFlag == 1)
		{
			g_cWifiWebNetworkState = 4;
			i32Ret = hal_wifiAirkissNetworkClose();
			if(i32Ret < 0)
            {
                return i32Ret;
            }
            //wifiInit_lib();
			return 0;
		}
		
		if(hal_sysGetTickms() >= g_ui64AirkissNetworkTime)
		{
			g_cWifiWebNetworkState = 4;
			i32Ret = hal_wifiAirkissNetworkClose();
			if(i32Ret < 0)
            {
                return i32Ret;
            }
			return 3;
		}
		cnt++;
		sysDelayMs(50);
	}
	return g_cWifiAirkissNetState;
}

int hal_WifiLocalDown(char *IP, int len)
{
	char cSendBuff[128];
	char cRecvBuff[2048];

	int i32Ret = -1;
	int i = 0;
	Wifi_wkup(TRUE);
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
	memset(cSendBuff, 0x00, sizeof(cSendBuff));
	i = strlen("AT+CIUPDATE=1,1,\"");
	memcpy(cSendBuff, "AT+CIUPDATE=1,1,\"", i);
	memcpy(cSendBuff+i, IP, len);
	i += len;
	memcpy(cSendBuff+i, "\",\"/csms/AppFile/user.bin\"\r\n", strlen("\",\"/csms/AppFile/user.bin\"\r\n"));
	i += strlen("\",\"/csms/AppFile/user.bin\"\r\n");

	i32Ret = ESP_SendAndWaitCmd(cSendBuff, i, cRecvBuff, sizeof(cRecvBuff), 80000, 1, "device upgrade success\r\n");
	if(i32Ret < 0)
	{		
		Wifi_wkup(FALSE);
		return WIFI_LOCAL_UPDATE_FAIL;
	}

	sysDelayMs(3000);
	wifiInit_lib();
	Wifi_wkup(FALSE);
	return 0;
}

int hal_espDownload(char *pcName, char *pcData, int iLen)
{
	int i32Ret = -1;
	int i = 0;
	unsigned int ui32Time=0;
	char acCmd[128];
	unsigned char acBuf[200];
	Wifi_wkup(TRUE);

    memset(acCmd, 0x00, sizeof(acCmd));
    memset(acBuf, 0x00, sizeof(acBuf));
    sprintf(acCmd, "AT+SYSFLASH=0,\"%s\"\r\n", pcName);
    i = strlen(acCmd);
    i32Ret = ESP_SendAndWaitCmd(acCmd, i, acBuf, sizeof(acBuf), 5000, 1, "\r\nOK\r\n");
    if (i32Ret < 0)
    {
		sysLOG(HALESP8266_LOG_LEVEL_2, " i32Ret=%d\r\n", i32Ret);
        Wifi_wkup(FALSE);
        return -1;
    }

    memset(acCmd, 0x00, sizeof(acCmd));
    memset(acBuf, 0x00, sizeof(acBuf));
    sprintf(acCmd, "AT+SYSFLASH=1,\"%s\",0,%d\r\n", pcName, iLen);
    i = strlen(acCmd);
    i32Ret = ESP_SendAndWaitCmd(acCmd, i, acBuf, sizeof(acBuf), 5000, 1, "\r\n>");
    if (i32Ret < 0)
    {
		sysLOG(HALESP8266_LOG_LEVEL_2, " i32Ret=%d\r\n", i32Ret);
        Wifi_wkup(FALSE);
        return -2;
    }

    memset(acBuf, 0, sizeof(acBuf));
    if (ESP_SendAndWaitCmd(pcData, iLen, acBuf, 100, 6000, 1, "\r\nOK\r\n") < 0)
    {
		sysLOG(HALESP8266_LOG_LEVEL_2, " i32Ret=%d\r\n", i32Ret);
        Wifi_wkup(FALSE);
        return -3;
    }
    Wifi_wkup(FALSE);
	return 0;
}

int hal_espDownloadCrt(char cType, char *pcData, int iLen)
{
	int i32Ret = -1;
	int i = 0;
	char *pcName[20];
	memset(pcName, 0x00, sizeof(pcName));

	if((cType>5) || (pcData == NULL) || (iLen > 8192))
	{
		return WIFI_CRT_DOWNLOAD_PARA_ERR;
	}

	switch(cType)
	{
		case 0:
			memcpy(pcName, "client_ca", strlen("client_ca"));
			i32Ret = hal_espDownload(pcName, pcData, iLen);
			if(i32Ret < 0)
			{
				return WIFI_CA_CRT_DOWNLOAD_ERR;
			}
			break;
		case 1:
			memcpy(pcName, "client_cert", strlen("client_cert"));
			i32Ret = hal_espDownload(pcName, pcData, iLen);
			if(i32Ret < 0)
			{
				return WIFI_CLIENT_CRT_DOWNLOAD_ERR;
			}
			break;   
		case 2:
			memcpy(pcName, "client_key", strlen("client_key"));
			i32Ret = hal_espDownload(pcName, pcData, iLen);
			if(i32Ret < 0)
			{
				return WIFI_CLIENT_KEY_DOWNLOAD_ERR;
			}
			break;
		case 3:
			memcpy(pcName, "mqtt_ca", strlen("mqtt_ca"));
			i32Ret = hal_espDownload(pcName, pcData, iLen);
			if(i32Ret < 0)
			{
				return WIFI_MQTT_CA_CRT_DOWNLOAD_ERR;
			}
			break;
		case 4:
			memcpy(pcName, "mqtt_cert", strlen("mqtt_cert"));
			i32Ret = hal_espDownload(pcName, pcData, iLen);
			if(i32Ret < 0)
			{
				return WIFI_MQTT_CRT_DOWNLOAD_ERR;
			}
			break;
		case 5:
			memcpy(pcName, "mqtt_key", strlen("mqtt_key"));
			i32Ret = hal_espDownload(pcName, pcData, iLen);
			if(i32Ret < 0)
			{
				return WIFI_MQTT_KEY_DOWNLOAD_ERR;
			}
			break;
		default:
			return WIFI_CRT_DOWNLOAD_PARA_ERR;
	}
	return 0;
}

