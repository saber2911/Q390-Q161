
#include "LC610N_api_dcep.h"
#include "mbedtls\x509_crt.h"
#include "LC610N_hal_dcep.h"


#define  P10_INDEX  	999
#define  CERT_MAXLEN  	1024*4
#define  DCEP_CERT_DIR 	"/ext/cert/"
#define  DCEP_CERT_LOG 	"/ext/cert/dcepdev.log"

unsigned char *pKeypair = NULL;

/*
*@Brief:		解析证书
*@Param IN:		[in]pbInBuffer	 证书16进制数据
				[in]iLen	     证书数据长度
*@Param OUT:	[out]pbPKout     证书64字节公钥数据，如果不解析传NULL
                [out]piLenout    证书公钥长度，如果不解析传NULL
                [out]pbtbsCer    证书tbs数据，如果不解析传NULL
                [out]pitbsCerLen 证书tbs长度，如果不解析传NULL
                [out]pbSig    	 证书64字节签名值数据，如果不解析传NULL
                [out]piSigLen    证书签名值长度，如果不解析传NULL
*@Return:		true:成功; false:失败
*/
bool getInfoFormCertBuf(unsigned char * pbInBuffer, int iLen, unsigned char * pbPKout, int* piLenout, unsigned char * pbtbsCer, int* pitbsCerLen, unsigned char * pbSig, int* piSigLen)
{
	bool blRet = false;
	if((pbInBuffer[0] != 0x30) || (pbInBuffer[1] != 0x81 && pbInBuffer[1] != 0x82))
	{
		return blRet;
	}
	unsigned char *pbBuffer = (unsigned char*)malloc(iLen + 100);//相对足够大
	memset(pbBuffer, 0, iLen + 100);
	memcpy(pbBuffer, pbInBuffer, iLen);
	int iAllLen = 0;
	int iMove = 2;
	if(pbBuffer[1] == 0x82)
	{
		i_Reverse(pbBuffer + iMove, 2);
		memcpy(&iAllLen, pbBuffer + iMove, 2);
		iAllLen = iAllLen + 4;
	}
	else
	{
		iAllLen = pbBuffer[2] + 3;
	}
	sysLOG(API_LOG_LEVEL_4, "  iAllLen=%d, iLen=%d\r\n", iAllLen, iLen);
	if(iAllLen < iLen)
	{
		goto RET_END;
	}	
	//第一个SEQUENCE是待签的证书，tbsCertificate=TO BE Signed Certificate
	int itbsCerLen = 0;
	if(pbBuffer[1] == 0x82)
	{
		iMove += 4;
		i_Reverse(pbBuffer + iMove, 2);
		memcpy(&itbsCerLen, pbBuffer + iMove, 2);
		itbsCerLen += 4;
		iMove += 2;
		if(pbtbsCer != NULL)
		{
			memcpy(pbtbsCer, pbBuffer + iMove - 4, itbsCerLen);
		}
	}
	else
	{
		iMove += 3;
		itbsCerLen = pbBuffer[iMove] + 3;
		iMove += 1;
		if(pbtbsCer != NULL)
		{
			memcpy(pbtbsCer, pbBuffer + iMove - 3, itbsCerLen);
		}
	}
	if(pitbsCerLen != NULL)
	{
		*pitbsCerLen = itbsCerLen;
		sysLOG(API_LOG_LEVEL_4, "  pitbsCerLen=%d, itbsCerLen=%d\r\n", *pitbsCerLen, itbsCerLen);
	}
#if 0
	if(pbBuffer[iMove] == 0xA0)
	{
		iMove += 2;//version版本
		iMove += pbBuffer[iMove] + 1;
	}
	int iSerialLen = 0;
	if(pbBuffer[iMove] == 0x02)//serialNumber系列号
	{
		iSerialLen = pbBuffer[iMove + 1];
	}
	else
	{
		goto RET_END;
	}
	iMove += 2;
	iMove += iSerialLen;
	int iSignatureLen = 0;
	if(pbBuffer[iMove] == 0x30)//signature签名算法
	{
		iSignatureLen = pbBuffer[iMove + 1];
	}
	else
	{
		goto RET_END;
	}
	iMove += 2;
	iMove += iSignatureLen;
	int iIssuerLen = 0;
	if(pbBuffer[iMove] == 0x30)//issuer颁发者
	{
		iIssuerLen = pbBuffer[iMove + 1];
	}
	else
	{
		goto RET_END;
	}
	iMove += 2;
	iMove += iIssuerLen;
	int iValidityLen = 0;
	if(pbBuffer[iMove] == 0x30)//validity有效期
	{
		iValidityLen = pbBuffer[iMove + 1];
	}
	else
	{
		goto RET_END;
	}
	iMove += 2;
	iMove += iValidityLen;
	int iSubjectLen = 0;
	if(pbBuffer[iMove] == 0x30)//subject主题(使用者)
	{
		iSubjectLen = pbBuffer[iMove + 1];
	}
	else
	{
		goto RET_END;
	}
	iMove += 2;
	iMove += iSubjectLen;
#else
	unsigned char subPublicKeyinfo[] = {0x30, 0x59,0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x82, 0x2D};//subjectPublicKeyinfo主题公钥信息 公钥参数
	for(int i = 0; i < iLen - 50; i++)
	{
		if(memcmp(pbBuffer + i, subPublicKeyinfo, sizeof(subPublicKeyinfo)) == 0)
		{
			iMove = i;
			break;
		}
	}
#endif
	int iPublicKeyinfoLen = 0;
	if(pbBuffer[iMove] == 0x30)//subjectPublicKeyinfo主题公钥信息
	{
		iPublicKeyinfoLen = pbBuffer[iMove + 1];
		iMove += 2;
		iMove += pbBuffer[iMove+1];//公钥参数
		int iPublicKeyLen = 0;
		iMove += 2;
		if(pbBuffer[iMove] == 0x03)//公钥
		{
			iPublicKeyLen  = pbBuffer[iMove + 1];
			if(pbPKout != NULL)
			{
				if(iPublicKeyLen == 0x42 && pbBuffer[iMove + 2] == 0x00 && pbBuffer[iMove + 3] == 0x04)
				{
					iPublicKeyLen -=2;
					memcpy(pbPKout, pbBuffer+ iMove + 4, iPublicKeyLen);
				}
			}
			if(piLenout != NULL)
			{
				*piLenout = iPublicKeyLen;
				sysLOG(API_LOG_LEVEL_4, "  piLenout=%d, iPublicKeyLen=%d\r\n", *piLenout, iPublicKeyLen);
			}
		}
		else
		{
			goto RET_END;
		}
	}
	else
	{
		goto RET_END;
	}
	// 第二个SEQUENCE是签名算法，就是CA准备采用什么签名算法对tbsCertificate进行签名
	int iSeg2Len = 0;
	if(pbBuffer[1] == 0x82)
	{
		iMove = 4;
	}
	else
	{
		iMove = 3;
	}
	iMove += itbsCerLen;
	if(pbBuffer[iMove] == 0x30)
	{
		iSeg2Len = pbBuffer[iMove + 1];
	}
	//第三个 BIT STRING是签名信息，就是CA对tbsCertificate通过signatureAlgorithm签名算法签出来的签名信息
	int iSigLen = 0;
	iMove += 2;
	iMove += iSeg2Len;
	if(pbBuffer[iMove] == 0x03)//sig
	{
		iSigLen = pbBuffer[iMove + 1];
		if(pbSig != NULL)
		{
			//memcpy(pbSig, pbBuffer+ iMove + 2, iSigLen);
			//*piSigLen = iSigLen;
			iMove += 6;
			if(pbBuffer[iMove] == 0x21)
			{
				memcpy(pbSig, pbBuffer+ iMove + 2, 32);
				iMove += 34;
			}
			else
			{
				memcpy(pbSig, pbBuffer+ iMove + 1, 32);
				iMove += 33;
			}
			iMove += 1;
			if(pbBuffer[iMove] == 0x21)
			{
				memcpy(pbSig+32, pbBuffer+ iMove + 2, 32);
				iMove += 34;
			}
			else
			{
				memcpy(pbSig+32, pbBuffer+ iMove + 1, 32);
				iMove += 33;
			}
			if(piSigLen != NULL)
			{
				*piSigLen = 64;
				sysLOG(API_LOG_LEVEL_4, "  piSigLen=%d\r\n", *piSigLen);
			}
		}
	}
	else
	{
		goto RET_END;
	}
	blRet = true;
RET_END:
	free(pbBuffer);
	return blRet;
}


int getKeyPair(uint8* buf, int* len)
{
	int iRet = ERR_DCEP_ERR;

	if ((buf == NULL) || (*len < 96)) {
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	char caPriKeyName[256] = {0};
	sprintf(caPriKeyName, "%sp10key.key", DCEP_CERT_DIR);
    iRet = fibo_file_exist(caPriKeyName);
    if(iRet < 0)
    {
        iRet = ERR_DCEP_FILE_NOTEXITST;
		goto RET_END;
    }
	int iFileSize = fibo_sfile_size(caPriKeyName);
    if(iFileSize <= 0)
    {
        iRet = ERR_DCEP_FILE_NOTEXITST;
		goto RET_END;
    }
	
	iRet = fibo_sfile_init(caPriKeyName);
    if(iRet < 0)
    {
        iRet =  ERR_DCEP_ERR;
		goto RET_END;
    }
	uint8* tlvData = (uchar*)fibo_malloc(iFileSize+1);
	memset(tlvData, 0, iFileSize+1);
	iRet = fibo_sfile_read(caPriKeyName, tlvData, iFileSize);
    if(iRet != iFileSize || iFileSize != 96)
    {
        iRet = ERR_DCEP_CERT_READ;
		fibo_free(tlvData);
		goto RET_END;
    }
	if(buf)
	{
	    memcpy(buf, tlvData, iFileSize);
	}
	*len = 96;
	iRet = ERR_DCEP_OK;
	fibo_free(tlvData);
#if 0	
	int iFd = fileOpen_lib(caPriKeyName, O_RDWR);
    if(iFd >= 0)
    {
	    fileSeek_lib(iFd, 0, SEEK_SET);
	    iRet = fileRead_lib(iFd, buf, len);
		if(iRet != 96)
		{
			iRet = ERR_DCEP_PRIKEY_READ;
			fileClose_lib(iFd);
			goto RET_END;
		}
		*len = 96;
    }
    fileClose_lib(iFd);
#endif
RET_END:

	return iRet;
}

int setKeyPair(uint8* buf, int len)
{
	int iRet = ERR_DCEP_ERR;
	if ((buf == NULL) || (len != 96)) {
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	char caPriKeyName[256] = {0};
	sprintf(caPriKeyName, "%sp10key.key", DCEP_CERT_DIR);
	int iFd = fileOpen_lib(caPriKeyName, O_CREAT|O_RDWR);
    if(iFd >= 0)
    {
        fileClose_lib(iFd);
	    iRet = fibo_sfile_init(caPriKeyName);
	    if(iRet < 0)
	    {
	        iRet =  ERR_DCEP_ERR;
			goto RET_END;
	    }
	    iRet = fibo_sfile_write(caPriKeyName, buf, len);
		if(iRet != len)
		{
			iRet = ERR_DCEP_CERT_WRITE;
			goto RET_END;
		}
    }
    iRet = ERR_DCEP_OK;
RET_END:
	return iRet;
}

/*
*@Brief:		写入证书
*@Param IN:		[in]index	索引号  (目前索引999为个人证书)
				[in]certBuffer	证书内容
				[in]len	证书长度				
*@Param OUT:	
*@Return:		>=0:成功; <0:失败
*/
int setCert(int iIndex, uint8* pbTlvData, int iLen)
{
	sysLOG(API_LOG_LEVEL_4, "  index = %d, len = %d\r\n", iIndex, iLen);

    int iRet = ERR_DCEP_ERR;

	if((iIndex == NULL) || pbTlvData == NULL || (iLen <= 0)  || (iLen > CERT_MAXLEN) || (iIndex < 0) || (iIndex > P10_INDEX))
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}

	if(fibo_filedir_exist(DCEP_CERT_DIR) != 1)
	{
		if(fibo_file_mkdir(DCEP_CERT_DIR) != 0)
		{
			iRet = ERR_DCEP_CERT_DIR;
			goto RET_END;
		}
	}
	
	char caCertName[256] = {0};
	sprintf(caCertName, "%scert_%d.cer", DCEP_CERT_DIR, iIndex);
	iRet = fibo_file_exist(caCertName);
    if(iRet >= 0)
    {
        fibo_file_delete(caCertName);
    }
	int iFd = fileOpen_lib(caCertName, O_CREAT|O_RDWR);
    if(iFd >= 0)
    {
        fileClose_lib(iFd);
	    iRet = fibo_sfile_init(caCertName);
	    if(iRet < 0)
	    {
	        iRet =  ERR_DCEP_ERR;
			goto RET_END;
	    }
	    iRet = fibo_sfile_write(caCertName, pbTlvData, iLen);
		if(iRet != iLen)
		{
			iRet = ERR_DCEP_CERT_WRITE;
			goto RET_END;
		}
    }
	else
		iRet = iFd;

	if (iIndex == P10_INDEX &&  iRet > 0 && pKeypair!=NULL) {
	    setKeyPair(pKeypair, 96);
		free(pKeypair);
	    pKeypair = NULL;
	}

RET_END:
	sysLOG(API_LOG_LEVEL_1, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		读取证书
*@Param IN:		[in]iIndex	索引号  (目前索引999为个人证书)
*@Param OUT:	[out]pbOut 证书数据,如果输入NULL为获取证书数据长度,返回证书数据长度
*@Return:		>0成功 为证书数据长度,	 <0	错误
*/
int getCert(int iIndex, uint8 *pbOut)
{
    sysLOG(API_LOG_LEVEL_4, "  index=%d\r\n", iIndex);
    int iRet = ERR_DCEP_ERR;
	if(iIndex < 0 || iIndex > P10_INDEX)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}

	char caCertName[256] = {0};
	sprintf(caCertName, "%scert_%d.cer", DCEP_CERT_DIR, iIndex);
	iRet = fibo_file_exist(caCertName);
    if(iRet < 0)
    {
        iRet = ERR_DCEP_FILE_NOTEXITST;
		goto RET_END;
    }
	int iFileSize = fibo_sfile_size(caCertName);
    if(iFileSize <= 0)
    {
        iRet = ERR_DCEP_FILE_NOTEXITST;
		goto RET_END;
    }
	
	iRet = fibo_sfile_init(caCertName);
    if(iRet < 0)
    {
        iRet =  ERR_DCEP_ERR;
		goto RET_END;
    }
	uint8* tlvData = (uchar*)fibo_malloc(iFileSize+1);
	memset(tlvData, 0, iFileSize+1);
	iRet = fibo_sfile_read(caCertName, tlvData, iFileSize);
    if(iRet != iFileSize)
    {
        iRet = ERR_DCEP_CERT_READ;
		fibo_free(tlvData);
		goto RET_END;
    }
	if(pbOut)
	{
	    memcpy(pbOut, tlvData, iFileSize);
	}
	iRet = iFileSize;
	fibo_free(tlvData);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*******************************************************************************************
函数名称: BcdTime2String12
描   述 : BCD时间转为字符串函数	（按照格式）			
输入参数: 将要转换的BCD数组:bcd_buf ，以及转换之后将要保存到的数组:tmp_buf
输出参数: 无
返   回 : 无
********************************************************************************************/
void BcdTime2String12(BYTE bcd_buf[], BYTE tmp_buf[])
{
	/* 文件名格式为： “2018-09-15 12:25:26.mp4” */
	//tmp_buf[0] = '2'; //2 /* 年份 */
	//tmp_buf[1] = '0'; //0
	tmp_buf[0] =( bcd_buf[0] >> 4) +( 0x30);  //1    (BCD码转为ASCII码时要加0x30，还要注意运算符优先级)
	tmp_buf[1] = (bcd_buf[0] & 0x0f)  + (0x30); //8
	//tmp_buf[4] = 0x2D; // - 中间横杠的ASCII码
	tmp_buf[2] =( bcd_buf[1] >> 4) +( 0x30);  //0 /* 月份 */
	tmp_buf[3] = (bcd_buf[1] & 0x0f)  + (0x30); //9
	//tmp_buf[7] = 0x2D; // - ()
	tmp_buf[4] = ( bcd_buf[2] >> 4) +( 0x30);  //1 /* 天数 */
	tmp_buf[5] = (bcd_buf[2] & 0x0f)  + (0x30);//5
	//tmp_buf[10] = 0x20; // 空格的ASCII码	
	tmp_buf[6] = ( bcd_buf[3] >> 4) +( 0x30);//1 /* 小时 */
	tmp_buf[7] = (bcd_buf[3]& 0x0f)  + (0x30);//2
	//tmp_buf[13] = 0x3A; //冒号的ASCII码	
	tmp_buf[8] = ( bcd_buf[4] >> 4) +( 0x30);  //2 /* 分钟 */
	tmp_buf[9] = (bcd_buf[4] & 0x0f)  + (0x30); //5
	//tmp_buf[16] = 0x3A; //冒号的ASCII码	
	tmp_buf[10] = ( bcd_buf[5] >> 4) +( 0x30);  //2 /* 秒数 */
	tmp_buf[11] = (bcd_buf[5] & 0x0f)  + (0x30); //6
		
}
#if 0
/*
30820169    第一个SEQUENCE是待签的证书，tbsCertificate=TO BE Signed Certificate
a003020102     version版本
02085000000092574833  serialNumber系列号
300a06082a811ccf55018375  signature签名算法
303c310b300906035504061302434e31123010060355040a0c09494342435f746573743119301706035504030c10494342435f444543505f746573744341   issuer颁发者
301E170D3231303133303037353031375A
    170D3234303133303037353130375A  validity有效期
3037
    310D300B060355040A0C0444434550
    310B300906035504061302434E
    3119301706035504030C1030303231353931323632303034303136    subject主题(使用者)
3059   subjectPublicKeyinfo主题公钥信息
301306072A8648CE3D020106082A811CCF5501822D  公钥参数
0342   公钥
00048ADC10503BF023496239E7BC4A8DAFB9FBB574C0C4227B30B99F08431D7231432E30FB175C7810F6FB353ED3A2F73ADC86528E2E59006DBCC064EE49EA75209A
A35A  扩展项
305830090603551d1304023000
300b0603551d0f0404030206c0
301f0603551d230418301680146d165114032fdbc1d81f4d7f061005b02e0343cf
301D0603551D0E041604148BD493895B1353326BF2C0F6F5CA73BBC36A6ED7
*/
bool getTbsCert(char* X500NameSubject, uchar* pbPubKey, uchar* pbOutTbsCert, int* piOutTbsCertLen)
{
	bool blRet = FALSE;
	BYTE SeqAll[4] = {0x30, 0x82, 0x01, 0x69};//变长
	BYTE Context0[5] = {0xA0, 0x03, 0x02, 0x01, 0x02};//version版本
	BYTE serialNumber[10] = {0x02, 0x08, 0x50, 0x00, 0x00, 0x00, 0x92, 0x57, 0x48, 0x33};//serialNumber系列号
	BYTE sigAlg[12] = {0x30, 0x0a, 0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x01, 0x83, 0x75};//signature签名算法
	BYTE issuer[62] = {0x30, 0x3c, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x4e, 0x31, 0x12, 0x30, 0x10, 0x06, 
		               0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x49, 0x43, 0x42, 0x43, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x31, 0x19, 0x30, 0x17, 0x06, 
		               0x03, 0x55, 0x04, 0x03, 0x0c, 0x10, 0x49, 0x43, 0x42, 0x43, 0x5f, 0x44, 0x45, 0x43, 0x50, 0x5f, 0x74, 0x65, 0x73, 0x74, 
		               0x43, 0x41};//issuer颁发者
	BYTE validityBegin[17] = {0x30, 0x1E, 0x17, 0x0D, 0x32, 0x31, 0x30, 0x31, 0x33, 0x30, 0x30, 0x37, 0x35, 0x30, 0x31, 0x37, 0x5A};//validity有效期	 开始   
	BYTE validityEnd[15]   = {0x17, 0x0D, 0x32, 0x34, 0x30, 0x31, 0x33, 0x30, 0x30, 0x37, 0x35, 0x31, 0x30, 0x37, 0x5A};//validity有效期	 结束
	BYTE subject[2] = {0x30, 0x37};//subject主题(使用者) 变长
	BYTE subjectSet1[90] = {0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x04, 0x44, 0x43, 0x45, 0x50};//subject主题(使用者) O = DCEP 变长
	BYTE subjectSet2[90] = {0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x4E};//subject主题(使用者) C = CN
	BYTE subjectSet3[90] = {0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x10, 0x30, 0x30, 0x32, 0x31, 0x35, 0x39, 0x31, 0x32, 0x36, 0x32, 0x30, 0x30, 0x34, 0x30, 0x31, 0x36};//subject主题(使用者) CN = 0021591262004016
	BYTE subPublicKeyinfo[] = {0x30, 0x59,0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x82, 0x2D};//subjectPublicKeyinfo主题公钥信息 公钥参数
	BYTE subPublicKey[] = {0x03, 0x42, 0x00, 0x04, 
		                   0x8A, 0xDC, 0x10, 0x50, 0x3B, 0xF0, 0x23, 0x49, 0x62, 0x39, 0xE7, 0xBC, 0x4A, 0x8D, 0xAF, 0xB9, 
		                   0xFB, 0xB5, 0x74, 0xC0, 0xC4, 0x22, 0x7B, 0x30, 0xB9, 0x9F, 0x08, 0x43, 0x1D, 0x72, 0x31, 0x43, 
		                   0x2E, 0x30, 0xFB, 0x17, 0x5C, 0x78, 0x10, 0xF6, 0xFB, 0x35, 0x3E, 0xD3, 0xA2, 0xF7, 0x3A, 0xDC, 
		                   0x86, 0x52, 0x8E, 0x2E, 0x59, 0x00, 0x6D, 0xBC, 0xC0, 0x64, 0xEE, 0x49, 0xEA, 0x75, 0x20, 0x9A};//公钥
	BYTE Context3[]   = {0xA3, 0x5A, 0x30, 0x58};//扩展项
	BYTE Context3_1[70] = {0x30, 0x09, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x02, 0x30, 0x00};//扩展项 1
	BYTE Context3_2[70] = {0x30, 0x0b, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x04, 0x04, 0x03, 0x02, 0x06, 0xc0};//扩展项 2
	BYTE Context3_3[70] = {0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x6d, 0x16, 0x51, 0x14, 0x03, 0x2f, 0xdb, 0xc1, 0xd8, 0x1f, 0x4d, 0x7f, 0x06, 0x10, 0x05, 0xb0, 0x2e, 0x03, 0x43, 0xcf};//扩展项 3
	BYTE Context3_4[70] = {0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x8B, 0xD4, 0x93, 0x89, 0x5B, 0x13, 0x53, 0x32, 0x6B, 0xF2, 0xC0, 0xF6, 0xF5, 0xCA, 0x73, 0xBB, 0xC3, 0x6A, 0x6E, 0xD7};//扩展项 4

	//更新序列号
	BYTE baRandom[10] = {0};
	sysGetRandom_lib(4, baRandom);
	memcpy(serialNumber +6, baRandom, 4);

	//更新有效期
	unsigned char aucTime[6] = {0};
	unsigned char timeAsc[16]={0};
	sysGetTime_lib(aucTime);
	BcdTime2String12(aucTime, timeAsc);
	memcpy(validityBegin + 4, timeAsc, 12);
	memcpy(validityEnd + 2, timeAsc, 12);
	unsigned char aucYear[6] = {0};
	memcpy(aucYear, timeAsc, 2);
	int iYear = atoi((const char *)aucYear);
	iYear += 3;//3年有效期
	itoa(iYear,(char*)aucYear,10); 
	memcpy(validityEnd + 2, aucYear, 2);

	//更新 subject主题(使用者) O = DCEP 变长
	int iX500NameSubjectLen = strlen(X500NameSubject);
	if(iX500NameSubjectLen > 70)
		iX500NameSubjectLen = 70;
	subjectSet1[1] = 9 + iX500NameSubjectLen;
	subjectSet1[3] = 7 + iX500NameSubjectLen;
	subjectSet1[10] = iX500NameSubjectLen;
	memcpy(subjectSet1 + 11, X500NameSubject, iX500NameSubjectLen);

	subject[1] = subjectSet1[1] + subjectSet2[1] + subjectSet3[1] + 6;

	//更新公钥
	memcpy(subPublicKey+4, pbPubKey, 64);

	int iSegallLen = sizeof(Context0) + sizeof(serialNumber) + sizeof(sigAlg) + sizeof(issuer) + sizeof(validityBegin) + sizeof(validityEnd) +
		             subject[1] + 2 + sizeof(subPublicKeyinfo) + sizeof(subPublicKey) + Context3[1] + 2;
    if(pbOutTbsCert == NULL)
    {
        *piOutTbsCertLen = iSegallLen + 4;
        goto RET_END;
    }
	if(*piOutTbsCertLen < iSegallLen + 4)
	{
		goto RET_END;
	}
	memcpy(SeqAll + 2, &iSegallLen, 2);
	i_Reverse(SeqAll + 2, 2);
    int iMove = 0;
	memcpy(pbOutTbsCert + iMove, SeqAll, sizeof(SeqAll));
	iMove += sizeof(SeqAll);
	memcpy(pbOutTbsCert + iMove, Context0, sizeof(Context0));
	iMove += sizeof(Context0);
    memcpy(pbOutTbsCert + iMove, serialNumber, sizeof(serialNumber));
	iMove += sizeof(serialNumber);
	memcpy(pbOutTbsCert + iMove, sigAlg, sizeof(sigAlg));
	iMove += sizeof(sigAlg);
	memcpy(pbOutTbsCert + iMove, issuer, sizeof(issuer));
	iMove += sizeof(issuer);
	memcpy(pbOutTbsCert + iMove, validityBegin, sizeof(validityBegin));
	iMove += sizeof(validityBegin);
	memcpy(pbOutTbsCert + iMove, validityEnd, sizeof(validityEnd));
	iMove += sizeof(validityEnd);
	memcpy(pbOutTbsCert + iMove, subject, sizeof(subject));
	iMove += sizeof(subject);
	memcpy(pbOutTbsCert + iMove, subjectSet1, subjectSet1[1] + 2);
	iMove += subjectSet1[1] + 2;
	memcpy(pbOutTbsCert + iMove, subjectSet2, subjectSet2[1] + 2);
	iMove += subjectSet2[1] + 2;
	memcpy(pbOutTbsCert + iMove, subjectSet3, subjectSet3[1] + 2);
	iMove += subjectSet3[1] + 2;
	memcpy(pbOutTbsCert + iMove, subPublicKeyinfo, sizeof(subPublicKeyinfo));
	iMove += sizeof(subPublicKeyinfo);
	memcpy(pbOutTbsCert + iMove, subPublicKey, sizeof(subPublicKey));
	iMove += sizeof(subPublicKey);
	memcpy(pbOutTbsCert + iMove, Context3, sizeof(Context3));
	iMove += sizeof(Context3);
	memcpy(pbOutTbsCert + iMove, Context3_1, Context3_1[1] + 2);
	iMove += Context3_1[1] + 2;
	memcpy(pbOutTbsCert + iMove, Context3_2, Context3_2[1] + 2);
	iMove += Context3_2[1] + 2;
	memcpy(pbOutTbsCert + iMove, Context3_3, Context3_3[1] + 2);
	iMove += Context3_3[1] + 2;
	memcpy(pbOutTbsCert + iMove, Context3_4, Context3_4[1] + 2);
	iMove += Context3_4[1] + 2;
	
    *piOutTbsCertLen = iSegallLen + 4;
	blRet = TRUE;
RET_END:
	return blRet;
}

/*
*@Brief:		生成P10证书请求数据
*@Param IN:		[in]pcX500NameSubject	证书主体信息
*@Param OUT:	[out]pbOut 返回证书请求数据
*@Return:		>0成功,返回数据长度,	<0 错误
*/
int getP10CertReq(char* pcX500NameSubject, uint8 *pbOut)
{
	sysLOG(API_LOG_LEVEL_2, "  X500NameSubject = %s\r\n", pcX500NameSubject);
	int iRet = ERR_DCEP_ERR;
    
	unsigned char baKeypair[100] = {0};

	//生成sm2密钥对
	gmSm2Init_lib(NULL);
	
	iRet = gmSm2ExportPk_lib(2, baKeypair);
	if(iRet != 0)
	{
        goto RET_END;
	}
	iRet = gmSm2ExportPk_lib(3, baKeypair+32);
	if(iRet != 0)
	{
        goto RET_END;
	}

    iRet = gmSm2ExportPk_lib(1, baKeypair+64);
	if(iRet != 0)
	{
        goto RET_END;
	}
	uchar* pKeypairtmp = (uchar*)fibo_malloc(100);
	memset(pKeypairtmp, 0, sizeof(pKeypairtmp));
	memcpy(pKeypairtmp, baKeypair, 96);
	pKeypair = pKeypairtmp;

    //
	uchar pbOutTbsCert[1024] = {0};
	int iOutTbsCertLen = sizeof(pbOutTbsCert);
    bool blRet = getTbsCert(pcX500NameSubject, pKeypairtmp, pbOutTbsCert, &iOutTbsCertLen);
	if(!blRet)
	{
	    sysLOG(API_LOG_LEVEL_1, "  getTbsCert err\r\n");
		goto RET_END;
	}
	//对pbOutTbsCert做签名
	unsigned char *user_id = "1234567812345678";
	int userid_len = strlen(user_id);
	uchar* pbsig = (uchar*)fibo_malloc(64);
	if(gmSm2Sign_lib(user_id, userid_len, baKeypair, baKeypair+64, pbOutTbsCert, iOutTbsCertLen, pbsig) != 0)
	{
		sysLOG(API_LOG_LEVEL_1, "  gmSm2Sign_lib err\r\n");
		fibo_free(pbsig);
		goto RET_END;
	}

	//组织p10数据包
	BYTE SeqAll[4] = {0x30, 0x82, 0x01, 0xC4};//变长
	BYTE baSequence2[] = {0x30, 0x0A, 0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x83, 0x75};//第二个SEQUENCE是签名算法，就是CA准备采用什么签名算法对tbsCertificate进行签名
    BYTE baSignature[] = {0x03, 0x49, 0x00, 0x30, 0x46, 
		0x02, 0x21, 0x00, 0xA0, 0x35, 0xCC, 0xA9, 0x3F, 0x4A, 0x40, 0xBD, 0x33, 0xAD, 0x7E, 0xE1, 0x14, 0x08, 0x8C, 0x06, 0xD6, 0xDF, 0xB6, 0xC1, 0xB8, 0x9A, 0x19, 0xDD, 0x64, 0xBE, 0x5A, 0x65, 0x2A, 0x9B, 0x0B, 0x30, 
		0x02, 0x21, 0x00, 0xB7, 0x42, 0x78, 0x7F, 0xB9, 0xD0, 0xF6, 0x76, 0x22, 0x7A, 0xA0, 0x64, 0x17, 0xFB, 0xAE, 0xF0, 0x42, 0x00, 0xF1, 0x46, 0xCF, 0x2C, 0xED, 0x6C, 0x01, 0xF5, 0x03, 0xE0, 0x85, 0x66, 0xAF, 0x02};// 第三个 BIT STRING是签名信息，就是CA对tbsCertificate通过signatureAlgorithm签名算法签出来的签名信息
    
    uchar bRlen = 0;
	if(pbsig[0] >= 0x80)
	{
		memcpy(baSignature + 8, pbsig, 32);
		bRlen = 0x21;
		baSignature[7] = 0x00;
	}
	else
	{
		memcpy(baSignature + 7, pbsig, 32);
		bRlen = 0x20;
	}
	baSignature[6] = bRlen;	

	uchar bSlen = 0;
	if(pbsig[32] >= 0x80)
	{
		memcpy(baSignature + 10 + bRlen, pbsig + 32, 32);
		bSlen = 0x21;
		baSignature[8 + bRlen + 1] = 0x00;
	}
	else
	{
		memcpy(baSignature + 9 + bRlen, pbsig + 32, 32);
		bSlen = 0x20;
	}
	baSignature[8 + bRlen] = bSlen;
	baSignature[4] = bRlen + bSlen + 4;
	baSignature[1] = bRlen + bSlen + 4 + 3;
    int iSegAllLen = iOutTbsCertLen + sizeof(baSequence2) + baSignature[1] + 2;
	memcpy(SeqAll+2, &iSegAllLen, 2);
	i_Reverse(SeqAll+2, 2);

	uchar* pbP10Cert = (uchar*)fibo_malloc(1024);
	memset(pbP10Cert, 0, 1024);
	memcpy(pbP10Cert, SeqAll, sizeof(SeqAll));
	int iMove = 4;
	memcpy(pbP10Cert + iMove, pbOutTbsCert, iOutTbsCertLen);
    iMove += iOutTbsCertLen;
	memcpy(pbP10Cert + iMove, baSequence2, sizeof(baSequence2));
	iMove += sizeof(baSequence2);
	memcpy(pbP10Cert + iMove, baSignature, baSignature[1] + 2);

	iRet = iMove + baSignature[1] + 2;
	if(pbOut)
	{
		memcpy(pbOut, pbP10Cert, iRet);
	}
	fibo_free(pbsig);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  iRet = %d\r\n", iRet);
	return iRet;
}

#else
/*
308199    第一个SEQUENCE是待签的证书，tbsCertificate=TO BE Signed Certificate
020100  serialNumber系列号
3037    subject主题(使用者) "CN = 0021591262004016,C = CN,O = DCEP"
    310D300B060355040A0C04 44434550    O = DCEP
    310B300906035504061302434E         C = CN
    3119301706035504030C1030303231353931323632303034303136 CN = 0021591262004016
3059   subjectPublicKeyinfo主题公钥信息
301306072A8648CE3D020106082A811CCF5501822D  公钥参数
0342   公钥
0004 8ADC10503BF023496239E7BC4A8DAFB9FBB574C0C4227B30B99F08431D7231432E30FB175C7810F6FB353ED3A2F73ADC86528E2E59006DBCC064EE49EA75209A
A000  扩展项
*/
bool getTbsCert(char* X500NameSubject, uchar* pbPubKey, uchar* pbOutTbsCert, int* piOutTbsCertLen)
{
    sysLOG(API_LOG_LEVEL_2, "  X500NameSubject = %s\r\n", X500NameSubject);
	bool blRet = FALSE;
	BYTE SeqAll[4] = {0x30, 0x82, 0x01, 0x69};//变长
	BYTE serialNumber[3] = {0x02, 0x01, 0x00};//serialNumber系列号
	BYTE subject[2] = {0x30, 0x37};//subject主题(使用者) 变长
	BYTE subjectSetO[90] =  {0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x04, 0x44, 0x43, 0x45, 0x50};//subject主题(使用者) O = DCEP 变长
	BYTE subjectSetC[90] =  {0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x43, 0x4E};//subject主题(使用者) C = CN
	BYTE subjectSetCN[90] = {0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x10, 0x30, 0x30, 0x32, 0x31, 0x35, 0x39, 0x31, 0x32, 0x36, 0x32, 0x30, 0x30, 0x34, 0x30, 0x31, 0x36};//subject主题(使用者) CN = 0021591262004016
	BYTE subPublicKeyinfo[] = {0x30, 0x59,0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x82, 0x2D};//subjectPublicKeyinfo主题公钥信息 公钥参数
	BYTE subPublicKey[] = {0x03, 0x42, 0x00, 0x04, 
		                   0x8A, 0xDC, 0x10, 0x50, 0x3B, 0xF0, 0x23, 0x49, 0x62, 0x39, 0xE7, 0xBC, 0x4A, 0x8D, 0xAF, 0xB9, 
		                   0xFB, 0xB5, 0x74, 0xC0, 0xC4, 0x22, 0x7B, 0x30, 0xB9, 0x9F, 0x08, 0x43, 0x1D, 0x72, 0x31, 0x43, 
		                   0x2E, 0x30, 0xFB, 0x17, 0x5C, 0x78, 0x10, 0xF6, 0xFB, 0x35, 0x3E, 0xD3, 0xA2, 0xF7, 0x3A, 0xDC, 
		                   0x86, 0x52, 0x8E, 0x2E, 0x59, 0x00, 0x6D, 0xBC, 0xC0, 0x64, 0xEE, 0x49, 0xEA, 0x75, 0x20, 0x9A};//公钥
	//BYTE Context3[]   = {0xA3, 0x5A, 0x30, 0x58};//扩展项
	BYTE Context3[]   = {0xA0, 0x00};//扩展项
	//更新 subject主题(使用者) O = DCEP 变长 "CN = 0021642768922019,C = CN,O = VASTONE"
	char bcSubO[100]={0};
	char bcSubC[100]={0};
	char bcSubCN[100]={0};
    memset(bcSubO, 0, sizeof(bcSubO));
	memset(bcSubC, 0, sizeof(bcSubC));
	memset(bcSubCN, 0, sizeof(bcSubCN));
	char *pcSubO = strstr(X500NameSubject, "O=");
	char *pcTemp = NULL;
	if(pcSubO)
	{
	    sysLOG(API_LOG_LEVEL_4, "  pcSubO=%s\r\n", pcSubO);
		pcTemp = strstr(pcSubO, ",");
		if(pcTemp)
		{
		    memcpy(bcSubO, pcSubO+2, pcTemp - pcSubO - 2);
		}
		else
		{
		    memcpy(bcSubO, pcSubO+2, strlen(pcSubO) - 2);
		}
	}
	char *pcSubC = strstr(X500NameSubject, "C=");
	if(pcSubC)
	{
	    sysLOG(API_LOG_LEVEL_4, "  pcSubC=%s\r\n", pcSubC);
		pcTemp = strstr(pcSubC, ",");
		if(pcTemp)
		{
		    memcpy(bcSubC, pcSubC+2, pcTemp - pcSubC - 2);
		}
		else
		{
		    memcpy(bcSubC, pcSubC+2, strlen(pcSubC) - 2);
		}
	}
	char *pcSubCN = strstr(X500NameSubject, "CN=");
	if(pcSubCN)
	{
	    sysLOG(API_LOG_LEVEL_4, "  pcSubCN=%s\r\n", pcSubCN);
		pcTemp = strstr(pcSubCN, ",");
		if(pcTemp)
		{
		    memcpy(bcSubCN, pcSubCN+3, pcTemp - pcSubCN - 3);
		}
		else
		{
		    memcpy(bcSubCN, pcSubCN+3, strlen(pcSubCN) - 3);
		}
	}
	sysLOG(API_LOG_LEVEL_4, "  O=%s,C=%s,CN=%S\r\n", bcSubO,bcSubC,bcSubCN);
	subjectSetO[1] = 9 + strlen(bcSubO);
	subjectSetO[3] = 7 + strlen(bcSubO);
	subjectSetO[10] = strlen(bcSubO);
	memcpy(subjectSetO + 11, bcSubO, strlen(bcSubO));
	
	subjectSetC[1] = 9 + strlen(bcSubC);
	subjectSetC[3] = 7 + strlen(bcSubC);
	subjectSetC[10] = strlen(bcSubC);
	memcpy(subjectSetC + 11, bcSubC, strlen(bcSubC));

	subjectSetCN[1] = 9 + strlen(bcSubCN);
	subjectSetCN[3] = 7 + strlen(bcSubCN);
	subjectSetCN[10] = strlen(bcSubCN);
	memcpy(subjectSetCN + 11, bcSubCN, strlen(bcSubCN));

	subject[1] = subjectSetO[1] + subjectSetC[1] + subjectSetCN[1] + 6;

	//更新公钥
	memcpy(subPublicKey+4, pbPubKey, 64);

	int iSegallLen = sizeof(serialNumber) + subject[1] + 2 + sizeof(subPublicKeyinfo) + sizeof(subPublicKey) + Context3[1] + 2;
    if(pbOutTbsCert == NULL)
    {
        *piOutTbsCertLen = iSegallLen + 4;
        goto RET_END;
    }
	if(*piOutTbsCertLen < iSegallLen + 4)
	{
		goto RET_END;
	}
	int iMove = 0;
	if(iSegallLen > 0xFF)
	{
		memcpy(SeqAll + 2, &iSegallLen, 2);
		i_Reverse(SeqAll + 2, 2);
		memcpy(pbOutTbsCert + iMove, SeqAll, sizeof(SeqAll));
		iMove += sizeof(SeqAll);
		*piOutTbsCertLen = iSegallLen + 4;
	}
    else
	{
	    SeqAll[1] = 0x81;
		SeqAll[2] = iSegallLen;
		memcpy(pbOutTbsCert + iMove, SeqAll, 3);
		iMove += 3;
		*piOutTbsCertLen = iSegallLen + 3;
	}	
    memcpy(pbOutTbsCert + iMove, serialNumber, sizeof(serialNumber));
	iMove += sizeof(serialNumber);
	memcpy(pbOutTbsCert + iMove, subject, sizeof(subject));
	iMove += sizeof(subject);
	memcpy(pbOutTbsCert + iMove, subjectSetO, subjectSetO[1] + 2);
	iMove += subjectSetO[1] + 2;
	memcpy(pbOutTbsCert + iMove, subjectSetC, subjectSetC[1] + 2);
	iMove += subjectSetC[1] + 2;
	memcpy(pbOutTbsCert + iMove, subjectSetCN, subjectSetCN[1] + 2);
	iMove += subjectSetCN[1] + 2;
	memcpy(pbOutTbsCert + iMove, subPublicKeyinfo, sizeof(subPublicKeyinfo));
	iMove += sizeof(subPublicKeyinfo);
	memcpy(pbOutTbsCert + iMove, subPublicKey, sizeof(subPublicKey));
	iMove += sizeof(subPublicKey);
	memcpy(pbOutTbsCert + iMove, Context3, sizeof(Context3));
	iMove += sizeof(Context3);    
	blRet = TRUE;
RET_END:
	return blRet;
}

/*
* 函数名称: DelSpaceAndCheck
* 函数功能: 去掉空格并检查合法性
* 参    数: 
* 返 回 值: 
*/
BOOL DelSpaceAndCheck(IN   char *pcSrc, OUT char *pcDest)
{
	if(NULL == pcSrc)
	{
		return FALSE;
	}
	unsigned int uiLen = strlen(pcSrc);
	int iDestIndex = 0;
	for(int i = 0; i < uiLen; i++)
	{
		if(' ' == pcSrc[i])
		{
			continue;
		}
        pcDest[iDestIndex] = pcSrc[i];
		iDestIndex++;
	}
	if(strstr(pcDest,"CN=") == NULL && strstr(pcDest,"C=") == NULL && strstr(pcDest,"O=") == NULL)
		return FALSE;
	else
    	return TRUE;
}

/*
*@Brief:		生成P10证书请求数据
*@Param IN:		[in]pcX500NameSubject	证书主体信息 类似 "CN = 0021642768922019,C = CN,O = VASTONE"
*@Param OUT:	[out]pbOut 返回证书请求数据
*@Return:		>0成功,返回数据长度,	<0 错误
*/
int getP10CertReq(char* pcX500NameSubject, uint8 *pbOut)
{
	sysLOG(API_LOG_LEVEL_4, "  X500NameSubject = %s\r\n", pcX500NameSubject);
	int iRet = ERR_DCEP_ERR;
	char* pcX500NameSubjectDest = NULL;
	if(NULL == pcX500NameSubject || strlen(pcX500NameSubject) <= 0)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	pcX500NameSubjectDest = (char*)fibo_malloc(strlen(pcX500NameSubject) + 2);
	memset(pcX500NameSubjectDest, 0, strlen(pcX500NameSubject) + 2);
    if(!DelSpaceAndCheck(pcX500NameSubject, pcX500NameSubjectDest))
    {
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
    }
	unsigned char baKeypair[100] = {0};

	//生成sm2密钥对
	gmSm2Init_lib(NULL);
	
	iRet = gmSm2ExportPk_lib(2, baKeypair);
	if(iRet != 0)
	{
        goto RET_END;
	}
	iRet = gmSm2ExportPk_lib(3, baKeypair+32);
	if(iRet != 0)
	{
        goto RET_END;
	}

    iRet = gmSm2ExportPk_lib(1, baKeypair+64);
	if(iRet != 0)
	{
        goto RET_END;
	}
	uint8* pKeypairtmp = (uint8*)fibo_malloc(100);
	memset(pKeypairtmp, 0, sizeof(pKeypairtmp));
	memcpy(pKeypairtmp, baKeypair, 96);
	pKeypair = pKeypairtmp;

    //
	uint8 pbOutTbsCert[1024] = {0};
	int iOutTbsCertLen = sizeof(pbOutTbsCert);
    bool blRet = getTbsCert(pcX500NameSubjectDest, pKeypairtmp, pbOutTbsCert, &iOutTbsCertLen);
	if(!blRet)
	{
	    sysLOG(API_LOG_LEVEL_2, "  getTbsCert err\r\n");
		goto RET_END;
	}
	//对pbOutTbsCert做签名
	unsigned char *user_id = "1234567812345678";
	int userid_len = strlen(user_id);
	uint8* pbsig = (uint8*)fibo_malloc(64+2);
	if(gmSm2Sign_lib(user_id, userid_len, baKeypair, baKeypair+64, pbOutTbsCert, iOutTbsCertLen, pbsig) != 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  gmSm2Sign_lib err\r\n");
		fibo_free(pbsig);
		goto RET_END;
	}

	//组织p10数据包
	BYTE SeqAll[4] = {0x30, 0x82, 0x01, 0xC4};//变长
	BYTE baSequence2[] = {0x30, 0x0A, 0x06, 0x08, 0x2A, 0x81, 0x1C, 0xCF, 0x55, 0x01, 0x83, 0x75};//第二个SEQUENCE是签名算法，就是CA准备采用什么签名算法对tbsCertificate进行签名
    BYTE baSignature[] = {0x03, 0x49, 0x00, 0x30, 0x46, 
		0x02, 0x21, 0x00, 0xA0, 0x35, 0xCC, 0xA9, 0x3F, 0x4A, 0x40, 0xBD, 0x33, 0xAD, 0x7E, 0xE1, 0x14, 0x08, 0x8C, 0x06, 0xD6, 0xDF, 0xB6, 0xC1, 0xB8, 0x9A, 0x19, 0xDD, 0x64, 0xBE, 0x5A, 0x65, 0x2A, 0x9B, 0x0B, 0x30, 
		0x02, 0x21, 0x00, 0xB7, 0x42, 0x78, 0x7F, 0xB9, 0xD0, 0xF6, 0x76, 0x22, 0x7A, 0xA0, 0x64, 0x17, 0xFB, 0xAE, 0xF0, 0x42, 0x00, 0xF1, 0x46, 0xCF, 0x2C, 0xED, 0x6C, 0x01, 0xF5, 0x03, 0xE0, 0x85, 0x66, 0xAF, 0x02};// 第三个 BIT STRING是签名信息，就是CA对tbsCertificate通过signatureAlgorithm签名算法签出来的签名信息
    
    uchar bRlen = 0;
	if(pbsig[0] >= 0x80)
	{
		memcpy(baSignature + 8, pbsig, 32);
		bRlen = 0x21;
		baSignature[7] = 0x00;
	}
	else
	{
		memcpy(baSignature + 7, pbsig, 32);
		bRlen = 0x20;
	}
	baSignature[6] = bRlen;	

	uint8 bSlen = 0;
	if(pbsig[32] >= 0x80)
	{
		memcpy(baSignature + 10 + bRlen, pbsig + 32, 32);
		bSlen = 0x21;
		baSignature[8 + bRlen + 1] = 0x00;
	}
	else
	{
		memcpy(baSignature + 9 + bRlen, pbsig + 32, 32);
		bSlen = 0x20;
	}
	baSignature[8 + bRlen] = bSlen;
	baSignature[4] = bRlen + bSlen + 4;
	baSignature[1] = bRlen + bSlen + 4 + 3;
    int iSegAllLen = iOutTbsCertLen + sizeof(baSequence2) + baSignature[1] + 2;
	uint8* pbP10Cert = (uint8*)fibo_malloc(1024);
	memset(pbP10Cert, 0, 1024);
	int iMove = 4;
	if(iSegAllLen > 0xFF)
	{
		memcpy(SeqAll+2, &iSegAllLen, 2);
		i_Reverse(SeqAll+2, 2);
		memcpy(pbP10Cert, SeqAll, sizeof(SeqAll));
	}
	else
	{
		SeqAll[1] = 0x81;
		SeqAll[2] = iSegAllLen;
		iMove = 3;
		memcpy(pbP10Cert, SeqAll, 3);
	}
	memcpy(pbP10Cert + iMove, pbOutTbsCert, iOutTbsCertLen);
    iMove += iOutTbsCertLen;
	memcpy(pbP10Cert + iMove, baSequence2, sizeof(baSequence2));
	iMove += sizeof(baSequence2);
	memcpy(pbP10Cert + iMove, baSignature, baSignature[1] + 2);

	iRet = iMove + baSignature[1] + 2;
	if(pbOut)
	{
		memcpy(pbOut, pbP10Cert, iRet);
	}
	fibo_free(pbsig);
RET_END:
	if(pcX500NameSubjectDest)
		fibo_free(pcX500NameSubjectDest);
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}
#endif

/*
*@Brief:		用指定索引的证书校验传入的证书
*@Param IN:		[in]iIndex	上级证书索引
				[in]pbCert	待校验证书
				[in]iCertLen	待校验证书长度
*@Param OUT:	
*@Return:		0	成功    <0	失败
*/
int verifyCert(int iIndex, uint8* pbCert, int iCertLen)
{
    LC610_writeDcepLog(DCEP_CERT_LOG, "into verifyCert iIndex=%d, iCertLen=%d", iIndex, iCertLen);
	LC610_LogHexData(DCEP_CERT_LOG, "verifyCert pbCert=", pbCert, iCertLen);
	sysLOG(API_LOG_LEVEL_4, " index = %d, cert = 0x%x, iCertLen=%d\r\n", iIndex, pbCert, iCertLen);
    int iRet = ERR_DCEP_ERR;
	if(iIndex < 0 || iIndex > P10_INDEX)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
    uint8* pGetCert = (uint8*)fibo_malloc(CERT_MAXLEN);
	memset(pGetCert, 0, sizeof(pGetCert));
	iRet = getCert(iIndex, pGetCert);
	if(iRet < 0)
	{
		goto RET_END1;
	}
	LC610_LogHexData(DCEP_CERT_LOG, "verifyCert pGetCert=", pGetCert, iRet);
	int iPcertLen = iRet;
	unsigned char baPKout[200] = {0};
	int iPkLen = sizeof(baPKout);
	unsigned char baBody[1024] = {0};
	int iBodyLen = sizeof(baBody);
	unsigned char baSig[200] = {0};
	int iSigLen = sizeof(baSig);
    memset(baPKout, 0, sizeof(baPKout));
	iRet = getInfoFormCertBuf(pGetCert, iPcertLen, baPKout, &iPkLen, NULL, NULL, NULL, NULL);
    if(!iRet)
    {
        sysLOG(API_LOG_LEVEL_2, "  getInfoFormCertBuf 1 err iPkLen=%d\r\n", iPkLen);
		iRet = ERR_DCEP_CERT_GETINFO;
		goto RET_END1;
    }
	LC610_LogHexData(DCEP_CERT_LOG, "verifyCert baPKout=", baPKout, iPkLen);
	memset(baBody, 0, sizeof(baBody));
	memset(baSig, 0, sizeof(baSig));
	iRet = getInfoFormCertBuf(pbCert, iCertLen, NULL, NULL, baBody, &iBodyLen, baSig, &iSigLen);
    if(!iRet)
    {
        sysLOG(API_LOG_LEVEL_2, "  getInfoFormCertBuf 2 err iBodyLen=%d,iSigLen=%d\r\n", iBodyLen, iSigLen);
		iRet = ERR_DCEP_CERT_GETINFO;
		goto RET_END1;
    }
	LC610_LogHexData(DCEP_CERT_LOG, "verifyCert baBody=", baBody, iBodyLen);
	LC610_LogHexData(DCEP_CERT_LOG, "verifyCert baSig=", baSig, iSigLen);
#if 0
	char buff[4096] = {0};

	mbedtls_x509_crt Cacert;
	mbedtls_x509_crt_init(&Cacert);
	iRet = mbedtls_x509_crt_parse(&Cacert, (unsigned char*)Test_cert, sizeof(Test_cert));
	if(iRet != 0)
	{
	    sysLOG(API_LOG_LEVEL_1, "  mbedtls_x509_crt_parse err %d\r\n", iRet);
	    goto RET_END;
	}
    unsigned char *public_key; 
	//mbedtls_pk_parse_public_key(Cacert.pk);
    sysLOG(API_LOG_LEVEL_2, "  version = %d\r\n", Cacert.version);
	//sysLOG(API_LOG_LEVEL_2, "  sig = %s\r\n", Cacert.sig.p);
	//sysLOG(API_LOG_LEVEL_2, "  pk = %s\r\n", Cacert.pk);
	iRet = mbedtls_x509_crt_info(buff, sizeof(buff)-1, "    ", &Cacert);
	if(iRet < 0)
	{
	    sysLOG(API_LOG_LEVEL_1, "  mbedtls_x509_crt_info err %d\r\n", iRet);
	    goto RET_END;
	}
	sysLOG(API_LOG_LEVEL_2, "  buff = %s\r\n", buff);
	mbedtls_x509_crt_free(&Cacert);
#endif

	//用指定索引的证书公钥验签待校验证书
	unsigned char *user_id = "1234567812345678";
	int userid_len = strlen(user_id);
	if(gmSm2Verify_lib(user_id, userid_len, baPKout, baSig, baBody, iBodyLen) != 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  gmSm2Verify_lib err\r\n");
		iRet = ERR_DCEP_CERT_VERIFY;
		goto RET_END1;
	}

	iRet = 0;
RET_END1:
	fibo_free(pGetCert);
RET_END:
    sysLOG(API_LOG_LEVEL_1, "  blRet = %d\r\n", iRet);
	LC610_writeDcepLog(DCEP_CERT_LOG, "exit verifyCert iRet=%d", iRet);
	return iRet;
}

/*
*@Brief:		用指定索引的证书校验传入的数据的签名
*@Param IN:		[in]index	证书索引
				[in]sign	待校验数据的签名数据
				[in]iSignLen 待校验数据的签名数据长度 64字节
				[in]data	待校验数据
				[in]len	数据长度
*@Param OUT:	
*@Return:		0	成功    <0	失败
*/
int verifyByCert(int iIndex, uint8* pbSign, int iSignLen, uint8* pbData, int iLen)
{
	sysLOG(API_LOG_LEVEL_4, " index = %d, iSignLen = %d, len = %d\r\n", iIndex, iSignLen, iLen);
    int iRet = ERR_DCEP_ERR;
	if (iIndex < 0 || iIndex > P10_INDEX || pbSign==NULL || pbData == NULL || iSignLen != 64 || iLen<0) {
		sysLOG(API_LOG_LEVEL_2, "  verifyByCert paramete error\r\n");
		iRet = ERR_DCEP_PARAM;
	    goto RET_END;
	}

	uint8* pbCert = (uint8*)fibo_malloc(CERT_MAXLEN);
	memset(pbCert, 0, CERT_MAXLEN);
	iRet = getCert(iIndex, pbCert);
	if(iRet < 0)
	{
		goto RET_END1;
	}
	int iPcertLen = iRet;
	unsigned char baPKout[200] = {0};
	memset(baPKout, 0, sizeof(baPKout));
	int iPkLen = sizeof(baPKout);
	iRet = getInfoFormCertBuf(pbCert, iPcertLen, baPKout, &iPkLen, NULL, NULL, NULL, NULL);
    if(!iRet)
    {
		iRet = ERR_DCEP_CERT_GETINFO;
		goto RET_END1;
    }

	//用指定索引的证书公钥验签待校验证书
	//gmSm2Init_lib(NULL);
	unsigned char *user_id = "1234567812345678";
	int userid_len = strlen(user_id);
	if(gmSm2Verify_lib(user_id, userid_len, baPKout, pbSign, pbData, iLen) != 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  gmSm2Verify_lib err\r\n");
		goto RET_END1;
	}

	iRet = 0;
RET_END1:
	fibo_free(pbCert);
RET_END:
    sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		用指定索引的证书对数据签名
*@Param IN:		[in]index	证书索引
				[in]data	待签名数据
				[in]len	数据长度
*@Param OUT:	[out]pcOut	签名值数据
*@Return:		>	成功 返回签名值数据长度   
                <0	失败
*/
int signByCert(int iIndex, uint8* pbData, int iLen, uint8 *pbOut)
{
	sysLOG(API_LOG_LEVEL_4, " index = %d, len = %d\r\n", iIndex, iLen);
    int iRet = ERR_DCEP_ERR;
	if (iIndex != P10_INDEX || pbData==NULL || iLen <= 0 || pbOut == NULL) {
		sysLOG(API_LOG_LEVEL_2, "  signByCert paramete error\r\n");
		iRet = ERR_DCEP_PARAM;
	    goto RET_END;
	}
	
    uint8 baKeypair[200] = {0};
	int ikeypairLen = sizeof(baKeypair);
    iRet = getKeyPair(baKeypair, &ikeypairLen);
    if(iRet != 0) {
        sysLOG(API_LOG_LEVEL_2, "  getKeyPair error\r\n");
        return iRet;
    }
	unsigned char *user_id = "1234567812345678";
	int userid_len = strlen(user_id);
	if(gmSm2Sign_lib(user_id, userid_len, baKeypair, baKeypair+64, pbData, iLen, pbOut) != 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  gmSm2Verify_lib err\r\n");
		goto RET_END;
	}
	iRet = 64;
RET_END:
    sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		用证书对数据加密 
*@Param IN:		[in]iMode	加密模式 0：C1C2C3      	1：C1C3C2 目前只支持1
				[in]pbCert	证书数据
				[in]iCertLen	证书数据长度
				[in]pbData	待加密数据
				[in]iLen	    待加密数据数据长度，不大于1024字节
*@Param OUT:	[out]pbOut	    密文数据,分配的空间要大于iLen+96字节
*@Return:		>0 成功,返回密文数据长度     
                <0 失败
*/
int encrypByCert(int iMode, uint8* pbCert, int iCertLen, uint8* pbData, int iLen, uint8 *pbOut)
{
	sysLOG(API_LOG_LEVEL_4, " mode = %d, iCertLen = %d, len = %d\r\n", iMode, iCertLen, iLen);
    int iRet = ERR_DCEP_ERR;
	if (iMode != 1 || pbCert == NULL || pbData==NULL || pbOut == NULL || iLen <= 0 || iLen > 1024 || iCertLen > 4096) {
		iRet = ERR_DCEP_PARAM;
	    goto RET_END;
	}
	unsigned char baPKout[200] = {0};
	int iPkLen = sizeof(baPKout);

	bool blRet = getInfoFormCertBuf(pbCert, iCertLen, baPKout, &iPkLen, NULL, NULL, NULL, NULL);
    if(!blRet)
    {
		iRet = ERR_DCEP_CERT_GETINFO;
		goto RET_END;
    }
	int iOutLen = iLen+96;
    iRet = gmSm2_lib(baPKout, iPkLen, pbData, iLen, pbOut, &iOutLen, 1);
	if(iRet != 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  gmSm2_lib err\r\n");
		goto RET_END;
	}

	iRet = iOutLen;
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;

}


/*
*@Brief:		用指定证书对数据解密
*@Param IN:		[in]mode	解密模式 0：C1C2C3      	1：C1C3C2 目前只支持1
				[in]index	证书索引
				[in]data	密文数据
				[in]iLen	    数据长度，不大于1024+96
*@Param OUT:	[out]pcOut	    明文数据，要分配大于iLen-96字节空间
*@Return:		>0 成功,返回明文数据长度
				<0 失败
*/
int decrypByCert(int iMode, int iIndex, uint8* pbData, int iLen, uint8* pbOut)
{
	sysLOG(API_LOG_LEVEL_4, " mode = %d, index = %d, len = %d\r\n", iMode, iIndex, iLen);
    int iRet = ERR_DCEP_ERR;
	if (iMode != 1 || iIndex != P10_INDEX || pbData==NULL || pbOut == NULL || iLen <= 0  || iLen > 1024+96) {
		iRet = ERR_DCEP_PARAM;
	    goto RET_END;
	}
	uint8 baKeypair[100] = {0};
	int ikeypairLen = sizeof(baKeypair);
    iRet = getKeyPair(baKeypair, &ikeypairLen);
    if(iRet != 0) {
        sysLOG(API_LOG_LEVEL_2, "  getKeyPair error\r\n");
        goto RET_END;
    }
    int iOutLen = iLen - 96;
	iRet = gmSm2_lib(baKeypair, ikeypairLen, pbData, iLen, pbOut, &iOutLen, 0);
	if(iRet != 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  gmSm2_lib err\r\n");
		goto RET_END;
	}
	iRet = iOutLen;
RET_END:
    sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/**
 * sm2非对称加密算法
 *
 * @param key		密钥数据。
 *					Sm2算法的私钥长度为32字节，公钥长度为64字节。
 *					Mode=0时，Key为96字节，数据结构为：64字节公钥（X+Y） +32字节私钥。
 *					Mode=1时，Key为64字节公钥（X+Y）
 * @param keyLen    密钥数据长度
 * @param input 	待加密或解密的数据
 * @param inputLen  待加密或解密的数据长度,不大于1024字节
 * @param output    加密或解密后的数据
 * @param mode		0x00：使用SM2 私钥解密数据
 *					0x01：使用SM2公钥加密数据
 * @return 大于0表示成功，此时返回值表示加密或解密后的数据长度，小于0失败
 */
int sm2(uint8* pbKey, int iKeyLen, uint8* pbInput, int iInputLen, uint8* pbOutput, uint8 bMode) 
{
	sysLOG(API_LOG_LEVEL_4, " mode = %d, keyLen = %d, inputLen = %d\r\n", bMode, iKeyLen, iInputLen);
    int iRet = ERR_DCEP_ERR;
	if((bMode != 0x00 && bMode != 0x01) || pbKey == NULL || pbInput == NULL || pbOutput == NULL || iInputLen > 1024)
	{
		iRet = ERR_DCEP_PARAM;
	    goto RET_END;
	}
    unsigned int OutputLen = 0;
    if(bMode == 0x00 && iKeyLen == 96 && iInputLen > 96)
    {
        OutputLen = iInputLen - 96;
    }
	else if(bMode == 0x01 && iKeyLen == 64 && iInputLen > 0)
    {
        OutputLen = iInputLen + 96;
    }
	else
	{
		iRet = ERR_DCEP_PARAM;
	    goto RET_END;
	}
	iRet = gmSm2_lib(pbKey, iKeyLen, pbInput, iInputLen, pbOutput, &OutputLen, bMode);
    if(iRet == 0)
	{
        iRet = OutputLen;
	}
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;

}

/**
 * 使用sm2算法签名数据
 *
 * @param user_id	  签名者信息
 * @param user_id_len 签名者信息长度,长度最长为32字节
 * @param public_key  公钥数据，64字节
 * @param pbPrivate_key 私钥数据，32字节
 * @param pbMsg		  待签名数据
 * @param msg_len	  待签名数据长度，不大于1024字节
 * @param out_buf	  输出缓冲区
 * @return 大于0表示成功,并返回签名数据长度，小于0失败
 */
int sm2Sign(uint8* pbUser_id, int iUser_id_len, uint8* pbPublic_key, uint8* pbPrivate_key, uint8* pbMsg, int iMsg_len, uint8* pbOut_buf)
{
	sysLOG(API_LOG_LEVEL_4, " user_id_len = %d, msg_len = %d\r\n", iUser_id_len, iMsg_len);
    int iRet = ERR_DCEP_ERR;
	if(iUser_id_len < 0x00 || iUser_id_len > 32 || pbUser_id == NULL || pbPublic_key == NULL || pbPrivate_key == NULL || pbMsg == NULL || pbOut_buf == NULL)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	iRet = gmSm2Sign_lib(pbUser_id, iUser_id_len, pbPublic_key, pbPrivate_key, pbMsg, iMsg_len, pbOut_buf);
	if(iRet == 0)
	{
		iRet = 64;
	}
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}
					
/**
 * 使用sm2算法验证签名
 *
 * @param user_id	  签名者信息
 * @param user_id_len 签名者信息长度,长度最长为32字节
 * @param public_key  公钥数据，64字节
 * @param signed_data 签名信息，64字节
 * @param msg		  待签名数据
 * @param msg_len	  待签名数据长度，不大于1024字节
 * @return =0表示成功，小于0失败
 */
int sm2Verify(uint8* pbUser_id, int iUser_id_len, uint8* pbPublic_key, uint8* pbSigned_data,	 uint8* pbMsg, int iMsg_len) 
{
	sysLOG(API_LOG_LEVEL_4, " user_id_len = %d, msg_len = %d\r\n", iUser_id_len, iMsg_len);
    int iRet = ERR_DCEP_ERR;
	if(iUser_id_len < 0x00 || iUser_id_len > 32 || pbUser_id == NULL || pbPublic_key == NULL || pbSigned_data == NULL || pbMsg == NULL || iMsg_len > 1024)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	iRet = gmSm2Verify_lib(pbUser_id, iUser_id_len, pbPublic_key, pbSigned_data, pbMsg, iMsg_len);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/**
* sm3散列算法，获取不大于1024字节的数据的32字节hash值
*
* @param input	 待散列数据，不大于1024字节
* @param input_len 待散列数据长度
* @param out_buf	 输出缓冲区，不小于32字节
* @return 大于0表示成功,返回结果数据长度，小于0失败
*/
int sm3(uint8* pbInput, int iInput_len, uint8* pbOut_buf) 
{
	sysLOG(API_LOG_LEVEL_4, " input_len = %d\r\n", iInput_len);
    int iRet = ERR_DCEP_ERR;
	if(iInput_len < 0x00 || iInput_len > 1024|| pbInput == NULL || pbOut_buf == NULL)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	iRet = gmSm3_lib(pbInput, iInput_len, pbOut_buf);
	if(iRet == 0)
		iRet = 32;
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/**
 * sm4对称分组加密算法，加密块与秘钥均为16字节，每次加密的数据量最大为1024字节，
 * 当待加密数据大于1024字节且mode为ECB时，可以分多次调用
 *
 * @param input 	待加密数据，必须为16字节的整数倍，不大于1024字节
 * @param input_len 待加密数据长度
 * @param out_buf	加密数据缓冲区，由于sm1为对称分组算法，此缓冲区长度应大于等于待加密数据长度
 * @param pbKey 	16字节秘钥
 * @param vector	初始化向量，16字节，当mode为ECB时，可以为null
 * @param mode		模式，0为ECB解密，1为ECB加密，2为CBC解密，3位CBC加密
 * @return 大于0表示成功,返回数据长度，小于0失败
 */
int sm4(uint8* pbInput, int iInput_len, uint8* pbOut_buf, uint8* pbKey, uint8* pbVector, int iMode) 
{
	sysLOG(API_LOG_LEVEL_4, " input_len = %d, mode = %d\r\n", iInput_len, iMode);
    int iRet = ERR_DCEP_ERR;
	if(iInput_len < 0x00 || iInput_len > 1024 || iInput_len % 16 != 0 || pbInput == NULL || pbOut_buf == NULL || pbKey == NULL || iMode < 0 || iMode > 3)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	if(pbVector == NULL && (iMode == 2 || iMode == 3))
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	iRet = gmSm4_lib(pbInput, iInput_len, pbOut_buf, pbKey, pbVector, iMode);
	if(iRet == 0)
		iRet = iInput_len;
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		保存SM4密钥 
*@Param IN:		[in] index	保存的索引
				[in]key	    16字节密钥
				[in]iKeyLen	密钥长度16字节
*@Param OUT:	
*@Return:	>0成功，返回写入密钥长度，<0失败	
*/
int writeSm4Key(int iIndex, uint8* pbKey, int iKeyLen)
{
	sysLOG(API_LOG_LEVEL_4, " index = %d\r\n", iIndex);
	int iRet = ERR_DCEP_ERR;
	if ((pbKey == NULL) || (iIndex < 0) || iKeyLen != 16) {
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	if(fibo_filedir_exist(DCEP_CERT_DIR) != 1)
	{
		if(fibo_file_mkdir(DCEP_CERT_DIR) != 0)
		{
			iRet = ERR_DCEP_CERT_DIR;
			goto RET_END;
		}
	}
	char caSm4KeyName[256] = {0};
	sprintf(caSm4KeyName, "%ssm4_%d.key", DCEP_CERT_DIR, iIndex);
	iRet = fibo_file_exist(caSm4KeyName);
	if(iRet < 0)
    {
        int iFd = fileOpen_lib(caSm4KeyName, O_CREAT|O_RDWR);
		fileClose_lib(iFd);
		iRet = fibo_file_exist(caSm4KeyName);
    }
    if(iRet >= 0)
    {
	    iRet = fibo_sfile_init(caSm4KeyName);
	    if(iRet < 0)
	    {
	        iRet =  ERR_DCEP_ERR;
			goto RET_END;
	    }
	    iRet = fibo_sfile_write(caSm4KeyName, pbKey, iKeyLen);
		if(iRet != iKeyLen)
		{
			iRet = ERR_DCEP_SM4KEY_WRITE;
			goto RET_END;
		}
    }
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

int getSm4Key(int iIndex, uint8* pbKey)
{
	sysLOG(API_LOG_LEVEL_4, " index = %d\r\n", iIndex);
	int iRet = ERR_DCEP_ERR;
	if ((pbKey == NULL) || (iIndex < 0)) {
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	char caSm4KeyName[256] = {0};
	sprintf(caSm4KeyName, "%ssm4_%d.key", DCEP_CERT_DIR, iIndex);
	iRet = fibo_file_exist(caSm4KeyName);
	if(iRet < 0)
	{
		iRet = ERR_DCEP_FILE_NOTEXITST;
		goto RET_END;
	}
	int iFileSize = fibo_sfile_size(caSm4KeyName);
	if(iFileSize <= 0)
	{
		iRet = ERR_DCEP_FILE_NOTEXITST;
		goto RET_END;
	}
	if(iFileSize > 16)
		iFileSize = 16;
	iRet = fibo_sfile_init(caSm4KeyName);
	if(iRet < 0)
	{
		iRet =	ERR_DCEP_ERR;
		goto RET_END;
	}
	uint8* pbData = (uint8*)fibo_malloc(iFileSize+1);
	iRet = fibo_sfile_read(caSm4KeyName, pbData, iFileSize);
	if(iRet != iFileSize)
	{
		iRet = ERR_DCEP_SM4KEY_READ;
		fibo_free(pbData);
		goto RET_END;
	}
	if(pbKey)
	{
		memcpy(pbKey, pbData, iFileSize);
	}
	iRet = iFileSize;
	fibo_free(pbData);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		用保存的SM4密钥CBC加密 
*@Param IN:		[in]iIndex	密钥索引
				[in]iv	初始向量 16字节
				[in]data	待加密数据
				[in]iLen	数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pbOut	密文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回密文数据长度，小于0失败
*/
int sm4CBCEncryp(int iIndex, uint8* pbIv, uint8* pbData, int iLen, uint8* pbOut) 
{
	sysLOG(API_LOG_LEVEL_4, " index = %d, len = %d\r\n", iIndex, iLen);
	int iRet = ERR_DCEP_ERR;
	if(iLen < 0x00 || iLen > 1024 || iLen % 16 != 0 || pbData == NULL || pbIv == NULL || pbOut == NULL)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
    uint8 baSm4Key[20] = {0};
    iRet = getSm4Key(iIndex, baSm4Key);
	if(iRet != 16)
		goto RET_END;
	iRet = sm4(pbData, iLen, pbOut, baSm4Key, pbIv, 3);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
    return iRet;
}

/*
*@Brief:		用保存的SM4密钥CBC解密 
*@Param IN:		[in] iIndex	密钥索引
				[in]pbIv	初始向量  16字节
				[in]data	待解解数据 
				[in]len	    数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pbOut	明文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回明文数据长度，小于0失败
*/
int sm4CBCDecryp(int iIndex, uint8* pbIv, uint8* pbData, int iLen, uint8* pbOut) 
{
	sysLOG(API_LOG_LEVEL_4, " index = %d, len = %d\r\n", iIndex, iLen);
    int iRet = ERR_DCEP_ERR;
	if(iLen < 0x00 || iLen > 1024 || iLen % 16 != 0 || pbData == NULL || pbIv == NULL || pbOut == NULL)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
    uint8 baSm4Key[20] = {0};
    iRet = getSm4Key(iIndex, baSm4Key);
	if(iRet != 16)
		goto RET_END;
	iRet = sm4(pbData, iLen, pbOut, baSm4Key, pbIv, 2);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
    return iRet;
}

/*
*@Brief:		用保存的SM4密钥ECB加密 
*@Param IN:		[in] iIndex	密钥索引
				[in]data	待加密数据
				[in]iLen	数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pbOut	密文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回密文数据长度，小于0失败
*/
int sm4ECBEncryp(int iIndex, uint8* pbData, int iLen, uint8* pbOut) 
{
	sysLOG(API_LOG_LEVEL_4, " index = %d, len = %d\r\n", iIndex, iLen);
	int iRet = ERR_DCEP_ERR;
	if(iLen < 0x00 || iLen > 1024 || iLen % 16 != 0 || pbData == NULL || pbOut == NULL)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	uint8 baSm4Key[20] = {0};
	iRet = getSm4Key(iIndex, baSm4Key);
	if(iRet != 16)
		goto RET_END;
    iRet = sm4(pbData, iLen, pbOut, baSm4Key, NULL, 1);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
    return iRet;
}

/*
*@Brief:		用保存的SM4密钥ECB解密 
*@Param IN:		[in] index	密钥索引
				[in]data	待解密数据
				[in]len	数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pcOut	明文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回明文数据长度，小于0失败
*/
int sm4ECBDecryp(int iIndex, uint8* pbData, int iLen, uint8* pbOut) 
{
	sysLOG(API_LOG_LEVEL_4, " index = %d, len = %d\r\n", iIndex, iLen);
	int iRet = ERR_DCEP_ERR;
	if(iLen < 0x00 || iLen > 1024 || iLen % 16 != 0 || pbData == NULL || pbOut == NULL)
	{
		iRet = ERR_DCEP_PARAM;
		goto RET_END;
	}
	uint8 baSm4Key[20] = {0};
	iRet = getSm4Key(iIndex, baSm4Key);
	if(iRet != 16)
		goto RET_END;
	iRet = sm4(pbData, iLen, pbOut, baSm4Key, NULL, 0);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "  iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		生成硬件随机数
*@Param IN:		uiLen:  获取随机数的长度
*@Param OUT:	pbData: 随机数缓存地址
*@Return:		>0:成功,获取到的字节数; <0:失败
*/
int getRandom(uint8_t* pbData, int uiLen)
{
    int iRet = 0;

    if((pbData == NULL) || (uiLen <= 0))
    {
        return DCEP_ERR_Rand_PARAM;
    }
    iRet = fibo_rng_generate(pbData, uiLen);
    if(iRet != 0)
    {
        return DCEP_ERR_Rand_GET;
    }
    
    return uiLen;
}

int getDcepWallData(sDCEP_wall *pcDcepWallData)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32Len = 0;
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    iRet = hal_readDCEPWall(aucBalData);
    if(iRet < 0)
    {
        return -1;
    }
    i32Len = iRet;
    
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_4, "getDcepWallData desDec iRet=%d\r\n", iRet);
        return -2;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    iRet = sizeof(sDCEP_wall);
   
    memcpy(pcDcepWallData, aucOutData, i32Len);
    
    return i32Len;
}


int writeDcepWallData(unsigned char* WallParam, unsigned int lenth, int type)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32Len = 0;
    sDCEP_wall dcepWall;
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if (iRet != sizeof(sDCEP_wall))
    {
        return -1;
    }

    switch(type)
    {
        case WALLSEID:
             dcepWall.seidLen = lenth;
             memset(dcepWall.seid, 0x00, 32);
             memcpy(dcepWall.seid, WallParam, dcepWall.seidLen);
             break;
        case WALLPRTWLTID:
             dcepWall.parentWltIdLen = lenth;
             memset(dcepWall.parentWltId, 0x00, 16);
             memcpy(dcepWall.parentWltId, WallParam, dcepWall.parentWltIdLen);
            break;
        case WALLCID:
            dcepWall.CIDLen = lenth;          
            memset(dcepWall.CID, 0x00, 16);
            memcpy(dcepWall.CID, WallParam, dcepWall.CIDLen);
            break;
        case WALLWLTID:
             dcepWall.wltIDLen = lenth;
             memset(dcepWall.wltID, 0x00, 16);
             memcpy(dcepWall.wltID, WallParam, dcepWall.wltIDLen);
            break;
        case WALLHWNAME:
             dcepWall.hwNameLen = lenth;
             memset(dcepWall.hwName, 0x00, 60);
             memcpy(dcepWall.hwName, WallParam, dcepWall.hwNameLen);
            break;
        case WALLAPPTYPE:
             dcepWall.appType = *WallParam;
            break;
        case WALLWLTTXCD:
            dcepWall.wltTxnCd = *WallParam;
            break;
        case WALLTM:
             dcepWall.tmLen = lenth;
             memset(dcepWall.tm, 0x00, 8);
             memcpy(dcepWall.tm, WallParam, dcepWall.tmLen);
            break;
        case WALLLIMAMT:
             dcepWall.limAmtLen = lenth;
             memset(dcepWall.limAmt, 0x00, 6);
             memcpy(dcepWall.limAmt, WallParam, dcepWall.limAmtLen);
            break;
        case WALLWALSTATUS:
             dcepWall.walStatusLen = lenth;
             memset(dcepWall.walStatus, 0x00, 8);
             memcpy(dcepWall.walStatus, WallParam, dcepWall.walStatusLen);
            break;
        case WALLWALTYPE:
             dcepWall.walTypeLen = lenth;
             memset(dcepWall.WalType, 0x00, 8);
             memcpy(dcepWall.WalType, WallParam, dcepWall.walTypeLen);
            break;
        case WALLWALLEVEL:
             dcepWall.walLevelLen = lenth;
             memset(dcepWall.walLevel, 0x00, 8);
             memcpy(dcepWall.walLevel, WallParam, dcepWall.walLevelLen);
            break;
        case WALLINFO:
             dcepWall.infoLen = lenth;
             memset(dcepWall.info, 0x00, 64);
             memcpy(dcepWall.info, WallParam, dcepWall.infoLen);
             break;     
            default: 
             break;
    }
    
    memcpy(aucBalData, &dcepWall, sizeof(sDCEP_wall));
    i32Len = sizeof(sDCEP_wall);
    if((sizeof(sDCEP_wall) % 8) != 0)
    {
        i32Len += (8-(sizeof(sDCEP_wall) % 8));
        aucBalData[i32Len] = (8-(sizeof(sDCEP_wall) % 8));
    }

    iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_4, "setDcepWallData desEnc iRet=%d\r\n", iRet);
        return -2;
    }
    
    iRet = hal_writeDCEPWall(aucOutData, i32Len+8);
    if(iRet == (i32Len+8))
    {
        return iRet;
    }
    
    return -3;
}

/*
*@Brief:		读取硬件钱包信息
*@Param IN:		NULL
*@Param OUT:	pcSeid: 获取的pcSeid内容，长度最大32字节
*@Return:		>0:成功,读取的pcSeid长度; <0:失败
*/
int getSEId(char *pcSeid)
{
    int iRet = 0;
    int len = 0;
    sDCEP_wall dcepWall;

    if(pcSeid == NULL)
    {
        return DCEP_ERR_GET_SEID_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pcSeid, dcepWall.seid, dcepWall.seidLen);
        if(dcepWall.seidLen > 0)
        {
            return dcepWall.seidLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        } 
    }
    else
    {
        return DCEP_ERR_GET_SEID;
    }
}

/*
*@Brief:		设置硬件钱包SEID信息
*@Param IN:		pcSeid:设置的硬件钱包SEID; uiLen硬件钱包长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setSEId(char *pcSeid, uint32_t uiLen)
{
    int iRet = 0;
    int len = 0;
    if((pcSeid == NULL) && (uiLen<0) && (uiLen>32))
    {
        return DCEP_ERR_SET_SEID_PARAM;
    }
    
    iRet = writeDcepWallData(pcSeid, uiLen, WALLSEID);
    if(iRet > 0)
    {
        //len = strlen(seid);
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_SEID;
    } 
}

/*
*@Brief:		获取母钱包ID
*@Param IN:		NULL
*@Param OUT:	parentWltId: 获取的母钱包ID
*@Return:		>0:成功,母钱包ID长度; <0:失败
*/
int getParentWalletId(uint8_t* pbParentWltId)
{
    int iRet = 0;
    sDCEP_wall dcepWall;
    if(pbParentWltId == NULL)
    {
        return DCEP_ERR_GET_PARENT_WALL_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbParentWltId, dcepWall.parentWltId, dcepWall.parentWltIdLen); 
        if(dcepWall.parentWltIdLen > 0)
        {
            return dcepWall.parentWltIdLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }  
    }
    else
    {
        return DCEP_ERR_GET_PARENT_WALL;
    }
}

/*
*@Brief:		设置母钱包ID
*@Param IN:		parentWltId：母钱包ID（16字节）; uiLen:parentWltId长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setParentWalletId(uint8_t* pbParentWltId, uint32_t uiLen)
{
    int iRet = 0;
    if((pbParentWltId == NULL) &&  (uiLen<0) && (uiLen>16))
    {
        return DCEP_ERR_SET_PARENT_WALL_PARAM;
    }

    iRet = writeDcepWallData(pbParentWltId, uiLen, WALLPRTWLTID);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_PARENT_WALL;
    } 
}

/*
*@Brief:		获取钱包关联码
*@Param IN:		NULL
*@Param OUT:	pbCID: 钱包关联码
*@Return:		>0:成功,CID字节数; <0:失败
*/
int getContextId(uint8_t* pbCID)
{
    int iRet = 0;
    int len = 0;
    sDCEP_wall dcepWall;
    if(pbCID == NULL)
    {
        return DCEP_ERR_GET_CID_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbCID, dcepWall.CID, dcepWall.CIDLen);
        if(dcepWall.CIDLen > 0)
        {
            return dcepWall.CIDLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        } 
    }
    else
    {
        return DCEP_ERR_GET_CID;
    }
}

/*
*@Brief:		设置钱包关联码
*@Param IN:		pbCID：钱包关联码; uiLen:pbCID长度,uiLen<16
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setContextId(uint8_t* pbCID, uint32_t uiLen)
{
    int iRet = 0;
    if((pbCID == NULL) &&  (uiLen<0) && (uiLen>16))
    {
        return DCEP_ERR_SET_CID_PARAM;
    }
    
    iRet = writeDcepWallData(pbCID, uiLen, WALLCID);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_CID;
    } 
}

/*
*@Brief:		获取钱包ID
*@Param IN:		NULL
*@Param OUT:	pbWltID: 读取到的硬件钱包ID
*@Return:		>0:成功,钱包ID长度; <0:失败
*/
int getWalletId(uint8_t* pbWltID)
{
    int iRet = 0;
    sDCEP_wall dcepWall;
    if(pbWltID == NULL)
    {
        return DCEP_ERR_GET_WALL_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbWltID, dcepWall.wltID, dcepWall.wltIDLen);
        if(dcepWall.wltIDLen > 0)
        {
            return dcepWall.wltIDLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        } 
    }
    else
    {
        return DCEP_ERR_GET_WALL;
    }
}

/*
*@Brief:		设置钱包ID
*@Param IN:		pbWltID: 钱包ID; uiLen:pbWltID长度，uiLen<16
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletId(uint8_t* pbWltID, uint32_t uiLen)
{
    int iRet = 0;
    if((pbWltID == NULL) &&  (uiLen<0) && (uiLen>16))
    {
        return DCEP_ERR_SET_WALL_PARAM;
    }

    iRet = writeDcepWallData(pbWltID, uiLen, WALLWLTID);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_WALL;
    } 
}

/*
*@Brief:		获取钱包名字
*@Param IN:		NULL
*@Param OUT:	pcHwName: 钱包名,最大60字节
*@Return:		>0:成功,返回钱包名字字节数; <0:失败
*/
int getWalletName(char *pcHwName)
{
    int iRet = 0;
    int len = 0;
    sDCEP_wall dcepWall;
    if(pcHwName == NULL)
    {
        return DCEP_ERR_GET_WALLET_NAME_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pcHwName, dcepWall.hwName, dcepWall.hwNameLen);
        if(dcepWall.hwNameLen > 0)
        {
            return dcepWall.hwNameLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        } 
    }
    else
    {
        return DCEP_ERR_GET_WALLET_NAME;
    }
}

/*
*@Brief:		设置钱包名
*@Param IN:		pcHwName: 钱包名,最大60字节;   uiLen：pcHwName长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletName(char *pcHwName, uint32_t uiLen)
{
    int iRet = 0;
    if((pcHwName == NULL) &&  (uiLen<0) && (uiLen>60))
    {
        return DCEP_ERR_SET_WALLET_NAME_PARAM;
    }

    iRet = writeDcepWallData(pcHwName, uiLen, WALLHWNAME);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_WALLET_NAME;
    } 
}

/*
*@Brief:		获取应用标识
*@Param IN:		NULL
*@Param OUT:	pAppType: 标识内容
*@Return:		>0:成功,返回标识内容字节数; <0:失败
*/
int getAppType(uint8_t* pbAppType)
{
    int iRet = 0;
    sDCEP_wall dcepWall;
    if(pbAppType == NULL)
    {
        return DCEP_ERR_GET_APPTYPE_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        *pbAppType = dcepWall.appType;
        
         return 1;
    }
    else
    {
        return DCEP_ERR_GET_APPTYPE;
    }
}

/*
*@Brief:		设置应用标识
*@Param IN:		bAppType：应用方标识,1字节
*@Param OUT:	NULL
*@Return:		=0:成功,返回写入字节个数; <0:失败
*/
int setAppType(uint8_t bAppType)
{
    int iRet = 0;

    iRet = writeDcepWallData(&bAppType, 0, WALLAPPTYPE);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_APPTYPE;
    }
}

/*
*@Brief:		获取钱包设备种类
*@Param IN:		NULL
*@Param OUT:	pbWltTxnCd: 设备种类（1字节）
*@Return:		>0:成功,返回钱包设备种类字节数; <0:失败
*/
int getWltTxnCd(uint8_t* pbWltTxnCd)
{
    int iRet = 0;
    sDCEP_wall dcepWall;

    if(pbWltTxnCd == NULL)
    {
        return DCEP_ERR_GET_WLT_TXN_PARAM;
    }
    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));

    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        *pbWltTxnCd = dcepWall.wltTxnCd;
        return 1;
    }
    else
    {
        return DCEP_ERR_GET_WLT_TXN_CD;
    }
}

/*
*@Brief:		设置钱包设备种类
*@Param IN:		bWltTxnCds：钱包设备种类,1字节
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWltTxnCd(uint8_t bWltTxnCd)
{
    int iRet = 0;

    iRet = writeDcepWallData(&bWltTxnCd, 0, WALLWLTTXCD);
    if(iRet > 0)
    {
         return 0;
    }
    else
    {
        return DCEP_ERR_SET_WLT_TXN_CD;
    }   
}

/*
*@Brief:		获取钱包开立时间
*@Param IN:		NULL
*@Param OUT:	pbTm: 钱包开立时间(ISO 时间：YYYYMMDDHHMMSS)
*@Return:		>0:成功,返回时间节数; <0:失败
*/
int getCreateTime(uint8_t * pbTm)
{
    int iRet = 0;
    int i = 0;
    sDCEP_wall dcepWall;

    if(pbTm == NULL)
    {
        return DCEP_ERR_GET_WLT_CRT_TIME_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));
    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
      memcpy(pbTm, dcepWall.tm, dcepWall.tmLen);
      if(dcepWall.tmLen > 0)
      {
          return dcepWall.tmLen;
      }
      else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }
    }
    else
    {
        return DCEP_ERR_GET_WLT_CRT_TIME;
    }
}

/*
*@Brief:		设置钱包开立时间
*@Param IN:		pbTm：钱包开立时间(ISO 时间：YYYYMMDDHHMMSS); uiLen：pbTm长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setCreateTime(uint8_t * pbTm, uint32_t uiLen)
{
    int iRet = 0;
    if((pbTm == NULL) && (uiLen<0) && (uiLen>8))
    {
        return DCEP_ERR_SET_WLT_CRT_TIME_PARAM;
    }

    iRet = writeDcepWallData(pbTm, uiLen, WALLTM);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_WLT_CRT_TIME;
    } 
}

/*
*@Brief:		获取钱包限额
*@Param IN:		NULL
*@Param OUT:	pbLimAmt：钱包额度（6字节）
*@Return:		>0:成功,返回字节数; <0:失败
*/
int getWalletLimit(uint8_t* pbLimAmt)
{
    int iRet = 0;
    int i = 0;
    sDCEP_wall dcepWall;

    if(pbLimAmt == NULL)
    {
        return DCEP_ERR_GET_LIMAMT_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));
    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbLimAmt, dcepWall.limAmt, dcepWall.limAmtLen);
        if(dcepWall.limAmtLen > 0)
        {
            return dcepWall.limAmtLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }
    }
    else
    {
        return DCEP_ERR_GET_LIMAMT;
    }
}

/*
*@Brief:		设置钱包额度
*@Param IN:		pbLimAmt：钱包限额; uiLen: pbLimAmt长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletLimit(uint8_t * pbLimAmt, uint32_t uiLen)
{
    int iRet = 0;

    if((pbLimAmt == NULL) && (uiLen<0) && (uiLen>6))
    {
        return DCEP_ERR_SET_LIMAMT_PARAM;
    }

    iRet = writeDcepWallData(pbLimAmt, uiLen, WALLLIMAMT);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_LIMAMT;
    } 
}

/*
*@Brief:		获取钱包状态
*@Param IN:		NULL
*@Param OUT:	pbWalStatus:钱包状态
*@Return:		>0:成功; <0:失败
*/
int getWalletStatus(uint8_t* pbWalStatus)
{
    int iRet = 0;
    sDCEP_wall dcepWall;

    if(pbWalStatus == NULL)
    {
        return DCEP_ERR_GET_WAL_STATUS_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));
    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbWalStatus, dcepWall.walStatus, dcepWall.walStatusLen);
        if(dcepWall.walStatusLen > 0)
        {
             return dcepWall.walStatusLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }
       
    }
    else
    {
        return DCEP_ERR_GET_WAL_STATUS;
    }
}

/*
*@Brief:		设置钱包状态
*@Param IN:		pbWalStatus：钱包状态; uiLen：pbWalStatus长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletStatus(uint8_t * pbWalStatus, uint32_t uiLen)
{
    int iRet = 0;
   
    if((pbWalStatus == NULL) && (uiLen<0) && (uiLen>8))
    {
        return DCEP_ERR_SET_WAL_STATUS_PARAM;
    }
    iRet = writeDcepWallData(pbWalStatus, uiLen, WALLWALSTATUS);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_WAL_STATUS;
    } 
}

/*
*@Brief:		获取钱包类型
*@Param IN:		NULL
*@Param OUT:	pbWalType：钱包类型
*@Return:		>0:成功; <0:失败
*/
int getWalletType(uint8_t* pbWalType)
{
    int iRet = 0;
    sDCEP_wall dcepWall;

    if(pbWalType == NULL)
    {
        return DCEP_ERR_GET_WAL_TYPE_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));
    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbWalType, dcepWall.WalType, dcepWall.walTypeLen);
        if(dcepWall.walTypeLen > 0)
        {
            return dcepWall.walTypeLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }
        
    }
    else
    {
        return DCEP_ERR_GET_WAL_TYPE;
    }
}

/*
*@Brief:		设置钱包类型
*@Param IN:		pbWalType：钱包类型; uiLen:pbWalType长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletType(uint8_t* pbWalType, uint32_t uiLen)
{
    int iRet = 0;

    if((pbWalType == NULL) && (uiLen<0) && (uiLen>8))
    {
        return DCEP_ERR_SET_WAL_TYPE_PARAM;
    }

    iRet = writeDcepWallData(pbWalType, uiLen, WALLWALTYPE);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_WAL_TYPE;
    } 
}

/*
*@Brief:		获取钱包等级
*@Param IN:		NULL
*@Param OUT:	pbWalLevel：钱包等级
*@Return:		>0:成功; <0:失败
*/
int getWalletLevel(uint8_t* pbWalLevel)
{
    int iRet = 0;
    sDCEP_wall dcepWall;

    if(pbWalLevel == NULL)
    {
        return DCEP_ERR_GET_WAL_LEVEL_PARAM;
    }

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));
    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbWalLevel, dcepWall.walLevel, dcepWall.walLevelLen);
        if(dcepWall.walLevelLen > 0)
        {
            return dcepWall.walLevelLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }
        
    }
    else
    {
        return DCEP_ERR_GET_WAL_LEVEL;
    }
}

/*
*@Brief:		设置钱包等级
*@Param IN:		pbWalLevel：钱包等级; uiLen：pbWalLevel长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletLevel(uint8_t *pbWalLevel, uint32_t uiLen)
{
    int iRet = 0;
    if((pbWalLevel == NULL) && (uiLen < 0) && (uiLen > 8))
    {
        return DCEP_ERR_SET_WAL_LEVEL_PARAM;
    }

    iRet = writeDcepWallData(pbWalLevel, uiLen, WALLWALLEVEL);
    if(iRet > 0)
    {
        return 1;
    }
    else
    {
        return DCEP_ERR_SET_WAL_LEVEL;
    }
}

/*
*@Brief:		获取钱包控制信息
*@Param IN:		NULL
*@Param OUT:	pbInfo：钱包控制信息
*@Return:		>0:成功,返回钱包控制信息字节数; <0:失败
*/
int getCtrlInfo(uint8_t* pbInfo)
{
    int iRet = 0;
    sDCEP_wall dcepWall;

    memset(&dcepWall, 0x00, sizeof(sDCEP_wall));
    iRet = getDcepWallData(&dcepWall);
    if(iRet == sizeof(sDCEP_wall))
    {
        memcpy(pbInfo, dcepWall.info, dcepWall.infoLen);
        if(dcepWall.infoLen > 0)
        {
            return dcepWall.infoLen;
        }
        else{
            return DCEP_ERR_GET_DATA_NON_EXIST;
        }
    }
    else
    {
        return DCEP_ERR_GET_WAL_INFO;
    }
}

/*
*@Brief:		设置钱包控制信息
*@Param IN:		info：钱包控制信息;uiLen:info长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setCtrlInfo(uint8_t* pbInfo, uint32_t uiLen)
{
    int iRet = 0;
    int len = 0;

    if((pbInfo == NULL) && (uiLen < 0) && (uiLen > 64))
    {
        return DCEP_ERR_SET_WAL_INFO_PARAM;
    }

    iRet = writeDcepWallData(pbInfo, uiLen, WALLINFO);
    if(iRet > 0)
    {
        return 0;
    }
    else
    {
        return DCEP_ERR_SET_WAL_INFO;
    } 
}

int readBalVoucherAdmblnFile(uint32_t *voucherInfo, sDCEP_VoucherHead* VoucherHead)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    unsigned char ucLabel[]="AA55AA5";
    int iRet = 0;
    int i32FileSize = 0;
    int i32Len = 0;

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    i32FileSize = fibo_sfile_size(ADMBLNFILENAME);
    if(i32FileSize < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return i32FileSize;
    }

    iRet = fibo_sfile_read(ADMBLNFILENAME, aucBalData, i32FileSize);
    if(iRet != i32FileSize)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return iRet; 
    }
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_4, "desDec iRet=%d\r\n", iRet);
        return ;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    memcpy(VoucherHead, aucOutData, i32Len);
   
    i32FileSize = fibo_sfile_size(LISTBLNFILENAME);
    if(i32FileSize < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return i32FileSize;
    }
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));
    iRet = fibo_sfile_read(LISTBLNFILENAME, aucBalData, i32FileSize);
    if(iRet != i32FileSize)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return iRet; 
    }
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_4, "desDec iRet=%d\r\n", iRet);
        return iRet;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    memcpy(voucherInfo, aucOutData, i32Len);
    return 0;
}

int writeBalVoucherAdmblnFile(DCEP_pwrDownOpt sPwrDownOpt, uint32_t *voucherInfo, sDCEP_VoucherHead VoucherHead)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    unsigned char ucLabel[]="AA55AA5";
    int iRet = 0;
    int i32Len = 0;
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    memcpy(aucBalData, voucherInfo, LISTBLNFILELEN);
    i32Len = LISTBLNFILELEN;
    if(LISTBLNFILELEN%8 != 0)
    {
        i32Len += (8-(LISTBLNFILELEN % 8));
        aucBalData[i32Len] = (8-(LISTBLNFILELEN % 8));
    }
    
    iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit;
    }

    iRet = fibo_sfile_write(LISTBLNFILENAME, aucOutData, i32Len+8);
    if(iRet != i32Len+8)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit; 
    }

    if(sPwrDownOpt.voucherSaveOpt > 0)
    {
        sPwrDownOpt.voucherSaveOpt = 2;
    }
    else if(sPwrDownOpt.voucherVGOpt > 0)
    {
        sPwrDownOpt.voucherVGOpt = 2;
    }
    else if(sPwrDownOpt.voucherDelAllOpt > 0)
    {
        sPwrDownOpt.voucherDelAllOpt = 2;
    }
    sPwrDownOpt.voucherFileCnt = VoucherHead.blnTtlCnt;
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(API_LOG_LEVEL_4, "saveError iRet=%d\r\n",iRet);
        goto Exit;
    }

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));
    
    memcpy(aucBalData, &VoucherHead, sizeof(sDCEP_VoucherHead));
    i32Len = sizeof(sDCEP_VoucherHead);
    if(sizeof(sDCEP_VoucherHead)%8 != 0)
    {
        i32Len += (8-(sizeof(sDCEP_VoucherHead) % 8));
        aucBalData[i32Len] = (8-(sizeof(sDCEP_VoucherHead) % 8));
    }

    iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit;
    }
    iRet = fibo_sfile_write(ADMBLNFILENAME, aucOutData, i32Len+8);
    if(iRet != i32Len+8)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit; 
    }

    if(sPwrDownOpt.voucherVGOpt > 0)
    {
        if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
        {
            iRet = fileRemove_lib(sPwrDownOpt.PDOptFileName);
            if(iRet != 0)
            {
                sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
                goto Exit; 
            }
        }
        sPwrDownOpt.voucherVGOpt = 0;
    }
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(API_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -1;
    }
    return 0;

Exit: 
    if(sPwrDownOpt.voucherSaveOpt > 0)
    {
        fileRemove_lib(sPwrDownOpt.PDOptFileName);
        sPwrDownOpt.voucherSaveOpt = 0;
    }
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(API_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -2;
    }
    return -3;
}

/*
*@Brief:		保存一个余额凭证
*@Param IN:		bMode：0：余额凭证，1：币串;pbBalVoucher:余额凭证数据; uiLen:存储长度
*@Param OUT:	NULL
*@Return:		>=:成功; <0:失败
*/
int saveBalVoucher(uint8_t bMode, uint8_t * pbBalVoucher, uint32_t uiLen)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    unsigned char ucLabel[]="AA55AA5";
    int iRet = 0;
    int i32Ret = 0;
    int i32RemLen = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    if((pbBalVoucher == NULL) || (uiLen <= 0) || ((bMode != 1) && (bMode != 0)))
    {
        return DCEP_ERR_SAVE_BAL_VOUC_PARAM;
    }
    i32RemLen = (uiLen % 8);
    memcpy(aucBalData, pbBalVoucher, uiLen);
    if(i32RemLen != 0)
    {
        //sysLOG(BASE_LOG_LEVEL_1, "uiLen = %d\r\n", uiLen);
        uiLen += (8-i32RemLen);
        aucBalData[uiLen] = (8-i32RemLen);
        //sysLOG(BASE_LOG_LEVEL_1, "uiLen = %d %d\r\n", uiLen,aucBalData[uiLen]);
    }
    else{
        aucBalData[uiLen] = 0;
    }

    iRet = calcTdesEnc_lib(aucBalData, uiLen+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_4, "saveBalVoucher desEnc iRet=%d\r\n", iRet);
        return DCEP_ERR_SAVE_BAL_VOUC;
    }

    if(bMode == 1)
    {
        iRet = hal_saveCurrencyString(aucOutData, uiLen+8);
        if(iRet != 0)
        {
            return DCEP_ERR_SAVE_CS;
        }
		return 0;
    }
    else if(bMode == 0)
    {
        iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
        if(iRet == 0)
        {
            iRet = hal_saveBalVoucher(&sPwrDownOpt, voucherInfo, &VoucherHead, aucOutData, uiLen+8);
            if(iRet >= 0)
            {
                voucherInfo[VoucherHead.blnTtlCnt] = iRet;
                VoucherHead.blnTtlCnt += 1;
                i32Ret = writeBalVoucherAdmblnFile(sPwrDownOpt, voucherInfo, VoucherHead);
                if(i32Ret == 0)
                {
                    return iRet;
                } 
            }
        }
        return DCEP_ERR_SAVE_BAL_VOUC;
    }
    else{
        return DCEP_ERR_SAVE_BAL_VOUC_PARAM;
    }
}

/*
*@Brief:		读取指定位置的余额凭证
*@Param IN:		bMode：0：余额凭证，1：币串; uiIndex：索引
*@Param OUT:	pbBalData:余额凭证数据
*@Return:		>0:成功,余额凭证数据长度; 其他:失败
*/
int getBalVoucher(uint8_t bMode, uint32_t uiIndex, uint8_t* pbBalData)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32Len = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];

    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

     if(pbBalData == NULL)
     {
            return DCEP_ERR_GET_BAL_VOUC_PARAM;
     }

    if(bMode == 1)
    {
        iRet = hal_getCurrencyString(aucBalData);
        if(iRet <= 0)
        {
            return DCEP_ERR_GET_CS;
        }
    }
    else if(bMode == 0)
    {
        iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
        if(iRet != 0)
        {
            return DCEP_ERR_GET_BAL_VOUC;
        }

        iRet = hal_getBalVoucher(uiIndex, voucherInfo, VoucherHead, aucBalData);
        if(iRet <= 0)
        {
            return DCEP_ERR_GET_BAL_VOUC;
        }
    }
    else{
        return DCEP_ERR_GET_BAL_VOUC_PARAM;
    }
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "getBalVoucher desDec iRet=%d\r\n", iRet);
        return DCEP_ERR_GET_BAL_VOUC;;
    }
    //sysLOG(BASE_LOG_LEVEL_1, "i32Len = %d %d\r\n", i32Len,aucOutData[i32Len-8]);
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    //sysLOG(BASE_LOG_LEVEL_1, "i32Len = %d\r\n", i32Len);
    memcpy(pbBalData, aucOutData, i32Len);

    return i32Len;
}

/*
*@Brief:		更新余额凭证
*@Param IN:		uiIndex:索引；pbBalVoucher：更新数据；uiLen:更新数据长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setBalVoucher(uint32_t uiIndex, uint8_t *pbBalVoucher, uint32_t uiLen)
{
	unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32RemLen = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];

    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

	if(pbBalVoucher == NULL)
	{
		return  DCEP_ERR_SET_BAL_PARAM;
	}
    i32RemLen = (uiLen % 8);
    memcpy(aucBalData, pbBalVoucher, uiLen);
    if(i32RemLen != 0)
    {
        uiLen += (8-i32RemLen);
        aucBalData[uiLen] = (8-i32RemLen);
    }
    else{
        aucBalData[uiLen] = 0;
    }

    iRet = calcTdesEnc_lib(aucBalData, uiLen+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "setBalVoucher desEnc iRet=%d\r\n", iRet);
        return DCEP_ERR_SET_BAL;
    }

    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
    if (iRet != 0)
    {
        return DCEP_ERR_SET_BAL;
    }

    iRet = hal_setBalVoucher(uiIndex, voucherInfo, VoucherHead, aucOutData, uiLen+8);
	if(iRet < 0)
	{
		return DCEP_ERR_SET_BAL;
	}
	return 0;
}

/*
*@Brief:		获取余额凭证个数
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功,余额凭证个数; <0:失败
*/
int getBalVoucherCount()
{
    int iRet = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];

    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
    if (iRet != 0)
    {
        return DCEP_ERR_GET_BAL_COUNT;
    }

    iRet = hal_getBalVoucherData(0, VoucherHead);
    if(iRet < 0)
    {
        return DCEP_ERR_GET_BAL_COUNT;
    }
    return iRet;
}

/*
*@Brief:		获取余额凭证索引表
*@Param IN:		NULL
*@Param OUT:	iIndexTable：索引表
*@Return:		>0:成功; <0:失败
*/
int getBalVoucherIndexTable(int32_t* iIndexTable)
{
    int iRet = 0;
    int i=0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];

    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
    if (iRet != 0)
    {
        return DCEP_ERR_BAL_GET_INDEX;
    }

    if(iIndexTable == NULL)
    {
        return DCEP_ERR_BAL_GET_INDEX_PARAM;
    }

    iRet = hal_getBalVoucherIndexTable(iIndexTable, voucherInfo, VoucherHead);
    if(iRet < 0)
    {
        return DCEP_ERR_BAL_GET_INDEX;
    }

    return iRet;
}

/*
*@Brief:		获取余额
*@Param IN:		NULL
*@Param OUT:	pbBalData:余额
*@Return:		>0：成功,返回余额字节数; <:失败
*/
int getBalance(uint8_t*  pbBalData)
{
    /*uint64_t iRet = 0;
    char balData[8]={0,0,0,0,0,0};

    iRet = hal_getBalVoucherData(1, index);
    if(iRet < 0)
    {
        return DCEP_ERR_GET_BAL;
    }

    balData[0] = iRet & 0xFF;
    balData[1] = (iRet >> 8) & 0xFF;
    balData[2] = (iRet >> 16) & 0xFF;
    balData[3] = (iRet >> 24) & 0xFF;
    balData[4] = (iRet >> 32) & 0xFF;
    balData[5] = (iRet >> 40) & 0xFF;

    return balData;*/
    int iRet = 0;

    if(pbBalData == NULL)
    {
        return DCEP_ERR_GET_BAL_PARAM;
    }

    iRet = hal_dcepGetBalance(pbBalData);
    if(iRet != 0)
    {
        return DCEP_ERR_GET_BAL;
    }
    return 6;
}

/*
*@Brief:		删除指定位置的余额凭证
*@Param IN:		uiIndex：索引
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteBalVoucher(uint32_t uiIndex)
{
    int iRet = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];
    uint32_t voucherInfoTrs[MAXBALAMOUNT];
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(voucherInfoTrs, 0x00, sizeof(voucherInfoTrs));
    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    sysLOG(API_LOG_LEVEL_4, " index=%d\r\n", uiIndex);
    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
    if(iRet != 0)
    {
        return DCEP_ERR_DEL_BAL_VOUC;
    }

    iRet = hal_deletBalVoucher(uiIndex, &sPwrDownOpt, voucherInfo, &VoucherHead);
    if(iRet < 0)
    {
        return DCEP_ERR_DEL_BAL_VOUC;
    }
    memcpy(voucherInfoTrs, &voucherInfo[iRet+1], (VoucherHead.blnTtlCnt-iRet)*sizeof(uint32_t));
    memcpy(&voucherInfo[iRet], voucherInfoTrs, (VoucherHead.blnTtlCnt-iRet)*sizeof(uint32_t));
    iRet = writeBalVoucherAdmblnFile(sPwrDownOpt, voucherInfo, VoucherHead);
    if(iRet != 0)
    {
        return DCEP_ERR_DEL_BAL_VOUC;
    }
    return iRet;
}

/*
*@Brief:		删除全部余额凭证
*@Param IN:		bMode：0：余额凭证，1：币串
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteAllBalVoucher(uint8_t bMode)
{
    int iRet = 0;

    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 
    if((bMode != 0) && (bMode != 1))
    {
        return DCEP_ERR_DEL_BAL_VOUC_PARAM;
    }

    if(bMode == 1)
    {
        iRet = hal_delCurrencyString();
        if(iRet < 0)
        {
            return DCEP_ERR_DEL_CS;
        }
    }
    else if(bMode == 0)
    {
        iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
        if(iRet != 0)
        {
             return DCEP_ERR_DEL_ALL_BAL_VOUC;
        }

        iRet = hal_deleteAllBalVoucher(&sPwrDownOpt,voucherInfo, &VoucherHead);
        if(iRet < 0)
        {
            return DCEP_ERR_DEL_ALL_BAL_VOUC;
        }
        memset(voucherInfo, 0x00, sizeof(voucherInfo));
        iRet = writeBalVoucherAdmblnFile(sPwrDownOpt, voucherInfo, VoucherHead);
        if(iRet != 0)
        {
            return DCEP_ERR_DEL_ALL_BAL_VOUC;
        }
    }
    else{
        return DCEP_ERR_DEL_BAL_VOUC_PARAM;
    }

    return 0;
}

int readBillAdmblnFile(uint32_t *billInfo, sDCEP_BillHead *BillHead)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    unsigned char ucLabel[]="AA55AA5";
    int iRet = 0;
    int i32FileSize = 0;
    int i32Len = 0;
    
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    i32FileSize = fibo_sfile_size(LISTBILLFILENAME);
    if(i32FileSize < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return i32FileSize;
    }

    iRet = fibo_sfile_read(LISTBILLFILENAME, aucBalData, i32FileSize);
    if(iRet != i32FileSize)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return iRet; 
    }
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return -1;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    memcpy(billInfo, aucOutData, i32Len);

    i32FileSize = fibo_sfile_size(ADMBILLFILE);
    if(i32FileSize < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return i32FileSize;
    }
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));
    iRet = fibo_sfile_read(ADMBILLFILE, aucBalData, i32FileSize);
    if(iRet != i32FileSize)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return iRet; 
    }
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return -2;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    memcpy(BillHead, aucOutData, i32Len);
    return 0;
}

int writeBillAdmblnFile(DCEP_pwrDownOpt sPwrDownOpt, uint32_t *billInfo, sDCEP_BillHead BillHead)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    unsigned char ucLabel[]="AA55AA5";
    int iRet = 0;
    int i32Len = 0;
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    memcpy(aucBalData, billInfo, LISTBILLFILELEN);
    i32Len = LISTBILLFILELEN;
    if((LISTBILLFILELEN%8) != 0)
    {
        i32Len += (8-(LISTBILLFILELEN % 8));
        aucBalData[i32Len] = (8-(LISTBILLFILELEN % 8));
    }

    iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit;
    }

    iRet = fibo_sfile_write(LISTBILLFILENAME, aucOutData, i32Len+8);
    if(iRet != i32Len+8)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit; 
    }

    if(sPwrDownOpt.billSaveOpt > 0)
    {
        sPwrDownOpt.billSaveOpt = 2;
    }
    else if(sPwrDownOpt.billVGOpt > 0)
    {
        sPwrDownOpt.billVGOpt = 2;
    }
    else if(sPwrDownOpt.billDelAllOpt >  0)
    {
        sPwrDownOpt.billVGOpt = 2;
    }
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(API_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        goto Exit;
    }
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    memcpy(aucBalData, &BillHead, sizeof(sDCEP_BillHead));
    i32Len = sizeof(sDCEP_BillHead);
    if(sizeof(sDCEP_BillHead)%8 != 0)
    {
        i32Len += (8-(sizeof(sDCEP_BillHead) % 8));
        aucBalData[i32Len] = (8-(sizeof(sDCEP_BillHead) % 8));
    }

    iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit;
    }
    iRet = fibo_sfile_write(ADMBILLFILE, aucOutData, i32Len+8);
    if(iRet != i32Len+8)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        goto Exit; 
    }

    if(sPwrDownOpt.billVGOpt > 0)
    {
        if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
        {
            iRet = fileRemove_lib(sPwrDownOpt.PDOptFileName);
            if(iRet != 0)
            {
                sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
                goto Exit; 
            }
        }
        sPwrDownOpt.billVGOpt = 0;
    }
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(API_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -1;
    }
    return 0;
Exit:
    if(sPwrDownOpt.billSaveOpt > 0)
    {
        fileRemove_lib(sPwrDownOpt.PDOptFileName);
        sPwrDownOpt.billSaveOpt = 0;
    }
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(API_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -2;
    }
    return -3;
}

/*
*@Brief:		保存交易记录
*@Param IN:		pbBill:交易记录数据; uiLen:交易记录长度
*@Param OUT:	NULL
*@Return:		>=0:成功,返回索引号; <0:失败
*/
int saveBill(uint8_t* pbBill, uint32_t uiLen)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32Ret = 0;
    int i32RemLen = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    if((pbBill == NULL) && (uiLen <= 0))
    {
        return DCEP_ERR_BILL_SAVE_PARAM;
    }
    i32RemLen = uiLen % 8;
    memcpy(aucBalData, pbBill, uiLen);
    if(i32RemLen != 0)
    {
        uiLen += (8-i32RemLen);
        aucBalData[uiLen] = (8-i32RemLen);
    }
    else{
        aucBalData[uiLen] = 0;
    }

    iRet = calcTdesEnc_lib(aucBalData, uiLen+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "saveBill desEnc iRet=%d\r\n", iRet);
        return DCEP_ERR_BILL_SAVE;
    }

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_BILL_SAVE;
    }

    iRet = hal_saveBill(&sPwrDownOpt, billInfo, &BillHead, aucOutData, uiLen+8);
    if(iRet >= 0)
    {
        i32Ret =  writeBillAdmblnFile(sPwrDownOpt, billInfo, BillHead);
        if(i32Ret != 0)
        {
            return DCEP_ERR_BILL_SAVE;
        }
        return iRet;
    }
    else
    {
        return DCEP_ERR_BILL_SAVE;
    }
}

/*
*@Brief:		读取交易记录
*@Param IN:		uiPos：索引
*@Param OUT:	pbBillData:交易记录数据
*@Return:		>0:成功,交易记录数据长度; 其他:失败
*/
int getBill(uint32_t uiPos, uint8_t* pbBillData)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32Len = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;

    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    if(pbBillData == NULL)
    {
        return DCEP_ERR_BILL_GET_PARAM;
    }

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_BILL_GET;
    }

    iRet = hal_getBill(uiPos, billInfo, BillHead, aucBalData);
    if(iRet < 0)
    {
        return DCEP_ERR_BILL_GET;
    }
    
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "getBalVoucher desDec iRet=%d\r\n", iRet);
        return DCEP_ERR_BILL_GET;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    memcpy(pbBillData, aucOutData, i32Len);

    return i32Len;
}

/*
*@Brief:		获取交易记录个数
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功,交易记录个数; <0:失败
*/
int getBillCount()
{
    int iRet = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;

    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_BILL_GET_COUNT;
    }

    return billInfo[0];
}

/*
*@Brief:		获取交易记录索引表
*@Param IN:		NULL
*@Param OUT:	iIndexTable：索引表
*@Return:		>0:成功; <0:失败
*/
int getBillIndexTable(int32_t* iIndexTable)
{
    int iRet = 0;
    int i=0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;

    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    if(iIndexTable == NULL)
    {
        return DCEP_ERR_BILL_GET_INDEX_PARAM;
    }

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_BILL_GET_INDEX;
    }

    if(billInfo[0] > 0)
    {
        memcpy(iIndexTable, &billInfo[1], billInfo[0]*sizeof(int));
        return billInfo[0];
    }
    else{
        return DCEP_ERR_BILL_GET_INDEX;
    }
}

/*
*@Brief:		更新交易记录
*@Param IN:		uiPos：索引；pbBill:交易记录数据；uiLen：交易记录数据长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setBill(uint32_t uiPos, uint8_t* pbBill, uint32_t uiLen)
{
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32RemLen = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;

    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    if (pbBill == NULL)
    {
        return DCEP_ERR_SET_BILL_PARAM;
    }
    i32RemLen = uiLen % 8;
    memcpy(aucBalData, pbBill, uiLen);
    if (i32RemLen != 0)
    {
        uiLen += (8 - i32RemLen);
        aucBalData[uiLen] = (8 - i32RemLen);
    }
    else
    {
        aucBalData[uiLen] = 0;
    }

    iRet = calcTdesEnc_lib(aucBalData, uiLen + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if (iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "saveBill desEnc iRet=%d\r\n", iRet);
        return DCEP_ERR_SET_BILL;
    }

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_SET_BILL;
    }

   iRet = hal_setBill(uiPos, billInfo, aucOutData, uiLen+8);
   if(iRet < 0)
   {
	   return DCEP_ERR_SET_BILL;
   }

   return 0;
}

/*
*@Brief:		删除指定位置的交易记录
*@Param IN:		uiPos：索引
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteBill(uint32_t uiPos)
{
    int iRet = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_BILL_DEL;
    }

    iRet = hal_deleteBill(uiPos, &sPwrDownOpt, billInfo, &BillHead);
    if(iRet != 0)
    {
        return DCEP_ERR_BILL_DEL;
    }

    iRet =  writeBillAdmblnFile(sPwrDownOpt, billInfo, BillHead);
    if(iRet != 0)
    {
        return DCEP_ERR_BILL_DEL;
    }
    return iRet;
}

/*
*@Brief:		删除所有交易记录
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteAllBill()
{
    int iRet = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    iRet = readBillAdmblnFile(billInfo, &BillHead);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "Error=%d\r\n", iRet);
        return DCEP_ERR_BILL_DEL_ALL;
    }

    iRet = hal_deleteAllBill(&sPwrDownOpt, billInfo, &BillHead);
    if(iRet != 0)
    {
        return DCEP_ERR_BILL_DEL_ALL;
    }

    iRet =  writeBillAdmblnFile(sPwrDownOpt, billInfo, BillHead);
    if(iRet != 0)
    {
        return DCEP_ERR_BILL_DEL_ALL;
    }
    return iRet;
}

int dcepInit()
{
    int i32Ret = 0;

    i32Ret = hal_dcepWallInit();
    if(i32Ret < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "dcepWallInit Error\r\n");
        return -1;
    }

    i32Ret = hal_voucherInit();
    if(i32Ret < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "voucherInit Error\r\n");
        return -2;
    }

    i32Ret = hal_billdcepInit();
    if(i32Ret < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "billdcepInit Error\r\n");
        return -3;
    }

    i32Ret = hal_atcInit();
    if(i32Ret < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "atcInit Error\r\n");
        return -3;
    }

    i32Ret = hal_backupListFileInit();
    if(i32Ret < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "backupListFileInit Error = %d\r\n", i32Ret);
        return -3;
    }

    i32Ret = hal_pwrDownOpt();
    if(i32Ret < 0)
    {
        sysLOG(API_LOG_LEVEL_2, "pwrDownOpt Error = %d\r\n", i32Ret);
        return -3;
    }
    return 0;
}

/*
*@Brief:		写入ATC值
*@Param IN:		atc：atc值
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setATC(long lAtc)
{
    unsigned char aucBalData[24];
    unsigned char aucOutData[24];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";

    int iRet = 0;
    int i32Len = 0;
    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    memcpy(aucBalData, &lAtc, sizeof(long));
    i32Len = sizeof(long);
    if ((sizeof(long) % 8) != 0)
    {
        i32Len += (8 - ((sizeof(long) % 8)));
        aucBalData[i32Len] = (8 - ((sizeof(long) % 8)));
    }
    else
    {
        i32Len = 8;
        aucBalData[i32Len] = 0;
    }

    iRet = calcTdesEnc_lib(aucBalData, i32Len + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if (iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "saveATC desEnc iRet=%d\r\n", iRet);
        return DCEP_ERR_ATC_SET;
    }

    iRet = hal_setATC(aucOutData, i32Len + 8);
    if(iRet != 0)
    {
        return DCEP_ERR_ATC_SET;
    }
    return iRet;
}

/*
*@Brief:		获取ATC值
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功，ATC值; <0:失败
*/
long getATC()
{
    unsigned char aucBalData[24];
    unsigned char aucOutData[24];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";

    int iRet = 0;
    int i32Len = 0;
    long l32Atc = 0;

    memset(aucBalData, 0x00, sizeof(aucBalData));
    memset(aucBalData, 0x00, sizeof(aucOutData));

    iRet = hal_getATC(aucBalData);
    if(iRet < 0)
    {
        return DCEP_ERR_ATC_GET;
    }
    i32Len = iRet;
    iRet = calcTdesDec_lib(aucBalData, i32Len, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
    if(iRet != 0)
    {
        sysLOG(API_LOG_LEVEL_2, "getATC desDec iRet=%d\r\n", iRet);
        return DCEP_ERR_ATC_GET;
    }
    i32Len = i32Len- aucOutData[i32Len-8] - 8;
    memcpy(&l32Atc, aucOutData, i32Len);

    return l32Atc;
}

#if 0
const uint8_t hw_bal_voucher0[]={
0x61, 0x82, 0x02, 0x56, 0x72, 0x82, 0x01, 0xA6, 0x41, 0x02, 0x00, 0x01, 0x42, 0x20, 0x3D, 0x68,
0xF4, 0x20, 0x8B, 0x9C, 0x73, 0x7C, 0x0F, 0x5F, 0x72, 0x58, 0x72, 0xBF, 0xB9, 0x67, 0x1A, 0xF4,
0x93, 0x56, 0x9B, 0x24, 0xA1, 0x1F, 0xBE, 0x5E, 0x23, 0x1E, 0x50, 0xD3, 0x3A, 0xCE, 0x43, 0x20,
0xAA, 0x16, 0xEF, 0x7B, 0x8D, 0x65, 0x19, 0x74, 0xA8, 0x18, 0x3D, 0x57, 0x33, 0x79, 0xAD, 0x09,
0x3D, 0xB3, 0x35, 0x85, 0x8A, 0xA7, 0xA0, 0x90, 0x4D, 0x70, 0x9F, 0xFD, 0x6E, 0x47, 0x6C, 0x75,
0x44, 0x06, 0x00, 0x00, 0x00, 0x01, 0x86, 0xA0, 0x45, 0x03, 0x00, 0x07, 0xD0, 0x46, 0x06, 0x5F,
0xC3, 0x38, 0x0B, 0x00, 0x16, 0x47, 0x81, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x40, 0x4E, 0x19, 0x5A, 0x93, 0xE6, 0x7B,
0x58, 0x76, 0xF2, 0x65, 0xBD, 0x8B, 0x15, 0x15, 0x58, 0x2D, 0xD0, 0xE9, 0x80, 0x2C, 0xDF, 0x87,
0x55, 0x59, 0xB3, 0x26, 0xBC, 0xC9, 0xBB, 0x8C, 0xFE, 0xEE, 0x9D, 0x58, 0x30, 0xEF, 0x4B, 0x24,
0x85, 0x59, 0xB9, 0x52, 0x4C, 0x52, 0xB0, 0xD6, 0xF3, 0x52, 0x19, 0x11, 0xEE, 0xE1, 0x60, 0x85,
0x03, 0x79, 0x84, 0xFE, 0xAA, 0x71, 0x4A, 0x39, 0x37, 0xD3, 0x49, 0x40, 0x17, 0xED, 0xE9, 0x23,
0x48, 0xF8, 0x52, 0x23, 0x74, 0x07, 0x1A, 0x30, 0xEE, 0x5D, 0x2C, 0x71, 0x78, 0x19, 0x91, 0x62,
0x40, 0xB1, 0x16, 0xA2, 0xB5, 0x69, 0xF5, 0x1F, 0xC7, 0xD0, 0x19, 0xA9, 0x7D, 0x60, 0x4F, 0xB4,
0xF0, 0xF5, 0x8B, 0xF9, 0x14, 0x85, 0xE6, 0x90, 0x6F, 0x3A, 0xBC, 0x62, 0x05, 0x83, 0x35, 0x0B,
0x39, 0x6B, 0x3C, 0x30, 0x0D, 0x9B, 0xE0, 0x0C, 0xC2, 0x0D, 0x00, 0xDB, 0x4B, 0x40, 0x34, 0x8F,
0xA0, 0x85, 0x94, 0x71, 0xEF, 0x67, 0xE8, 0xA4, 0x88, 0xFC, 0x08, 0x6A, 0x05, 0x3C, 0xCD, 0x0A,
0xBC, 0xF3, 0xED, 0x90, 0xD2, 0xC0, 0x6B, 0xF9, 0xEF, 0xFC, 0x91, 0x8E, 0x5D, 0x58, 0x73, 0x33,
0xAC, 0x71, 0xFA, 0x6A, 0x78, 0xAB, 0xA4, 0xBB, 0x48, 0xDF, 0x83, 0xD1, 0xD5, 0x38, 0x09, 0x59,
0xE9, 0x1C, 0x01, 0x93, 0x7E, 0x38, 0x38, 0x4A, 0x53, 0x93, 0xE0, 0x6B, 0x34, 0x06, 0x60, 0x81,
0xA1, 0x73, 0x81, 0x9E, 0x51, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x52, 0x10, 0xC0, 0xD7,
0x8D, 0x7F, 0x53, 0xE9, 0x65, 0x61, 0xDA, 0x5E, 0xDA, 0x63, 0x83, 0x30, 0xF8, 0xE4, 0x53, 0x40,
0xED, 0x17, 0xF6, 0xEF, 0xCA, 0x12, 0x5E, 0x0E, 0xC3, 0xB4, 0xA1, 0x28, 0x05, 0x0B, 0x97, 0x40,
0xBB, 0xA9, 0xBD, 0x19, 0xB0, 0x70, 0xB9, 0x03, 0xE1, 0xAB, 0x18, 0x99, 0x09, 0x8F, 0xC9, 0x19,
0xFE, 0xEA, 0x52, 0xC6, 0xD6, 0xE4, 0x74, 0x32, 0xF6, 0x9D, 0xB9, 0xB4, 0xFE, 0x2C, 0xC4, 0xA0,
0x65, 0x99, 0x20, 0xB7, 0x35, 0x5F, 0x21, 0x5C, 0x42, 0x3A, 0x23, 0xBA, 0xC1, 0x3D, 0xE8, 0x30,
0x54, 0x40, 0x71, 0xCE, 0x3E, 0x93, 0xAC, 0x05, 0x23, 0x21, 0xA5, 0x84, 0xEB, 0xBE, 0xD4, 0x05,
0x30, 0x29, 0xCD, 0xE9, 0x04, 0xBD, 0x3D, 0x5D, 0x8B, 0x0A, 0xE4, 0xAF, 0xF6, 0x6C, 0x2A, 0x39,
0x73, 0x90, 0x22, 0xCB, 0x16, 0x30, 0xBC, 0xFA, 0xC7, 0xA2, 0x70, 0x38, 0x17, 0xCD, 0xAB, 0x51,
0x6D, 0x80, 0x72, 0xD6, 0x3A, 0x39, 0x74, 0x36, 0xE5, 0x43, 0xEB, 0x9C, 0x30, 0x6B, 0xF7, 0x45,
0x79, 0x67, 0x5A, 0x06, 0x01, 0x23, 0x45, 0x67, 0x89, 0x99, 0x01,
};

const uint8 hw_bal_voucher1[]={
0x61, 0x82, 0x02, 0x56, 0x72, 0x82, 0x01, 0xA6, 0x41, 0x02, 0x00, 0x01, 0x42, 0x20, 0x3D, 0x68,
0xF4, 0x20, 0x8B, 0x9C, 0x73, 0x7C, 0x0F, 0x5F, 0x72, 0x58, 0x72, 0xBF, 0xB9, 0x67, 0x1A, 0xF4,
0x93, 0x56, 0x9B, 0x24, 0xA1, 0x1F, 0xBE, 0x5E, 0x23, 0x1E, 0x50, 0xD3, 0x3A, 0xCE, 0x43, 0x20,
0xAA, 0x16, 0xEF, 0x7B, 0x8D, 0x65, 0x19, 0x74, 0xA8, 0x18, 0x3D, 0x57, 0x33, 0x79, 0xAD, 0x09,
0x3D, 0xB3, 0x35, 0x85, 0x8A, 0xA7, 0xA0, 0x90, 0x4D, 0x70, 0x9F, 0xFD, 0x6E, 0x47, 0x6C, 0x75,
0x44, 0x06, 0x00, 0x00, 0x00, 0x01, 0x86, 0xA0, 0x45, 0x03, 0x00, 0x07, 0xD0, 0x46, 0x06, 0x5F,
0xC3, 0x38, 0x0B, 0x00, 0x16, 0x47, 0x81, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x40, 0x4E, 0x19, 0x5A, 0x93, 0xE6, 0x7B,
0x58, 0x76, 0xF2, 0x65, 0xBD, 0x8B, 0x15, 0x15, 0x58, 0x2D, 0xD0, 0xE9, 0x80, 0x2C, 0xDF, 0x87,
0x55, 0x59, 0xB3, 0x26, 0xBC, 0xC9, 0xBB, 0x8C, 0xFE, 0xEE, 0x9D, 0x58, 0x30, 0xEF, 0x4B, 0x24,
0x85, 0x59, 0xB9, 0x52, 0x4C, 0x52, 0xB0, 0xD6, 0xF3, 0x52, 0x19, 0x11, 0xEE, 0xE1, 0x60, 0x85,
0x03, 0x79, 0x84, 0xFE, 0xAA, 0x71, 0x4A, 0x39, 0x37, 0xD3, 0x49, 0x40, 0x17, 0xED, 0xE9, 0x23,
0x48, 0xF8, 0x52, 0x23, 0x74, 0x07, 0x1A, 0x30, 0xEE, 0x5D, 0x2C, 0x71, 0x78, 0x19, 0x91, 0x62,
0x40, 0xB1, 0x16, 0xA2, 0xB5, 0x69, 0xF5, 0x1F, 0xC7, 0xD0, 0x19, 0xA9, 0x7D, 0x60, 0x4F, 0xB4,
0xF0, 0xF5, 0x8B, 0xF9, 0x14, 0x85, 0xE6, 0x90, 0x6F, 0x3A, 0xBC, 0x62, 0x05, 0x83, 0x35, 0x0B,
0x39, 0x6B, 0x3C, 0x30, 0x0D, 0x9B, 0xE0, 0x0C, 0xC2, 0x0D, 0x00, 0xDB, 0x4B, 0x40, 0x34, 0x8F,
0xA0, 0x85, 0x94, 0x71, 0xEF, 0x67, 0xE8, 0xA4, 0x88, 0xFC, 0x08, 0x6A, 0x05, 0x3C, 0xCD, 0x0A,
0xBC, 0xF3, 0xED, 0x90, 0xD2, 0xC0, 0x6B, 0xF9, 0xEF, 0xFC, 0x91, 0x8E, 0x5D, 0x58, 0x73, 0x33,
0xAC, 0x71, 0xFA, 0x6A, 0x78, 0xAB, 0xA4, 0xBB, 0x48, 0xDF, 0x83, 0xD1, 0xD5, 0x38, 0x09, 0x59,
0xE9, 0x1C, 0x01, 0x93, 0x7E, 0x38, 0x38, 0x4A, 0x53, 0x93, 0xE0, 0x6B, 0x34, 0x06, 0x60, 0x81,
0xA1, 0x73, 0x81, 0x9E, 0x51, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x52, 0x10, 0x3F, 0x21,
0xAC, 0x44, 0xCD, 0x8E, 0x6D, 0xAF, 0x9B, 0xCE, 0x26, 0xBD, 0x2D, 0x8D, 0xA2, 0xDE, 0x53, 0x40,
0xED, 0x17, 0xF6, 0xEF, 0xCA, 0x12, 0x5E, 0x0E, 0xC3, 0xB4, 0xA1, 0x28, 0x05, 0x0B, 0x97, 0x40,
0xBB, 0xA9, 0xBD, 0x19, 0xB0, 0x70, 0xB9, 0x03, 0xE1, 0xAB, 0x18, 0x99, 0x09, 0x8F, 0xC9, 0x19,
0xFE, 0xEA, 0x52, 0xC6, 0xD6, 0xE4, 0x74, 0x32, 0xF6, 0x9D, 0xB9, 0xB4, 0xFE, 0x2C, 0xC4, 0xA0,
0x65, 0x99, 0x20, 0xB7, 0x35, 0x5F, 0x21, 0x5C, 0x42, 0x3A, 0x23, 0xBA, 0xC1, 0x3D, 0xE8, 0x30,
0x54, 0x40, 0xB2, 0xE4, 0xB7, 0xFD, 0x75, 0xEF, 0x6B, 0xDB, 0x01, 0xC7, 0x63, 0x92, 0x42, 0x41,
0x45, 0xC9, 0xA0, 0x1B, 0xA1, 0x12, 0x6C, 0xB5, 0x94, 0x92, 0xA8, 0x02, 0x71, 0x4F, 0x99, 0xEE,
0x82, 0x3D, 0xF6, 0x22, 0x86, 0xEC, 0x9B, 0xEA, 0xA7, 0xEC, 0xDC, 0x8E, 0xA1, 0x5B, 0x47, 0xDB,
0x03, 0xF7, 0x37, 0x7D, 0x5C, 0xFA, 0x33, 0x99, 0x64, 0x14, 0x39, 0x93, 0xFB, 0x1B, 0x30, 0xF4,
0xEB, 0xBA, 0x5A, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

const uint8 hw_bal_voucher99998[]=
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 

};

const uint8 hw_bal_voucher99999[]=
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 

};

const uint8 hw_bal_voucher99997[]=
{
	0x61, 0x82, 0x01, 0xB2, 0x72, 0x82, 0x01, 0xA6, 0x41, 0x02, 0x00, 0x01, 0x42, 0x20, 0x40, 0xF0,
	0xB7, 0xA2, 0xB2, 0x3B, 0x92, 0xAA, 0xB4, 0x0A, 0x62, 0x66, 0x00, 0x92, 0xFB, 0x4B, 0x2C, 0xFE,
	0xE2, 0xDF, 0x99, 0x1B, 0xBE, 0x0D, 0xAF, 0x90, 0x3F, 0x26, 0x9D, 0x12, 0x28, 0xD0, 0x43, 0x20,
	0xC3, 0xA0, 0x0E, 0xDF, 0x51, 0xDB, 0x74, 0x2F, 0x1E, 0x99, 0xCF, 0xFC, 0x1B, 0xDF, 0xF1, 0xBB,
	0x5F, 0xAC, 0x6F, 0x83, 0xE0, 0xF8, 0x85, 0x77, 0x94, 0x91, 0x51, 0x49, 0x7D, 0x79, 0x85, 0xD6,
	0x44, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x45, 0x03, 0x00, 0x07, 0xD0, 0x46, 0x06, 0x60,
	0xF0, 0xE2, 0x0E, 0x00, 0xCD, 0x47, 0x81, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x40, 0xCA, 0xC3, 0x02, 0xA0, 0x29, 0xD4,
	0xB9, 0x9A, 0xB0, 0x13, 0xF9, 0x2C, 0x97, 0x57, 0x9D, 0x9E, 0x19, 0x00, 0xC3, 0xD4, 0x4D, 0xDB,
	0xE1, 0x3F, 0x1F, 0xA1, 0x71, 0xB3, 0x65, 0x42, 0x28, 0x67, 0x4D, 0x8B, 0x14, 0x8C, 0xFD, 0x75,
	0xBE, 0x99, 0x17, 0x2B, 0x60, 0xBC, 0x9B, 0xDC, 0x88, 0x83, 0x64, 0xA4, 0x92, 0x15, 0x24, 0x72,
	0x82, 0x8A, 0xE8, 0xF7, 0x78, 0xA2, 0xC7, 0xA2, 0x76, 0x9A, 0x49, 0x40, 0x73, 0x4C, 0xCC, 0xA1,
	0x65, 0xD6, 0xB2, 0x55, 0xAE, 0x5C, 0xA8, 0x58, 0x38, 0x40, 0x7C, 0x07, 0x71, 0x51, 0x45, 0x20,
	0x0F, 0x45, 0x8C, 0x9F, 0x98, 0xF2, 0xD0, 0xC0, 0xC8, 0x5F, 0xD1, 0x90, 0x03, 0xB6, 0xF2, 0xA2,
	0xD3, 0x6C, 0xC2, 0x0C, 0x36, 0x96, 0x74, 0x56, 0x15, 0x78, 0xB1, 0xCA, 0x99, 0xD0, 0x6F, 0x37,
	0x4F, 0xED, 0xC5, 0x5C, 0xF0, 0x13, 0x69, 0xDF, 0x1D, 0x7E, 0x9F, 0x5E, 0x4B, 0x40, 0x4F, 0x32,
	0xDF, 0xD0, 0x45, 0xF6, 0x52, 0x22, 0xF9, 0x04, 0xCF, 0x25, 0x8F, 0xA1, 0xF6, 0x15, 0x49, 0x05,
	0xF7, 0x21, 0xAE, 0xD2, 0xFF, 0x72, 0x39, 0x5E, 0x0B, 0x77, 0xC2, 0xD1, 0x66, 0x5D, 0x06, 0x0F,
	0x45, 0xAD, 0xBF, 0x04, 0x98, 0xD6, 0xCB, 0x74, 0x4F, 0xF3, 0x01, 0x9E, 0xE8, 0xED, 0x56, 0x30,
	0x42, 0x74, 0x31, 0xC8, 0x44, 0x24, 0xB4, 0x2B, 0x9F, 0x26, 0x46, 0x4A, 0x60, 0x1C, 0x5A, 0x06,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02,

};

int DCEPWallFunctionTest(unsigned char type)
{
    uint8_t key = 0;
	int i = 0;
    int iRet = 0;
    char seid[33] = "12345678901234567890123456789012\0";
    uint8_t CID[17] = "CID1234567890123\0";
    uint8_t wltID[17] = "wltID12345678901\0";
    uint8_t parentWltId[17] = "parentWltId12345\0";
    uint8_t hwName[37] = "hwName123456789012345678901234567890\0";
    uint8_t appType = 0;
    uint8_t wltTxnCd = 0;
    uint8_t setTm[8]= {20,21,12,16,17,18,50};
    uint8_t getTm[15];
    uint8_t limAmt[7];
    uint8_t walStatus[9] = "walStatu\0";
    uint8_t WalType[9] = "WalType0\0";
    uint8_t walLevel[9] = "walLevel\0";
    uint8_t buf[11];
    uint8_t info[21] = "12345678901234567890\0";
    uint64_t limAmtVal = 0x2bdb34f1e02cd7;
    uint8_t balVoucher[4096];
    uint8_t getBillData[4096];
    uint8_t balance[6];
    unsigned char ucCmd = 0;
    int pos = 0;
    int ucPwdLen = 0;
    char indexCurrencyStr[] = TYPE_TRANCHAIN_INDEX;
    char indexBal[10];
    int balVoucherLenth = 0;
    int BillLen = 0;
    int IndexTable[1000];
    memset(indexBal, 0x00, sizeof(indexBal));
    memcpy(indexBal, TYPE_BAL_COUNT_INDEX, 6);
    
    scrCls_lib();
	kbFlush_lib();

    //while(1)
    {
        switch(type)
        {
            case 0:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test get Random");
                    memset(buf, 0x55, sizeof(11));
                    iRet = getRandom(buf, 10);
					scrClrLine_lib(4,6);
					scrPrint_lib(0,4,0,"Randomlenth=%d", iRet);
                    if(iRet != 10)
                    {
                        return -1;
                    }
                    scrClrLine_lib(4,7);
					/*for(i = 0; i< 10; i++)
					{
						buf[i] += '0';
					}*/
					scrPrint_lib(0,5,0, "randomData=%x%x%x%x%x%x%x%x%x%x\r\n\r\n",  buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9]);
					//scrPrint_lib(0,5,0,"Random=%s",buf);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
				    break; 
            case 1:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test seid");
                    //memset(seid, 0x55, sizeof(seid));
                    iRet = setSEId(seid,sizeof(seid)-1);
					scrClrLine_lib(4,6);
					scrPrint_lib(0,4,0,"setSied=%d", iRet);
					//sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: setSeid=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__,iRet);
                    if(iRet < 0)
                    {
                        return -2;
                    }
                    memset(seid, 0x00, sizeof(seid));
                    iRet = getSEId(seid);
					scrClrLine_lib(4,6);
					scrPrint_lib(0,4,0,"getSied=%d", iRet);
                    if(iRet != (sizeof(seid)-1))
                    {
                        return -3;
                    }
                    //scrClrLine_lib(4,7);
					scrPrint_lib(0,5,0,"seid=%s",seid);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 2:	
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test parentWltId");
                    //memset(parentWltId, 0x55, sizeof(parentWltId));
                    iRet = setParentWalletId(parentWltId, sizeof(parentWltId)-1);
					scrClrLine_lib(4,5);
					scrPrint_lib(0,4,0,"setParentWltId=%d", iRet);	
                    if(iRet < 0)
                    {	
                        return -4;
                    }
                    memset(parentWltId, 0x00, sizeof(parentWltId));
                    iRet = getParentWalletId(parentWltId);
                    if(iRet != (sizeof(parentWltId)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getParentWltId=%d", iRet);
                        return -5;
                    }
                    scrClrLine_lib(4,7);
					scrPrint_lib(0,4,0,"parentWltId=%s",parentWltId);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 3:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test CID");
                    //memset(CID, 0x55, sizeof(CID));
                    iRet = setContextId(CID, sizeof(CID)-1);	
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setCID=%d", iRet);
                        return -6;
                    }
                    memset(CID, 0x00, sizeof(CID));
                    iRet = getContextId(CID);
                    if(iRet != (sizeof(CID)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getCID=%d", iRet);
                        return -7;
                    }
                    scrClrLine_lib(4,7);
					scrPrint_lib(0,4,0,"CID=%s",CID);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 4:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test wltID");
                    //memset(wltID, 0x55, sizeof(wltID));
                    iRet = setWalletId(wltID, sizeof(wltID)-1);	
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setwltID=%d", iRet);
                        return -8;
                    }
                    memset(wltID, 0x00, sizeof(wltID));
                    iRet = getWalletId(wltID);
                    if(iRet != (sizeof(wltID)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getwltID=%d", iRet);
                        return -9;
                    }
                    scrClrLine_lib(4,7);
					scrPrint_lib(0,4,0,"wltID=%s",wltID);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 5:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test hwName");
                    //memset(hwName, 0x55, sizeof(hwName));
                    iRet = setWalletName(hwName, sizeof(hwName)-1);	
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"sethwName=%d", iRet);
                        return -10;
                    }
                    memset(hwName, 0x00, sizeof(hwName));
                    iRet = getWalletName(hwName);
                    if(iRet != (sizeof(hwName)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"gethwName=%d", iRet);
                        return -11;
                    }
                    scrClrLine_lib(4,5);
					scrPrint_lib(0,4,0,"gethwName=%d", iRet);
                    scrClrLine_lib(6,7);
					scrPrint_lib(0,6,0,"hwName=%s",hwName);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 6:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test appType");
                    iRet = setAppType(0x01);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setAppType=%d", iRet);
                            return -12;
                    }
                    appType = 0;
                    iRet = getAppType(&appType);
                    if(iRet != 1)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getAppType=%d", iRet);
                            return -13;
                    }
                    scrClrLine_lib(4,7);
					scrPrint_lib(0,4,0,"appType=%x",appType);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 7:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test wltTxnCd");
                    iRet = setWltTxnCd(0x55);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setWltTxnCd=%d", iRet);
                            return -14;
                    }
                    wltTxnCd = 0;
                    iRet = getWltTxnCd(&wltTxnCd);
                    if(iRet <= 0 )
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getWltTxnCd=%d", iRet);
                            return -15;
                    }
                    scrClrLine_lib(4,7);
					scrPrint_lib(0,4,0,"wltTxnCd=%x",wltTxnCd);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 8:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test Time");
                    iRet = setCreateTime(setTm, sizeof(setTm)-1);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setTime=%d", iRet);
                            return -16;
                    }
                    memset(getTm, 0x00, sizeof(getTm));
                    iRet = getCreateTime(getTm);
                    if(iRet != (sizeof(setTm)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getTime=%d", iRet);
                            return -17;
                    }
                    scrClrLine_lib(4,7);
                    scrPrint_lib(0,4,0,"Time=%02d%02d%02d%02d%02d%02d%02d",getTm[0],getTm[1],getTm[2],getTm[3],getTm[4],getTm[5],getTm[6]);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 9:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test Wallet Limit");
                    limAmt[0] = limAmtVal & 0xFF;
                    limAmt[1] = (limAmtVal >> 8) & 0xFF;
                    limAmt[2] = (limAmtVal >> 16) & 0xFF;
                    limAmt[3] = (limAmtVal >> 24) & 0xFF;
                    limAmt[4] = (limAmtVal >> 32) & 0xFF;
                    limAmt[5] = (limAmtVal >> 40) & 0xFF;

                    iRet = setWalletLimit(limAmt, sizeof(limAmt)-1);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setWalletLimit=%d", iRet);
                            return -18;
                    }
                    memset(limAmt, 0x00, sizeof(limAmt));
                    iRet = getWalletLimit(limAmt);
                    if(iRet != (sizeof(limAmt)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getWalletLimit=%d", iRet);
                            return -19;
                    }
                    limAmtVal = 0;
                    limAmtVal |= (limAmt[5] << 40);
                    limAmtVal |= (limAmt[4] << 32);
                    limAmtVal |= (limAmt[3] << 24);
                    limAmtVal |= (limAmt[2] << 16);
                    limAmtVal |= (limAmt[1] << 8);
                    limAmtVal |= limAmt[0];

                    scrClrLine_lib(4,7);
                    scrPrint_lib(0,4,0,"limAmt=%x%x%x%x%x%x\r\n",limAmt[5],limAmt[4],limAmt[3],limAmt[2],limAmt[1],limAmt[0]);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 10:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test Wallet Status");
                    iRet = setWalletStatus(walStatus, sizeof(walStatus)-1);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setWalletStatus=%d", iRet);
                            return -20;
                    }
                    memset(walStatus, 0x00, sizeof(walStatus));
                    iRet = getWalletStatus(walStatus);
                    if(iRet != sizeof(walStatus)-1)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getWalletStatus=%d", iRet);
                            return -21;
                    }
                    scrClrLine_lib(4,7);
                    scrPrint_lib(0,4,0,"walStatus=%s",walStatus);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return ;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 11:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test Wallet Type");
                    iRet = setWalletType(WalType, sizeof(WalType)-1);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setWalletType=%d", iRet);
                            return -22;
                    }
                    memset(WalType, 0x00,sizeof(WalType));
                    iRet = getWalletType(WalType);
                    if(iRet != sizeof(WalType)-1)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getWalletType=%d", iRet);
                            return -23;
                    }
                    scrClrLine_lib(4,5);
                    scrPrint_lib(0,4,0,"WalType=%s",WalType);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 12:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test Wallet Level");
                    iRet = setWalletLevel(walLevel, sizeof(walLevel)-1);
                    if(iRet < 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setWalletLevel=%d", iRet);
                            return -24;
                    }
                    memset(walLevel, 0x00, sizeof(walLevel));
                    iRet = getWalletLevel(walLevel);
                    if(iRet != sizeof(walLevel)-1)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getWalletLevel=%d", iRet);
                            return -25;
                    }
                    scrClrLine_lib(4,5);
                    scrPrint_lib(0,4,0,"walLevel=%s",walLevel);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 13:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test Ctrl Info");
                    //memset(info, 0x55, sizeof(info));
                    iRet = setCtrlInfo(info, sizeof(info)-1);
                    if(iRet < 0 )
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setCtrlInfo=%d", iRet);
                        return -26;
                    }
                    memset(info, 0x00, sizeof(info));
                    iRet = getCtrlInfo(info);
                    if(iRet != (sizeof(info)-1))
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getCtrlInfo=%d", iRet);
                        return -27;
                    }
					scrClrLine_lib(4,5);
                    scrPrint_lib(0,4,0,"info=%s",info);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 14:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test SaveBalVoucher");
                    //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: saveBalVoucher indexBal=%s\r\n", filename(__FILE__), __FUNCTION__, __LINE__, indexBal);
                    for(i=0; i<300; i++)
                    {
                        iRet = saveBalVoucher(0, hw_bal_voucher0, sizeof(hw_bal_voucher0));
                        if(iRet < 0)
                        {
                            scrClrLine_lib(4,5);
                            scrPrint_lib(0,4,0,"SaveBalVoucher=%d", iRet);
                            return -27;
                        }
                    }
                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "SaveBalVoucher OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 15:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test GetBalVoucher");
                    memset(balVoucher, 0x00, sizeof(balVoucher));

                    iRet = getBalVoucherCount();
                    if(iRet <= 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getBalVoucherCount=%d", iRet);
                        return -28;
                    }
                    scrClrLine_lib(4,5);
					scrPrint_lib(0,4,0,"Pls Input Get Pos");
                    iRet = portOpen_lib(P_USB, NULL);
                    if(iRet != 0)
                    {
                        return -29;
                        //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portOpen_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                    }
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            ucCmd = kbGetKey_lib();
                            if((KEY0 <= ucCmd) && (ucCmd <= KEY9))
                            {
                                if(ucPwdLen<3)
                                {
                                    pos = pos* 10 + (ucCmd - '0');
                                    scrClrLine_lib(5,6);
                                    if(ucPwdLen == 0)
					                scrPrint_lib(0,5,0,"Num=%d",pos);
                                    if(ucPwdLen == 1)
					                scrPrint_lib(0,5,0,"Num=%02d",pos);
                                    if(ucPwdLen == 2)
					                scrPrint_lib(0,5,0,"Num=%03d",pos);
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -30;
                                }
                                ucPwdLen++;
                            }
                            else if(ucCmd == KEYCANCEL)
                            {
                                return 0;
                            }  
                            else if(ucCmd == KEYENTER)
                            {
                                if(ucPwdLen==3)
                                {
                                    break;
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -31;
                                } 
                            }                     
                        }
                        sysDelayMs(50);
                    }

                    balVoucherLenth = getBalVoucher(0, pos, balVoucher);
                    if(balVoucherLenth <= 0)
                    {
						scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getBalVoucher=%d", balVoucherLenth);
                        return -32;
                    }

                    do
                    {
                        if(balVoucherLenth>=1024)
                        {
                            iRet = portSends_lib(P_USB, balVoucher, 1024);
                            if(iRet < 0)
                            {
                                return -33;
                                //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portSends_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                            }
                            balVoucherLenth -= 1024;
                        }
                        else
                        {  
                            iRet = portSends_lib(P_USB, balVoucher, balVoucherLenth);
                            if(iRet < 0)
                            {
                                return -34;
                                //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portSends_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                            }
                            balVoucherLenth = 0;
                        }
                    }while(balVoucherLenth>0);

                    iRet = portClose_lib(P_USB);
                    if(iRet < 0)
                    {
                        return -35;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "getBalVoucher OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 16:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test GetBalVoucherCount");

                    iRet = getBalVoucherCount();
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getBalVoucherCount=%d", iRet);
                        return -36;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "BalVoucherCount=%d", iRet);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 17:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test GetBalance");
                   // balance = malloc(6);
                     memset(balance, 0x00, sizeof(balance));
                     iRet = getBalance(balance);
                    if(iRet != 6){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"GetBalance Error=%d",iRet);
                        return -37;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "GetBalance=%x%x%x%x%x%x", balance[0],balance[1],balance[2],balance[3],balance[4],balance[5]);
                    //free(balance);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 18:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test deleteBalVoucher");
                    scrClrLine_lib(4,5);
					scrPrint_lib(0,4,0,"Pls Input Del Pos");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            ucCmd = kbGetKey_lib();
                            if((KEY0 <= ucCmd) && (ucCmd <= KEY9))
                            {
                                if(ucPwdLen<3)
                                {
                                    pos = pos* 10 + (ucCmd - '0');
                                    scrClrLine_lib(5,6);
					                 if(ucPwdLen == 0)
					                scrPrint_lib(0,5,0,"Num=%d",pos);
                                    if(ucPwdLen == 1)
					                scrPrint_lib(0,5,0,"Num=%02d",pos);
                                    if(ucPwdLen == 2)
					                scrPrint_lib(0,5,0,"Num=%03d",pos);
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -38;
                                }
                                ucPwdLen++;
                            }
                            else if(ucCmd == KEYCANCEL)
                            {
                                return 0;
                            }  
                            else if(ucCmd == KEYENTER)
                            {
                                if(ucPwdLen==3)
                                {
                                    break;
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -39;
                                } 
                            }                     
                        }
                        sysDelayMs(50);
                    }
                    //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: index=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, pos);
                    iRet = deleteBalVoucher(pos);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"deleteBalVoucher=%d", iRet);
                        return -40;
                    }

                    scrClrLine_lib(5,5);
                    scrPrint_lib(0, 5, 2, "deleteBalVoucher OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
             case 19:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test deleteAllBalVoucher");

                    iRet = deleteAllBalVoucher(0);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"deleteAllBalVoucher=%d", iRet);
                        return -41;
                    }

                    scrClrLine_lib(5,5);
                    scrPrint_lib(0, 5, 2, "deleteAllBalVoucher OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 20:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test saveCurrencyString");

                    iRet = saveBalVoucher(1, hw_bal_voucher99997, sizeof(hw_bal_voucher99997));
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"saveCurrencyString=%d", iRet);
                        return -42;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "saveCurrencyString OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 21:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test getCurrencyString");
                    memset(balVoucher, 0x00, sizeof(balVoucher));
                    iRet = portOpen_lib(P_USB, NULL);
                    if(iRet != 0)
                    {
                        //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portOpen_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                        return -43;
                    }
                   
                    balVoucherLenth = getBalVoucher(1, 0, balVoucher);
                    if(balVoucherLenth < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getCurrencyString=%d", balVoucherLenth);
                        return -44;
                    }
                    
                    sysDelayMs(500);
                    do
                    {
                        if(balVoucherLenth>=1024)
                        {
                            iRet = portSends_lib(P_USB, balVoucher, 1024);
                            if(iRet < 0)
                            {
                                //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portSends_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                                return -45;
                            }
                            balVoucherLenth -= 1024;
                        }
                        else
                        {  
                            iRet = portSends_lib(P_USB, balVoucher, balVoucherLenth);
                            if(iRet < 0)
                            {  
                                //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portSends_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                                return -46;
                            }
                            balVoucherLenth = 0;
                        }
                    }while(balVoucherLenth>0);
                     
                    iRet = portClose_lib(P_USB);
                    if(iRet < 0)
                    {
                        return -47;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "getCurrencyString OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 22:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test delCurrencyString");

                    iRet =deleteAllBalVoucher(1);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"delCurrencyString=%d", iRet);
                        return -48;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "delCurrencyString OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 23:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test saveBill");
                    for (i = 0; i < 300; i++)
                    {
                        iRet =saveBill(hw_bal_voucher99998, sizeof(hw_bal_voucher99998));
                        if(iRet < 0){
                            scrClrLine_lib(4,5);
                            scrPrint_lib(0,4,0,"saveBill=%d", iRet);
                            return -49;
                        }
                    }
                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "saveBill OK");
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 24:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test getBill");

                    scrClrLine_lib(4,5);
					scrPrint_lib(0,4,0,"Pls Input Get Num");
                    iRet = portOpen_lib(P_USB, NULL);
                    if(iRet != 0)
                    {
                        return -50;
                    }
                    sysDelayMs(500);
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            ucCmd = kbGetKey_lib();
                            if((KEY0 <= ucCmd) && (ucCmd <= KEY9))
                            {
                                if(ucPwdLen<3)
                                {
                                    pos = pos* 10 + (ucCmd - '0');
                                    scrClrLine_lib(5,6);
					                 if(ucPwdLen == 0)
					                scrPrint_lib(0,5,0,"Num=%d",pos);
                                    if(ucPwdLen == 1)
					                scrPrint_lib(0,5,0,"Num=%02d",pos);
                                    if(ucPwdLen == 2)
					                scrPrint_lib(0,5,0,"Num=%03d",pos);
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -51;
                                }
                                ucPwdLen++;
                            }
                            else if(ucCmd == KEYCANCEL)
                            {
                                return 0;
                            }  
                            else if(ucCmd == KEYENTER)
                            {
                                if(ucPwdLen==3)
                                {
                                    break;
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -52;
                                } 
                            }                     
                        }
                        sysDelayMs(50);
                    }

                    BillLen =getBill(pos, getBillData);
                    if(BillLen < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getBill=%d", BillLen);
                        return -53;
                    }

                    do
                    {
                        if(BillLen>=1024)
                        {
                            iRet = portSends_lib(P_USB, getBillData, 1024);
                            if(iRet < 0)
                            {
                                return -54;
                                //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portSends_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                            }
                            BillLen -= 1024;
                        }
                        else
                        {  
                            iRet = portSends_lib(P_USB, getBillData, BillLen);
                            if(iRet < 0)
                            {
                                return -55;
                                //sysLOG_lib(BASE_LOG_LEVEL_1, "[%s] -%s- Line=%d: portSends_lib1\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
                            }
                            BillLen = 0;
                        }
                    }while(BillLen>0);

                    iRet = portClose_lib(P_USB);
                    if(iRet < 0)
                    {
                        return -56;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "getBill OK");
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 25:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test getBillCount");

                    iRet = getBillCount();
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getBillCount=%d", iRet);
                        return -57;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "getBillCount=%d", iRet);
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
             case 26:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test deleteBill");
                    scrClrLine_lib(4,3);
                    scrPrint_lib(0,4,0,"Pls Input Del Pos");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            ucCmd = kbGetKey_lib();
                            if((KEY0 <= ucCmd) && (ucCmd <= KEY9))
                            {
                                if(ucPwdLen<3)
                                {
                                    pos = pos* 10 + (ucCmd - '0');
                                    scrClrLine_lib(5,6);
					                 if(ucPwdLen == 0)
					                scrPrint_lib(0,5,0,"Num=%d",pos);
                                    if(ucPwdLen == 1)
					                scrPrint_lib(0,5,0,"Num=%02d",pos);
                                    if(ucPwdLen == 2)
					                scrPrint_lib(0,5,0,"Num=%03d",pos);
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -58;
                                }
                                ucPwdLen++;
                            }
                            else if(ucCmd == KEYCANCEL)
                            {
                                return 0;
                            }  
                            else if(ucCmd == KEYENTER)
                            {
                                if(ucPwdLen==3)
                                {
                                    break;
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -59;
                                } 
                            }                     
                        }
                        sysDelayMs(50);
                    }

                    iRet = deleteBill(pos);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"deleteBill=%d", iRet);
                        return -60;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "deleteBill OK");
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
             case 27:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test deleteAllBill");

                    iRet = deleteAllBill();
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"deleteAllBill=%d", iRet);
                        return -61;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "deleteAllBill OK");
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 28:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test setATC");

                    iRet = setATC(12864);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setATC=%d", iRet);
                        return -62;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "setATC=12864");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 29:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test getATC");
                   
                    iRet = getATC();
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getATC=%d", iRet);
                        return -63;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "getATC=%d", iRet);
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            case 30:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test getBillIndexTable");
                     memset(IndexTable, 0x00, sizeof IndexTable);
                    iRet = getBillIndexTable(IndexTable);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getIndexTable=%d", iRet);
                        return -63;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "Counter=%d,IndexTable=%d %d %d %d %d", iRet,IndexTable[0],IndexTable[1],IndexTable[iRet-3],IndexTable[iRet-2],IndexTable[iRet-1]);
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib(); 
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
             case 31:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test getBalIndexTable");
                     memset(IndexTable, 0x00, sizeof IndexTable);
                    iRet = getBalVoucherIndexTable(IndexTable);
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"getIndexTable=%d", iRet);
                        return -63;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "Counter=%d,IndexTable=%d %d %d %d %d", iRet,IndexTable[0],IndexTable[1],IndexTable[iRet-3],IndexTable[iRet-2],IndexTable[iRet-1]);
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
		     case 32:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test setBalVoucher");
                    scrClrLine_lib(4,5);
					scrPrint_lib(0,4,0,"Pls Input Del Pos");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            ucCmd = kbGetKey_lib();
                            if((KEY0 <= ucCmd) && (ucCmd <= KEY9))
                            {
                                if(ucPwdLen<3)
                                {
                                    pos = pos* 10 + (ucCmd - '0');
                                    scrClrLine_lib(5,6);
					                 if(ucPwdLen == 0)
					                scrPrint_lib(0,5,0,"Num=%d",pos);
                                    if(ucPwdLen == 1)
					                scrPrint_lib(0,5,0,"Num=%02d",pos);
                                    if(ucPwdLen == 2)
					                scrPrint_lib(0,5,0,"Num=%03d",pos);
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -38;
                                }
                                ucPwdLen++;
                            }
                            else if(ucCmd == KEYCANCEL)
                            {
                                return 0;
                            }  
                            else if(ucCmd == KEYENTER)
                            {
                                if(ucPwdLen==3)
                                {
                                    break;
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -39;
                                } 
                            }                     
                        }
                        sysDelayMs(50);
                    }

                    iRet = setBalVoucher(pos, hw_bal_voucher1, sizeof(hw_bal_voucher1));
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setBalVoucher=%d", iRet);
                        return -40;
                    }

                    scrClrLine_lib(5,5);
                    scrPrint_lib(0, 5, 2, "setBalVoucher OK");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
			case 33:
                    scrClrLine_lib(2,3);
                    scrPrint_lib(0, 3, 2, "test setBill");
                    scrClrLine_lib(4,3);
                    scrPrint_lib(0,4,0,"Pls Input Del Pos");
                    while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            ucCmd = kbGetKey_lib();
                            if((KEY0 <= ucCmd) && (ucCmd <= KEY9))
                            {
                                if(ucPwdLen<3)
                                {
                                    pos = pos* 10 + (ucCmd - '0');
                                    scrClrLine_lib(5,6);
					                 if(ucPwdLen == 0)
					                scrPrint_lib(0,5,0,"Num=%d",pos);
                                    if(ucPwdLen == 1)
					                scrPrint_lib(0,5,0,"Num=%02d",pos);
                                    if(ucPwdLen == 2)
					                scrPrint_lib(0,5,0,"Num=%03d",pos);
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -58;
                                }
                                ucPwdLen++;
                            }
                            else if(ucCmd == KEYCANCEL)
                            {
                                return 0;
                            }  
                            else if(ucCmd == KEYENTER)
                            {
                                if(ucPwdLen==3)
                                {
                                    break;
                                }
                                else
                                {
                                    scrClrLine_lib(4,5);
					                scrPrint_lib(0,4,0,"Input Error");
                                    return -59;
                                } 
                            }                     
                        }
                        sysDelayMs(50);
                    }

                    iRet = setBill(pos, hw_bal_voucher99999, sizeof(hw_bal_voucher99999));
                    if(iRet < 0){
                        scrClrLine_lib(4,5);
						scrPrint_lib(0,4,0,"setBill=%d", iRet);
                        return -60;
                    }

                    scrClrLine_lib(4,5);
                    scrPrint_lib(0, 4, 2, "setBill OK");
                     while(1)
                    {
                        if(kbHit_lib() == 0)
                        {
                            key = kbGetKey_lib();
                            if(key == KEYCANCEL)
                            {
                                return 0;
                            }
                        }
                        sysDelayMs(50);
                    }
                    break;
            default:
                    break;          
        }
    }
}

void DCEPWalltest(void)
{
    int iRet = 0;
    unsigned char ucCmd = 0;
    unsigned char ucPwd[16] = {0};
    int ucPwdLen = 0;
	unsigned char ucXINHAO[16] = {0};
	unsigned char key = 0;
    int type = 0;
    scrCls_lib();
	kbFlush_lib();
	memset(ucPwd, 0x00, 16);
	memset(ucXINHAO, 0x00, 16);
    scrPrint_Ex(0x00, 3, 0x02, "请输入测试编号", "Pls Input test Num");
    while(1)
    {
		if(kbHit_lib() == 0)
        {
            ucCmd = kbGetKey_lib();
            switch(ucCmd)
            {
				case KEYENTER:
                    if(ucPwdLen == 2)
                    {
						ucPwdLen = 0;
                        type = ucPwd[0]*10 + ucPwd[1];
                        iRet = DCEPWallFunctionTest(type);
						memset(ucPwd, 0x00, 16);
						memset(ucXINHAO, 0x00, 16);
						if(iRet < 0)
						{
							scrClrLine_lib(6,7);
							scrPrint_lib(0,6,2,"ERROR NUM=%d",iRet);
							while(1)
							{
								if(kbHit_lib() == 0)
								{
									key = kbGetKey_lib();
									if(key == KEYCANCEL)
									{	
										break;
									}
								}
								sysDelayMs(20);
							}
						}
                        scrCls_lib();
                        scrPrint_Ex(0x00, 3, 0x02, "请输入测试编号", "Pls Input test Num");
                    }
                    else{
                        scrClrLine_lib(5,7);
                        scrPrint_lib(0, 5, 2, "Input test Num ERROR");
                    }
                    
				break;
				case KEYCANCEL:
					return ;
				break;
				case KEY_FN:
					//return UNLOCKPAD_FN;
				break;
				case KEYCLEAR:
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
					if(ucPwdLen < 2)
					{
						ucXINHAO[ucPwdLen] = ucCmd;
						ucPwd[ucPwdLen++] = ucCmd - '0';
						scrClrLine_lib(5,7);
						scrPrint_lib(1,5,2,"TestNum=%s",ucXINHAO);
					}
				break;
				default:
				break;

			}
        }
		sysDelayMs(20);
    }
}
#endif
