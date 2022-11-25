/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_load.c
** Description:		下载及外部通信相关接口
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

#define FILE_SIGN_FLAG "\xAA\x55\xAA\x55\x55\xAA\x55\xAA"
#define FILE_SIGN_FLAG_AA66 "\xAA\x66\xAA\x66\x66\xAA\x66\xAA"

#define SIGN_FILE_NAME "/app/ufs/rsa_sign_file.bin"
#define FILE_PACK_SIZE 1024

#define FILE_NAME_TMP "filename_tmp.txt"
#define CFG_FILE_NAME "/app/ufs/cfg_file.ini"
#define AUDIO_NAME_PLAY "/FFS/audioplay.mp3"


LD_PKG g_stRecvPkg;
LD_PKG g_stSendPkg;

APP_S g_stAppStr;
LOADFILE_S g_stLoadfileStr;


static uint8 g_ui8LoadAppStart = 0;
static uint8 g_ui8ShakehandFlag = 0;

static uint8 g_ui8LoadStep = 0;


/*
*Function:		hal_ldAPPStrFormat
*Description:	初始化结构体APP_S
*Input:			*ap_sP:APP_S结构体
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ldAPPStrFormat(APP_S *ap_sP)
{
	ap_sP->sizeAPP = 0;
	ap_sP->sizeAPP_recvd = 0;
	ap_sP->iPackNo = 0;
	memset(ap_sP->g_DesAppName, 0, sizeof(ap_sP->g_DesAppName));
	memset(ap_sP->SHA_value, 0, sizeof(ap_sP->SHA_value));
	memset(ap_sP->SHA_value_recvd, 0, sizeof(ap_sP->SHA_value_recvd));
	memset(ap_sP->MD5_value, 0, sizeof(ap_sP->MD5_value));
	memset(ap_sP->MD5_value_recvd, 0, sizeof(ap_sP->MD5_value_recvd));
	ap_sP->g_signLen = 0;
	ap_sP->g_isSignExist = 0;
	memset(ap_sP->signVal, 0, sizeof(ap_sP->signVal));
	ap_sP->appflag = 0;

	memset(ap_sP->bininfoAA66.head, 0, sizeof(ap_sP->bininfoAA66.head));
	memset(ap_sP->bininfoAA66.format, 0, sizeof(ap_sP->bininfoAA66.format));
	ap_sP->bininfoAA66.appsize = 0;
	memset(ap_sP->bininfoAA66.compatible, 0, sizeof(ap_sP->bininfoAA66.compatible));
	memset(ap_sP->bininfoAA66.reservedinfo, 0, sizeof(ap_sP->bininfoAA66.reservedinfo));
	memset(ap_sP->bininfoAA66.hashvalue, 0, sizeof(ap_sP->bininfoAA66.hashvalue));

}


/*
*Function:		hal_ldIsSignFlagExist
*Description:	判断是否有签名区
*Input:			*data,app前8个字节
*Output:		NULL
*Hardware:
*Return:		TRUE:有签名区; FALSE:无签名区
*Others:
*/
int hal_ldIsSignFlagExist(int8 *data)
{
    int i;
    
    if(0 == memcmp(FILE_SIGN_FLAG,data,8))
    {
    	g_stAppStr.appflag = 0;
        return TRUE;
    }
	else if(0 == memcmp(FILE_SIGN_FLAG_AA66,data,8))
	{
		g_stAppStr.appflag = 1;
		return TRUE;
	}
    else
    {
        for(i=0;i<8;i++)
        {
        
			sysLOG(LOAD_LOG_LEVEL_3, "<ERR>, data:%x\r\n", data[i]);
        }
        return FALSE;
    }
}


/*
*Function:		hal_ldWriteSignInfo
*Description:	写签名信息到SIGN_FILE_NAME文件中
*Input:			*data:接收到的数据指针; dataLen:接收到的数据长度; lastLen:剩余长度
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:写入长度
*Others:
*/
int hal_ldWriteSignInfo(uint8 *data,uint32 dataLen,uint32 lastLen)
{
    int fd;
    int iRet = -1;
    uint32 uiwLen;//写入数据长度
    uint8 isFirstFrame = 0;  
    uint32 fileSize = 0;
    if(dataLen > lastLen)//剩余长度写入
    {
        uiwLen = lastLen;
    }
    else//写入整包数据
    {
        uiwLen = dataLen;
    }

    //第一帧数据
    if(lastLen >= 1024)
    {
        isFirstFrame = 1;
		
        fd = hal_fileOpen(SIGN_FILE_NAME, O_RDWR | O_CREAT);
        if(fd < 0)
        {
            iRet = -1;
            return iRet;
        }
        iRet = hal_fileGetFileSize(SIGN_FILE_NAME);
        if(iRet < 0)
        {
        
			sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_fileGetFileSize failed, iRet:%d, SIGN_FILE_NAME:%s\r\n", iRet, SIGN_FILE_NAME);
            iRet = -2;
            goto end;
        }
		fileSize = iRet;
		
		sysLOG(LOAD_LOG_LEVEL_4, "<SUCC> hal_fileGetFileSize, fileSize:%d, SIGN_FILE_NAME:%s\r\n", fileSize, SIGN_FILE_NAME);
        if(fileSize>0)
        {                      
            iRet = hal_fileClose(fd);
    		if(iRet != 0)
    		{
    		
				sysLOG(LOAD_LOG_LEVEL_2, "<ERR> hal_fileClose, iRet:%d, fd:%x\r\n", iRet, fd);
    			return -3;
    		} 
            iRet = hal_fileRemove(SIGN_FILE_NAME);
            if(iRet != 0)
    		{
    		
				sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_fileRemove, iRet:%d, fd:%x\r\n", iRet, fd);
    		    iRet = -4;
                goto end;
    		}            
        }
        else
        {
            iRet = hal_fileClose(fd);
    		if(iRet != 0)
    		{
    		
				sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_fileClose, iRet:%d, fd:%x\r\n", iRet, fd);
    			return -3;
    		} 
        }
    }
    //创建签名文件，将数据写入缓存区
    
  	fd = hal_fileOpen(SIGN_FILE_NAME, O_RDWR | O_CREAT);
	if(fd >= 0)
	{
	    //第一帧数据
        if(1 == isFirstFrame)
        {
        	iRet = hal_fileSeek(fd, 0, SEEK_SET);
        	if(iRet < 0)
        	{
        	
				sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_fileSeek, %d\r\n", iRet);
                iRet = -5;
        		goto end;
        	}
        }
        else
        {
        	iRet = hal_fileSeek(fd, 0, SEEK_END);
        	if(iRet < 0)
        	{
        	
				sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_fileSeek, %d\r\n", iRet);
                iRet = -6;
        		goto end;
        	}
        }

    	iRet = hal_fileWrite(fd, data, uiwLen);      
    	if(iRet != (int)uiwLen)
    	{
    	
			sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_fileWrite, %d, %d\r\n", iRet,uiwLen);
            iRet = -7;
    		goto end;
    	}

    end:        
		hal_fileClose(fd);
		if(iRet != 0)
		{
			return iRet;
		}	
	}  
    else
    {
        return -9;
    }

    return uiwLen;

    
}




/*
*Function:		hal_ldGetAppName
*Description:	获得appname文件名(全路径)
*Input:			NULL
*Output:		*appname:文件名
*Hardware:
*Return:		>0: 文件名长度;其他：失败
*Others:
*/
static int hal_ldGetAppName(int8 *appname)
{
	int iRet = -1;
	int8 appnametemp[64];
	memset(appnametemp, 0, sizeof(appnametemp));
	sprintf(appnametemp, "/app/ufs/UserAPP.bin");
	iRet = strlen(appnametemp);
	memcpy(appname, appnametemp, iRet);
	return iRet;
}

/*
*Function:		hal_ldDownloadApp
*Description:	下载APP文件到制定目录
*Input:			*app_sP:APP_S结构体，*data:数据指针,数据都是16进制，dataLen:要写入的数据长度
*Output:		NULL
*Hardware:
*Return:		>0:成功写入字节数；<=0:失败
*Others:
*/
static int32 hal_ldDownloadApp(APP_S *app_sP, int8 *data, uint32 dataLen)
{
	
	int32 fd;
	int32 iRet = 0;

	if(g_ui8LoadAppStart == 0)//第一帧
	{
		g_ui8LoadAppStart = 1;
		hal_ldAPPStrFormat(app_sP);
		iRet = hal_ldGetAppName(app_sP->g_DesAppName);
		if(iRet <= 0)return iRet;
		hal_fileRemove(app_sP->g_DesAppName);
		
		//2、获取是否有签名信息
        if(0 != hal_ldIsSignFlagExist(data))
        {
            //存在签名，需要将签名内容存储到文件中备用
            
            g_stAppStr.g_signLen = 1024;
            g_stAppStr.g_isSignExist = 1;
        }
        else
        {
            g_stAppStr.g_isSignExist = 0;
        }
		sysLOG(LOAD_LOG_LEVEL_1, "g_isSignExist=%d\r\n", g_stAppStr.g_isSignExist);
	}

	app_sP->sizeAPP_recvd += dataLen;
	app_sP->iPackNo += 1;


	if(0 != g_stAppStr.g_signLen)
	{
		//write sign info to file
		iRet = hal_ldWriteSignInfo(data,dataLen,g_stAppStr.g_signLen);
		if(iRet < 0)//写入出错 返回
		{
			return FALSE;
		}
		else
		{	// 更新剩余数据长度
			g_stAppStr.g_signLen -= iRet;
		}		 
		if(g_stAppStr.g_signLen == 0)
		{
			data+=iRet;
			dataLen -=iRet; 
			//iDelete = 1;
			
			sysLOG(LOAD_LOG_LEVEL_4, "SIGN DATA WRITE FINISH  %x,%d\r\n", (uint32)data, dataLen);
		}		   
		else if(g_stAppStr.g_signLen > 0)
		{
		
			sysLOG(LOAD_LOG_LEVEL_4, "SIGN DATA WRITEing  %d\r\n", g_stAppStr.g_signLen);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}


	fd = hal_fileOpen(app_sP->g_DesAppName, O_RDWR | O_CREAT);
	if(fd < 0)
	{
		return fd;
	}

	iRet = hal_fileSeek(fd, 0, SEEK_END);
	if(iRet < 0)
	{
		hal_fileClose(fd);
		return iRet;
	}

	iRet = hal_fileWrite(fd, data, dataLen);
	if(iRet != (dataLen))
	{
		hal_fileClose(fd);
		return iRet;
	}

	iRet = hal_fileClose(fd);
	if(iRet != 0)
	{
		
		return iRet;
	}
	
	return dataLen;
}



/*
*Function:		hal_ldUpdateApp
*Description:	update APP 接口
*Input:			*ldappname:APP文件名称；offset:APP读取偏移量,applen:ldappname文件中原生APP大小
*Output:		NULL
*Hardware:
*Return:		<0:失败；0：成功
*Others:
*/
static int32 hal_ldUpdateApp(int8 *ldappname, uint32 offset, int applen)
{
	int32 iRet = -1;
	int8 *rP = NULL;
	int32 filesizetmp;
	
	iRet = hal_fileGetFileSize(ldappname);
	sysLOG(FOTA_LOG_LEVEL_4, "hal_fileGetFileSize file:%s, iRet = %d offset:%d\r\n",ldappname, iRet,offset);
	if(iRet < 0)
	{
		return iRet;
	}
	filesizetmp = iRet - offset;
	if(filesizetmp < 0)
	{
		return filesizetmp;
	}
	if(filesizetmp < applen)
	{
		return -2;
	}
	filesizetmp = applen;
	//if(iRet != app_sP->sizeAPP_recvd)
	//{		
	//	sysLOG(LOAD_LOG_LEVEL_2, "<ERR>, size err,the iRet=%d\r\n", iRet);
	//	return -1;
	//}

	rP = malloc(filesizetmp+1);
	if(rP == NULL)
	{
		return -1;
	}
	memset(rP, 0, filesizetmp+1);
	iRet = hal_fileReadPro(ldappname, offset, rP, filesizetmp);
	sysLOG(FOTA_LOG_LEVEL_5, "hal_fileReadPro file:%s, iRet = %d, filesizetmp:%d\r\n",ldappname, iRet,filesizetmp);
	if(iRet != filesizetmp)
	{
		free(rP);
		return -1;
	}
	iRet = hal_fileRemove(ldappname);//读取APP之后就把文件系统中的删除掉
	sysLOG(FOTA_LOG_LEVEL_5, "hal_fileRemove:%s, iRet = %d\r\n",ldappname, iRet);
	iRet = hal_fotaAppCheck((int8 *)rP, filesizetmp);
	if(iRet < 0)
	{
		free(rP);
		sysLOG(LOAD_LOG_LEVEL_2, "<ERR> hal_fotaAppCheck failed! iRet = %d\r\n", iRet);
		return iRet;
	}
	g_stUpdateAppParam.rP = rP;
	g_stUpdateAppParam.size = filesizetmp;

	g_ui32Timer3ID = fibo_timer_new(2000, hal_fotaTimer3Period, NULL);//2000ms后开始升级
	sysLOG(LOAD_LOG_LEVEL_2, "<SUCC> hal_fotaAppCheck success! iRet = %d\r\n", iRet);

	return iRet;
}


/*
*Function:		hal_ldVerifySignDigist
*Description:	验证签名文件中的签名区摘要值是否正确
*Input:			*sign_file:签名文件
*Output:		NULL
*Hardware:
*Return:		TRUE: SUCCESS; FALSE: FAIL
*Others:
*/
static int hal_ldVerifySignDigist(uint8 *sign_file)
{
    int fd;
    int iRet;
    int lenth;
    
    uint8 *headerInfo = NULL;
    uint8 hashVal[SHA256_RESULT_LENGTH];

	headerInfo = malloc(1024);
	if(headerInfo == NULL)
	{
		return false;
	}

	memset(headerInfo, 0, 1024);
	
    if(sign_file == NULL)
    {
        sign_file = SIGN_FILE_NAME;
    }
    fd = hal_fileOpen(sign_file, O_RDWR | O_CREAT);
	sysLOG(LOAD_LOG_LEVEL_5, "hal_fileOpen,fd:%d\r\n", fd);
    if(fd < 0)
    {
        iRet = FALSE;
		goto exit;
    }
    iRet = hal_fileRead(fd, headerInfo, 1024);	
	sysLOG(LOAD_LOG_LEVEL_5, "hal_fileRead,iRet:%d\r\n", iRet);
    if(iRet < 0)
    {
        iRet = FALSE;
		goto exit;
    }
    iRet = hal_fileClose(fd);
	sysLOG(LOAD_LOG_LEVEL_5, "hal_fileClose,fd:%d\r\n", fd);
    if(iRet < 0)
    {
        iRet = FALSE;
		goto exit;
    }
    lenth = 1024 - SHA256_RESULT_LENGTH;
    
    saSha256(NULL, 0, NULL, HASH_MODE_INIT);
    saSha256(headerInfo, lenth, NULL, HASH_MODE_UPDATE); 
    saSha256(NULL, SHA256_RESULT_LENGTH, hashVal, HASH_MODE_FINAL);
    
    if( 0 == memcmp((uint8*)hashVal,headerInfo+lenth,SHA256_RESULT_LENGTH))
    {
        if(g_stAppStr.appflag == 0)
    	{
	        memcpy(g_stAppStr.signVal,headerInfo+16 ,256);
			sysLOG(LOAD_LOG_LEVEL_4, "hash value is right, g_stAppStr.appflag == 0\r\n");
	        iRet = TRUE;
			goto exit;
    	}
		else if(g_stAppStr.appflag == 1)
		{
			memcpy(g_stAppStr.bininfoAA66.head, headerInfo, BININFO_HEAD_SIZE);
			memcpy(g_stAppStr.bininfoAA66.format, headerInfo+BININFO_HEAD_SIZE, BININFO_FORMAT_SIZE);
			g_stAppStr.bininfoAA66.appsize |= headerInfo[BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE]<<24;
			g_stAppStr.bininfoAA66.appsize |= headerInfo[BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE+1]<<16;
			g_stAppStr.bininfoAA66.appsize |= headerInfo[BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE+2]<<8;
			g_stAppStr.bininfoAA66.appsize |= headerInfo[BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE+3];
			memcpy(g_stAppStr.bininfoAA66.compatible, headerInfo+BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE+4, BININFO_COMPA_SIZE);
			memcpy(g_stAppStr.bininfoAA66.reservedinfo, headerInfo+BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE+4+BININFO_COMPA_SIZE, BININFO_RESINFO_SIZE);
			memcpy(g_stAppStr.bininfoAA66.hashvalue, headerInfo+BININFO_HEAD_SIZE+BININFO_FORMAT_SIZE+4+BININFO_COMPA_SIZE+BININFO_RESINFO_SIZE, BININFO_HASHV_SIZE);
			sysLOG(LOAD_LOG_LEVEL_4, "hash value is right, g_stAppStr.appflag == 1\r\n");
			iRet = TRUE;
			goto exit;
		}
		else
		{
			sysLOG(LOAD_LOG_LEVEL_4, "hash value is right, But g_stAppStr.appflag == %d\r\n", g_stAppStr.appflag);
			iRet = false;
			goto exit;
		}
    }
    else
    {
    
		sysLOG(LOAD_LOG_LEVEL_2, "hash value is ERR\r\n");
        iRet = FALSE;
		goto exit;
    }
	
exit:

	sysLOG(LOAD_LOG_LEVEL_2, "g_stAppStr.appflag == %d\r\n", g_stAppStr.appflag);

	if(headerInfo != NULL)
		free(headerInfo);
	
	return iRet;
}


/*
*Function:		hal_ldVerifyAppDigist
*Description:	验证App签名
*Input:			*ldappname:APP文件名;offset:APP在文件中的偏移量
*Output:		NULL
*Hardware:
*Return:		TRUE: SUCCESS; FALSE: FAIL
*Others:
*/
static BOOL hal_ldVerifyAppDigist(uint8 *ldappname, uint32 offset)
{
    int fd;
    int iRet;
    int i;
    
    uint32 file_size;
    uint32 last_size;   
    
    uint8 hashVal[SHA256_RESULT_LENGTH];   
    uint8 *g_readbuf = NULL;

    uint8 output[256];
    uint32 lenth;

	g_readbuf = malloc(FILE_PACK_SIZE);
	if(g_readbuf == NULL)
	{
		iRet = false;
		goto exit;
	}

	memset(g_readbuf, 0, FILE_PACK_SIZE);
    
//1.2签名值正确
    //1.2.1计算摘要值
    fd = hal_fileOpen(ldappname, O_RDWR | O_CREAT);
    if(fd < 0)
    {
    
		sysLOG(LOAD_LOG_LEVEL_4, "hal_fileOpen, fd:%x\r\n", fd);
        iRet = FALSE;
		goto exit;
    }
    saSha256(NULL, 0, NULL, HASH_MODE_INIT);

    iRet = hal_fileGetFileSize(ldappname);
    if(iRet < 0)
    {
    
		sysLOG(LOAD_LOG_LEVEL_4, "hal_fileGetFileSize, iRet:%d\r\n", iRet);
        iRet = FALSE;
		goto exit;
    }
	file_size = iRet - offset;
	if(file_size < 0)
	{
		iRet = file_size;
		goto exit;
	}
    memset(hashVal,0x00,sizeof(hashVal));

    last_size = file_size % FILE_PACK_SIZE;
    file_size -= last_size;

	iRet = hal_fileSeek(fd, offset, SEEK_SET);
	if(iRet < 0)
    {
		sysLOG(LOAD_LOG_LEVEL_4, "hal_fileSeek, iRet:%d\r\n", iRet);
        iRet = FALSE;
		goto exit;
    }
    for(i=0;i<file_size;i+=FILE_PACK_SIZE)
    {
        memset(g_readbuf,0x00,FILE_PACK_SIZE);
        iRet = hal_fileSeek(fd, 0, SEEK_CUR);
        if(iRet < 0)
        {
        
			sysLOG(LOAD_LOG_LEVEL_4, "hal_fileSeek, iRet:%d\r\n", iRet);
            iRet = FALSE;
			goto exit;
        }
        iRet = hal_fileRead(fd, g_readbuf, FILE_PACK_SIZE);
        if(iRet < 0)
        {
			sysLOG(LOAD_LOG_LEVEL_4, "hal_fileRead, iRet:%d\r\n", iRet);
            iRet = FALSE;
			goto exit;
        }
        saSha256((uint8*)g_readbuf, (uint16)FILE_PACK_SIZE, NULL, HASH_MODE_UPDATE); 

    }
	sysLOG(LOAD_LOG_LEVEL_4, "hal_fileGetFileSize, size:%d, i:%d\r\n", file_size, i);
	  
    if(last_size != 0)
    { 
    
		sysLOG(LOAD_LOG_LEVEL_4, "last_size=%d\r\n", last_size);
        memset(g_readbuf,0x00,FILE_PACK_SIZE);
        
        iRet = hal_fileSeek(fd, 0, SEEK_CUR);
        if(iRet < 0)
        {
        
			sysLOG(LOAD_LOG_LEVEL_4, "hal_fileSeek, iRet:%d\r\n", iRet);
            iRet = FALSE;
			goto exit;
        }
        iRet = hal_fileRead(fd, g_readbuf, last_size);
        if(iRet < 0)
        {
        
			sysLOG(LOAD_LOG_LEVEL_4, "hal_fileRead, iRet:%d\r\n", iRet);
            iRet = FALSE;
			goto exit;
        }
       
        saSha256((uint8*)g_readbuf, (uint16)last_size, NULL, HASH_MODE_UPDATE); 
    }
    saSha256(NULL, SHA256_RESULT_LENGTH, hashVal, HASH_MODE_FINAL);
   
    iRet = hal_fileClose(fd);
    if(iRet < 0)
    {
    
		sysLOG(LOAD_LOG_LEVEL_4, "hal_fileClose, iRet:%d\r\n", iRet);
        iRet = FALSE;
		goto exit;
    }

   
    //1.2.2计算签名值，与写入文件进行比对
    //stpuk_config();
    stpuk_config_security();
 
    iRet = RSAPublicDecrypt(output,                 /* output block */
                            &lenth,                 /* length of output block */
                            g_stAppStr.signVal,          /* input block */
                            256,   /* length of input block */
                            &g_stpuk);     /* RSA public key */   
    
	sysLOG(LOAD_LOG_LEVEL_2, "app file rsa puk decrypt: lenth %d,iret=%d\r\n", lenth, iRet); 

    if(0 == memcmp(&output[lenth-SHA256_RESULT_LENGTH], hashVal, SHA256_RESULT_LENGTH))
    {
    
		sysLOG(LOAD_LOG_LEVEL_2, "<SUCC> sign val succ\r\n");
        iRet = TRUE;
		goto exit;
    }
    else
    {
    
		sysLOG(LOAD_LOG_LEVEL_2, "<ERR> sign val err\r\n");
        iRet = FALSE;
		goto exit;
    }

exit:

	if(g_readbuf != NULL)
		free(g_readbuf);
	
	return iRet;
  
}


/*
*Function:		hal_ldUpdateSignApp
*Description:	update APP(带签名) 接口
*Input:			*ldappname:APP文件名; offset:APP在文件中的偏移量; *sign_name:签名文件名
*Output:		NULL
*Hardware:
*Return:		<0:失败；0:成功
*Others:
*/
int32 hal_ldUpdateSignApp(int8 *ldappname, uint32 offset, int8 *sign_name)
{
    int iRet;
    BOOL bl_ret;
	int applentmp = 0;
	char *rP = NULL;
	char *headinfo = NULL;

	headinfo = malloc(1024);
	if(headinfo == NULL)
	{
		iRet = -1;
		goto exit;
	}

	memset(headinfo, 0, 1024);
	sysLOG(LOAD_LOG_LEVEL_4, "ldappname:%s,offset:%d,signname:%s\r\n",ldappname,offset,sign_name);
	
    //1.判断签名文件是否存在
    if((1 == g_stAppStr.g_isSignExist)||(sign_name != NULL))
    {
        g_stAppStr.g_isSignExist = 0;
		
		sysLOG(LOAD_LOG_LEVEL_4, "g_stAppStr.g_isSignExist\r\n");
        //1.1签名文件存在验证签名区摘要值是否正确
        iRet = hal_ldVerifySignDigist(sign_name);
		if(sign_name != NULL && hal_fileExist(sign_name) == 1)
		{
			hal_fileReadPro(sign_name, 0, headinfo, 1024);
			hal_fileRemove(sign_name);
		}
		if(hal_fileExist(SIGN_FILE_NAME) == 1)
		{
			hal_fileReadPro(SIGN_FILE_NAME, 0, headinfo, 1024);
			hal_fileRemove(SIGN_FILE_NAME);
		}
		sysLOG(LOAD_LOG_LEVEL_4, "hal_ldVerifySignDigist %d\r\n", iRet);
        if(TRUE != iRet)
        {          
            iRet = -1;
			goto exit;
        }
        if(g_stAppStr.appflag == 0)
		{
	        //1.2签名值正确
	            //1.2.1计算摘要值
	            //1.2.2计算签名值，与写入文件进行比对      
	        iRet = hal_ldVerifyAppDigist(ldappname, offset);
			
			sysLOG(LOAD_LOG_LEVEL_4, "hal_ldVerifyAppDigist %d\r\n", iRet);
	        if(TRUE != iRet)
	        {
	            iRet = -2;
				goto exit;
	        }
			applentmp = hal_fileGetFileSize(ldappname);
			applentmp -= offset;
			if(applentmp <= 0)
			{
				sysLOG(LOAD_LOG_LEVEL_4, "applentmp=%d\r\n", applentmp);
				iRet = -4;
				goto exit;
			}

#if APP_OLDSIGN_EN
			//_nop_();
#else
			iRet = -3;
			goto exit;
#endif

		}
		else if(g_stAppStr.appflag == 1)
		{	

			applentmp = hal_fileGetFileSize(ldappname);
			if(applentmp <= 0)
			{
				sysLOG(LOAD_LOG_LEVEL_4, "applentmp=%d\r\n", applentmp);
				iRet = -5;
				goto exit;
			}
			applentmp += 1024;//头部1K信息被单独存放，要取出来一起校验
			rP = malloc(applentmp);
			if(rP == NULL)
			{
				sysLOG(LOAD_LOG_LEVEL_4, "malloc err\r\n");
				iRet = -6;
				goto exit;
			}

			memset(rP, 0, applentmp);
			
			memcpy(rP, headinfo, 1024);//读取前1K的binInfo信息
			
			iRet = hal_fileReadPro(ldappname, offset, rP+1024, applentmp-1024);//读取APP部分（如果有签名，尾部的签名部分也读进来）
			if(iRet < 0)
			{
				sysLOG(LOAD_LOG_LEVEL_4, "hal_fileReadPro err, iRet=%d\r\n", iRet);
				goto exit;
			}
		
			
			
			iRet = sysAppSignVerify(rP);
		    if(iRet != 0)
		    {
				sysLOG(LOAD_LOG_LEVEL_4, "sysAppSignVerify err, iRet=%d\r\n", iRet);
				goto exit;
			}
			applentmp = g_stAppStr.bininfoAA66.appsize;//applentmp=app原生文件大小

#if APP_NEWSIGN_EN
			//_nop_();
#else
			iRet = -3;
			goto exit;
#endif

		}
    }
	else
	{
		applentmp = hal_fileGetFileSize(ldappname);
		applentmp -= offset;
		if(applentmp <= 0)
		{
			sysLOG(LOAD_LOG_LEVEL_4, "applentmp=%d\r\n", applentmp);
			iRet = -4;
			goto exit;
		}

#if APP_NOSIGN_EN
		//_nop_();
#else
		iRet = -3;//不带签名禁止升级应用！
		goto exit;
#endif

	}

    //2.验证文件MD5是否正确,校验成功会进行更新
   	bl_ret = hal_ldUpdateApp(ldappname, offset, applentmp);

    iRet = bl_ret;

exit:

	if(rP != NULL)
		free(rP);

	if(headinfo != NULL)
		free(headinfo);

	return iRet;
    
}


/*
*Function:		hal_ldLoadFileStrFormat
*Description:	初始化结构体LOADFILE_S
*Input:			*loadfile_sP:LOADFILE_S结构体
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_ldLoadFileStrFormat(LOADFILE_S *loadfile_sP)
{
	loadfile_sP->sizeFile = 0;
	loadfile_sP->sizeFile_recvd = 0;
	loadfile_sP->iPackNo = 0;
	loadfile_sP->paramtypeFile = 0;
	loadfile_sP->numFile = 0;
	memset(loadfile_sP->nameFile, 0, sizeof(loadfile_sP->nameFile));
	memset(loadfile_sP->nameFile_recvd, 0, sizeof(loadfile_sP->nameFile_recvd));
	loadfile_sP->typefile = 0;
	loadfile_sP->fontwriteP = 0;
	memset(loadfile_sP->tmpnameFile, 0, sizeof(loadfile_sP->tmpnameFile));

}

int hal_GetTmpFileName(char *filename, char *tmpfilename)
{
	int iRet;
	char *rP = NULL;
	rP = strrchr(filename,'/');
	if(rP == NULL)
	{
		sprintf(tmpfilename, "/%s", FILE_NAME_TMP);
	}
	else
	{
		memcpy(tmpfilename, filename, rP-filename+1);
		memcpy(tmpfilename+(rP-filename+1), FILE_NAME_TMP, strlen(FILE_NAME_TMP));
	}

	sysLOG(LOAD_LOG_LEVEL_2, "filename:%s, tmpfilename:%s\r\n", filename, tmpfilename);

	return strlen(tmpfilename);
}


/*
*Function:		hal_ldEraseFontFlash
*Description:	擦除flash中的字库
*Input:			startaddr:起始地址;offset:偏移量;len:要擦除的长度
*Output:		NULL
*Hardware:
*Return:		0-success;other-failed
*Others:
*/
static int hal_ldEraseFontFlash(uint32_t startaddr, uint32 offset, uint32 len)
{
	int iRet;
	uint32 quotient = 0, remainder = 0;
	uint32 neederase_sectornum = 0;//需要擦除的页数
	
	sysLOG(LOAD_LOG_LEVEL_4, "-----------startaddr=%d,offset=%d,len=%d\r\n", startaddr, offset, len);

	quotient = offset/4096;//已写完毕的页数
	remainder = offset%4096;//本页中已写完的字节数
	

	if(len > (4096-remainder))//此页中剩余空间已不足写入
	{
		neederase_sectornum = (len-(4096-remainder))/4096;
		if(0 != (len-(4096-remainder))%4096)
			neederase_sectornum += 1;
	}
	

	sysLOG(LOAD_LOG_LEVEL_4, "remainder=%d,quotient=%d,neederase_sectornum=%d\r\n", remainder, quotient, neederase_sectornum);

	if(0 == remainder)
	{
		iRet = hal_flashSectorErase(startaddr+quotient*4096);
		sysLOG(LOAD_LOG_LEVEL_4, "ERASE1 startaddr+quotient*4096=%d\r\n", startaddr+quotient*4096);

	}
	
	if(0 != neederase_sectornum)
	{
		for(uint32 i=0; i<neederase_sectornum; i++)
		{
			iRet = hal_flashSectorErase(startaddr+quotient*4096+(i+1)*4096);
			sysLOG(LOAD_LOG_LEVEL_4, "ERASE2 startaddr+quotient*4096+(i+1)*4096=%d\r\n", startaddr+quotient*4096+(i+1)*4096);
		}
	}
	
	return 0;
	
}


/*
*Function:		hal_ldWriteGBFont
*Description:	写GB2312字库
*Input:			*data:要写入的数据指针;offeset:文件中的偏移量;len:要写入的数据长度
*Output:		NULL
*Hardware:
*Return:		>=0:实际写入的长度;<0:失败
*Others:
*/
static int hal_ldWriteGBFont(int8 *data, int offset, int len)
{
	int iRet = -1;
	static char *fontPtr = NULL;
	
	if(offset == 0)//第一帧
	{
		if(*(data+8) != FONTVERSIONGB2312)
			return -2;
		
		if(*(data+9) == 1)
		{
			switch(*(data+0x11))
			{
				case 12:
					fontPtr = GB2312FONT12X12FILENAME;
				break;
				case 16:
					fontPtr = GB2312FONT16X16FILENAME;
				break;
				case 24:
					fontPtr = GB2312FONT24X24FILENAME;
				break;
				default:
					
				break;
			}
		}
		else
		{
			fontPtr = GB2312FONTFILENAME;
		}

		hal_fileMkdir(GB2312FONTDIR);
		
		if(fontPtr != NULL)
		{
			iRet = hal_fileRemove(fontPtr);
		}
	}

	if(fontPtr != NULL)
	{
		iRet = hal_fileWritePro(fontPtr, offset, data, len);
	}

	return iRet;
	
}


/*
*Function:		hal_ldWriteUniFont
*Description:	写Unicode字库
*Input:			*data:要写入的数据指针;offeset:要写入的数据在flash中的偏移量;len:要写入的数据长度
*Output:		NULL
*Hardware:
*Return:		>=0:实际写入的长度;<0:失败
*Others:
*/
static int hal_ldWriteUniFont(int8 *data, int offset, int len)
{
	int iRet = -1;
	static char *fontPtr = NULL;
	
	if(offset == 0)//第一帧
	{
		if(*(data+8) != FONTVERSIONUNICODE)
			return -2;
	}

	hal_ldEraseFontFlash(EXT_FLASH_FONT_START, offset, len);
	iRet = hal_flashWrite(data, EXT_FLASH_FONT_START+offset, len);

	return iRet;
	
}


/*
*Function:		hal_ldCMDLoadFile
*Description:	通过指令下载文件接口
*Input:			*loadfile_sP:LOADFILE_S指针；*data:传输下来的数据指针；dataLen:数据长度
*Output:		NULL
*Hardware:
*Return:		<0:失败；>0:下载成功的字节数
*Others:
*/
static int hal_ldCMDLoadFile(LOADFILE_S *loadfile_sP, int8 *data, uint32 dataLen)
{
	int iRet;
	int32 fd;
	int8 fileNameTmp[64];

	if(loadfile_sP->typefile == FILETYPE_PARAMFILE)
	{
		memset(fileNameTmp, 0, sizeof(fileNameTmp));

		if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_FILE)//带绝对路径的文件名,直接写入本文件中即可
		{
			strcpy(loadfile_sP->tmpnameFile, loadfile_sP->nameFile_recvd);
			strcpy(fileNameTmp, loadfile_sP->tmpnameFile);
		}
		else
		{
			hal_GetTmpFileName(CFG_FILE_NAME, loadfile_sP->tmpnameFile);
			strcpy(fileNameTmp, loadfile_sP->tmpnameFile);
		}

		if(loadfile_sP->iPackNo == 0)//first
		{
			if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_CFGFILE)
			{
				memcpy(loadfile_sP->nameFile, CFG_FILE_NAME, sizeof(CFG_FILE_NAME));
			}
			else if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_APPFILE)//应用自定义文件，默认放到/ext/app/data中
			{
				memcpy(loadfile_sP->nameFile, APP_DIR, sizeof(APP_DIR));
				memcpy(&(loadfile_sP->nameFile[strlen(loadfile_sP->nameFile)]), loadfile_sP->nameFile_recvd, strlen(loadfile_sP->nameFile_recvd));
			}
			else if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_AUDIOFILE)//音频文件
			{
				memcpy(loadfile_sP->nameFile, AUDIO_NAME_PLAY, sizeof(AUDIO_NAME_PLAY));
			}
			else if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_FILE)//带绝对路径的文件名,直接写入本文件中即可
			{
				memcpy(loadfile_sP->nameFile, loadfile_sP->nameFile_recvd, strlen(loadfile_sP->nameFile_recvd));
				
			}
			else
			{
				return -1;
			}
		
			sysLOG(LOAD_LOG_LEVEL_2, "%s, loadfile_sP->nameFile:%s\r\n", fileNameTmp, loadfile_sP->nameFile);
			iRet = hal_fileExist(fileNameTmp);
			if(iRet == 1)
			{
				hal_fileRemove(fileNameTmp);
			}
		}
		
		loadfile_sP->iPackNo = loadfile_sP->iPackNo + 1;
		sysLOG(LOAD_LOG_LEVEL_4, "%s, loadfile_sP->iPackNo:%d\r\n", fileNameTmp, loadfile_sP->iPackNo);
		fd = hal_fileOpen(fileNameTmp, O_CREAT | O_RDWR);
		if(fd < 0)
		{
			return fd;
		}

		iRet = hal_fileSeek(fd, 0, SEEK_END);
		if(iRet < 0)
		{
			hal_fileClose(fd);
			return iRet;
		}

		iRet = hal_fileWrite(fd, data, dataLen);
		if(iRet != (dataLen))
		{
			hal_fileClose(fd);
			return -4;
		}

		iRet = hal_fileClose(fd);
		if(iRet != 0)
		{
			
			return -5;
		}

		return dataLen;
	}
	else if(loadfile_sP->typefile == FILETYPE_APP)
	{
		iRet = hal_ldDownloadApp(&g_stAppStr, data, dataLen);
		return iRet;
	}
	else if(loadfile_sP->typefile == FILETYPE_FONT)
	{
		if(FONTFS == hal_fontGetFontType())
		{
			iRet = hal_ldWriteGBFont(data, loadfile_sP->fontwriteP, dataLen);
			loadfile_sP->fontwriteP += dataLen;
		}
//		else
//		{
//			iRet = hal_ldWriteUniFont(data, loadfile_sP->fontwriteP, dataLen);
//			loadfile_sP->fontwriteP += dataLen;
//		}
		return iRet;
	}
	
	
	
}


/*
*Function:		hal_ldCMDWriteFile
*Description:	通过指令写入到指定文件接口
*Input:			*loadfile_sP:LOADFILE_S指针
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功
*Others:
*/
static int hal_ldCMDWriteFile(LOADFILE_S *loadfile_sP)
{
	int iRet;
	int8 *rP = NULL;
	int lentmp = 0;

	if(loadfile_sP->typefile == FILETYPE_PARAMFILE)
	{
		if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_FILE)//带绝对路径的文件名,已经直接写入本文件中
		{
			return 0;
		}
		
		iRet = hal_fileExist(loadfile_sP->nameFile);
		if(iRet == 1)
		{
			hal_fileRemove(loadfile_sP->nameFile);
		}
		
		sysLOG(LOAD_LOG_LEVEL_4, "loadfile_sP->tmpnameFile:%s, loadfile_sP->nameFile:%s\r\n", loadfile_sP->tmpnameFile, loadfile_sP->nameFile);
		lentmp = hal_fileGetFileSize(FILE_NAME_TMP);
		rP = malloc(lentmp+1);
		if(rP == NULL)
		{
			return -3;
		}
		memset(rP, 0, lentmp+1);
		iRet = hal_fileReadPro(FILE_NAME_TMP, 0, rP, lentmp);
		if(iRet != lentmp)
		{
			free(rP);
			sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, hal_fileReadPro, iRet:%d\r\n", iRet);
			return -11;
		}
		
		iRet = hal_fileRemove(FILE_NAME_TMP);
		if(iRet < 0)
		{
			free(rP);
			sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, hal_fileRemove, iRet:%d\r\n", iRet);
			return iRet;
		}
		
		iRet = hal_fileWritePro(loadfile_sP->nameFile, 0, rP, lentmp);
		if(iRet < 0)
		{
			free(rP);
			sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, hal_fileWritePro, iRet:%d\r\n", iRet);
			return iRet;
		}
		free(rP);

		if(loadfile_sP->paramtypeFile == FILETYPE_PARAMTYPE_CFGFILE)
		{
			iRet = hal_cfgReadCfgFile(loadfile_sP->nameFile);
		}
		
		return iRet;
	}
	else if(loadfile_sP->typefile == FILETYPE_APP)
	{
		iRet = hal_ldUpdateSignApp(g_stAppStr.g_DesAppName, 0, NULL);
		return iRet;
	}
	else if(loadfile_sP->typefile == FILETYPE_FONT)
	{
		iRet = 0;
		return iRet;
	}
	
	
}


/*
*Function:		hal_ldCMDLoadPUK
*Description:	通过指令下载PUK
*Input:			*data:传输下来的数据指针；dataLen:数据长度
*Output:		NULL
*Hardware:
*Return:		<0:失败；0:成功
*Others:
*/
static int hal_ldCMDLoadPUK(int8 *data, uint32 dataLen)
{
	int iRet = 0;
	int8 fileNameTmp[64];

	memset(fileNameTmp, 0, sizeof(fileNameTmp));

	hal_GetTmpFileName(g_stLoadfileStr.nameFile_recvd, g_stLoadfileStr.tmpnameFile);
	strcpy(fileNameTmp, g_stLoadfileStr.tmpnameFile);
	
	iRet = hal_fileWritePro(fileNameTmp, 0, data, dataLen);
	iRet = WritePuk(fileNameTmp);
	hal_fileRemove(fileNameTmp);
	return iRet;
	
}


/*
*Function:		hal_ldCMDSendWiFiAT
*Description:	发送WiFi的AT指令接口
*Input:			*cmd:指令指针; cmdlen:指令长度;
*Output:		*buf:接收指针
*Hardware:
*Return:		<0:失败；>0：收到的数据长度
*Others:
*/
static int32 hal_ldCMDSendWiFiAT(char *cmd, uint32 cmdlen, char *buf)
{
	int32 ret;
	
	Wifi_wkup(TRUE);
	sysDelayMs(10);
	if(0 == memcmp(cmd, "AT+CIUPDATE", 11))
	{
		memset(buf, 0x00, sizeof(buf));
		ret = ESP_SendAndWaitCmd((char *)cmd, cmdlen, (char *)buf, 256, 60*10*1000, 1, "\r\nready\r\n");

	}
	else if(0 == memcmp(cmd, "AT+RESTORE", 10))//清除wifi热点信息，芯片会重启收不到返回，此处反0只代表指令已发送，后续通过查保存热点来确认是否清除成功
	{
		memset(buf, 0x00, sizeof(buf));
		ret = ESP_SendAndWaitCmd((char *)cmd, cmdlen, (char *)buf, 256, 500, 3, "\r\nOK\r\n");
		sysLOG(LOAD_LOG_LEVEL_4, "ESP_SendAndWaitCmd, the ret=%d, buf:%s\r\n", ret, buf);
		ret = 0;
	}
	else
	{
		memset(buf, 0x00, sizeof(buf));
		ret = ESP_SendAndWaitCmd((char *)cmd, cmdlen, (char *)buf, 256, 500, 3, "\r\nOK\r\n");
		sysLOG(LOAD_LOG_LEVEL_4, "ESP_SendAndWaitCmd, the ret=%d, buf:%s\r\n", ret, buf);
		
	}
	Wifi_wkup(FALSE);
	return ret;
	
}


/*
*Function:		hal_ldLDPkgFormat
*Description:	LD_PKG格式化
*Input:			*ld_pkg:LD_PKG指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_ldLDPkgFormat(LD_PKG *ld_pkg)
{
	ld_pkg->start_code = 0;
	ld_pkg->pkg_index = 0;
	ld_pkg->cmd_type = 0;
	ld_pkg->cmd = 0;
	ld_pkg->datalen = 0;
	ld_pkg->lrc = 0;
	memset(ld_pkg->data, 0, sizeof(ld_pkg->data));
}

void clear_LoadStep()
{	
	g_ui8LoadStep = 0;
}

/*
*Function:		hal_ldRecvDataAnalysis
*Description:	读数据包并解析数据到LD_PKG中
*Input:			*data:需要解析的数据指针；datalen:数据长度
*Output:		*ld_pkg_recv:LD_PKG接收指针,*usedlen:已解析完的长度
*Hardware:
*Return:		<0:失败；1:成功;
*Others:
*/
int32 hal_ldRecvDataAnalysis(uint8 *data, int16 datalen, LD_PKG *ld_pkg_recv, int16 *usedlen)
{
	int32 iRet = -1;
	uint8 *rPDat = NULL;
	uint8 *rP = NULL;
	int16 datalentmp = 0;

	rPDat = malloc(datalen+1);
	if(rPDat == NULL)
	{
		sysLOG(LOAD_LOG_LEVEL_3, "<ERR> malloc failed.");
		goto end;
	}

	memset(rPDat, 0, datalen+1);
	CBuffCopy(&g_stUsbBuffStr, rPDat, datalen);
	//sysLOG(LOAD_LOG_LEVEL_1, "g_stUsbBuffStr.read_P=%d, g_stUsbBuffStr.write_P=%d", g_stUsbBuffStr.read_P, g_stUsbBuffStr.write_P);

	rP = rPDat;
	datalentmp = datalen;
	*usedlen = 0;

	
	
START:
	
	if(datalentmp <= 0)
	{
		goto end;
	}
	
	if(g_ui8LoadStep == LD_POSSHAKEHAND_SUCC)//如果握手成功
	{

		iRet = memcmp(rP, "T", strlen("T"));
		if(iRet == 0)
		{
			g_ui8LoadStep = LD_POSTYPEREAD;
			sysLOG(LOAD_LOG_LEVEL_5, "T\r\n");
			datalentmp -=1;
			iRet = 1;
			goto end;
		}
		
	}

	iRet = memcmp(rP, "Q", strlen("Q"));//握手
	if(iRet == 0)
	{
		g_ui8ShakehandFlag+=1;
		if(g_ui8ShakehandFlag>100)g_ui8ShakehandFlag = 100;
		g_ui8LoadStep = LD_POSSHAKEHAND;
		sysLOG(LOAD_LOG_LEVEL_5, "Q\r\n");
		datalentmp -=1;
		iRet = 1;
		goto end;
	}
	else
	{
		g_ui8ShakehandFlag = 0;
	}

	sysLOG(LOAD_LOG_LEVEL_5, "0x%x\r\n", *rP);
	iRet = COMMERR_LOAD_UNKNOWNCMD;

	
	if(*rP != 0x02)
	{
		rP +=1;
		datalentmp -=1;
		if(datalentmp <= 0)
		{
			goto end;
		}
		goto START;
	}
	
	if(g_ui8LoadStep == LD_POSTYPEREAD_SUCC || g_ui8LoadStep == LD_POSSHAKEHAND_SUCC)//读机器码成功或握手成功
	{
		if(datalentmp < 7)
		{
			iRet = COMMERR_LOAD_INCOMPLETEDATA;//数据长度不够
			goto end;
		}
		
		hal_ldLDPkgFormat(ld_pkg_recv);
		
		ld_pkg_recv->start_code = *rP;
		ld_pkg_recv->pkg_index = *(rP+1);
		ld_pkg_recv->cmd_type = *(rP+2);
		ld_pkg_recv->cmd = *(rP+3);
		ld_pkg_recv->datalen += (*(rP+4))<<8;
		ld_pkg_recv->datalen += (*(rP+5));
		
		if(datalentmp < (ld_pkg_recv->datalen+7))
		{
			iRet = COMMERR_LOAD_INCOMPLETEDATA;//数据长度不够
			goto end;
		}
		memcpy(ld_pkg_recv->data, rP+6, ld_pkg_recv->datalen);
		ld_pkg_recv->lrc = *(rP+6+ld_pkg_recv->datalen);
		iRet = CalLRC(rP+1, 5+ld_pkg_recv->datalen);
		datalentmp -= (ld_pkg_recv->datalen+7);
		sysLOG(LOAD_LOG_LEVEL_4, "CalLRC iRet=%d,ld_pkg_recv->lrc=%d,ld_pkg_recv->datalen=%d\r\n", iRet, ld_pkg_recv->lrc, ld_pkg_recv->datalen);
		if((iRet&0xFF) == ld_pkg_recv->lrc)
		{
		
			iRet = 1;
		}
		else
		{
			iRet = COMMERR_LOAD_LRCERR;
			goto end;
		}
		sysLOG(LOAD_LOG_LEVEL_5, "iRet=%d\r\n", iRet);
	}
	
end:

	*usedlen = datalen - datalentmp;
	//sysLOG(LOAD_LOG_LEVEL_1, "the iRet=%d, datalen=%d, *usedlen=%d\r\n", iRet, datalen, *usedlen);
	if(rPDat != NULL)
	{
		free(rPDat);
	}
	return iRet;
	
}

/*
*Function:		hal_ldProcessHandle
*Description:	处理下载APP等协议数据的解析句柄
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
void hal_ldProcessHandle(void)
{
	int32 iRet;
	int16 usedlen = 0;	

	iRet = hal_ldRecvDataAnalysis(g_stUsbBuffStr.buff, g_stUsbBuffStr.count, &g_stRecvPkg, &usedlen);		
	CBuffClean(&g_stUsbBuffStr, usedlen);
	if(iRet == 1)
	{
		hal_ldRecvDataHandle(&g_stRecvPkg, &g_stSendPkg);
	}

}


/*
*Function:		hal_ldSendDataPack
*Description:	发送包赋值
*Input:			start_code:起始字符；pkg_index:包序；cmd_type:指令类别
*				cmd:指令；datalen:数据域长度；re_code:返回码；*data:数据指针
*Output:		*ld_pkg_send:LD_PKG指针；
*Hardware:
*Return:		NULL
*Others:
*/
static void hal_ldSendDataPack(LD_PKG *ld_pkg_send, uint8 start_code, uint8 pkg_index,
							uint8 cmd_type, uint8 cmd, uint16 datalen, uint8 re_code, int8 *data)
{
	ld_pkg_send->start_code = start_code;
	ld_pkg_send->pkg_index = pkg_index;
	ld_pkg_send->cmd_type = cmd_type;
	ld_pkg_send->cmd = cmd;
	ld_pkg_send->datalen = datalen;
	ld_pkg_send->re_code = re_code;
	if(data != NULL)
	{
		memcpy(ld_pkg_send->data, data, datalen-1);
	}
}


/*
*Function:		hal_ldSendData
*Description:	将ld_pkg_send内容打包发送
*Input:			*ld_pkg_send:LD_PKG指针
*Output:		NULL
*Hardware:
*Return:		<0:发送失败；>0:发送数据长度
*Others:
*/
int32 hal_ldSendData(LD_PKG *ld_pkg_send)
{
	int32 iRet = -1;
	uint8 *rP = NULL;
	uint32 lentmp;

	lentmp = 7+ld_pkg_send->datalen;
	rP = malloc(lentmp+1);
	if(rP == NULL)
	{
		return iRet;
	}

	memset(rP, 0, lentmp+1);
	*rP = ld_pkg_send->start_code;
	*(rP+1) = ld_pkg_send->pkg_index;
	*(rP+2) = ld_pkg_send->cmd_type;
	*(rP+3) = ld_pkg_send->cmd;
	*(rP+4) = (ld_pkg_send->datalen&0xFF00)>>8;
	*(rP+5) = (ld_pkg_send->datalen&0x00FF);
	
	*(rP+6) = ld_pkg_send->re_code;
	memcpy(rP+7, ld_pkg_send->data, ld_pkg_send->datalen-1);
	*(rP+6+ld_pkg_send->datalen) = CalLRC(rP+1, 5+ld_pkg_send->datalen);

	hal_portUsbSend(rP, lentmp);

	free(rP);

	return lentmp;
	
}


/*
*Function:		hal_ldRecvDataHandle
*Description:	针对解析好的数据做出相应的处理并回复，不可识别的数据不做处理也不做回应
*Input:			*ld_pkg_recv:LD_PKG接收指针
*Output:		*ld_pkg_send:LD_PKG发送指针
*Hardware:
*Return:		<0:失败；>0:发送数据的长度
*Others:
*/
int32 hal_ldRecvDataHandle(LD_PKG *ld_pkg_recv, LD_PKG *ld_pkg_send)
{
	int32 iRet = -1;
	int8 *rP = NULL;
	uint8 *retP = NULL;
	CellInfoAll_Str cellinfoall_info;
	uint8 caip[64]= {0};//注网IP
	int iLen = sizeof(caip);
	
	sysLOG(LOAD_LOG_LEVEL_4, "g_ui8LoadStep=%x\r\n", g_ui8LoadStep);

	if(g_ui8LoadStep == LD_POSSHAKEHAND)//握手
	{
		
		rP = malloc(256);
		if(rP == NULL)return iRet;
		memset(rP, 0, 256);
		sprintf(rP, "R");
		hal_portUsbSend(rP, strlen(rP));
		iRet = strlen(rP);
		free(rP);
		g_ui8LoadAppStart = 0;
		ld_pkg_send->pkg_index = 0xFF;//初始化0xFF
		g_ui8LoadStep = LD_POSSHAKEHAND_SUCC;
		sysLOG(LOAD_LOG_LEVEL_5, "LD_POSSHAKEHAND_SUCC\r\n");
		return iRet;
	}
	else if(g_ui8LoadStep == LD_POSTYPEREAD)//读机型码
	{
		
		rP = malloc(256);
		if(rP == NULL)return iRet;
		memset(rP, 0, 256);
		*rP = LD_POSTYPE_LC610N;
		hal_portUsbSend(rP, strlen(rP));
		iRet = strlen(rP);
		free(rP);
		g_ui8LoadAppStart = 0;
		ld_pkg_send->pkg_index = 0xFF;//初始化0xFF
		g_ui8LoadStep = LD_POSTYPEREAD_SUCC;
		sysLOG(LOAD_LOG_LEVEL_5, "LD_POSTYPEREAD_SUCC\r\n");
		return iRet;
	}
	else if(g_ui8LoadStep == LD_POSTYPEREAD_SUCC || g_ui8LoadStep == LD_POSSHAKEHAND_SUCC)//正式走协议部分
	{
	
		if(ld_pkg_recv->cmd_type == 0x80)
		{
			if(ld_pkg_send->pkg_index == ld_pkg_recv->pkg_index)//包序相同则重发上一条指令
			{
				goto repeat_send;
				
			}
			
			hal_ldLDPkgFormat(ld_pkg_send);
			
			switch(ld_pkg_recv->cmd)
			{
				case LD_CMDCODE_TEST:
					hal_pmPwrOFF();
					//rP = malloc(2);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 2);
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_TEST, 1, LD_OK, NULL);
					//free(rP);
					goto normal_send;
					
				break;
				case LD_CMDCODE_NREBOOT:
					
					
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_NREBOOT, 1, LD_OK, NULL);
					
					iRet = hal_ldSendData(ld_pkg_send);	
					sysLOG(LOAD_LOG_LEVEL_4, "<SUCC> normal_send, the iRet=%d\r\n", iRet);
					
				break;
				case LD_CMDCODE_REBOOT:
					
					//rP = malloc(2);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 2);
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_REBOOT, 1, LD_OK, NULL);
					//free(rP);
					iRet = hal_ldSendData(ld_pkg_send);	
					sysLOG(LOAD_LOG_LEVEL_4, "<SUCC> normal_send, the iRet=%d\r\n", iRet);
					sysDelayMs(500);
					fibo_set_shutdown_mode(OSI_SHUTDOWN_RESET);
				break;
				case LD_CMDCODE_READSN:
					sysLOG(LOAD_LOG_LEVEL_5, " LD_CMDCODE_READSN, recv datalen=%d\r\n", ld_pkg_recv->datalen);
					sysLOG(LOAD_LOG_LEVEL_5, " LD_CMDCODE_READSN, recv data=%d\r\n", ld_pkg_recv->data);
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					//iRet = hal_nvReadSN(rP);
					if(strstr(ld_pkg_recv->data, "TUSN"))
					{
						iRet = sysReadTUSN_lib(0x0055FFAA,rP);
					}
					else
					{
						iRet = sysReadSn_lib(0x0055FFAA,rP);
					}	
					if(iRet <= 0)
					{		
					
						sysLOG(LOAD_LOG_LEVEL_2, "<ERR> LD_CMDCODE_READSN, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READSN, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READSN, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WRITESN:
					
					//rP = malloc(64);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 64);
					sysLOG(LOAD_LOG_LEVEL_5, " LD_CMDCODE_WRITESN, recv datalen=%d\r\n", ld_pkg_recv->datalen);
					if(ld_pkg_recv->datalen > NV_SN_LEN-3)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITESN, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						//iRet = hal_snWriteSN(ld_pkg_recv->data);
						sysLOG(LOAD_LOG_LEVEL_5, "LD_CMDCODE_WRITESN, data=%s\r\n", ld_pkg_recv->data);
						iRet = sysWriteTUSN_lib(ld_pkg_recv->data, ld_pkg_recv->datalen);
						if(iRet > 0)//写入成功
						{				
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITESN, 1, LD_OK, NULL);
						}
						else
						{	
							
							sysLOG(LOAD_LOG_LEVEL_2, "<ERR> LD_CMDCODE_WRITESN, the iRet=%d\r\n", iRet);
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITESN, 1, LDERR_GENERIC, NULL);
						}
					}
					
					
					//free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_CLOSEPORT:
					fibo_set_usbmode(34);//关闭USB0-5
					//fibo_set_usbmode(31);//打开USN 0-5
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_CLOSEPORT, 1, LD_OK, NULL);
					goto normal_send;
				break;
				case LD_CMDCODE_READFTTYPE:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					iRet = hal_nvReadFontType(rP);
					if(iRet <= 0)
					{		
						
						sysLOG(LOAD_LOG_LEVEL_2, "<ERR> hal_nvReadFontType, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READFTTYPE, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READFTTYPE, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WRITEFTTYPE:
					
					//rP = malloc(64);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 64);
					if(ld_pkg_recv->datalen > NV_FTTYPE_LEN-3)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEFTTYPE, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						iRet = hal_nvWriteFontType(ld_pkg_recv->data, ld_pkg_recv->datalen);
						if(iRet > 0)//写入成功
						{				
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEFTTYPE, 1, LD_OK, NULL);
						}
						else
						{	
							
							sysLOG(LOAD_LOG_LEVEL_2, "<ERR> hal_nvWriteFontType, the iRet=%d\r\n", iRet);
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEFTTYPE, 1, LDERR_GENERIC, NULL);
						}
					}
					
					
					//free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_READTERM:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					iRet = hal_nvReadTerm(rP);
					if(iRet <= 0)
					{		
						
						sysLOG(LOAD_LOG_LEVEL_2, "<ERR> hal_nvReadTerm, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READTERM, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READTERM, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WRITETERM:
					
					//rP = malloc(64);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 64);
					if(ld_pkg_recv->datalen > NV_TERM_LEN-3)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITETERM, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						iRet = hal_nvWriteTerm(ld_pkg_recv->data, ld_pkg_recv->datalen);
						if(iRet > 0)//写入成功
						{				
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITETERM, 1, LD_OK, NULL);
						}
						else
						{	
							
							sysLOG(LOAD_LOG_LEVEL_2, "<ERR> hal_nvWriteHwVersion, the iRet=%d\r\n", iRet);
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITETERM, 1, LDERR_GENERIC, NULL);
						}
					}
					
					
					//free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_READHWVER:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					iRet = hal_nvReadHwVersion(rP);
					if(iRet <= 0)
					{		
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvReadHwVersion, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READHWVER, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READHWVER, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WRITEHWVER:
					
					//rP = malloc(64);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 64);
					if(ld_pkg_recv->datalen > NV_HW_LEN-3)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEHWVER, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						iRet = hal_nvWriteHwVersion(ld_pkg_recv->data, ld_pkg_recv->datalen);
						if(iRet > 0)//写入成功
						{				
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEHWVER, 1, LD_OK, NULL);
						}
						else
						{	
							
							sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvWriteHwVersion, the iRet=%d\r\n", iRet);
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEHWVER, 1, LDERR_GENERIC, NULL);
						}
					}
					
					
					//free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_READCID:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					iRet = hal_nvReadCustomerID(rP);
					if(iRet <= 0)
					{		
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvReadCustomerID, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READCID, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READCID, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WRITECID:
					sysLOG(LOAD_LOG_LEVEL_5, " LD_CMDCODE_WRITECID, recv datalen=%d\r\n", ld_pkg_recv->datalen);
					sysLOG(LOAD_LOG_LEVEL_5, " LD_CMDCODE_WRITECID, recv data=%d\r\n", ld_pkg_recv->data);
					//rP = malloc(64);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 64);
					if(ld_pkg_recv->datalen > NV_CID_LEN-3)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITECID, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						iRet = hal_nvWriteCustomerID(ld_pkg_recv->data, ld_pkg_recv->datalen);
						if(iRet > 0)//写入成功
						{				
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITECID, 1, LD_OK, NULL);
						}
						else
						{	
							
							sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvWriteCustomerID, the iRet=%d\r\n", iRet);
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITECID, 1, LDERR_GENERIC, NULL);
						}
					}
					
					
					//free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_TERMINALNAME:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					iRet = sysGetTermTypeImperInfo_lib(rP);
					if(iRet < 0)
					{		
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> sysGetTermTypeImperInfo_lib, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_TERMINALNAME, 1, LDERR_GENERIC, NULL);
					}
					else
					{

						iRet = strlen(rP);
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_TERMINALNAME, iRet+1, LD_OK, rP);
					}
					sysLOG(LOAD_LOG_LEVEL_4, "<SUCC> sysGetTermTypeImperInfo_lib, the iRet=%d\r\n", iRet);
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_LOGLEVEL:
					
					//rP = malloc(2);
					//if(rP == NULL)return iRet;
					//memset(rP, 0, 2);
					if(ld_pkg_recv->datalen != 1 || ld_pkg_recv->data[0] < 0 || ld_pkg_recv->data[0] >10)
					{		
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_LOGLEVEL, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						g_ui8LogLevel = ld_pkg_recv->data[0];
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_LOGLEVEL, 1, LD_OK, NULL);
					}
					
					//free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_READAPPVER:
					
					rP = malloc(1024);
					if(rP == NULL)return iRet;
					memset(rP, 0, 1024);
					sprintf(rP, "\r\n+APPINFO:\r\n");
					hal_sysGetAppInfo(rP+strlen(rP));
						
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READAPPVER, strlen(rP)+1, LD_OK, rP);
					
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_READDRVER:
					
					rP = malloc(256);
					if(rP == NULL)return iRet;
					memset(rP, 0, 256);
					iRet = hal_sysGetHalInfo(rP);
					if(iRet < 0)
					{		
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READDRVER, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READDRVER, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WIFIAT:
					
					rP = malloc(2048);
					if(rP == NULL)return iRet;
					memset(rP, 0, 2048);
					iRet = -1;
					
									
					iRet = hal_ldCMDSendWiFiAT(ld_pkg_recv->data, ld_pkg_recv->datalen, rP);
					
					if(iRet < 0)
					{		
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WIFIAT, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WIFIAT, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_BATMILLIVOLT:
					
					rP = malloc(512);
					if(rP == NULL)return iRet;
					memset(rP, 0, 512);
					if(hal_pmGetChargerValue() >= 0)
					{
						if(hal_pmGetChargerValue() < 2)//未充电
						{
							sprintf(rP, "\r\n+BAT:\r\nCharger:%d\r\nBattery_Voltage:%dmV\r\nQuantity_Bat:%d%%\r\n", g_stBatStr.charger_v, g_stBatStr.bat_value, hal_pmBatGetValue());
						}
						else//充电中
						{
							sprintf(rP, "\r\n+BAT:\r\nCharger:%d\r\nBattery_Voltage:%dmV\r\nQuantity_Bat:\r\n", g_stBatStr.charger_v, g_stBatStr.bat_value);
						}
					}
					else

					{
						sprintf(rP, "\r\n+BAT:\r\nDR_ChargerGetValue() ERROR\r\n");
					}
						
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_BATMILLIVOLT, strlen(rP)+1, LD_OK, rP);
			
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WIFIPWR:
					
					if(ld_pkg_recv->datalen != 1 || ld_pkg_recv->data[0] < 0 || ld_pkg_recv->data[0] >1)
					{		
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WIFIPWR, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						if(ld_pkg_recv->data[0] == 0)
						{
							//下电
							iRet = wifiClose_lib();
							if(iRet < 0)
							{
								hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WIFIPWR, 1, LDERR_GENERIC, NULL);
								goto normal_send;
							}
							
						}
						else if(ld_pkg_recv->data[0] == 1)
						{
							//上电
							iRet = wifiOpen_lib();
							if(iRet < 0)
							{
								hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WIFIPWR, 1, LDERR_GENERIC, NULL);
								goto normal_send;
							}
						}
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WIFIPWR, 1, LD_OK, NULL);
					}
					
					goto normal_send;
				break;
				case LD_CMDCODE_READHWVERSTRING:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					iRet = hal_nvReadHwVersionString(rP);
					if(iRet <= 0)
					{		
					
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvReadHwVersionString, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READHWVERSTRING, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READHWVERSTRING, iRet+1, LD_OK, rP);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_WRITEHWVERSTRING:
					
					iRet = hal_nvWriteHwVersionString(ld_pkg_recv->data);
					if(iRet > 0)//写入成功
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEHWVERSTRING, 1, LD_OK, NULL);
					}
					else

					{	
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvWriteHwVersionString, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEHWVERSTRING, 1, LDERR_GENERIC, NULL);
					}
					
					
					goto normal_send;
				break;
				case LD_CMDCODE_SETTIME:
					
					iRet = 0;//hal_sysSetTime(ld_pkg_recv->data);
					if(iRet == 0)//写入成功
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_SETTIME, 1, LD_OK, NULL);
					}
					else

					{	
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_nvWriteHwVersionString, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_SETTIME, 1, LDERR_GENERIC, NULL);
					}
					
					
					goto normal_send;
				break;
				case LD_CMDCODE_STARTLOADPARAM:

					
					hal_ldLoadFileStrFormat(&g_stLoadfileStr);
					if(ld_pkg_recv->datalen == 0x06)//下载参数文件
					{
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[0]<<24;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[1]<<16;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[2]<<8;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[3];
						
						g_stLoadfileStr.numFile = ld_pkg_recv->data[4];
						g_stLoadfileStr.paramtypeFile = ld_pkg_recv->data[5];
						g_stLoadfileStr.typefile = FILETYPE_PARAMFILE;
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADPARAM, 1, LD_OK, NULL);
					}
					else if(0x06 < ld_pkg_recv->datalen && ld_pkg_recv->datalen <= 0x16*2)//下载应用自定义文件
					{
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[0]<<24;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[1]<<16;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[2]<<8;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[3];
						
						g_stLoadfileStr.numFile = ld_pkg_recv->data[4];
						g_stLoadfileStr.paramtypeFile = ld_pkg_recv->data[5];
						memcpy(g_stLoadfileStr.nameFile_recvd, &(ld_pkg_recv->data[6]), ld_pkg_recv->datalen-6);
						g_stLoadfileStr.typefile = FILETYPE_PARAMFILE;

						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADPARAM, 1, LD_OK, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADPARAM, 1, LDERR_GENERIC, NULL);					
					}
					sysLOG(LOAD_LOG_LEVEL_5, "ld_pkg_recv->data:%x,%x,%x,%x,%x,%x,%x\r\n", ld_pkg_recv->data[0],ld_pkg_recv->data[1],\
					ld_pkg_recv->data[2],ld_pkg_recv->data[3],ld_pkg_recv->data[4],ld_pkg_recv->data[5],ld_pkg_recv->data[6]);
					sysLOG(LOAD_LOG_LEVEL_4, "sizeFile=%d,numFile=%d,paramtypeFile=%d\r\n", g_stLoadfileStr.sizeFile, g_stLoadfileStr.numFile, g_stLoadfileStr.paramtypeFile);
					goto normal_send;	
				break;
				case LD_CMDCODE_STARTLOADAPP:

					
					hal_ldLoadFileStrFormat(&g_stLoadfileStr);
					if(ld_pkg_recv->datalen == 0x04)//下载APP文件
					{
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[0]<<24;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[1]<<16;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[2]<<8;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[3];
						
						g_stLoadfileStr.typefile = FILETYPE_APP;
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADAPP, 1, LD_OK, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADAPP, 1, LDERR_GENERIC, NULL);					
					}
					goto normal_send;	
				break;
				case LD_CMDCODE_STARTLOADFONT:

					
					hal_ldLoadFileStrFormat(&g_stLoadfileStr);
					if(ld_pkg_recv->datalen == 0x04)//下载字库
					{
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[0]<<24;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[1]<<16;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[2]<<8;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[3];
						
						g_stLoadfileStr.typefile = FILETYPE_FONT;

						uint8 sectorpercent=0, sectorlastpercent=0;
						
						if(FONTFS == hal_fontGetFontType())//下载文件系统中的字库方案
						{
							sectorpercent = 100;
							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFONT, 2, 1, &sectorpercent);
							iRet = hal_ldSendData(ld_pkg_send);	
							sysLOG(LOAD_LOG_LEVEL_4, "<SUCC> normal_send, the iRet=%d\r\n", iRet);
						
						}
//						else//下载存储到spiflash中的字库方案
//						{
//#if 0						
//							uint32 sectornum = g_stLoadfileStr.sizeFile%4096;
//							if(sectornum != 0)
//							{
//								sectornum = 1;
//							}
//							sectornum += g_stLoadfileStr.sizeFile/4096;
//
//							uint32 i=0;
//							
//							sysLOG(LOAD_LOG_LEVEL_2, "sectorpercent=%d,sectorlastpercent=%d,sectornum=%d\r\n", sectorpercent, sectorlastpercent, sectornum);
//							for(i=0; i<sectornum; i++)
//							{
//								iRet = hal_flashSectorErase(0+i*4096);
//								sectorpercent = (i+1)*100/sectornum;
//								if(sectorpercent != sectorlastpercent)
//								{
//									//sysLOG(LOAD_LOG_LEVEL_2, "sectorpercent=%d,sectorlastpercent=%d,sectornum=%d\r\n", sectorpercent, sectorlastpercent, sectornum);
//									hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFONT, 2, 1, &sectorpercent);
//									sectorlastpercent = sectorpercent;
//									
//									iRet = hal_ldSendData(ld_pkg_send);	
//									sysLOG(LOAD_LOG_LEVEL_2, "<SUCC> normal_send, the iRet=%d\r\n", iRet);
//									
//								}
//								
//								sysDelayMs(5);
//							}
//#endif
//							/*
//							*下载18030字库时，在这一步不做erase flash操作，把erase和write动作放到一起放置到下一步LD_CMDCODE_LOADFILE中
//							*这样，如果下错字库，会通过识别第一帧中的字库内容终止下载
//							*/
//							sectorpercent = 100;
//							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFONT, 2, 1, &sectorpercent);
//							iRet = hal_ldSendData(ld_pkg_send);	
//							sysLOG(LOAD_LOG_LEVEL_2, "<SUCC> normal_send, the iRet=%d\r\n", iRet);
//						
//						}
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFONT, 1, LD_OK, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFONT, 1, LDERR_GENERIC, NULL);					
					}
					goto normal_send;	
				break;
				case LD_CMDCODE_STARTLOADFILE:

					
					hal_ldLoadFileStrFormat(&g_stLoadfileStr);
					
					if(0x06 < ld_pkg_recv->datalen && ld_pkg_recv->datalen <= 0x16*2)//下载应用自定义文件
					{
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[0]<<24;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[1]<<16;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[2]<<8;
						g_stLoadfileStr.sizeFile += ld_pkg_recv->data[3];
						
						g_stLoadfileStr.numFile = ld_pkg_recv->data[4];
						g_stLoadfileStr.paramtypeFile = ld_pkg_recv->data[5];
						memcpy(g_stLoadfileStr.nameFile_recvd, &(ld_pkg_recv->data[6]), ld_pkg_recv->datalen-6);
						g_stLoadfileStr.typefile = FILETYPE_PARAMFILE;

						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFILE, 1, LD_OK, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTLOADFILE, 1, LDERR_GENERIC, NULL);					
					}
					sysLOG(LOAD_LOG_LEVEL_5, "ld_pkg_recv->data:%x,%x,%x,%x,%x,%x,%x\r\n", ld_pkg_recv->data[0],ld_pkg_recv->data[1],\
					ld_pkg_recv->data[2],ld_pkg_recv->data[3],ld_pkg_recv->data[4],ld_pkg_recv->data[5],ld_pkg_recv->data[6]);
					sysLOG(LOAD_LOG_LEVEL_4, "sizeFile=%d,numFile=%d,paramtypeFile=%d\r\n", g_stLoadfileStr.sizeFile, g_stLoadfileStr.numFile, g_stLoadfileStr.paramtypeFile);
					goto normal_send;	
				break;
				case LD_CMDCODE_LOADFILE:
					
					iRet = hal_ldCMDLoadFile(&g_stLoadfileStr, ld_pkg_recv->data, ld_pkg_recv->datalen);
					if(iRet > 0)//写入成功
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_LOADFILE, 1, LD_OK, NULL);
					}
					else
					{	
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_ldCMDLoadFile, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_LOADFILE, 1, LDERR_GENERIC, NULL);
					}
					
					
					goto normal_send;
				break;
				case LD_CMDCODE_WRITEFILE:
					
					rP = malloc(64);
					if(rP == NULL)return iRet;
					memset(rP, 0, 64);
					*rP = 0x00;
					*rP = 0x64;//没有任何意义，只是传 0x00/0x64，代表100%
					iRet = hal_ldCMDWriteFile(&g_stLoadfileStr);
					if(iRet >= 0)//写入成功
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEFILE, 1+2, LD_OK, rP);
					}
					else
					{	
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_ldCMDWriteFile, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEFILE, 1, LDERR_GENERIC, NULL);
					}
					
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_LOADDONE:
								
					hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_LOADDONE, 1, LD_OK, NULL);
	
					goto normal_send;
				break;
				case LD_CMDCODE_PUK:

					
					iRet = hal_ldCMDLoadPUK(ld_pkg_recv->data, ld_pkg_recv->datalen);
						
					if(iRet >= 0)//写入成功
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_PUK, 1, LD_OK, NULL);
					}
					else
					{	
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_ldCMDLoadPUK, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_PUK, 1, LDERR_GENERIC, NULL);
					}
					
					goto normal_send;	
				break;
				case LD_CMDCODE_CLRQUICKPARAM:
					
					iRet = hal_pmQClrParam();
					if(iRet > 0)//写入成功
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_CLRQUICKPARAM, 1, LD_OK, NULL);
					}
					else
					{	
						
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> hal_ldCMDWriteFile, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_CLRQUICKPARAM, 1, LDERR_GENERIC, NULL);
					}
					goto normal_send;
				break;
				case LD_CMDCODE_AUDIOPLAY:
					iRet = hal_audioFilePlay("audioplay.mp3");
					if(iRet == 0)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIOPLAY, 1, LD_OK, NULL);
					}
					else
					{	
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIOPLAY, 2, iRet, NULL);
					}
					goto normal_send;
				break;
				case LD_CMDCODE_AUDIOPAUSE:
					iRet = hal_audioPause();
					if(iRet == 0)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIOPAUSE, 1, LD_OK, NULL);
					}
					else
					{	
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIOPAUSE, 1, LDERR_GENERIC, NULL);
					}
					goto normal_send;
				break;
				case LD_CMDCODE_AUDIORESUME:
					iRet = hal_audioResume();
					if(iRet == 0)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIORESUME, 1, LD_OK, NULL);
					}
					else
					{	
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIORESUME, 1, LDERR_GENERIC, NULL);
					}
					goto normal_send;
				break;
				case LD_CMDCODE_AUDIOSTOP:
					iRet = hal_audioStop();
					if(iRet == 0)
					{				
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIOSTOP, 1, LD_OK, NULL);
					}
					else
					{	
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR>, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_AUDIOSTOP, 1, LDERR_GENERIC, NULL);
					}
					goto normal_send;
				break;
				case LD_CMDCODE_STARTGETPEDTLOG:
					rP = malloc(5);
					if(rP == NULL)return iRet;
					iRet = hal_fileExist(ld_pkg_recv->data);
					sysLOG(PWR_LOG_LEVEL_5, "pedlog  Name=%s, hal_fileExist iRet=%d\r\n", ld_pkg_recv->data, iRet);
					if(iRet > 0)
					{
					    memset(rP, 0, 5);
						iRet = hal_fileGetFileSize(ld_pkg_recv->data);
				        sysLOG(PWR_LOG_LEVEL_5, "pedlog  Name==%s, iRet=%d, iRet=0x%x\r\n", ld_pkg_recv->data, iRet, iRet);
						memcpy(rP, &iRet, 4);

					}
					
					if(iRet <= 0)
					{		
						sysLOG(LOAD_LOG_LEVEL_4, "<ERR> LD_CMDCODE_STARTGETPEDTLOG, the iRet=%d\r\n", iRet);
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTGETPEDTLOG, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_STARTGETPEDTLOG, 5, LD_OK, rP);
					}
					free(rP);
					goto normal_send;
				break;
				case LD_CMDCODE_READPEDTESTLOG:
                    memcpy(&iRet, ld_pkg_recv->data, 4);
					sysLOG(LOAD_LOG_LEVEL_4, " pedlog, iRet=%d\r\n", iRet);
					rP = malloc(iRet+1);
					if(rP != NULL)
					{
						memset(rP, 0, iRet+1);
	  				    iRet = DR_ReadPedTestLog(ld_pkg_recv->data, rP);
					}
					else
					    iRet = -1;
					
				
					if(iRet <= 0)
					{		
						
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READPEDTESTLOG, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_READPEDTESTLOG, iRet+1, LD_OK, rP);
						free(rP);
					}
					goto normal_send;
				break;
				case LD_CMDCODE_DELETEPEDTESTLOG:
					sysLOG(LOAD_LOG_LEVEL_5, " pedlog, delete name=%s\r\n", ld_pkg_recv->data);
					iRet = hal_fileRemove(ld_pkg_recv->data);
					sysLOG(LOAD_LOG_LEVEL_4, " pedlog, iRet=%d\r\n", iRet);
					if(iRet != 0)
					{		
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_DELETEPEDTESTLOG, 1, LDERR_GENERIC, NULL);
					}
					else
					{
						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_DELETEPEDTESTLOG, 1, LD_OK, NULL);
					}
					goto normal_send;
				break;
//				case LD_CMDCODE_WRITEKEY:
//					if((ld_pkg_recv->datalen > 50) || (ld_pkg_recv->datalen < 19))
//					{				
//						hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEKEY, 1, LDERR_GENERIC, NULL);
//					}
//					else
//					{
//						iRet = DR_CMD_WriteKey(ld_pkg_recv->data, ld_pkg_recv->datalen);
//						if(iRet == 0)
//						{				
//							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEKEY, 1, LD_OK, NULL);
//						}
//						else
//						{	
//							sysLOG(LOAD_LOG_LEVEL_2, "<ERR> DR_CMD_WriteKey, the iRet=%d\r\n", iRet);
//							hal_ldSendDataPack(ld_pkg_send, LD_STARTCODE, ld_pkg_recv->pkg_index, LD_CMD_TYPE, LD_CMDCODE_WRITEKEY, 1, iRet, NULL);
//						}
//					}
//					goto normal_send;
//				break;
				default:
					iRet = COMMERR_LOAD_UNKNOWNCMD;
					sysLOG(LOAD_LOG_LEVEL_4, "<warn> the iRet=%d\r\n", iRet);
				break;
			}
		}
		else
		{
			iRet = COMMERR_LOAD_UNKNOWNCMDTYPE;
			sysLOG(LOAD_LOG_LEVEL_4, "<warn> the iRet=%d\r\n", iRet);
		}

		return iRet;
	normal_send:
		
		iRet = hal_ldSendData(ld_pkg_send);	
		sysLOG(LOAD_LOG_LEVEL_3, "<SUCC> normal_send, the iRet=%d\r\n", iRet);
		return iRet;
		
	repeat_send:
		//*********发送ld_pkg_send数据
		
		iRet = hal_ldSendData(ld_pkg_send);//LDWAR_REPEATPKGINDEX;
		sysLOG(LOAD_LOG_LEVEL_3, "<warn> repeat_send, the iRet=%d\r\n", iRet);
		return iRet;
	}
//	else
//	{
//		g_ui8LoadStep = 0;
//	}
}



