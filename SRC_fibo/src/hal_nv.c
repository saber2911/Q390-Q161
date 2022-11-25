/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_nv.c
** Description:		NV区相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"


/*
*Function:		hal_nvRead
*Description:	读取NV区数据
*Input:			len-读取数据长度
*Output:		*data-读取数据指针;
*Hardware:
*Return:		<0-读取失败; >=0-读取到的长度
*Others:
*/
int16 hal_nvRead(int8 *data, int8 len)
{	
	int16 iRet = -1;
	int8 *rP = NULL;

	rP = malloc(128);
	if(rP == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc(128) failed!\r\n");
		return iRet;
	}
	
	memset(rP, 0, 128);
	iRet = fibo_set_get_deviceNUM(GET_DEVICE_NUM, rP, len);
	if(iRet < 0)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> fibo_get_device_num get data failed! iRet = %d\r\n", iRet);
		free(rP);
		return iRet;
	}
	memcpy(data, rP, len);
	iRet = len;
	free(rP);
	
	return iRet;
	
}


/*
*Function:		hal_nvWrite
*Description:	写入NV区数据
*Input:			*data-写入数据指针; len-写入数据长度
*Output:		NULL
*Hardware:
*Return:		>=0-写入数据长度; -1-写入失败; -2-数据长度大于NV区空间
*Others:
*/
int16 hal_nvWrite(int8 *data, int8 len)
{
	int16 iRet = -1;
	int8 *rP = NULL;

	if(len > NV_LEN)
	{	
		sysLOG(NV_LOG_LEVEL_3, "<ERR> len > NV_LEN, failed!\r\n");
		return -2;
	}
	rP = malloc(128);
	if(rP == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc(128) failed!\r\n");
		return iRet;
	}

	memset(rP, 0, 128);
	iRet = hal_nvRead(rP, NV_LEN);
	if(iRet != NV_LEN)
	{
		sysLOG(NV_LOG_LEVEL_4, "<ERR> hal_nvRead failed!, iRet = %d\r\n", iRet);	
		
	}

	memcpy(rP, data, len);
	iRet = fibo_set_get_deviceNUM(SET_DEVICE_NUM, rP, NV_LEN);
	if(iRet < 0)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> fibo_set_get_deviceNUM set data failed! iRet = %d\r\n", iRet);
		free(rP);
		return iRet;
	}
	free(rP);
	return len;
	

	
}

/*
*NV区：0-63为SN存储空间；64-71为硬件版本号存储空间,
*/

/*
*Function:		hal_nvCrcCheck
*Description:	CRC校验
*Input:			*data-数据指针; datalen-数据长度
*Output:		NULL
*Hardware:
*Return:		CRC结果
*Others:
*/
static uint8 hal_nvCrcCheck(int8 *data, int8 datalen)
{
	uint8 i;
	uint8 crc_d = 0;
	for(i = 0; i < datalen; i++)
	{
		crc_d ^= *(data + i);
	}
	return crc_d;
}


/*
*Function:		hal_nvReadHwVersionString
*Description:	读硬件版本接口，字符串输出
*Input:			NULL
*Output:		*hw_data:硬件版本内容，字符串类型
*Hardware:
*Return:		<0:失败；>=0:成功，读到的字节数
*Others:
*/
int32 hal_nvReadHwVersionString(int8 *hw_data)
{
	int32 fd;
	int32 iRet, Ret;
	uint8 filesizeofHwVerString = 0;
	int8 fileofHwVerString[64];
	int8 HwVerStringName[32];

	memset(HwVerStringName, 0, sizeof(HwVerStringName));
	sprintf(HwVerStringName, "/app/ufs/HwVerString.txt");

	iRet = hal_fileGetFileSize(HwVerStringName);
	if(iRet <= 0)//获取文件大小失败或者文件大小为0
	{
		return iRet;
	}
	filesizeofHwVerString = iRet;
	/*打开文件*/
	fd = hal_fileOpen(HwVerStringName, O_RDONLY);
	if(fd < 0)
	{
		sysLOG(NV_LOG_LEVEL_4, "<ERR> hal_fileOpen fd=%d\r\n", fd);
		return fd;
	}
	
	iRet = hal_fileSeek(fd, 0, SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(NV_LOG_LEVEL_4, "<ERR> Seek File Fail %d\r\n", iRet);
		return iRet;
	}

	memset(fileofHwVerString,0,sizeof(fileofHwVerString));
	iRet = hal_fileRead(fd, fileofHwVerString,filesizeofHwVerString);
	if(iRet<0)
	{		
		sysLOG(NV_LOG_LEVEL_4, "<ERR> Read File Fail %d\r\n", iRet);
		return iRet;
	}
	else if(iRet==0)
	{
		Ret = hal_fileClose(fd);
		if(Ret != 0)
		{			
			sysLOG(NV_LOG_LEVEL_4, "<ERR> Close File Fail %d\r\n", Ret);
			return Ret;
		}
		
		sysLOG(NV_LOG_LEVEL_4, "<ERR> /app/ufs/HwVerString.txt read NULL\r\n");
		return iRet;
	}
	else
	{
		Ret = hal_fileClose(fd);
		if(Ret != 0)
		{
		
			sysLOG(NV_LOG_LEVEL_4, "<ERR> Close File Fail %d\r\n", Ret);
			return Ret;
		}

		memcpy(hw_data, fileofHwVerString, filesizeofHwVerString);
		
		sysLOG(NV_LOG_LEVEL_3, "<SUCC> /app/ufs/HwVerString.txt read OK! filesizeofHwVerString:%d\r\n", filesizeofHwVerString);
		return filesizeofHwVerString;
	}



}


/*
*Function:		hal_nvWriteHwVersionString
*Description:	写硬件版本号接口，字符串形式
*Input:			*hw_data:硬件版本号内容
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功写入的长度
*Others:
*/
int32 hal_nvWriteHwVersionString(int8 *hw_data)
{
	int32 fd;
	int32 iRet, Ret;
	int8 HwVerStringName[32];

	memset(HwVerStringName, 0, sizeof(HwVerStringName));
	sprintf(HwVerStringName, "/app/ufs/HwVerString.txt");

	iRet = hal_fileExist(HwVerStringName);
	if(iRet == 1)//文件存在
	{
		iRet = hal_fileRemove(HwVerStringName);
		if(iRet < 0)
		{
			sysLOG(NV_LOG_LEVEL_2, "<ERR> hal_fileRemove iRet=%d\r\n", iRet);
			return iRet;
		}
	}

	/*创建文件*/
	fd = hal_fileOpen(HwVerStringName, O_CREAT | O_RDWR);
	if(fd < 0)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> hal_fileOpen fd=%d\r\n", fd);
		return fd;
	}
	
	iRet = hal_fileSeek(fd, 0, SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(NV_LOG_LEVEL_2, "<ERR> Seek File Fail %d\r\n", iRet);
		return iRet;
	}
	
	iRet = hal_fileWrite(fd, hw_data, strlen(hw_data));
	
	Ret = hal_fileClose(fd);
	if(Ret != 0)
	{
	
		sysLOG(NV_LOG_LEVEL_2, "<ERR> Close File Fail %d\r\n", Ret);
		return Ret;
	}
	
	sysLOG(NV_LOG_LEVEL_2, "/app/ufs/HwVerString.txt end! iRet:%d\r\n", iRet);
	return iRet;
	



}



/*
*Function:		hal_nvReadLanguageString
*Description:	获取系统语言接口，字符串输出
*Input:			NULL
*Output:		*lg_data:系统语言内容，字符串类型
*Hardware:
*Return:		<0:失败；>=0:成功，读到的字节数
*Others:
*/
int32 hal_nvReadLanguageString(int8 *lg_data)
{
	int32 fd;
	int32 iRet, Ret;
	uint8 filesizelg = 0;
	int8 fileoflg[64];
	int8 sysLanguageName[32];

	memset(sysLanguageName, 0, sizeof(sysLanguageName));
	sprintf(sysLanguageName, "/app/ufs/sysLanguage.txt");

	iRet = hal_fileGetFileSize(sysLanguageName);
	if(iRet <= 0)//获取文件大小失败或者文件大小为0
	{
		return iRet;
	}
	filesizelg = iRet;
	/*打开文件*/
	fd = hal_fileOpen(sysLanguageName, O_RDONLY);
	if(fd < 0)
	{
		sysLOG(NV_LOG_LEVEL_4, "<ERR> hal_fileOpen fd=%d\r\n", fd);
		return fd;
	}
	
	iRet = hal_fileSeek(fd, 0, SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(NV_LOG_LEVEL_4, "<ERR> Seek File Fail %d\r\n", iRet);
		return iRet;
	}

	memset(fileoflg,0,sizeof(fileoflg));
	iRet = hal_fileRead(fd, fileoflg,filesizelg);
	if(iRet<0)
	{		
		sysLOG(NV_LOG_LEVEL_4, "<ERR> Read File Fail %d\r\n", iRet);
		return iRet;
	}
	else if(iRet==0)
	{
		Ret = hal_fileClose(fd);
		if(Ret != 0)
		{			
			sysLOG(NV_LOG_LEVEL_4, "<ERR> Close File Fail %d\r\n", Ret);
			return Ret;
		}
		
		sysLOG(NV_LOG_LEVEL_4, "<ERR> /app/ufs/HwVerString.txt read NULL\r\n");
		return iRet;
	}
	else
	{
		Ret = hal_fileClose(fd);
		if(Ret != 0)
		{
		
			sysLOG(NV_LOG_LEVEL_4, "<ERR> Close File Fail %d\r\n", Ret);
			return Ret;
		}

		memcpy(lg_data, fileoflg, filesizelg);
		
		sysLOG(NV_LOG_LEVEL_3, "<SUCC> /app/ufs/sysLanguage.txt read OK! filesizelg:%d\r\n", filesizelg);
		return filesizelg;
	}



}


/*
*Function:		hal_nvWriteLanguageString
*Description:	写系统语言接口，字符串形式
*Input:			*lg_data:系统语言内容
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功写入的长度
*Others:
*/
int32 hal_nvWriteLanguageString(int8 *lg_data)
{
	int32 fd;
	int32 iRet, Ret;
	int8 lgName[32];

	memset(lgName, 0, sizeof(lgName));
	sprintf(lgName, "/app/ufs/sysLanguage.txt");

	iRet = hal_fileExist(lgName);
	if(iRet == 1)//文件存在
	{
		iRet = hal_fileRemove(lgName);
		if(iRet < 0)
		{
			sysLOG(NV_LOG_LEVEL_4, "<ERR> hal_fileRemove iRet=%d\r\n", iRet);
			return iRet;
		}
	}

	/*创建文件*/
	fd = hal_fileOpen(lgName, O_CREAT | O_RDWR);
	if(fd < 0)
	{
		sysLOG(NV_LOG_LEVEL_4, "<ERR> hal_fileOpen fd=%d\r\n", fd);
		return fd;
	}
	
	iRet = hal_fileSeek(fd, 0, SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(NV_LOG_LEVEL_4, "<ERR> Seek File Fail %d\r\n", iRet);
		return iRet;
	}
	
	iRet = hal_fileWrite(fd, lg_data, strlen(lg_data));
	
	Ret = hal_fileClose(fd);
	if(Ret != 0)
	{
	
		sysLOG(NV_LOG_LEVEL_4, "<ERR> Close File Fail %d\r\n", Ret);
		return Ret;
	}
	
	sysLOG(NV_LOG_LEVEL_4, "/app/ufs/sysLanguage.txt end! iRet:%d\r\n", iRet);
	return iRet;
	
}


/*
*Function:		hal_nvReadSN
*Description:	读取SN号
*Input:			NULL
*Output:		*sn_data-读取sn存储指针
*Hardware:
*Return:		>0-sn长度；<=0-读取失败
*Others:
*/
int16 hal_nvReadSN(int8 *sn_data)
{
	int16 ret = -1;
    int16 result = 0;
    int16 len = 0;
	int8 *rP = NULL;
	rP = malloc(NV_SN_LEN+1);
	if(rP == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN+1);
	result = hal_nvRead(rP, NV_SN_LEN);
	if(result <= 0)
	{
		free(rP);
		return result;
	}
	
	len = *(rP+1);
	sysLOG(NV_LOG_LEVEL_5, "result:%d, len:%d\r\n", result, len);
	if(memcmp(rP, "$", 1) == 0)
	{
		if(hal_nvCrcCheck(rP+2, len) == (uint8)*(rP+len+2))
		{
			memcpy(sn_data, rP+2, len);
			sysDelayMs(10);
			sysLOG(NV_LOG_LEVEL_5, "SN get right!,len:%d,the SN :%s\r\n", len, sn_data);
			ret = len;
		}
	}
	free(rP);
	return ret;    
}


/*
*Function:		hal_snWriteSN
*Description:	写SN号
*Input:			*sn_data-SN号指针
*Output:		NULL
*Hardware:
*Return:		>0-成功写入字节数; <=0-失败
*Others:
*/
int16 hal_snWriteSN(int8 *sn_data)
{
	int16 ret = -1;
	int16 len = 0;
	int8 *rP = NULL;
	
	
	rP = malloc(NV_SN_LEN + 1);
	if(rP == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN + 1);
	
	len = strlen(sn_data);
	sprintf(rP, "$");
	*(rP+1) = len;
	sprintf(rP+2,"%s", sn_data);
	*(rP+len+2) = hal_nvCrcCheck(rP+2, len);
	
	
	ret = hal_nvWrite(rP, NV_SN_LEN);	
	sysLOG(NV_LOG_LEVEL_5, "The content of SN is %s,ret is %d\r\n", sn_data, ret);
	free(rP);
	if(ret > 0)
	{
		ret = len;
	}
	return ret;
	
	
}


/*
*Function:		hal_nvReadCustomerID
*Description:	读取客户标识ID
*Input:			NULL
*Output:		*cid_data-读取客户标识ID存储指针
*Hardware:
*Return:		>0-ID读取到的长度；<=0-读取失败
*Others:
*/
int16 hal_nvReadCustomerID(int8 *cid_data)
{
	int16 ret = -1;
    int16 result = 0;
    int16 len = 0;
	int8 *rP_t = NULL;
	int8 *rP = NULL;
	
	rP_t = malloc(NV_SN_LEN + NV_CID_LEN + 1);
	if(rP_t == NULL)
	{	
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + 1);
	
	rP = malloc(NV_CID_LEN + 1);
	if(rP == NULL)
	{
		free(rP_t);			
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_CID_LEN + 1);
	
	result = hal_nvRead(rP_t, (NV_SN_LEN+NV_CID_LEN));
	if(result <= 0)
	{
		
		sysLOG(NV_LOG_LEVEL_4, "<ERR> hal_nvRead failed! result:%d\r\n", result);
		free(rP_t);
		free(rP);
		return result;
	}
	result = memcpy(rP, rP_t+NV_SN_LEN, NV_CID_LEN);
	free(rP_t);
	
	len = *(rP+1);
	if(memcmp(rP, "$", 1) == 0)
	{
		if(hal_nvCrcCheck(rP+2, len) == (uint8)*(rP+len+2))
		{
			memcpy(cid_data, rP+2, len);			
			sysLOG(NV_LOG_LEVEL_5, "Cid get right!,len:%d,the Cid :byte0-%x,byte1-%x\r\n", len, cid_data[0],cid_data[1]);
			ret = len;
		}
	}
	
	sysLOG(NV_LOG_LEVEL_4, "ret=%d\r\n", ret);
	free(rP);
	return ret;    
}



/*
*Function:		hal_nvWriteCustomerID
*Description:	写客户标识ID
*Input:			*cid_data-客户标识ID指针, cid_len-客户标识ID字节个数,最大支持16个字节
*Output:		NULL
*Hardware:
*Return:		>0-成功写入字节数；<=0-失败
*Others:
*/
int16 hal_nvWriteCustomerID(int8 *cid_data, uint8 cid_len)
{
	int16 ret = -1;
	//int8 len = 0;
	int16 len_t = 0;
	int8 *rP = NULL;
	int8 *rP_t = NULL;

	if(cid_len > 16)
	{
		sysLOG(NV_LOG_LEVEL_3, "<ERR> lenth failed!\r\n");
		return -2;
	}
	
	rP = malloc(NV_SN_LEN + NV_CID_LEN + 1);
	if(rP == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN + NV_CID_LEN + 1);

	rP_t = malloc(NV_SN_LEN + 1);
	if(rP_t == NULL)
	{
		free(rP);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN + 1);
	
	len_t = hal_nvReadSN(rP_t);
	if(len_t == -1)//no SN
	{
		
	}
	else// 存在 SN
	{
		*(rP) = '$';
		*(rP+1) = len_t;
		memcpy(rP+2, rP_t, len_t);
		*(rP+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	
	
	//len = strlen(hw_data);
	*(rP+NV_SN_LEN) = '$';
	*(rP+NV_SN_LEN+1) = cid_len;
	memcpy(rP+NV_SN_LEN+2, cid_data, cid_len);
	*(rP+NV_SN_LEN+cid_len+2) = hal_nvCrcCheck(rP+NV_SN_LEN+2, cid_len);
	
	ret = hal_nvWrite(rP, NV_SN_LEN+NV_CID_LEN);	
	sysLOG(NV_LOG_LEVEL_5, "The content of Cid is byte0-%d,byte1-%d,byte2-%d,ret is %d\r\n", cid_data[0], cid_data[1],cid_data[2], ret);
	free(rP_t);
	free(rP);
	if(ret > 0)
	{
		ret = cid_len;
	}
	return ret;
	
	
}


/*
*Function:		hal_nvReadTerm
*Description:	读取内部机型数据
*Input:			NULL
*Output:		*term_data-读取内部机型数据指针
*Hardware:
*Return:		>0-读取到的长度；<=0-读取失败
*Others:
*/
int16 hal_nvReadTerm(int8 *term_data)
{
	int16 ret = -1;
    int16 result = 0;
    int16 len = 0;
	int8 *rP_t = NULL;
	int8 *rP = NULL;
	
	rP_t = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + 1);
	if(rP_t == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + 1);
	
	rP = malloc(NV_TERM_LEN + 1);
	if(rP == NULL)
	{
		free(rP_t);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_TERM_LEN + 1);
	
	result = hal_nvRead(rP_t, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN);
	if(result <= 0)
	{
		free(rP_t);
		free(rP);
		return result;
	}
	result = memcpy(rP, rP_t+NV_SN_LEN+NV_CID_LEN, NV_TERM_LEN);
	free(rP_t);
	
	len = *(rP+1);
	
	sysLOG(NV_LOG_LEVEL_5, "the rP:%d, %s\r\n", *rP, rP);
	if(memcmp(rP, "$", 1) == 0)
	{
		if(hal_nvCrcCheck(rP+2, len) == (uint8)*(rP+len+2))
		{
			memcpy(term_data, rP+2, len);			
			sysLOG(NV_LOG_LEVEL_5, "term get right!,len:%d,the term :byte0-%x,byte1-%x,byte2-%x,byte3-%x,byte4-%x\r\n", len, term_data[0],term_data[1],term_data[2],term_data[3], term_data[4]);
			ret = len;
		}
	}
	free(rP);
	return ret;    
}



/*
*Function:		hal_nvWriteTerm
*Description:	写内部机型数据
*Input:			*term_data-内部机型数据指针, hw_len-字节个数
*Output:		NULL
*Hardware:
*Return:		>0-成功写入字节数；<=0-失败
*Others:
*/
int16 hal_nvWriteTerm(int8 *term_data, uint8 term_len)
{
	int16 ret = -1;
	//int8 len = 0;
	int16 len_t = 0;
	int8 *rP = NULL;
	int8 *rP_t = NULL;
	
	rP = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + 1);
	if(rP == NULL)
	{	
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN + NV_CID_LEN +NV_TERM_LEN + 1);

	rP_t = malloc(NV_SN_LEN + NV_CID_LEN + 1);
	if(rP_t == NULL)
	{
		free(rP);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN +NV_CID_LEN + 1);
	
	len_t = hal_nvReadSN(rP_t);
	if(len_t == -1)//no SN
	{
		
	}
	else// 存在 SN
	{
		*(rP) = '$';
		*(rP+1) = len_t;
		memcpy(rP+2, rP_t, len_t);
		*(rP+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + 1);
	len_t = hal_nvReadCustomerID(rP_t);
	if(len_t == -1)//no CID
	{
		sysLOG(NV_LOG_LEVEL_4, "no CID\r\n");
		
	}
	else// 存在 CID
	{
		*(rP+NV_SN_LEN) = '$';
		*(rP+NV_SN_LEN+1) = len_t;
		memcpy(rP+NV_SN_LEN+2, rP_t, len_t);
		*(rP+NV_SN_LEN+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}
	
	
	//len = strlen(hw_data);
	*(rP+NV_SN_LEN+NV_CID_LEN) = '$';
	*(rP+NV_SN_LEN+NV_CID_LEN+1) = term_len;
	memcpy(rP+NV_SN_LEN+NV_CID_LEN+2, term_data, term_len);
	*(rP+NV_SN_LEN+NV_CID_LEN+term_len+2) = hal_nvCrcCheck(rP+NV_SN_LEN+NV_CID_LEN+2, term_len);
	
	ret = hal_nvWrite(rP, NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN);	
	sysLOG(NV_LOG_LEVEL_5, "The content of term is byte0-%d,byte1-%d,byte2-%d,byte3-%d,byte4-%d,ret is %d\r\n", term_data[0], term_data[1],term_data[2],term_data[3],term_data[4], ret);
	free(rP_t);
	free(rP);
	if(ret > 0)
	{
		ret = term_len;
	}
	return ret;
	
	
}


/*
*Function:		hal_nvReadHwVersion
*Description:	读取硬件版本
*Input:			NULL
*Output:		hw_data-读取硬件版本存储指针
*Hardware:
*Return:		 >0-版本读取到的长度；<=0-读取失败
*Others:
*/
int16 hal_nvReadHwVersion(int8 *hw_data)
{
	int16 ret = -1;
    int16 result = 0;
    int16 len = 0;
	int8 *rP_t = NULL;
	int8 *rP = NULL;
	
	rP_t = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	if(rP_t == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	
	rP = malloc(NV_HW_LEN + 1);
	if(rP == NULL)
	{
		free(rP_t);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_HW_LEN + 1);
	
	result = hal_nvRead(rP_t, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN);
	if(result <= 0)
	{
		free(rP_t);
		free(rP);
		return result;
	}
	result = memcpy(rP, rP_t+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN, NV_HW_LEN);
	free(rP_t);
	
	len = *(rP+1);
	
	sysLOG(NV_LOG_LEVEL_5, "the rP:%d, %s\r\n", *rP, rP);
	if(memcmp(rP, "$", 1) == 0)
	{
		if(hal_nvCrcCheck(rP+2, len) == (uint8)*(rP+len+2))
		{
			memcpy(hw_data, rP+2, len);			
			sysLOG(NV_LOG_LEVEL_5, "Hw get right!,len:%d,the Hw :byte0-%x,byte1-%x,byte2-%x,byte3-%x\r\n", len, hw_data[0],hw_data[1],hw_data[2],hw_data[3]);
			ret = len;
		}
	}
	free(rP);
	return ret;    
}


/*
*Function:		hal_nvWriteHwVersion
*Description:	写硬件版本号
*Input:			*hw_data-硬件版本号指针, hw_len-硬件版本字节个数
*Output:		NULL
*Hardware:
*Return:		>0:成功写入字节数；<=0:失败
*Others:
*/
int16 hal_nvWriteHwVersion(int8 *hw_data, uint8 hw_len)
{
	int16 ret = -1;
	//int8 len = 0;
	int16 len_t = 0;
	int8 *rP = NULL;
	int8 *rP_t = NULL;
	
	rP = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	if(rP == NULL)
	{	
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN + NV_CID_LEN +NV_TERM_LEN + NV_HW_LEN + 1);

	rP_t = malloc(NV_SN_LEN + NV_CID_LEN +NV_TERM_LEN + 1);
	if(rP_t == NULL)
	{
		free(rP);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN +NV_CID_LEN + NV_TERM_LEN + 1);
	
	len_t = hal_nvReadSN(rP_t);
	if(len_t == -1)//no SN
	{
		
	}
	else// 存在 SN
	{
		*(rP) = '$';
		*(rP+1) = len_t;
		memcpy(rP+2, rP_t, len_t);
		*(rP+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + 1);
	len_t = hal_nvReadCustomerID(rP_t);
	if(len_t == -1)//no CID
	{
		sysLOG(NV_LOG_LEVEL_4, "no CID\r\n");
		
	}
	else// 存在 CID
	{
		*(rP+NV_SN_LEN) = '$';
		*(rP+NV_SN_LEN+1) = len_t;
		memcpy(rP+NV_SN_LEN+2, rP_t, len_t);
		*(rP+NV_SN_LEN+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + 1);
	len_t = hal_nvReadTerm(rP_t);
	if(len_t == -1)//no term
	{
		sysLOG(NV_LOG_LEVEL_4, "no Term\r\n");
		
	}
	else// 存在 Term
	{
		*(rP+NV_SN_LEN+NV_CID_LEN) = '$';
		*(rP+NV_SN_LEN+NV_CID_LEN+1) = len_t;
		memcpy(rP+NV_SN_LEN+NV_CID_LEN+2, rP_t, len_t);
		*(rP+NV_SN_LEN+NV_CID_LEN+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}
	
	
	//len = strlen(hw_data);
	*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN) = '$';
	*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+1) = hw_len;
	memcpy(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+2, hw_data, hw_len);
	*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+hw_len+2) = hal_nvCrcCheck(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+2, hw_len);
	
	ret = hal_nvWrite(rP, NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN);	
	sysLOG(NV_LOG_LEVEL_5, "The content of Hw is byte0-%d,byte1-%d,byte2-%d,byte3-%d,byte4-%d,ret is %d\r\n", hw_data[0], hw_data[1],hw_data[2],hw_data[3],hw_data[4], ret);
	free(rP_t);
	free(rP);
	if(ret > 0)
	{
		ret = hw_len;
	}
	return ret;
	
	
}

/*
*
*Fttype在NV区中共分配了8个字节，一个起始位，一个len，一个校验位，其中数据位只用到了第一个字节。
*fttype_data[0]:FONTFLASH or FONTFS
*
*/


/*
*Function:		hal_nvReadFontType
*Description:	读取字库类型标识
*Input:			NULL
*Output:		*fttype_data-读取字库类型存储指针
*Hardware:
*Return:		>0-读取到的长度；<=0-读取失败
*Others:
*/
int16 hal_nvReadFontType(int8 *fttype_data)
{
	int16 ret = -1;
    int16 result = 0;
    int16 len = 0;
	int8 *rP_t = NULL;
	int8 *rP = NULL;
	
	rP_t = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + NV_FTTYPE_LEN + 1);
	if(rP_t == NULL)
	{
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + NV_FTTYPE_LEN + 1);
	
	rP = malloc(NV_FTTYPE_LEN + 1);
	if(rP == NULL)
	{
		free(rP_t);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_FTTYPE_LEN + 1);
	
	result = hal_nvRead(rP_t, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + NV_FTTYPE_LEN);
	if(result <= 0)
	{
		free(rP_t);
		free(rP);
		return result;
	}
	result = memcpy(rP, rP_t+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN, NV_FTTYPE_LEN);
	free(rP_t);
	
	len = *(rP+1);
	
	sysLOG(NV_LOG_LEVEL_5, "the rP:%d, %s\r\n", *rP, rP);
	if(memcmp(rP, "$", 1) == 0)
	{
		if(hal_nvCrcCheck(rP+2, len) == (uint8)*(rP+len+2))
		{
			memcpy(fttype_data, rP+2, len);			
			sysLOG(NV_LOG_LEVEL_4, "FontType get right!,len:%d\r\n", len);
			ret = len;
		}
	}
	free(rP);
	return ret;    
}



/*
*Function:		hal_nvWriteFontType
*Description:	写字库类型标识
*Input:			*hw_data-硬件版本号指针, hw_len-硬件版本字节个数
*Output:		NULL
*Hardware:
*Return:		>0-成功写入字节数；<=0-失败
*Others:
*/
int16 hal_nvWriteFontType(int8 *fttype_data, uint8 fttype_len)
{
	int16 ret = -1;
	//int8 len = 0;
	int16 len_t = 0;
	int8 *rP = NULL;
	int8 *rP_t = NULL;
	
	rP = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + NV_FTTYPE_LEN + 1);
	if(rP == NULL)
	{	
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN + NV_CID_LEN +NV_TERM_LEN + NV_HW_LEN + NV_FTTYPE_LEN + 1);

	rP_t = malloc(NV_SN_LEN + NV_CID_LEN +NV_TERM_LEN + NV_HW_LEN + 1);
	if(rP_t == NULL)
	{
		free(rP);		
		sysLOG(NV_LOG_LEVEL_2, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP_t, 0, NV_SN_LEN +NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	
	len_t = hal_nvReadSN(rP_t);
	if(len_t == -1)//no SN
	{
		
	}
	else// 存在 SN
	{
		*(rP) = '$';
		*(rP+1) = len_t;
		memcpy(rP+2, rP_t, len_t);
		*(rP+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	len_t = hal_nvReadCustomerID(rP_t);
	if(len_t == -1)//no CID
	{
		sysLOG(NV_LOG_LEVEL_4, "no CID\r\n");
		
	}
	else// 存在 CID
	{
		*(rP+NV_SN_LEN) = '$';
		*(rP+NV_SN_LEN+1) = len_t;
		memcpy(rP+NV_SN_LEN+2, rP_t, len_t);
		*(rP+NV_SN_LEN+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	len_t = hal_nvReadTerm(rP_t);
	if(len_t == -1)//no term
	{
		sysLOG(NV_LOG_LEVEL_4, "no Term\r\n");
		
	}
	else// 存在 Term
	{
		*(rP+NV_SN_LEN+NV_CID_LEN) = '$';
		*(rP+NV_SN_LEN+NV_CID_LEN+1) = len_t;
		memcpy(rP+NV_SN_LEN+NV_CID_LEN+2, rP_t, len_t);
		*(rP+NV_SN_LEN+NV_CID_LEN+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}

	memset(rP_t, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN + 1);
	len_t = hal_nvReadHwVersion(rP_t);
	if(len_t == -1)//no HW
	{
		sysLOG(NV_LOG_LEVEL_4, "no HW\r\n");
		
	}
	else// 存在 HW
	{
		*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN) = '$';
		*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+1) = len_t;
		memcpy(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+2, rP_t, len_t);
		*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+2+len_t) = hal_nvCrcCheck(rP_t, len_t);
	}
	
	
	*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN) = '$';
	*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN+1) = fttype_len;
	memcpy(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN+2, fttype_data, fttype_len);
	*(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN+fttype_len+2) = hal_nvCrcCheck(rP+NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN+2, fttype_len);
	
	ret = hal_nvWrite(rP, NV_SN_LEN+NV_CID_LEN+NV_TERM_LEN+NV_HW_LEN+NV_FTTYPE_LEN);	
	sysLOG(NV_LOG_LEVEL_5, "The content of FTTYPE is byte0-0x%x,byte1-0x%x,byte2-0x%x,byte3-0x%x,byte4-0x%x,ret is %d\r\n", fttype_data[0], fttype_data[1], fttype_data[2], fttype_data[3], fttype_data[4], ret);
	free(rP_t);
	free(rP);
	if(ret > 0)
	{
		ret = fttype_len;
	}
	return ret;
	
	
}

/**********************************TEST**********************************/

#if MAINTEST_FLAG

void hal_nvWriteSN_Test(void)
{
	int8 sn_temp[20];
	memset(sn_temp,0,sizeof(sn_temp));
	sprintf(sn_temp,"11234567890");
	
	sysLOG(NV_LOG_LEVEL_2, "hal_snWriteSN ret:%d\r\n", hal_snWriteSN(sn_temp));
}


void hal_nvWriteHw_Test(void)
{
	int8 hw_temp[20];
	memset(hw_temp,0,sizeof(hw_temp));
	hw_temp[0] = 0x31;
	hw_temp[1] = 0x31;
	hw_temp[2] = 0x32;
	hw_temp[3] = 0x33;
	
	sysLOG(NV_LOG_LEVEL_2, "hal_nvWriteHwVersion ret:%d\r\n", hal_nvWriteHwVersion(hw_temp, 4));
}

void hal_nvWriteCustomerID_Test(void)
{
	int8 cid_temp[20];
	memset(cid_temp,0,sizeof(cid_temp));
	cid_temp[0] = 0x31;
	cid_temp[1] = 0x32;

	sysLOG(NV_LOG_LEVEL_2, "hal_nvWriteCustomerID ret:%d\r\n", hal_nvWriteCustomerID(cid_temp, 2));
}

#if 0
static int16 NV_Clear(void)
{
	int16 ret = -1;
	int8 *rP = NULL;
	
	rP = malloc(NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN);
	if(rP == NULL)
	{		
		sysLOG(NV_LOG_LEVEL_1, "<ERR> malloc failed!\r\n");
		return ret;
	}
	memset(rP, 0, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN);
	
	ret = hal_nvWrite(rP, NV_SN_LEN + NV_CID_LEN + NV_TERM_LEN + NV_HW_LEN);
	
	sysLOG(NV_LOG_LEVEL_1, "ret:%d\r\n", ret);
	free(rP);
	return ret;

	
}
#endif

void SN_Hw_test(void)
{
	
	int8 nv_testbuff[50];

	
	
	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadCustomerID ret:%d\r\n", hal_nvReadCustomerID(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2]);

	memset(nv_testbuff, 0, sizeof(nv_testbuff));	
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadSN ret:%d\r\n", hal_nvReadSN(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%s\r\n", nv_testbuff);

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadHwVersion ret:%d\r\n", hal_nvReadHwVersion(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2],nv_testbuff[3],nv_testbuff[4]);

	
#if 1		

	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");
	hal_nvWriteSN_Test();
	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadCustomerID ret:%d\r\n", hal_nvReadCustomerID(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2]);

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadSN ret:%d\r\n", hal_nvReadSN(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%s\r\n", nv_testbuff);

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadHwVersion ret:%d\r\n", hal_nvReadHwVersion(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2],nv_testbuff[3],nv_testbuff[4]);
		

	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");

	hal_nvWriteHw_Test();
	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");

	memset(nv_testbuff, 0, sizeof(nv_testbuff));	
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadCustomerID ret:%d\r\n", hal_nvReadCustomerID(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2]);
	
	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadSN ret:%d\r\n", hal_nvReadSN(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%s\r\n", nv_testbuff);

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadHwVersion ret:%d\r\n", hal_nvReadHwVersion(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2],nv_testbuff[3],nv_testbuff[4]);
	
	

	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");
#endif
	hal_nvWriteCustomerID_Test();
	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadCustomerID ret:%d\r\n", hal_nvReadCustomerID(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2]);
	
	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadSN ret:%d\r\n", hal_nvReadSN(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%s\r\n", nv_testbuff);

	memset(nv_testbuff, 0, sizeof(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "hal_nvReadHwVersion ret:%d\r\n", hal_nvReadHwVersion(nv_testbuff));
	sysLOG(NV_LOG_LEVEL_2, "nv_testbuff:%x,%x,%x,%x,%x,\r\n", nv_testbuff[0],nv_testbuff[1],nv_testbuff[2],nv_testbuff[3],nv_testbuff[4]);
	
	
	
	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");
	//CZ_TRACE(2,"--SN_Hw_test--,NV_Clear ret:%d\r\n",NV_Clear());

	sysLOG(NV_LOG_LEVEL_2, "---------------------\r\n");

}

void static DR_loop_printf(int8 *data, int len)
{
	for(int i = 0; i < len; i++)
	{
		sysLOG(NV_LOG_LEVEL_2, "data=%x, i=%d\r\n", *(data+i), i);
		
	}
}

void NV_test(void)
{
	int32 iRet = -1;
	int8 nvtest_tmp[128];

	memset(nvtest_tmp, 0, sizeof(nvtest_tmp));
	iRet = fibo_set_get_deviceNUM(GET_DEVICE_NUM, nvtest_tmp, NV_LEN);
	DR_loop_printf(nvtest_tmp, NV_LEN);
	sysLOG(NV_LOG_LEVEL_2, "The fibo_get_device_num iRet is %d, nvtest_tmp:%s\r\n", iRet, nvtest_tmp);
	if(iRet < 0)
	{		
		sysLOG(NV_LOG_LEVEL_2, "<ERR>The fibo_get_device_num iRet is %d\r\n", iRet);
		//return;
	}

	memset(nvtest_tmp, 0, sizeof(nvtest_tmp));
	sprintf(nvtest_tmp ,"012345678901234567890123456789");
	sprintf(&nvtest_tmp[40], "abc");
	DR_loop_printf(nvtest_tmp, NV_LEN);
	iRet = fibo_set_get_deviceNUM(SET_DEVICE_NUM, nvtest_tmp, NV_LEN);
	if(iRet < 0)
	{		
		sysLOG(NV_LOG_LEVEL_2, "<ERR>The fibo_set_get_deviceNUM iRet is %d\r\n", iRet);
		return;
	}

	memset(nvtest_tmp, 0, sizeof(nvtest_tmp));
	iRet = fibo_set_get_deviceNUM(GET_DEVICE_NUM, nvtest_tmp, NV_LEN);
	DR_loop_printf(nvtest_tmp, NV_LEN);
	sysLOG(NV_LOG_LEVEL_2, "The fibo_get_device_num iRet is %d, nvtest_tmp:%s, %s\r\n", iRet, nvtest_tmp, &nvtest_tmp[40]);
	if(iRet < 0)
	{		
		sysLOG(NV_LOG_LEVEL_2, "<ERR>The fibo_get_device_num iRet is %d\r\n", iRet);
		return;
	}

	memset(nvtest_tmp, 0, sizeof(nvtest_tmp));
	sprintf(nvtest_tmp ,"abcdef");
	sprintf(&nvtest_tmp[20] ,"hijk");
	DR_loop_printf(nvtest_tmp, NV_LEN);
	iRet = fibo_set_get_deviceNUM(SET_DEVICE_NUM, nvtest_tmp, NV_LEN);
	if(iRet < 0)
	{		
		sysLOG(NV_LOG_LEVEL_2, "<ERR>The fibo_set_get_deviceNUM iRet is %d\r\n", iRet);
		return;
	}

	memset(nvtest_tmp, 0, sizeof(nvtest_tmp));
	iRet = fibo_set_get_deviceNUM(GET_DEVICE_NUM, nvtest_tmp, NV_LEN);
	DR_loop_printf(nvtest_tmp, NV_LEN);
	sysLOG(NV_LOG_LEVEL_2, "The fibo_get_device_num iRet is %d, nvtest_tmp:%s, %s, %s\r\n", iRet, nvtest_tmp, &nvtest_tmp[20], &nvtest_tmp[40]);
	if(iRet < 0)
	{		
		sysLOG(NV_LOG_LEVEL_2, "<ERR>The fibo_get_device_num iRet is %d\r\n", iRet);
		return;
	}

	memset(nvtest_tmp, 0, sizeof(nvtest_tmp));
	iRet = fibo_set_get_deviceNUM(GET_DEVICE_NUM, nvtest_tmp, 3);
	DR_loop_printf(nvtest_tmp, NV_LEN);
	sysLOG(NV_LOG_LEVEL_2, "The fibo_get_device_num iRet is %d, nvtest_tmp:%s\r\n", iRet, nvtest_tmp);
	if(iRet < 0)
	{		
		sysLOG(NV_LOG_LEVEL_2, "<ERR>The fibo_get_device_num iRet is %d\r\n", iRet);
		return;
	}
	

}

#endif

