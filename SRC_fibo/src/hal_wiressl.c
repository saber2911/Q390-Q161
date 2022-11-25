/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_wiressl.c
** Description:		4G SSL相关接口
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


/*
*Function:		hal_wiresslSetSslVer
*Description:	SSL版本设置
*Input:			ucVer：1-SSL3.0;2-TLS1.0;3-TLS1.1;4-TLS1.2
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败
*Others:
*/
int hal_wiresslSetSslVer(unsigned char ucVer)
{
	int iRet;
	iRet = fibi_mqtt_set_tls_ver(ucVer);
	sysLOG(SSL_LOG_LEVEL_1, "fibi_mqtt_set_tls_ver, ucVer:%d, iRet:%d\r\n", ucVer, iRet);
	if(iRet < 0)
	{
		return ERR_4G_AT_CFUN_FAIL;
	}
	return iRet;
}


/*
*Function:		hal_wiresslDefault
*Description:	SSL工作方式默认设置
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_wiresslDefault(void)
{
	sysLOG(SSL_LOG_LEVEL_1, "\r\n");

	fibo_ssl_default();
}


/*
*Function:		hal_wiresslSendCertFile
*Description:	下载证书
*Input:			ucType: 证书类型：0-CA File; 1-KEY File; 2-TRUST File
*				*pucData: 数据指针；
*				iLen: 数据长度；
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_wiresslSendCertFile (unsigned char ucType, unsigned char *pucData, int iLen)
{
	int iRet = -1;
	switch (ucType)
	{
		case 0://CA File
		
			iRet = fibo_write_ssl_file("CAFILE", pucData, iLen);

		break;
		case 1://KEY File
		
			iRet = fibo_write_ssl_file("CAKEY", pucData, iLen);

		break;
		case 2://TRUST File
		
			iRet = fibo_write_ssl_file("TRUSTFILE", pucData, iLen);

		break;
		default:
			
			sysLOG(SSL_LOG_LEVEL_2, "<ERR> ucType cannot matched\r\n");

		break;
	}
	if(iRet < 0)
	{
		return ERR_4G_AT_QISEND_FAIL;
	}
	return iRet;
}


/*
*Function:		hal_wiresslSetMode
*Description:	设置认证模式
*Input:			ucMode:1-双向验证；0-不需要验证
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_wiresslSetMode(unsigned char  ucMode)
{
	
	sysLOG(SSL_LOG_LEVEL_1, "\r\n");

	fibo_set_ssl_chkmode(ucMode);
	return 0;

}


/*
*Function:		hal_wiresslSocketCreate
*Description:	创建一个安全socket
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		-1:失败；其他值：成功，值为SSL Socket ID
*Others:
*/
int hal_wiresslSocketCreate(void)
{
	int iRet = -1;

	iRet = fibo_ssl_sock_create();
	sysLOG(SSL_LOG_LEVEL_1, "fibo_ssl_sock_create, iRet:%x\r\n", iRet);

	return iRet;
}


/*
*Function:		hal_wiresslSocketClose
*Description:	释放SSL Socket句柄
*Input:			sockid:已创建的socket ID
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_wiresslSocketClose(int sockid)
{
	int iRet = -1;
	
	iRet = fibo_ssl_sock_close(sockid);
	sysLOG(SSL_LOG_LEVEL_1, "fibo_ssl_sock_close, iRet:%d, sockid=0x%x\r\n", iRet, sockid);
	if(iRet < 0)
	{
		return ERR_4G_AT_SOCKCOLOSE_FAIL;
	}
	return iRet;
}


/*
*Function:		hal_wiresslConnect
*Description:	与远端服务器建立SSL连接
*Input:			sockid: socket ID；*putDestIP: 服务端IP/域名；pucDestPort: 端口;timeout:超时时间
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_wiresslConnect(int sockid, char *pucDestIP, char *pcDestPort, int timeout)
{
	

	int iRet;
	int sslsock_fd;
	uint16 pucDestPort = atoi(pcDestPort);
	sysLOG(SSL_LOG_LEVEL_4, "pucDestPort:%d\r\n", pucDestPort);
	iRet = fibo_ssl_sock_connect(sockid, pucDestIP, pucDestPort);
	if(iRet < 0)
	{
	
		sysLOG(SSL_LOG_LEVEL_1, "<ERR> fibo_ssl_sock_connect iRet:%d, sockid=0x%x\r\n", iRet, sockid);
		return ERR_4G_TCP_CONNECT_FAIL;
	}

	sslsock_fd = fibo_ssl_sock_get_fd(sockid);
	if(sslsock_fd <= 0)
	{
		
		sysLOG(SSL_LOG_LEVEL_1, "<ERR> sockid=0x%x, fibo_ssl_sock_get_fd sslsock_fd:%d\r\n", sockid, sslsock_fd);
		return ERR_4G_TCP_CONNECT_FAIL;
	}

	hal_wiresockSetNoBlock(sslsock_fd);
	sysLOG(SSL_LOG_LEVEL_1, "hal_wiresslConnect, iRet:%x, sslsock_fd:%d, sockid=0x%x, pucDestIP:%s\r\n", iRet, sslsock_fd, sockid, pucDestIP);
	return 0;

}


/*
*Function:		hal_wiresslSend
*Description:	向服务端发送数据
*Input:			sockid:socket ID；*pucData:发送数据缓冲指针；uiLen:发送数据长度
*Output:		NULL
*Hardware:
*Return:		>=0:实际发送的字节数；<0:失败
*Others:
*/
int hal_wiresslSend(int sockid, unsigned char *pucData, unsigned int uiLen)
{
	int iRet;
	uint packgenum1,packgenum2;
	int Sendlentmp = 0;
	packgenum1 = uiLen/1024;
	packgenum2 = uiLen%1024;

	if(packgenum1 != 0)
	{
		for(uint8 i=0; i<packgenum1; i++)
		{
			iRet = fibo_ssl_sock_send(sockid, pucData+(i*1024), 1024);
			sysLOG(SSL_LOG_LEVEL_4, " 1 hal_wiresslSend iRet:%d, sockid:0x%x, pucData:%s, iLen:%d\r\n", iRet, sockid, pucData, uiLen);
			if(iRet < 0)
			{
				goto exit;
			}
			else

			{
				Sendlentmp += iRet;
			}
		}
	}
	if(packgenum2 != 0)
	{
		iRet = fibo_ssl_sock_send(sockid, pucData+(packgenum1*1024), packgenum2);
		sysLOG(SSL_LOG_LEVEL_4, " 2 hal_wiresslSend iRet:%d, sockid:0x%x, pucData:%s, iLen:%d\r\n", iRet, sockid, pucData, packgenum2);
		if(iRet < 0)
		{
			goto exit;
		}
		else

		{
			Sendlentmp += iRet;
		}
		
	}
	iRet = Sendlentmp;
exit:
	if(iRet < 0)
	{
		iRet = ERR_4G_SEND_DATA_FAIL;
	}
	sysLOG(SSL_LOG_LEVEL_1, " hal_wiresslSend iRet:%d, sockid:0x%x, pucData:%s, iLen:%d\r\n", iRet, sockid, pucData, uiLen);
	return iRet;
}


static uint32 ssl_recvdata(int sockid, unsigned char *pucdata, unsigned short iLen)
{
	int Ret = 0;
	uint32 recvdlen = 0;
	
	while (1)
	{
		Ret = fibo_ssl_sock_recv(sockid, (unsigned char *)(pucdata), iLen);
		if (Ret < 0)
		{
			sysLOG(SSL_LOG_LEVEL_2, "<ERR> Ret = %d recvdlen=%d\r\n", Ret, (int)recvdlen);
			return Ret;
		}
		else if (Ret == 0)
		{
			sysLOG(SSL_LOG_LEVEL_5, "recv finish, next recvdlen=%d",(int)recvdlen);
			return recvdlen;
		}
		else
		{
			recvdlen += Ret;
			if (recvdlen > 0)
			{

				sysLOG(SSL_LOG_LEVEL_3, "<SUCC> wirelessSslRecv_lib, recvdlen:%d\r\n", recvdlen);
				return recvdlen;
			}
		}
	}
}


/*
*Function:		hal_wiresslRecv
*Description:	接收服务端数据，一次读取最长不超过2048个字节
*Input:			sockid: socket ID; iLen:需要读取的长度; uiTimeOut:超时时间，单位ms
*Output:		*pucdata:读取数据缓存指针
*Hardware:
*Return:		>0:实际读取到的长度；<0:失败; 0:超时
*Others:
*/
int hal_wiresslRecv(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut)
{

	int sslsock_fd;

	sslsock_fd = fibo_ssl_sock_get_fd(sockid);
	if (sslsock_fd <= 0)
	{
		return sslsock_fd;
	}

	int iRet = -1;
	int Ret = 0;
	uint32 recvdlen = 0;
	unsigned long long uTime;

	uTime = hal_sysGetTickms() + uiTimeOut;

	sysLOG(SSL_LOG_LEVEL_2, "sockid=0x%x, DR_GetSystick_ms():%lld, uTime:%lld, uiTimeOut:%d\r\n", sockid, hal_sysGetTickms(), uTime, uiTimeOut);
	
	recvdlen += ssl_recvdata(sockid, (unsigned char *)(pucBuff + recvdlen), uiMaxLen - recvdlen);
	if (recvdlen > 0)
	{

		sysLOG(SSL_LOG_LEVEL_2, "<SUCC> wirelessSslRecv_lib, recvdlen:%d, pucdata:%s\r\n", recvdlen, pucBuff);
		return recvdlen;
	}
	
	while (1)
	{

		FD_ZERO(&g_SocketReadfd);
		FD_ZERO(&g_SocketWritefd);
		FD_ZERO(&g_SocketErrfd);
		FD_SET(sslsock_fd, &g_SocketReadfd);
		FD_SET(sslsock_fd, &g_SocketWritefd);
		FD_SET(sslsock_fd, &g_SocketErrfd);
		g_stSocketTimeval.tv_sec  = uiTimeOut/1000;
		g_stSocketTimeval.tv_usec = (uiTimeOut % 1000) * 1000;
		
		iRet = fibo_sock_lwip_select(sslsock_fd + 1, &g_SocketReadfd, NULL, &g_SocketErrfd, &g_stSocketTimeval);
		if (iRet < 0)
		{
			sysLOG(SSL_LOG_LEVEL_2, "select ret < 0, error\r\n");
			return iRet;
		}

		if(FD_ISSET(sslsock_fd, &g_SocketErrfd))	
		{
			int error = 0;
			int len = 4;
			
			iRet = fibo_sock_getOpt(sslsock_fd, SOL_SOCKET, SO_ERROR,&error, &len);
			sysLOG(SSL_LOG_LEVEL_2, "fibo_sock_getOpt, error = %d\r\n", error);

			return -1;
		}
		       
		Ret = ssl_recvdata(sockid, (unsigned char *)(pucBuff + recvdlen), uiMaxLen - recvdlen);		
		if(Ret == 0)
		{
			//
		}
		else if(Ret < 0)
		{
			sysLOG(SSL_LOG_LEVEL_1, "<ERR> Ret = %d\r\n", Ret);
			return Ret;
		}
		else
		{
			recvdlen += Ret;
			if(recvdlen > 0)
			{
				
				sysLOG(SSL_LOG_LEVEL_2, "<SUCC> wirelessSslRecv_lib, recvdlen:%d, pucdata:%s\r\n", recvdlen, pucBuff);
				return recvdlen;
			}
		}

		if(hal_sysGetTickms() > uTime)
		{
			
//			sysLOG(SSL_LOG_LEVEL_2, "timeout, DR_GetSystick_ms():%lld\r\n", hal_sysGetTickms());
			return recvdlen;
		}
		sysDelayMs(200);
	}

}


/*
*Function:		hal_wiresslRecv
*Description:	接收服务端数据，一次读取最长不超过2048个字节
*Input:			sockid: socket ID; iLen:需要读取的长度; uiTimeOut:超时时间，单位ms
*Output:		*pucdata:读取数据缓存指针
*Hardware:
*Return:		>0:实际读取到的长度；<0:失败; 0:超时
*Others:
*/
int hal_wiresslRecv0(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut)
{

	int sslsock_fd;

	sslsock_fd = fibo_ssl_sock_get_fd(sockid);
	if(sslsock_fd <= 0)
	{
		return ERR_4G_AT_SSLSOCK_FAIL;
	}
	
	int iRet = -1;
	int Ret = 0;
	uint32 recvdlen = 0;
	unsigned long long uTime;
	
	uTime = hal_sysGetTickms() + uiTimeOut;
	
	sysLOG(SSL_LOG_LEVEL_4, "hal_sysGetTickms():%lld, uTime:%lld, uiTimeOut:%d\r\n", hal_sysGetTickms(), uTime, uiTimeOut);
	while(1)
	{	
		
		FD_ZERO(&g_SocketReadfd);
		FD_ZERO(&g_SocketWritefd);
		FD_ZERO(&g_SocketErrfd);
		FD_SET(sslsock_fd, &g_SocketReadfd);
		FD_SET(sslsock_fd, &g_SocketWritefd);
		FD_SET(sslsock_fd, &g_SocketErrfd);
		g_stSocketTimeval.tv_sec  = 0;
		g_stSocketTimeval.tv_usec = 1000;//单位微秒
		
		iRet = fibo_sock_lwip_select(sslsock_fd+1, &g_SocketReadfd, NULL, &g_SocketErrfd, &g_stSocketTimeval);		
		if (iRet < 0)
		{		
			sysLOG(SSL_LOG_LEVEL_2, "select ret < 0, error\r\n");
			return ERR_4G_AT_CIPRXGET_2_FAIL;
		}
			
		if(FD_ISSET(sslsock_fd, &g_SocketErrfd))	
		{
			int error = 0;
			int len = 4;
			
			iRet = fibo_sock_getOpt(sslsock_fd, SOL_SOCKET, SO_ERROR,&error, &len);
			
			sysLOG(SSL_LOG_LEVEL_2, "fibo_sock_getOpt,sslsock_fd=%d,error=%d\r\n", sslsock_fd, error);

			return ERR_4G_AT_SOCKCHECKERR_FAIL;
		}
		
		while(1)
		{

			Ret = fibo_ssl_sock_recv(sockid, (unsigned char *)(pucBuff+recvdlen), uiMaxLen-recvdlen);		
			if(Ret == 0)
			{
				break;
			}
			else if(Ret < 0)
			{
				sysLOG(SSL_LOG_LEVEL_1, "<ERR> Ret = %d\r\n", Ret);
				return ERR_4G_READ_DATA_FAIl;
			}
			else
			{
				recvdlen += Ret;
				if(recvdlen >= uiMaxLen)
				{
					
					sysLOG(SSL_LOG_LEVEL_1, "<SUCC> hal_wiresslRecv, recvdlen:%d, pucdata:%s\r\n", recvdlen, pucBuff);
					return recvdlen;
				}
			}

			if(hal_sysGetTickms() > uTime)
			{				
				sysLOG(SSL_LOG_LEVEL_3, "timeout, hal_sysGetTickms():%lld\r\n", hal_sysGetTickms());
				return recvdlen;
			}
			
			sysDelayMs(20);

		}

		if(hal_sysGetTickms() > uTime)
		{	
			sysLOG(SSL_LOG_LEVEL_3, "timeout, hal_sysGetTickms():%lld\r\n", hal_sysGetTickms());
			return recvdlen;
		}
		sysDelayMs(200);
	}
	

}


/*
*Function:		hal_wiresslGetErrcode
*Description:	获取SSL错误码取得的值为最近一次SSL会话中发生的错误
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:正常；<0:SSL错误码
*Others:
*/
int hal_wiresslGetErrcode(void)
{
	return fibo_get_ssl_errcode();
}




/***************************TEST***************************/

#if MAINTEST_FLAG

#if 0

#define TEST_CA_FILE "-----BEGIN CERTIFICATE-----\r\n"			\
"MIID8zCCAtugAwIBAgIJAMGiFaH7f/n1MA0GCSqGSIb3DQEBCwUAMIGOMQswCQYD\r\n"\
"VQQGEwJ6ZzEQMA4GA1UECAwHYmVpamluZzEQMA4GA1UEBwwHaGFpZGlhbjERMA8G\r\n"\
"A1UECgwIdmFuc3RvbmUxFDASBgNVBAsMC2RldmVsb3BtZW50MQ0wCwYDVQQDDAR6\r\n"\
"aGFvMSMwIQYJKoZIhvcNAQkBFhR6aGFvQHZhbnN0b25lLmNvbS5jbjAgFw0xNzAz\r\n"\
"MDQxMTQ3MTVaGA8yMTE3MDIwODExNDcxNVowgY4xCzAJBgNVBAYTAnpnMRAwDgYD\r\n"\
"VQQIDAdiZWlqaW5nMRAwDgYDVQQHDAdoYWlkaWFuMREwDwYDVQQKDAh2YW5zdG9u\r\n"\
"ZTEUMBIGA1UECwwLZGV2ZWxvcG1lbnQxDTALBgNVBAMMBHpoYW8xIzAhBgkqhkiG\r\n"\
"9w0BCQEWFHpoYW9AdmFuc3RvbmUuY29tLmNuMIIBIjANBgkqhkiG9w0BAQEFAAOC\r\n"\
"AQ8AMIIBCgKCAQEArxaR1ZBc04+QaemsAkNPCHCqAA4LkjQdIb13JVMhHMoZbFgS\r\n"\
"fdFDcp/8usHeRJ5uMN6FItZfLm09JN9b3/LvtrEr97we2YO41atYA4RU4SEYYNlA\r\n"\
"sIeXE9X+Kdyc9xHpQdWVHORFsA2w4S/Y/HSuMmKmmEMWCGTYgxTznn5fKebkhSgh\r\n"\
"ft7XsdB/f93wZQvu58jflAFP5NuGrV/EYSXSTOyms0hJj3iqGXsnUNt+hwhj20fP\r\n"\
"gH7+SYrRjPNr9OivH30FBcbHDwPPfbm9v5fRHKJibl7CICub5YIkcNqJgq7M27Dm\r\n"\
"xEsOyq+eMhv4KCMuBLlDWv4ynxeW5yTnEny0CQIDAQABo1AwTjAdBgNVHQ4EFgQU\r\n"\
"jJse4ToGyf6JQo/LyF84/njH854wHwYDVR0jBBgwFoAUjJse4ToGyf6JQo/LyF84\r\n"\
"/njH854wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEADgIjW3ZmEHgu\r\n"\
"i11IYAG/rmlBScYcxkFDVc94JUoqNpmhFmF1taK9wuyPElD1Q0xFWyRDSRnfXypA\r\n"\
"g3brUqxCR9tiov8QRmRcrIErTw+hF8B7tWmJDpbRqz5O4zPkP7OUB/LIHO4kJBYC\r\n"\
"voQQ/jUoGsLEfPXlvA64ma2gVjVvtdSGH1whU5sZHEY+dF0Vp06wko3ToH+9yL+o\r\n"\
"p86IEcrkeod/I2cApz/q3idf2lyIwh3EJQhB9t9YKFbuCLi8UoJV7ePHfiCmJrtn\r\n"\
"EZ8v/LDexm4wI955cZX8y5T13jx/EzkJ6bDzx82XSZokQfqUsA4AISv69MwLYOYf\r\n"\
"l2oRYjzbhg==\r\n"\
"-----END CERTIFICATE-----\r\n"


#define TEST_CLIENT_CRT_FILE "-----BEGIN CERTIFICATE-----\r\n"	\
"MIIDmTCCAoECCQCVBWJvDCU1aTANBgkqhkiG9w0BAQsFADCBjjELMAkGA1UEBhMC\r\n"\
"emcxEDAOBgNVBAgMB2JlaWppbmcxEDAOBgNVBAcMB2hhaWRpYW4xETAPBgNVBAoM\r\n"\
"CHZhbnN0b25lMRQwEgYDVQQLDAtkZXZlbG9wbWVudDENMAsGA1UEAwwEemhhbzEj\r\n"\
"MCEGCSqGSIb3DQEJARYUemhhb0B2YW5zdG9uZS5jb20uY24wHhcNMTcwMzA0MTE1\r\n"\
"NDI0WhcNMjcwMzAyMTE1NDI0WjCBjTELMAkGA1UEBhMCemcxEDAOBgNVBAgMB2Jl\r\n"\
"aWppbmcxEDAOBgNVBAcMB2hhaWRpYW4xETAPBgNVBAoMCHZhbnN0b25lMQ8wDQYD\r\n"\
"VQQLDAZiZWl5YW4xDzANBgNVBAMMBmNsaWVudDElMCMGCSqGSIb3DQEJARYWY2xp\r\n"\
"ZW50QHZhbnN0b25lLmNvbS5jbjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\r\n"\
"ggEBAOH+gXCQJ4f0boidF1wVe/HRbUX/Kpd3iE3WipKz4TOzYn2OJ/020AvoeB+S\r\n"\
"2eFi0Wb0cwrBQKjxNRkXrpOAti2OHjIlLWBe08+BaPnSnHw9v9xZIRiw78pG0Ud8\r\n"\
"1MQnRruFKvXAsZDtwEHLBGI2Ti1W351owerD96HF1cRBUyetuxw7flaqxtZDi57r\r\n"\
"fy3/RPmCkWJ1gOIBD81KduVM/4AfKFUetC/aBu+NR4LTWf/LB70x2Aw/XG5FLPkT\r\n"\
"+kOM2YXIBQJNpblNce+rXk2KmTH9U6K+A9i4ACj3ud76oa42qBQGGWnS82S57FOc\r\n"\
"y2EzF5YbK3av32kx9Gdf2IrLUscCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAhYzN\r\n"\
"pPLYoKXyfrTPy4wPazdAqpoGw3Ld+ycNkgbEOqPiBT1GxxKcuIJ+9dIOyPUJJiTf\r\n"\
"9JFfLDPixoUbEBr8FEwz8f10+GPUO5hiVh6kthHDwODqT9EugsdK1GHsneKsEuZ1\r\n"\
"QPjiJHIrzNhya09+Mxc+FFf9UDTstfGA34CH2GSxBiyqJOEFr2oAvkgxKZrEBLLD\r\n"\
"rbobtrDrWPOiQ5HSKNv4OB97WDrwhuH31AtgbzQvjmKXHWd9X4wDqvX5iwInRkTG\r\n"\
"2rWZpCEQoE5V4f7AH1MrWaSWF9Ai9yqidI4NyLOg3AdrCaXd9Ll0gZRatbQeqO/v\r\n"\
"sanZ8BokHy7XNNf6Zg==\r\n"\
"-----END CERTIFICATE-----\r\n"



#define TEST_CLIENT_KEY_FILE "-----BEGIN RSA PRIVATE KEY-----\r\n"\
"MIIEpQIBAAKCAQEA4f6BcJAnh/RuiJ0XXBV78dFtRf8ql3eITdaKkrPhM7NifY4n\r\n"\
"/TbQC+h4H5LZ4WLRZvRzCsFAqPE1GReuk4C2LY4eMiUtYF7Tz4Fo+dKcfD2/3Fkh\r\n"\
"GLDvykbRR3zUxCdGu4Uq9cCxkO3AQcsEYjZOLVbfnWjB6sP3ocXVxEFTJ627HDt+\r\n"\
"VqrG1kOLnut/Lf9E+YKRYnWA4gEPzUp25Uz/gB8oVR60L9oG741HgtNZ/8sHvTHY\r\n"\
"DD9cbkUs+RP6Q4zZhcgFAk2luU1x76teTYqZMf1Tor4D2LgAKPe53vqhrjaoFAYZ\r\n"\
"adLzZLnsU5zLYTMXlhsrdq/faTH0Z1/YistSxwIDAQABAoIBAQC9QIZhHzcbkURn\r\n"\
"PzZqtMsgzIK31WFzinQoyvsss1pqE1TtU/iFAjvjXQALYMz5A+ncT+VvIjrlv2j5\r\n"\
"5G0btPPa3mLHF38SbxbtdK/WVdsZ6BQdkL44kSOdvwRO22jKAyImsvQw7PdGKPOR\r\n"\
"pVO8c0Gwkkmc5jdORGHBpYzIVvQuCqm6FqYwUz2iu4fS1zr4kayrcnJNjuQ5m++C\r\n"\
"3paIUF6M8eyTMbW92yzoYPl7lfIvO8OtvO6LmGLm7NTJvJTt/+oQ7lWgAwVILXDT\r\n"\
"MOzZAo5zmcE3Wxnu13xy8aoxsaP/OlEYrO4UNh0B98rPxQD8ha09eo8Iry9ISgdU\r\n"\
"0En8ThhBAoGBAPnrebAN5vEEl7Iq8RObivbJ0au87xmpXNGAF3n1Ts51sxKPGc8A\r\n"\
"7KuTEod4/1qZv3T7TzOISTf2Clk7RAHmehpR9siJhxFMJO/Bm5LKsAVLNdxaYP82\r\n"\
"dOr3YRUfc8PD35F6POnT62ohx3GpEvkXq7K8Z6Vxme2onS+JMlnWa8gxAoGBAOd+\r\n"\
"BNpQcgwRYjZSkzj2bmwb3IfLtnojase5kMuEWXkLw20nbqxyGg7y2LkOv0nBYwBb\r\n"\
"p3UcoMKZ6Lt5SwSZw7N4xrXcSUirIMxJkwvFNIgP5qF0UhQQf1oko51ySrueF4ly\r\n"\
"XGetGH+8chtmPaezunQIZPXx1ridK8pUr+cc+IR3AoGBAJu8cAUUdLAGQ51kAvxL\r\n"\
"9gmZdA3H21srXcqzPm3iPmyLjb5n7BZBpjm4mVhQPxevLU5xeEZNjArSSFqYguWI\r\n"\
"QNh78QPAJ6nQwEejZxXPNmbmGjcpHr6KqcrtwtKN2e7I9V1LRgAT7eQiDo22ZTtR\r\n"\
"0826d//xzZD6fJDttrGi22FRAoGAG4bXquIf/aFfjVgaW46qKL7TZW5q33EwtFkz\r\n"\
"h4/QV913OpaSyXyz7o0gCjlfs7SEQjmj2wAxeZJ1oz1UF1L1e3TkJCHysOpwdvpU\r\n"\
"uvSTP46sfQxo6ivgCOcMcZ9ylYbCA70OkeF6ZU4SP4HT2Qo9JKkO4FQGlWNTHrcd\r\n"\
"R50hTBcCgYEAzQoXB7P0RR8m+MkkxuaVbVS59r/H6pxH3sRyuynHFHTuRB28ZI2O\r\n"\
"nXXXxj+95xr7QU5SKzQVJ0pWaKcEQC4fOkxAIRGb+x1DMZZ2tReORlhv/tt1+Vqx\r\n"\
"d/b0sd/xs5uKIkMWJrrRIqA2UOGtIVzn//tOf5aSTUigH9T6FJRW4x0=\r\n"\
"-----END RSA PRIVATE KEY-----\r\n"

#endif
#if 1
#define TEST_CA_FILE "-----BEGIN CERTIFICATE-----\r\n" \
"MIID9TCCAt2gAwIBAgIJAOEIwHHcR9K7MA0GCSqGSIb3DQEBBQUAMIGPMQswCQYD\r\n" \
"VQQGEwJDTjEPMA0GA1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQK\r\n" \
"DAdmaWJvY29tMRAwDgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0\r\n" \
"LjM2MSQwIgYJKoZIhvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wIBcNMTkw\r\n" \
"ODMwMDY1MjUwWhgPMjExOTA4MDYwNjUyNTBaMIGPMQswCQYDVQQGEwJDTjEPMA0G\r\n" \
"A1UECAwGc2hhbnhpMQ0wCwYDVQQHDAR4aWFuMRAwDgYDVQQKDAdmaWJvY29tMRAw\r\n" \
"DgYDVQQLDAdmaWJvY29tMRYwFAYDVQQDDA00Ny4xMTAuMjM0LjM2MSQwIgYJKoZI\r\n" \
"hvcNAQkBFhV2YW5zLndhbmdAZmlib2NvbS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\r\n" \
"A4IBDwAwggEKAoIBAQC6UMQfxHL0oW9pY1cGvq5QPdw8OU7dX2YsCbPdEiXePKce\r\n" \
"E6AN3IKqOuZhEd1iIypXG2AywzIu9bd5w1d4COjjSC/Tpf2AKYw+jqfxHsQAvSKt\r\n" \
"Rvwp1wrS5IvWy8yEG9lNpyVJHBUWlVpU/tUf02MYYU5xUBS+mJE9Tc10j7kd/uV7\r\n" \
"aEfM0pYhm7VmHPWDHXeXj3LKYigjttNxUgpDh2UVpq6ejzzHA5T/k2+XtKtWu7Pb\r\n" \
"ag6lONzz6Zxya9htVLBy7I4uTFrcRPxNgc/KF2BuwEVc4rqGUZ4vpVdwmCyKGIua\r\n" \
"fvit1nsvnhvYMu01HhWuK6e3IO6hOpeyR1wk75ofAgMBAAGjUDBOMB0GA1UdDgQW\r\n" \
"BBTT9RodyqsY/C2WS/7k8GFWidQrlTAfBgNVHSMEGDAWgBTT9RodyqsY/C2WS/7k\r\n" \
"8GFWidQrlTAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUAA4IBAQCkg9dUYBRs\r\n" \
"uqCz71Q75B2n768icIeMfQmf969fNVK/mwaUnFxjqq+4Xw3zADdUdnTZ8FEfjAt2\r\n" \
"LQaxmsiGlM3KmUhXy/k1xKypIu2KecxEX/NqdG02SYcBmrIAP6ZxOxyyJZXbPRBt\r\n" \
"11W3e9+MsRFjRNDxvi5xPcBTu7padUXf7gZp/U8RTc9r0RzsTJu0oFx1Vl6B9m9Z\r\n" \
"4Ae7EshqUrGbnQMJ9XinPVMhuPB4UTc5H9F9ZEswkd/8fK1kXE2aD9LOUD3ITpfH\r\n" \
"h4UBb/UX3VY2eoLC6T5FzPggAcyxU/S2svZaq2+fSWvA7WpEYmTvzQTeT+y1BaUW\r\n" \
"9SoOHidKUkQe\r\n" \
"-----END CERTIFICATE-----\r\n"

#define TEST_CLIENT_CRT_FILE "-----BEGIN CERTIFICATE-----\r\n" \
"MIIEBzCCAu+gAwIBAgIBAzANBgkqhkiG9w0BAQUFADCBjzELMAkGA1UEBhMCQ04x\r\n" \
"DzANBgNVBAgMBnNoYW54aTENMAsGA1UEBwwEeGlhbjEQMA4GA1UECgwHZmlib2Nv\r\n" \
"bTEQMA4GA1UECwwHZmlib2NvbTEWMBQGA1UEAwwNNDcuMTEwLjIzNC4zNjEkMCIG\r\n" \
"CSqGSIb3DQEJARYVdmFucy53YW5nQGZpYm9jb20uY29tMB4XDTE5MDgzMDA3MTI0\r\n" \
"MVoXDTIwMDgyOTA3MTI0MVowgYAxCzAJBgNVBAYTAkNOMQ8wDQYDVQQIDAZzaGFu\r\n" \
"eGkxEDAOBgNVBAoMB2ZpYm9jb20xEDAOBgNVBAsMB2ZpYm9jb20xFjAUBgNVBAMM\r\n" \
"DTQ3LjExMC4yMzQuMzYxJDAiBgkqhkiG9w0BCQEWFXZhbnMud2FuZ0BmaWJvY29t\r\n" \
"LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAK8GXpBMGm3qlN73\r\n" \
"1l30o7NTElBdtAKq0N4l3OZ7ClrZrSmpK0+wx4Hf4zSTc6Gx530cA8Jybr6btwhH\r\n" \
"YOVR7mLWeNEd6jEdzaHVuFLSZc8mjg2AsWoiOqt916D5sEKPyHbNxdcfzso/MJtR\r\n" \
"c5M5tP+tJQscbxyOb61KdiCNYeMit06UGqXl+B1dt/l5ggKcjxhEYlxY75QMPvTT\r\n" \
"Vei1me1WyCutx6L5wPx8j7qK/EVuIstDYJTnPNjRk+YW9tEHBErygzyTzbrXASMq\r\n" \
"zFSOIVjHoJi1+4LOJVOV4Go60L0DR9KunvuiOi8VLe6aqqKTlHW4LNAI5llenKTI\r\n" \
"ovw6RGsCAwEAAaN7MHkwCQYDVR0TBAIwADAsBglghkgBhvhCAQ0EHxYdT3BlblNT\r\n" \
"TCBHZW5lcmF0ZWQgQ2VydGlmaWNhdGUwHQYDVR0OBBYEFCtm3ynu+dcRVshCs8sA\r\n" \
"zsb9JChEMB8GA1UdIwQYMBaAFNP1Gh3Kqxj8LZZL/uTwYVaJ1CuVMA0GCSqGSIb3\r\n" \
"DQEBBQUAA4IBAQA65P1Q9wNio/GdhSGAhO/Bq6jvtKC5ioTJXJta5zHKsKxgg1mh\r\n" \
"HH1WV+sIh5QNfwpxVF9yQkYh9CegRloX/I9oQizCGukVYmLUflpAPXQMiZY6PTC8\r\n" \
"32tIke0kG96RqYBJOAg+S01NJHleurEic7CtxofAaPNY4faJdA7Sx49+/otAbfuW\r\n" \
"ogtI6clCxQA7CrbDxdyMXFmIIGwouwjwK16qgSoCo7lYtvMijBtwVlq03l/K/qke\r\n" \
"JBCkzVByePwFhfGuZ6DH5cI0qdNmmms4WW1/DZi5xdvrKuYr5lj7oxTOJepk53FX\r\n" \
"wzVu2hiFCsqNAiQG4D5IjID29ZjcvbPVfj/p\r\n" \
"-----END CERTIFICATE-----\r\n"

#define TEST_CLIENT_KEY_FILE "-----BEGIN RSA PRIVATE KEY-----\r\n" \
"MIIEowIBAAKCAQEArwZekEwabeqU3vfWXfSjs1MSUF20AqrQ3iXc5nsKWtmtKakr\r\n" \
"T7DHgd/jNJNzobHnfRwDwnJuvpu3CEdg5VHuYtZ40R3qMR3NodW4UtJlzyaODYCx\r\n" \
"aiI6q33XoPmwQo/Ids3F1x/Oyj8wm1Fzkzm0/60lCxxvHI5vrUp2II1h4yK3TpQa\r\n" \
"peX4HV23+XmCApyPGERiXFjvlAw+9NNV6LWZ7VbIK63HovnA/HyPuor8RW4iy0Ng\r\n" \
"lOc82NGT5hb20QcESvKDPJPNutcBIyrMVI4hWMegmLX7gs4lU5XgajrQvQNH0q6e\r\n" \
"+6I6LxUt7pqqopOUdbgs0AjmWV6cpMii/DpEawIDAQABAoIBADrTbjcbjQqCfJTQ\r\n" \
"QdmEXvznn9EpHVaEKP1xRmSk2B8e6GeHN0pqhTOul0PVh1jCXaacIttY8MXZulPr\r\n" \
"AbMxrWjE4wiOAGePt8x7857KnnNYZwg8x+R/Kq729eFh6o8EmoDrumIKi8tIH8Mk\r\n" \
"Ri8mhyIkBL5OST4U1Y4t57QbMNpRA3bSAIoD6/QxaZ9t2/m7IUyClf1KKFbXCaTL\r\n" \
"6FZh68mXoJpPNV75rXTq9TNYtVO2k7h6oW8iuu5UwnQQpXBkLloOm6VMmFRCFuC+\r\n" \
"fh2LBgxTEkFdfDJnJpeuEVCc558zPonLvKVmD8rkaCLHIETePm9R5JOC/Rv9ROH1\r\n" \
"tBgrQwkCgYEA1DoL2Qu1A2xg8jJfHPOqv17vlrxa4ENq2xG1yG0siKOpd6X1MRwc\r\n" \
"dc3uh5DAA/80KTn7xFArB1KHweosUGIO9fmU/gkvXfLlGQnNsgIdKFZvFyM1sTP5\r\n" \
"fdXcsJS5fKAcC1CsvPVvpxJyiT3LhGCRO4hfmcl6jJ9OzsnW/eplIh8CgYEA0x//\r\n" \
"csZ0V1OoXQeTsJ5QRod8Qh0dPwkLqDQsVZyb4bllO2TkC/dLyqVowijpHxBh5Elq\r\n" \
"jCa/jjNSYGo4hiKdvlriYEFTDebDC3SXxdcv86ixC0vH4zKti8NDKhunVIG/8fCi\r\n" \
"iqvB/Tjf7SQEGWRqjykyLRVeH4kDF03kLJW3TDUCgYEAh4wzeQMzL+aO3OIzQYiX\r\n" \
"6/a0y++tk0M8AoODOWoRYYw2dwb2XdF4k/1ddhSLr4HWTOaN2Uri0KBzuPTaLNUU\r\n" \
"fSJVeRNgv36duKo8SI91FAhwl7STXIS3uxlXBSlYdzLD9q4mReH02B6+LM3dKMWM\r\n" \
"vRtTBCRdM2ekrAraV/7XbT0CgYBAy+dIwKPgUWqw8qxfXpdgriBy4iChwhLz0t9w\r\n" \
"fxpQkugA7JwZGBMI5O9b99ZklFCXEflDfnj4GcRElxU2BdXIIHit9h6Ze6ONFoGm\r\n" \
"VL8A11tPDjkQ//LHnGw2tjoK86+Hf8VDLifhod0IGS+w42LZAVnHAHHc1948/sjy\r\n" \
"7hhNqQKBgBs5xypuEz9MMof6+6vxFAWDmTMuZr7CVrgR3AaksHHD1w5Pz0J6wp6I\r\n" \
"kOs3Yx8d14lzom1voiO/MxBpiFssymxLl/0XAtY6NwwMYpvxgGntjABZJsTZ+xsv\r\n" \
"SVqKuwoZ7c0EXnBRKTcWwIhN/YgrEZ2ljqnseDAARdXFEY+Ga070\r\n" \
"-----END RSA PRIVATE KEY-----\r\n"

#endif

uint8 pssltmp[256] = {\
	0x10, 0xDA, 0x01, 0x00, 0x04, 0x4D, 0x51, 0x54, 0x54, 0x04, 0xC0, 0x02, 0x58, 0x00, 0x73, 0x61, \
	0x31, 0x48, 0x63, 0x72, 0x6C, 0x4B, 0x6F, 0x55, 0x4F, 0x41, 0x2E, 0x30, 0x30, 0x30, 0x34, 0x34, \
	0x30, 0x30, 0x30, 0x30, 0x33, 0x36, 0x7C, 0x74, 0x69, 0x6D, 0x65, 0x73, 0x74, 0x61, 0x6D, 0x70, \
	0x3D, 0x32, 0x35, 0x32, 0x34, 0x36, 0x30, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2C, 0x5F, \
	0x76, 0x3D, 0x73, 0x64, 0x6B, 0x2D, 0x63, 0x2D, 0x33, 0x2E, 0x30, 0x2E, 0x31, 0x2C, 0x73, 0x65, \
	0x63, 0x75, 0x72, 0x65, 0x6D, 0x6F, 0x64, 0x65, 0x3D, 0x33, 0x2C, 0x73, 0x69, 0x67, 0x6E, 0x6D, \
	0x65, 0x74, 0x68, 0x6F, 0x64, 0x3D, 0x68, 0x6D, 0x61, 0x63, 0x73, 0x68, 0x61, 0x32, 0x35, 0x36, \
	0x2C, 0x6C, 0x61, 0x6E, 0x3D, 0x43, 0x2C, 0x67, 0x77, 0x3D, 0x30, 0x2C, 0x65, 0x78, 0x74, 0x3D, \
	0x30, 0x7C, 0x00, 0x17, 0x30, 0x30, 0x30, 0x34, 0x34, 0x30, 0x30, 0x30, 0x30, 0x33, 0x36, 0x26, \
	0x61, 0x31, 0x48, 0x63, 0x72, 0x6C, 0x4B, 0x6F, 0x55, 0x4F, 0x41, 0x00, 0x40, 0x36, 0x46, 0x43, \
	0x36, 0x44, 0x33, 0x31, 0x34, 0x42, 0x38, 0x46, 0x43, 0x36, 0x35, 0x44, 0x35, 0x39, 0x32, 0x38, \
	0x43, 0x35, 0x36, 0x32, 0x41, 0x34, 0x37, 0x39, 0x38, 0x30, 0x37, 0x45, 0x37, 0x41, 0x31, 0x33, \
	0x32, 0x44, 0x30, 0x42, 0x41, 0x30, 0x35, 0x46, 0x38, 0x44, 0x42, 0x33, 0x37, 0x37, 0x36, 0x31, \
	0x36, 0x45, 0x32, 0x45, 0x44, 0x36, 0x46, 0x30, 0x42, 0x33, 0x32, 0x44, 0x38};

uint8 p1[] = {
	0x82, 0x26, 0x00, 0x02, 0x00, 0x21, 0x2F, 0x61, 0x31, 0x48, 0x63, 0x72, 0x6C, 0x4B, 0x6F, 0x55, \
	0x4F, 0x41, 0x2F, 0x30, 0x30, 0x30, 0x34, 0x34, 0x30, 0x30, 0x30, 0x30, 0x33, 0x36, 0x2F, 0x75, \
	0x73, 0x65, 0x72, 0x2F, 0x67, 0x65, 0x74, 0x01};

void hal_wiresslTest(void)
{
	int32 iRet;
	//sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
    
    //如果需要验证服务器的证书，将这个值设置为１，否则设置为０
    hal_wiresslSetMode(1);

    //如果上一步（fibo_set_ssl_chkmode）的值设置为１，则这里必须加载自己信任的ca证书，否则会连接失败
    //fibo_write_ssl_file("TRUSTFILE", (uint8 *)TEST_CA_FILE, sizeof(TEST_CA_FILE) - 1);

    //如果服务器也需要验证客户端，那么这里需要加载客户端的证书和私钥，否则会连接失败
    //fibo_write_ssl_file("CAFILE", (uint8 *)TEST_CLIENT_CRT_FILE, sizeof(TEST_CLIENT_CRT_FILE) - 1);
    //fibo_write_ssl_file("CAKEY", (uint8 *)TEST_CLIENT_KEY_FILE, sizeof(TEST_CLIENT_KEY_FILE) - 1);

	hal_wiresslDefault();
	hal_wiresslSetSslVer(0);

    sysDelayMs(2000);

    UINT8 buf[2048] = {0};
#if 0
    UINT8 p[] = "POST /gzcs/terminal/getTerminalParams.do HTTP/1.1\
Host: sys00.jiewen.com.cn\
sign: F94BAD5C903DA529D6B86234AC98DF37\
Content-Length: 185\
Content-Type: application/json\
\
{\"device_sn\":\"AT00000002\",\"app_version\":\"V1.4\",\"version\":\"V1.0.0\",\"device_type\":\"Q180\",\"manufacturer\":\"Vanstone\",\"nonce_str\":\"21E2780E7FB307B93AADC76F43AE5026\",\"connection_mode\":\"WIFI\"}";
#endif
#if 0
	uint8 p[] = "POST /terminal/termparam/v1.do HTTP/1.1\
Host: bank-sp.vanstone.com.cn\
sign: 7BFC4A44BB14EC615C658FC1209E084E\
Content-Length: 290\
Content-Type: application/json\
\
{\"device_sn\":\"00044000036\",\"app_version\":\"V1.00.32\",\"version\":\"V1.0.7\",\"hardware_version\":\"V1.5\",\"gprs_version\":\"V2.3.17\",\"wifi_version\":\"V1.01.03\",\"customer_flag\":\"0102\",\"device_type\":\"Q181\",\"manufacturer\":\"Vanstone\",\"nonce_str\":\"EC868171BD8664F47D2A8FFFAFF29DBF\",\"connection_mode\":\"WIFI\"}";
#endif


	INT32 sock = hal_wiresslSocketCreate();
    if (sock == -1)
    {
    
		sysLOG(BASE_LOG_LEVEL_1, "<ERR> create ssl sock failed\r\n");
        return;//fibo_thread_delete();
    }
	
	sysLOG(BASE_LOG_LEVEL_1, "fibossl fibo_ssl_sock_create %x\r\n", sock);
	/*阿里："a1HcrlKoUOA.iot-as-mqtt.cn-shanghai.aliyuncs.com",443*/
	/*贵州银行：https://pos.bgzchina.com:6500/terminal/getTerminalParams.do*/
	/* sys00.jiewen.com.cn,443*/
	/*广和通："47.110.234.36", 8888*/

	int ret = hal_wiresslConnect(sock, "a1HcrlKoUOA.iot-as-mqtt.cn-shanghai.aliyuncs.com", "443", 10000);//103.235.231.22：8739
	sysLOG(BASE_LOG_LEVEL_1, "fibossl sys_sock_connect %d\r\n", ret);
#if 1

    ret = hal_wiresslSend(sock, pssltmp, sizeof(pssltmp));	
	sysLOG(BASE_LOG_LEVEL_1, "fibossl sys_sock_send %d\r\n", ret);

    ret = hal_wiresslRecv(sock, buf, 1, 1000);	
	sysLOG(BASE_LOG_LEVEL_1, "fibossl sys_sock_recv %d\r\n", ret);
	ret = hal_wiresslRecv(sock, buf, 2, 1000);	
	sysLOG(BASE_LOG_LEVEL_1, "fibossl sys_sock_recv %d\r\n", ret);
    if (ret > 0)
    {        
		sysLOG(BASE_LOG_LEVEL_1, "fibossl fibo_ssl_sock_recv %s\r\n", (char *)buf);

		ret = hal_wiresslSend(sock, p1, sizeof(p1));	
		sysLOG(BASE_LOG_LEVEL_1, "fibossl sys_sock_send %d\r\n", ret);

  	  	ret = hal_wiresslRecv(sock, buf, sizeof(buf), 1000);	
		sysLOG(BASE_LOG_LEVEL_1, "fibossl sys_sock_recv %d\r\n", ret);
		sysLOG(BASE_LOG_LEVEL_1, "fibossl fibo_ssl_sock_recv %s\r\n", (char *)buf);
	}
	hal_wiresslSocketClose(sock);
	ret = hal_wiresslGetErrcode();
	sysLOG(BASE_LOG_LEVEL_1, "fibo_get_ssl_errcode, ret = %d\r\n", ret);
	
	uint32 port = 6500;

	//wifiOpen_lib();
	//sysDelayMs(10000);
   //iRet = wifiCommConnect_lib("SSL", "pos.bgzchina.com", "6500", 50000);
	
	sysLOG(BASE_LOG_LEVEL_1, "wifiCommConnect_lib, iRet = %d\r\n", iRet);
    //fibo_thread_delete();
#endif
}

#endif


