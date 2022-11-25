
#include "comm.h"



int8 cldsd_closemode(void);

/*******************************************************************************
 ** External Function Declarations
 ******************************************************************************/
 int dev_cpydata_IPD(void);


struct UART_S g_stWifiUart;
struct WIFI_CONNECT g_stWifiConnect;
struct CLOSED_S g_staClosedBuff[100];


uint8 g_ui8CldsdClosemodeFlag=0;//溢出后发送CIPCLOSEMODE标志位，发送完毕则置为0。
volatile uint8 g_ui8WifiConnectClosed=0;//终端主动挂断通信
uint32 g_ui32IPDMutexHandle = 0;
int g_i32UartExter = 0;
int g_i32UartExit = 0;

uint32 APTimecount=0;//发完指令后的滴答时钟计时
uint32 APtypetimeout=0;//记录的应用层传递的超时时间

void DR_IPDmutex_lock(UINT32 mutex_id)
{
	//fibo_mutex_lock(mutex_id);
}

void DR_IPDmutex_unlock(UINT32 mutex_id)
{
	//fibo_mutex_unlock(mutex_id);
}



int32 DR_Wifi_Pwr_EN(BOOL pwr_v)
{
	int iRet = 0;
		
	iRet = fibo_gpio_set(WIFI_PWR_EN_GPIO, pwr_v);
	
	return iRet;
	
}

void DR_Wifi_Pwr_init(void)
{
	
	
	fibo_gpio_mode_set(WIFI_WKUP_GPIO, GpioFunction1);
	fibo_gpio_cfg(WIFI_WKUP_GPIO, GpioCfgOut);
	fibo_gpio_set(WIFI_WKUP_GPIO, false);

	if (g_iType_machine == 0)	//机器类型Q390
	{
		fibo_gpio_mode_set(WIFI_PWR_EN_GPIO, GpioFunction1);
		fibo_gpio_cfg(WIFI_PWR_EN_GPIO, GpioCfgOut);
		fibo_gpio_set(WIFI_PWR_EN_GPIO, false);
	}
	else if (g_iType_machine == 1)	//机器类型Q161
	{
		fibo_gpio_mode_set(WIFI_PWR_EN_GPIO, GpioFunction4);
		fibo_gpio_cfg(WIFI_PWR_EN_GPIO, GpioCfgOut);
		fibo_gpio_set(WIFI_PWR_EN_GPIO, false);
	}
	
	
	
}

void hal_WiFiUartRecvCB(hal_uart_port_t uart_port, UINT8 *data, UINT16 len, void *arg)
{
	BOOL ret = FALSE;
	uint32 uartcb_templen;
	uint8 *uartcb_tempdata;
	
#if 1
	if(uart_port == WIFI_PORT)
	{
		if(g_i32UartExit != g_i32UartExter)
		{
			//scrPrint_lib(0, 6, 0, "Exit=%d,Exter=%d",g_i32UartExit,g_i32UartExter);		
			sysLOG(WIFI_LOG_LEVEL_4, "Exit:%d, Exter:%d\r\n",g_i32UartExit,g_i32UartExter);
		}
		g_i32UartExter++;
		sysLOG(WIFI_LOG_LEVEL_4, "Exit:%d, Exter:%d\r\n",g_i32UartExit,g_i32UartExter);
		if(g_ui8WifiConnectClosed==0)     //没有关闭wifi
		{
			//sysLOG(WIFI_LOG_LEVEL_4, "len:%d, %s\r\n", len, data);
			if(g_ui8CldsdClosemodeFlag==0)//数据没有溢出
			{
				sysLOG(WIFI_LOG_LEVEL_4, "g_ui8CldsdClosemodeFlag:%d\r\n", g_ui8CldsdClosemodeFlag);
				if((g_stWifiUart.uart_buff_count+len)<UARTBUFF_LEN)//数据没有溢出
				{
					uartcb_templen=len;
					uartcb_tempdata=data;
					if(g_stWifiUart.invailddatalen!=0)//后续有需要丢掉的数据
					{
						sysLOG(WIFI_LOG_LEVEL_4, "g_stWifiUart.invailddatalen:%d \r\n", g_stWifiUart.invailddatalen);
						if(uartcb_templen<=g_stWifiUart.invailddatalen)
						{
							g_stWifiUart.invailddatalen-=uartcb_templen;
							uartcb_templen=0;
						}
						else
						{				
							uartcb_templen-=g_stWifiUart.invailddatalen;
							uartcb_tempdata+=g_stWifiUart.invailddatalen;
							g_stWifiUart.invailddatalen=0;
						}
					}
					
				    sysLOG(WIFI_LOG_LEVEL_4, "uartcb_templen:%d\r\n", uartcb_templen);
					if(uartcb_templen!=0)
					{
						if((UARTBUFF_LEN-g_stWifiUart.write_P)<=uartcb_templen)
						{
							sysLOG(WIFI_LOG_LEVEL_4, "uartcb_templen:%d g_stWifiUart.write_P:%d\r\n", uartcb_templen,g_stWifiUart.write_P);
							memcpy(&g_stWifiUart.uart_buff[g_stWifiUart.write_P],(const void *)uartcb_tempdata,UARTBUFF_LEN-g_stWifiUart.write_P);
							memcpy(&g_stWifiUart.uart_buff[0],(const void *)(uartcb_tempdata+(UARTBUFF_LEN-g_stWifiUart.write_P)),uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P));
							//sysLOG(WIFI_LOG_LEVEL_4, "uartcb_templen:%d, 1uartrecvCB:%s\r\n", uartcb_templen, &g_stWifiUart.uart_buff[g_stWifiUart.write_P]);
							//sysLOG(WIFI_LOG_LEVEL_4, "1uartrecvCB:%s\r\n", &g_stWifiUart.uart_buff[0]);
							g_stWifiUart.write_P=uartcb_templen-(UARTBUFF_LEN-g_stWifiUart.write_P);
							g_stWifiUart.uart_buff_count+=uartcb_templen;
						}
						else
						{
							sysLOG(WIFI_LOG_LEVEL_4, "uartcb_templen:%d g_stWifiUart.write_P:%d\r\n", uartcb_templen,g_stWifiUart.write_P);
							memcpy(&g_stWifiUart.uart_buff[g_stWifiUart.write_P],(const void *)uartcb_tempdata,uartcb_templen);
							//sysLOG(WIFI_LOG_LEVEL_4, "uartcb_templen:%d, uartrecvCB:%s\r\n", uartcb_templen, &g_stWifiUart.uart_buff[g_stWifiUart.write_P]);
							g_stWifiUart.write_P+=uartcb_templen;
							g_stWifiUart.uart_buff_count+=uartcb_templen;
						}
						
						//fibo_sem_signal(Sem_Uart2CB_signal);
						dev_cpydata_IPD();
						
					}
				}
				else
				{
					g_ui8CldsdClosemodeFlag=1;
					sysLOG(WIFI_LOG_LEVEL_4, "g_ui8CldsdClosemodeFlag:%d\r\n", g_ui8CldsdClosemodeFlag);
				}

			}
			else
			{
				//g_ui8CldsdClosemodeFlag=0;
			}
		}

		ret = TRUE;
		g_i32UartExit++;
	}
#endif
}



#if WIFI_ATTYPE


/*
*@brief:检测到了缓存满了后，发送AT+CIPCLOSEMODE=0,1\r\n，设置abort强制断开，这样应用掉用close时，能及时响应
*@return:-1-失败；0-成功
*/
int8 cldsd_closemode(void)
{
	//unsigned char buf[100];
    //int ret = -1;

	Wifi_wkup(TRUE);
	if(g_cWifiVersionFlag == NOSDKVERSION)
	{
		//memset(buf, 0, sizeof(buf));
		sysLOG(WIFI_LOG_LEVEL_1, "start send AT+CIPCLOSEMODE\r\n");
		Wifi_send("AT+CIPCLOSEMODE=1\r\n",strlen("AT+CIPCLOSEMODE=1\r\n"));
	}
	
	Wifi_wkup(FALSE);
	return 0;
}

#endif
void delay_ms(int ms)
{
	unsigned long long uiTime;
	
	uiTime = hal_sysGetTickms() + ms;
	while(hal_sysGetTickms() < uiTime);
}
void Wifi_send(char *data, UINT32 dataLen)
{
	fibo_hal_uart_put(WIFI_PORT, data, dataLen);	
	delay_ms(3);
}

/*
*@brief:控制蓝牙电源
*@param1:TRUE-上电；FALSE-下电
*@return:TRUE-已上电，FALSE-已下电, <0-失败
*/
int8 DR_BTPwrCtl(BOOL Pwrvalue)
{
#if 0//scanoff
	int8 ret=-1;
	uint8 btpwrtmp;
	fibo_gpio_set(BTPWR_GPIO, Pwrvalue);
	ret = fibo_gpio_get(BTPWR_GPIO, &btpwrtmp);
	if(ret < 0)
	{
		sysLOG(LED_LOG_LEVEL_2, "<ERR> fibo_gpio_get ret:%d\r\n", ret);
		return ret;
	}
	sysLOG(WIFI_LOG_LEVEL_2, "btpwrtmp:%d\r\n", btpwrtmp);
	return btpwrtmp;
#endif
}

void wifiChangeBrand()
{
	uint8_t buf[64], sendbuf[64];
	int ret;
	int port = WIFI_PORT;
	int iLimit;
	hal_uart_config_t uart2_drvcfg = {0};

	/*修改波特率为*/
	memset(buf, 0, sizeof(buf));
	memset(sendbuf, 0, sizeof(sendbuf));
	if((g_cWifiVersionFlag == NOSDKVERSION) && (WIFI_BAUDSPEED >  460800))
	{
		sprintf(sendbuf, "AT+UART_CUR=%d,8,1,0,0\r\n", 460800);
	}
	else{
		sprintf(sendbuf, "AT+UART_CUR=%d,8,1,0,0\r\n", WIFI_BAUDSPEED);
	}
	
	ret = ESP_SendAndWaitCmd(sendbuf, strlen(sendbuf), (char *)buf, 100, 500, 3, "\r\nOK\r\n");
	
    uart2_drvcfg.baud = WIFI_BAUDSPEED;
    uart2_drvcfg.parity = HAL_UART_NO_PARITY;
    uart2_drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    uart2_drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    uart2_drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
    uart2_drvcfg.tx_buf_size = UART_TX_BUF_SIZE;
	uart2_drvcfg.recv_timeout = 1;


    fibo_hal_uart_init(port, &uart2_drvcfg, hal_WiFiUartRecvCB, NULL);	

	while(1)
	{
	/*测试AT启动*/	
		sysLOG(WIFI_LOG_LEVEL_1, "ESP_Cmd_Init is opened!!\r\n");
		memset(buf, 0, sizeof(buf));

		ret = ESP_SendAndWaitCmd("AT\r\n", strlen("AT\r\n"), (char *)buf, 100, 500, 1, "\r\nOK\r\n");
		if(ret < 0)
		{
			iLimit++;
			if(iLimit >= 3)
			{
				Wifi_wkup(FALSE);
				return ret;
			}
		}

		break;
	}
}

void Wifi_init(uint32_t baudrate)
{
	
//	fibo_gpio_mode_set(WIFI_TXD_GPIO, GpioFunction1);
//	fibo_gpio_mode_set(WIFI_RXD_GPIO, GpioFunction1);

	int port = WIFI_PORT;
	hal_uart_config_t uart2_drvcfg = {0};
	
    uart2_drvcfg.baud = baudrate;
    uart2_drvcfg.parity = HAL_UART_NO_PARITY;
    uart2_drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    uart2_drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    uart2_drvcfg.rx_buf_size = UART_RX_BUF_SIZE;
    uart2_drvcfg.tx_buf_size = UART_TX_BUF_SIZE;
	uart2_drvcfg.recv_timeout = 1;

    fibo_hal_uart_init(port, &uart2_drvcfg, hal_WiFiUartRecvCB, NULL);
	
	//fibo_hal_uart_put(port, uart2_send_testbuf, sizeof(uart2_send_testbuf) - 1);


	fibo_gpio_mode_set(WIFI_WKUP_GPIO, GpioFunction1);
	fibo_gpio_cfg(WIFI_WKUP_GPIO, GpioCfgOut);
	fibo_gpio_set(WIFI_WKUP_GPIO, TRUE);

	DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
	sysLOG(WIFI_LOG_LEVEL_1, "<SUCC> wifi uart2 init ok!\r\n");	
}

extern int g_wifiAutoSleepMode;

int wifi_setWkupTrue()
{
	fibo_gpio_set(WIFI_WKUP_GPIO, TRUE);
	sysDelayMs(10);
	return 0;
}

BOOL Wifi_wkup(BOOL wkup_V)
{
	BOOL ret;

	#if 1
	int8 io_tmp;
	uint32 uTime;
	if(g_wifiAutoSleepMode == 0)
	{
		//fibo_gpio_set(WIFI_WKUP_GPIO, TRUE);
	}
	else
	{
		fibo_gpio_set(WIFI_WKUP_GPIO, wkup_V);
		
		uTime = hal_sysGetTickms();
		if(TRUE == wkup_V)
		{
			//sysDelayMs(10);
			do
			{
				
			}while(hal_sysGetTickms()<(uTime+10));
		}

	}

	fibo_gpio_get(WIFI_WKUP_GPIO, &io_tmp);
	sysLOG(WIFI_LOG_LEVEL_4, "WIFI_WKUP io_tmp:%d\r\n", io_tmp);

	return io_tmp;
	#endif
}

BOOL Wifi_wkup_ex(BOOL wkup_V)
{
	BOOL ret;
	int8 io_tmp;
	uint32 uTime;
	
	fibo_gpio_set(WIFI_WKUP_GPIO, wkup_V);
	uTime = hal_sysGetTickms();
	if(TRUE == wkup_V)
	{
		//sysDelayMs(10);
		do
		{
			
		}while(hal_sysGetTickms()<(uTime+10));
	}
	fibo_gpio_get(WIFI_WKUP_GPIO, &io_tmp);
	sysLOG(WIFI_LOG_LEVEL_4, "WIFI_WKUP io_tmp:%d\r\n", io_tmp);

	return io_tmp;
}


struct MYSTR_TEMP_S{
	int8 buf[100];
	uint8 read_P;
	uint8 write_P;
	
};

struct MYSTR_TEMP_S mystr_temp_s;

char* myStrStr(char *pcSrc, char *pcDes, uint32 readP, uint32 writeP)
{
 char *pcStrH = 0;
 const char *bp = NULL;
 const char *sp = NULL;
 int iLen = 0;
 int iSrcLen = 0;
 int iSubLen = strlen(pcDes);
 
 if(0 == iSubLen)
 {
  return NULL;
 }

 iSrcLen = writeP - readP;
 if((iSrcLen <= 0) || (iSrcLen < iSubLen))
 {
  return NULL;
 }

 pcStrH = pcSrc;

 while(1)
 {
  bp = pcStrH;
  
  sp = pcDes;
 
  do
  {
   if(!*sp)
    return pcStrH;
  }while(*bp++==*sp++);
  
  pcStrH++;
  iLen++;

  iSrcLen = writeP - readP;
  if((iLen >= iSrcLen) || ((iSrcLen - iLen) < iSubLen))
  {
   return NULL;
  }
 }

 return NULL;
}

#if 0
char* myStrStr_IPD(char *pcSrc, char *pcDes)
{
 char *pcStrH = 0;
 const char *bp = NULL;
 const char *sp = NULL;
 int iLen = 0;
 int iSrcLen = 0;
 int iSubLen = strlen(pcDes);
 
 if(0 == iSubLen)
 {
  return NULL;
 }

 iSrcLen = g_stWifiUart.IPD_write_P - g_stWifiUart.IPD_read_P;
 if((iSrcLen <= 0) || (iSrcLen < iSubLen))
 {
  return NULL;
 }

 pcStrH = pcSrc;

 while(1)
 {
  bp = pcStrH;
  
  sp = pcDes;
 
  do
  {
   if(!*sp)
    return pcStrH;
  }while(*bp++==*sp++);
  
  pcStrH++;
  iLen++;

  iSrcLen = g_stWifiUart.IPD_write_P - g_stWifiUart.IPD_read_P;
  if((iLen >= iSrcLen) || ((iSrcLen - iLen) < iSubLen))
  {
   return NULL;
  }
 }

 return NULL;
}

char* myStrStr_CMD(char *pcSrc, char *pcDes)
{
 char *pcStrH = 0;
 const char *bp = NULL;
 const char *sp = NULL;
 int iLen = 0;
 int iSrcLen = 0;
 int iSubLen = strlen(pcDes);
 
 if(0 == iSubLen)
 {
  return NULL;
 }

 iSrcLen = g_stWifiUart.CMD_write_P - g_stWifiUart.CMD_read_P;
 if((iSrcLen <= 0) || (iSrcLen < iSubLen))
 {
  return NULL;
 }

 pcStrH = pcSrc;

 while(1)
 {
  bp = pcStrH;
  
  sp = pcDes;
 
  do
  {
   if(!*sp)
    return pcStrH;
  }while(*bp++==*sp++);
  
  pcStrH++;
  iLen++;

  iSrcLen = g_stWifiUart.CMD_write_P - g_stWifiUart.CMD_read_P;
  if((iLen >= iSrcLen) || ((iSrcLen - iLen) < iSubLen))
  {
   return NULL;
  }
 }

 return NULL;
}



char* myStrStr_temp(char *pcSrc, char *pcDes)
{
 char *pcStrH = 0;
 const char *bp = NULL;
 const char *sp = NULL;
 int iLen = 0;
 int iSrcLen = 0;
 int iSubLen = strlen(pcDes);
 
 if(0 == iSubLen)
 {
  return NULL;
 }

 iSrcLen = mystr_temp_s.write_P - mystr_temp_s.read_P;
 if((iSrcLen <= 0) || (iSrcLen < iSubLen))
 {
  return NULL;
 }

 pcStrH = pcSrc;

 while(1)
 {
  bp = pcStrH;
  
  sp = pcDes;
 
  do
  {
   if(!*sp)
    return pcStrH;
  }while(*bp++==*sp++);
  
  pcStrH++;
  iLen++;

  iSrcLen = mystr_temp_s.write_P - mystr_temp_s.read_P;
  if((iLen >= iSrcLen) || ((iSrcLen - iLen) < iSubLen))
  {
   return NULL;
  }
 }

 return NULL;
}
#endif

uint8 *strstr_cmd(uint8 *source, uint8 *key,int len)
{
	uint8 *rP=NULL;
	int len_offset=0,len_temp=0;
	uint8 r_offset=0;

	while(1)
	{
		
		r_offset=memcmp(source+len_offset,"\0",1);
		//rP=strstr(source+len_offset,"\0");
		
		if(r_offset==0)
		{
			
			if(len_offset<len)
			{
				rP=strstr(source+len_temp,key);
				if(rP!=NULL)
				{
					break;	
				}
				else
				{
					len_offset=len_offset+1;
					//rP=NULL;
					r_offset=0;

				
				}
				
			}
			else
			{
				rP=strstr(source+len_temp,key);
				if(rP!=NULL)
				{
					break;	
				}
				break;
			}
			len_temp=len_offset;
			sysLOG(WIFI_LOG_LEVEL_1, "len_temp:%d,len:%d\r\n", len_temp, len);
		}
		else
		{
			len_offset+=1;
			//break;
		}
	}
	
	return rP;
}





uint32 readptemp_cmd;

int dev_wifiWaitString(char *cmd)
{
	int ret=-1;
	int8 *rP=NULL;
	if(g_stWifiUart.CMD_read_P==g_stWifiUart.CMD_write_P)//以供收到数据后进行第二次匹配cmd使用
	{
	
		
	}
	else if(g_stWifiUart.CMD_read_P<g_stWifiUart.CMD_write_P)
	{
		//sysDelayMs(10);
		
		rP=myStrStr(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P],cmd,g_stWifiUart.CMD_read_P,g_stWifiUart.CMD_write_P);
		//rP=strstr_cmd(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],cmd,(g_stWifiUart.write_P-g_stWifiUart.read_P));//应该判断一下是否正确，
		if(rP!=NULL)
		{
			readptemp_cmd=(rP-&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P]);
			
			//g_stWifiUart.uart_buff_len=g_stWifiUart.CMD_write_P-g_stWifiUart.CMD_read_P;	
			sysLOG(WIFI_LOG_LEVEL_4, "rP:%u,&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P]:%u,g_stWifiUart.CMD_read_P:%d\r\n", rP, &g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P], g_stWifiUart.CMD_read_P);
			ret=(rP-&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P]);
			
			
		}
		
	}
	else
	{
	
	}
	return ret;
}


uint32 dev_wifiReadString(char *data,uint32 datalen,uint32 offset)
{
	//memcpy(data,&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P+offset],datalen);
	//return datalen;
#if 1
	int i32Ret;
	i32Ret = g_stWifiUart.CMD_write_P-g_stWifiUart.CMD_read_P;
	if(datalen >= i32Ret)
	{
		memcpy(data,&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P],i32Ret);
		return i32Ret;

	}
	else
	{
		memcpy(data, &g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P], datalen);
		return datalen;
	}
#endif
}

/*
*@return :cmd的位置相对于readP的偏移量
*/
int dev_wifiWaitStringCLOSED(int8 *buf,uint32 readP,uint32 writeP,uint8 *cmd)
{
	int ret=-1;
	int8 *rP=NULL;
	//uint32 uiTime;
	
	
	if(readP==writeP)
	{
		//sysLOG(2,"read_P==write_P\r\n");//未收到数据
	}
	else if(readP<writeP)
	{
		sysDelayMs(10);
		
		rP=myStrStr(buf+readP,cmd,readP,writeP);
		
		if(rP!=NULL)
		{
			
			//g_stWifiUart.uart_buff_len=writeP-readP;
			sysLOG(WIFI_LOG_LEVEL_1, "write_P:%d,read_P:%d\r\n", writeP, readP);
			ret=(rP-(buf+readP));
		}
	}
	
	
	
	return ret;
}


int DecToString(unsigned int uiDec, char *pcString)
{
 unsigned char ucTemp[32];
 int i = 0, j = 0;

 memset(ucTemp,0x00,32);
 
 do
 {
       ucTemp[i++] = uiDec%10+'0';  //取下一个数字
 }
 while ((uiDec/=10)>0);   //删除该数字

 for(j = 0; j < i; j++)   //生成的数字是逆序的，所以要逆序输出
 {
  pcString[j] = ucTemp[i-j-1];
 }
}

BOOL Check_string09(uint8 *data)
{
	if((*data>=0x30) && (*data<=0x39))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*
*
*@return -2:有CLOSED;0:无
*/
int dev_wifiRead_CLOSED(void)
{
	int8 i=0,j=0,k=0;
	int ret0, i32Ret=0;
	int8 *ret1=NULL,*ret2=NULL;
	uint32 readP_temp;
	uint32 dlenth;
	uint8 dlenth_temp[20];
	
	uint8 m=0,n;

	void *pBuf = NULL;

    readP_temp=g_stWifiUart.CMD_read_P;
    
	while(1)
	{
		ret0=dev_wifiWaitStringCLOSED(g_stWifiUart.CMD_uart_buff,readP_temp,g_stWifiUart.CMD_write_P,"+IPD");//获得+IPD指针
		if(ret0>=0)//找到+IPD
		{
			if(ret0>0)//+IPD 前面有数据
			{	
				ret1=strstr(&g_stWifiUart.CMD_uart_buff[readP_temp],":");//获得:位置
				ret2=strstr(&g_stWifiUart.CMD_uart_buff[readP_temp],",");
				if(ret1!=NULL)
				{	
					for(i=1;i<20;i++)
					{
						dlenth_temp[i-1]=*(ret2+i);
						sysLOG(WIFI_LOG_LEVEL_4, "0 CLOSED +IPD:dlenth_temp[%d]=%d\r\n", i-1, dlenth_temp[i-1]);
						if(dlenth_temp[i-1]==0x3A)
						{
							break;
						}
					}
					
					dlenth=dlenth_temp[i-2]-0x30;
					for(k=i-2;k>0;k--)
					{
						j++;
						dlenth+=(dlenth_temp[k-1]-0x30)*pow(10,j);	
					}
					j=0;

					g_staClosedBuff[m].read_P=readP_temp;
					g_staClosedBuff[m].num=ret0;
					sysLOG(WIFI_LOG_LEVEL_4, "find +IPD %d:read_P:%d,num:%d\r\n", m, g_staClosedBuff[m].read_P, g_staClosedBuff[m].num);
					m++;

					readP_temp+=(ret1-&g_stWifiUart.CMD_uart_buff[readP_temp])+dlenth+1;
				}
			}
			else//+IPD前面没数据
			{
				ret1=strstr(&g_stWifiUart.CMD_uart_buff[readP_temp],":");//获得:位置
				ret2=strstr(&g_stWifiUart.CMD_uart_buff[readP_temp],",");
				
				if(ret1!=NULL)
				{
					
					for(i=1;i<20;i++)
					{
						
						dlenth_temp[i-1]=*(ret2+i);
						sysLOG(WIFI_LOG_LEVEL_4, "1 CLOSED +IPD:dlenth_temp[%d]=%d\r\n", i-1, dlenth_temp[i-1]);
						if(dlenth_temp[i-1]==0x3A)
						{
							break;
						}
					}
					
					dlenth=dlenth_temp[i-2]-0x30;
					for(k=i-2;k>0;k--)
					{
						j++;
						dlenth+=(dlenth_temp[k-1]-0x30)*pow(10,j);	
					}
					j=0;

					readP_temp+=(ret1-&g_stWifiUart.CMD_uart_buff[readP_temp])+dlenth+1;
				}
			}
		}
		else//后续没有+IPD
		{
			if(readP_temp<g_stWifiUart.CMD_write_P)//有可能存在收到两帧+IPD,第一帧完整，第二帧不完整，那么第二帧+IPD后面的就不用判断CLOSED
			{
				g_staClosedBuff[m].read_P=readP_temp;
				g_staClosedBuff[m].num=g_stWifiUart.CMD_write_P-readP_temp;
				sysLOG(WIFI_LOG_LEVEL_4, "find %d:read_P:%d,num:%d\r\n", m, g_staClosedBuff[m].read_P, g_staClosedBuff[m].num);
				m++;
			}

			break;

		}
	}

	for(n=0;n<m;n++)
	{
		 pBuf = malloc(g_staClosedBuff[n].num+1);
	    if (NULL == pBuf)
	    {
	        return;
	    }
		memset(pBuf,0,g_staClosedBuff[n].num+1);
		memcpy(pBuf,&g_stWifiUart.CMD_uart_buff[g_staClosedBuff[n].read_P],g_staClosedBuff[n].num);
		ret0=dev_wifiWaitStringCLOSED(pBuf,0,g_staClosedBuff[n].num,"CLOSED");
		if(ret0>=0)
		{
			i32Ret=-2;
			sysLOG(WIFI_LOG_LEVEL_4, "find CLOSED %d,,read_P:%d\r\n", n, g_staClosedBuff[n].read_P);
			free(pBuf);
			break;
		}
		free(pBuf);
	}
	return i32Ret;
	
}

//CLOSEd+IPD,12:123456789abc defghijk+IPD,3:oko+IPD,6:closed 9876543210+IPD,8:CLOSED34 aaaCLOSEdbb+IPD,2:uu CLOSEDn+IPD,5:abc
//		 6					    26	   34		 44			  57		  67			   82			93		 102      109
void dev_read_CLOSED_test(void)
{
	g_stWifiUart.CMD_read_P=g_stWifiUart.CMD_write_P=0;
	memset(g_stWifiUart.CMD_uart_buff,0,sizeof(g_stWifiUart.CMD_uart_buff));
	sprintf(g_stWifiUart.uart_buff,"CLOSEd+IPD,12:123456789abcdefghijk+IPD,3:oko+IPD,6:closed9876543210+IPD,8:CLOSED34aaaCLOSEdbb+IPD,2:uuCLOSEDn+IPD,5:abc");
	g_stWifiUart.write_P=119;
	sysLOG(WIFI_LOG_LEVEL_3, "dev_wifiRead_CLOSED return :%d\r\n", dev_wifiRead_CLOSED());
}

/*
*
*@return:返回cmd相对于read_P的偏移量
*
*/
int dev_wifiWaitStringData1(int8 *cmd)
{
	int ret=-1;
	int8 *rP=NULL;
	//uint32 uiTime;
	int readptemp;
	//uint8 uartbufftemp[10];
	
	if(g_stWifiUart.read_P==g_stWifiUart.write_P)
	{
		//sysLOG(2,"read_P==write_P\r\n");//未收到数据
	}
	else if(g_stWifiUart.read_P<g_stWifiUart.write_P)
	{
		//sysDelayMs(10);
		
		rP=myStrStr(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],cmd,g_stWifiUart.read_P,g_stWifiUart.write_P);
		//rP=strstr_cmd(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],cmd,(g_stWifiUart.write_P-g_stWifiUart.read_P));
		
		if(rP!=NULL)
		{
			readptemp=(rP-&g_stWifiUart.uart_buff[g_stWifiUart.read_P]);
			sysLOG(WIFI_LOG_LEVEL_5, "11 read_P<write_P,readptemp:%d\r\n", readptemp);
			//g_stWifiUart.uart_buff_len=g_stWifiUart.write_P-g_stWifiUart.read_P;						
			ret=(rP-&g_stWifiUart.uart_buff[g_stWifiUart.read_P]);
			sysLOG(WIFI_LOG_LEVEL_5, "11uart uart_buff_count:%d,write_P:%d,read_P:%d,ret:%d\r\n", g_stWifiUart.uart_buff_count, g_stWifiUart.write_P, g_stWifiUart.read_P, ret);
		}
	}
	else if(g_stWifiUart.read_P>g_stWifiUart.write_P)
	{
		//sysDelayMs(10);
		
		rP=myStrStr(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],cmd,g_stWifiUart.read_P,UARTBUFF_LEN);
		//sysLOG(WIFI_LOG_LEVEL_5, "read_P>write_P,uart_buff:%s\r\n", &g_stWifiUart.uart_buff[g_stWifiUart.read_P]);
		//rP=strstr_cmd(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],cmd,(g_stWifiUart.write_P-g_stWifiUart.read_P));	
		if(rP!=NULL)
		{
			readptemp=(rP-&g_stWifiUart.uart_buff[g_stWifiUart.read_P]);
			sysLOG(WIFI_LOG_LEVEL_5, "22 read_P>write_P\r\n");
			//g_stWifiUart.uart_buff_len=g_stWifiUart.write_P-g_stWifiUart.read_P;						
			ret=(rP-&g_stWifiUart.uart_buff[g_stWifiUart.read_P]);
			sysLOG(WIFI_LOG_LEVEL_5, "22uart uart_buff_count:%d,write_P:%d,read_P:%d,ret:%d\r\n", g_stWifiUart.uart_buff_count, g_stWifiUart.write_P, g_stWifiUart.read_P, ret);
			return ret;
		}
		rP=myStrStr(&g_stWifiUart.uart_buff[0],cmd,0,g_stWifiUart.write_P);
		if(rP!=NULL)
		{
			readptemp=rP-&g_stWifiUart.uart_buff[0];
			if(readptemp<g_stWifiUart.write_P)//只读到writeP;
			{
				//g_stWifiUart.uart_buff_len=UARTBUFF_LEN-g_stWifiUart.read_P+g_stWifiUart.write_P;
				ret=rP-&g_stWifiUart.uart_buff[0]+UARTBUFF_LEN-g_stWifiUart.read_P;
				sysLOG(WIFI_LOG_LEVEL_5, "33uart uart_buff_count:%d,write_P:%d,read_P:%d,ret:%d\r\n", g_stWifiUart.uart_buff_count, g_stWifiUart.write_P, g_stWifiUart.read_P, ret);
				return ret;
			}
			
		}

		memset(mystr_temp_s.buf,0,sizeof(mystr_temp_s.buf));
		mystr_temp_s.read_P=mystr_temp_s.write_P=0;
		memcpy(mystr_temp_s.buf,&g_stWifiUart.uart_buff[UARTBUFF_LEN-5],5);
		memcpy(&mystr_temp_s.buf[5],&g_stWifiUart.uart_buff[0],5);
		rP=myStrStr(&mystr_temp_s.buf[0],cmd,0,10);
		//sysLOG(WIFI_LOG_LEVEL_5, "mystr_temp_s.buf:%s\r\n", mystr_temp_s.buf);
		if(rP!=NULL)
		{
			readptemp=rP-&mystr_temp_s.buf[0];
			//g_stWifiUart.uart_buff_len=UARTBUFF_LEN-g_stWifiUart.read_P+g_stWifiUart.write_P;
			ret=rP-&mystr_temp_s.buf[0]+UARTBUFF_LEN-5-g_stWifiUart.read_P;
			sysLOG(WIFI_LOG_LEVEL_5, "44uart uart_buff_count:%d,write_P:%d,read_P:%d,ret:%d\r\n", g_stWifiUart.uart_buff_count, g_stWifiUart.write_P, g_stWifiUart.read_P, ret);
			return ret;
		}

#if 0		
		memset(uartbufftemp,0,sizeof(uartbufftemp));
		memcpy(uartbufftemp,&g_stWifiUart.uart_buff[UARTBUFF_LEN-5],5);
		memcpy(&uartbufftemp[5],&g_stWifiUart.uart_buff[0],5);
		rP=strstr_cmd(uartbufftemp,cmd,10);
		if(rP!=NULL)
		{
			readptemp=rP-&uartbufftemp[0];
			//g_stWifiUart.uart_buff_len=UARTBUFF_LEN-g_stWifiUart.read_P+g_stWifiUart.write_P;
			ret=rP-&uartbufftemp[0]+UARTBUFF_LEN-5-g_stWifiUart.read_P;
			return ret;
		}
#endif		
	}
	
	
	
	return ret;
}

/*
*@brief:如果uart_buff满了，则要记录+IPD后面剩下的数据都要丢掉，否则就会copy到CMD_buf里，以及已经收到的部分数据也要丢掉，
*@param1:+IPD后面的有效数据长度,param2:实际收到的数据长度
*@return:-1-数据空间够用，1-数据空间不够用需要丢掉完整一帧+IPD
*/
int dev_cpy_data_judge(uint32 ipd_datalenth,uint32 ipd_datarecvd)
{
	int ret = -1;
	if(ipd_datalenth>=(UARTBUFF_LEN-g_stWifiUart.uart_buff_count-64))//剩余空间不够ipd数据存入，留出64个字节的余量，
	{
		
		sysLOG(WIFI_LOG_LEVEL_4, " ipd_datalenth:%d,g_stWifiUart.uart_buff_count:%d\r\n", ipd_datalenth, g_stWifiUart.uart_buff_count);
		g_stWifiUart.invailddatalen=ipd_datalenth-ipd_datarecvd;//获得后面剩余的ipd数据
		ret = 1;
	}
	return ret;
}

void resetWifiBuf(void)
{
	DR_IPDmutex_lock(g_ui32IPDMutexHandle);
    g_stWifiUart.IPD_holdflag = 0;
    g_stWifiUart.read_P = g_stWifiUart.write_P = 0;
    g_stWifiUart.uart_buff_count = 0;
    memset(g_stWifiUart.uart_buff, 0, sizeof(g_stWifiUart.uart_buff));
    g_stWifiUart.CMD_read_P = g_stWifiUart.CMD_write_P = 0;
    memset(g_stWifiUart.CMD_uart_buff, 0, sizeof(g_stWifiUart.CMD_uart_buff));
    g_stWifiUart.invailddatalen = 0;//
    /////g_stWifiUart.IPD_read_P = g_stWifiUart.IPD_write_P = 0;
    /////g_stWifiUart.IPD_uart_buff_len = 0;
    /////memset(g_stWifiUart.IPD_uart_buff, 0, sizeof(g_stWifiUart.IPD_uart_buff));
    g_ui8CldsdClosemodeFlag = 0;
    //g_ui8WifiConnectClosed = 0;

    DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
}

void getIPDDataOne(int8 *IPD_uart_buff, volatile uint32 *IPD_uart_buff_len, volatile uint32 *IPD_write_P, uint32 lenth, int i32Ret0, int8 *i32Ret1)
{
	uint32 dataoffset=0;
    int32 ipd_onceoffset;
    int32 ipd_head_len;
	char printDataBuf[1025];
	memset(printDataBuf, 0x00, 1025);

	DR_IPDmutex_lock(g_ui32IPDMutexHandle);
    if ((IPDUARTBUFF_LEN - *IPD_uart_buff_len) > lenth) //接收到的数据IPD_buf能存下
    {
        dataoffset = i32Ret0; //获得+IPD相对于read_P的偏移量
        ipd_head_len = (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1);
        sysLOG(WIFI_LOG_LEVEL_4, "111 1dataoffset:%d,IPDlen:%d\r\n", dataoffset, lenth+ipd_head_len);
		sysLOG(WIFI_LOG_LEVEL_4, "*IPD_write_P:%d,lenth:%d\r\n", *IPD_write_P, lenth);
        ipd_onceoffset = (lenth) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入
        if (ipd_onceoffset < 0)
        {
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			sysLOG(WIFI_LOG_LEVEL_4, "ipd_onceoffset:%d,lenth:%d\r\n", ipd_onceoffset, lenth);
           // sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d %d\r\n",g_stWifiUart.read_P,dataoffset,ipd_head_len,*IPD_write_P);
			memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset + ipd_head_len]), lenth);
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			 memcpy(printDataBuf, &IPD_uart_buff[*IPD_write_P], lenth);
			// sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			 //sysLOG(WIFI_LOG_LEVEL_4, "lenth:%d,printDataBuf:%s\r\n", lenth, printDataBuf);
            *IPD_write_P = *IPD_write_P + lenth;
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
            *IPD_uart_buff_len += (lenth);
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
        }
        else
        {
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			sysLOG(WIFI_LOG_LEVEL_4, "ipd_onceoffset:%d,lenth:%d\r\n", ipd_onceoffset, lenth);
            memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset + ipd_head_len]), (IPDUARTBUFF_LEN - *IPD_write_P));
            memcpy(&IPD_uart_buff[0], &(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset + ipd_head_len + (IPDUARTBUFF_LEN - *IPD_write_P)]), ipd_onceoffset);
            //sysLOG(WIFI_LOG_LEVEL_4, "2recvd dlenth:%d,IPD:%s\r\n", lenth, &IPD_uart_buff[0+5]);
            *IPD_write_P = ipd_onceoffset;
            *IPD_uart_buff_len += (lenth);
			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
        }

        memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, lenth + (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1));
        g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1));
        g_stWifiUart.read_P += lenth + (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1);
		//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
    }
    else //存不下就删掉
    {
		//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
        dataoffset = i32Ret0; //获得+IPD相对于read_P的偏移量
        ipd_head_len = (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1);
        sysLOG(WIFI_LOG_LEVEL_4, "111 2dataoffset:%d,IPDlen:%d\r\n", dataoffset, lenth+ipd_head_len);

        ipd_onceoffset = (lenth) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入

        memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, lenth + (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1));
        g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1));
        g_stWifiUart.read_P += lenth + (i32Ret1 - &g_stWifiUart.uart_buff[g_stWifiUart.read_P] - i32Ret0 + 1);
		//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
    }
	//sysLOG(WIFI_LOG_LEVEL_1, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
	DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
	sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len12:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
}

void getIPDDataTwo(int8 *IPD_uart_buff, volatile uint32 *IPD_uart_buff_len, volatile uint32 *IPD_write_P, uint32 lenth, int i32Ret0, uint32 ipd_tempdata_P)
{
	uint32 dataoffset=0;
    int32 ipd_onceoffset;
    int32 ipd_head_len;

    DR_IPDmutex_lock(g_ui32IPDMutexHandle);
    if ((IPDUARTBUFF_LEN - *IPD_uart_buff_len) > lenth)
    {
        dataoffset = i32Ret0; //获得+IPD相对于read_P的偏移量
        ipd_head_len = (ipd_tempdata_P + UARTBUFF_LEN - g_stWifiUart.read_P + 1);
        //sysLOG(WIFI_LOG_LEVEL_3, "222 1dataoffset:%d,IPDlen:%d\r\n", dataoffset, dlenth+ipd_head_len);
		sysLOG(WIFI_LOG_LEVEL_4, "*IPD_write_P:%d,lenth:%d\r\n", *IPD_write_P, lenth);
        ipd_onceoffset = (lenth) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入
        if(ipd_onceoffset < 0)
        {
			sysLOG(WIFI_LOG_LEVEL_4, "ipd_onceoffset:%d,lenth:%d\r\n", ipd_onceoffset, lenth);
            memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[0 + ipd_tempdata_P + 1]), lenth);
            *IPD_write_P = *IPD_write_P + lenth;
            *IPD_uart_buff_len += (lenth);
			sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
        }
        else
        {
			sysLOG(WIFI_LOG_LEVEL_4, "ipd_onceoffset:%d,lenth:%d\r\n", ipd_onceoffset, lenth);
            memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[0 + ipd_tempdata_P + 1]), (IPDUARTBUFF_LEN - *IPD_write_P));
            memcpy(&IPD_uart_buff[0], &(g_stWifiUart.uart_buff[0 + ipd_tempdata_P + 1 + (IPDUARTBUFF_LEN - *IPD_write_P)]), ipd_onceoffset);
            //sysLOG(WIFI_LOG_LEVEL_3, "4recvd dlenth:%d,IPD:%s\r\n", dlenth, &g_stWifiUart.IPD_uart_buff[0+5]);
            *IPD_write_P = ipd_onceoffset;
            *IPD_uart_buff_len += (lenth);
			sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
        }

        memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, UARTBUFF_LEN - g_stWifiUart.read_P);
        memset(&g_stWifiUart.uart_buff[0], 0, (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P));
        g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (ipd_head_len));
        g_stWifiUart.read_P = (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P);
    }
    else
    {
        dataoffset = i32Ret0; //获得+IPD相对于read_P的偏移量
        ipd_head_len = (ipd_tempdata_P + UARTBUFF_LEN - g_stWifiUart.read_P + 1);
        sysLOG(WIFI_LOG_LEVEL_3, "222 2dataoffset:%d,IPDlen:%d\r\n", dataoffset, lenth+ipd_head_len);

        ipd_onceoffset = (lenth) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入

        memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, UARTBUFF_LEN - g_stWifiUart.read_P);
        memset(&g_stWifiUart.uart_buff[0], 0, (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P));
        g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (ipd_head_len));
        g_stWifiUart.read_P = (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P);
    }
    DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
    //sysLOG(APIWIFI_LOG_LEVEL_4, "<END> fibo_mutex_unlock g_ui32IPDMutexHandle\r\n");
}

void getIPDDataThr(int8 *IPD_uart_buff, volatile uint32 *IPD_uart_buff_len, volatile uint32 *IPD_write_P, uint32 lenth, int i32Ret0, uint32 ipd_tempdata_P)
{
	uint32 dataoffset=0;
    int32 ipd_onceoffset;
    int32 ipd_head_len;
	uint32 LL1, LL2;

    DR_IPDmutex_lock(g_ui32IPDMutexHandle);
    if ((IPDUARTBUFF_LEN - *IPD_uart_buff_len) > lenth)
    {
        dataoffset = i32Ret0; //获得+IPD相对于read_P的偏移量

        ipd_head_len = (ipd_tempdata_P - g_stWifiUart.read_P + 1);
        //sysLOG(WIFI_LOG_LEVEL_3, "333 1dataoffset:%d,IPDlen:%d\r\n", dataoffset, dlenth+(ipd_head_len));
       sysLOG(WIFI_LOG_LEVEL_4, "*IPD_write_P:%d,lenth:%d\r\n", *IPD_write_P, lenth);
	    LL1 = UARTBUFF_LEN - (ipd_tempdata_P + 1); //dlenth在末尾的长度
        if (LL1 <= lenth)
        {
            LL2 = lenth - LL1; //dlenth在开头的长度
            //sysLOG(WIFI_LOG_LEVEL_3, "dlenthrecvd>=dlenth--,LL1:%d,LL2:%d\r\n", LL1, LL2);
            ipd_onceoffset = (LL1) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入
            if (ipd_onceoffset < 0)
            {
				sysLOG(WIFI_LOG_LEVEL_4, "ipd_onceoffset:%d,lenth:%d\r\n", ipd_onceoffset, lenth);
                memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[ipd_tempdata_P + 1]), LL1);
                *IPD_write_P = *IPD_write_P + LL1;
                *IPD_uart_buff_len += (LL1);
				sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
            }
            else
            {
				sysLOG(WIFI_LOG_LEVEL_4, "ipd_onceoffset:%d,lenth:%d\r\n", ipd_onceoffset, lenth);
                memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[ipd_tempdata_P + 1]), (IPDUARTBUFF_LEN - *IPD_write_P));
                memcpy(&IPD_uart_buff[0], &(g_stWifiUart.uart_buff[ipd_tempdata_P + 1 + (IPDUARTBUFF_LEN - *IPD_write_P)]), ipd_onceoffset);
                //sysLOG(WIFI_LOG_LEVEL_3, "6recvd dlenth:%d,IPD:%s\r\n", dlenth, &g_stWifiUart.IPD_uart_buff[0+5]);
                *IPD_write_P = ipd_onceoffset;
                *IPD_uart_buff_len += (LL1);
				sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
            }

            ipd_onceoffset = (LL2) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入
            if (ipd_onceoffset < 0)
            {
                memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[0]), LL2);
                *IPD_write_P = *IPD_write_P + LL2;
                *IPD_uart_buff_len += (LL2);
				sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
            }
            else
            {
                memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[0]), (IPDUARTBUFF_LEN - *IPD_write_P));
                memcpy(&IPD_uart_buff[0], &(g_stWifiUart.uart_buff[(IPDUARTBUFF_LEN - *IPD_write_P)]), ipd_onceoffset);
                //sysLOG(WIFI_LOG_LEVEL_3, "8recvd dlenth:%d,IPD:%s\r\n", dlenth, &g_stWifiUart.IPD_uart_buff[0+5]);
                *IPD_write_P = ipd_onceoffset;
                *IPD_uart_buff_len += (LL2);
				sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
            }

            memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, UARTBUFF_LEN - g_stWifiUart.read_P);
            memset(&g_stWifiUart.uart_buff[0], 0, (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P));
            g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (ipd_head_len));
            g_stWifiUart.read_P = (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P);
        }
        else //存在虽然read_P>write_P,但是dlenth长度的内容都在read_P之后，后面的是其他数据
        {
            LL1 = lenth;
            sysLOG(WIFI_LOG_LEVEL_3, "dlenthrecvd>=dlenth--,LL1:%d\r\n", LL1);
            ipd_onceoffset = (LL1) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入
            if (ipd_onceoffset < 0)
            {
                memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[ipd_tempdata_P + 1]), LL1);
                *IPD_write_P = *IPD_write_P + LL1;
                *IPD_uart_buff_len += (LL1);
				sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
            }
            else
            {
                memcpy(&IPD_uart_buff[*IPD_write_P], &(g_stWifiUart.uart_buff[ipd_tempdata_P + 1]), (IPDUARTBUFF_LEN - *IPD_write_P));
                memcpy(&IPD_uart_buff[0], &(g_stWifiUart.uart_buff[ipd_tempdata_P + 1 + (IPDUARTBUFF_LEN - *IPD_write_P)]), ipd_onceoffset);
                //sysLOG(WIFI_LOG_LEVEL_3, "10recvd dlenth:%d,IPD:%s\r\n", dlenth, &g_stWifiUart.IPD_uart_buff[0+5]);
                *IPD_write_P = ipd_onceoffset;
                *IPD_uart_buff_len += (LL1);
				sysLOG(WIFI_LOG_LEVEL_4, "*IPD_uart_buff_len:%d,*IPD_write_P:%d\r\n", *IPD_uart_buff_len, *IPD_write_P);
            }

            memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, lenth + (ipd_head_len));
            g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (ipd_head_len));
            g_stWifiUart.read_P += (ipd_head_len) + lenth;
        }
    }
    else
    {
        dataoffset = i32Ret0; //获得+IPD相对于read_P的偏移量

        ipd_head_len = (ipd_tempdata_P - g_stWifiUart.read_P + 1);
        sysLOG(WIFI_LOG_LEVEL_3, "333 2dataoffset:%d,IPDlen:%d\r\n", dataoffset, lenth+(ipd_head_len));
        LL1 = UARTBUFF_LEN - (ipd_tempdata_P + 1); //dlenth在末尾的长度
        if (LL1 <= lenth)
        {
            LL2 = lenth - LL1; //dlenth在开头的长度
            sysLOG(WIFI_LOG_LEVEL_3, "dlenthrecvd>=dlenth--,LL1:%d,LL2:%d\r\n", LL1, LL2);
            ipd_onceoffset = (LL1) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入

            ipd_onceoffset = (LL2) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入

            memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, UARTBUFF_LEN - g_stWifiUart.read_P);
            memset(&g_stWifiUart.uart_buff[0], 0, (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P));
            g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (ipd_head_len));
            g_stWifiUart.read_P = (ipd_head_len) + lenth - (UARTBUFF_LEN - g_stWifiUart.read_P);
        }
        else //存在虽然read_P>write_P,但是dlenth长度的内容都在read_P之后，后面的是其他数据
        {
            LL1 = lenth;
            sysLOG(WIFI_LOG_LEVEL_3, "dlenthrecvd>=dlenth,LL1:%d\r\n", LL1);
            ipd_onceoffset = (LL1) - (IPDUARTBUFF_LEN - *IPD_write_P); //<0后续空间够用，>=0则需要循环写入

            memset(&(g_stWifiUart.uart_buff[g_stWifiUart.read_P + dataoffset]), 0, lenth + (ipd_head_len));
            g_stWifiUart.uart_buff_count = g_stWifiUart.uart_buff_count - (lenth + (ipd_head_len));
            g_stWifiUart.read_P += (ipd_head_len) + lenth;
        }
    }
    DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
    //sysLOG(APIWIFI_LOG_LEVEL_4, "<END> fibo_mutex_unlock g_ui32IPDMutexHandle\r\n");
}

int dev_cpydata_IPD(void)
{
	while(1)
	{
		int8 i=0,j=0,k=0;
		int ret0=-1;
		int8 *ret1=NULL,*ret2=NULL, *ret3=NULL;
		int8 link_ID = 0;

		//uint32 uiTime;
		uint32 dlenth,dlenthrecvd;//datalen1=0;
		uint32 dataoffset=0;
		uint8 dlenth_temp[20];
		uint32 ipd_tempdata_P=0;
		int32 ipd_onceoffset;
		int32 ipd_head_len;
		
		if(g_stWifiUart.uart_buff_count == 0)
		{
			sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
			break;
		}
		ret0=dev_wifiWaitStringData1("+IPD");//获得+IPD指针
		//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
		if(ret0>=0)       //有+IPD
		{
			if(ret0>0)    //提取CMD，CMD缓冲不够时，直接把CMD部分丢掉
			{
				if((CMDUARTBUFF_LEN-g_stWifiUart.CMD_write_P)>=ret0)//CMD_buf能存下
				{
					if(ret0+g_stWifiUart.read_P>UARTBUFF_LEN)//
					{
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P],&g_stWifiUart.uart_buff[g_stWifiUart.read_P],UARTBUFF_LEN-g_stWifiUart.read_P);
						memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P+UARTBUFF_LEN-g_stWifiUart.read_P],&g_stWifiUart.uart_buff[0],ret0+g_stWifiUart.read_P-UARTBUFF_LEN);
						memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,UARTBUFF_LEN-g_stWifiUart.read_P);
						memset(&g_stWifiUart.uart_buff[0],0,ret0+g_stWifiUart.read_P-UARTBUFF_LEN);
						g_stWifiUart.CMD_write_P+=ret0;
						g_stWifiUart.read_P=ret0+g_stWifiUart.read_P-UARTBUFF_LEN;
						g_stWifiUart.uart_buff_count-=ret0;
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						sysLOG(WIFI_LOG_LEVEL_4, "CMD_write_P=%d read_P=%d uart_buff_count=%d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.read_P,g_stWifiUart.uart_buff_count);
						
					}
					else
					{
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P],&g_stWifiUart.uart_buff[g_stWifiUart.read_P],ret0);
						memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,ret0);
						g_stWifiUart.CMD_write_P+=ret0;
						g_stWifiUart.read_P+=ret0;
						g_stWifiUart.uart_buff_count-=ret0;
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						sysLOG(WIFI_LOG_LEVEL_4, "CMD_write_P=%d read_P=%d uart_buff_count=%d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.read_P,g_stWifiUart.uart_buff_count);
					}
					sysLOG(WIFI_LOG_LEVEL_4, "dev_wifiWaitStringData1\r\n");
					ret0=dev_wifiWaitStringData1("+IPD");//获得+IPD指针
					if(ret0<0)return ret0;
				}
				else
				{
					if(ret0+g_stWifiUart.read_P>UARTBUFF_LEN)//
					{
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,UARTBUFF_LEN-g_stWifiUart.read_P);
						memset(&g_stWifiUart.uart_buff[0],0,ret0+g_stWifiUart.read_P-UARTBUFF_LEN);
						//g_stWifiUart.CMD_write_P+=ret0;
						g_stWifiUart.read_P=ret0+g_stWifiUart.read_P-UARTBUFF_LEN;
						g_stWifiUart.uart_buff_count-=ret0;
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						sysLOG(WIFI_LOG_LEVEL_4, "CMD_write_P=%d read_P=%d uart_buff_count=%d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.read_P,g_stWifiUart.uart_buff_count);
					}
					else
					{
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						//memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P],&g_stWifiUart.uart_buff[g_stWifiUart.read_P],ret0);
						memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,ret0);
						//g_stWifiUart.CMD_write_P+=ret0;
						g_stWifiUart.read_P+=ret0;
						g_stWifiUart.uart_buff_count-=ret0;
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						sysLOG(WIFI_LOG_LEVEL_4, "CMD_write_P=%d read_P=%d uart_buff_count=%d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.read_P,g_stWifiUart.uart_buff_count);
					}
					sysLOG(WIFI_LOG_LEVEL_4, "dev_wifiWaitStringData1\r\n");
					ret0=dev_wifiWaitStringData1("+IPD");//获得+IPD指针
					if(ret0<0)return ret0;	
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);				
				}
			}

			//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
			if(g_stWifiUart.read_P<g_stWifiUart.write_P)
			{
				sysLOG(WIFI_LOG_LEVEL_4, "find +IPD offset :ret0 is %d\r\n", ret0);
				ret1=myStrStr(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],":",g_stWifiUart.read_P,g_stWifiUart.write_P);//获得:位置
				ret2=myStrStr(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],",",g_stWifiUart.read_P,g_stWifiUart.write_P);
				//if(g_cWifiVersionFlag == 1)
				{
					ret3=myStrStr(ret2+1,",", 0, ret1-ret2);
					link_ID = (*(ret2+1) - '0');
					if(((link_ID < 0x00) || (link_ID > 0x04)) && (ret3 == NULL)) 
					{
						resetWifiBuf();
					    return -3;
					}
					ret2 = ret3;
				}
				
				if(ret1!=NULL)
				{	
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					for(i=1;i<20;i++)
					{	
						dlenth_temp[i-1]=*(ret2+i);
						sysLOG(1,"dlenth_temp[%d]=%d\r\n",i-1,dlenth_temp[i-1]);
						if(dlenth_temp[i-1]==0x3A)
						{	
							sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
							break;
						}
						if(Check_string09(&dlenth_temp[i-1])==0)//判断dlenth是否有效
						{
							resetWifiBuf();
							return -3;
						}
					}
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					dlenth=dlenth_temp[i-2]-0x30;
					for(k=i-2;k>0;k--)
					{
						j++;
						dlenth+=(dlenth_temp[k-1]-0x30)*pow(10,j);	
					}
					j=0;
					
					dlenthrecvd=g_stWifiUart.uart_buff_count-(ret1-&g_stWifiUart.uart_buff[g_stWifiUart.read_P]+1);
					if(dev_cpy_data_judge(dlenth,dlenthrecvd) == 1)//需要把收到的部分+IPD数据也丢掉
					{
						g_stWifiUart.write_P = g_stWifiUart.read_P;
						g_stWifiUart.uart_buff_count = 0;
						return -4;
					}
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
					if(dlenthrecvd>=dlenth)//接收到的数据满足+IPD后面的数据
					{
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						if((link_ID == 0) && (g_stWifiUart.IPD_zero_uart_buff != NULL))
						{
							getIPDDataOne(g_stWifiUart.IPD_zero_uart_buff, &g_stWifiUart.IPD_zero_uart_buff_len, &g_stWifiUart.IPD_zero_write_P, dlenth, ret0, ret1);
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						}
						else if((link_ID == 1) && (g_stWifiUart.IPD_one_uart_buff != NULL))
						{
							getIPDDataOne(g_stWifiUart.IPD_one_uart_buff, &g_stWifiUart.IPD_one_uart_buff_len, &g_stWifiUart.IPD_one_write_P, dlenth, ret0, ret1);
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						}
						else if((link_ID == 2) && (g_stWifiUart.IPD_two_uart_buff != NULL))
						{
							getIPDDataOne(g_stWifiUart.IPD_two_uart_buff, &g_stWifiUart.IPD_two_uart_buff_len, &g_stWifiUart.IPD_two_write_P, dlenth, ret0, ret1);
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						}
						else if((link_ID == 3) && (g_stWifiUart.IPD_three_uart_buff != NULL))
						{
							getIPDDataOne(g_stWifiUart.IPD_three_uart_buff, &g_stWifiUart.IPD_three_uart_buff_len, &g_stWifiUart.IPD_three_write_P, dlenth, ret0, ret1);
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						}
						else if((link_ID == 4) && (g_stWifiUart.IPD_four_uart_buff != NULL))
						{
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d %d\r\n",g_stWifiUart.IPD_four_write_P,g_stWifiUart.IPD_four_uart_buff_len,dlenth,ret0);
							getIPDDataOne(g_stWifiUart.IPD_four_uart_buff, &g_stWifiUart.IPD_four_uart_buff_len, &g_stWifiUart.IPD_four_write_P, dlenth, ret0, ret1);
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d %d\r\n",g_stWifiUart.IPD_four_write_P,g_stWifiUart.IPD_four_uart_buff_len,dlenth,ret0);
							//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
						}
						//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					}
					else
					{
						sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
						break;
					}
				}
				else
				{
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
					break;
				}
			}
			else
			{
				//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
				sysLOG(WIFI_LOG_LEVEL_4, "find +IPD offset :ret0 is %d\r\n", ret0);
				ret1=myStrStr(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],":",g_stWifiUart.read_P,UARTBUFF_LEN);//获得:位置
				ret2=myStrStr(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],",",g_stWifiUart.read_P,UARTBUFF_LEN);
				if(ret1==NULL)  //:位置返回到缓冲头部
				{
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					ret1=myStrStr(&g_stWifiUart.uart_buff[0],":",0,g_stWifiUart.write_P);//获得:位置
					if(ret1!=NULL)
					{
						if((ret1-&g_stWifiUart.uart_buff[0])<g_stWifiUart.write_P)//判读出：在0地址之后
						{	
							memset(mystr_temp_s.buf,0,sizeof(mystr_temp_s.buf));
							mystr_temp_s.read_P=mystr_temp_s.write_P=0;
							memcpy(mystr_temp_s.buf,&g_stWifiUart.uart_buff[UARTBUFF_LEN-20],20);
							if(g_stWifiUart.write_P>=20)
							{
								memcpy(&mystr_temp_s.buf[20],&g_stWifiUart.uart_buff[0],20);
							}
							else
							{
								memcpy(&mystr_temp_s.buf[20],&g_stWifiUart.uart_buff[0],g_stWifiUart.write_P);
							}
							ret1=myStrStr(&mystr_temp_s.buf[0],":",0,40);//重新获得:位置
							ret2=myStrStr(&mystr_temp_s.buf[0],",",0,40);
							//if(g_cWifiVersionFlag == 1)
							{
								ret3=myStrStr(ret2+1,",",0, ret1-ret2);
								link_ID = (*(ret2+1) - '0');
								if(((link_ID < 0x00) || (link_ID > 0x04)) && (ret3 == NULL)) 
								{
									resetWifiBuf();
									return -3;
								}
								ret2 = ret3;
							}
							
							if(ret1!=NULL)
							{
								for(i=1;i<20;i++)
								{
									dlenth_temp[i-1]=*(ret2+i);
									if(dlenth_temp[i-1]==0x3A)
									{	
										sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
										break;
									}
									if(Check_string09(&dlenth_temp[i-1])==0)//判断dlenth是否有效
									{
										resetWifiBuf();
										return -3;
									}
								}
								
								dlenth=dlenth_temp[i-2]-0x30;
								for(k=i-2;k>0;k--)
								{
									j++;
									dlenth+=(dlenth_temp[k-1]-0x30)*pow(10,j);	
								}
								j=0;

								ipd_tempdata_P=ret1-&mystr_temp_s.buf[0];//读取：在ipd_temp中的偏移量
								ipd_tempdata_P-=20;//ipd_tempdata_P最终获得的是：在uart_buff中的指针

								dlenthrecvd=g_stWifiUart.uart_buff_count-(ipd_tempdata_P+UARTBUFF_LEN-g_stWifiUart.read_P+1);
								if(dev_cpy_data_judge(dlenth,dlenthrecvd) == 1)
								{
									g_stWifiUart.write_P = g_stWifiUart.read_P;
									g_stWifiUart.uart_buff_count = 0;
									return -4;
								}
								if(dlenthrecvd>=dlenth)
								{
									if((link_ID == 0) && (g_stWifiUart.IPD_zero_uart_buff != NULL))
									{
										getIPDDataTwo(g_stWifiUart.IPD_zero_uart_buff, &g_stWifiUart.IPD_zero_uart_buff_len, &g_stWifiUart.IPD_zero_write_P, dlenth, ret0, ipd_tempdata_P);
									}
									else if((link_ID == 1) && (g_stWifiUart.IPD_one_uart_buff != NULL))
									{
										getIPDDataTwo(g_stWifiUart.IPD_one_uart_buff, &g_stWifiUart.IPD_one_uart_buff_len, &g_stWifiUart.IPD_one_write_P, dlenth, ret0, ipd_tempdata_P);
									}
									else if((link_ID == 2) && (g_stWifiUart.IPD_two_uart_buff != NULL))
									{
										getIPDDataTwo(g_stWifiUart.IPD_two_uart_buff, &g_stWifiUart.IPD_two_uart_buff_len, &g_stWifiUart.IPD_two_write_P, dlenth, ret0, ipd_tempdata_P);
									}
									else if((link_ID == 3) && (g_stWifiUart.IPD_three_uart_buff != NULL))
									{
										getIPDDataTwo(g_stWifiUart.IPD_three_uart_buff, &g_stWifiUart.IPD_three_uart_buff_len, &g_stWifiUart.IPD_three_write_P, dlenth, ret0, ipd_tempdata_P);
									}
									else if((link_ID == 4) && (g_stWifiUart.IPD_four_uart_buff != NULL))
									{
										getIPDDataTwo(g_stWifiUart.IPD_four_uart_buff, &g_stWifiUart.IPD_four_uart_buff_len, &g_stWifiUart.IPD_four_write_P, dlenth, ret0, ipd_tempdata_P);
									}
								}
								else
								{
									sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
									break;
								}
								
							}
						}
					}
					else
					{
						
						sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
						break;
					}
				}
				else//判断出：在readP之后
				{
					//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					//if(g_cWifiVersionFlag == 1)
					{
						ret3=myStrStr(ret2+1,",", 0, ret1-ret2);
						link_ID = (*(ret2+1) - '0');
						if(((link_ID < 0x00) || (link_ID > 0x04)) && (ret3 == NULL)) 
						{
							resetWifiBuf();
							return -3;
						}
						ret2 = ret3;
					}
					if(ret1!=NULL)		
					{
						for(i=1;i<20;i++)
						{
							dlenth_temp[i-1]=*(ret2+i);
							sysLOG(1,"dlenth_temp[%d]=%d\r\n",i-1,dlenth_temp[i-1]);
							if(dlenth_temp[i-1]==0x3A)
							{
								sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
								break;
							}
							if(Check_string09(&dlenth_temp[i-1])==0)//判断dlenth是否有效
							{
								resetWifiBuf();
								return -3;
							}
						}
						
						dlenth=dlenth_temp[i-2]-0x30;
						for(k=i-2;k>0;k--)
						{
							j++;
							dlenth+=(dlenth_temp[k-1]-0x30)*pow(10,j);	
						}
						j=0;

						ipd_tempdata_P=ret1-&g_stWifiUart.uart_buff[g_stWifiUart.read_P];//读取：在&g_stWifiUart.uart_buff[g_stWifiUart.read_P]中的偏移量
						ipd_tempdata_P+=g_stWifiUart.read_P;//ipd_tempdata_P最终获得的是：在uart_buff中的指针

						dlenthrecvd=g_stWifiUart.uart_buff_count-(ipd_tempdata_P-g_stWifiUart.read_P+1);
						if(dev_cpy_data_judge(dlenth,dlenthrecvd) == 1)
						{
							g_stWifiUart.write_P = g_stWifiUart.read_P;
							g_stWifiUart.uart_buff_count = 0;
							return -4;
						}
						if(dlenthrecvd>=dlenth)
						{
							if((link_ID == 0) && (g_stWifiUart.IPD_zero_uart_buff != NULL))
                            {
                                getIPDDataThr(g_stWifiUart.IPD_zero_uart_buff, &g_stWifiUart.IPD_zero_uart_buff_len, &g_stWifiUart.IPD_zero_write_P, dlenth, ret0, ipd_tempdata_P);
                            }
                            else if((link_ID == 1) && (g_stWifiUart.IPD_one_uart_buff != NULL))
                            {
                                getIPDDataThr(g_stWifiUart.IPD_one_uart_buff, &g_stWifiUart.IPD_one_uart_buff_len, &g_stWifiUart.IPD_one_write_P, dlenth, ret0, ipd_tempdata_P);
                            }
                            else if((link_ID == 2) && (g_stWifiUart.IPD_two_uart_buff != NULL))
                            {
                                getIPDDataThr(g_stWifiUart.IPD_two_uart_buff, &g_stWifiUart.IPD_two_uart_buff_len, &g_stWifiUart.IPD_two_write_P, dlenth, ret0, ipd_tempdata_P);
                            }
                            else if((link_ID == 3) && (g_stWifiUart.IPD_three_uart_buff != NULL))
                            {
                                getIPDDataThr(g_stWifiUart.IPD_three_uart_buff, &g_stWifiUart.IPD_three_uart_buff_len, &g_stWifiUart.IPD_three_write_P, dlenth, ret0, ipd_tempdata_P);
                            }
                            else if((link_ID == 4) && (g_stWifiUart.IPD_four_uart_buff != NULL))
                            {
                                getIPDDataThr(g_stWifiUart.IPD_four_uart_buff, &g_stWifiUart.IPD_four_uart_buff_len, &g_stWifiUart.IPD_four_write_P, dlenth, ret0, ipd_tempdata_P);
                            }
                        }
						else
						{
							sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
							break;
						}
						
					}
				}
			}
			
		}
		//sysLOG(WIFI_LOG_LEVEL_4, " ERROR123:%d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
		g_stWifiUart.CMD_cpy_num=0;
		if(g_stWifiUart.uart_buff_count!=0)//如果后续有数据
		{
			ret0=dev_wifiWaitStringData1("+I");//获得+I指针
			
			if(ret0==0)//+I之前没有CMD
			{
				g_stWifiUart.CMD_cpy_num=0;
				if(g_stWifiUart.uart_buff_count <= 4)//后面只有"+IPD"4个字节的一部分,退出copy
				{
					sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
					break;
				}
				else
				{
					ret0=dev_wifiWaitStringData1("+IPD");//如果接收长度超过了4个字节，但是还没有找到+IPD，则直接break	
					if(ret0 < 0)
					{
						break;
					}
				}
			}
			else if(ret0>0)//+I之前有CMD
			{
				g_stWifiUart.CMD_cpy_num=ret0;
				sysLOG(WIFI_LOG_LEVEL_4, "g_stWifiUart.uart_buff_count!=0 && dev_wifiWaitStringData1(\"+I\") ret0:%d\r\n", ret0);
			}			
			else//查找最后一个字节是不是"+"
			{
				uint32 uart_buff_end;
				if(g_stWifiUart.write_P==0)uart_buff_end=UARTBUFF_LEN-1;
				else uart_buff_end=g_stWifiUart.write_P-1;
				if(g_stWifiUart.uart_buff[uart_buff_end]=='+')//最后一个字节是+
				{
					if(g_stWifiUart.uart_buff_count >= 1)
					{
						g_stWifiUart.CMD_cpy_num=g_stWifiUart.uart_buff_count - 1;
					}
					else
					{
						g_stWifiUart.CMD_cpy_num = 0;
					}
				}
				else
				{
					g_stWifiUart.CMD_cpy_num=g_stWifiUart.uart_buff_count;
				}
				sysLOG(WIFI_LOG_LEVEL_4, "g_stWifiUart.uart_buff_count!=0,CMD_cpy_num:%d,uart_buff_count:%d\r\n", g_stWifiUart.CMD_cpy_num, g_stWifiUart.uart_buff_count);
				if(g_stWifiUart.CMD_cpy_num == 0)
				{
					if(g_stWifiUart.uart_buff_count <= 4)//后面只有+IPD的一部分,退出copy
					{
						sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
						break;
					}
				}
			}
		}
		else//后续没有数据则退出copy
		{		
			sysLOG(WIFI_LOG_LEVEL_4, " dev_cpydata_IPD, break!\r\n");
			break;
		}
					
		if(g_stWifiUart.CMD_cpy_num!=0)
		{
			if((CMDUARTBUFF_LEN-g_stWifiUart.CMD_write_P)>=g_stWifiUart.CMD_cpy_num)//CMD_buf能存下
			{
				//ysLOG_lib(WIFI_LOG_LEVEL_4, " 1888\r\n");
				if((g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num)<UARTBUFF_LEN)
				{
					sysLOG(WIFI_LOG_LEVEL_4, " %d %d %d %d\r\n",g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num,g_stWifiUart.read_P,g_stWifiUart.write_P,g_stWifiUart.CMD_cpy_num);
					sysLOG(WIFI_LOG_LEVEL_4, " %d %d %d\r\n",g_stWifiUart.CMD_write_P,g_stWifiUart.CMD_read_P,g_stWifiUart.uart_buff_count);
					memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P],&g_stWifiUart.uart_buff[g_stWifiUart.read_P],g_stWifiUart.CMD_cpy_num);
					sysLOG(WIFI_LOG_LEVEL_4, " %d\r\n",g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num);
					memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,g_stWifiUart.CMD_cpy_num);
					g_stWifiUart.CMD_write_P+=g_stWifiUart.CMD_cpy_num;
					g_stWifiUart.read_P+=g_stWifiUart.CMD_cpy_num;
					g_stWifiUart.uart_buff_count-=g_stWifiUart.CMD_cpy_num;
					sysLOG(WIFI_LOG_LEVEL_4, " %d\r\n",g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num);
				}
				else
				{
					sysLOG(WIFI_LOG_LEVEL_4, " %d\r\n",g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num);
					memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P],&g_stWifiUart.uart_buff[g_stWifiUart.read_P],UARTBUFF_LEN-g_stWifiUart.read_P);
					memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,UARTBUFF_LEN-g_stWifiUart.read_P);
					memcpy(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P+UARTBUFF_LEN-g_stWifiUart.read_P],&g_stWifiUart.uart_buff[0],g_stWifiUart.CMD_cpy_num-(UARTBUFF_LEN-g_stWifiUart.read_P));
					memset(&g_stWifiUart.uart_buff[0],0,g_stWifiUart.CMD_cpy_num-(UARTBUFF_LEN-g_stWifiUart.read_P));
					g_stWifiUart.CMD_write_P+=g_stWifiUart.CMD_cpy_num;
					g_stWifiUart.read_P=0+g_stWifiUart.CMD_cpy_num-(UARTBUFF_LEN-g_stWifiUart.read_P);
					g_stWifiUart.uart_buff_count-=g_stWifiUart.CMD_cpy_num;
					sysLOG(WIFI_LOG_LEVEL_4, " %d\r\n",g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num);
				}
			}
			else
			{
				sysLOG(WIFI_LOG_LEVEL_4, " %d\r\n",CMDUARTBUFF_LEN-g_stWifiUart.CMD_write_P);
				if((g_stWifiUart.read_P+g_stWifiUart.CMD_cpy_num)<UARTBUFF_LEN)
				{
					memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,g_stWifiUart.CMD_cpy_num);
					g_stWifiUart.read_P+=g_stWifiUart.CMD_cpy_num;
					g_stWifiUart.uart_buff_count-=g_stWifiUart.CMD_cpy_num;
				}
				else
				{
					memset(&g_stWifiUart.uart_buff[g_stWifiUart.read_P],0,UARTBUFF_LEN-g_stWifiUart.read_P);
					memset(&g_stWifiUart.uart_buff[0],0,g_stWifiUart.CMD_cpy_num-(UARTBUFF_LEN-g_stWifiUart.read_P));
					g_stWifiUart.read_P=0+g_stWifiUart.CMD_cpy_num-(UARTBUFF_LEN-g_stWifiUart.read_P);
					g_stWifiUart.uart_buff_count-=g_stWifiUart.CMD_cpy_num;
				}
				//sysLOG(WIFI_LOG_LEVEL_4, "1uart_s.uart_buff_count!=0,CMD_buff:%s\r\n", &g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_write_P-g_stWifiUart.CMD_cpy_num]);
			}
		}			
	}
	
}

int wifiReadDatadProcessing(char *data, uint32 datalen, int8 *IPD_uart_buff, uint32 *IPD_uart_buff_len, uint32 *IPD_read_P, uint32  i32Timeout)
{
	unsigned long long uiTime;
	int32 ret=-1;
	uiTime = hal_sysGetTickms() + i32Timeout;
			
	while(1)
	{
		DR_IPDmutex_lock(g_ui32IPDMutexHandle);
		sysLOG(APIWIFI_LOG_LEVEL_4, "<END> fibo_mutex_lock g_ui32IPDMutexHandle\r\n");
		if(g_ui8WifiConnectClosed==0)
		{
			sysLOG(APIWIFI_LOG_LEVEL_4, "<END> fibo_mutex_lock g_ui32IPDMutexHandle\r\n");
			//sysLOG(2,"--dev_wifiReadData--start!\r\n");
			if(*IPD_uart_buff_len>=datalen)  //接收数据
			{
				if(datalen<(IPDUARTBUFF_LEN-*IPD_read_P))
				{
					memcpy(data,&IPD_uart_buff[*IPD_read_P],datalen);
					memset(&IPD_uart_buff[*IPD_read_P],0,datalen);
					*IPD_read_P+=datalen;
					*IPD_uart_buff_len-=datalen;
					//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
				}	
				else
				{
					memcpy(data,&IPD_uart_buff[*IPD_read_P],IPDUARTBUFF_LEN-*IPD_read_P);
					memset(&IPD_uart_buff[*IPD_read_P],0,IPDUARTBUFF_LEN-*IPD_read_P);
					memcpy(data+IPDUARTBUFF_LEN - *IPD_read_P,&IPD_uart_buff[0],datalen-(IPDUARTBUFF_LEN-*IPD_read_P));
					memset(&IPD_uart_buff[0],0,datalen-(IPDUARTBUFF_LEN-*IPD_read_P));
					*IPD_read_P=datalen-(IPDUARTBUFF_LEN-*IPD_read_P);
					*IPD_uart_buff_len-=datalen;
					//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
				}
				ret=datalen;
				
				break;  //正常接收到数据断掉
			}
			sysLOG(WIFI_LOG_LEVEL_4, "hal_sysGetTickms i32Timeout\r\n");				
			if(hal_sysGetTickms()>=uiTime)
			{
				sysLOG(WIFI_LOG_LEVEL_4, "hal_sysGetTickms i32Timeout\r\n");				
				if(*IPD_uart_buff_len>0)
				{
					if(*IPD_uart_buff_len>=datalen)//需要的数据小于等于存在的数据，则返回需要的个数即可
					{
						if(datalen<(IPDUARTBUFF_LEN-*IPD_read_P))
						{
							memcpy(data,&IPD_uart_buff[*IPD_read_P],datalen);
							memset(&IPD_uart_buff[*IPD_read_P],0,datalen);
							*IPD_read_P+=datalen;
							*IPD_uart_buff_len-=datalen;
							//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
						}	
						else
						{
							memcpy(data,&IPD_uart_buff[*IPD_read_P],IPDUARTBUFF_LEN-*IPD_read_P);
							memset(&IPD_uart_buff[*IPD_read_P],0,IPDUARTBUFF_LEN-*IPD_read_P);
							memcpy(data+IPDUARTBUFF_LEN-*IPD_read_P,&IPD_uart_buff[0],datalen-(IPDUARTBUFF_LEN-*IPD_read_P));
							memset(&IPD_uart_buff[0],0,datalen-(IPDUARTBUFF_LEN-*IPD_read_P));
							*IPD_read_P=datalen-(IPDUARTBUFF_LEN-*IPD_read_P);
							*IPD_uart_buff_len-=datalen;
							//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
						}
						
						ret=datalen;
					}
					else//需要的字节数大于存在的个数，则返回全部即可
					{
						if(*IPD_uart_buff_len<(IPDUARTBUFF_LEN-*IPD_read_P))
						{
							memcpy(data,&IPD_uart_buff[*IPD_read_P],*IPD_uart_buff_len);
							memset(&IPD_uart_buff[*IPD_read_P],0,*IPD_uart_buff_len);
							ret=*IPD_uart_buff_len;
							*IPD_read_P+=*IPD_uart_buff_len;
							*IPD_uart_buff_len-=*IPD_uart_buff_len;
							//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
						}
						else
						{
							memcpy(data,&IPD_uart_buff[*IPD_read_P],IPDUARTBUFF_LEN-*IPD_read_P);
							memset(&IPD_uart_buff[*IPD_read_P],0,IPDUARTBUFF_LEN-*IPD_read_P);
							memcpy(data+IPDUARTBUFF_LEN-*IPD_read_P,&IPD_uart_buff[0],*IPD_uart_buff_len-(IPDUARTBUFF_LEN-*IPD_read_P));
							memset(&IPD_uart_buff[0],0,*IPD_uart_buff_len-(IPDUARTBUFF_LEN-*IPD_read_P));
							ret=*IPD_uart_buff_len;
							*IPD_read_P=*IPD_uart_buff_len-(IPDUARTBUFF_LEN-*IPD_read_P);
							*IPD_uart_buff_len-=*IPD_uart_buff_len;
							//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
						}
					}
					
				}
				else
				{
					ret=0;
				}
				
				sysDelayMs(5);
				break;
				
			}
		}
		else
		{
			g_ui8WifiConnectClosed=0;
			
			if(*IPD_uart_buff_len>0)
			{
				if(*IPD_uart_buff_len>=datalen)//需要的数据小于等于存在的数据，则返回需要的个数即可
				{
					if(datalen<(IPDUARTBUFF_LEN-*IPD_read_P))
					{
						memcpy(data,&IPD_uart_buff[*IPD_read_P],datalen);
						memset(&IPD_uart_buff[*IPD_read_P],0,datalen);
						*IPD_read_P+=datalen;
						*IPD_uart_buff_len-=datalen;
						//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
					}	
					else
					{
						memcpy(data,&IPD_uart_buff[*IPD_read_P],IPDUARTBUFF_LEN-*IPD_read_P);
						memset(&IPD_uart_buff[*IPD_read_P],0,IPDUARTBUFF_LEN-*IPD_read_P);
						memcpy(data+IPDUARTBUFF_LEN-*IPD_read_P,&IPD_uart_buff[0],datalen-(IPDUARTBUFF_LEN-*IPD_read_P));
						memset(&IPD_uart_buff[0],0,datalen-(IPDUARTBUFF_LEN-*IPD_read_P));
						*IPD_read_P=datalen-(IPDUARTBUFF_LEN-*IPD_read_P);
						*IPD_uart_buff_len-=datalen;
						//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
					}
					
					ret=datalen;
				}
				else//需要的字节数大于存在的个数，则返回全部即可
				{
					if(*IPD_uart_buff_len<(IPDUARTBUFF_LEN-*IPD_read_P))
					{
						memcpy(data,&IPD_uart_buff[*IPD_read_P],*IPD_uart_buff_len);
						memset(&IPD_uart_buff[*IPD_read_P],0,*IPD_uart_buff_len);
						ret=*IPD_uart_buff_len;
						*IPD_read_P+=*IPD_uart_buff_len;
						*IPD_uart_buff_len-=*IPD_uart_buff_len;
						//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
					}
					else
					{
						memcpy(data,&IPD_uart_buff[*IPD_read_P],IPDUARTBUFF_LEN-*IPD_read_P);
						memset(&IPD_uart_buff[*IPD_read_P],0,IPDUARTBUFF_LEN-*IPD_read_P);
						memcpy(data+IPDUARTBUFF_LEN-*IPD_read_P,&IPD_uart_buff[0],*IPD_uart_buff_len-(IPDUARTBUFF_LEN-*IPD_read_P));
						memset(&IPD_uart_buff[0],0,*IPD_uart_buff_len-(IPDUARTBUFF_LEN-*IPD_read_P));
						ret=*IPD_uart_buff_len;
						*IPD_read_P=*IPD_uart_buff_len-(IPDUARTBUFF_LEN-*IPD_read_P);
						*IPD_uart_buff_len-=*IPD_uart_buff_len;
						//sysLOG(APIWIFI_LOG_LEVEL_1, "wifiReadDatadProcessing datalen=%d\r\n",datalen);
					}
				}
			}
			else
			{
				ret=0;
			}
			break;
		}
		DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
		sysDelayMs(10);
	}
	DR_IPDmutex_unlock(g_ui32IPDMutexHandle);
				
	return ret;
}

//+IPD,12:123456789abc
int dev_wifiReadData(int i32Sockid, char *data, uint32 datalen,uint32 i32Timeout)
{
	int32 ret=-1;

	if(i32Sockid == 0)
	{
		ret = wifiReadDatadProcessing(data, datalen, g_stWifiUart.IPD_zero_uart_buff, &g_stWifiUart.IPD_zero_uart_buff_len, &g_stWifiUart.IPD_zero_read_P, i32Timeout);
	}
	else if(i32Sockid == 1)
	{
		ret = wifiReadDatadProcessing(data, datalen, g_stWifiUart.IPD_one_uart_buff, &g_stWifiUart.IPD_one_uart_buff_len, &g_stWifiUart.IPD_one_read_P, i32Timeout);
	}
	else if(i32Sockid == 2)
	{
		ret = wifiReadDatadProcessing(data, datalen, g_stWifiUart.IPD_two_uart_buff, &g_stWifiUart.IPD_two_uart_buff_len, &g_stWifiUart.IPD_two_read_P, i32Timeout);
	}
	else if(i32Sockid == 3)
	{
		ret = wifiReadDatadProcessing(data, datalen, g_stWifiUart.IPD_three_uart_buff, &g_stWifiUart.IPD_three_uart_buff_len, &g_stWifiUart.IPD_three_read_P, i32Timeout);
	}
	else if(i32Sockid == 4)
	{
		ret = wifiReadDatadProcessing(data, datalen, g_stWifiUart.IPD_four_uart_buff, &g_stWifiUart.IPD_four_uart_buff_len, &g_stWifiUart.IPD_four_read_P, i32Timeout);
	}
    
	return ret;
}


int8 Wifi_Set_MACAddr(char *data)
{
	unsigned char buf[100];
	unsigned char aucMac[64];
	int ret;
	
	Wifi_wkup(TRUE);


	/*设置MAC*/
	memset(aucMac, 0x00, sizeof(aucMac));			/*wifi芯片的MAC与蓝牙芯片复用，注意格式*/
	memset(buf, 0, sizeof(buf));
	//ret = hal_blueToothMakeMac(buf);
	buf[0]=*data;
	buf[1]=*(data+1);
	buf[2]=*(data+2);
	buf[3]=*(data+3);
	buf[4]=*(data+4);
	buf[5]=*(data+5);
	//if(ret != 0)
	//{
	//	return ret;
	//}
	memcpy(aucMac, "AT+CIPSTAMAC_DEF=\"", 18);
    aucMac[18] = HexToAscii(buf[0] >> 4);
    aucMac[19] = HexToAscii((buf[0] & 0x0E));			/*esp8266 MAC地址第一个字节的bit0不能为1*/
    aucMac[20] = ':';
    
    aucMac[21] = HexToAscii(buf[1] >> 4);
    aucMac[22] = HexToAscii(buf[1] & 0x0f);
    aucMac[23] = ':';
    
    aucMac[24] = HexToAscii(buf[2] >> 4);
    aucMac[25] = HexToAscii(buf[2] & 0x0f);
    aucMac[26] = ':';
    
    aucMac[27] = HexToAscii(buf[3] >> 4);
    aucMac[28] = HexToAscii(buf[3] & 0x0f);
    aucMac[29] = ':';
    
    aucMac[30] = HexToAscii(buf[4] >> 4);
    aucMac[31] = HexToAscii(buf[4] & 0x0f);
    aucMac[32] = ':';
        
    aucMac[33] = HexToAscii(buf[5] >> 4);
    aucMac[34] = HexToAscii(buf[5] & 0x0f);	
	aucMac[35] = '\"';
	aucMac[36] = '\r';
	aucMac[37] = '\n';

	memset(buf, 0, sizeof(buf));
    ret = ESP_SendAndWaitCmd((char *)aucMac, 38, (char *)buf, 100, 500, 3, "\r\nOK\r\n");
    if (ret < 0)
    {
    	Wifi_wkup(FALSE);
		return ret;
    }
	Wifi_wkup(FALSE);
}

void wifi_setmacaddr_test(void)
{
	int8 mactemp[10]={0x00,0x11,0x22,0x33,0x44,0x55};
	Wifi_Set_MACAddr(mactemp);
}

char  HexToAscii(unsigned char    data_hex)
{
 char  ASCII_Data;
 ASCII_Data=data_hex & 0x0F;
 if(ASCII_Data<10) 
  ASCII_Data=ASCII_Data+0x30;//‘0--9’
 else  
  ASCII_Data=ASCII_Data+0x37;//‘A--F’
 return ASCII_Data;
}

int Search_StringBInStringA(char *pcStrA, char *pcStrB)
{
	char *pcStr = NULL;
    
	pcStr = strstr(pcStrA, pcStrB);
	if(pcStr == NULL)
	{
		return -1;
	}

	return (pcStr - pcStrA);
}

/*
*@brief:airkiss和web配网中查状态
*@return:4-等待进入配网；0-准备配网，应用语音播报配网；1-正在联网；2-已经联网成功；3-超时并退出；//WiFi固件升级后应用在做相应的修改
*
*/
int8 AP_connect_status=4;
volatile int8 AP_connect_check_flag=0;//进入APconnect_type后，要等到有返回值在进入AP_connect_check，不然就把APconnect_type返回结果给清掉了
int AP_connect_check(void)
{
	//int ret;
	uint8 bufrP=0;
	int8 *rP=NULL;
	int8 buf[100];
	int	ap_sta=4;
	unsigned long long uiTime = 0;
	uint32 i32Timeout=500;
	int32 i32Ret = 0;
	//uint32 write_P_temp=0;

	if(NO_OPEN == g_stWifiState.cOpenState)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> WIFI_NOT_OPEN_ERR\r\n");
		return WIFI_NOT_OPEN_ERR;
	}
	
	switch(AP_connect_check_flag)
	{
		case 0:
	
			uiTime = hal_sysGetTickms() + i32Timeout;
#if 0	
			if(g_stWifiUart.write_P>5*1024)
			{
				write_P_temp=g_stWifiUart.write_P;
				g_stWifiUart.write_P=100;
				memcpy(g_stWifiUart.uart_buff,&g_stWifiUart.uart_buff[write_P_temp-100],100);
			}
#endif	
			while(1)
			{
				sysDelayMs(2);
				if(AP_connect_check_flag!=0)//在已经进入一种配网等待状态期间，切换配网类型，所以需要随时break;
				{
					break;
				}
				
				if(hal_sysGetTickms() > (APTimecount+16384*APtypetimeout))//超时如果收不到WiFi的退出配网则主动返回退出值
				{
					APconnect_quit();
					Wifi_wkup(FALSE);
					return 3;
				}
				
				i32Ret = dev_wifiWaitString("OK");
				if(i32Ret >= 0)
				{
					if((i32Ret)<90)
					{
						memcpy(buf,&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P],i32Ret+2);
						memset(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P],0,i32Ret+2);
						g_stWifiUart.CMD_read_P+=i32Ret+2;
						//memcpy(buf,&g_stWifiUart.uart_buff[g_stWifiUart.read_P],100);
					}
					else
					{
						memcpy(buf,&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P+i32Ret-90],90+2);
						memset(&g_stWifiUart.CMD_uart_buff[g_stWifiUart.CMD_read_P+i32Ret-90],0,90+2);
						g_stWifiUart.CMD_read_P+=90+2;
						//memcpy(buf,&g_stWifiUart.uart_buff[g_stWifiUart.read_P+i32Ret-90],100);
					}
					sysLOG(WIFI_LOG_LEVEL_4, "AP_connect_check1 ok,i32Ret :%d,g_stWifiUart.CMD_read_P:%d, recv is %s\r\n", i32Ret, g_stWifiUart.CMD_read_P, buf);
					
					
					break;
				}
				
				if(hal_sysGetTickms() >= uiTime)
				{
					sysLOG(WIFI_LOG_LEVEL_3, "cmd hal_sysGetTickms i32Timeout!=%d\r\n",uiTime);
					if((AP_connect_status == 2) || (AP_connect_status == 3))//联网成功或者超时退出则进休眠
					{
						APconnect_quit();
						Wifi_wkup(FALSE);
					}
					return AP_connect_status;
				}			
			}
			
			//sysLOG(WIFI_LOG_LEVEL_2, "wait cmd buf:%s\r\n", buf);
			
			rP=myStrStr(buf,"+ConfigCheck:",0,99);
			if(rP==NULL)
			{
				if((AP_connect_status == 2) || (AP_connect_status == 3))//联网成功或者超时退出则进休眠
				{
					APconnect_quit();
					Wifi_wkup(FALSE);
				}
				return AP_connect_status;
			}
			
			bufrP=rP-buf;
			ap_sta=buf[bufrP+14]-0x30;
			AP_connect_status=ap_sta;
			if(AP_connect_status == 1)
			{
				APTimecount += 16384*APtypetimeout;//重新获得配网开始时间；
			}
			sysLOG(WIFI_LOG_LEVEL_3, "AP_connect_check_flag:%d,AP_connect_status:%d\r\n", AP_connect_check_flag, AP_connect_status);
		break;
		
		case 1:
			sysLOG(WIFI_LOG_LEVEL_3, "wait AP_connect_check_flag:%d\r\n", AP_connect_check_flag);
			ap_sta = 4;
		break;
		
		default:
			ap_sta=4;
		break;
	}
	if((AP_connect_status == 2) || (AP_connect_status == 3))//联网成功或者超时退出则进休眠
	{
		APconnect_quit();
		Wifi_wkup(FALSE);
	}
	return ap_sta;
}





/*
* Description: 热点配置AT+WEBSERVER,AT+AIRKISS,AT+BT
* Input: @param1:AP_AIRKISS,AP_WEB,AP_BT，@param2:超时时间
* Output:N
* Return: <0:初始化失败
*/


int APconnect_type(uint8 connect_type,uint32 i32Timeout)
{
	int8 buf[100];
	int8 sendbuf[100];
	int8 AP_name[20];
	int8 AP_keys[64];
    int ret = -1;
	uint8 SN_len = 0;

	int i32Ret = -1;
	
	if(NO_OPEN == g_stWifiState.cOpenState)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "WIFI_NOT_OPEN_ERR\r\n");
		
		return WIFI_NOT_OPEN_ERR;
	}
	
	g_stWifiConnect.apconnect_type_flag=0;
	memset(buf,0,sizeof(buf));
	memset(sendbuf,0,sizeof(sendbuf));

	memset(AP_name,0,sizeof(AP_name));
	memset(AP_keys,0,sizeof(AP_keys));
	
	i32Ret = hal_sysSetSleepMode(0);//ccb
	sysLOG(WIFI_LOG_LEVEL_4, "hal_sysSetSleepMode(0), i32Ret:%d\r\n", i32Ret);
	Wifi_wkup(TRUE);	
	AP_connect_status=4;
	
	//hal_espAPClose();
	switch (connect_type)
	{
		case AP_AIRKISS:
			AP_connect_check_flag=1;
			sysLOG(WIFI_LOG_LEVEL_4, "AP_AIRKISS,AP_connect_check_flag:%d\r\n", AP_connect_check_flag);
			
			SN_len = hal_nvReadSN(AP_name);
			if(SN_len < 0)
			{
				memset(AP_name, 0, sizeof(AP_name));
				memcpy(AP_name, "vanstone", sizeof("vanstone"));
			}
			else
			{
				if(SN_len > 16)
				{
					for(uint8 i = 0; i < (SN_len - 16); i++)
					{
						BuffLeftMove(AP_name, 20);
					}
					memset(&AP_name[16], 0, 20-16);
				}
				else if(SN_len < 16)
				{
					sysLOG(WIFI_LOG_LEVEL_4, "SN_len <= 16,SN_len:%d\r\n", SN_len);
					for(uint8 i = 0; i < (16 - SN_len); i++)
					{
						BuffRightMove(AP_name, 20);
					}
					memset(&AP_name[16], 0, 20-16);
					memset(&AP_name[0], 0x30, 16 - SN_len);
				
				}
			}
			sysLOG(WIFI_LOG_LEVEL_4, "Airkiss AP_name:%s\r\n", AP_name);		

			sprintf(sendbuf,"AT+AIRKISS=%d,\"%s\"\r\n",i32Timeout, AP_name);
			//sprintf(sendbuf,"AT+AIRKISS\r\n");
			memset(buf, 0x00, sizeof(buf));
			ret = ESP_SendAndWaitCmd(sendbuf, strlen(sendbuf), buf, 50, 1500, 3, "\r\nOK\r\n");
		    if(ret < 0)
		    {
		    	sysLOG(WIFI_LOG_LEVEL_2, "airkiss ret is %d\r\n", ret);
				hal_espSleepNotice();
				i32Ret = hal_sysSetSleepMode(1);//ccb
				sysLOG(APIWIFI_LOG_LEVEL_2, "hal_sysSetSleepMode(1) i32Ret=%d\r\n", i32Ret);
				Wifi_wkup(FALSE);
				AP_connect_check_flag=0;
		        return ret;
		    }

			g_stWifiConnect.apconnect_type_flag=AP_AIRKISS;
			g_stWifiConnect.apconnect_count=0;
			g_stWifiConnect.apconnect_timeout=i32Timeout;
			g_stWifiConnect.apconnect_time=0;
			g_stWifiConnect.apconnected_flag=0;
			g_stWifiConnect.apconnect_timeout_flag=0;
			sysLOG(WIFI_LOG_LEVEL_1, "AIRKISS connecting...\r\n");

		break;
		case AP_WEB:
			AP_connect_check_flag=1;
			sysLOG(WIFI_LOG_LEVEL_4, "AP_WEB,AP_connect_check_flag:%d\r\n", AP_connect_check_flag);
			if(hal_nvReadSN(AP_name)<0)
			{
				memset(AP_name,0,sizeof(AP_name));
				memcpy(AP_name,"vanstone",sizeof("vanstone"));
			}
			//memcpy(AP_keys,"12345678",sizeof("12345678"));
			sprintf(sendbuf,"AT+WEBSERVER=%d,\"%s\"\r\n",i32Timeout,AP_name);
			//sprintf(sendbuf,"AT+WEBSERVER=\"%s\"\r\n",AP_name);
			memset(buf, 0x00, sizeof(buf));
			ret = ESP_SendAndWaitCmd(sendbuf, strlen(sendbuf),buf,50,1500,3,"\r\nOK\r\n");
			//ret = ESP_SendAndWaitCmd("AT+WEBSERVER\r\n", strlen("AT+WEBSERVER\r\n"), buf, 50, 50000, 1, "\r\nOK\r\n");
			if(ret < 0)
			{
				sysLOG(WIFI_LOG_LEVEL_2, "web AP ret is %d\r\n", ret);
				hal_espSleepNotice();
				i32Ret = hal_sysSetSleepMode(1);//ccb
				sysLOG(APIWIFI_LOG_LEVEL_2, "hal_sysSetSleepMode(1) i32Ret=%d\r\n", i32Ret);
				Wifi_wkup(FALSE);
				AP_connect_check_flag=0;
				return ret;
			}
			g_stWifiConnect.apconnect_type_flag=AP_WEB;
			g_stWifiConnect.apconnect_count=0;
			g_stWifiConnect.apconnect_timeout=i32Timeout;
			g_stWifiConnect.apconnect_time=0;
			g_stWifiConnect.apconnected_flag=0;
			g_stWifiConnect.apconnect_timeout_flag=0;
			sysLOG(WIFI_LOG_LEVEL_2, "WEB connecting...\r\n");
		break;
		default:
			ret = -2;
		break;
	}
	//Wifi_wkup(FALSE);//成功进入配网模式后不进休眠，直到退出配网模式或者配网成功、配网超时再进休眠
	APTimecount = hal_sysGetTickms();//获得配网开始时间；
	APtypetimeout = i32Timeout + 5;
	AP_connect_check_flag=0;
	return ret;
}

/*
*@brief:退出配网
*@return:>=0-成功；<0-失败
*/
int APconnect_quit(void)
{
	char buf[100];
	int ret=0;
	int i32Ret;
	if(g_stWifiConnect.apconnect_type_flag != AP_BT)
	{
		Wifi_wkup(TRUE);
	    memset(buf, 0, sizeof(buf));
		AP_connect_check_flag=1;
	    ret = ESP_SendAndWaitCmd("AT+QUIT\r\n", strlen("AT+QUIT\r\n"), (char *)buf, 100, 500, 3, "\r\nOK\r\n");
	    if (ret < 0)
	    {
	    	hal_espSleepNotice();
	    	i32Ret = hal_sysSetSleepMode(1);//ccb
	    	sysLOG(APIWIFI_LOG_LEVEL_2, "hal_sysSetSleepMode(1) i32Ret=%d\r\n", i32Ret);
	    	Wifi_wkup(FALSE);
			AP_connect_check_flag==0;
			return ret;
	    }
	}
	else
	{
		if(AP_connect_status != 2)//蓝牙配网失败重启下WiFi
		{
			wifiReset_lib();
		}
		DR_BTPwrCtl(FALSE);
	}
	hal_espSleepNotice();
	i32Ret = hal_sysSetSleepMode(1);//ccb
	sysLOG(APIWIFI_LOG_LEVEL_4, "hal_sysSetSleepMode(1) i32Ret=%d\r\n", i32Ret);
	Wifi_wkup(FALSE);
	AP_connect_check_flag=0;
	sysDelayMs(500);
	return ret;
}

int APconnect_check(void)
{
	int ret=-1;
	
	if(g_stWifiConnect.apconnect_type_flag!=0)
	{
		g_stWifiConnect.apconnect_count++;
		if(g_stWifiConnect.apconnect_count>20*2)
		{
			g_stWifiConnect.apconnect_count=0;
			g_stWifiConnect.apconnect_time+=1;
			
			memset(&g_stWifiConnect.stAp, 0x00, sizeof(g_stWifiConnect.stAp));
			ret = hal_espCheckAp(&g_stWifiConnect.stAp);
			sysLOG(WIFI_LOG_LEVEL_1, "AP checking ...,ret :%d\r\n", ret);
			if(ret >= 0)
			{
					g_stWifiConnect.apconnect_type_flag=0;
					g_stWifiConnect.apconnected_flag=1;
					g_stWifiConnect.apconnect_time=0;
					g_stWifiConnect.apconnect_timeout=0;
//#if TTS_DEBUG_ENABLE		
//					sysLOG(WIFI_LOG_LEVEL_2, "wifi AP set ok\r\n");
//					//tts_play_send("网络配置成功");
//					
//#endif					
					return 0;
			}
			else
			{
				if(g_stWifiConnect.apconnect_time*2>=g_stWifiConnect.apconnect_timeout)
				{
					sysLOG(WIFI_LOG_LEVEL_2, "ap connect error!ret :%d\r\n", ret);
					g_stWifiConnect.apconnect_type_flag=0;
					g_stWifiConnect.apconnected_flag=0;
					g_stWifiConnect.apconnect_time=0;
					g_stWifiConnect.apconnect_timeout=0;
					g_stWifiConnect.apconnect_timeout_flag=1;
//#if TTS_DEBUG_ENABLE
//					sysLOG(WIFI_LOG_LEVEL_2, "wifi AP set error\r\n");
//
//					//tts_play_send("网络配置失败");
//					
//#endif
				}
				
				
			}
		}
	}
	return ret;
}

int APstatus_Check(void)
{
	int ret=-1;

	if(g_stWifiConnect.apconnect_checkflag==1)
	{
		g_stWifiConnect.apconnect_checkcount++;
		if(g_stWifiConnect.apconnect_checkcount>20*2)
		{
			g_stWifiConnect.apconnect_checkcount=0;
			g_stWifiConnect.apconnect_checktime+=1;
			
			memset(&g_stWifiConnect.checkstAp, 0x00, sizeof(g_stWifiConnect.checkstAp));
			ret = hal_espCheckAp(&g_stWifiConnect.checkstAp);
			sysLOG(WIFI_LOG_LEVEL_1, "AP checking ...,ret :%d\r\n", ret);
			if(ret >= 0)
			{
					g_stWifiConnect.apconnected_flag=1;
					g_stWifiConnect.apconnect_checkflag=0;
					g_stWifiConnect.apconnect_checktime=0;
					g_stWifiConnect.apconnect_checktimeout=0;
//#if TTS_DEBUG_ENABLE	
//					sysLOG(WIFI_LOG_LEVEL_2, "wifi connected\r\n");
//
//					//tts_play_send("网络已连接");
//					
//#endif
					return 0;
			}
			else
			{
				if(g_stWifiConnect.apconnect_checktime*2>=g_stWifiConnect.apconnect_checktimeout)
				{
					sysLOG(WIFI_LOG_LEVEL_2, "ap connect error!ret :%d\r\n", ret);

					g_stWifiConnect.apconnected_flag=0;
					g_stWifiConnect.apconnect_checkflag=0;
					g_stWifiConnect.apconnect_checktime=0;
					g_stWifiConnect.apconnect_checktimeout=0;
					g_stWifiConnect.apconnect_checktimeout_flag=1;
//#if TTS_DEBUG_ENABLE
//					sysLOG(WIFI_LOG_LEVEL_2, "wifi disconnected\r\n");
//
//					//tts_play_send("网络连接失败");
//					
//#endif
				}
				
				
			}
		}
	}
	return ret;
}


#if MAINTEST_FLAG


int WifiTest_Connect(void)
{
	int8 auccSsid[64]={0};
    char aucPassWord[32];

    int i32Ret = 0;
	int32 Ret=0;
    int i = 0;
    //int j = 0;
	ST_AP_LIST stAp[30];
//    ST_AP_INFO stApInfo;
//	ST_AP_MAC stApMac;
//	ST_WIFI_PARAM stWifiParam;
	
	sprintf(auccSsid,"WIN123+*");

    i32Ret = wifiOpen_lib();
    if(i32Ret)
    {
    	sysLOG(WIFI_LOG_LEVEL_2, "Wifi Open Err:%d\r\n", i32Ret);
		return -1;
    }
    else
    {
    	sysLOG(WIFI_LOG_LEVEL_2, "Wifi Open Success\r\n");
    }
	while(1)
	{

	    i32Ret = wifiScan_lib(stAp, 30);
		sysLOG(WIFI_LOG_LEVEL_2, "wifiScan_lib number is %d\r\n", i32Ret);
		if(i32Ret <= 0)
		{
			sysLOG(WIFI_LOG_LEVEL_2, "Wifi Scan err %d\r\n", i32Ret);
			return -2;
		}
		for(i=0; i<i32Ret; i++)
		{
			sysLOG(WIFI_LOG_LEVEL_2, "i=%d, stAp[i].cSsid:%s,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", i, stAp[i].cSsid,\
					stAp[i].cSsid[0], stAp[i].cSsid[1], stAp[i].cSsid[2], stAp[i].cSsid[3], stAp[i].cSsid[4], stAp[i].cSsid[5], stAp[i].cSsid[6], stAp[i].cSsid[7], stAp[i].cSsid[8], stAp[i].cSsid[9]);
		}


	    for(i = 0; i < i32Ret; i++)
	    {
	    	sysLOG(WIFI_LOG_LEVEL_2, "%d.%-16.16s  %ddBm\r\n", i+1, stAp[i].cSsid, stAp[i].iRssi);
			
	        Ret=memcmp(stAp[i].cSsid,auccSsid,strlen(auccSsid));
			sysLOG(WIFI_LOG_LEVEL_2, "Ret is %d\r\n", Ret);
			if(Ret==0)
	        {
	        	memset(aucPassWord,0,sizeof(aucPassWord));
				memcpy(aucPassWord, "aaaaaaaa", strlen("aaaaaaaa"));
				//memcpy(aucPassWord, "12345678", strlen("12345678"));
				sysLOG(WIFI_LOG_LEVEL_2, "connect..\r\n");
				sysLOG(WIFI_LOG_LEVEL_2, "cSsid is %s,password is %s\r\n", stAp[i].cSsid,(char*)aucPassWord);

				if(0 == wifiAPConnect_lib(stAp[i].cSsid, (char*)aucPassWord))
	            {
	               	sysLOG(WIFI_LOG_LEVEL_2, "%s Connect OK\r\n", stAp[i].cSsid);
					return 0;
		   		}
				else
				{
					sysLOG(WIFI_LOG_LEVEL_2, "%s connect err\r\n", stAp[i].cSsid);
					break;
				}
			}
			else
			{
				sysLOG(WIFI_LOG_LEVEL_2, "AP sCan error,can not find the AP named %s\r\n", auccSsid);
			}
			
			
		}

		sysDelayMs(100);
	}

}



uint8 tcp_wifi_testbuf[150]={0x47,0x45,0x54,0x20,0x2F,0x73,0x63,0x66,0x69,0x6C,0x65,0x73,0x2F,0x41,0x70,0x70,0x46,0x69,0x6C,0x65,\
						0x2F,0x56,0x61,0x6E,0x73,0x74,0x6F,0x6E,0x65,0x2F,0x32,0x30,0x32,0x30,0x30,0x32,0x30,0x38,0x2F,0x73,0x61,\
						0x6D,0x70,0x6C,0x65,0x5F,0x61,0x70,0x70,0x5F,0x74,0x74,0x73,0x5F,0x72,0x65,0x6C,0x65,0x61,0x73,0x65,0x5F,\
						0x66,0x6F,0x74,0x61,0x2E,0x62,0x69,0x6E,0x20,0x48,0x54,0x54,0x50,0x2F,0x31,0x2E,0x31,0x0D,0x0A,0x48,0x6F,\
						0x73,0x74,0x3A,0x20,0x73,0x79,0x73,0x30,0x30,0x2E,0x6A,0x69,0x65,0x77,0x65,0x6E,0x2E,0x63,0x6F,0x6D,0x2E,\
						0x63,0x6E,0x0D,0x0A,0x0D,0x0A};


int server_connect_test(void)
{
	int ret=-1,i32Ret=-1;
	int8 *ip_t="sys00.jiewen.com.cn";//"sys00.jiewen.com.cn";
	int8 *port_t="3000";//TCP-80;SSL-443
	uint32 i=0x30;
	char cSendBuff[2048];
	char cRecvBuff[2048];
	int iSuccT = 0;
	int iSuccR = 0;
	int iSum = 0;
	unsigned char ucKey = 0xFF;
	
	memset(cSendBuff, 0, sizeof(cSendBuff));
	memset(cRecvBuff, 0x00, sizeof(cRecvBuff));

	

	ret=wifiCommConnect_lib(0,"TCP",ip_t,port_t,6000);
	sysLOG(WIFI_LOG_LEVEL_2, "wifiCommConnect_lib ret is %d\r\n", ret);
	if(ret<0)
	{
		if(ret==WIFI_TCP_CONNECTED)
		{
			sysLOG(WIFI_LOG_LEVEL_2, "WIFI_TCP_CONNECTED\r\n");
			sysDelayMs(50);
		}
	}

	i32Ret = wifiGetLinkStatus_lib();
	sysLOG(WIFI_LOG_LEVEL_2, "wifiGetLinkStatus_lib i32Ret is%d\r\n", i32Ret);
	sysDelayMs(10);
	while(1)
	{
		i32Ret = wifiGetLinkStatus_lib();
		if(g_stWifiState.cOpenState!=1)
		{
			sysLOG(WIFI_LOG_LEVEL_2, "state is%d\r\n", g_stWifiState.cOpenState);
			sysLOG(WIFI_LOG_LEVEL_2, "wifiGetLinkStatus_lib i32Ret is%d\r\n", i32Ret);
			break;
		}
		
		sysDelayMs(10);
		if(3 == i32Ret)
		{
			sprintf(cSendBuff,"POST /gzcs/terminal/getTerminalParams.do HTTP/1.1\r\nHost: sys00.jiewen.com.cn\r\nsign: F94BAD5C903DA529D6B86234AC98DF37\r\nContent-Length: 185\r\n\Content-Type: application/json\r\n\r\n");
			sysLOG(WIFI_LOG_LEVEL_2, "cSendBuff :%s\r\n", cSendBuff);

			i++;
			if(i>0x39)
			{
				i=0x30;
			}
			iSum += 1;
			sysLOG(WIFI_LOG_LEVEL_2, "wifisenddata start!!!\r\n");
			i32Ret = wifiCommSendData_lib(0, (char *)cSendBuff, strlen(cSendBuff), 3000, 1);
			if(i32Ret == 0)
			{
				iSuccT += 1;
				memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
				sysLOG(WIFI_LOG_LEVEL_2, "wifi recv start\r\n");
				sysDelayMs(5);
				i32Ret = wifiWaitData_lib(0, cRecvBuff, 511, 3000);
				sysLOG(WIFI_LOG_LEVEL_2, "recvd:%s\r\n", cRecvBuff);
#if 1				
				memset(cSendBuff,0,sizeof(cSendBuff));
				sprintf(cSendBuff,"{\"device_sn\":\"AT00000002\",\"app_version\":\"V1.4\",\"version\":\"V1.0.0\",\"device_type\":\"Q180\",\"manufacturer\":\"Vanstone\",\"nonce_str\":\"21E2780E7FB307B93AADC76F43AE5026\",\"connection_mode\":\"WIFI\"}");
				sysLOG(WIFI_LOG_LEVEL_2, "cSendBuff :%s\r\n", cSendBuff);
				i32Ret = wifiCommSendData_lib(0, (char *)cSendBuff, strlen(cSendBuff), 12000, 1);
				if(i32Ret == 0)
				{
					iSuccT += 1;
					memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
					sysLOG(WIFI_LOG_LEVEL_2, "wifi recv start\r\n");
					//sysDelayMs(5);
					i32Ret = wifiWaitData_lib(0, cRecvBuff, 1, 3000);
					sysLOG(WIFI_LOG_LEVEL_2, "wifiWaitData_lib i32Ret:%d, recvd:%s\r\n", i32Ret, cRecvBuff);

					memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
					i32Ret = wifiWaitData_lib(0, cRecvBuff, 1, 3000);
					sysLOG(WIFI_LOG_LEVEL_2, "wifiWaitData_lib i32Ret:%d, recvd:%s\r\n", i32Ret, cRecvBuff);
#if 1
					memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
					i32Ret = wifiWaitData_lib(0, cRecvBuff, 2, 3000);
					sysLOG(WIFI_LOG_LEVEL_2, "wifiWaitData_lib i32Ret:%d, recvd:%s\r\n", i32Ret, cRecvBuff);
					
					memset(cRecvBuff, 0x00, sizeof(cRecvBuff));
					i32Ret = wifiWaitData_lib(0, cRecvBuff, 190, 3000);
					
					sysLOG(WIFI_LOG_LEVEL_2, "wifiWaitData_lib i32Ret:%d, recvd:%s\r\n", i32Ret, cRecvBuff);
#endif
				}
				
#endif
				sysLOG(WIFI_LOG_LEVEL_2, "TCPCLOSE:%d\r\n", wifiCommClose_lib(0));
				i32Ret=0;
				while(i32Ret!=4)
				{
					i32Ret = wifiGetLinkStatus_lib();
					sysLOG(WIFI_LOG_LEVEL_2, "wifiGetLinkStatus_lib i32Ret is%d\r\n", i32Ret);
					sysDelayMs(1000);
				}			
				
				sysLOG(WIFI_LOG_LEVEL_2, "S:%d T:%d R:%d\r\n", iSum, iSuccT, iSuccR);
				break;
			}			
			else
			{
				switch(i32Ret)
				{
					case 4:
						sysLOG(WIFI_LOG_LEVEL_2, "TCP DisConnect\r\n");
					break;
					
					case 5:
						sysLOG(WIFI_LOG_LEVEL_2, "AP DisConnect\r\n");
					break;
					default:
						sysLOG(WIFI_LOG_LEVEL_2, "Err: %d\r\n", i32Ret);
					break;
				}
				sysLOG(WIFI_LOG_LEVEL_2, "server connect error\r\n");
				
				break;				
			}
		}
		else
		{
			sysLOG(WIFI_LOG_LEVEL_2, "wifiCommConnect_lib error!\r\n");
			break;
		}
		break;
		sysDelayMs(1000);

	}

	
}

int WiFiSSL_ConnectTest(void)
{

	int i32Ret = -1;
	int sslfd = 0;

	uint8 recvtmp[1024];
	
	memset(recvtmp, 0, sizeof(recvtmp));

	sslfd = wifiSSLSocketCreate_lib();
	if(sslfd < 0)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> wifiSSLSocketCreate_lib sslfd=%d\r\n", sslfd);
		return sslfd;
	}
		
	//"sys00.jiewen.com.cn", "443"
	//"https://gw.open.icbc.com.cn", "443"
	//"posnova.icbc.com.cn", "20492"//tlsV1.0
	i32Ret = wifiSSLConnect_lib(sslfd, "https://gw.open.icbc.com.cn", "443", 10000);
	if(i32Ret < 0)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> wifiSSLConnect_lib i32Ret=%d\r\n", i32Ret);
		return i32Ret;
	}
	sysLOG(WIFI_LOG_LEVEL_2, "<SUCC> wifiSSLConnect_lib i32Ret=%d, sslfd=%d\r\n", i32Ret, sslfd);

	i32Ret = wifiSSLSend_lib(sslfd, tcp_wifi_testbuf, strlen(tcp_wifi_testbuf), 5000);
	if(i32Ret < 0)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> wifiSSLSend_lib i32Ret=%d\r\n", i32Ret);
		return i32Ret;
	}

	i32Ret = wifiSSLRecv_lib(sslfd, recvtmp, sizeof(recvtmp), 5000);
	if(i32Ret < 0)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> wifiSSLRecv_lib i32Ret=%d\r\n", i32Ret);
		return i32Ret;
	}
	
	sysLOG(WIFI_LOG_LEVEL_2, "wifiSSLRecv_lib i32Ret=%d\r\n", i32Ret);

	sysDelayMs(2000);

	i32Ret = wifiSSLClose_lib(sslfd);
	if(i32Ret < 0)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> wifiSSLClose_lib i32Ret=%d\r\n", i32Ret);
		return i32Ret;
	}

	i32Ret = wifiSSLSocketClose_lib(sslfd);
	if(i32Ret < 0)
	{
		sysLOG(WIFI_LOG_LEVEL_2, "<ERR> wifiSSLSocketClose_lib i32Ret=%d\r\n", i32Ret);
		return i32Ret;
	}


}

#endif

void wifi_test_demo(void)
{
	//api_wifiTest_Open();
	
	//sysDelayMs(50);
	//WifiTest_Connect();
	//sysDelayMs(50);

//	server_connect_test();
	
}




