#include "LC610N_api_sys.h"

extern unsigned char g_aucSeSleepPara[5];

/*
*@Brief:		蜂鸣器立即发出一声“嘀”，持续时间为100ms。注：该函数非阻塞。
*@Param IN: 	无 			
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
void sysBeep_lib(void)
{
	static unsigned char beepReq = 0;
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 10;
	unsigned char ucCmdHead[6] = {0x00, 0xa5, 0x01, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);

	memset(ucCmd,0,iCmdLen + 1);

	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	ucCmd[6]=0x64;
	ucCmd[7]=0x00;

	ucCmd[8]=0x60;
	ucCmd[9]=0x09;


#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_1, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,beepReq++,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = RET_RF_OK;
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
*@Brief:		蜂鸣器按参数指定的频率和持续时间发声
*@Param IN:		ucMode [输入] 发声频率,有效值：0~6 0 为最低频率 6 为最高频率 
				usDlyTime[输 入] 发声持续时间(单位：ms) 非阻塞 
*@Param OUT:	null 
*@Return:		0:成功; <0:失败
*/
const unsigned short gfreq[7]={2700,3000,3200,3400,3600,3800,4000};

int sysBeepF_lib(unsigned char ucMode, unsigned short usDlyTime)
{
	static unsigned char beepfReq = 0;
	unsigned short usfreq;
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 10;
	unsigned char ucCmdHead[6] = {0x00, 0xa5, 0x01, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);

	memset(ucCmd,0,iCmdLen + 1);

	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	ucCmd[6]=usDlyTime & 0xFF;
	ucCmd[7]=(usDlyTime >> 8)&0xFF;

	if(ucMode > 6)
		ucMode = 0;

	usfreq = gfreq[ucMode];
	
	ucCmd[8]=usfreq & 0xFF;
	ucCmd[9]=(usfreq >> 8)&0xFF;



#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_1, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,beepfReq++,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = RET_RF_OK;
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
函数名称：int GetWeekDay(int year,int month,int day)
函数功能: 返回输入日期对应的一周中的第几天。
函数参数：year 输入日期的年；month 输入日期的月；day输入日期的日
         如：2000年1月1日则是GetWeekDay(2000,1,1)
*/
int GetWeekDay(int year,int month,int day)
{
int i,j,count=0;
int Day_Index;
int days[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
int MonthAdd[12] = {0,31,59,90,120,151,181,212,243,273,304,334};
#if 0
    for(i = 1;i < month;i++)
       count = count + days[i];
#else
    count = MonthAdd[month-1];
#endif
    count = count + day;
    if((year%4 == 0&&year %100 != 0||year % 400 == 0)&& month >= 3)
        count += 1;
    count = count + (year - 1901) * 365;
    for(j = 1901;j < year;j++)
    {
        if(j % 4 == 0 && j % 100 != 0 || j % 400 == 0)
          count++;
    }
    return ((count+1) % 7);
}

#if 0
/*
*@Brief:		写入终端信息
*@Param IN:		uiIdMask[输入] Idmask 取值如下: 0xAAFF5500：写入扩展序列号 ExSN 0xFFAA0055：写入客制信息 CIF 不支持（0x0055FFAA ：写入序列号 SN） 
                Info [输入] 指向存放系统信息的指针
*@Param OUT:	
*@Return:		0:成功; <0:失败
*/
int sysWriteSn(unsigned int uiIdMask, unsigned char* pucInfo)
{
    if(uiIdMask != 0x0055FFAA)
    {
    	return ERR_PARA_INVALID;
    }
	int iRet = 0;
	int iCmdLen = 6 + 11;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x0f, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, pucInfo, 11);

	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d\r\n", iRet);
	if(iRet <0) {
		goto RET_END;
	}
	memset(caShow, 0, sizeof(caShow));
	HexToStr(retfrm.data, retfrm.length, caShow);
	sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
	iRet=retfrm.data[2]<<8 | retfrm.data[3];

	if(0x9000 == iRet) 
	{
		iRet = PED_RET_OK;
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
#endif

/*
*@Brief:		读取终端序列号
*@Param IN:		uiIdMask[输入] Idmask 取值如下: 0x0055FFAA ：读取序列号 0xAAFF5500：读取扩展序列号 0xFFAA0055：读取客制信息 
*@Param OUT:	pucInfo[输出] 用于存放产品序列号的缓冲区地址,需要预先分配 64 字节 的空间。 
*@Return:		N:长度; <0:失败
*/
int sysReadSn_lib(unsigned int uiIdMask, unsigned char* pucInfo)
{
	int8    Tem_buf[32] = {0};

    if(uiIdMask != 0x0055FFAA)
    {
    	return ERR_PARA_INVALID;
    }
	int iRet = 0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x10, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif	
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d\r\n", iRet);
	if(iRet <0) {
		goto RET_END;
	}
	//memset(caShow, 0, sizeof(caShow));
	//HexToStr(retfrm.data, retfrm.length, caShow);
	//sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
	iRet=retfrm.data[2]<<8 | retfrm.data[3];

	if(0x9000 == iRet) {
	    memcpy(Tem_buf, retfrm.data + 6, 4);
       
	   	if(strncmp(Tem_buf,"SN",2)==0)    //返回值前2位，是SN号的前缀
    	{
    		output_len = retfrm.data[5]<<8 | retfrm.data[4]-strlen("SN");
	
		    memcpy(pucInfo, retfrm.data + 6+2 , output_len);
		   //iRet = PED_RET_DATA_LEN_ERR;
		   iRet = output_len;
    	}
        else if(strncmp(Tem_buf,"TUSN",4)==0)     //返回值前4位，是TUSN号的前缀,要去除"TUSN"4字符ascii码前缀
    	{

		    output_len = retfrm.data[5]<<8 | retfrm.data[4]-strlen("TUSN")-8;
	        
		    memcpy(pucInfo, retfrm.data + 6 +12, output_len);

		   iRet = output_len;
		  
    	}
         else
		{
			output_len = retfrm.data[5]<<8 | retfrm.data[4];
			memcpy(pucInfo, retfrm.data + 6 , output_len);
			iRet = output_len;
		}
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
*@Brief:		读取终端TUSN序列号
*@Param IN:		uiIdMask[输入] Idmask 取值如下: 0x0055FFAA ：读取序列号 0xAAFF5500：读取扩展序列号 0xFFAA0055：读取客制信息 
*@Param OUT:	pucInfo[输出] 用于存放产品序列号的缓冲区地址,需要预先分配 64 字节 的空间。 
*@Return:		N:长度; <0:失败
*/

int sysReadTUSN_lib(unsigned int uiIdMask, unsigned char* pucInfo)
{
	int8    Tem_buf[32] = {0};

    if(uiIdMask != 0x0055FFAA)
    {
    	return ERR_PARA_INVALID;
    }
	int iRet = 0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x10, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif	
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d\r\n", iRet);
	if(iRet <0) {
		goto RET_END;
	}

	iRet=retfrm.data[2]<<8 | retfrm.data[3];

	if(0x9000 == iRet) 
	{
	    memcpy(Tem_buf, retfrm.data + 6 , 4);
	   	if(strncmp(Tem_buf,"SN",2)==0)    //返回值前2位，是SN号的前缀,报错
    	{
		   iRet = PED_RET_DATA_LEN_ERR;
    	}
        else if(strncmp(Tem_buf,"TUSN",4)==0)         //返回值前4位，是TUSN号的前缀,要去除"TUSN"4字符ascii码前缀
    	{
			output_len = retfrm.data[5]<<8 | retfrm.data[4]-strlen("TUSN");
			memcpy(pucInfo, retfrm.data + 6 +4, output_len);
			iRet = output_len;
    	}
		else
		{
			iRet = -2;
		}

	}
	else
	{
		iRet = -2;
	}

	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}


/*
*@Brief:		写入终端TUSN序列号
*@Param IN:		data[输入]:写入的tusn、sn数据; len 待写入数据的长度 最大64 字节
*@Param OUT:	
*@Return:		N:长度; <0:失败
*/

int sysWriteTUSN_lib(  char* data,int len)
{
	int8    Tem_buf[32] = {0};

	int iRet = 0;
	int iCmdLen = len + 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x25, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	if(len > 64)
	{
		fibo_free(ucCmd);
		iRet = -1;
		goto RET_END;
	}
	memcpy(ucCmd+sizeof(ucCmdHead),data,len);
	
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif	
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d\r\n", iRet);
	if(iRet <0) {
		goto RET_END;
	}

	iRet=retfrm.data[2]<<8 | retfrm.data[3];

	if(0x9000 == iRet) 
	{
	    iRet = len;

	}
	else
	{
		iRet = -2;
	}

	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		读取终端的版本信息
*@Param IN:		uiId[输入] 0   boot 版本 1  vos 版本 2  硬件配置版本 3  tms 版本 4  Lib 版本 5  HVN 和 FVN 版本 
*@Param OUT:	pucInfo[输出] 用于存放产品序列号的缓冲区地址,需要预先分配 64 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysReadVerInfo_lib(unsigned int uiId, unsigned char* pucVerInfo)
{
	if((uiId < 0) || (uiId > 5))
	{
		return ERR_PARA_INVALID;
	}
	int iRet = 0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucType = 0;
	switch(uiId)
	{
		case 0:
             ucType = 0x12;
			 break;
		case 1:
			 ucType = 0x0e;
			 break;
		case 2:
             ucType = 0x11;
			 break;
		case 3:
			 return ERR_NOSUCHDATA;//ucType = 0x15;//
			 break;
		case 4:
             //ucType = 0x13;
             memcpy(pucVerInfo, DR_VER, strlen(DR_VER));
			 return 0;
			 break;
		case 5:
			 //ucType = 0x14;//
			 memcpy(pucVerInfo, "1.00", 4);
			 return 0;
			 break;

		default: 
			return ERR_PARA_INVALID;
			break;
	}

	unsigned char ucCmdHead[6] = {0x00, 0xe2, ucType, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	//sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	//sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d\r\n", iRet);
	if(iRet <0) {
		goto RET_END;
	}
	//memset(caShow, 0, sizeof(caShow));
	//HexToStr(retfrm.data, retfrm.length, caShow);
	//sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		memcpy(pucVerInfo, retfrm.data + 6 , output_len);
		iRet = PED_RET_OK;
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

void sysGetRandom_lib (unsigned int lenth, unsigned char *pucRandom)
{
	sysLOG(API_LOG_LEVEL_2, " into\r\n");
	int iRet = 0;
	int iCmdLen = 6 + 2;
	unsigned char ucCmdHead[6] = {0x00, 0xe5, 0x08, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &lenth, 2);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
    sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	//sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d\r\n", iRet);
	if(iRet <0) {
		goto RET_END;
	}
	//memset(caShow, 0, sizeof(caShow));
	//HexToStr(retfrm.data, retfrm.length, caShow);
	//sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
	iRet=retfrm.data[2]<<8 | retfrm.data[3];

	if(0x9000 == iRet) {
		memcpy(pucRandom, retfrm.data + 6 , lenth);
		iRet = PED_RET_OK;
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	//return iRet;
}
/*
*@Brief:		读取终端配置信息,信息缓冲区应不少于30个字节
*@Param IN:		无
*@Param OUT:	out_info [输出]	
				out_info[0]	是否支持国密
				out_info[1]	是否支持蜂鸣器
				out_info[2]	是否支持闪烁灯
				out_info[3]	是否有触屏
				out_info[4]	是否支持磁条卡
				out_info[5]	是否支持接触IC卡
				out_info[6]	是否支持非接卡
				out_info[7]	是否支持蓝牙
				out_info[8]	是否支持断码屏
				out_info[9]	PSAM卡配置信息
				out_info[10]	LAN(TCP/IP)模块配置信息
				out_info[11]	GPS模块配置信息
				out_info[12]	4G/2G模块配置信息
				out_info[13]	WI-FI模块配置信息
				out_info[14]	是否支持显示屏
				out_info[15]	是否支持打印机（1-热敏、2-针打、0-无）
				out_info[16]-[29]	保留
*@Return:		>=0:返回终端信息有效字节长度; <0:失败
*/
int sysGetTermInfo_lib (unsigned char *out_info)
{
	int iRet = 0;
	unsigned char temp_info[30] = {0};
#if 0 //先返回0
    memcpy(out_info, temp_info, sizeof(temp_info));
    return sizeof(temp_info);
#else
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x31, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(output_len * 2 + 1);
		HexToStr(retfrm.data + 6 , output_len, caShow);
		sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
		fibo_free(caShow);
#endif
		memcpy(temp_info, retfrm.data + 6 , output_len);
	    temp_info[10] = 0;
		temp_info[11] = 1;
		temp_info[12] = 1;
		temp_info[13] = 1;
		temp_info[14] = 1;
		temp_info[15] = 0;
		memcpy(out_info, temp_info, 16);
#ifdef PRINT_API_CMD
		caShow = (char*) fibo_malloc(16 * 2 + 1);
		HexToStr(temp_info, 16, caShow);
		sysLOG(API_LOG_LEVEL_2, "  temp_info = %s\r\n", caShow);
		fibo_free(caShow);
#endif
		iRet = 16;
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
#endif
}
/*
*@Brief:		从SE读取终端机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型，如：Q360,需要预先分配 20 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysGetTermType_se(char *out_type)
{
	int iRet = 0;
	int i =0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x16, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		if(output_len > 20)
			output_len = 20;
		for(i= 0; i<20; i++)
		{
			if((retfrm.data[6+i]) == '-')
				break;
		}
		memcpy(out_type, retfrm.data + 6, i);
		iRet = PED_RET_OK;
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d, out_type=%s\r\n", iRet, out_type);
	return iRet;
}

/*
*@Brief:		读取终端机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型，如：Q360,需要预先分配 20 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysGetTermType_lib(char *out_type)
{

	return sysGetTermType_se(out_type);
}

/*
*@Brief:		读取国密版本号
*@Param IN:		null 
*@Param OUT:	pcGmVersion[输出] 国密版本号。
*@Return:		>:成功,版本号字节; <0:失败
*/
int sysReadGmVersion_lib(char *pcGmVersion)
{
	int iRet = 0;
	int i =0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x33, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		memcpy(pcGmVersion, retfrm.data + 6, output_len);
		iRet = output_len;
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, pcGmVersion=%s\r\n", iRet, pcGmVersion);
	return iRet;
}

/*
*@Brief:		获取设备温度
*@Param IN:		null 
*@Param OUT:	pscTemp[输出] 温度,1字节  -20℃ < pscTemp < 80℃
*@Return:		=0:成功; <0:失败
*/
int sysGetBattChargTemp_lib(signed char *pscTemp)
{
	int iRet = 0;
	int i =0;
	
	int iCmdLen = 6;
	char cFlag = 0;
	unsigned char ucStatus = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x34, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	if(is_se_sleepping == 1)                                       //SE休眠状态下，is_se_sleepping会被下面程序置0
	{
		cFlag = 1;
	}
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);

	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		pscTemp[0] = (signed char)retfrm.data[6];

		if((pscTemp[0] >= (-20)) && (pscTemp[0] <= 80))
		{
			iRet = PED_RET_OK;
		}
		else{
			iRet = ERR_GET_BATT_TEMP;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, pscTemp=%d\r\n",iRet, pscTemp[0]);
	if(cFlag == 1)
	{
		SE_Sleep(g_aucSeSleepPara);
	}
	return iRet;
}

/*
*@Brief:		读取按键适配的内部机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型，如：Q161,需要预先分配 20 字节 的空间。 
*@Return:		0:成功; <0:失败
*/
int sysGetKeyboardType_lib(char *out_type)
{
	int iRet = 0;

	iRet = hal_sysReadTermType();
	switch(iRet){

		case 0://Q161
		
			strcpy(out_type, "Q161");
		break;
	}

	return 0;

}


/*
*@Brief:		读取终端机型
*@Param IN:		null 
*@Param OUT:	out_type[输出] 返回终端机型完整信息。 
*@Return:		0:成功; <0:失败
*/
int sysGetTermTypeImperInfo_lib(char *out_type)
{
	int iRet = 0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x16, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		if(output_len > 20)
			output_len = 20;
		memcpy(out_type, retfrm.data + 6 , output_len);
		iRet = PED_RET_OK;
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d, out_type=%s\r\n", iRet, out_type);
	return iRet;
}

void apiSysTest()
{
    int iRet = 0;
	unsigned char baRandom[100] = {0};
	sysGetRandom_lib(16, baRandom);
	char* caShow = (char*) fibo_malloc(100 * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(baRandom, 16, caShow);
	sysLOG(API_LOG_LEVEL_2, " sysGetRandom_lib baRandom = %s\r\n", caShow);

	unsigned char out_info[30] = {0};
	sysGetTermInfo_lib (out_info);
	char out_type[20] = {0};
	sysGetTermType_se(out_type);

	memset(out_type, 0, sizeof(out_type));
	iRet = sysGetTermType_lib(out_type);
	sysLOG(API_LOG_LEVEL_2, " sysGetTermType_lib iRet=%d, out_type:%s\r\n", iRet, out_type);

	memset(out_type, 0, sizeof(out_type));
	iRet = sysGetKeyboardType_lib(out_type);
	sysLOG(API_LOG_LEVEL_2, " sysGetKeyboardType_lib iRet=%d, out_type:%s\r\n", iRet, out_type);

	memset(out_type, 0, sizeof(out_type));
	iRet = sysGetTermTypeImperInfo_lib(out_type);
	sysLOG(API_LOG_LEVEL_2, " sysGetTermTypeImperInfo_lib iRet=%d, out_type:%s\r\n", iRet, out_type);
	
	
	fibo_free(caShow);
#if 0
	unsigned char ucTime[100] = {0};
	iRet = hal_sysGetTime(ucTime);
	sysLOG(API_LOG_LEVEL_2, " hal_sysGetTime iRet=%d, ucTime = %s\r\n", iRet, ucTime);

	unsigned int uiIdMask = 0x0055FFAA;
	unsigned char pucInfo[200] = {0};

    memset(pucInfo, 0, sizeof(pucInfo));
	iRet = sysReadSn_lib(uiIdMask, pucInfo);
	sysLOG(API_LOG_LEVEL_2, " sysReadSn_lib iRet=%d, pucInfo = %s\r\n", iRet, pucInfo);

	memset(pucInfo, 0, sizeof(pucInfo));
	iRet = sysReadVerInfo_lib(1, pucInfo);
	sysLOG(API_LOG_LEVEL_2, " sysReadVerInfo_lib iRet=%d, pucInfo = %s\r\n", iRet, pucInfo);
#endif
}

int sysGetHeapInfo_lib(uint32_t * size,uint32_t * avail_size,uint32_t * max_block_size)
{
	return fibo_get_heapinfo(size,avail_size,max_block_size);
}

