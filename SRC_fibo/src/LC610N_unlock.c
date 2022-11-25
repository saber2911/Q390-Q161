/*
* Copyright (c) 2017, Beijing Vanstone Technology Co.,Ltd.
* All rights reserved.
*
* File Name:hal_unlock.c
* 
* Description:解锁方式
*/
#include "comm.h"



#define CHALLANGE_CODE_LENTH            8

#define UNLOCKPAD_INPUTOK   0
#define UNLOCKPAD_INPUTERR  -1
#define UNLOCKPAD_TIMEOUT   -2
#define UNLOCKPAD_CANCEL    -3
#define UNLOCKPAD_FN    -4


#define PWD_FOR_CLIENT 			"47821596"

#define EASY_PWD_MASS_PRODUCT   "00000000"

#define CHALLANGE_CODE_LENTH            8
#define SN_LEN	11

unsigned char gucDesKey[16]={0x64, 0x30, 0x94, 0xA8, 0x49, 0x71, 0x0D, 0x56, 0xCD, 0xF7, 0x13, 0xDD, 0xFC, 0x61, 0x2E, 0x44};
int getChallengeCodeAndUnlockPwd(unsigned char *ucStr,unsigned char *ucResult)
{
	unsigned char ucDataIn[16],ucDataOut[16];
	unsigned char i;
	int iRet = -1;
    
/**1.get rand num, take this as chanllenge code**/
	memset(ucStr,0,CHALLANGE_CODE_LENTH);
    
	sysGetRandom_lib(CHALLANGE_CODE_LENTH, ucStr);
	
	for(i = 0; i < CHALLANGE_CODE_LENTH; i++)
	{
		 ucStr[i] %=10;
		 ucStr[i] +='0' ;
	}
	ucStr[CHALLANGE_CODE_LENTH] = '\0';
	
/**2.get the unlock password;**/
	memset(ucDataIn,0xFF,sizeof(ucDataIn));
	memcpy(ucDataIn,ucStr,CHALLANGE_CODE_LENTH);//take the ascii code

/**正式使用时需要打开**/    
	//hal_iTdes(ucDataIn,ucDataOut,sizeof(ucDataIn),gucDesKey,16,PED_ENCRYPT);
	iRet = calcTdesEnc_lib(ucDataIn, sizeof(ucDataIn), ucDataOut, gucDesKey,16, NULL, 0);
    sysLOG(LCD_LOG_LEVEL_2, "calcTdesEnc_lib, ucStr= %s, iRet=%d\r\n", ucStr, iRet);
	for(i=0;i<CHALLANGE_CODE_LENTH;i++)
	{
		ucDataOut[i] ^=ucDataOut[i+8] ;
		ucDataOut[i] %=10;
		ucDataOut[i] +='0' ;
	}
	memcpy(ucResult,ucDataOut,CHALLANGE_CODE_LENTH);
	ucResult[CHALLANGE_CODE_LENTH] = '\0';
	sysLOG(LCD_LOG_LEVEL_2, "calcTdesEnc_lib, ucResult= %s\r\n", ucResult);
	return CHALLANGE_CODE_LENTH;

}

/**
 * [Function]       hal_unlockPassWord
 * [Description]    密码解锁
 * [return]
 * [modify]         [author]  [version]  2017-10-25
 */
int hal_unlockPassWord(void)
{
    unsigned char aucChanllengeCode[9];
    unsigned char aucUnlockPwd[9];
    unsigned char aucPassWord[9];
	unsigned char aucSN[SN_LEN];
	unsigned char ucXINHAO[16] = {0};
	unsigned char ucPwd[16] = {0};
	unsigned char ucCmd = 0xFF;
	char strSN[64];
	char SNFlag = 0;
    int ucPwdLen = 0;
    int iRet = 0;
	unsigned char inputRow = 0x00;
    hal_keypadFlush();
	memset(aucChanllengeCode,0x00,sizeof(aucChanllengeCode));
	memset(aucUnlockPwd,0x00,sizeof(aucUnlockPwd));
    getChallengeCodeAndUnlockPwd(aucChanllengeCode, aucUnlockPwd);
	memset(strSN, 0, sizeof(strSN));
	sysReadSn_lib(0x0055FFAA, (unsigned char *)strSN);
	if(strSN[0])
	{
		SNFlag = 1;
	}
    hal_scrCls();
	hal_scrPrintEx(0x00, 1, 0x02, "挑战码", "Challenge code");
	hal_scrPrint(0x00, 2, 0x02, "%s", aucChanllengeCode);
	hal_scrPrintEx(0x00, 3, 0x02, "请输入密码", "Pls Input Password");
	inputRow = 0x04;
    sysLOG(LCD_LOG_LEVEL_2, " aucPassWord=%s\r\n", aucPassWord);
	memset(aucPassWord,0x00,sizeof(aucPassWord));
    while(1)
    {

		if(hal_keypadHit() == 0)
        {
            ucCmd = hal_keypadGetKey();
			sysLOG(LCD_LOG_LEVEL_2, "hal_keypadGetKey ucCmd=0x%x\r\n", ucCmd);

            switch(ucCmd)
            {
				case KEYENTER:
					if(ucPwdLen == 8)
					{
					    memcpy(aucPassWord, ucPwd, 8); 
						if(SNFlag == 1)
						{
							if((memcmp(aucPassWord, PWD_FOR_CLIENT, 8) == 0) || (memcmp(aucPassWord, aucUnlockPwd, 8) == 0) )
							{
								hal_scrCls();
								hal_scrPrintEx(0x00, 1, 0x02, "验证成功", "PASSWORD OK");
								sysDelayMs(2000);
								return UNLOCKPAD_INPUTOK;
							}
							else
							{
								hal_scrCls();
								hal_scrPrintEx(0x00, 1, 0x02, "密码错误", "PASSWORD ERROR");
								sysDelayMs(2000);	
								return UNLOCKPAD_INPUTERR;
							}
						}
						else{	
							if((memcmp(aucPassWord, EASY_PWD_MASS_PRODUCT, 8) == 0) || (memcmp(aucPassWord, aucUnlockPwd, 8) == 0) )
							{
								hal_scrCls();
								hal_scrPrintEx(0x00, 1, 0x02, "验证成功", "PASSWORD OK");
								sysDelayMs(2000);
								return UNLOCKPAD_INPUTOK;
							}
							else
							{
								hal_scrCls();
								hal_scrPrintEx(0x00, 1, 0x02, "密码错误", "PASSWORD ERROR");
								sysDelayMs(2000);	
								return UNLOCKPAD_INPUTERR;
							}
						}
						
					}
	   
				break;
				case KEYCANCEL:
					return UNLOCKPAD_CANCEL;
				break;
				case KEY_FN:
					return UNLOCKPAD_FN;
				break;
				
				case KEYCLEAR:
					if(ucPwdLen > 0)
					{
						ucPwdLen--;
						ucXINHAO[ucPwdLen] = 0x00;
						hal_scrClrLine(5,7);
						hal_scrPrint(1,5,2,"%s",ucXINHAO);
					}
				break;
				case KEY0:
				case KEY1:		
				case KEY2:			
				case KEY3:			
				case KEY4:			
				case KEY5:			
				case KEY6:			
				case KEY7:			
				case KEY8:			
				case KEY9:
					if(ucPwdLen < 8)
					{
						ucXINHAO[ucPwdLen] = '*';
						ucPwd[ucPwdLen++] = ucCmd;
						hal_scrClrLine(5,7);
						hal_scrPrint(1,5,2,"%s",ucXINHAO);
					}
				break;
				default:
				break;

			}
        }
		sysDelayMs(20);
		//sysLOG(LCD_LOG_LEVEL_2, "unlockPassWord sysDelayMs\r\n");

    }
    
    return 0;
}


#define V_NEED_SIZE             (1+4+1+1+1+64)          // 0x02 + len(4) + alg(1) + cmd(1) + data(n) + algdata(n) + 0x03
#define ONLINE_COMPPORT         P_USB                       //通讯端口

/**授权协议**/ 
#define V_A_REQURE				0x5A 					//90
#define V_A_AUTHOR				0x5B					//91

/**校验方式**/ 
#define V_ALG_NONE			    0x00
#define V_ALG_CRC32			    0x01
#define V_ALG_CRC8			    0x02

#define V_STA				    0x02		// 起始位
#define V_END				    0x03		// 结束位

#define VERSION_AUTHOR	        0

#define U32ToU8Arr(pU8Arr, v) ((pU8Arr)[0] = (unsigned long)(v)  >> 24, \
								(pU8Arr)[1] = (unsigned long)(v) >> 16,   \
								(pU8Arr)[2] = (unsigned long)(v) >> 8,    \
								(pU8Arr)[3] = (unsigned char)(unsigned long)(v) )
#define U8ArrToU32(pU) ((unsigned long)(((pU)[0] << 24) + ((pU)[1] << 16) + ((pU)[2] << 8) + (pU)[3]))

int g_iOnlinePort = P_USB;    
int g_HandType;                         // 握手类型, 0-下载 1-放窃机授权
int g_result = 0;	                   // 授权结果
unsigned char g_ucCmdId = 0;
unsigned char g_ucRandKey[8] = {0};
const unsigned char g_auc3DesBuf[15][16] = 
{	 
"\x6a\xFE\xED\x07\x9A\xE4\xD3\xCC\xB0\x3C\xB1\xBD\xA8\x25\xF5\x3D",
"\x87\xDA\x1D\x3F\x48\x18\x2A\x2B\x89\x8F\xCD\xF8\x47\x96\x84\xD6",
"\x62\xFD\x5E\xC5\x16\x1A\x3E\x85\xA6\x09\x87\x6E\x0E\x43\x46\x48",
"\x36\x4B\x70\xD0\xE0\xD6\x20\xB6\x15\xFC\x9F\xaE\xFC\xA4\x74\x94",
"\xE1\x4A\xB3\xa8\xB9\x3E\xFD\x07\x8D\x0B\x5a\x1B\xF7\x7F\x59\x60",
"\x19\x48\x13\x96\xa7\x00\xCE\x89\x2A\xF3\xF3\x4a\x11\xAE\xA4\x17",
"\x04\x9A\xAB\xBF\x8F\x72\xa1\xB0\x00\xBF\x54\xFa\xD6\xF3\x0D\xA9",
"\xF1\x43\x71\x99\x1a\xF7\xA1\xC4\x33\x14\x79\xa4\x61\x90\x91\xEE",
"\x57\x9E\xCD\xD5\x8a\x70\x0F\x0C\xFF\x7A\x7a\xC9\xE8\x83\xDE\x8D",
"\x0A\x99\x71\xC0\xDF\x01\x3a\x3C\x82\x72\xC7\x3a\x14\x6A\x23\xA0",
"\xF2\xB5\xBC\xEa\xE7\x81\xDB\x6B\xEa\x68\x9F\x2A\xB7\x85\xFA\x35",
"\x49\x42\x28\x2a\x7B\x68\xEE\x14\xEB\x44\x0D\x3a\x53\x19\xAB\xC5",
"\x33\x8F\x02\x62\x65\xCB\xa5\xE0\x90\x7A\x86\x9a\xA0\xFF\x2F\xDA",
"\x12\x97\x52\xCA\x29\x13\xa8\x76\x24\xB6\xA4\x4E\xCa\x1F\x10\xD2",
"\x8A\xF2\x79\x71\x47\xFD\xBa\x3D\x20\x9E\x6A\x7F\xFa\xC2\xF7\x7E"
};

const unsigned long g_aucOnlineUpdateCRC32[256] = 
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/*************************************************************************************
作    者: 卢建兴
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: V协议获取校验数据长度
入口参数: alg:校验类型
出口参数: 无
返 回 值: 校验数据长度
备    注: 无
*************************************************************************************/
int V_GetAlgDataLen(unsigned char alg)
{
	if(alg == V_ALG_CRC32)
    {
        return 4;
    }
	else if(alg == V_ALG_CRC8)
    {
        return 1;
    }
	
	return 0;
}

static unsigned char dat_low4bit(unsigned char inchar) 
{
	return (inchar & 0x0f);
}

static long dat_asclen(unsigned char *ptr,long len)
{
	long rlen = 0;
	for(rlen = 0;rlen<len;rlen++)
	{
		if((ptr[rlen] > '9') || (ptr[rlen] < '0') ) //不是数字
		{
			break;
		}
	}
	return rlen;
}

unsigned long hal_AscToLong_Api(unsigned char *sAsc, unsigned char ucLen)
{
    unsigned long result;
    unsigned char ii;

    if(sAsc == NULL)
    {
        return 0;
    }
    result = 0;

    ucLen = (unsigned char)(dat_asclen(sAsc, ucLen));

    for(ii = 0; ii < ucLen; ii++) 
    {
        result = result*10 + dat_low4bit(sAsc[ii]);
    }
    return result;
}

/***************************************************************
作    者: 姜子平
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: 文件数据CRC校验，由FileCrc32()函数调用
入口参数: crc校验结果，szSrc需计算的内容，dwSrcLen需校验的长度    
出口参数: 无
返 回 值: Crc32结果
备    注: 无
****************************************************************/
unsigned long V_CalcCrc32(unsigned long crc, unsigned char* szSrc, unsigned long dwSrcLen)
{
	int len = dwSrcLen;
	unsigned char* buf = (unsigned char *)szSrc;	
	while (len--)
	{
        crc = (crc >> 8) ^ g_aucOnlineUpdateCRC32[(crc & 0xFF) ^ *buf++];
    }
    return crc;
}

unsigned char CalcCrc08(unsigned char *pStr, unsigned long lLength)
{
	unsigned char bValue=0;
	unsigned long i;

	for(i=0; i<lLength; i++)
		bValue ^= pStr[i];
	
	return bValue;
}

/*************************************************************************************
作    者: 卢建兴
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: V协议获取校验数据
入口参数: alg:校验类型  data:要校验的数据  dataLen:数据长度
出口参数: dataOut:校验结果
返 回 值: 0:成功  其他失败
备    注: 无
*************************************************************************************/
int V_CalAlgFun(unsigned char alg, unsigned char *data, unsigned int dataLen, unsigned char *dataOut)
{
	unsigned long crc = 0xffffffff;
	
	if(alg == V_ALG_NONE)
    {
        return 0;
    }
	else if(alg == V_ALG_CRC32)
	{
		crc = V_CalcCrc32(crc, data, dataLen);
		U32ToU8Arr(dataOut, crc);
		return 0;
	}
	else if(alg == V_ALG_CRC8)
	{
		crc = 0;
		crc = CalcCrc08(data, dataLen);
		*dataOut = crc;
		return 0;
	}
	
	return -1;
}


/*************************************************************************************
作    者: 卢建兴
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: V协议组包数据包
入口参数: alg:校验算法 order:命令码 dataInOut:要发送的数据内容 dataLenInOut:数据长度
出口参数: dataInOut:组包后的数据包内容  dataLenInOut:组包后的数据包长度
返 回 值: 0:数据包正确 其他:数据包有问题
备    注: // data 是输入输出参数, 开辟内存一定要 sendlen + NEED_SIZE (最少)
*************************************************************************************/
int V_PackData(unsigned char alg, unsigned char order, unsigned char *dataInOut, unsigned int *dataLenInOut)
{
	int pos = 0, Ret, datalen;
	
	if(dataInOut == NULL || dataLenInOut == NULL)
    {
        return 1;
    }
    
	memmove(dataInOut+7, dataInOut, *dataLenInOut);

	dataInOut[pos] = V_STA;
	pos += 5;
	dataInOut[pos++] = alg;   // 校验算法
	dataInOut[pos++] = order;  // 命令码
	pos += *dataLenInOut;

	datalen = pos - 5;  
	datalen += V_GetAlgDataLen(alg);
	U32ToU8Arr(dataInOut+1, (unsigned long)datalen);
    
	// 校验
	Ret = V_CalAlgFun(alg, dataInOut+1, pos-1, dataInOut+pos);
	if(Ret != 0)
		return 2;
	pos += V_GetAlgDataLen(alg);

	dataInOut[pos++] = V_END;

	*dataLenInOut = pos;
	return 0;
}

// 授权请求
// 结果+索引+8密钥+命令码+sn+mac
int Cmd_Requar(unsigned char* dataIn, unsigned int dataLen) 
{
	unsigned char ucRand = 0x00;
	unsigned char ucKeyIndex = 0x00;
    unsigned char aucSendBuff[256];
    unsigned char aucTempBuff[256];
    unsigned char aucSnBuff[16];
    unsigned char aucOutBuff[8];
    int iRet = 0;
    unsigned int pos = 0;
    int posTemp = 0;
    int iLen = 0;
    int i = 0, j = 0;
    
    memset(aucSendBuff, 0, sizeof(aucSendBuff));
	memset(aucSnBuff, 0, sizeof(aucSnBuff));
    memset(aucOutBuff, 0, sizeof(aucOutBuff));
    
	g_ucCmdId = dataIn[0];				        // cmd
    hal_scrCls();
    switch(g_ucCmdId)
	{
	case 1:
        hal_scrPrintEx(0x00, 1, 0x02, "刷机请求", "Root Request");
		break;
	case 2:
        hal_scrPrintEx(0x00, 1, 0x02, "初始化公钥系统请求", "Init PukSys Request");
		break;
	default:
        hal_scrPrintEx(0x00, 1, 0x02, "未知请求", "Unknow Request");
		iRet = 1;
		break;
	}
    
	pos = 2;                                    // 第一个字节是请求结果  0-成功 其他-失败  ;  第二个字节是版本号
	if(iRet == 0)
	{	
		sysGetRandom_lib(1, &ucRand);
		ucKeyIndex = ucRand % 15; 
		aucSendBuff[pos++] = ucKeyIndex;

/**8个字节的随机密钥**/		
		for(i = 0; i < 8; i++)
		{	
			sysGetRandom_lib(1, &ucRand);
			aucSendBuff[pos++] = ucRand & 0xff;
//			V_HexToAscii(aucSendBuff[pos-1], tempkey + i*2);
		}
//        hal_getRand(8, aucSendBuff+pos);
//        pos += 8;
		
		memcpy(g_ucRandKey, aucSendBuff+3, 8);
		
		memset(aucTempBuff, 0, sizeof(aucTempBuff));
        posTemp = 0;
/**1,命令码**/        
		aucTempBuff[posTemp++] = g_ucCmdId;
        
/**2,sn号**/ 
		iRet = sysReadSn_lib(0x0055FFAA, aucSnBuff);        //hal_readSnSimp
		if(iRet<0)
		{
			strcpy((char *)aucSnBuff, "00000000000");
			iRet = 11;
		}
		
		sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:  onlinedecode sn is:%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, aucSnBuff);
		//strcpy(aucSnBuff,"00023388734");

		sysLOG_lib(API_LOG_LEVEL_2, "[%s] -%s- Line=%d:  force make onlinedecode sn is:%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, aucSnBuff);
		
		iLen = iRet;//AscToLong_Api(SnBuf, 2);

		aucTempBuff[posTemp++] = iLen;
		memcpy(aucTempBuff+posTemp, aucSnBuff, iLen);
		posTemp += iLen;
        
/**3,算mac**/ 
		posTemp = (posTemp+7)/8*8;
		for(j = 0 ; j < posTemp / 8 ; j++)
		{
			for(i = 0 ; i < 8 ; i++)
			{
				aucTempBuff[i+posTemp] = aucTempBuff[i+posTemp] ^ aucTempBuff[j*8 + i];
			}
		}
        
		//hal_pedDes(aucTempBuff+posTemp, aucOutBuff, g_ucRandKey, 1);
		DesECB(FALSE, g_ucRandKey, aucTempBuff+posTemp, 8, aucOutBuff);
		memcpy(aucTempBuff+posTemp, aucOutBuff, 8);
		posTemp += 8;

/**4,拷贝 1个字节SN长度+SN+补0（使前面数据为8的倍数）+ 8个字节MAC**/
		memcpy(aucSendBuff+pos, aucTempBuff, posTemp);
		pos += posTemp;
		iRet = 0;
		// 从第3个字节开始 全报文加密
		for(i = 0; i < (pos-3)/8; i++)
		{
			//hal_iTdes(aucSendBuff+3+i*8, aucOutBuff, 8, (unsigned char *)g_auc3DesBuf[ucKeyIndex],16,1);
			calcTdesEnc_lib(aucSendBuff+3+i*8, 8, aucOutBuff, (unsigned char *)g_auc3DesBuf[ucKeyIndex],16, NULL, 0);
			memcpy(aucSendBuff+3+i*8,  aucOutBuff, 8);
		}
	}
    
	aucSendBuff[0] = iRet;
	aucSendBuff[1] = VERSION_AUTHOR;   // 版本号

	if(V_PackData(V_ALG_CRC8, V_A_REQURE, aucSendBuff, &pos))
	{
		return 1;
	}
    
	hal_portSends(g_iOnlinePort, aucSendBuff, pos);
    
	return 0;
}


/**授权回复**/
/**版本号+命令码+SN+1字节结果**/
int Cmd_Author(unsigned char* dataIn, unsigned int dataLen)
{
	unsigned int pos = 0, posTemp = 0;
	unsigned int len;
	int iRet = 0, i;
	unsigned char SendBuf[512], SnBuf[40], TempBuf[512];
    
	memset(SendBuf, 0, sizeof(SendBuf));
	memset(SnBuf, 0, sizeof(SnBuf));
	
	if(dataIn[0] != VERSION_AUTHOR)
	{
		iRet = 1;
		pos = 1;
        hal_scrCls();        
        hal_scrPrintEx(0x00, 1, 0x02, "版本错误", "Version Error");   
		goto OUTR;
	}
    
	if(g_ucCmdId != dataIn[1])				// cmd
	{
		iRet = 1;
		pos = 1;
        hal_scrCls();
        hal_scrPrintEx(0x00, 1, 0x02, "请求码错误", "Requre Code Error");
		goto OUTR;
	}

	if(dataLen <= 2)
	{
		return -1;
	}
	
	dataLen -= 1;                           // 有效包的长度

	memset(TempBuf, 0, sizeof(TempBuf));
//    #ifndef _FAST_TDES_
	for(i=0; i<dataLen/8; i++)
	{
		//hal_pedDes(dataIn+2+i*8, TempBuf+i*8, g_ucRandKey, 0); // 解密
		DesECB(TRUE, g_ucRandKey, dataIn+2+i*8, 8, TempBuf+i*8);
	}

	iRet = sysReadSn_lib(0x0055FFAA, SnBuf);
	if(iRet<0)
	{
		strcpy((char *)SnBuf, "00000000000");
		iRet = 11;
	}
		
	len = iRet;//AscToLong_Api(SnBuf, 2);


	if((len != TempBuf[0]) || (memcmp(SnBuf, TempBuf+1, len))) //SN号不匹配
	{
        hal_scrCls();
        hal_scrPrintEx(0x00, 1, 0x02, "非法授权", "Illegal author");
		iRet = 1;
		pos = 1;
		goto OUTR;
	}
    
	if(TempBuf[len+1] != 0)
	{
        hal_scrCls();
        hal_scrPrintEx(0x00, 1, 0x02, "授权失败", "author failed");
		iRet = 1;
		pos = 1;
		goto OUTR;
	}
	
	pos = 1;
	iRet = 0;
	g_result = 1;
    hal_scrCls();
    hal_scrPrintEx(0x00, 1, 0x02, "授权成功", "author success");
    
OUTR:
	SendBuf[0] = iRet;

	if(V_PackData(V_ALG_CRC8, V_A_AUTHOR, SendBuf, &pos))
	{
        return 1;
    }

	hal_portSends(g_iOnlinePort, SendBuf, pos);

	sysDelayMs(1000);
    
	return iRet;
}


/*************************************************************************************
作    者: 卢建兴
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: V协议处理命令码
入口参数: order:命令码   Data:接收到的数据包   Len:数据包长度
出口参数: 无
返 回 值: 0:处理结束
备    注: 无
*************************************************************************************/
int V_RunCmd(unsigned char order, unsigned char *Data, unsigned long Len)
{
	int iRet = -1;
	
	switch(order)
	{
		case V_A_REQURE:                /**授权请求**/
			Cmd_Requar(Data, Len);
			break;
		case V_A_AUTHOR:                /**授权执行**/
			iRet = Cmd_Author(Data, Len);
			break;
        default:
            break;
	}
	
	return iRet;
}


/*************************************************************************************
作    者: 卢建兴
版    权: 艾体威尔电子技术(北京)有限公司
函数功能: V协议握手
入口参数: 无
出口参数: 无
返 回 值: >0:握手成功  -1:用户取消  -2:超时
备    注: 无
*************************************************************************************/
int V_HandShake(void)
{
    char step = 0;
    unsigned char ucRecvData = 0x00;
    unsigned char ucSendData = 0x00;
	int iRet = 0;

	g_HandType = 0;

	hal_keypadFlush();
    hal_scrCls();
    hal_scrPrintEx(0x00, 1, 0x02, "等待握手...", "Waiting HandShake...");

	while(1)
	{
		if(hal_keypadHit() == 0)
		{
			if(hal_keypadGetKey() == KEYCANCEL)
            {
                return -1;
            }
		}

        if(0 == hal_portOpen(P_USB, NULL))
        {
             g_iOnlinePort = P_USB;

             iRet = 1;
        }
        else
        {
            iRet = 0;
        }

		iRet = hal_portRecvs(P_USB, &ucRecvData, 1 , 100);	
		if(iRet > 0)
		{
            hal_scrPrint(0x00, 4, 0x02, "->%c", ucRecvData); 
			if((ucRecvData == 'Q') || (ucRecvData == 'Z') || ((ucRecvData == 0x44) && (step == 0))) 
			{
				if(ucRecvData == 'Z')
				{
					if(g_result == 1) // 授权成功了只允许下载
                    {
                        continue;
                    }
					g_HandType = 1;
				}
				else if(ucRecvData == 'Q')
                {
                    g_HandType = 0;
                }
				
				//hal_userTimerSet(TIMER_EVENT_TAMPER,1000);
				
				if(step == 0)
				{              
					if(ucRecvData == 'Q')
                    {
                        ucSendData = 'R';
                    }
					else if(ucRecvData == 'Z')
                    {
                        ucSendData = 'X';
                    }
                     
					hal_portSends(g_iOnlinePort, &ucSendData, 1);
                    hal_scrPrint(0x00, 3, 0x02, "<-%c", ucSendData); 
					step = 1;
                    
					continue;
				}
                
				if(ucRecvData == 'Q')
                {
                    ucSendData = 'R';
                }
				else if(ucRecvData == 'Z')
                {
                    ucSendData = 'X';
                }
                
				hal_portSends(g_iOnlinePort, &ucSendData, 1);
                hal_scrPrint(0x00, 3, 0x02, "<-%c", ucSendData);
				continue;
			}
			
			if(step == 1)
			{
				//hal_userTimerSet(TIMER_EVENT_TAMPER,1000);
				if(ucRecvData == 'P')
                {
                    return 0;
                }
				else if(ucRecvData == 'D')
                {
                    return 1;
                }
				else if(ucRecvData == 'C')
                {
                    return 2;
                }
				else if(ucRecvData == 'T')
				{
					ucSendData = 0;
					hal_portSends(g_iOnlinePort, &ucSendData, 1);
                    hal_scrPrint(0x00, 3, 0x02, "<-%c", ucSendData);
				}
				else{}
                    
				step = 0;
                    
				continue;
			}
		}
#if 0
		if(hal_userTimerCheck(TIMER_EVENT_TAMPER) == 1)
		{
            api_scrCls();
            api_sCLcdPrint(0x00, 1, 0x02, "等待握手超时", "Waiting HandShake Timeout");
            delay_ms(1000);            
			return -2;
		}
#endif
	}
}

int DelOrderFun(void)
{
    unsigned char aucBuf[512+V_NEED_SIZE+64];
	unsigned short usRecvLen = 0;
	int iRet = 0;
	
	usRecvLen = 256;
	iRet = V_HandShake();
    if(iRet < 0)
	{
        hal_scrCls();
		
        hal_scrPrintEx(0x00, 1, 0x02, "握手失败", "Shake failed !");  
		
		sysDelayMs(2000);
		
		return -1;
	}

    hal_scrCls();
    hal_scrPrintEx(0x00, 1, 0x02, "握手成功", "Shake Success");       
    hal_keypadFlush();
	while(1)
	{
		iRet = hal_portRecvs(g_iOnlinePort, aucBuf, sizeof(aucBuf), 500);
		if(iRet > 0)
		{
            usRecvLen = iRet;
			iRet = V_RunCmd(aucBuf[1+4+1], aucBuf+7, usRecvLen-2-4-V_GetAlgDataLen(aucBuf[1+4])-1-1);
			if(iRet == 0)
            {
                break;
            }
		}
        
		if(hal_keypadHit() == 0)
		{ 
			if(hal_keypadGetKey() == KEYCANCEL)
            {
                return -1;
            }
		}
	}
    
	return iRet;
}

/**
 * [Function]       hal_unlockOnline
 * [Description]    联机解锁
 * [return]
 * [modify]         [author]  [version]  2017-10-25
 */
int hal_unlockOnline(void)
{
	return DelOrderFun(); 
}




