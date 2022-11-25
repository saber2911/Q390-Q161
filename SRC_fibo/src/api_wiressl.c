/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_wiressl.c
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
*Function:		wirelessSetSslVer_lib
*Description:	SSL版本设置
*Input:			ucVer：1-SSL3.0;2-TLS1.0;3-TLS1.1;4-TLS1.2
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败
*Others:
*/
int wirelessSetSslVer_lib(unsigned char ucVer)
{
	return hal_wiresslSetSslVer(ucVer);
}


/*
*Function:		wirelessSslDefault_lib
*Description:	SSL工作方式默认设置
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void wirelessSslDefault_lib(void)
{
	hal_wiresslDefault();
}


/*
*Function:		wirelessSendSslFile_lib
*Description:	下载证书
*Input:			ucType: 证书类型：0-CA File; 1-KEY File; 2-TRUST File
*				*pucData: 数据指针；
*				iLen: 数据长度；
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int wirelessSendSslFile_lib (unsigned char ucType, unsigned char *pucData, int iLen)
{
	return hal_wiresslSendCertFile(ucType, pucData, iLen);
}


/*
*Function:		wirelessSetSslMode_lib
*Description:	设置认证模式
*Input:			ucMode:1-双向验证；0-不需要验证
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int wirelessSetSslMode_lib(unsigned char  ucMode)
{
	return hal_wiresslSetMode(ucMode);
}


/*
*Function:		wirelessSslSocketCreate_lib
*Description:	创建一个安全socket
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		-1:失败；其他值：成功，值为SSL Socket ID
*Others:
*/
int wirelessSslSocketCreate_lib(void)
{
	return hal_wiresslSocketCreate();
}


/*
*Function:		wirelessSslSocketClose_lib
*Description:	释放SSL Socket句柄
*Input:			sockid:已创建的socket ID
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int wirelessSslSocketClose_lib(int sockid)
{
	return hal_wiresslSocketClose(sockid);
}


/*
*Function:		wirelessSslConnect_lib
*Description:	与远端服务器建立SSL连接
*Input:			sockid: socket ID；*putDestIP: 服务端IP/域名；pucDestPort: 端口;timeout:超时时间
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int wirelessSslConnect_lib(int sockid, char *pucDestIP, char *pcDestPort, int timeout)
{
	return hal_wiresslConnect(sockid, pucDestIP, pcDestPort, timeout);
}


/*
*Function:		wirelessSslSend_lib
*Description:	向服务端发送数据
*Input:			sockid:socket ID；*pucData:发送数据缓冲指针；uiLen:发送数据长度
*Output:		NULL
*Hardware:
*Return:		>=0:实际发送的字节数；<0:失败
*Others:
*/
int wirelessSslSend_lib(int sockid, unsigned char *pucData, unsigned int uiLen)
{
	return hal_wiresslSend(sockid, pucData, uiLen);
}


/*
*Function:		wirelessSslRecv_lib
*Description:	接收服务端数据，一次读取最长不超过2048个字节
*Input:			sockid: socket ID; iLen:需要读取的长度; uiTimeOut:超时时间，单位ms
*Output:		*pucdata:读取数据缓存指针
*Hardware:
*Return:		>0:实际读取到的长度；<0:失败; 0:超时
*Others:
*/
int wirelessSslRecv_lib(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut)
{
	return hal_wiresslRecv(sockid, pucBuff, uiMaxLen, uiTimeOut);
}


/*
*Function:		wirelessSslGetErrcode_lib
*Description:	获取SSL错误码取得的值为最近一次SSL会话中发生的错误
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:正常；<0:SSL错误码
*Others:
*/
int wirelessSslGetErrcode_lib(void)
{
	return hal_wiresslGetErrcode();
}





