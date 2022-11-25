/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_wiresocket.c
** Description:		4G 通信及socket相关接口
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


int wirelessNtp_lib(char *pucIP, unsigned int uiPort, uint32 timeout)
{
	return hal_wiresockNtp(pucIP, uiPort, timeout);
}

int wirelessGetDNS_lib(char *pri_dns, char *sec_dns)
{
	return hal_wiresockGetDNS(pri_dns, sec_dns);
}

int wirelessSetDNS_lib(const char *pri_dns, const char *sec_dns)
{
	return hal_wiresockSetDNS(pri_dns, sec_dns);
}



/*
*Function:		wirelessGetIP_lib
*Description:	获取注网IP
*Input:			iIPlen= pcIP的内存空间大小
*Output:		pcIP：IP值，iIPlen返回IP长度
*Hardware:
*Return:		0:成功；<0：失败(ERR_4G_AT_CGATT_FAIL 未注网，ERR_4G_AT_BUFFOVER pcIP空间不够；
*Others:
*/
int wirelessGetIP_lib(unsigned char *pcIP, int *iIPlen)
{
	return hal_wiresockGetIP(pcIP, iIPlen);
}


/*
*Function:		wirelessSetNtpEn_lib
*Description:	设置pdp激活后是否做NTP对时的使能状态，掉电保存
*Input:			*value：0-不做对时,1-做NTP对时
*Output:		NULL
*Hardware:
*Return:		0:成功；其他:失败
*Others:
*/
int wirelessSetNtpEn_lib(int *value)
{
	return hal_wirelessSetNtpEn(value);
}


/*
*Function:		wirelessGetNtpEn_lib
*Description:	读取pdp激活后是否做NTP对时的使能状态,默认值为1
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:不做NTP对时；1:做NTP对时；<0:失败
*Others:
*/
int wirelessGetNtpEn_lib(void)
{
	return hal_wirelessGetNtpEn();
}


/*
*Function:		wirelessPdpWriteParam_lib
*Description:	写用户配置文件,重启机器后参数生效
*Input:			*apn:apn字符串；*username:用户名；*password:密码，如果有的不需要设置，则传NULL即可
*Output:		NULL
*Hardware:
*Return:		<0:失败，0:成功
*Others:
*/
int wirelessPdpWriteParam_lib(char *apn, char *username, char *password)
{
	
	return hal_wirePdpWriteParam(apn, username, password);

}

/*
*Function:		wirelessPppOpenex_lib
*Description:	模块pdp激活
*Input:			*pucApn:APN;*pucUserName:用户名;*pucPassword:密码;timeout:超时时间
*Output:		NULL
*Hardware:
*Return:		0:激活成功；ERR_4G_AT_CGATT_FAIL：激活失败；ERR_TIMEOUT:超时；
*Others:
*/
int wirelessPppOpenex_lib(unsigned char *pucApn, unsigned char *pucUserName, unsigned char *pucPassword, unsigned int timeout)
{
	return hal_wiresockPppOpenex(pucApn, pucUserName, pucPassword, timeout);
}


/*
*Function:		wirelessPppOpen_lib
*Description:	模块pdp激活
*Input:			*pucApn:APN;*pucUserName:用户名;*pucPassword:密码
*Output:		NULL
*Hardware:
*Return:		0:激活成功；ERR_4G_AT_CGATT_FAIL：激活失败；ERR_TIMEOUT:超时；
*Others:
*/
int wirelessPppOpen_lib(unsigned char *pucApn, unsigned char *pucUserName, unsigned char *pucPassword)
{
	return hal_wiresockPppOpen(pucApn, pucUserName, pucPassword);
}


/*
*Function:		wirelessCheckPppDial_lib
*Description:	查询PDP激活状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:已激活; ERR_4G_AT_NOREG_FAIL:未激活；ERR_4G_AT_CEREG_FAIL:超时,查询失败
*Others:
*/
int wirelessCheckPppDial_lib(void)
{
	return hal_wiresockCheckPppDial();
}



/*
*Function:		wirelessSocketCreate_lib
*Description:	创建一个socket
*Input:			nProtocol: 协议类型，0-TCP；1-UDP
*Output:		NULL
*Hardware:
*Return:		ERR_4G_AT_SOCKCREATE_FAIL: 失败; >=0: 成功，该值为创建的socket ID
*Others:
*/
int wirelessSocketCreate_lib(int nProtocol)
{
	return hal_wiresockSocketCreate(nProtocol);
}


/*
*Function:		wirelessSocketClose_lib
*Description:	关闭已经打开的socket
*Input:			sockid: socket ID
*Output:		NULL
*Hardware:
*Return:		0: 成功; ERR_4G_AT_SOCKCOLOSE_FAIL: 失败
*Others:
*/
int wirelessSocketClose_lib(int sockid)
{
	return hal_wiresockSocketClose(sockid);
}


/*
*Function:		wirelessTcpConnect_lib
*Description:	与远端socket建立连接
*Input:			sockid: socket ID; *pucIP:服务端IP/域名; uiPort:端口号
*Output:		NULL
*Hardware:
*Return:		0:成功; ERR_4G_TCP_CONNECT_FAIL:失败
*Others:
*/
int wirelessTcpConnect_lib(int sockid, char *pucIP, char *pucPort, int timeout)
{
	return hal_wiresockTcpConnect(sockid, pucIP, pucPort, timeout);
}


/*
*Function:		wirelessSend_lib
*Description:	TCP/UDP数据发送
*Input:			sockid: socket ID; *pucdata: 发送数据指针; uiLen: 发送数据长度
*Output:		NULL
*Hardware:
*Return:		>=0: 返回实际发送的数据长度; ERR_4G_SEND_DATA_FAIL: 失败
*Others:
*/
int wirelessSend_lib(int sockid, unsigned char *pucData, unsigned int uiLen)
{
	return hal_wiresockSend(sockid, pucData, uiLen);
}


/*
*Function:		wirelessRecv_lib
*Description:	接收TCP数据，一次读取最大不要超过2048个字节
*Input:			sockid: socket ID; iLen: 本次可接收的最大长度; uiTimeOut: 超时时间，单位ms 
*Output:		*pucdata: 接收数据指针;
*Hardware:
*Return:		>0:实际读取到的长度；<0:失败; ERR_TIMEOUT:超时
*Others:
*/
int wirelessRecv_lib(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut)
{
	return hal_wiresockRecv(sockid, pucBuff, uiMaxLen, uiTimeOut);
}


/*
*Function:		wirelessGetIMSIAndIMEI_lib
*Description:	读取IMSI和IMEI接口
*Input:			NULL
*Output:		指针不能为空，*pucIMSI: IMSI接收指针; *pucIMEI: IMEI接收指针
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int wirelessGetIMSIAndIMEI_lib(unsigned char *pucIMSI, unsigned char *pucIMEI)
{
	return hal_wiresockGetIMSIAndIMEI(pucIMSI, pucIMEI);
}


/*
*Function:		wirelessGetCSQ_lib
*Description:	获取模块CSQ信号强度等
*Input:			NULL
*Output:		*rssi:信号强度值，*ber:误码率
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int wirelessGetCSQ_lib(int *rssi, int *ber)
{
	return hal_wiresockGetCSQ(rssi, ber);
}


/*
*Function:		wirelessGetSingnal_lib
*Description:	获取信号强度等级
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0:信号强度等级; 其他:失败
*Others:
*/
int wirelessGetSingnal_lib(void)
{
	return hal_wiresockGetSingnal();
}


/*
*Function:		wirelessGetCCID_lib
*Description:	获取模块ccid号
*Input:			NULL
*Output:		*ccid: ccid号指针
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int wirelessGetCCID_lib(unsigned char *ccid)
{
	return hal_wiresockGetCCID(ccid);
}


/*
*Function:		wirelessGetSimStatus_lib
*Description:	获取SIM卡插拔状态
*Input:			NULL
*Output:		*status:状态指针，1-已插卡；0-未插卡
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int wirelessGetSimStatus_lib(unsigned char *status)
{
	return hal_wiresockGetSimStatus(status);
}


/*
*Function:		wirelessGetRegInfo_lib
*Description:	获取注册状态；0/2/3/4 表示未注册，其他值表示注册
*Input:			NULL
*Output:		*status:注册状态指针
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int wirelessGetRegInfo_lib(unsigned char *status)
{
	return hal_wiresockGetRegInfo(status);
}


/*
*Function:		wirelessGetPdpStatus_lib
*Description:	获取PDP状态，默认cid=CID1
*Input:			cid: 指定已激活的PDP profile ID
*Output:		*status:输出激活状态，0-未激活;1-已激活
*Hardware:
*Return:		0:成功，<0失败
*Others:
*/
int wirelessGetPdpStatus_lib(char cid, unsigned char *status)
{
	return hal_wiresockGetPdpStatus(cid, status);
}


/*
*Function:		wirelessPppClose_lib
*Description:	PDP去激活
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int wirelessPppClose_lib(void)
{
	return hal_wiresockPppClose();
}


/*
*Function:		wirelessGetGSMorLTE_lib
*Description:	获取注册的是4G还是2G网络
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败；0:未读取到；1:4G网络；2:2G网络
*Others:
*/
int wirelessGetGSMorLTE_lib(void)
{
	return hal_wiresockGetGSMorLTE();
}


/*
*Function:		wirelessSetGSMorLTE_lib
*Description:	设置注册的是4G还是2G网络,掉电不保存,切换网络后必须重新pdpOpen
*Input:			mode:0-Auto,LTE优先;1-LTE only;2-GSM only
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功
*Others:
*/
int wirelessSetGSMorLTE_lib(int mode)
{
	return hal_wiresockSetGSMorLTE(mode);
}


/*
*Function:		wirelessSetGSMorLTEex_lib
*Description:	设置注册的是4G还是2G网络,掉电保存,切换网络后必须重新pdpOpen
*Input:			mode:0-Auto,LTE优先;1-LTE only;2-GSM only
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功
*Others:
*/
int wirelessSetGSMorLTEex_lib(int mode)
{
	return hal_wiresockSetGSMorLTEex(mode);
}


/*
*Function:		wirelessGetCellInfo_lib
*Description:	读取小区信息
*Input:			*cellinfoall_info:CELLINFOALL_STR指针，timeout：超时时间
*Output:		NULL
*Hardware:
*Return:		>=0:opencpu_cellinfo_t里面有数据,读到的小区个数，<0:失败
*Others:
*/
int wirelessGetCellInfo_lib(  CellInfoAll_Str *cellinfoall_info, uint32 timeout)
{
	return hal_wiresockGetCellInfo(cellinfoall_info, timeout);
}


/*
*Function:		wirelessGetHostByName_lib
*Description:	域名解析
*Input:			*Domainname：域名；*addr：解析后的ip
*Output:		NULL
*Hardware:
*Return:		0-succ;<0-failed
*Others:
*/
int wirelessGetHostByName_lib(char *Domainname, char *addr)
{
	return hal_wiresockGetHostByName(Domainname, addr);
}






