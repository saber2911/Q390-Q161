#include "LC610N_api_uart.h"

unsigned char ucPortRecvFlag = 0;
unsigned char ucPortRecvDataFlag = 0;
/*
*@Brief:		打开指定的通讯口,并设定通讯参数
*@Param IN:	    参数1：通道号 参数2：UART配置参数
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int portOpenEx(SER_PORTNUM_t emPort,char *Attr)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 7;	
	iCmdLen += strlen(Attr);
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);		
	unsigned char ucCmdHead[7] = {0x00, 0xF5, 0x00, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort};
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
    memcpy(ucCmd+7, Attr, strlen(Attr));

#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		关闭指定的通讯口
*@Param IN:	    参数1：通道号 
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int portCloseEx(SER_PORTNUM_t emPort)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 7;
	unsigned char ucCmdHead[7] = {0x00, 0xF5, 0x01, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort};
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
    
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		复位通讯口,该函数将清除串口接收缓冲区中的所有数据
*@Param IN:	    参数1：通道号 
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int portFlushBufEx(SER_PORTNUM_t emPort)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 7;
	unsigned char ucCmdHead[7] = {0x00, 0xF5, 0x02, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort};
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
    
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		使用指定的通讯口发送若干字节的数据
*@Param IN:	    参数1：通道号 参数2：发送的数据串指针 参数3：发送数据串的字节数
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int portSendsEx(SER_PORTNUM_t emPort,uchar *str, ushort str_len)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 9;
	iCmdLen += str_len;//strlen((char*)str);//
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 32);
	unsigned char ucCmdHead[9] = {0x00, 0xF5, 0x03, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort, str_len & 0xFF, str_len >> 8};
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	//sysLOG(API_LOG_LEVEL_2, "  iCmdLen = %d str_len = %d\r\n", iCmdLen,str_len);
	
    memcpy(ucCmd+9, str, str_len);//strlen(str));//
    
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 128);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		执行SE的端口接收期望长度的数据
*@Param IN:	    参数1：通道号  参数2：接收的字节数 参数3：接收超时时长（单位：毫秒）；为 0 时，则有数据便接收后立即退出，无数据也立即退出，均返回收到的字节数
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int portRecvsCmd(SER_PORTNUM_t emPort, ushort usBufLen, ushort usTimeoutMs)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 11;
	unsigned char ucCmdHead[11] = {0x00, 0xF5, 0x04, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort, usBufLen & 0xFF, usBufLen >> 8, usTimeoutMs & 0xFF, usTimeoutMs >> 8};
    
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

int portRecvsDataState(SER_PORTNUM_t emPort)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 7;
	char ver[10];
	int  version = 0;
	unsigned char ucCmdHead[7] = {0x00, 0xF5, 0x06, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort};
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
    /*memset(ver, 0x00, sizeof(ver));
	strcpy(ver, DR_VER);
	version = (ver[6]-'0')*10  + (ver[7]-'0');
	sysLOG(BASE_LOG_LEVEL_1, " version=%d\r\n",version);
	if(version > 37)
	{
		fibo_free(ucCmd);
		return 0;
	}*/
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
    fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(BASE_LOG_LEVEL_4, " portRecvs iRet=%d\r\n",iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
	sysLOG(BASE_LOG_LEVEL_4, " portRecvs iRet=%d\r\n",iRet);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = retfrm.data[6];
	}
	else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		sysLOG(BASE_LOG_LEVEL_4, " portRecvs iRet=%d\r\n",iRet);
    }
	else
	{
		iRet = -iRet;
		sysLOG(BASE_LOG_LEVEL_4, " portRecvs iRet=%d\r\n",iRet);
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		在给定的时限内，最多接收期望长度的数据
*@Param IN:	    参数1：通道号  参数3：接收的字节数 参数4：接收超时时长（单位：毫秒）；为 0 时，则有数据便接收后立即退出，无数据也立即退出，均返回收到的字节数
*@Param OUT:	参数2：接收缓冲区地址指针 
*@Return:		>=0:实际接收到的字节数; <0:失败
*/
int portRecvsEx(SER_PORTNUM_t emPort, uchar *pszBuf, ushort usBufLen, ushort usTimeoutMs)
{
	int iRet = RET_RF_ERR_PARAM;
	int recvState = 0;
	int iCmdLen = 7;
	unsigned char ucCmdHead[7] = {0x00, 0xF5, 0x05, 0x00, iCmdLen-6, (iCmdLen -6) >> 8, emPort};
    UINT64 uitime = 0;
    UINT64 time_start = 0;
	UINT64 time_end = 0;	

    if(portRecvsCmd(emPort, usBufLen, usTimeoutMs) != 0)
    {
		sysLOG(API_LOG_LEVEL_4, "  iRet = %d\r\n", iRet);
        return iRet; 
    }
	//sysLOG(API_LOG_LEVEL_2, "  iRet = %d\r\n", iRet);
    ucPortRecvFlag = 0;
	ucPortRecvDataFlag = 1;
    //uitime = hal_sysGetTickms()+usTimeoutMs;
    time_start = hal_sysGetTickms();
    while(!ucPortRecvFlag)
    {
        time_end = hal_sysGetTickms();
		if(time_end>=time_start)
		{
        	if((time_end > (time_start + usTimeoutMs + 20)))
        	{
		   		sysLOG(API_LOG_LEVEL_4, " recv timeout\r\n");
           		break;
        	}
		}else{
			time_start = 0;
			time_end = 0;
		}
		
		recvState = portRecvsDataState(emPort);
		if(recvState == 1)
		{
			sysLOG(API_LOG_LEVEL_4, " recvState=%d\r\n",recvState);
			sysDelayMs(100);
			break;
		}
		sysDelayMs(50);
		//sysLOG(API_LOG_LEVEL_2, " wait recv\r\n");
    }
	ucPortRecvDataFlag = 0;
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
    fibo_free(caShow);
#endif
	sysLOG(API_LOG_LEVEL_4, " get data\r\n");
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
        iRet = retfrm.data[7]<<8 | retfrm.data[6];
        memcpy(pszBuf, retfrm.data+8, iRet);
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
	sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}
