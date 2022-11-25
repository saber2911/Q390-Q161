#include "comm.h"


ring_buffer_t se_rev_ringbuff;
static int time_of_413 = 0;//连续出现-413的次数
char  is_se_sleepping = 0;
static int continueRecv = 1;
unsigned char pkgIndex = 0;
static unsigned char pkgIndex_first = 0;
unsigned int se_comm_mutex;
extern uint8 DownLoadVOSFlag;
extern unsigned char ucPortRecvDataFlag;
/* brief  : SE通信初始化
 * param in: NULL
 * retval out: NULL
 * return : NULL
*/
void scanpos_se_comm_init(void)
{
	hal_utSEUartInit(DEFAULT_RATE);

	se_comm_mutex = fibo_mutex_create();
	ring_buffer_init(&se_rev_ringbuff,SE_FRAMEREV_BUFF_SIZE);
	config_se_communicationCtrl_io();
	config_se_communicationRTS_io();
	SE_callBack_Queue = fibo_queue_create(SE_CB_QUEUE_LEN, sizeof(char));
	if(0 == SE_callBack_Queue)
	{
		sysLOG(BASE_LOG_LEVEL_2, " create SE callBack_queue error,id = %d\r\n",SE_callBack_Queue);		
	}
	
	//config_se_callBack_Ctrl_init();
	config_se_callBack_Ctrl();
	//config_se_reboot_io();
	//IO_test_4G_RTS();//拉IO调试程序时使用
}

/* brief  : LRC校验
 * param in: *pbInData 待校验数据，ulBytesIn 数据长度
 * retval out: NULL
 * return : bXor 校验结果
*/
unsigned char u_GetXOR_B(unsigned char*pbInData,unsigned int ulBytesIn)
{
	unsigned char bXor;
	bXor = 0;
	while(ulBytesIn>0)
	{
		bXor ^= pbInData[0];
		pbInData++;
		ulBytesIn--;
	}
	return bXor;
}

/* brief  : 获取数据包中的帧序号seq
 * param in: NULL
 * retval out: NULL
 * return : pkgIndex 帧序号
*/
unsigned char scanpos_comm_get_pkgIndex(void)
{
	if(pkgIndex_first==0)
	{
		pkgIndex_first = 1;
		pkgIndex = (unsigned char)fibo_getSysTick();
	}
	else
	{
		pkgIndex++;
	}
	sysLOG(BASE_LOG_LEVEL_4, " pkgindex=%x\r\n",pkgIndex);

	return pkgIndex;
}

void bytesToHexstring(char* bytes,int bytelength,char *hexstring,int hexstrlength)
{
    int i,j;
    char str2[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    for (i=0,j=0;i<bytelength,j<hexstrlength;i++,j++)
    {
        int b;
        b = 0x0f&(bytes[i]>>4);
        char s1 = str2[b];
        hexstring[j] = s1;
        b = 0x0f & bytes[i];
        char s2 = str2[b];
        j++;
        hexstring[j] = s2;
    }
    hexstring[hexstrlength] = '\0';
}

void printFrame(Frame frm)
{
#if 1
    int length=HEADSIZE+frm.length+1;
    int n=0;
    unsigned char *rawdata = (unsigned char*)fibo_malloc(length);
    char *hexStr = (char*)fibo_malloc(length*2+256);
	memset(hexStr,0,length*2+256);
    getByteofFrame(frm, rawdata);
	if(length>2048)//此处对输出到trace数据长度做限定，因输出长度大于4K时，L610_LOG不能通过trace输出，因此只输出前2K数据到trace
	{
		length =256;
	}	
	bytesToHexstring(rawdata,length,hexStr,length*2);
	
	sysLOG(BASE_LOG_LEVEL_4, " frm.length:%d data %s\r\n",frm.length,hexStr);	
    fibo_free(rawdata);
    fibo_free(hexStr);
	//sysDelayMs(10);
#endif	
}
void printBytes(unsigned char* data,int length)
{
#if 1
	sysLOG(BASE_LOG_LEVEL_4, " serever:printBytes--->\r\n");			
    if(data==NULL)
    {
		sysLOG(BASE_LOG_LEVEL_4, " NULL\r\n");			
        return ;
    }
    int i=0;
    char *hexStr = (char*)fibo_malloc(length*2+256);
	if(NULL == hexStr)
	{
		sysLOG(BASE_LOG_LEVEL_1, " malloc error\r\n");			
	}
	if(length>2048)//此处对输出到trace数据长度做限定，因输出长度大于4K时，L610_LOG不能通过trace输出，因此只输出前2K数据到trace
	{
		length =256;
	}
	bytesToHexstring(data,length,hexStr,length*2);

	sysLOG(BASE_LOG_LEVEL_4, " %s\r\n",hexStr);			
    fibo_free(hexStr);
#endif
}

/* brief  : 数据存入frm
 * param in: *data 数据
 * retval out: *frm 数据包
 * return : ret 数据存放情况，<0 失败
*/
int putByteToFrame(unsigned char *data, Frame *frm)
{
    if(data == NULL || frm == NULL)
    {
        return COM_DATA_NULL;
    }
    frm->tag = data[0];
    frm->length = data[1] | data[2] << 8;
    frm->sequence = data[3];
    frm->state = data[4];
    frm->reserve = data[5];

    /*notice:malloc memory of the data ,but not free  now!!!!,should free in API!!*/
    frm->data=(unsigned char*)fibo_malloc(frm->length);
	if(NULL == frm->data)
	{
		sysLOG(BASE_LOG_LEVEL_1, " malloc error\r\n");		
		return COM_DATA_NULL;
	}
    memcpy(frm->data, data + 6, frm->length);
    frm->checksum = data[HEADSIZE + frm->length];

    return 0;
}
/* brief  : copy数据包中的数据到*data
 * param in: frm 数据包
 * retval out: *data 存放提取数据
 * return : ret 数据提取情况，<0 失败
*/
int getByteofFrame(Frame frm, unsigned char *data)
{
    if(data == NULL)
        return COM_DATA_NULL;
    data[0] = frm.tag;
    data[1] = frm.length & 0x00ff;
    data[2] = (frm.length & 0xff00) >> 8;
    data[3] = frm.sequence;
    data[4] = frm.state;
    data[5] = frm.reserve;
    memcpy(data + HEADSIZE, frm.data, frm.length);
    data[frm.length + HEADSIZE] = frm.checksum;
    return 0;
}

/* brief  : 数据打包
 * param in: *data 数据帧，*frm 存放打包好的数据，tag 命令头，length数据帧长度，seq 帧序号，state 状态
 * retval out: NULL
 * return : ret 数据打包情况，<0 失败
*/
int frameFactory(unsigned char *data, Frame *frm, unsigned char tag, unsigned short length, unsigned char seq, unsigned char state)
{
	int ret = 0;
	if(NULL == data || NULL == frm)
	{
		 return COM_DATA_NULL;
	}
	/*data length+frame head+checksum */
    int totalLength = length + HEADSIZE + 1;
	unsigned char *rawData = (unsigned char*)fibo_malloc(totalLength);
	if(NULL == rawData)
	{
		sysLOG(BASE_LOG_LEVEL_1, " rawData malloc error\r\n");
		return COM_MALLOC_ERROR;
	}
    rawData[0] = tag;
    rawData[1] = length & 0x00ff;
    rawData[2] = (length & 0xff00) >> 8;	
	rawData[3] = scanpos_comm_get_pkgIndex();
    rawData[4] = state;
    rawData[5] = 0x00;	
    memcpy(rawData + 6, data, length);
    rawData[totalLength - 1] = u_GetXOR_B(rawData, totalLength - 1);

	ret = putByteToFrame(rawData,frm);
	if(ret<0)
	{
		fibo_free(rawData);
		return ret;
	}
	fibo_free(rawData);
	
	return COM_OK;
}
/* brief  : 发送数据包
 * param in: frm 数据包
 * retval out: NULL
 * return : ret 数据包发送情况，<0 失败
*/
int sendFrame(Frame frm)
{

#if 0	
	if(NULL != sessionKey && 0x52 != frm.tag)
	{
	}
#endif
    int ret = COM_OK;
    int i = 0;
    int status = 1;
    int length = 6 + frm.length + 1; /* frame head+data length+checksum */
    unsigned char *rawdata = (unsigned char*)fibo_malloc(length);
    if(rawdata == NULL)
    {
        return COM_DATA_NULL;
    }
#if 1	
    while(i < 100)//100 ms timeout
	{
        status = get_se_cb_status();
        if(!status)
            break;
        i++;
        sysDelayMs(1);
    }	
#endif
    if(i == 100 && status) 
	{
		sysLOG(BASE_LOG_LEVEL_1, " SE ctrl_io always high\r\n");
        ret = COM_SEND_ERR_SE_BUSY;
    }
	else 
	{
        getByteofFrame(frm, rawdata);
		if(DownLoadVOSFlag == 0)
		{
			ret = fibo_hal_uart_put(SE_COMMUNICATION_PORT, rawdata, length);
			if(ret < 0 || ret != length)
			{
				ret =  COM_SEND_ERROR;
			}
		}
		else
		{
			ret =  COM_ISR_HANDLE;
		}
    }
    fibo_free(rawdata);
    //release encrypted frame data
    if(0x43 == frm.tag) 
	{
        fibo_free(frm.data);
    }
    return ret;
}
/* brief  : 接收SE返回数据包
 * param in: timeout 接收超时时间
 * retval out: *revFrm 存放返回数据包指针
 * return : iRet 处理结果
*/
int receive_frame(Frame *revFrm,int timeout)
{
	int iRet = 0;
	unsigned long long uitime = 0;
	unsigned long long time_start = 0;
	unsigned long long time_end = 0;	
	unsigned char wait_head = 1;
	char se_status = -1;
	unsigned char frame_head[HEADSIZE] = {0};
	unsigned char *total_frame;
	int position = 0;
	int available = 0;
	unsigned int length = 0;
	unsigned int wait_head_timeout = 2000;	//等待流控IO拉高或接收到SE应答数据等待时间，LC610串口发送为异步，因此此处等待时间加大为500ms，确保LC610数据包已发送完成，并且SE端接收完成，原来为100ms
	
	if(NULL == revFrm)
	{
		return COM_DATA_NULL;
	}
	uitime = hal_sysGetTickms()+timeout;
	time_start = hal_sysGetTickms();
	sysLOG(BASE_LOG_LEVEL_4, " time_start=%lld uitime=%lld\r\n",time_start,uitime);			
	while(1)
	{
        if(continueRecv != 1)
        {
            return COM_BEEN_STOPPED;
        }
		while(wait_head)
		{
			se_status = get_se_cb_status();
			if(se_status == 1 || get_ringBuffer_valid(&se_rev_ringbuff)>0)
			{
				sysLOG(BASE_LOG_LEVEL_4, " se_status=%d, available=%d\r\n",se_status,get_ringBuffer_valid(&se_rev_ringbuff));			
				wait_head = 0;
				break;
			}
			time_end = hal_sysGetTickms();
			if(time_end-time_start>wait_head_timeout)
			{
				sysLOG(BASE_LOG_LEVEL_3, " se_status=%d, available=%d\r\n",se_status,get_ringBuffer_valid(&se_rev_ringbuff));						
				sysLOG(BASE_LOG_LEVEL_3, " time_start=%lld time_end=%lld\r\n",time_start,time_end);					
				sysLOG(BASE_LOG_LEVEL_3, " timeout\r\n");			
				return COM_WAIT_IO_TIMEOUT;
			}
		}
		if(hal_sysGetTickms()>uitime)
		{
			sysLOG(BASE_LOG_LEVEL_3, " head_timeout\r\n");					
			return COM_RECEIVE_HEAD_TIMEOUT;
		}
		available = get_ringBuffer_valid(&se_rev_ringbuff);
		if(available>0)
		{
            if(position == 0)
            {
				ring_buffer_read(&se_rev_ringbuff,frame_head,1);
				sysLOG(BASE_LOG_LEVEL_4, " avilable=%d, frame_head[0]=%x\r\n",available,frame_head[0]);			
	            if((frame_head[0] & 0x20) == 0x20)
	            {
		            position++;
			        continue;
	            }
            }
            if(available + position >= HEADSIZE)
            {
                iRet = ring_buffer_read(&se_rev_ringbuff,frame_head + position,HEADSIZE - position);
                break;
            }
            else
            {
				iRet = ring_buffer_read(&se_rev_ringbuff,frame_head + position,available);				
            }
            position += available;		
		}
	}
	printBytes(frame_head,6);
	if(frame_head[0]!=0x23 && frame_head[0]!=0x20) /*the first byte of head error*/
	{
		sysLOG(BASE_LOG_LEVEL_1, " head error=%x\r\n",frame_head[0]);				
		return COM_RECEIVE_HEAD_ERROR;
	}
	length = frame_head[1] | ((unsigned short)frame_head[2]<<8);
	if(length == 0)
	{
		sysLOG(BASE_LOG_LEVEL_4, " type=%x\r\n",frame_head[4]);			
		return COM_RECEIVE_NULL;
	}
	length += 1;
	total_frame = (unsigned char*)fibo_malloc(HEADSIZE + length);
	if(NULL == total_frame)
	{
		return COM_DATA_NULL;
	}
	memcpy(total_frame,frame_head,HEADSIZE);
	position = HEADSIZE;
	while(1)
	{
		if(hal_sysGetTickms()>uitime)
		{
			sysLOG(BASE_LOG_LEVEL_3, " timeout\r\n");							
			return COM_RECEIVE_HEAD_TIMEOUT;
		}
		available = get_ringBuffer_valid(&se_rev_ringbuff);
		if(available>0)
		{
			if((available + position) > (HEADSIZE + length))
			{
                iRet = COM_RECEIVE_FRAME_ERROR;
				sysLOG(BASE_LOG_LEVEL_1, " fram_rev over\r\n");									
                break;
			}
		}
		iRet = ring_buffer_read(&se_rev_ringbuff,total_frame + position,available);	
		position += available;
		if(position >= (length+HEADSIZE))
		{
			sysLOG(BASE_LOG_LEVEL_1, " frame read over\r\n");							
			break;
		}
	}
	putByteToFrame(total_frame, revFrm);
	if(u_GetXOR_B(total_frame, HEADSIZE + length - 1) != total_frame[HEADSIZE+length-1])
	{
		sysLOG(BASE_LOG_LEVEL_1, " receive checksum data error!\r\n");			
		iRet = COM_RECEIVE_CHECKSUM_ERR;
	}
   if(position > 10)//se recive null error for 0x9005
   {
       if(total_frame[8] == 0x90 && (total_frame[9] == 0x05 || total_frame[9] == 0x16))
	   {
           iRet = COM_RECEIVE_NULL;
           printFrame(*revFrm);
       }
   }
   fibo_free(total_frame);
#if 0  
	if(NULL != sessionKey && 0x23 == frm->tag) 
	{
		unsigned char* plaintext = (unsigned char*) fibo_malloc(frm->length);
		int plaintext_len = decrypt_3des_ecb2(frm->data, frm->length, sessionKey, plaintext);
		memcpy(frm->data, plaintext, plaintext_len);
		fibo_free(plaintext);

		frm->length = plaintext_len;
	}
#endif	
    return iRet;   
}
/* brief  : 发送一次数据包并接收SE返回数据包
 * param in: sendFrm 发送数据包，接收超时时间，timeout 超时时间
 * retval out: *receiveFrm 存放返回数据包指针
 * return :iRet 处理结果
*/
int transceiveFrameOnce(Frame sendFrm, Frame *receiveFrm, int timeout)
{
	int iRet = 0;

    if(is_se_sleepping) 
	{
        SE_awaken();
        is_se_sleepping = 0;
    }	

	if(NULL == receiveFrm)
	{
		sysLOG(BASE_LOG_LEVEL_4, " revfram==null\r\n");
		return COM_DATA_NULL;
	}

	ring_buffer_deinit(&se_rev_ringbuff);
    continueRecv = 1;

	sysLOG(BASE_LOG_LEVEL_4, " %s\r\n","send --------->");	
    printFrame(sendFrm);
	iRet = sendFrame(sendFrm);
	sysLOG(BASE_LOG_LEVEL_1, " iRet=%d\r\n",iRet);	
	if(iRet>0)
	{		
		sysLOG(BASE_LOG_LEVEL_4, " %s\r\n","receive --------->");			
		iRet = receive_frame(receiveFrm,timeout);
	}
	sysLOG(BASE_LOG_LEVEL_1, " iRet=%d\r\n",iRet);
	if((iRet>=0)||(iRet == -406))
	{
    	printFrame(*receiveFrm);
		sysLOG(BASE_LOG_LEVEL_4, " printFrame end ------>\r\n");					
	}
	return iRet;
}
/* brief  : 发送数据包并接收SE返回数据包
 * param in: sendFrm 发送数据包，接收超时时间，timeout 超时时间
 * retval out: *receiveFrm 存放返回数据包指针
 * return :iRet 处理结果
*/
int transceiveFrame(Frame sendFrm, Frame *receiveFrm, int timeout)
{
	int iRet = 0,i = 0;
#if 0	
    if(isPortLocked()) //serial forward is running
        return COM_BEEN_LOCKED;
#endif
	
	fibo_mutex_lock(se_comm_mutex);//lock
	//fibo_gpio_set(SE_CB_CTRL,TRUE);
	//config_se_callBack_Ctrl_NoIRQ();
	for(i=0;i<FRAME_SEND_TIMES;i++)
	{
		sysLOG(BASE_LOG_LEVEL_4, " fram send times=%d,req = %d\r\n",i,sendFrm.sequence);		    
		iRet = transceiveFrameOnce(sendFrm,receiveFrm,timeout);
		sysLOG(BASE_LOG_LEVEL_1, " transceiveFrameOnce iRet = %d\r\n",iRet);		
        /*some case to add sequence value*/
        if(iRet == COM_RECEIVE_NULL && iRet == COM_RECEIVE_HEAD_TIMEOUT)		
    	{
            sendFrm.sequence = scanpos_comm_get_pkgIndex();//change the seq for resend
            unsigned char* frmBytes = (unsigned char*)fibo_malloc(sendFrm.length + HEADSIZE + 1);
            getByteofFrame(sendFrm, frmBytes);
            sendFrm.checksum = u_GetXOR_B(frmBytes, sendFrm.length + HEADSIZE);
            fibo_free(frmBytes);    		
    	}
        if((iRet >= 0) || (ucPortRecvDataFlag == 1))
        {
            time_of_413 = 0;//如果通信成功，就重置次数
			sysLOG(BASE_LOG_LEVEL_4, " communication success!!!\r\n");		            
            break;
        }	
        if(iRet == COM_SEND_ERR_SE_BUSY)
        {
            time_of_413++;
            sysLOG(BASE_LOG_LEVEL_4, " time_of_413 = %d\r\n",time_of_413);
            if(time_of_413 == 10)//如果连续10次 -413就重启se
            {
#if 1            
				time_of_413 = 0;//TODO计数清零
				sysLOG(BASE_LOG_LEVEL_1, " reboot se\r\n");
				se_reboot();
				sysLOG(BASE_LOG_LEVEL_1, " se already reboot.\r\n");				
#endif				
            }
        }	
		sysDelayMs(30);
	}	
	
	//config_se_callBack_Ctrl();
	//sysDelayMs(1);
	//fibo_gpio_set(SE_CB_CTRL,FALSE);
	fibo_mutex_unlock(se_comm_mutex);//unlock
	sysLOG(BASE_LOG_LEVEL_1, " transceiveFrame end iRet = %d\r\n",iRet);		            	
	
	return iRet;
}

int sendRowData(unsigned char* in, int inLen)
{
	int ret = 0;
    if(in == NULL)
	{
        return COM_DATA_NULL;
    }
	else
	{
        printBytes(in, inLen);
    }
	
    //send row data
	if(DownLoadVOSFlag == 0)
	{
		ret = fibo_hal_uart_put(SE_COMMUNICATION_PORT,in, inLen);
		if(ret < 0) 
		{
			ret = COM_SEND_ERROR;
		}
	}
    
    return ret;
}
int receiveRowData(unsigned char* out, int* outLen, int timeout)
{
    Frame receiveFrm;
    int ret = receive_frame(&receiveFrm, timeout);

    pthread_mutex_unlock(&se_comm_mutex); //unlock serialport
	sysLOG(BASE_LOG_LEVEL_1, " receive:ret=%d\r\n",ret);		
    if(ret >= 0)
	{
        printFrame(receiveFrm);
        *outLen = receiveFrm.length + 7;
        getByteofFrame(receiveFrm, out);
    }
    fibo_free(receiveFrm.data);
    return ret;
}


/******************************TEST******************************/

#if MAINTEST_FLAG

/* brief  : 串口协议通信测试
 * param in: NULL
 * retval out: NULL
 * return : ret 串口数据发送情况
*/
int test_serialport(unsigned char *pData, int dataLen, unsigned char *pBuffer, int *bufferLen)
{
	if(NULL == pBuffer)
	{
		return COM_DATA_NULL;
	}
	memset(pBuffer,0,sizeof(pBuffer));
	
    int ret=0;
    int cmdLength= 6 + dataLen;
    unsigned char* cmd_test = (unsigned char*)fibo_malloc(cmdLength);
    cmd_test[0] = 0X00;
    cmd_test[1] = 0xee;
    cmd_test[2] = 0x00;
    cmd_test[3] = 0x00;
    cmd_test[4] = dataLen;
    cmd_test[5] = dataLen >> 8;
    memcpy(cmd_test + 6, pData, dataLen);
    Frame frm,retfrm;
    ret = frameFactory(cmd_test, &frm, 0x40, cmdLength, 0x01, 0x00);
    fibo_free(cmd_test);
    if(ret < 0) 
	{
		sysLOG(BASE_LOG_LEVEL_1, " frameFactory error\r\n");
        return ret;
    }
    ret = transceiveFrame(frm, &retfrm, 1000);
    fibo_free(frm.data);
	sysLOG(BASE_LOG_LEVEL_1, " rev_fram free\r\n");
    if(ret <0) 
	{
        return ret;
    }
    ret=retfrm.data[2]<<8 | retfrm.data[3];
    if(0x9000 == ret) 
	{
        ret = retfrm.data[5]<<8 | retfrm.data[4];
        memcpy(pBuffer,retfrm.data + 6, ret);
		printBytes(pBuffer,ret);		
    }
	else 
	{
        ret = -ret;
    }
    fibo_free(retfrm.data);
	sysLOG(BASE_LOG_LEVEL_1, " test_serialport end,ret = %d\r\n",ret);	
    return ret;
}

#endif


