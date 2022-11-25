/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_wiresocket.c
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

uint32 g_ui32MutexPdpOpenHandle;
uint PdpOpenStatus = 0;

struct timeval g_stSocketTimeval;
fd_set g_SocketReadfd;
fd_set g_SocketErrfd;
fd_set g_SocketWritefd;


uint8 wireless_ip[256]= {0};//注网IP

int rtcNTPflag = 0;

static int g_iWirelessCSQValue = 0;


#define in_range(c, lo, up) ((u8_t)c >= lo && (u8_t)c <= up)
#define HTONL(x) (x) = htonl((INT32)x)

unsigned long htonl(unsigned long n)
{
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
}

unsigned short htons(unsigned short n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

char *inet_ntoa_self(unsigned int addr)
{
	static char buffer[16];
	
	memset(buffer, 0, sizeof(buffer));
	
    unsigned char *bytes = (unsigned char *)&addr;
    snprintf( buffer, sizeof (buffer), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    return buffer;
}

int ip4addr_aton(const char *cp, ip4_addr_t *addr)
{
    u32_t val;
    u8_t base;
    char c;
    u32_t parts[4];
    u32_t *pp = parts;

    c = *cp;
    for (;;)
    {
        /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
        if (!isdigit(c))
        {
            return 0;
        }
        val = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c = *++cp;
            }
            else
            {
                base = 8;
            }
        }
        for (;;)
        {
            if (isdigit(c))
            {
                val = (val * base) + (u32_t)(c - '0');
                c = *++cp;
            }
            else if (base == 16 && isxdigit(c))
            {
                val = (val << 4) | (u32_t)(c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            }
            else
            {
                break;
            }
        }
        if (c == '.')
        {
            /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
            if (pp >= parts + 3)
            {
                return 0;
            }
            *pp++ = val;
            c = *++cp;
        }
        else
        {
            break;
        }
    }
    /*
   * Check for trailing characters.
   */
    if (c != '\0' && !isspace(c))
    {
        return 0;
    }
    /*
   * Concoct the address according to
   * the number of parts specified.
   */
    switch (pp - parts + 1)
    {

    case 0:
        return 0; /* initial nondigit */

    case 1: /* a -- 32 bits */
        break;

    case 2: /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
        {
            return 0;
        }
        if (parts[0] > 0xff)
        {
            return 0;
        }
        val |= parts[0] << 24;
        break;

    case 3: /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4: /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    default:
        break;
    }
    if (addr)
    {
        ip4_addr_set_u32(addr, val);
    }
    return 1;
}


/*
*Function:		hal_wiresockBufIsWritten
*Description:	判断一个数组是否被写入数据
*Input:			*buf-数组指针; len-数组长度
*Output:		NULL
*Hardware:
*Return:		0-数组没有被写入；1-数组又被写入
*Others:
*/
static int32 hal_wiresockBufIsWritten(uint8 *buf, int32 len)
{
	int32 iRet = 0;
	for(int32 i = 0; i < len; i++)
	{
		if(*(buf+i) != 0)
		{
			return 1;
		}
	}
	return iRet;
}


int hal_GetBpVersion(void)
{
	int ret;
	char bpversion[256];	
	int bpversion_num = 065707;
	int tab_version = 0;

	memset(bpversion, 0, sizeof(bpversion));
	ret = sysReadBPVersion_lib(bpversion);
	if(ret < 0)
		return bpversion_num;
	
	if(NULL != cus_export_api)
	{
		tab_version = cus_export_api->tab_ver;
	}
	
	ret = GetNumFromAscii(bpversion, 0, '.', 2, 1);
	bpversion_num = ret*10000;
	ret = GetNumFromAscii(bpversion, 3, '.', 2, 1);
	bpversion_num += ret*100;
	ret = GetNumFromAscii(bpversion, 6, '.', 2, 1);
	bpversion_num += ret;
//	return bpversion_num;

	cus_export_api_ver = tab_version;
	sysLOG(SOCKET_LOG_LEVEL_1, "cus_export_api_ver:%d\r\n", cus_export_api_ver);

	return tab_version;
	
}


int hal_JudgeBpVersion(void)
{

	return cus_export_api_ver;
	
}


static fibo_ntp_rsp NTPrsp;
static int8 NTPrspFlag = 0;
void hal_wiresockNtpCallback(void *param)
{
	NTPrspFlag = 1;
	
	memcpy(&NTPrsp, param, sizeof(fibo_ntp_rsp));
	sysLOG(SOCKET_LOG_LEVEL_2, "result:%d, timestamp=%d\r\n", NTPrsp.result, NTPrsp.time);
	
}


int hal_wiresockNtp(char *pucIP, unsigned int uiPort, uint32 timeout)
{

	int ret = -1;
	uint urllentmp = 0;
	char *pUrlTmp = NULL;
	unsigned int portTmp = 123;
	unsigned long long uTime;
	unsigned char rtctimeBCD[16];

	NTPrspFlag = 0;

	if(pucIP == NULL)
	{
		urllentmp = 256;
		pUrlTmp = malloc(urllentmp);
		if(pUrlTmp == NULL)
			goto exit;
		memset(pUrlTmp, 0, urllentmp);
		sprintf(pUrlTmp, "ntp.aliyun.com");
		portTmp = 123;
		
	}
	else
	{
		urllentmp = strlen(pucIP)+1;
		pUrlTmp = malloc(urllentmp);
		if(pUrlTmp == NULL)
			goto exit;
		memset(pUrlTmp, 0, urllentmp);
		memcpy(pUrlTmp, pucIP, strlen(pucIP));
		portTmp = uiPort;
		
	}

	uTime = hal_sysGetTickms() + timeout;
	ret = fibo_sock_ntp(pUrlTmp, portTmp, 0, hal_wiresockNtpCallback);
	if(ret < 0)
		goto exit;
	
	while(1)
	{
		if(NTPrspFlag == 1)
		{
			if(NTPrsp.result == 1)
				ret = 0;
			else
				ret = -3;

			break;
		}

		if(hal_sysGetTickms() > uTime)
		{
			sysLOG(SOCKET_LOG_LEVEL_2, "timeout\r\n");
			ret = -2;
			break;
		}
		sysDelayMs(100);

	}
	
	if(ret == 0)
	{
		memset(rtctimeBCD, 0, sizeof(rtctimeBCD));
		ret = sysGetTime_lib(rtctimeBCD);
		if(0 == ret)
			sysSetTimeSE_lib(rtctimeBCD);
	}
	

exit:

	if(pUrlTmp != NULL)
		free(pUrlTmp);
	
	sysLOG(SOCKET_LOG_LEVEL_2, "ret = %d\r\n", ret);
	return ret;

}


int hal_wiresockGetDNS(char *pri_dns, char *sec_dns)
{
	int ret;

	ret = hal_JudgeBpVersion();
	sysLOG(SOCKET_LOG_LEVEL_2, "hal_JudgeBpVersion:%d\r\n", ret);
	if(ret < 1)
	{
		return 0;
	}

	if(NULL != cus_export_api)
	{
		ret = cus_export_api->fibo_get_global_DNS(pri_dns, sec_dns);
		sysLOG(SOCKET_LOG_LEVEL_2, "cus_export_api\r\n");
	}
	else
	{
		sysLOG(SOCKET_LOG_LEVEL_2, "NULL == cus_export_api\r\n");
		ret = 0;
	}
	
//	ret = fibo_get_global_DNS(pri_dns, sec_dns);

	return ret;
}

int hal_wiresockSetDNS(const char *pri_dns, const char *sec_dns)
{
	int ret;
	char dns1[64], dns2[64];

	ret = hal_JudgeBpVersion();
	sysLOG(SOCKET_LOG_LEVEL_2, "hal_JudgeBpVersion:%d\r\n", ret);
	if(ret < 1)
	{
		return 0;
	}
	if(NULL != cus_export_api)
	{
		memset(dns1,0, sizeof(dns1));
		memset(dns2,0, sizeof(dns2));
		wirelessGetDNS_lib(dns1, dns2);
		sysLOG(SOCKET_LOG_LEVEL_2, "dns1:%s, dns2:%s\r\n", dns1, dns2);

		ret = cus_export_api->fibo_set_global_DNS(pri_dns, sec_dns);
		sysLOG(SOCKET_LOG_LEVEL_2, "cus_export_api\r\n");

		memset(dns1,0, sizeof(dns1));
		memset(dns2,0, sizeof(dns2));
		wirelessGetDNS_lib(dns1, dns2);
		sysLOG(SOCKET_LOG_LEVEL_2, "ret=%d,dns1:%s, dns2:%s\r\n", ret, dns1, dns2);

	}
	else
	{
		sysLOG(SOCKET_LOG_LEVEL_2, "NULL == cus_export_api\r\n");
		ret = 0;
	}
	
	return ret;

}




/*
*Function:		hal_wiresockGetIP
*Description:	获取注网IP
*Input:			iIPlen= pcIP的内存空间大小
*Output:		pcIP：IP值，iIPlen返回IP长度
*Hardware:
*Return:		0:成功；<0：失败(ERR_4G_AT_CGATT_FAIL 未注网，ERR_4G_AT_BUFFOVER pcIP空间不够；
*Others:
*/
int hal_wiresockGetIP(unsigned char *pcIP, int *iIPlen)
{
    int iLen = 0;

	if(pcIP == NULL || iIPlen == NULL)
	{
		return ERR_FUNC_PARAM_INVALID;
	}
	hal_wiresockPppOpen(NULL, NULL, NULL);
    if(hal_wiresockBufIsWritten(wireless_ip, sizeof(wireless_ip)) == 1)
    {
        iLen = strlen(wireless_ip);
		if(iLen > *iIPlen)
		{
			return ERR_4G_AT_BUFFOVER;
		}
		memcpy(pcIP, wireless_ip, iLen);
		*iIPlen = iLen;
		return SOCKET_SSL_OK;
    }
    return ERR_4G_AT_CGATT_FAIL;//未注网
}



/*
*Function:		hal_wirelessSetNtpEn
*Description:	设置pdp激活后是否做NTP对时的使能状态，掉电保存
*Input:			*value：0-不做对时,1-做NTP对时
*Output:		NULL
*Hardware:
*Return:		0:成功；其他:失败
*Others:
*/
int hal_wirelessSetNtpEn(int *value)
{
	return hal_cfgWriteNtpEn(value);
}


/*
*Function:		hal_wirelessGetNtpEn
*Description:	读取pdp激活后是否做NTP对时的使能状态,默认值为1
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:不做NTP对时；1:做NTP对时；<0:失败
*Others:
*/
int hal_wirelessGetNtpEn(void)
{
	int iRet = -1;
	int value = 0;
	
	iRet = hal_cfgReadNtpEn(&value);
	if(iRet == 0)
		iRet = value;
	else if(iRet == -4 || iRet == -5)
		iRet = 1;

	sysLOG(SOCKET_LOG_LEVEL_2, "iRet=%d\r\n", iRet);
	return iRet;
}

#define PDPACTIVE_TYPE		1//0-同步;1-异步
#define PDPACTIVE_IPTYPE	"IPV4V6"//IP,IPV6,IPV4V6

int hal_PdpOpen(UINT8 active, UINT8 nAuthProt, CFW_SIM_ID nSimID, UINT8 *ip)
{
	int iRet = -1;
	int8 *apn_rP = NULL;
	int8 *username_rP = NULL;
	int8 *password_rP = NULL;
	pdp_profile_t pdpprofile;

	memset(&pdpprofile, 0, sizeof(pdpprofile));
	
	pdpprofile.cid = CID1;
	memcpy(pdpprofile.nPdpType, PDPACTIVE_IPTYPE, strlen(PDPACTIVE_IPTYPE));
	
	

	iRet = hal_cfgReadPdpCfg(&g_stPdpCfg);
	if(iRet < 0)//读配置失败，则传NULL即可
	{
		sysLOG(SOCKET_LOG_LEVEL_2, "DR_ReadUserCfgini iRet=%d\r\n", iRet);
#if (PDPACTIVE_TYPE == 0)
		iRet = fibo_PDPActive(active, apn_rP, username_rP, password_rP, nAuthProt, nSimID, ip);
#else
		if(NULL != apn_rP)
		{
			memcpy(pdpprofile.apn, apn_rP, strlen(apn_rP));
		}
		if(NULL != username_rP)
		{
			memcpy(pdpprofile.apnUser, username_rP, strlen(username_rP));
			pdpprofile.apnUserSize = strlen(username_rP);
		}
		if(NULL != password_rP)
		{
			memcpy(pdpprofile.apnPwd, password_rP, strlen(password_rP));
			pdpprofile.apnPwdSize = strlen(password_rP);
		}
		
		iRet = fibo_asyn_PDPActive(active, &pdpprofile, nSimID);
		sysLOG(SOCKET_LOG_LEVEL_2, "fibo_asyn_PDPActive, iRet=%d\r\n", iRet);
		if(iRet < 0)
			goto exit;
#endif

	}
	else
	{
		if(memcmp(g_stPdpCfg.apn, "NULL", 4) != 0)
		{
			apn_rP = malloc(strlen(g_stPdpCfg.apn)+1);
			if(apn_rP == NULL){
				iRet = -1;
				goto exit;
			}
			memset(apn_rP, 0, strlen(g_stPdpCfg.apn)+1);
			memcpy(apn_rP, g_stPdpCfg.apn, strlen(g_stPdpCfg.apn));
		}
		if(memcmp(g_stPdpCfg.username, "NULL", 4) != 0)
		{
			username_rP = malloc(strlen(g_stPdpCfg.username)+1);
			if(username_rP == NULL){
				if(apn_rP != NULL){
					free(apn_rP);
				}
				iRet = -1;
				goto exit;
			}
			memset(username_rP, 0, strlen(g_stPdpCfg.username)+1);
			memcpy(username_rP, g_stPdpCfg.username, strlen(g_stPdpCfg.username));
		}
		if(memcmp(g_stPdpCfg.password, "NULL", 4) != 0)
		{
			password_rP = malloc(strlen(g_stPdpCfg.password)+1);
			if(password_rP == NULL){
				if(apn_rP != NULL){
					free(apn_rP);
				}
				if(username_rP != NULL){
					free(username_rP);
				}
				iRet = -1;
				goto exit;
			}
			memset(password_rP, 0, strlen(g_stPdpCfg.password)+1);
			memcpy(password_rP, g_stPdpCfg.password, strlen(g_stPdpCfg.password));
		}
		if(apn_rP == NULL){
			sysLOG(SOCKET_LOG_LEVEL_2, "apn_rP == NULL\r\n");
		}else{
			sysLOG(SOCKET_LOG_LEVEL_2, "apn_rP=%s\r\n", apn_rP);
		}
		if(username_rP == NULL){
			sysLOG(SOCKET_LOG_LEVEL_2, "username_rP == NULL\r\n");
		}else{
			sysLOG(SOCKET_LOG_LEVEL_2, "username_rP=%s\r\n", username_rP);
		}
		if(password_rP == NULL){
			sysLOG(SOCKET_LOG_LEVEL_2, "password_rP == NULL\r\n");
		}else{
			sysLOG(SOCKET_LOG_LEVEL_2, "password_rP=%s\r\n", password_rP);
		}
		
#if (PDPACTIVE_TYPE == 0)
		iRet = fibo_PDPActive(active, apn_rP, username_rP, password_rP, nAuthProt, nSimID, ip);
#else
		if(NULL != apn_rP)
		{
			memcpy(pdpprofile.apn, apn_rP, strlen(apn_rP));
		}
		if(NULL != username_rP)
		{
			memcpy(pdpprofile.apnUser, username_rP, strlen(username_rP));
			pdpprofile.apnUserSize = strlen(username_rP);
		}
		if(NULL != password_rP)
		{
			memcpy(pdpprofile.apnPwd, password_rP, strlen(password_rP));
			pdpprofile.apnPwdSize = strlen(password_rP);
		}
		
		iRet = fibo_asyn_PDPActive(active, &pdpprofile, nSimID);
		sysLOG(SOCKET_LOG_LEVEL_2, "fibo_asyn_PDPActive, iRet=%d\r\n", iRet);
		if(iRet < 0)
			goto exit;
#endif

		if(apn_rP != NULL){
			free(apn_rP);
		}
		if(username_rP != NULL){
			free(username_rP);
		}
		if(password_rP != NULL){
			free(password_rP);
		}
	}
#if (PDPACTIVE_TYPE == 0)
	if(iRet == 0 && hal_wiresockBufIsWritten(ip, sizeof(ip)) == 1)
	{
		memcpy(wireless_ip, ip, sizeof(ip));
	}
#else
	unsigned long long uTime;
	unsigned char cidstatus_tmp = 0;
	
	uTime = hal_sysGetTickms() + 120000;
	while(1)
	{

		iRet = fibo_PDPStatus(CID1, ip, &cidstatus_tmp, nSimID);
		if(iRet == 0 && hal_wiresockBufIsWritten(ip, sizeof(ip)) == 1)//已PDP激活
		{
			if(cidstatus_tmp == 1)
			{
				sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> fibo_PDPStatus iRet=%d, ip:%s,cidstatus_tmp=%d,CID1\r\n", iRet, ip, cidstatus_tmp);
				memcpy(wireless_ip, ip, sizeof(ip));
				break;
			}
		}

		if(hal_sysGetTickms() > uTime)
		{
			iRet = -2;
			break;
		}

		sysDelayMs(1000);

	}
#endif

exit:


	return iRet;
}

/*
*Function:		hal_wirePdpWriteParam
*Description:	写用户配置文件,重启机器后参数生效
*Input:			*apn:apn字符串；*username:用户名；*password:密码，如果有的不需要设置，则传NULL即可
*Output:		NULL
*Hardware:
*Return:		<0:失败，0:成功
*Others:
*/
int hal_wirePdpWriteParam(char *apn, char *username, char *password)
{
	
	return hal_cfgWritePdpCfg(&g_stPdpCfg, apn, username, password);

}


/*
*Function:		hal_wiresockPppOpenex
*Description:	模块pdp激活
*Input:			*pucApn:APN;*pucUserName:用户名;*pucPassword:密码;timeout:超时时间
*Output:		NULL
*Hardware:
*Return:		0:激活成功；ERR_4G_AT_CGATT_FAIL：激活失败；ERR_TIMEOUT:超时；
*Others:
*/
int hal_wiresockPppOpenex(unsigned char *pucApn, unsigned char *pucUserName, unsigned char *pucPassword, unsigned int timeout)
{
	int32 iRet = -1, Ret = -1;
	uint8 ip[256];
	reg_info_t reg_info;
	unsigned long long uTime;
	uint8 cidstatus_tmp = 0;
	unsigned char SIM_status = 0;

	int ret = 0;
	int8 rtctimgC = 1;
	RTC_time rtctimetmp;

	if(rtcNTPflag == 1)
		rtcNTPflag = 0;

	uTime = hal_sysGetTickms()+5000;
	while(1)
	{
		iRet = fibo_get_sim_status(&SIM_status);
		if(SIM_status == 1)
		{
			break;
		}
		if(hal_sysGetTickms() > uTime)
		{
			break;
		}
		sysDelayMs(200);
	}
	if(SIM_status != 1)
	{
		sysLOG(SOCKET_LOG_LEVEL_2, "<ERR> SIM_status=%d\r\n", SIM_status);
		return -2;//如果没插SIM卡直接return
	}

	fibo_mutex_lock(g_ui32MutexPdpOpenHandle);
	sysLOG(SOCKET_LOG_LEVEL_4, "hal_wiresockPppOpen START\r\n");
	memset(wireless_ip, 0, sizeof(wireless_ip));
	memset(ip, 0, sizeof(ip));
	uTime = hal_sysGetTickms()+timeout;
	while(1)
	{
	    fibo_getRegInfo(&reg_info, 0);
		sysDelayMs(1000);
		
		sysLOG(SOCKET_LOG_LEVEL_4, "reg_info.nStatus=%d\r\n", reg_info.nStatus);
		if(reg_info.nStatus==1)
		{
			
			memset(ip, 0, sizeof(ip));
			iRet = fibo_PDPStatus(CID1, ip, &cidstatus_tmp, 0);
			sysLOG(SOCKET_LOG_LEVEL_4, "fibo_PDPStatus iRet=%d\r\n", iRet);
			if((iRet == 0 && cidstatus_tmp == 1)&& hal_wiresockBufIsWritten(ip, sizeof(ip)) == 1)//已PDP激活
			{	
			    memcpy(wireless_ip, ip, sizeof(ip));
				Ret = 0;
				sysLOG(SOCKET_LOG_LEVEL_1, "<SUCC> fibo_PDPStatus iRet=%d, CID1 wireless_ip=%s\r\n", iRet, wireless_ip);
			}
			else
			{
				PdpOpenStatus = 1;
				memset(ip, 0, sizeof(ip));
				iRet = hal_PdpOpen(1, 0, 0, ip);
			}
			
			
			if(Ret >= 0)
			{

//				ret = wirelessSetDNS_lib("8.8.8.8", "114.114.114.114");
			
				sysGetRTC_lib(&rtctimetmp);
				sysLOG(SOCKET_LOG_LEVEL_2, "0 Local rtctime:%d-%d-%d %02d:%02d:%02d %d\r\n", rtctimetmp.year, rtctimetmp.month, rtctimetmp.day, rtctimetmp.hour, rtctimetmp.min, rtctimetmp.sec, rtctimetmp.wDay);
				while(1)
				{
					if(hal_wirelessGetNtpEn() == 1)	
						ret = wirelessNtp_lib(NULL, 123, 5000);
					else
						ret = 0;
					
					if(ret >= 0)
					{
						rtcNTPflag = 2;
						break;
					}
					rtctimgC --;
					if(rtctimgC <= 0)
						break;
					
					sysDelayMs(500);
				}
				
				sysGetRTC_lib(&rtctimetmp);
				sysLOG(SOCKET_LOG_LEVEL_2, "1 Local rtctime:%d-%d-%d %02d:%02d:%02d %d\r\n", rtctimetmp.year, rtctimetmp.month, rtctimetmp.day, rtctimetmp.hour, rtctimetmp.min, rtctimetmp.sec, rtctimetmp.wDay);
			
				if(rtcNTPflag == 0)
					rtcNTPflag = 1;
			
				if(g_stWifiState.cOpenState == NO_OPEN)//wifi 未open则pdp激活后进休眠
				{
					iRet = hal_sysSetSleepMode(1);
					sysLOG(SOCKET_LOG_LEVEL_1, "hal_sysSetSleepMode(1) iRet=%d, g_stWifiState.cOpenState:%d\r\n", iRet, g_stWifiState.cOpenState);
				}
				else//ccb
				{
					hal_espSleepNotice();
					iRet = hal_sysSetSleepMode(1);
					sysLOG(SOCKET_LOG_LEVEL_1, "hal_sysSetSleepMode(1) iRet=%d, g_stWifiState.cOpenState:%d\r\n", iRet, g_stWifiState.cOpenState);
				}
				
				sysLOG(SOCKET_LOG_LEVEL_4, "<SUCC> hal_wiresockPppOpen\r\n");
				Ret = 0;
				break;
			}
			
				
			
		}

		if(hal_sysGetTickms() > uTime)
		{
		
			sysLOG(SOCKET_LOG_LEVEL_2, "<ERR> reg_info.nStatus:%d\r\n", reg_info.nStatus);
			Ret = ERR_TIMEOUT;
			break;
		}
	}

	PdpOpenStatus = 0;
	fibo_mutex_unlock(g_ui32MutexPdpOpenHandle);
	return Ret;
}


/*
*Function:		hal_wiresockPppOpen
*Description:	模块pdp激活
*Input:			*pucApn:APN;*pucUserName:用户名;*pucPassword:密码
*Output:		NULL
*Hardware:
*Return:		0:激活成功；ERR_4G_AT_CGATT_FAIL：激活失败；ERR_TIMEOUT:超时；
*Others:
*/
int hal_wiresockPppOpen(unsigned char *pucApn, unsigned char *pucUserName, unsigned char *pucPassword)
{
	int32 iRet = -1, Ret = -1;
	uint8 ip[256];
	reg_info_t reg_info;
	unsigned long long uTime;
	uint8 cidstatus_tmp = 0;
	unsigned char SIM_status = 0;

	int ret = 0;
	int8 rtctimgC = 1;
	RTC_time rtctimetmp;

	if(rtcNTPflag == 1)
		rtcNTPflag = 0;

	uTime = hal_sysGetTickms()+5000;
	while(1)
	{
		iRet = fibo_get_sim_status(&SIM_status);
		if(SIM_status == 1)
		{
			break;
		}
		if(hal_sysGetTickms() > uTime)
		{
			break;
		}
		sysDelayMs(200);
	}
	if(SIM_status != 1)
	{
		sysLOG(SOCKET_LOG_LEVEL_2, "<ERR> SIM_status=%d\r\n", SIM_status);
		return -2;//如果没插SIM卡直接return
	}

	fibo_mutex_lock(g_ui32MutexPdpOpenHandle);
	sysLOG(SOCKET_LOG_LEVEL_4, "hal_wiresockPppOpen START\r\n");
	memset(wireless_ip, 0, sizeof(wireless_ip));
	memset(ip, 0, sizeof(ip));
	uTime = hal_sysGetTickms()+50000;
	while(1)
	{
	    fibo_getRegInfo(&reg_info, 0);
		sysDelayMs(1000);
		
		sysLOG(SOCKET_LOG_LEVEL_4, "reg_info.nStatus=%d\r\n", reg_info.nStatus);
		if(reg_info.nStatus==1)
		{
			
			memset(ip, 0, sizeof(ip));
			iRet = fibo_PDPStatus(CID1, ip, &cidstatus_tmp, 0);
			sysLOG(SOCKET_LOG_LEVEL_4, "fibo_PDPStatus iRet=%d,cidstatus_tmp=%d\r\n", iRet, cidstatus_tmp);
			if((iRet == 0 && cidstatus_tmp == 1)&& hal_wiresockBufIsWritten(ip, sizeof(ip)) == 1)//已PDP激活
			{	
			    memcpy(wireless_ip, ip, sizeof(ip));
				Ret = 0;
				sysLOG(SOCKET_LOG_LEVEL_1, "<SUCC> fibo_PDPStatus iRet=%d, CID1 wireless_ip=%s\r\n", iRet, wireless_ip);
			}
			else
			{
				PdpOpenStatus = 1;
				memset(ip, 0, sizeof(ip));
				iRet = hal_PdpOpen(1, 0, 0, ip);
			}
			
			
			if(Ret >= 0)
			{

//				ret = wirelessSetDNS_lib("8.8.8.8", "114.114.114.114");
			
				sysGetRTC_lib(&rtctimetmp);
				sysLOG(SOCKET_LOG_LEVEL_2, "0 Local rtctime:%d-%d-%d %02d:%02d:%02d %d\r\n", rtctimetmp.year, rtctimetmp.month, rtctimetmp.day, rtctimetmp.hour, rtctimetmp.min, rtctimetmp.sec, rtctimetmp.wDay);
				while(1)
				{
					if(hal_wirelessGetNtpEn() == 1)	
						ret = wirelessNtp_lib(NULL, 123, 5000);
					else
						ret = 0;
					
					if(ret >= 0)
					{
						rtcNTPflag = 2;
						break;
					}
					rtctimgC --;
					if(rtctimgC <= 0)
						break;
					
					sysDelayMs(500);
				}
				
				sysGetRTC_lib(&rtctimetmp);
				sysLOG(SOCKET_LOG_LEVEL_2, "1 Local rtctime:%d-%d-%d %02d:%02d:%02d %d\r\n", rtctimetmp.year, rtctimetmp.month, rtctimetmp.day, rtctimetmp.hour, rtctimetmp.min, rtctimetmp.sec, rtctimetmp.wDay);
			
				if(rtcNTPflag == 0)
					rtcNTPflag = 1;
			
				if(g_stWifiState.cOpenState == NO_OPEN)//wifi 未open则pdp激活后进休眠
				{
					iRet = hal_sysSetSleepMode(1);
					sysLOG(SOCKET_LOG_LEVEL_1, "hal_sysSetSleepMode(1) iRet=%d, g_stWifiState.cOpenState:%d\r\n", iRet, g_stWifiState.cOpenState);
				}
				else//ccb
				{
					hal_espSleepNotice();
					iRet = hal_sysSetSleepMode(1);
					sysLOG(SOCKET_LOG_LEVEL_1, "hal_sysSetSleepMode(1) iRet=%d, g_stWifiState.cOpenState:%d\r\n", iRet, g_stWifiState.cOpenState);
				}
				
				sysLOG(SOCKET_LOG_LEVEL_4, "<SUCC> hal_wiresockPppOpen\r\n");
				Ret = 0;
				break;
			}
			
				
			
		}

		if(hal_sysGetTickms() > uTime)
		{
		
			sysLOG(SOCKET_LOG_LEVEL_2, "<ERR> reg_info.nStatus:%d\r\n", reg_info.nStatus);
			Ret = ERR_TIMEOUT;
			break;
		}
	}

	PdpOpenStatus = 0;
	fibo_mutex_unlock(g_ui32MutexPdpOpenHandle);
	return Ret;
}


/*
*Function:		hal_wiresockCheckPppDial
*Description:	查询PDP激活状态
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:已激活; ERR_4G_AT_NOREG_FAIL:未激活；ERR_4G_AT_CEREG_FAIL:超时,查询失败
*Others:
*/
int hal_wiresockCheckPppDial(void)
{
	reg_info_t reg_info;
	int iRet = -1;
	uint8 ip[64];
	unsigned long long uTime;
	uint8 cidstatus_tmp = 0;
	
	uTime = hal_sysGetTickms()+1000;

	while(1)
	{
		if(rtcNTPflag != 0 && PdpOpenStatus == 0)//已对时且已执行完激活
			break;
		if(hal_sysGetTickms() > uTime)
			return -2;

		sysDelayMs(10);
	}
	
	while(1)
	{
	    iRet = fibo_getRegInfo(&reg_info, 0);	
		if(reg_info.nStatus==1)
		{
			
			memset(ip, 0, sizeof(ip));
			iRet = fibo_PDPStatus(CID1, ip, &cidstatus_tmp, 0);
			if(iRet == 0 && hal_wiresockBufIsWritten(ip, sizeof(ip)) == 1)//已PDP激活
			{	
				if(cidstatus_tmp == 1)
				{
					sysLOG(SOCKET_LOG_LEVEL_1, "<SUCC> fibo_PDPStatus iRet=%d, CID1\r\n", iRet);
					return 0;
				}
			}
			else
			{	
				sysLOG(SOCKET_LOG_LEVEL_3, "<WARN> fibo_PDPStatus iRet=%d, CID1\r\n", iRet);
				if(hal_sysGetTickms() > uTime)
				{
					sysLOG(SOCKET_LOG_LEVEL_2, "<ERR> 1reg_info.nStatus:%d\r\n", reg_info.nStatus);
					return ERR_4G_AT_NOREG_FAIL;
				}
			}
		}

		if(hal_sysGetTickms() > uTime)
		{
			sysLOG(SOCKET_LOG_LEVEL_3, "<ERR> 2reg_info.nStatus:%d\r\n", reg_info.nStatus);
			return ERR_4G_AT_CEREG_FAIL;
		}
		sysDelayMs(1000);
	}
}


/*
*Function:		hal_wiresockSetNoBlock
*Description:	设置非阻塞
*Input:			sockfd:socket id 句柄
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_wiresockSetNoBlock(int sockfd)
{  
    int flag;
	flag = fibo_sock_lwip_fcntl(sockfd, F_GETFL, 0); //不能直接覆盖原标识，先获取
    if (flag < 0)
	{  
		sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> fcntl F_GETFL fail, flag:%d\r\n", flag);
        return;  
    }  
    if (fibo_sock_lwip_fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0)
	{  
		sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> fcntl F_SETFL fail\r\n");
    }  
}


/*
*Function:		hal_wiresockSetBlock
*Description:	设置阻塞
*Input:			sockfd:socket id 句柄
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_wiresockSetBlock(int sockfd)
{   
	int flag;
	
	flag = fibo_sock_lwip_fcntl(sockfd, F_GETFL, 0); //不能直接覆盖原标识，先获取
    if (flag < 0)
	{  
		sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> fcntl F_GETFL fail, flag:%d\r\n", flag);
        return;  
    }  
    if (fibo_sock_lwip_fcntl(sockfd, F_SETFL, flag & ~O_NONBLOCK) < 0)
	{  
		sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> fcntl F_SETFL fail\r\n");
    }  
}


/*
*Function:		hal_wiresockSocketCreate
*Description:	创建一个socket
*Input:			nProtocol: 协议类型，0-TCP；1-UDP
*Output:		NULL
*Hardware:
*Return:		ERR_4G_AT_SOCKCREATE_FAIL: 失败; >=0: 成功，该值为创建的socket ID
*Others:
*/
int hal_wiresockSocketCreate(int nProtocol)
{
	int iRet = -1;
	int NODELAY = 1;

	iRet = fibo_sock_create(nProtocol);
	if(iRet < 0)
	{
	
		sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockSocketCreate iRet:%d\r\n", iRet);
		return ERR_4G_AT_SOCKCREATE_FAIL;
	}
	fibo_sock_setOpt(iRet,IPPROTO_TCP,TCP_NODELAY,&NODELAY,sizeof(NODELAY));
	//sysDelayMs(1000);
	hal_wiresockSetBlock(iRet);
	
	sysLOG(SOCKET_LOG_LEVEL_1, "<SUCC> hal_wiresockSocketCreate iRet:%d\r\n", iRet);

	return iRet;
}


/*
*Function:		hal_wiresockSocketClose
*Description:	关闭已经打开的socket
*Input:			sockid: socket ID
*Output:		NULL
*Hardware:
*Return:		0: 成功; ERR_4G_AT_SOCKCOLOSE_FAIL: 失败
*Others:
*/
int hal_wiresockSocketClose(int sockid)
{
	int iRet;
	uint32 queuetmp = 0;
	
	iRet = fibo_sock_close(sockid);
	if(iRet < 0)
	{
		return ERR_4G_AT_SOCKCOLOSE_FAIL;
	}
	queuetmp = 0xFF;
	sysLOG(SOCKET_LOG_LEVEL_1, "<SUCC> hal_wiresockSocketClose iRet:%d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_wiresockTcpConnect
*Description:	与远端socket建立连接
*Input:			sockid: socket ID; *pucIP:服务端IP/域名; uiPort:端口号
*Output:		NULL
*Hardware:
*Return:		0:成功; ERR_4G_TCP_CONNECT_FAIL:失败
*Others:
*/
int hal_wiresockTcpConnect(int sockid, char *pucIP, char *pucPort, int timeout)
{
	int iRet = -1;
	unsigned int uiPort = atoi(pucPort);
	GAPP_TCPIP_ADDR_T addr;
	ip_addr_t addr_t;
	

	addr.sin_port = htons(uiPort);//htons//lwip_htons

	fibo_getHostByName(pucIP, &addr_t, 1, 0);// sys00.jiewen.com.cn
	addr.sin_addr.u_addr.ip4.addr = addr_t.u_addr.ip4.addr;
	iRet = fibo_sock_connect(sockid, &addr);
	if(iRet < 0)
	{
		sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockTcpConnect iRet:%d\r\n", iRet);
		return ERR_4G_TCP_CONNECT_FAIL;
	}
	sysLOG(SOCKET_LOG_LEVEL_1, "<SUCC> hal_wiresockTcpConnect iRet:%d\r\n", iRet);
#if 0//创建非阻塞socket	,需查询socket是否可写，可写才能进行send
	unsigned long long uTime;
	uint32 uiTimeOut = 5000;

	uTime = hal_sysGetTickms() + uiTimeOut;
	while(1)
	{
		
		sysLOG(SOCKET_LOG_LEVEL_2, "While\r\n");

		FD_ZERO(&g_SocketReadfd);
		FD_ZERO(&g_SocketWritefd);
		FD_ZERO(&g_SocketErrfd);
		FD_SET(sockid, &g_SocketReadfd);
		FD_SET(sockid, &g_SocketWritefd);
		FD_SET(sockid, &g_SocketErrfd);
		g_stSocketTimeval.tv_sec  = 0;
		g_stSocketTimeval.tv_usec = 100;
		
		iRet = fibo_sock_lwip_select(sockid+1, NULL, NULL, &g_SocketErrfd, &g_stSocketTimeval); 
		
		if (iRet < 0)
		{		
			sysLOG(SOCKET_LOG_LEVEL_1, "select ret < 0, error\r\n");
			return iRet;
		}

		if(FD_ISSET(sockid, &g_SocketErrfd))	
		{
			int error = 0;
			int len = 4;
			
			sysLOG(SOCKET_LOG_LEVEL_1, "socketid = %d, error = %d\r\n", sockid, error);

			iRet = fibo_sock_getOpt(sockid, SOL_SOCKET, SO_ERROR,&error, &len);
			
			sysLOG(SOCKET_LOG_LEVEL_1, "fibo_sock_getOpt, error = %d\r\n", error);

			sysDelayMs(1000);
			iRet = fibo_sock_close(sockid);
			return -1;
		}
		else if(FD_ISSET(sockid, &g_SocketWritefd))
		{
			
			sysLOG(SOCKET_LOG_LEVEL_1, "FD_ISSET(sockid, &g_SocketWritefd)\r\n");
			break;
			
		}
		if(hal_sysGetTickms() > uTime)
		{
		
			sysLOG(SOCKET_LOG_LEVEL_1, "timeout!\r\n");
			break;
		}
		sysDelayMs(10);
	}
#endif	
	return iRet;

}


/*
*Function:		hal_wiresockSend
*Description:	TCP/UDP数据发送
*Input:			sockid: socket ID; *pucdata: 发送数据指针; uiLen: 发送数据长度
*Output:		NULL
*Hardware:
*Return:		>=0: 返回实际发送的数据长度; ERR_4G_SEND_DATA_FAIL: 失败
*Others:
*/
int hal_wiresockSend(int sockid, unsigned char *pucData, unsigned int uiLen)
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
			iRet = fibo_sock_send(sockid, pucData+(i*1024), 1024);
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
		iRet = fibo_sock_send(sockid, pucData+(packgenum1*1024), packgenum2);
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
	sysLOG(SOCKET_LOG_LEVEL_1, "hal_wiresockSend iRet:%d\r\n", iRet);
	
	return iRet;
}


/*
*Function:		hal_wiresockRecv
*Description:	接收TCP数据，一次读取最大不要超过2048个字节
*Input:			sockid: socket ID; iLen: 本次可接收的最大长度; uiTimeOut: 超时时间，单位ms 
*Output:		*pucdata: 接收数据指针;
*Hardware:
*Return:		>0:实际读取到的长度；<0:失败; ERR_TIMEOUT:超时
*Others:
*/
int hal_wiresockRecv(int sockid, unsigned char *pucBuff, unsigned int uiMaxLen, unsigned int uiTimeOut)
{
	int iRet = -1;
	uint32 recvdlen = 0;
	unsigned long long uTime;
		
	uTime = hal_sysGetTickms() + uiTimeOut;
	
	while(1)
	{
		
		sysLOG(SOCKET_LOG_LEVEL_4, "While\r\n");

		FD_ZERO(&g_SocketReadfd);
		FD_ZERO(&g_SocketWritefd);
		FD_ZERO(&g_SocketErrfd);
		FD_SET(sockid, &g_SocketReadfd);
		FD_SET(sockid, &g_SocketWritefd);
		FD_SET(sockid, &g_SocketErrfd);
		g_stSocketTimeval.tv_sec  = uiTimeOut/1000;
		g_stSocketTimeval.tv_usec = (uiTimeOut%1000)*1000;//单位微秒
		
		iRet = fibo_sock_lwip_select(sockid+1, &g_SocketReadfd, NULL, &g_SocketErrfd, &g_stSocketTimeval);	
		
		sysLOG(SOCKET_LOG_LEVEL_4, "fibo_sock_lwip_select,iRet:%d\r\n", iRet);
		if (iRet < 0)
		{		
			sysLOG(SOCKET_LOG_LEVEL_2, "select ret < 0, error\r\n");
			return ERR_4G_AT_CIPRXGET_2_FAIL;
		}
		
		if(FD_ISSET(sockid, &g_SocketErrfd))	
		{
			int error = 0;
			int len = 4;

			iRet = fibo_sock_getOpt(sockid, SOL_SOCKET, SO_ERROR,&error, &len);
			
			sysLOG(SOCKET_LOG_LEVEL_2, "fibo_sock_getOpt, error = %d\r\n", error);
			
			return ERR_4G_AT_SOCKCHECKERR_FAIL;
		}
		else if(FD_ISSET(sockid, &g_SocketReadfd))
		{
			
			sysLOG(SOCKET_LOG_LEVEL_4, "start hal_wiresockRecv\r\n");
			iRet = fibo_sock_recv(sockid, (unsigned char *)(pucBuff+recvdlen), uiMaxLen-recvdlen);
			sysLOG(SOCKET_LOG_LEVEL_4, "fibo_sock_recv iRet:%d, uiMaxLen-recvdlen:%d\r\n", iRet, uiMaxLen-recvdlen);

			if(iRet < 0)
			{
			
				sysLOG(SOCKET_LOG_LEVEL_2, "<ERR> hal_wiresockRecv iRet:%d\r\n", iRet);
				return ERR_4G_READ_DATA_FAIl;
			}
			else
			{
				
				recvdlen += iRet;
				if(recvdlen > 0)
				{
										
					sysLOG(SOCKET_LOG_LEVEL_1, "hal_wiresockRecv recvdlen:%d\r\n", recvdlen);
					return recvdlen;
				}
			}
			
			
		}
		if(hal_sysGetTickms() > uTime)
		{
		
//			sysLOG(SOCKET_LOG_LEVEL_3, "timeout. hal_wiresockRecv\r\n");
			return recvdlen;
		}
		sysDelayMs(20);
	}
	
}


/*
*Function:		hal_wiresockGetIMSIAndIMEI
*Description:	读取IMSI和IMEI接口
*Input:			NULL
*Output:		指针不能为空，*pucIMSI: IMSI接收指针; *pucIMEI: IMEI接收指针
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_wiresockGetIMSIAndIMEI(unsigned char *pucIMSI, unsigned char *pucIMEI)
{
	int iRet = -1;
	if(NULL != pucIMSI)
	{
		iRet = fibo_get_imsi(pucIMSI);
		if(iRet < 0)
		{
			return ERR_4G_AT_CGSN_FAIL;
		}
	}
	if(NULL!=pucIMEI)
	{
		iRet = fibo_get_imei(pucIMEI, 0);
		if(iRet < 0)
		{
			return ERR_4G_AT_CGSN_FAIL;
		}
	}
	return SOCKET_SSL_OK;
}


/*
*Function:		hal_wiresockGetCSQ
*Description:	获取模块CSQ信号强度等
*Input:			NULL
*Output:		*rssi:信号强度值，*ber:误码率
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_wiresockGetCSQ(int *rssi, int *ber)
{
	int iRet = -1;
	iRet = fibo_get_csq(rssi, ber);
	if(iRet < 0)
	{
		return ERR_4G_AT_CSQ_FAIL;
	}
	return SOCKET_SSL_OK;
}


/*
*Function:		hal_wiresockGetSingnal
*Description:	获取信号强度等级
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0:信号强度等级; 其他:失败
*Others:
*/
int hal_wiresockGetSingnal(void)
{
	int iRet = -1;
	int rssitmp, bertmp;
	iRet = hal_wiresockGetCSQ(&rssitmp, &bertmp);
	if(iRet < 0)
	{
		return ERR_4G_AT_CSQ_FAIL;
	}
	else if(iRet == 0)
	{
		iRet = Hysteresis(rssitmp, g_iWirelessCSQValue, 2, 2, 24, 1);
		iRet = Hysteresis(iRet, g_iWirelessCSQValue, 2, 2, 18, 1);
		iRet = Hysteresis(iRet, g_iWirelessCSQValue, 2, 2, 12, 1);
		iRet = Hysteresis(iRet, g_iWirelessCSQValue, 3, 3, 6, 1);
		g_iWirelessCSQValue = iRet;
		
		if(iRet > 24)
		{
			iRet = 5;
		}
		else if(iRet <= 24 && iRet > 18)
		{
			iRet = 4;
		}
		else if(iRet <= 18 && iRet > 12)
		{
			iRet = 3;
		}
		else if(iRet <= 12 && iRet > 6)
		{
			iRet = 2;
		}
		else if(iRet <= 6 && iRet > 0)
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}

	}
	
	return iRet;
}


/*
*Function:		hal_wiresockGetCCID
*Description:	获取模块ccid号
*Input:			NULL
*Output:		*ccid: ccid号指针
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_wiresockGetCCID(unsigned char *ccid)
{
	int iRet = -1;
	
	iRet = fibo_get_ccid(ccid);
	if(iRet < 0)
	{
		return ERR_4G_AT_CHECK_FAIL;
	}
	return SOCKET_SSL_OK;
}


/*
*Function:		hal_wiresockGetSimStatus
*Description:	获取SIM卡插拔状态
*Input:			NULL
*Output:		*status:状态指针，1-已插卡；0-未插卡
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_wiresockGetSimStatus(unsigned char *status)
{

	int iRet;
	unsigned long long uTime;
	uint8 i = 0;

	uTime = hal_sysGetTickms()+5000;

	while(1)
	{
		iRet = fibo_get_sim_status(status);
		if(iRet == 0 && *status == 1)
			break;
		i++;

		if(hal_sysGetTickms() > uTime)
			break;
		
		sysDelayMs(100);
	}

	sysLOG(SOCKET_LOG_LEVEL_2, "i=%d\r\n", i);

	if(iRet < 0)
	{
		return ERR_4G_AT_CPIN_FAIL;
	}
	return SOCKET_SSL_OK;
	
}


/*
*Function:		hal_wiresockGetRegInfo
*Description:	获取注册状态；0/2/3/4 表示未注册，其他值表示注册
*Input:			NULL
*Output:		*status:注册状态指针
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_wiresockGetRegInfo(unsigned char *status)
{
	int iRet = -1;
	reg_info_t reg_info;

	iRet = fibo_getRegInfo(&reg_info, 0);
	sysLOG(SOCKET_LOG_LEVEL_2, "iRet=%d, reg_info.nStatus=%d\r\n", iRet, reg_info.nStatus);
	if(iRet < 0)
	{
		return ERR_4G_AT_CGATT_FAIL;
	}
	*status = reg_info.nStatus;
	return SOCKET_SSL_OK;
}


/*
*Function:		hal_wiresockGetPdpStatus
*Description:	获取PDP状态，默认cid=CID1
*Input:			cid: 指定已激活的PDP profile ID
*Output:		*status:输出激活状态，0-未激活;1-已激活
*Hardware:
*Return:		0:成功，<0失败
*Others:
*/
int hal_wiresockGetPdpStatus(char cid, unsigned char *status)
{
	int iRet = -1;
	uint8 ip[64];

	*status = 0;

	sysLOG(SOCKET_LOG_LEVEL_2, "wirelessGetPdpStatus_lib\r\n");

	if(PdpOpenStatus != 0)
		return -2;
	
	if(rtcNTPflag == 0)
		return -2;
		
	memset(ip, 0, sizeof(ip));
	iRet = fibo_PDPStatus(cid, ip, status, 0);
	if(iRet == 0 && hal_wiresockBufIsWritten(ip, sizeof(ip)) == 1)
	{
		return SOCKET_SSL_OK;
	}
	else

	{
		return ERR_4G_AT_CGATT_FAIL;
	}
}


/*
*Function:		hal_wiresockPppClose
*Description:	PDP去激活
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_wiresockPppClose(void)
{
	int iRet = -1;
	iRet = fibo_PDPRelease(0, 0);
	if(iRet < 0)
	{
		return ERR_4G_AT_CIPSHUT_FAIL;
	}
	return SOCKET_SSL_OK;
}


/*
*Function:		hal_wiresockGetGSMorLTE
*Description:	获取注册的是4G还是2G网络
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		<0:失败；0:未读取到；1:4G网络；2:2G网络
*Others:
*/
int hal_wiresockGetGSMorLTE(void)
{
	int iRet = -1;
	reg_info_t reg_info;

	memset((uint8_t *)&reg_info, 0, sizeof(reg_info_t));

	iRet = fibo_getRegInfo(&reg_info, 0);
	if(iRet < 0)
	{
		return ERR_4G_AT_CGATT_FAIL;
	}
	if(reg_info.lte_scell_info.cell_id != 0)
	{
		iRet = 1;
	}
	else if(reg_info.gsm_scell_info.cell_id != 0)
	{
		iRet = 2;
	}
	else
	{
		iRet = 0;
	}
	return iRet;
}


/*
*Function:		hal_wiresockSetGSMorLTE
*Description:	设置注册的是4G还是2G网络,掉电不保存,切换网络后必须重新pdpOpen
*Input:			mode:0-Auto,LTE优先;1-LTE only;2-GSM only
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功
*Others:
*/
int hal_wiresockSetGSMorLTE(int mode)
{
	int iRet = -1;
	uint8 modetmp = 0;

#if 1//用接口实现，掉电不保存
	if(mode == 0)modetmp = mode;//Auto
	else if(mode == 1)modetmp = 4;//LTE
	else if(mode == 2)modetmp = 2;//GSM
	else return iRet;
	
	iRet = fibo_set_prior_RAT(modetmp, 0);
	sysLOG(SOCKET_LOG_LEVEL_1, " fibo_set_prior_RAT,modetmp:%d, iRet=%d\r\n", modetmp, iRet);
#endif

	return iRet;


}


/*
*Function:		hal_wiresockSetGSMorLTEex
*Description:	设置注册的是4G还是2G网络,掉电保存,切换网络后必须重新pdpOpen
*Input:			mode:0-Auto,LTE优先;1-LTE only;2-GSM only
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功
*Others:
*/
int hal_wiresockSetGSMorLTEex(int mode)
{
	int iRet = -1;
	uint8 modetmp = 0;


#if 1//用虚拟AT实现,掉电保存
	char sendtmp[256];
	char recvtmp[256];
	memset(sendtmp, 0, sizeof(sendtmp));
	memset(recvtmp, 0, sizeof(recvtmp));

	if(mode == 0)//Auto
	{
		sprintf(sendtmp, "AT+GTRAT=10,3,0\r\n");
	}
	else if(mode == 1)//LTE
	{
		sprintf(sendtmp, "AT+GTRAT=3\r\n");
	}
	else if(mode == 2)//GSM
	{
		sprintf(sendtmp, "AT+GTRAT=0\r\n");
	}
	else
	{
		return iRet;
	}


	iRet = hal_sysATTrans(sendtmp, strlen(sendtmp), recvtmp, 5000, "OK\r\n");
	
	sysLOG(BASE_LOG_LEVEL_1, "mode=%d, hal_sysATTrans iRet=%d:%s\r\n", mode, iRet, recvtmp);
#endif

	return iRet;


}


/*
*Function:		hal_wiresockGetCellInfo
*Description:	读取小区信息
*Input:			*cellinfoall_info:CELLINFOALL_STR指针，timeout：超时时间
*Output:		NULL
*Hardware:
*Return:		>=0:opencpu_cellinfo_t里面有数据,读到的小区个数，<0:失败
*Others:
*/
int hal_wiresockGetCellInfo(  CellInfoAll_Str *cellinfoall_info, uint32 timeout)
{
	int iRet = -1;
	int8 *rP_read = NULL;
	//iRet = fibo_getCellInfo(cellinfo, 0);
	int8 sendbufftmp[64];
	int8 *rP = NULL;
	int readp, writep;
	uint8 i;
	rP = malloc(2048);
	if(rP == NULL)
	{
		return ERR_4G_AT_CHECK_FAIL;
	}
	
    cellinfoall_info->cellinfonum = 0;
	readp = 0;
	writep = 0;
	memset(rP, 0, 2048);
	memset(sendbufftmp, 0, sizeof(sendbufftmp));
	sprintf(sendbufftmp, "AT+MCELL=0,26\r\n");
	sysLOG(SOCKET_LOG_LEVEL_4, " hal_sysATTrans, sendbufftmp=%s\r\n", sendbufftmp);
	iRet = hal_sysATTrans(sendbufftmp, strlen(sendbufftmp), rP, timeout, "OK");
	if(iRet >= 0)
	{
	    rP_read = rP;
		writep = iRet;
		for(i = 0;i<15;i++)
		{  
			
			rP_read = MyStrStr(rP_read, "MCC", readp, writep);
			if(rP_read == NULL)
			{
			    sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,MCC rP_read == NULL\r\n", i);
				break;
			}
			
			cellinfoall_info->cellinfo_str[i].MCC = GetNumFromAscii(rP_read, 4, ",", NULL, 0);
			sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,MCC=%d, rP_read=%s\r\n", i, cellinfoall_info->cellinfo_str[i].MCC, rP_read);
			
			//rP_read = NULL;
			rP_read = MyStrStr(rP_read, "MNC", readp, writep);
			if(rP_read == NULL)
			{
			    sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,MNC rP_read == NULL\r\n", i);
				break;
			}
			cellinfoall_info->cellinfo_str[i].MNC = GetNumFromAscii(rP_read, 4, ",", NULL, 0);

			//rP_read = NULL;
			rP_read = MyStrStr(rP_read, "LAC", readp, writep);
			if(rP_read == NULL)
			{
			    sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,LAC rP_read == NULL\r\n", i);
				break;
			}
			cellinfoall_info->cellinfo_str[i].LAC = GetNumFromAscii(rP_read, 4, ",", NULL, 3);

			//rP_read = NULL;
			rP_read = MyStrStr(rP_read, "CELL ID", readp, writep);
			if(rP_read == NULL)
			{
			    sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,CELL ID rP_read == NULL\r\n", i);
				break;
			}
			cellinfoall_info->cellinfo_str[i].CellID = GetNumFromAscii(rP_read, 8, ",", NULL, 3);

			//rP_read = NULL;
			rP_read = MyStrStr(rP_read, "RxDbm", readp, writep);
			if(rP_read == NULL)
			{
			    sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,RxDbm rP_read == NULL\r\n", i);
				break;
			}
			cellinfoall_info->cellinfo_str[i].RxDbm = GetNumFromAscii(rP_read, 6, ",", NULL, 0);
			sysLOG(SOCKET_LOG_LEVEL_4, "i=%d,RxDbm=%d, rP_read=%s\r\n", i, cellinfoall_info->cellinfo_str[i].RxDbm, rP_read);

		}
		cellinfoall_info->cellinfonum = i;
		
	}
	else
	{
		free(rP);
		return ERR_4G_AT_QENG_1_FAIL;
	}
	free(rP);
	return cellinfoall_info->cellinfonum;
}


/*
*Function:		hal_wiresockGetHostByName
*Description:	域名解析
*Input:			*Domainname：域名；*addr：解析后的ip
*Output:		NULL
*Hardware:
*Return:		0-succ;<0-failed
*Others:
*/
int hal_wiresockGetHostByName(char *Domainname, char *addr)
{
	int iRet = -1;
	//GAPP_TCPIP_ADDR_T gapp_addr;
	ip_addr_t addr_t;
	char *rP;

//	rP = addr;
	//gapp_addr.sin_port = htons(Domainname);

	iRet = fibo_getHostByName(Domainname, &addr_t, 1, 0);
	rP = inet_ntoa_self(addr_t.u_addr.ip4.addr);
	sysLOG(SOCKET_LOG_LEVEL_4, "ip4addr_ntoa, addr=0x%08x, rP=%s\r\n", addr_t.u_addr.ip4.addr, rP);
	memcpy(addr, rP, strlen(rP));
	return iRet;

}

/*
*Function:		hal_wiresockGetHostByName
*Description:	域名解析
*Input:			sig:sig;arg:入参
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_wiresockResCallback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
	{

		//fibo_PDPActive  ip address resopnse event
		case GAPP_SIG_PDP_ACTIVE_ADDRESS:
		{
		
			sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_PDP_ACTIVE_ADDRESS\r\n");
			UINT8 cid = (UINT8)va_arg(arg, int);
			char *addr = (char *)va_arg(arg, char *);
			if(addr != NULL)
			{
					
				sysLOG(SOCKET_LOG_LEVEL_1, "GAPP_SIG_PDP_ACTIVE_ADDRESS sig_res_callback	cid = %d, addr = %s\r\n", cid, addr);
					
					
			}
			else 
			{
			
				sysLOG(SOCKET_LOG_LEVEL_1, "GAPP_SIG_PDP_ACTIVE_ADDRESS\r\n");
			}
			va_end(arg);
		}
		break;

	    //fibo_getHostByName event
		case GAPP_SIG_DNS_QUERY_IP_ADDRESS:
		{   
		    char *host = (char *)va_arg(arg, char *);
	        char *ipv4_addr = (char *)va_arg(arg, char *);
			char *ipv6_addr = (char *)va_arg(arg, char *);

			if(host != NULL)
			{
			
				sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_DNS_QUERY_IP_ADDRESS sig_res_callback, host = %s\r\n", host);
			}
			if(ipv4_addr != NULL)
			{
				
				sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_DNS_QUERY_IP_ADDRESS sig_res_callback, ipv4_addr = %s\r\n", ipv4_addr);
				//strcpy(ip_test, ipv4_addr);

				
			}
			if(ipv6_addr != NULL)
			{
			
				sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_DNS_QUERY_IP_ADDRESS sig_res_callback, ipv6_addr = %s\r\n", ipv6_addr);
			}
			else
			{
			
				sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_DNS_QUERY_IP_ADDRESS sig_res_callback, ip_addr is NULL\r\n");
			}

			va_end(arg);
		}
		break;

		//fibo_PDPActive /fibo_asyn_PDPActive  pdp active status report
		case GAPP_SIG_PDP_ACTIVE_IND:
		{
			UINT8 cid = (UINT8)va_arg(arg, int);
			
			sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_PDP_ACTIVE_IND sig_res_callback  cid = %d\r\n", cid);
			va_end(arg);
				
			    
			sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_PDP_ACTIVE_IND\r\n");
			
			
		}
		break;

		//fibo_PDPRelease /fibo_asyn_PDPRelease pdp deactive status report
		case GAPP_SIG_PDP_RELEASE_IND:
		{
			UINT8 cid = (UINT8)va_arg(arg, int);
			
			sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_PDP_RELEASE_IND sig_res_callback  cid = %d\r\n", cid);
			va_end(arg);
			
				
			sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_PDP_RELEASE_IND\r\n");
			
		}
		break;

		case GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND:
		{
		    fibo_taskSleep(1000);
			
			sysLOG(SOCKET_LOG_LEVEL_4, "receive  GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND\r\n");
			UINT8 ip[50];	
			int ret;
			ret = hal_wiresockPppClose();	
			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockPppClose, ret=%d, \r\n", ret);
	
			ret = fibo_PDPActive(1, NULL, NULL, NULL, 0, 0, ip);
			
			sysLOG(SOCKET_LOG_LEVEL_4, "fibo_PDPActive ret = %d,ip=%s\r\n", ret, ip);
		}
		break;
		case GAPP_SIG_TTS_END:
	    	sysLOG(SOCKET_LOG_LEVEL_4, "GAPP_SIG_TTS_END\r\n");
			//hal_ttsAmpCtl(FALSE);

		break;
	    default:
	    {
	    	sysLOG(SOCKET_LOG_LEVEL_4, "sig=%d:default\r\n", sig);
	        break;
	    }
	}
	  
}



/*********************************TEST*********************************/

#if MAINTEST_FLAG

void wireless_test(void)
{
	int iRet = -1;
	int tmp1 = 0, tmp2 = 0;
	uint8 buftmp1[64], buftmp2[64];
	memset(buftmp1, 0, sizeof(buftmp1));
	memset(buftmp2, 0, sizeof(buftmp2));

	iRet = hal_wiresockGetIMSIAndIMEI(buftmp1, buftmp2);
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetIMSIAndIMEI, iRet=%d, buftmp1:%s, buftmp2:%s,\r\n", iRet, buftmp1, buftmp2);

	iRet = hal_wiresockGetCSQ(&tmp1, &tmp2);
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetCSQ, iRet=%d, tmp1:%d, tmp2:%d,\r\n", iRet, tmp1, tmp2);
	
	iRet = hal_wiresockGetSimStatus(&tmp1);
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetSimStatus, iRet=%d, tmp1:%d,\r\n", iRet, tmp1);

	iRet = hal_wiresockGetRegInfo(&tmp1);
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetRegInfo, iRet=%d, tmp1:%d,\r\n", iRet, tmp1);

	iRet = hal_wiresockGetPdpStatus(CID1, &tmp1);	
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetPdpStatus, iRet=%d, tmp1:%d,\r\n", iRet, tmp1);

	iRet = hal_wiresockPppClose();	
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockPppClose, iRet=%d, \r\n", iRet);
	
	sysDelayMs(5000);
	
	iRet = hal_wiresockGetPdpStatus(CID1, &tmp1);	
	sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetPdpStatus, iRet=%d, tmp1:%d,\r\n", iRet, tmp1);
	if(!(iRet ==0 && tmp1 == 1))
	{
		iRet = hal_wiresockPppOpen(NULL, NULL, NULL);
		while(1)
		{
			sysDelayMs(100);
			iRet = hal_wiresockGetPdpStatus(CID1, &tmp1);
			if(iRet == 0 && tmp1 == 1)
			{			
				sysLOG(SOCKET_LOG_LEVEL_2, " hal_wiresockGetPdpStatus, iRet=%d, tmp1:%d,\r\n", iRet, tmp1);
				break;
			}
		}
	}

}



static void hal_wiresockThreadTest(void *param)
{
	int iRet = -1;
	int sockid_tmp;

	uint8 simstatus = 0;

	//CblcdTest();

	usercfgtest();

	secscrtest();
	
//	FirmwareupdateTest();

	sysDelayMs(5000);
//	do{
//		
//		iRet = hal_wiresockGetSimStatus(&simstatus);
//		sysDelayMs(100);
//	}while(iRet != 0 || simstatus != 1);

//	hal_scrSetTurnDegree(180);
//	sysDelayMs(2000);
//	hal_scrSetTurnDegree(0);

	RTC_time rtctime;
	int32 timezonetmp = 0;

	iRet = sysGetTimezone_lib(&timezonetmp);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysGetTimezone_lib, iRet=%d, timezonetmp=%d\r\n", iRet, timezonetmp);
	sysDelayMs(100);
	
	iRet = sysGetRTC_lib(&rtctime);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysGetRTC_lib iRet=%d, %2d-%02d-%02d %02d:%02d:%02d", \
			iRet, rtctime.year, rtctime.month, rtctime.day, rtctime.hour, rtctime.min, rtctime.sec);
	sysDelayMs(100);
	
	iRet = sysSetTimezone_lib(-22);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysSetTimezone_lib, iRet=%d\r\n", iRet);
	sysDelayMs(100);

	iRet = sysGetTimezone_lib(&timezonetmp);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysGetTimezone_lib, iRet=%d, timezonetmp=%d\r\n", iRet, timezonetmp);
	sysDelayMs(100);
	
	iRet = sysGetRTC_lib(&rtctime);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysGetRTC_lib iRet=%d, %2d-%02d-%02d %02d:%02d:%02d", \
			iRet, rtctime.year, rtctime.month, rtctime.day, rtctime.hour, rtctime.min, rtctime.sec);
	sysDelayMs(100);

	iRet = sysSetTimezone_lib(32);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysSetTimezone_lib, iRet=%d\r\n", iRet);
	sysDelayMs(100);

	iRet = sysGetTimezone_lib(&timezonetmp);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysGetTimezone_lib, iRet=%d, timezonetmp=%d\r\n", iRet, timezonetmp);
	sysDelayMs(100);
	
	iRet = sysGetRTC_lib(&rtctime);
	sysLOG(SOCKET_LOG_LEVEL_1, "sysGetRTC_lib iRet=%d, %2d-%02d-%02d %02d:%02d:%02d", \
			iRet, rtctime.year, rtctime.month, rtctime.day, rtctime.hour, rtctime.min, rtctime.sec);
	sysDelayMs(100);

#if 0
	int32 socketfd;

	iRet = wifiOpen_lib();
	if(iRet == 0)
	{
		sysDelayMs(5000);
		while(1)
		{
			iRet = wifiGetLinkStatus_lib();
			if(iRet == 2 || iRet == 4)
			{
				sysLOG(SOCKET_LOG_LEVEL_1, "wifiGetLinkStatus_lib, iRet=%d\r\n", iRet);
				break;
			}
			sysDelayMs(2000);
		}
		socketfd = wifiSSLSocketCreate_lib();
		sysLOG(SOCKET_LOG_LEVEL_1, "wifiSSLSocketCreate_lib, socketfd=%d\r\n", socketfd);
		if(socketfd > 0)
		{
			iRet = wifiSSLConnect_lib(socketfd, "91tbicloud.com", "443", 20000);
			sysLOG(SOCKET_LOG_LEVEL_1, "wifiSSLConnect_lib, iRet=%d\r\n", iRet);

			iRet = wifiSSLSocketClose_lib(socketfd);
			sysLOG(SOCKET_LOG_LEVEL_1, "wifiSSLSocketClose_lib, iRet=%d\r\n", iRet);
		}
	}
#endif	
	while(1)
	{
SOCKETTEST:	
#if 1	
		/*******OPEN********/
		iRet = hal_wiresockPppOpen(NULL, NULL, NULL);
		if(iRet != 0)
		{
			
			sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockPppOpen, iRet=%d\r\n", iRet);
			goto end;
		}
		sysDelayMs(100);
#endif
	
		/*******CHECK********/ 
		iRet =  hal_wiresockCheckPppDial();
		if(iRet != 0)
		{
		
			sysLOG(SOCKET_LOG_LEVEL_1, "<ERR>  hal_wiresockCheckPppDial, iRet=%d\r\n", iRet);
			goto end;
		}
		sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC>  hal_wiresockCheckPppDial, iRet=%d\r\n", iRet);

		wirelessSetDNS_lib("8.8.8.8", "114.114.114.114");

#if 1
		/*******CREATE********/	
		sockid_tmp = hal_wiresockSocketCreate(0);
		if(sockid_tmp < 0)
		{		
			sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockSocketCreate, sockid_tmp=%d\r\n", sockid_tmp);
			goto end;
		}
		//socketnum_tmp += 1;
		sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> hal_wiresockSocketCreate, sockid_tmp=%d\r\n", sockid_tmp);
		sysDelayMs(100);

		/*******CONNECT********/
		iRet = hal_wiresockTcpConnect(sockid_tmp, "103.235.231.21", "3000", 1000);//103.235.231.21//sys00.jiewen.com.cn
		if(iRet < 0)
		{		
			sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockTcpConnect, iRet=%d\r\n", iRet);
			//goto end;
		}	
		sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> hal_wiresockTcpConnect, iRet=%d\r\n", iRet);
		sysDelayMs(100);

		/*******SEND********/
		uint8 sockdata_tmp[1024];
		uint32 recvdlen;
		memset(sockdata_tmp, 0, sizeof(sockdata_tmp));
		memcpy(sockdata_tmp, pssltmp, sizeof(pssltmp));
		iRet = hal_wiresockSend(sockid_tmp, sockdata_tmp, 1024);
		if(iRet != 1024)
		{		
			sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockSend, iRet=%d\r\n", iRet);
			goto end;
		}	
		sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> hal_wiresockSend, iRet=%d\r\n", iRet);
		sysDelayMs(100);

		/*******RECV********/
		//iRet = wirelessSocketLwipSelect(socketnum_tmp);

		memset(sockdata_tmp, 0, sizeof(sockdata_tmp));
		recvdlen = 0;
		do{
			
			iRet = hal_wiresockRecv(sockid_tmp, sockdata_tmp, 24, 200);
			if(iRet < 0)
			{		
				sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockRecv, iRet=%d\r\n", iRet);
				goto end;
			}
			recvdlen += iRet;
			sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> hal_wiresockRecv, iRet=%d\r\n", iRet);
			sysDelayMs(10);
		}while(recvdlen < 24);
		sysDelayMs(100);
		
		memset(sockdata_tmp, 0, sizeof(sockdata_tmp));
		recvdlen = 0;
		do{
			
			iRet = hal_wiresockRecv(sockid_tmp, sockdata_tmp, 1000, 200);
			if(iRet < 0)
			{		
				sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockRecv, iRet=%d\r\n", iRet);
				goto end;
			}	
			recvdlen += iRet;
			sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> hal_wiresockRecv, iRet=%d\r\n", iRet);
			sysDelayMs(10);
		}while(recvdlen < 1000);
		sysDelayMs(1000);

		/*******CLOSE********/
		iRet = hal_wiresockSocketClose(sockid_tmp);
		if(iRet != 0)
		{		
			sysLOG(SOCKET_LOG_LEVEL_1, "<ERR> hal_wiresockSocketClose, iRet=%d\r\n", iRet);
			goto end;
		}	
		sysLOG(SOCKET_LOG_LEVEL_2, "<SUCC> hal_wiresockSocketClose, iRet=%d\r\n", iRet);

		//wireless_test();
#endif
		//testBCD();
		hal_scrSetBackLightValue(99);
		iRet = hal_wiresockGetSingnal();
		sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetSingnal iRet=%d\r\n", iRet);
		iRet = wifiCheck_lib(NULL);
		sysLOG(SOCKET_LOG_LEVEL_2, "wifiCheck_lib iRet=%d\r\n", iRet);
		hal_wiresslTest();
		iRet = hal_sysSetSleepMode(1);
		sysLOG(SOCKET_LOG_LEVEL_2, "hal_sysSetSleepMode(1) iRet=%d\r\n", iRet);
		int8 DrInfo[1024];
		memset(DrInfo, 0, sizeof(DrInfo));
		iRet = hal_sysGetBPVersion(DrInfo);
		sysLOG(SOCKET_LOG_LEVEL_2, "hal_sysGetBPVersion iRet=%d:%s\r\n", iRet, DrInfo);
	end:
		hal_icionHysteresistest();
		//WifiTest_Connect();
		//server_connect_test();
		//WiFiSSL_ConnectTest();
		//hal_fileMkdir("/app/ufs/testdir/");
		filetest();
		//hal_fileRmdir("/app/ufs/testdir/");
		sysDelayMs(5000);
		ST_WIFI_PARAM st_wifi_param;
		iRet = wifiGetConnectParam_lib(&st_wifi_param);
		sysLOG(SOCKET_LOG_LEVEL_2, "wifiConfigConnectParam_lib,iRet=%d,cIp=%s,cGateWay=%s,cNetMask=%s,iDHCPEnable=%d\r\n", iRet,st_wifi_param.cIp, st_wifi_param.cGateWay, st_wifi_param.cNetMask, st_wifi_param.iDHCPEnable);
#if 1
		hal_scrSetBackLightValue(1);
		sysDelayMs(3000);
		hal_scrSetBackLightValue(50);
		sysDelayMs(3000);
		hal_scrSetBackLightValue(100);
		hal_scrSetBackLightMode(1, 1000);
//		sysDelayMs(2000);
//		hal_scrSetBackLightMode(0, 100);
		unsigned char DownCtrl[3]={'1','1',0x00};
//		iRet = SE_Sleep(DownCtrl);
//		sysLOG(SOCKET_LOG_LEVEL_2, "SE_Sleep, iRet=%d\r\n", iRet);
//		iRet = fibo_lcd_Sleep(1);
//		sysLOG(SOCKET_LOG_LEVEL_2, "fibo_lcd_Sleep, iRet=%d\r\n", iRet);
		wifiClose_lib();
		//fibo_gpio_set(VBAT3V3EN_GPIO, false);//关闭3V3电源
		sysDelayMs(1000);
		iRet = fibo_setSleepMode(1);
		sysLOG(SOCKET_LOG_LEVEL_2, "fibo_setSleepMode, iRet=%d\r\n", iRet);
		//sysDelayMs(10000);
		iRet = hal_wiresockGetGSMorLTE();		
		sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
		sysDelayMs(2000);

		uint8 i = 0;
		i = 0;
		sysLOG(SOCKET_LOG_LEVEL_2, "LTE or GSM Auto\r\n");
		hal_wiresockSetGSMorLTE(0);
		while(1)
		{
			i++;
			iRet = hal_wiresockGetGSMorLTE();		
			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
			if(i >10)break;
			sysDelayMs(1000);
			
		}
//		i = 0;
//		sysLOG(SOCKET_LOG_LEVEL_2, "GSM only\r\n");
//		hal_wiresockSetGSMorLTE(2);
//		while(1)
//		{
//			i++;
//			iRet = hal_wiresockGetGSMorLTE();		
//			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
//			if(i >10)break;
//			sysDelayMs(1000);
//			
//		}
//		i = 0;
//		sysLOG(SOCKET_LOG_LEVEL_2, "LTE only\r\n");
//		hal_wiresockSetGSMorLTE(1);
//		while(1)
//		{
//			i++;
//			iRet = hal_wiresockGetGSMorLTE();		
//			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
//			if(i >10)break;
//			sysDelayMs(1000);
//			
//		}
//		i = 0;
//		sysLOG(SOCKET_LOG_LEVEL_2, "GSM only\r\n");
//		hal_wiresockSetGSMorLTE(2);
//		while(1)
//		{
//			i++;
//			iRet = hal_wiresockGetGSMorLTE();		
//			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
//			if(i >10)break;
//			sysDelayMs(1000);
//			
//		}
//		i = 0;
//		sysLOG(SOCKET_LOG_LEVEL_2, "LTE or GSM Auto\r\n");
//		hal_wiresockSetGSMorLTE(0);
//		while(1)
//		{
//			i++;
//			iRet = hal_wiresockGetGSMorLTE();		
//			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
//			if(i >10)break;
//			sysDelayMs(1000);
//			
//		}
//		i = 0;
//		sysLOG(SOCKET_LOG_LEVEL_2, "GSM only\r\n");
//		hal_wiresockSetGSMorLTE(2);
//		while(1)
//		{
//			i++;
//			iRet = hal_wiresockGetGSMorLTE();		
//			sysLOG(SOCKET_LOG_LEVEL_2, "hal_wiresockGetGSMorLTE, iRet=%d\r\n", iRet);
//			if(i >10)break;
//			sysDelayMs(1000);
//			
//		}
//		sysDelayMs(5000);
//		goto SOCKETTEST;
	#endif	
		hal_portUSBTest();
		sysLOG(SOCKET_LOG_LEVEL_2, "fibo_thread_delete\r\n");
		fibo_thread_delete();
	}


}


void hal_wiresockSocketTest(void)
{
	fibo_thread_create(hal_wiresockThreadTest, "hal_wiresockThreadTest", 1024*32, NULL, OSI_PRIORITY_NORMAL);//1024*8*2
	
}

#endif



