
#include "comm.h"
#include "LC610N_api_ped.h"



/*
*@Brief:		写入一个密钥,包括 TLK,TMK 和 TWK 的写入、发散,并可以选择使用 KCV 验证密钥 正确性。
*@Param IN:		*KeyInfoIn [输入] 密钥信息 
                *KcvInfoIn [输入] KCV信息
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedWriteKey_lib(ST_KEY_INFO * KeyInfoIn, ST_KCV_INFO * KcvInfoIn)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + sizeof(ST_KEY_INFO) + sizeof(ST_KCV_INFO);
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x01, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, KeyInfoIn, sizeof(ST_KEY_INFO));
	memcpy(ucCmd + 6 + sizeof(ST_KEY_INFO), KcvInfoIn, sizeof(ST_KCV_INFO));
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
	iRet = transceiveFrame(frm, &retfrm, 3000);

	//memset(caShow, 0, sizeof(caShow));
	//HexToStr(retfrm.data, retfrm.length, caShow);
	//sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
	
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}



/*
*@Brief:		写入一个密钥,包括 TLK,TMK 和 TWK 的写入、发散,并可以选择使用 KCV 验证密钥 正确性。
*@Param IN:		*KeyInfoIn [输入] 密钥信息 
                *KcvInfoIn [输入] KCV信息
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedWriteKeyEx_lib(ST_KEY_INFO_EX * KeyInfoIn, ST_KCV_INFO * KcvInfoIn)
{
	sysLOG(API_LOG_LEVEL_2, " into\r\n");

	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + sizeof(ST_KEY_INFO_EX) + sizeof(ST_KCV_INFO);
	sysLOG(API_LOG_LEVEL_2, "  iCmdLen = %d\r\n", iCmdLen);
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x16, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*)fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, KeyInfoIn, sizeof(ST_KEY_INFO_EX));
	memcpy(ucCmd + 6 + sizeof(ST_KEY_INFO_EX), KcvInfoIn, sizeof(ST_KCV_INFO));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
    sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	Frame frm,retfrm;
    iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	//sysDelayMs(100);
	//sysLOG(API_LOG_LEVEL_2, "  frameFactory iRet= %d\r\n", iRet);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 5000);
	//sysLOG(API_LOG_LEVEL_2, "  transceiveFrame iRet= %d, retfrm.length=%d\r\n", iRet, retfrm.length);
	//memset(caShow, 0, sizeof(caShow));
	//HexToStr((char*)retfrm.data, retfrm.length, caShow);
	//sysLOG(API_LOG_LEVEL_2, "  retfrm.data = %s\r\n", caShow);
	
	fibo_free(frm.data);
	if((iRet <0) || (retfrm.length < 6)) {
		goto RET_END;
	}

	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	//sysLOG(API_LOG_LEVEL_2, " retfrm,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	//printBytes(retfrm.data,retfrm.length);
	//sysDelayMs(100);
	//sysLOG(API_LOG_LEVEL_2, "  fibo_free retfrm.data begin\r\n");
	fibo_free(retfrm.data);
	//sysDelayMs(100);
	//sysLOG(API_LOG_LEVEL_2, "  fibo_free retfrm.data end\r\n");
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;

}


/*
*@Brief:		
*@Param IN:		GroupIdx [输入] [1~10]  DUKPT 密钥组索引号
                SrcKeyIdx [ 输 入] [0~1]   用于分散的密钥的密钥索引 
				KeyLen  [输入] 16     TIK 的长度,现 DUKPT 算法支持 8/16 字节长度的密钥 
				*KeyValueIn  [输入] 指向 TIK 的密文 
				*KsnIn [输入] 指向初始化 KSN 
				*KcvInfoIn [ 输 入] 
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedWriteTiK_lib(uchar GroupIdx, uchar SrcKeyIdx, uchar KeyLen, uchar * KeyValueIn, uchar * KsnIn, ST_KCV_INFO * KcvInfoIn)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1 + 1 + 1 + KeyLen + 10 + sizeof(ST_KCV_INFO);
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x02, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memset(ucCmd, 0, iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = GroupIdx;
	ucCmd[7] = SrcKeyIdx;
	ucCmd[8] = KeyLen;
	memcpy(ucCmd + 9, KeyValueIn, KeyLen);
	memcpy(ucCmd + 9 + KeyLen, KsnIn, 10);
	memcpy(ucCmd + 9 + KeyLen + 10, KcvInfoIn, sizeof(ST_KCV_INFO));
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;

}


/*
*@Brief:		写工作主密钥 
*@Param IN:		key_no [输入] 密钥序号，取值范围 1~100，其他值非法。 
				key_len[输入] 密钥长度,只能取 8、16、24 这 3 个值，其他值非法。 
				*key_data[输入] 密钥数据 
				*KcvInfoIn [输入] 详情参考 pedWriteKey 
				mode[输入] 写入密钥的模式，直接写入或者加密，解密，使用 DES 或 SM4 算法 
				Bit[0]： 0: 直接写入，其他 bit 位无效 1：加密或解密写入   Bit[6]: 0：使用 DES/3DES 算法 1：使用 SM4 算法 Bit[7]: 0：解密 1：加密 
				mkey_no[输入] 主密钥号，1-100 
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedWriteMK_lib(uchar key_no, uchar key_len, uchar *key_data, ST_KCV_INFO *KcvInfoIn, uchar mode, uchar mkey_no)
{
	int iRet = PED_RET_ERROR;
	ushort ikeyno = key_no;
	int iCmdLen = 6 + 2 + 1 + key_len + sizeof(ST_KCV_INFO) + 1 + 2;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x13, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &ikeyno, 2);
	ucCmd[8] = key_len;
	memcpy(ucCmd + 9, key_data, key_len);
	memcpy(ucCmd + 9 + key_len, KcvInfoIn, sizeof(ST_KCV_INFO));
	ucCmd[9 + key_len + sizeof(ST_KCV_INFO)] = mode;
	memcpy(ucCmd + 9 + key_len + sizeof(ST_KCV_INFO) + 1, &mkey_no, 2);
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];

	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}



/*
*@Brief:		写工作密钥 
*@Param IN:		key_no [输入] 密钥序号，取值范围 1~100，其他值非法。 
				key_len[输入] 密钥长度,只能取 8、16、24 这 3 个值，其他值非法。 
				*key_data[输入] 密钥数据 
				*KcvInfoIn [输入] 详情参考 pedWriteKey 
				mode[输入] 写入密钥的模式，直接写入或者加密，解密，使用 DES 或 SM4 算法 
				Bit[0]： 0: 直接写入，其他 bit 位无效 1：加密或解密写入   Bit[6]: 0：使用 DES/3DES 算法 1：使用 SM4 算法 Bit[7]: 0：解密 1：加密 
				mkey_no[输入] 密钥号，1-100 
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedWriteWK_lib(uchar key_no, uchar key_len, uchar *key_data, ST_KCV_INFO *KcvInfoIn, uchar mode, uchar mkey_no)
{
	int iRet = PED_RET_ERROR;
	ushort ikeyno = key_no;
	int iCmdLen = 6 + 2 + 1 + key_len + sizeof(ST_KCV_INFO) + 1 + 2;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x14, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &ikeyno, 2);
	ucCmd[8] = key_len;
	memcpy(ucCmd + 9, key_data, key_len);
	memcpy(ucCmd + 9 + key_len, KcvInfoIn, sizeof(ST_KCV_INFO));
	ucCmd[9 + key_len + sizeof(ST_KCV_INFO)] = mode;
	memcpy(ucCmd + 9 + key_len + sizeof(ST_KCV_INFO) + 1, &mkey_no, 2);
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d\r\n", iRet);
	return iRet;
}


/*
*@Brief:		检查密钥是否存在
*@Param IN: 	ucKeyType [输入] 密钥类型 0:主密钥；1：Mac密钥；2：pin密钥；3：TD密钥
				usKeyID   [输入] 密钥索引 
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedKeyExist_lib(unsigned char ucKeyType, unsigned short usKeyID)
{
    int iRet = 0;
    int cmdLength = 6 + 3;
    unsigned char  *cmd = (unsigned char*)fibo_malloc(cmdLength);
    cmd[0] = 0x00;
    cmd[1] = 0xa1;
    cmd[2] = 0x18;
    cmd[3] = 0x01;
    cmd[4] = (cmdLength - 6) & 0xff;
    cmd[5] = ((cmdLength - 6) & 0xff00) >> 8;
    cmd[6] = ucKeyType;
    cmd[7] = usKeyID;
    cmd[8] = usKeyID>>8;
    Frame frm, retfrm;
    iRet = frameFactory(cmd, &frm, 0x40, cmdLength, 0x01, 0x00);
	free(cmd);
    if(iRet < 0)
    {
       goto RET_END;
    }
    iRet = transceiveFrame(frm, &retfrm, 2000);
    if(iRet <0)
    {
        goto RET_END;
    }
    iRet=retfrm.data[2]<<8 | retfrm.data[3];
    if(iRet == 0x9000)
    {
        iRet = 1;
    }
    else if(iRet == 0x9333)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }
    fibo_free(frm.data);
    fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d\r\n", iRet);
	return iRet;


}


/*
*@Brief:		用 KeyIdx 指定的 MAC 密钥对 DataIn 进行 Mode 指定的算法进行 MAC 运算,将 8 字 节的 MAC 结果输出到 MacOut 
*@Param IN:		KeyIdx [输入] 1~100   TAK 的索引 
				*DataIn [输入] 需进行 MAC 运算的数据包 
				DataInLen [输入] Mode != 2 时，<=1024 bytesMAC 运算的数据包的长度[输 入],长度不为 8 字节整除,则自动补“\x00” Mode ==2 时，长度无限制 
				Mode [输入]
*@Param OUT:	*MacOut [输出] 需进行 MAC 运算的数据包 
*@Return:		0:成功; <0:失败
*/
int pedGetMac_lib(uchar KeyIdx, uchar *DataIn, ushort DataInLen, uchar *MacOut, uchar Mode)
{
	int iRet = PED_RET_ERROR;
	ushort ikeyno = KeyIdx;
	int iCmdLen = 6 + 2 + DataInLen + 2 + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x04, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &ikeyno, 2);
	memcpy(ucCmd + 8, DataIn, DataInLen);
	memcpy(ucCmd + 8 + DataInLen, &DataInLen, 2);
	ucCmd[8 + DataInLen + 2] = Mode;

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		memcpy(MacOut, retfrm.data + 6 , 8);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d\r\n", iRet);
	return iRet;

}

/*
*@Brief:		使用 MK/WK 对数据进行 DES/TDES 运算,使用 DES 或 TDES 根据密钥的长度而定
*@Param IN:		key_no [输入] TDK 或 TWK 的索引，取值范围 1~100，其他值非法。
				key_part [输入] :取 0,1,2,3, 0-key 的前部组件 1-key 的组件 1（0-7 字节） 2-key 的组件 2（8-15 字节） 3-key 的组件 3（16-23 字节） 
				*indata [输入] 待加密数据。当采用国密算法时，indata 数据长度为 16 字 节，当采用 DES/3DES 算法时，indata 数据长度为 8 字节 
				Mode [输入] Bit[6] 1:国密 SM4 算法 0:DES/3DES 算法 Bit[5:4] 0：使用主密钥 MK ： 1：使用工作密钥 WK：Bit[0] 0x01：加密 0x00：解密
*@Param OUT:	*DataOut [输出] 指向已经运算后的数据
*@Return:		0:成功; <0:失败
*/
int pedCalcSym_lib (unsigned char key_no, unsigned char key_part, unsigned char *indata, unsigned char *outdata, unsigned char mode)
{
	sysLOG(API_LOG_LEVEL_2, "into  key_no = %d, key_part=%d\r\n", key_no, key_part);
    #define MODE_GM						0x40
	int iRet = PED_RET_ERROR;
	unsigned char ucModeGm;
	ushort DataInLen = 8;
	ucModeGm = mode & MODE_GM;
	if(ucModeGm)
	{
		DataInLen = 16;
	}
	ushort iKeyNo = key_no;
	int iCmdLen = 6 + 2 + 1 + DataInLen + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x15, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &iKeyNo, 2);
	ucCmd[8] = key_part;
	memcpy(ucCmd + 9, indata, DataInLen);
	ucCmd[9 + DataInLen] = mode;
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	
	if(0x9000 == iRet) {
		memcpy(outdata, retfrm.data + 6 , DataInLen);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}

/*
*@Brief:		使用 MK/WK 对数据进行 DES/TDES 运算,长度(8倍数)<=2048,
*@Param IN:		key_no [输入] TDK 或 TWK 的索引，取值范围 1~100，其他值非法。
				key_part [输入] :取 0,1,2,3, 0-key 的前部组件 1-key 的组件 1（0-7 字节） 2-key 的组件 2（8-15 字节） 3-key 的组件 3（16-23 字节） 
				*indata [输入] 待加密数据。当采用国密算法时，indata 数据长度为 16 字 节，当采用 DES/3DES 算法时，indata 数据长度为 8 字节 
				usDataLen[输入]输入数据长度
				Mode [输入] Bit[6] 1:国密 SM4 算法 0:DES/3DES 算法 Bit[5:4] 0：使用主密钥 MK ： 1：使用工作密钥 WK：Bit[0] 0x01：加密 0x00：解密
*@Param OUT:	*DataOut [输出] 指向已经运算后的数据
*@Return:		0:成功; <0:失败
*/
int pedBigDataCalcSym_lib(unsigned char key_no, unsigned char key_part, unsigned char *indata, unsigned short usDataLen, unsigned char *outdata, unsigned char mode)
{
	sysLOG(API_LOG_LEVEL_2, "into  key_no = %d, key_part=%d\r\n", key_no, key_part);
	int iRet = PED_RET_ERROR;
	unsigned char ucModeGm;
	ushort iKeyNo = key_no;
	int iCmdLen = 6 +  usDataLen + 6;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x24, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &iKeyNo, 2);
	ucCmd[8] = key_part;
	ucCmd[9] = usDataLen & 0xFF;
	ucCmd[10]= (usDataLen>>8) & 0xFF;
	memcpy(ucCmd + 11, indata, usDataLen);
	ucCmd[11 + usDataLen] = mode;
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	
	if(0x9000 == iRet) {
		memcpy(outdata, retfrm.data + 6 , usDataLen);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}

/*
*@Brief:		读取下一次计算的 KSN
*@Param IN:		GroupIdx [输入] 1~10 DUKPT group ID【最大密钥组索引号为 40】
*@Param OUT:	*KsnOut [输出] 10 bytes 当前的 KSN 
*@Return:		0:成功; <0:失败
*/
int pedGetDukptKSN_lib(uchar GroupIdx,	uchar * KsnOut)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x11, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = GroupIdx;

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		memcpy(KsnOut, retfrm.data + 6 , 10);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}

RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d\r\n", iRet);
	return iRet;

}



/*
*@Brief:		KSN 加 1
*@Param IN:		GroupIdx [输入] 1~10 DUKPT group ID【最大密钥组索引号为 40】
*@Param OUT:	null
*@Return:		0:成功; <0:失败
*/
int pedDukptIncreaseKsn_lib(uchar GroupIdx)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x12, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = GroupIdx;

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}

RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d\r\n", iRet);
	return iRet;

}

/*
*@Brief:		使用 DUKPT 的 MAC 密钥计算 MAC
*@Param IN:		GroupIdx [输入] [1~10]  DUKPT 密钥组索引号 【最大密钥组索引号为 40】 
				*DataIn [输入] 指向需要计算 MAC 的数据内容 
				DatainLen [输入] 数据的长度[输入]<=1024,长度不为 8 字节整除,则 自动补“\x00” 
				Mode [输入] 见API接口规范定义
*@Param OUT:	*MacOut [输出] 指向得到的 MAC 长度为DatainLen
                *KsnOut [输出] 指向当前的 KSN 10字节
*@Return:		0:成功; <0:失败
*/
int pedGetMacDukpt_lib(uchar GroupIdx, uchar* DataIn, ushort DatainLen, uchar * MacOut, uchar * KsnOut, uchar Mode)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 2 + DatainLen + 1;
	//unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x07, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x22, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = GroupIdx;
#if 0
	ucCmd[7] = DatainLen;
	ucCmd[8] = Mode;
	memcpy(ucCmd + 9, DataIn, DatainLen);
#else
	memcpy(ucCmd + 7, &DatainLen, 2);
	ucCmd[9] = Mode;
	memcpy(ucCmd + 10, DataIn, DatainLen);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		memcpy(KsnOut, retfrm.data + 6 , 10);
		memcpy(MacOut, retfrm.data + 16 , DatainLen);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }
RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}

/*
*@Brief:		使用 DUKPT 的 MAC 密钥或 DES 密钥，对输入缓存内数据进行加密或解密
*@Param IN:		GroupIdx [输入] [1~10]  DUKPT 密钥组索引号 【最大密钥组索引号为 40】 
				KeyVarType [输入] 0x00，用请求和应答 MAC 密钥 0x01，用 DUKPT DES 密钥运算 0x02, 用 DUKPT PIN 密钥做 ECB 加密（即取值该密钥分 量类型时，Mode 只能取值 0x01:ECB 加密） 
				*pucIV[输入] 8 字节初始向量，CBC 加解密时需要，如果传入 NULL， 将默认用“\x00\x00\x00\x00\x00\x00\x00\x00”作为初始向 量。 
				DataInLen [输入] 数据长度<=1024，8 整除 
				*DataIn [输入] 指向需要进行运算的数据 
				Mode [输入] 0x00:ECB 解密 0x01:ECB 加密 0x02:CBC 解密 0x03:CBC 加密 
*@Param OUT:	*DataOut [输出] 指向已经运算后的数据
                *KsnOut [输出] 指向当前的 KSN 10字节
*@Return:		0:成功; <0:失败
*/
int pedDukptDes_lib(uchar GroupIdx, uchar KeyVarType,	uchar *pucIV, ushort DataInLen, uchar * DataIn, uchar * DataOut, uchar * KsnOut , uchar Mode)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1 + 1 + 8 + 2 + DataInLen + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x23, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	
	memset(ucCmd, 0, iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = GroupIdx;
	ucCmd[7] = KeyVarType;
	if(((Mode == 0x02) || (Mode == 0x03)) && (pucIV == NULL))
	{

	}
	else if(pucIV != NULL)
	{
		memcpy(ucCmd + 8, pucIV, 8);
	}
#if 0
	ucCmd[16] = DataInLen;
	memcpy(ucCmd + 17, DataIn, DataInLen);
	ucCmd[17 + DataInLen] = Mode;
#else
	memcpy(ucCmd + 16, &DataInLen, 2);
    memcpy(ucCmd + 18, DataIn, DataInLen);
	ucCmd[18 + DataInLen] = Mode;
#endif

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
	iRet = transceiveFrame(frm, &retfrm, 5000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		memcpy(KsnOut, retfrm.data + 6 , 10);
		memcpy(DataOut, retfrm.data + 16 , DataInLen);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }

RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}


/*
*@Brief:		获取密钥的KCV值,以供对话双方进行密钥验证,用指定的密钥及算法对一段数据 进行加密,并返回部分数据密文
*@Param IN:		KeyType [输入] PED_TLK PED_TMK PED_TAK PED_TPK PED_TDK PED_TIK 
				KeyIdx [输入] 密钥的索引号,如： TLK,只能为 1。 TMK 可取值 1~100。 TWK 可取值 1~100。 TIK 可取值为 1~10。 
*@Param OUT:	*KcvInfoOut [输入] [输出] 
*@Return:		0:成功; <0:失败
*/
int pedGetKcv_lib(uchar KeyType, uchar KeyIdx, ST_KCV_INFO *KcvInfoOut)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1 + 2 + sizeof(ST_KCV_INFO);
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x0A, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = KeyType;
	memcpy(ucCmd + 7, &KeyIdx, 2);
	memcpy(ucCmd + 9, KcvInfoOut, sizeof(ST_KCV_INFO));
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
    memset(caShow, 0, sizeof(caShow));
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		memcpy(KcvInfoOut, retfrm.data + 6 , sizeof(ST_KCV_INFO));
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }

RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d\r\n", iRet);
	return iRet;

}


/*
*@Brief:		注入 RSA 密钥到 PED
*@Param IN:		RSAKeyIndex  [输入] 1byte 密钥索引 the index of RSAKEY, [1~10]; 
				*pstRsakeyIn [输入] RSA 
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int pedWriteRSAKey_lib (uchar RSAKeyIndex, ST_RSA_KEY* pstRsakeyIn)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1 + sizeof(ST_RSA_KEY);
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x0E, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = RSAKeyIndex;
	memcpy(ucCmd + 7, pstRsakeyIn, sizeof(ST_RSA_KEY));
#ifdef PRINT_API_CMD	
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
    memset(caShow, 0, sizeof(caShow));
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }

RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;

}

/*
*@Brief:		用存储在 PED 的 RSA 密钥进行数据 RSA 数据运算
*@Param IN:		RSAKeyIndex [输入] 1byte 密钥索引[1~10]; 
				*pucDataIn [输入] 被加解密的数据,和模等长。
*@Param OUT:	*pucDataOut [输出] 加密或解密后的数据 
				*pucKeyInfoOut [输出] 密钥信息,当 pucKeyInfoOut 等于 NULL 时,不输出密钥信 息。 
*@Return:		0:成功; <0:失败
*/
int PeCalcRSA_lib(uchar RSAKeyIndex, uchar *pucDataIn, ushort DataInLen, uchar * pucDataOut, uchar* pucKeyInfoOut)
{
    if(DataInLen > 512)
    {
    	DataInLen = DataInLen / 8;
    }
	if((DataInLen > 512) || (DataInLen < 64))
	{
	    return PED_RET_ERR_KEY_LEN_ERR;
	}
    	
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1 + 2 + DataInLen;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x0F, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = RSAKeyIndex;
	memcpy(ucCmd + 7, &DataInLen, 2);
	memcpy(ucCmd + 9, pucDataIn, DataInLen);
#ifdef PRINT_API_CMD	
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
    memset(caShow, 0, sizeof(caShow));
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
		//output_len = retfrm.data[5]<<8 | retfrm.data[4];
		DataInLen = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		memcpy(pucDataOut, retfrm.data + 6+4, DataInLen);
		if(pucKeyInfoOut)
		{
			memcpy(pucKeyInfoOut, retfrm.data + 6 +4+256, 128);
		}
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }

RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}


/*
*@Brief:		清除 PED 里的所有密钥信息
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int pedErase_lib(void)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x17, iCmdLen-6, (iCmdLen -6) >> 8};
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
	iRet = transceiveFrame(frm, &retfrm, 20000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}


/*
*@Brief:		返回 PED 的版本
*@Param IN:		NULL
*@Param OUT:	*VerInfoOut [输出] 指向当前 PED 的版本信息,最大 16 个字节。 
*@Return:		0:成功; <0:失败
*/
int pedGetVer_lib(uchar * VerInfoOut)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x0C, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 2000);
	if(iRet <0) {
		fibo_free(frm.data);
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	fibo_free(frm.data);
	if(0x9000 == iRet) {
		memcpy(VerInfoOut, retfrm.data + 6 , strlen(retfrm.data + 6));
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }
RET_END:
	fibo_free(retfrm.data);
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}


/*
*@Brief:		指定的时限内,扫描键盘上输入的 PIN 并输出 PIN BLOCK 加密数据块。
*@Param IN:		ucKeyIdx [输入] 1~100   TPK 的索引 
				pucExpPinLenIn [输入] 0~12 的枚举集合 此参数不起作用
				*pucDataIn [ 输 入]  
				ucMode [输入] 
				uiTimeOutMs[输入] 此参数不起作用
*@Param OUT:	*pucPinBlockOut [输出] 8bytes  指向生成的 PINBlock 
*@Return:		0:成功; <0:失败
*/
int pedGetPinBlock_lib(unsigned char ucKeyIdx, 
				  unsigned char *pucExpPinLenIn,  
				  unsigned char *pucDataIn,  unsigned short uiDataInLen,
				  unsigned char *pucPinBlockOut, 
				  unsigned char ucMode,   unsigned int uiTimeOutMs)
{
	int iRet = PED_RET_ERROR;
	ushort ikeyno = ucKeyIdx;
	int iCmdLen = 6 + 2 + uiDataInLen + 1 + 4;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x03, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memset(ucCmd, 0, iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, &ikeyno, 2);
	memcpy(ucCmd + 8, pucDataIn, uiDataInLen);
	ucCmd[8+uiDataInLen] = ucMode;
	memcpy(ucCmd + 8 + uiDataInLen +1, &uiTimeOutMs, 4);
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		memcpy(pucPinBlockOut, retfrm.data + 6 , 8);
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
    else
    {
        iRet = -iRet;
    }
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;

}


				  
/*
*@Brief:		在 PED 上输入 PIN,并使 DUKPT 的 PIN 密钥计算 PINBlock。 
*@Param IN:	  	ucGroupIdx [输入] [1~10]  DUKPT 密钥组索引号 
		  		pucExpPinLenIn [输入] 0~12 的枚举集合 此参数不起作用
		  		*pucDataIn [ 输 入]
		  		ucMode [输入] 
		  		uiTimeOutMs[输入] 此参数不起作用
*@Param OUT:	*pucKsnOut [输出]   指向当前的 KSN 
				*pucPinBlockOut [输出] 8bytes  指向生成的 PINBlock 
*@Return: 	  0:成功; <0:失败
*/
int pedGetPinDukpt_lib(unsigned char ucGroupIdx, unsigned char *pucExpPinLenIn,	unsigned char *pucDataIn, unsigned short uiDataInLen, unsigned char *pucKsnOut, 
			unsigned char *pucPinBlockOut, unsigned char ucMode, unsigned int uiTimeOutMs)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 1 + 1 + uiDataInLen;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x22, 0x06, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memset(ucCmd, 0, iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = ucGroupIdx;
	ucCmd[7] = ucMode;
	memcpy(ucCmd + 8, pucDataIn, uiDataInLen);
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
	iRet = transceiveFrame(frm, &retfrm, 2000);
	fibo_free(frm.data);
	if(iRet <0) {
	  goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
	  memcpy(pucKsnOut, retfrm.data + 6 , 10);
	  memcpy(pucPinBlockOut, retfrm.data + 16 , 8);
	  iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
	  if(retfrm.length >= 10)
	  {
		  iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
	  }
	}
	else
	{
	  iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
return iRet;

}

			
/*
*@Brief:		查询 PED 状态
*@Param IN: 	NULL
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int GetPedState(void)
{
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x02, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, " transceiveFrame,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	sysLOG(API_LOG_LEVEL_2, " iRet = %d, iRet=0x%x\r\n", iRet, iRet);
#if 0
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9399)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
#endif
	fibo_free(retfrm.data);
    sysDelayMs(200);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}
/*
*@Brief:		验证下发的AB管理员密码正确性
*@Param IN: 	adminA adminB
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int iPedAdminABLogOn(char* adminA, char * adminB)
{
	//sysLOG(API_LOG_LEVEL_2, "  adminA = %s, adminB = %s\r\n", adminA, adminB);
    if(strcmp(adminA, "28210528") != 0 || strcmp(adminB, "31210531") != 0)
    {
        return PED_RET_ERR_CHECK_KEY_FAIL;
    }
	
    if(strlen(adminA) != 8 || strlen(adminB) != 8)
    {
        return PED_RET_ERR_KEY_LEN_ERR;
    }
	int iRet = PED_RET_ERROR;
	int iCmdLen = 6 + 18;
	unsigned char ucCmdHead[6] = {0x00, 0xA1, 0x23, 0x01, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = 8;
	memcpy(ucCmd + 7, adminA, 8);
	ucCmd[7+8] = 8;
	memcpy(ucCmd + 16, adminB, 8);
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
	sysLOG(API_LOG_LEVEL_2, " transceiveFrame,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	sysLOG(API_LOG_LEVEL_2, " iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	if(0x9000 == iRet) {
		iRet = PED_RET_OK;
	}
	else if(iRet == 0x9369)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else if(iRet >= 0x9370)
	{
		if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
	}
	else
	{
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, " RET_END,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	return iRet;
}

/*
*@Brief:		读ped测试日志接口，字符串输出
*@Param IN: 	NULL
*@Param OUT:	*log_data:日志内容，字符串类型
*@Return:		<0:失败；>=0:成功，读到的字节数
*/
int32 DR_ReadPedTestLog(int8 *getlogInfo, int8 *log_data)
{
	int32 fd;
	int32 iRet, Ret;
	int32 filesizeofLogString = 0;
	int8 *fileofLogString = NULL;
	int8 LogStringName[64];
	int iOffset = 0;
	int iReadLen = 0;

	memcpy(&iReadLen, getlogInfo, 4);
	memcpy(&iOffset, getlogInfo+4, 4);
	sysLOG(PWR_LOG_LEVEL_2, "iReadLen=%d, iOffset=%d\r\n", iReadLen, iOffset);

	memset(LogStringName, 0, sizeof(LogStringName));
	//sprintf(LogStringName, "/a.txt");
	strcpy(LogStringName, "/API/ped.txt");
	iRet = hal_fileExist(LogStringName);
	sysLOG(PWR_LOG_LEVEL_2, "LogStringName=%s, hal_fileExist iRet=%d\r\n", LogStringName, iRet);
	if(iRet <= 0)//文件不存在
	{
		return iRet;
	}
	iRet = hal_fileGetFileSize(LogStringName);
	sysLOG(PWR_LOG_LEVEL_2, "LogStringName=%s, hal_fileGetFileSize iRet=%d\r\n", LogStringName, iRet);
	
	if(iRet <= 0)//获取文件大小失败或者文件大小为0
	{
		return iRet;
	}
	if(iRet < iReadLen + iOffset)
	{
		return -99;
	}
	filesizeofLogString = iReadLen;
	/*打开文件*/
	fd = hal_fileOpen(LogStringName, O_RDONLY);
	if(fd < 0)
	{
		sysLOG(PWR_LOG_LEVEL_2, "<ERR> hal_fileOpen fd=%d\r\n", fd);
		return fd;
	}
	
	iRet = hal_fileSeek(fd, iOffset, SEEK_SET);
	if(iRet < 0)
	{
	
		sysLOG(PWR_LOG_LEVEL_2, "<ERR> Seek File Fail %d\r\n", iRet);
		return iRet;
	}

	fileofLogString = malloc(filesizeofLogString+1);
	if(fileofLogString == NULL)return -1;
	memset(fileofLogString,0,sizeof(fileofLogString));
	iRet = hal_fileRead(fd, fileofLogString,filesizeofLogString);
	sysLOG(PWR_LOG_LEVEL_2, "LogStringName=%s, hal_fileRead iRet=%d\r\n", LogStringName, iRet);
	if(iRet<0)
	{		
		sysLOG(PWR_LOG_LEVEL_2, "<ERR> Read File Fail %d\r\n", iRet);
		free(fileofLogString);
		return iRet;
	}
	else if(iRet==0)
	{
		Ret = hal_fileClose(fd);
		if(Ret != 0)
		{			
			sysLOG(PWR_LOG_LEVEL_2, "<ERR> Close File Fail %d\r\n", Ret);
			free(fileofLogString);
			return Ret;
		}
		
		sysLOG(PWR_LOG_LEVEL_2, "<ERR> ped log read NULL\r\n");
		free(fileofLogString);
		return iRet;
	}
	else
	{
		Ret = hal_fileClose(fd);
		if(Ret != 0)
		{
			sysLOG(PWR_LOG_LEVEL_2, "<ERR> Close File Fail %d\r\n", Ret);
			free(fileofLogString);
			return Ret;
		}
        //if(filesizeofLogString > 4096)
        //{
		//	memcpy(log_data, fileofLogString, 4096);
		//	filesizeofLogString = 4096;
        //}
		//else
		{
		    memcpy(log_data, fileofLogString, filesizeofLogString);
		}
		sysLOG(PWR_LOG_LEVEL_2, "<SUCC> ped log read OK! filesizeofLogString:%d\r\n", filesizeofLogString);
		free(fileofLogString);
		return filesizeofLogString;
	}
}

#if 0
int myPedPlaintextTest(char *Tilte,uchar sKeyTp,uchar sKeyIdx,uchar DKeyTp,
						 uchar DKeyIdx,int iDKeyLen,int CheckMode)
{
  uchar Tlk[]={0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79,
			  0x89,0x9b,0xab,0xbc,0xcd,0xdf,0xef,0xe0,
			  0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79
						  };
  ST_KEY_INFO  KeyInfoIn;
  ST_KCV_INFO  KcvInfoIn;
  uchar KcvBuf[32];
  int Ret;

  memset(&KeyInfoIn , 0, sizeof(KeyInfoIn));
  memset(&KcvInfoIn , 0, sizeof(KcvInfoIn));
  memset(KcvBuf, 0, sizeof(KcvBuf));

  //api_pedErase();
  

  KeyInfoIn.ucSrcKeyType= sKeyTp;
  KeyInfoIn.ucSrcKeyIdx = sKeyIdx;
  KeyInfoIn.ucDstKeyType = DKeyTp;
  KeyInfoIn.ucDstKeyIdx  = DKeyIdx;
  KeyInfoIn.iDstKeyLen	= iDKeyLen;

  KcvInfoIn.iCheckMode = CheckMode;

  memcpy(KeyInfoIn.aucDstKeyValue, Tlk, KeyInfoIn.iDstKeyLen);
  if(iDKeyLen == 24)
  {
	  if(CheckMode == 1)  
	  {
		  memcpy(KcvInfoIn.aucCheckBuf, "\x04\x84\x99\x56\xB9\x64\xAA\xC9\x17", 9);
	  }
	  else if(CheckMode == 2) 
	  {
		  memcpy(KcvInfoIn.aucCheckBuf, "\x04\xA4\xE5\x8A\x72\x04\x35\x9B\xF1", 9);
	  }
  }
  else if(iDKeyLen == 16)
  {
	  if(CheckMode == 1)  
	  {
		  memcpy(KcvInfoIn.aucCheckBuf, "\x04\x84\x99\x56\xB9\x64\xAA\xC9\x17", 9);
	  }
	  else if(CheckMode == 2) 
	  {
		  memcpy(KcvInfoIn.aucCheckBuf, "\x04\xA4\xE5\x8A\x72\x04\x35\x9B\xF1", 9);
	  }
  }
  else if(iDKeyLen == 8)
  {
	  if(CheckMode == 1)  
	  {
		  memcpy(KcvInfoIn.aucCheckBuf, "\x04\x6D\xD8\xC8\xFB\xC0\x96\xA7\x09", 9);
	  }
	  else if(CheckMode == 2) 
	  {
		  memcpy(KcvInfoIn.aucCheckBuf, "\x04\x6F\x2D\xB0\x19\x3A\x9E\xAB\x8B", 9);
	  }
  }

  Ret = pedWriteKey_lib(&KeyInfoIn, & KcvInfoIn);
  sysLOG(API_LOG_LEVEL_2, "%s:  pedWriteKey_lib,iRet = %d\r\n", Tilte, Ret);
  return Ret;
}

//将数据从字符串表示转换为二进制表示
//例如 "13A06D49" =〉{0x13, 0xa0, 0x6d, 0x49}
int StrToHex(
		  uchar   *pbStr, 
		  unsigned int	  dwLen, 
		  uchar   *pbHex
		  )
{
  uchar   b;
  uchar   *pbTmp = (uchar*)fibo_malloc(dwLen + 2);//相对足够大
  int	  i;

  for(i=0; i<dwLen; i++) {
	  b = pbStr[i];
	  if((b>='a') && (b<='f')) {
		  b = b - 'a' + 0xa;
	  }
	  else if((b>='A') && (b<='F')) {
		  b = b - 'A' + 0xa;
	  }
	  else if((b>='0') && (b<='9')) {
		  b = b - '0';
	  }
	  else {
		  return 1;
	  }

	  pbTmp[i] = b;
  }

  for(i=0; i<dwLen/2; i++) {
	  pbHex[i] = pbTmp[i*2] * 16 + pbTmp[i*2+1];
  }

  fibo_free(pbTmp);

  return 0;
}


 int PedCiphertextTest(char *Tilte,uchar sKeyTp,uchar sKeyIdx,uchar DKeyTp,
							 uchar DKeyIdx,int iDKeyLen,int CheckMode, int MACMode)
 {
	 uchar Tlk[]={0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79,0x89,0x9b,0xab,0xbc,0xcd,0xdf,0xef,0xe0,0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79};
	 uchar Tlkpwd[]={0xA7,0x9E,0x4A,0x95,0xE2,0x0C,0x0E,0xF7,0xF8,0xD9,0xC7,0x8A,0x6C,0x29,0x59,0xD3,0xA7,0x9E,0x4A,0x95,0xE2,0x0C,0x0E,0xF7};
	 
	 ST_KEY_INFO  KeyInfoIn;
	 ST_KCV_INFO  KcvInfoIn;
	 uchar KcvBuf[32];
	 int Ret;
 
	 memset(&KeyInfoIn , 0, sizeof(KeyInfoIn));
	 memset(&KcvInfoIn , 0, sizeof(KcvInfoIn));
	 memset(KcvBuf, 0, sizeof(KcvBuf));
 
	 //api_pedErase();
	 if(DKeyTp == PED_TLK)
	 {
		 KeyInfoIn.ucSrcKeyType= 0;
		 KeyInfoIn.ucSrcKeyIdx = 0;
		 KeyInfoIn.ucDstKeyType = PED_TLK;
		 KeyInfoIn.ucDstKeyIdx	= 1;
		 KeyInfoIn.iDstKeyLen  = 24;
		 KcvInfoIn.iCheckMode = 0;
		 memcpy(KeyInfoIn.aucDstKeyValue, Tlk, KeyInfoIn.iDstKeyLen);
	 }
	 else
	 {
		 KeyInfoIn.ucSrcKeyType= 0;
		 KeyInfoIn.ucSrcKeyIdx = 0;
		 KeyInfoIn.ucDstKeyType = PED_TMK;
		 KeyInfoIn.ucDstKeyIdx	= 1;
		 KeyInfoIn.iDstKeyLen  = 24;
		 KcvInfoIn.iCheckMode = 0;
		 memcpy(KeyInfoIn.aucDstKeyValue, Tlk, KeyInfoIn.iDstKeyLen);
	 }
	 pedWriteKey_lib(&KeyInfoIn, & KcvInfoIn);//写明文的密钥，作为源密钥使用
 
	 KeyInfoIn.ucSrcKeyType= sKeyTp;
	 KeyInfoIn.ucSrcKeyIdx = sKeyIdx;
	 KeyInfoIn.ucDstKeyType = DKeyTp;
	 KeyInfoIn.ucDstKeyIdx	= DKeyIdx;
	 KeyInfoIn.iDstKeyLen  = iDKeyLen;
	 KcvInfoIn.iCheckMode = CheckMode;
	 memcpy(KeyInfoIn.aucDstKeyValue, Tlkpwd, KeyInfoIn.iDstKeyLen);
	 
	 if(iDKeyLen == 24)
	 {
		 if(CheckMode == 1)  //对8个字节的0x00计算DES/TDES加密,得到的密文的前4个字节即为KCV
			 memcpy(KcvInfoIn.aucCheckBuf, "\x04\x84\x99\x56\xB9\x64\xAA\xC9\x17", 9);
		 else if(CheckMode == 2) //奇校验
			 memcpy(KcvInfoIn.aucCheckBuf, "\x04\xA4\xE5\x8A\x72\x04\x35\x9B\xF1", 9);
		 else if(CheckMode == 3) // mac校验
		 {
			 if(MACMode == 0)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x00\x08\x15\x0B\x28\x79\xD5\x35\xC5\xD2",11);
			 else if(MACMode == 1)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x01\x08\xD4\x1F\xCE\x77\x52\x64\x96\xC7",11);
			 else if(MACMode == 2)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x02\x08\xBD\x0C\x26\x2F\x8A\x90\xC9\x2C",11);
		 }
	 }
	 else if(iDKeyLen == 16)
	 {
		 if(CheckMode == 1)  //对8个字节的0x00计算DES/TDES加密,得到的密文的前4个字节即为KCV
			 memcpy(KcvInfoIn.aucCheckBuf, "\x04\x84\x99\x56\xB9\x64\xAA\xC9\x17", 9);
		 else if(CheckMode == 2) //奇校验
			 memcpy(KcvInfoIn.aucCheckBuf, "\x04\xA4\xE5\x8A\x72\x04\x35\x9B\xF1", 9);
		 else if(CheckMode == 3) // mac校验
		 {
			 if(MACMode == 0)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x00\x08\x54\x35\x06\x57\xC1\x7A\x50\xA8",11);
			 else if(MACMode == 1)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x01\x08\xE6\xAB\x5C\x4A\xBC\x66\xC3\x9D",11);
			 else if(MACMode == 2)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x02\x08\x05\x98\x9D\xBA\xC2\x91\xBB\xFA",11);
		 }
	 }
	 else if(iDKeyLen == 8)
	 {
		 if(CheckMode == 1)  //对8个字节的0x00计算DES/TDES加密,得到的密文的前4个字节即为KCV
			 memcpy(KcvInfoIn.aucCheckBuf, "\x04\x6D\xD8\xC8\xFB\xC0\x96\xA7\x09", 9);
		 else if(CheckMode == 2) //奇校验
			 memcpy(KcvInfoIn.aucCheckBuf, "\x04\x6F\x2D\xB0\x19\x3A\x9E\xAB\x8B", 9);
		 else if(CheckMode == 3) // mac校验
		 {
			 if(MACMode == 0)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x00\x08\x02\x74\xB3\xB3\x99\x13\xE5\x6A",11);
			 else if(MACMode == 1)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x01\x08\x02\x74\xB3\xB3\x99\x13\xE5\x6A",11);
			 else if(MACMode == 2)
				 memcpy(KcvInfoIn.aucCheckBuf, "\x00\x02\x08\x02\x74\xB3\xB3\x99\x13\xE5\x6A",11);
		 }
	 }
	 
	 Ret = pedWriteKey_lib(&KeyInfoIn, & KcvInfoIn);
	 sysLOG(API_LOG_LEVEL_2, "%s:  pedWriteKey_lib,iRet = %d\r\n", Tilte, Ret);
	 return Ret;
 }
 int WriteKeyDBufLen(int CheckMode, int StartLen,int EndLen,int PreRet)
 {
	 uchar Tlk[]={0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79,0x89,0x9b,0xab,0xbc,0xcd,0xdf,0xef,0xe0,0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79};
	 uchar Tlkpwd[]={0xA7,0x9E,0x4A,0x95,0xE2,0x0C,0x0E,0xF7,0xF8,0xD9,0xC7,0x8A,0x6C,0x29,0x59,0xD3,0xA7,0x9E,0x4A,0x95,0xE2,0x0C,0x0E,0xF7};
	 const char *TempBuf[5] ={PED_TLK,PED_TMK,PED_TPK,PED_TAK,PED_TDK};
	 const char *TitleBuf[5] ={"写TLK","写TMK","写TPK","写TAK","写TDK"};
	 ST_KEY_INFO  KeyInfoIn;
	 ST_KCV_INFO  KcvInfoIn;
	 uchar KcvBuf[32];
	 int Ret,i,j,ErrFlag = 1;
 
	 memset(&KeyInfoIn , 0, sizeof(KeyInfoIn));
	 memset(&KcvInfoIn , 0, sizeof(KcvInfoIn));
	 memset(KcvBuf, 0, sizeof(KcvBuf));
 
	 //api_pedErase();
	 
	 KeyInfoIn.ucSrcKeyType= 0;
	 KeyInfoIn.ucSrcKeyIdx = 0;
	 KeyInfoIn.ucDstKeyType = PED_TLK;
	 KeyInfoIn.ucDstKeyIdx	= 1;
	 KeyInfoIn.iDstKeyLen  = 24;
	 KcvInfoIn.iCheckMode = 0;
	 memcpy(KeyInfoIn.aucDstKeyValue, Tlk, KeyInfoIn.iDstKeyLen);
 
	 pedWriteKey_lib(&KeyInfoIn, & KcvInfoIn);//写明文的密钥，作为源密钥使用
 
	 for(i = 0; i < 5; i++)
	 {
		 KeyInfoIn.ucSrcKeyType= PED_TLK;
		 KeyInfoIn.ucSrcKeyIdx = 1;
		 KeyInfoIn.ucDstKeyType = TempBuf[i];
		 KeyInfoIn.ucDstKeyIdx	= 1;
		 KeyInfoIn.iDstKeyLen  = 16;
		 KcvInfoIn.iCheckMode = CheckMode;
		 memcpy(KeyInfoIn.aucDstKeyValue, Tlkpwd, KeyInfoIn.iDstKeyLen);
	 
		 if(CheckMode == 1)  //对8个字节的0x00计算DES/TDES加密,得到的密文的前4个字节即为KCV
			 memcpy(KcvInfoIn.aucCheckBuf+1, "\x84\x99\x56\xB9\x64\xAA\xC9\x17", 8);
		 else if(CheckMode == 2) //奇校验
			 memcpy(KcvInfoIn.aucCheckBuf+1, "\xA4\xE5\x8A\x72\x04\x35\x9B\xF1", 8);
		 
		 for(j = StartLen;j<=EndLen;j++)
		 {
			 KcvInfoIn.aucCheckBuf[0]=j;
			 Ret = pedWriteKey_lib(&KeyInfoIn, & KcvInfoIn);
              sysLOG(API_LOG_LEVEL_2, "%s:  pedWriteKey_lib,iRet = %d\r\n", TitleBuf[i], Ret);

				 if(Ret == PreRet)
					 ErrFlag = 0;
				 //else
					 //break;
			 sysDelayMs(300);
		 }
		 
	 }
	 return ErrFlag;
 }

 unsigned char key_Data[102][24]=
 {
	 {0x04,0x57,0x92,0xDC,0x8A,0xF2,0x91,0x79,0x19,0x67,0x92,0x20,0x04,0x8A,0x7C,0x6E,0x01,0x73,0xEF,0x46,0xE0,0x0E,0x3D,0xFE},
	 {0xC1,0xBF,0x8C,0x92,0x38,0xF4,0xEA,0xAD,0xBC,0xC8,0xF8,0x97,0x54,0x76,0xE5,0x0E,0x83,0x7F,0x97,0x1A,0xC2,0xB3,0x29,0x8F},
	 {0x67,0x7F,0xBF,0xA8,0x49,0x43,0x49,0x54,0x23,0xFD,0x45,0x3D,0xA7,0x0D,0x8C,0x85,0xF2,0x62,0x3E,0x9E,0x54,0xFE,0x8C,0x04},
	 {0x91,0x08,0xBC,0x26,0xC8,0xA7,0x4A,0xF8,0x4A,0x3D,0x5D,0x08,0xF1,0x7C,0xFB,0xCE,0x76,0xA2,0xCD,0xBF,0x5B,0xA7,0x94,0x31},
	 {0x98,0x6D,0xB5,0xC2,0xFB,0x38,0x7F,0xE0,0x89,0x8C,0x6D,0x4F,0x37,0xBA,0x86,0xE6,0x13,0x32,0x0E,0x86,0xE3,0x91,0xA8,0x80},
	 {0x01,0xC4,0x46,0x9B,0xC8,0x62,0x46,0x2C,0x91,0x76,0xC7,0x80,0x5D,0x9B,0x6B,0x04,0x92,0xEF,0xC1,0xB3,0xB6,0xFD,0x49,0x20},
	 {0x6E,0x20,0x73,0x08,0x3B,0x32,0xCB,0x6E,0x75,0x0D,0xEA,0x08,0x07,0x8A,0xCD,0x5D,0xF2,0x58,0x92,0x64,0xCB,0x19,0x6B,0xA2},
	 {0xBC,0xDF,0xC7,0x9E,0xAE,0x1C,0x25,0xAB,0x5D,0x7C,0x16,0x23,0xDC,0xE5,0x13,0x70,0x70,0xFB,0x80,0x0D,0x83,0x92,0x62,0x6B},
	 {0xA4,0xC1,0x25,0xA7,0x4A,0x57,0x08,0x83,0xF4,0x2C,0x0B,0xEF,0xD0,0x51,0xA7,0x6E,0xE6,0xA1,0xA8,0x89,0x61,0x4A,0x46,0x34},
	 {0xA1,0x75,0x4F,0xF8,0x40,0xE9,0x92,0x20,0xBC,0x08,0x92,0x4F,0xAD,0x61,0x75,0x9B,0x45,0xFD,0xFB,0x86,0x5D,0xB5,0xBF,0x98},
	 {0xC2,0xAE,0x9E,0x52,0x0B,0x67,0x10,0x7A,0x45,0xC4,0x43,0xFB,0xD3,0x62,0x1A,0x67,0xCD,0xD9,0xDC,0xB0,0x64,0x4F,0x15,0x2C},
	 {0x9B,0x49,0xF2,0x01,0xB0,0xEC,0x15,0xB6,0xCB,0x75,0x2A,0x2C,0xDA,0x2A,0x97,0x4C,0x8F,0x97,0x08,0x9D,0x86,0xC8,0xCE,0xB3},
	 {0x4A,0x7C,0x97,0xDF,0xBA,0x8A,0xBC,0x0B,0xE5,0x0B,0x45,0xB0,0xAE,0xEC,0xC8,0xE0,0x0D,0xFD,0x75,0xD6,0xEC,0xC1,0xEC,0xFD},
	 {0xB0,0xC4,0x45,0xC7,0xF4,0x49,0xBF,0x91,0x7A,0x9E,0x5E,0x08,0x70,0xF1,0x0D,0xA8,0xD6,0x3D,0x1F,0x7A,0xBA,0x7A,0xEC,0x1A},
	 {0xB9,0x02,0x37,0xEA,0xC8,0x98,0x76,0xC7,0x51,0xC1,0xF7,0x8C,0x91,0xFB,0xDA,0x52,0xCD,0xE3,0x52,0x2A,0x1F,0x4F,0x6B,0x4C},
	 {0xEF,0xC8,0x51,0xF1,0x13,0xEF,0xA2,0x0B,0xEA,0x3B,0x64,0xF2,0x34,0x7A,0x68,0x6E,0xF4,0x49,0x26,0xA7,0x80,0x19,0xF7,0x3E},
	 {0x51,0xE5,0x10,0x3D,0x0B,0xF4,0x2F,0x73,0x97,0x01,0x0E,0x4F,0x1C,0x10,0x2C,0xF8,0x1A,0xA4,0xB9,0x68,0x80,0xC4,0xAB,0xE6},
	 {0x34,0x7F,0x8A,0x9B,0xAE,0x6D,0x1C,0xD9,0xA7,0x43,0x9E,0x8A,0x08,0x58,0xF7,0x0B,0x73,0x5B,0x2C,0x73,0xE5,0x3D,0xE9,0x45},
	 {0x34,0x5B,0x1C,0xB9,0x89,0xB3,0xCD,0xE9,0x07,0x3E,0xB0,0x1C,0x20,0xB0,0x9B,0x67,0x13,0x7C,0xE0,0x04,0x9E,0x3E,0xB6,0x9B},
	 {0x3E,0x10,0x64,0x34,0xEA,0x8F,0x52,0xC1,0xE3,0xB3,0x49,0xC1,0x58,0xB0,0xA4,0x4C,0xD0,0xD9,0x76,0x54,0xF8,0xE6,0x94,0xF1},
	 {0x49,0xE5,0x89,0x9E,0x4F,0x04,0xEA,0x16,0x26,0x75,0x46,0xD3,0xCE,0xD6,0x32,0xB5,0x8C,0x8F,0x67,0xF8,0xE3,0x25,0xCB,0x76},
	 {0xE0,0x43,0x8A,0x6D,0xC2,0xC8,0xE3,0x9B,0x79,0x46,0x94,0xC2,0xCB,0xB3,0x40,0xAB,0x89,0xD0,0x5E,0xB0,0x2A,0x6D,0x25,0xE6},
	 {0xB6,0xA1,0xE0,0xE0,0x32,0x10,0x1A,0xEA,0x94,0x1F,0x20,0x2F,0x10,0x04,0xEC,0x97,0x54,0x16,0xE5,0x83,0x1F,0xE5,0xCE,0xD6},
	 {0x43,0x38,0xB5,0x16,0x8C,0xF4,0x2F,0x9E,0xE6,0xC4,0x02,0xFB,0xF8,0x8A,0xF2,0x3B,0x5E,0x7A,0xD5,0x2F,0x83,0xCB,0x4F,0xF2},
	 {0x32,0x15,0x94,0x89,0xFE,0xFD,0x6E,0xB5,0x10,0xAD,0x38,0x92,0x1F,0xDC,0x62,0x3E,0x4F,0x89,0x54,0x0D,0x58,0x01,0x68,0x92},
	 {0x91,0xB0,0xEF,0xD3,0xDC,0xB9,0xCE,0x1C,0xB6,0x40,0xF8,0xEC,0x97,0xAB,0x7A,0xC7,0x5B,0x26,0x1A,0x5D,0xE9,0x8F,0x08,0x97},
	 {0x40,0xA7,0x0B,0x9D,0xAD,0x25,0x75,0x29,0xD6,0x1A,0x52,0x9D,0xB3,0xE0,0x2A,0x62,0x10,0xE0,0x92,0x26,0xE9,0x91,0x9B,0x45},
	 {0x86,0x31,0x8F,0x91,0x91,0x4F,0x07,0xCD,0xE0,0x8A,0x92,0x49,0x13,0x70,0x16,0x94,0xA8,0xDF,0x80,0x19,0xB3,0x3B,0xC8,0x83},
	 {0x8A,0x70,0x31,0x7F,0x6E,0xAB,0xC4,0xEC,0x0B,0x2C,0xBC,0xB5,0xD9,0xF8,0x43,0x43,0x07,0x86,0x0D,0x4A,0xCD,0xF7,0xFB,0xC1},
	 {0xB5,0x20,0x4F,0x54,0x58,0x0B,0x2C,0xC4,0x58,0xE0,0x4F,0xB0,0x68,0x5E,0x45,0x7C,0xC7,0x08,0x73,0x80,0xE3,0x8F,0xE6,0x5D},
	 {0x9E,0xBA,0x10,0x01,0x20,0x29,0x76,0x0E,0x54,0x8C,0xEF,0x19,0xBC,0xE3,0x94,0xB9,0xF2,0x08,0x38,0xBC,0xFB,0xDF,0x64,0xB0},
	 {0xB5,0x7F,0xCB,0xEA,0x85,0xF1,0x75,0x37,0x86,0x4F,0xD0,0xFD,0x2C,0x64,0xF8,0x85,0xFD,0x8A,0x34,0x2A,0x25,0xB3,0x29,0x3B},
	 {0x54,0x97,0xF1,0x2A,0x2C,0xA1,0xAB,0x16,0xE6,0x5D,0x97,0x49,0xCE,0x20,0x5B,0x29,0xDF,0x8F,0x4A,0x86,0x61,0x5E,0xA4,0x97},
	 {0x3E,0x70,0x92,0xAB,0x25,0x13,0x23,0x16,0x54,0xA1,0xA1,0x68,0x7F,0xF7,0x31,0x29,0x7A,0x1A,0xFB,0x75,0x89,0x0B,0xCB,0x58},
	 {0x76,0x40,0x25,0xF2,0x64,0x5B,0x9B,0x52,0x1F,0x10,0x54,0x07,0x2A,0x5D,0x76,0x08,0xD5,0x19,0x20,0xF4,0xDA,0xDA,0x26,0xE6},
	 {0xF7,0x5E,0xAD,0x46,0xD9,0x83,0xAD,0x3B,0x0B,0x0B,0x9E,0xD6,0xA7,0xA8,0xA8,0xEA,0x8C,0xD9,0xC2,0x58,0x52,0x6D,0x9B,0x0E},
	 {0x23,0x08,0x6D,0xC4,0x94,0x38,0xC1,0x04,0x85,0x58,0x57,0xD5,0xA4,0xCB,0x85,0xEF,0x45,0xAD,0x85,0xCB,0xA2,0x1A,0x3E,0x68},
	 {0xA1,0x58,0xA4,0xF8,0x32,0x37,0xA1,0xEC,0x9D,0x86,0xEC,0xA4,0xEC,0x9D,0x29,0x2F,0x37,0xFE,0xD5,0xEF,0x2A,0x1F,0x6E,0x57},
	 {0x32,0xB3,0x04,0x62,0xD6,0xF2,0xEF,0x02,0x57,0x20,0x43,0x73,0x61,0x07,0x37,0x6D,0x54,0x38,0xCE,0xE5,0x40,0xBA,0x19,0xF8},
	 {0x94,0xAE,0x43,0x10,0x20,0x2C,0xEC,0xA2,0x10,0x9E,0x62,0x94,0x79,0xAD,0x70,0x54,0x0B,0xA4,0x62,0x1A,0x5D,0xDF,0xE0,0xDC},
	 {0x04,0xF2,0xD3,0x6E,0x45,0xD9,0xDF,0x3E,0xC8,0x3E,0xE6,0x86,0x68,0xAE,0x5D,0x7C,0x1C,0x19,0xA2,0x34,0x75,0xD9,0xF2,0xFB},
	 {0xBC,0xD0,0x08,0x5B,0x25,0xE6,0x7C,0x5E,0x57,0x13,0x80,0x94,0x9D,0xB6,0x13,0x94,0xAD,0x9D,0xE0,0x51,0x04,0x70,0x83,0xB3},
	 {0x08,0xA7,0x0D,0xB9,0xD0,0xB5,0xD0,0x32,0xE9,0x3B,0x34,0x31,0x31,0xC7,0x37,0xBF,0x02,0x37,0x23,0x85,0x19,0xB3,0x83,0x3D},
	 {0x4C,0xD9,0xE3,0xAB,0xD9,0xDA,0x9B,0x85,0x13,0x8F,0x9B,0x02,0xF7,0x15,0x80,0x29,0x45,0x64,0xD9,0x32,0x9D,0x85,0x04,0x9B},
	 {0x5B,0x52,0xEA,0xB3,0x49,0x15,0x32,0x92,0xF2,0xD6,0x8C,0x16,0xB0,0x2F,0x85,0x67,0xAE,0x98,0xA2,0xA2,0x40,0x25,0x31,0x94},
	 {0x15,0x15,0xC4,0xF2,0x97,0x6B,0x79,0x9B,0xB9,0xBC,0x4C,0x94,0xF2,0x07,0x49,0x6D,0x15,0xB0,0xB3,0x5D,0x26,0xD0,0x83,0x73},
	 {0xAE,0x2C,0x8F,0x1F,0x9E,0xE0,0x1C,0x89,0xAE,0x07,0xA7,0x92,0xC7,0x43,0x61,0x5B,0xB0,0xE5,0x32,0xC1,0x79,0x16,0x37,0x62},
	 {0xDA,0xE9,0xB5,0x3B,0xD6,0x5E,0x26,0xE6,0x68,0x13,0xBF,0x43,0x91,0x80,0xC2,0xD0,0x70,0x98,0xCD,0x40,0x4A,0xB5,0x94,0x79},
	 {0x5E,0x15,0xE5,0x5E,0x62,0xC2,0x07,0x89,0x73,0xD6,0xB9,0x46,0x7A,0x68,0xB5,0x0E,0x57,0x34,0x20,0xAB,0xAB,0xE5,0x6B,0xFB},
	 {0x75,0x3E,0x0D,0xB9,0x43,0xF2,0xBF,0xD3,0x6D,0xFB,0x08,0xD5,0xF7,0xD3,0x68,0x5E,0xCD,0xE6,0xCD,0x08,0xAB,0x1A,0x0B,0xEF},
	 {0x2F,0x54,0x61,0x80,0xAD,0x15,0x57,0xC8,0xB3,0x51,0x9B,0x19,0xEC,0x49,0x7C,0xAD,0x6B,0xB6,0x51,0xDA,0xE0,0xD0,0xEA,0x0D},
	 {0x3D,0x49,0x02,0x40,0xDF,0xD3,0x76,0xD6,0x25,0xEC,0x23,0xDC,0x07,0x1C,0xF2,0x10,0x76,0x45,0x80,0x3B,0xEF,0x23,0x4A,0xE3},
	 {0xA4,0x6E,0xDF,0x0D,0x7C,0xA2,0xA4,0x98,0xE3,0x3D,0x38,0x62,0x2A,0xE9,0xFE,0x4A,0xB5,0xAB,0xD6,0x31,0xF7,0x5E,0xE0,0xEF},
	 {0x9B,0x25,0x3B,0xB6,0x92,0xB0,0x83,0x0B,0xA4,0xCE,0x6D,0x08,0x40,0xFB,0xCD,0x80,0x73,0xA2,0xC1,0xBC,0xC7,0xB5,0x1C,0x23},
	 {0xDF,0x92,0xC7,0x6B,0x38,0x01,0x2F,0xC2,0xA4,0xE0,0x31,0xDF,0xEA,0xAD,0xCE,0x9D,0x51,0x26,0x6E,0x6B,0x89,0x2C,0xC2,0x7F},
	 {0xAE,0xFB,0xE0,0x32,0xA2,0x6E,0x45,0x15,0x1C,0x19,0x73,0x5D,0x08,0xFE,0xB3,0xB0,0xCB,0xDF,0x9E,0x4A,0x15,0xDA,0x07,0x0B},
	 {0xE5,0xC8,0x2A,0xCE,0x67,0x62,0x08,0x79,0xA7,0xC2,0x49,0xD3,0x7F,0x32,0xD5,0x83,0x80,0xAE,0xEC,0x43,0xD6,0x32,0x91,0xDA},
	 {0x79,0xDF,0xA1,0x26,0x08,0x61,0x29,0x94,0x32,0x1C,0x61,0xD9,0x3E,0x23,0xDF,0xC8,0x08,0xC4,0x4C,0xF2,0x70,0xA1,0xF2,0x0B},
	 {0x89,0x15,0x61,0x13,0x9B,0x23,0x7C,0x58,0xB3,0x19,0x51,0xC8,0x2C,0x8C,0x94,0x68,0x4A,0x89,0xF2,0xD3,0x04,0x7F,0x0D,0x0D},
	 {0x5B,0x9B,0xC7,0x25,0x2A,0x2C,0x1A,0xF4,0x5B,0xA4,0x76,0x4A,0x34,0xF4,0x5B,0x43,0xCD,0x97,0x23,0xBF,0x01,0x13,0xDA,0x62},
	 {0xEA,0x51,0x51,0xC4,0xFE,0x85,0x29,0x3D,0x1C,0x10,0x83,0x75,0x98,0x9E,0xEC,0xB9,0x8C,0x67,0x94,0x89,0x15,0x2F,0x49,0x49},
	 {0x1A,0x3B,0x94,0x86,0xD9,0x73,0x6E,0xF7,0x8F,0xB3,0xE0,0xC8,0xF1,0x62,0x85,0x4F,0x8A,0xEF,0x89,0xB3,0x01,0x31,0x45,0xA2},
	 {0x07,0x7A,0xA8,0x13,0x0D,0x92,0xD6,0x4A,0x89,0xA4,0x8C,0x79,0xE9,0x02,0xE5,0x34,0x7C,0xC8,0xD0,0x2A,0x02,0xFB,0x92,0x29},
	 {0xAE,0xF4,0xD3,0x5E,0xA4,0x5D,0xB5,0x31,0x86,0x61,0xB0,0xCE,0x1C,0x1A,0xD9,0x43,0x4F,0xDA,0x5E,0x61,0x02,0xAB,0x1C,0x8A},
	 {0x51,0x62,0x58,0xF4,0xC2,0x04,0x38,0xE6,0x6D,0x68,0xC8,0x92,0x9B,0xC8,0xA7,0xFB,0xD0,0xDF,0xC1,0x4F,0xCB,0x2C,0xF2,0xAB},
	 {0x6E,0x2C,0xDA,0x5E,0x5D,0x38,0xF1,0x2C,0xA7,0x54,0x98,0x02,0x85,0x86,0x58,0x91,0x32,0x13,0x62,0x45,0xF8,0x0E,0x4A,0x46},
	 {0xBF,0x5D,0x54,0x1F,0x83,0x86,0x62,0xC1,0x37,0x83,0xBF,0x57,0x32,0xEC,0xD5,0x31,0x07,0x1C,0x37,0x83,0x29,0x23,0x32,0x98},
	 {0x79,0x91,0xA1,0x79,0xAE,0x61,0x64,0xB9,0xD9,0x19,0x10,0x3E,0xB0,0xE6,0x3B,0x58,0x51,0x86,0xB9,0xB3,0xDA,0x01,0x31,0x15},
	 {0xD0,0x54,0x51,0x70,0x4A,0x6B,0x54,0x8A,0x9D,0x2F,0x2F,0x54,0xD0,0x3D,0x73,0x8F,0x9E,0x10,0x7F,0xCE,0x2F,0x80,0xBA,0x2A},
	 {0xEC,0xB9,0xB6,0xFE,0x52,0x08,0x26,0xEA,0x0E,0x86,0xF7,0xBC,0xB3,0x13,0x5D,0x83,0xA8,0xEF,0x70,0x31,0x68,0xBA,0x40,0xD6},
	 {0xFE,0x13,0x20,0xE5,0xBC,0x64,0x98,0x2F,0xB6,0xAD,0x51,0x16,0x15,0x7C,0x3B,0xB0,0xD3,0x2A,0xA7,0xDA,0x25,0x43,0x57,0xD6},
	 {0x23,0xC4,0x02,0x9D,0xA2,0x62,0x29,0x85,0xD9,0xF1,0x01,0xD9,0x97,0xDC,0xC2,0x15,0x3D,0x62,0x9D,0x86,0x3D,0x15,0x4A,0x7F},
	 {0x91,0xBA,0x31,0x85,0xC8,0xEF,0x61,0xE3,0x8F,0x3E,0x91,0x01,0xF4,0x08,0x2F,0x51,0xC7,0x4A,0x64,0xA2,0xF7,0x1A,0x97,0xB3},
	 {0x91,0xC8,0x08,0x1A,0x76,0xCE,0x40,0xEC,0x40,0xE3,0xFD,0x29,0x15,0x1A,0x0E,0xD5,0x98,0x85,0x0E,0x45,0x8A,0x49,0xB6,0xA8},
	 {0x31,0xBA,0x7A,0x7A,0x3E,0x51,0x5E,0x6D,0x7A,0x92,0xEC,0x51,0xEC,0xD9,0x94,0x62,0x9E,0x4F,0x1F,0xFE,0x29,0x20,0x1A,0x0D},
	 {0xC1,0x52,0x5D,0x75,0x4C,0xDF,0x7C,0xB0,0x7A,0x46,0x43,0xD0,0x16,0x54,0xE6,0x3D,0x5B,0x0E,0xD0,0xAB,0x61,0x23,0x9E,0x0E},
	 {0x80,0x20,0xBA,0x6B,0x1C,0x75,0x67,0x13,0x57,0xD6,0x57,0x07,0x0E,0x13,0xE3,0x75,0xAB,0x70,0x51,0xC2,0xAB,0x02,0x94,0xC4},
	 {0x10,0x3D,0xB6,0x1A,0x75,0x6D,0x19,0x2C,0x04,0x13,0xE3,0xF4,0x70,0xF7,0x0D,0xDC,0x58,0xEF,0x49,0xCE,0x5E,0x49,0x92,0xEC},
	 {0x54,0x10,0x23,0x7C,0x34,0x32,0xAB,0x52,0x4C,0x80,0x0E,0x0E,0xA8,0x8C,0x01,0x64,0xBF,0xD3,0x6D,0xC1,0x5D,0x61,0xB6,0xC7},
	 {0x07,0xBC,0x57,0xCE,0x3D,0xC7,0xB6,0x29,0xD9,0x2F,0xCB,0x8C,0xD5,0x75,0x7C,0xA1,0xB5,0xCB,0x40,0x38,0xD9,0x4F,0x6E,0x01},
	 {0x5B,0x2F,0x13,0x70,0xB9,0xA8,0x2C,0x3E,0x3D,0xBA,0x4A,0xAD,0x13,0x51,0x20,0x98,0x15,0xEA,0xE3,0x91,0x3D,0x75,0xAB,0x67},
	 {0x4C,0x57,0xB6,0x3D,0x9D,0xBC,0x3D,0x8A,0x5B,0xB5,0x76,0x19,0x52,0x02,0x89,0xAB,0x13,0xB3,0x58,0xD9,0xA2,0xA4,0x2C,0xA4},
	 {0xE5,0xE3,0x32,0xC4,0x08,0x51,0x8A,0xEC,0x3E,0x52,0xEC,0x45,0x54,0x01,0xFD,0x76,0xBC,0xAE,0xF7,0xD3,0x8F,0x0E,0x57,0x85},
	 {0x37,0x9B,0xB9,0x61,0x40,0x23,0xD6,0xE0,0x19,0x38,0xFB,0x73,0x23,0xD9,0x1A,0x80,0x61,0xA4,0xA2,0xC7,0x08,0xB0,0xAE,0x25},
	 {0x5B,0x0D,0x19,0xB3,0xB0,0xD5,0x4A,0x85,0xCB,0x97,0x51,0xE0,0xCB,0x13,0xBC,0x92,0x9E,0x04,0x62,0x61,0x29,0xE6,0x34,0x4A},
	 {0x4C,0x1C,0xE9,0x02,0x9D,0x1A,0x94,0xA8,0xCB,0x92,0x73,0x9E,0xEC,0x3E,0x86,0x0D,0x0E,0x40,0xFD,0x83,0x75,0x76,0x86,0x29},
	 {0x19,0x46,0x75,0x6E,0x1F,0x32,0xD3,0x4C,0xDC,0x85,0x8F,0x57,0x34,0xAB,0x57,0xDF,0x1F,0xE5,0xE0,0x13,0x54,0xE9,0xD9,0xA4},
	 {0xD3,0x8F,0x62,0x49,0xD9,0x51,0xD5,0x38,0x91,0x1F,0x23,0xEA,0xA7,0xDF,0xDF,0x8A,0x02,0x67,0x92,0xE9,0x34,0x3E,0x23,0x75},
	 {0x23,0xEA,0x76,0xF4,0xA2,0x52,0xF7,0x19,0xC2,0x7A,0xBA,0xFD,0x29,0x15,0xEF,0xEF,0xBA,0xE5,0x4F,0xB0,0xB3,0x37,0xD0,0xB0},
	 {0x61,0x52,0xAD,0x98,0x38,0x6D,0x15,0x3E,0x9B,0xD0,0x62,0x58,0xDF,0x1C,0x0B,0xD6,0x91,0x2C,0xB3,0x4F,0x79,0xC4,0xB6,0x6E},
	 {0x8F,0x2A,0xBA,0x83,0x51,0xA7,0xD5,0xD9,0x6B,0x2C,0xAD,0xFE,0xC7,0x97,0x0E,0xFD,0x49,0xC8,0x64,0x10,0x29,0x92,0x64,0xEC},
	 {0xA1,0x92,0x54,0xAE,0x62,0xF7,0x73,0x40,0xCB,0xEC,0x97,0x1F,0x1F,0x8C,0x5E,0xC7,0x94,0x8F,0xEC,0x5B,0x97,0x0E,0x45,0xB3},
	 {0x98,0x19,0xDF,0xB0,0x86,0x89,0x58,0x61,0x4F,0x68,0x51,0xC7,0x54,0x64,0x5E,0x15,0xAE,0xAE,0x75,0x73,0xFD,0x32,0x1F,0x67},
	 {0xDA,0xCE,0x0B,0x23,0x98,0xD5,0x1C,0xE9,0x4F,0xBF,0xFB,0x16,0x89,0xE0,0x57,0x5E,0x25,0x70,0x2C,0x1A,0xDF,0xF7,0xF7,0xFE},
	 {0xF2,0xA8,0x70,0xEA,0x62,0x23,0x51,0x10,0xB6,0x20,0x4C,0xD0,0x1A,0x7C,0x0D,0xDF,0x97,0xD3,0x4C,0x23,0xA7,0xC7,0x40,0x3B},
	 {0x8F,0xCD,0x79,0x49,0x25,0xB6,0x98,0xDC,0x79,0x7A,0xF4,0x3D,0xAE,0xCB,0x9B,0xCB,0xC4,0xEC,0xD3,0xD5,0x0B,0x9D,0x23,0xE0},
	 {0x23,0x40,0x85,0x1C,0xA1,0x70,0x58,0x98,0xA8,0x2F,0x5E,0x76,0xF1,0x9B,0x46,0x20,0x73,0x68,0xAD,0x4C,0x46,0x32,0x73,0x79},
	 {0x1F,0x4A,0x98,0xCD,0x26,0xC2,0x1C,0x1F,0x32,0x9E,0xA7,0xE9,0xC4,0x3D,0x38,0x43,0x0B,0x23,0xA7,0xAE,0xC1,0x15,0x3D,0xC4},
	 {0xE6,0xBF,0x8C,0x61,0xFE,0x25,0xE6,0x75,0x26,0xB3,0xF4,0xAB,0xCB,0xBF,0x31,0x34,0x89,0xE9,0xA2,0x98,0x64,0x70,0x54,0xEF},
	 {0x04,0x08,0x31,0x83,0x02,0xE5,0x34,0x29,0x31,0x51,0x8A,0xFD,0xF8,0x23,0xD6,0xF8,0x01,0x91,0xBA,0x4C,0x83,0x9E,0xA8,0x29},
	 {0x29,0xFE,0xA7,0x6D,0xAB,0x1A,0xB6,0xFD,0x91,0x4A,0x3D,0xA8,0x75,0x6E,0xE0,0xC1,0x20,0x0B,0x29,0x76,0x64,0x75,0xC7,0x1F},
 };

#define     ENCRYPT             0//1
#define     DECRYPT             1//0

 int  Tdes(uchar * pszIn,uchar * pszOut,int iDataLen,uchar * pszKey,int iKeyLen,uchar ucMode)
 {	 
	 int iLoop;
 
	 if(24==iKeyLen)
	 {
 
		 if(ucMode==ENCRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_desecb(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,ENCRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,DECRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+16,ENCRYPT);
				 DesECB(ENCRYPT, pszKey, pszIn+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(DECRYPT, pszKey+8, pszOut+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(ENCRYPT, pszKey+16, pszOut+iLoop*8, 8, pszOut+iLoop*8);
			 }
		 }
		 else if(ucMode==DECRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_desecb(pszIn+iLoop*8,pszOut+iLoop*8,pszKey+16,DECRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,ENCRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,DECRYPT);
				 DesECB(DECRYPT, pszKey+16, pszIn+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(ENCRYPT, pszKey+8, pszOut+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(DECRYPT, pszKey, pszOut+iLoop*8, 8, pszOut+iLoop*8);
			 }
 
		 }
	 }
	 else if (16==iKeyLen)
	 {
		 if(ucMode==ENCRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_desecb(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,ENCRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,DECRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,ENCRYPT);
				 DesECB(ENCRYPT, pszKey, pszIn+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(DECRYPT, pszKey+8, pszOut+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(ENCRYPT, pszKey, pszOut+iLoop*8, 8, pszOut+iLoop*8);
			 }
		 }
		 else if(ucMode==DECRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_desecb(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,DECRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,ENCRYPT);
				 //api_desecb(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,DECRYPT);
				 DesECB(DECRYPT, pszKey, pszIn+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(ENCRYPT, pszKey+8, pszOut+iLoop*8, 8, pszOut+iLoop*8);
				 DesECB(DECRYPT, pszKey, pszOut+iLoop*8, 8, pszOut+iLoop*8);
			 }
 
		 }
		 else 
		 {
			 return -2;
		 }
	 }
	 else if(8==iKeyLen)
	 {	 
		 if(ucMode==ENCRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_desecb(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,ENCRYPT);
				 DesECB(ENCRYPT, pszKey, pszIn+iLoop*8, 8, pszOut+iLoop*8);
			 }
		 }
		 else if(ucMode==DECRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_desecb(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,DECRYPT);
				 DesECB(DECRYPT, pszKey, pszIn+iLoop*8, 8, pszOut+iLoop*8);
			 }
		 }
		 else 
		 {
			 return -2;
		 }
	 }
	 else  
	 {
		 return -3;
	 }
	 for(iLoop=0;iLoop<iDataLen%8;iLoop++)
	 {
		 pszOut[(iDataLen/8)*8+iLoop]=pszIn[(iDataLen/8)*8+iLoop]^0xff;
	 }
	 return 0;
 }

 static unsigned char abcd_to_asc(unsigned char ucBcd)
{
	unsigned char ucAsc = 0;
	
	ucBcd &= 0x0f;
	if (ucBcd <= 9)
		ucAsc = ucBcd + '0';
	else
		ucAsc = ucBcd + 'A' - 10;
	return (ucAsc);
}
 
 void BcdToAsc(char * sAscBuf, unsigned char * sBcdBuf, int iAscLen)
{
	int i, j;j = 0;

	if((sBcdBuf == NULL) || (sAscBuf == NULL) || (iAscLen < 0))
		return;
	
	for (i = 0; i < iAscLen / 2; i++) 
	{
		sAscBuf[j] = (sBcdBuf[i] & 0xf0) >> 4;
		sAscBuf[j] = abcd_to_asc(sAscBuf[j]);
		j++;
		sAscBuf[j] = sBcdBuf[i] & 0x0f;
		sAscBuf[j] = abcd_to_asc(sAscBuf[j]);
		j++;
	}
	if (iAscLen % 2) 
	{
		sAscBuf[j] = (sBcdBuf[i] & 0xf0) >> 4;
		sAscBuf[j] = abcd_to_asc(sAscBuf[j]);
	}
}
 void MAC_Arithmetic(uchar Mode,uchar *DataIn, ushort inlen,uchar *MacKey, uchar MacKeyLen, uchar *VerifyOut)
 {
	 unsigned char tempdata[2100];
	 int i,j;
	 unsigned char tempout[100];
	 unsigned char ascout[200];
	 memset(tempdata,0x00,sizeof(tempdata));
	 memcpy(tempdata,DataIn,inlen);
	 memset(tempout,0x00,sizeof(tempout));
	 memset(ascout,0x00,sizeof(ascout));
	 if((Mode == 0x40)||(Mode == 0x41)||(Mode == 0x42))
	 {
	 }
	 else
	 {
		 if(inlen%8) inlen=inlen+8;
	 }
 
	 switch(Mode)
	 {
		 case 0x00:
					 
 
			 for(i=0; i<inlen/8; i++)
			 {
				 for(j=0; j<8; j++)
				 {
					 VerifyOut[j]^=tempdata[i*8+j];
				 }
				 Tdes(VerifyOut,VerifyOut,8,MacKey,MacKeyLen,ENCRYPT);
			 }
			 break;
		 case 0x01:
			 for(i=0; i<inlen/8; i++)
			 {
				 for(j=0; j<8; j++)
				 {
					 VerifyOut[j]^=tempdata[i*8+j];
				 }
			 }
			 Tdes(VerifyOut,VerifyOut,8,MacKey,MacKeyLen,ENCRYPT);
			 break;
		 case 0x02:
			 for(i=0; i<inlen/8; i++)
			 {
				 for(j=0; j<8; j++) 
				 {
					 VerifyOut[j]^=tempdata[i*8+j];
				 }
				 if (i != inlen/8-1)
				 {
					 //api_desecb(VerifyOut,VerifyOut,MacKey,ENCRYPT);
					 DesECB(ENCRYPT, MacKey, VerifyOut, 8,VerifyOut);
				 }
				 else
				 {
					 Tdes(VerifyOut,VerifyOut,8,MacKey,MacKeyLen,ENCRYPT);
				 }
			 }
			 break;
		 case 0x03:
			 for(i=0; i<inlen/8; i++)
			 {	 
				 for(j=0; j<8; j++)
				 {
					 tempout[j]^=tempdata[i*8+j];
				 }
			 }
			 
			 BcdToAsc(ascout, tempout,16);
			 memcpy(VerifyOut,ascout,8);
			 //api_desecb(VerifyOut,VerifyOut,MacKey,ENCRYPT);
			 DesECB(ENCRYPT, MacKey, VerifyOut, 8,VerifyOut);
			 for(i = 0; i < 8;i++)
			 {
				 VerifyOut[i] ^= ascout[8+i];
			 }
			 //api_desecb(VerifyOut,VerifyOut,MacKey,ENCRYPT);
			 DesECB(ENCRYPT, MacKey, VerifyOut, 8,VerifyOut);
			 BcdToAsc(ascout, VerifyOut,16);
			 memcpy(VerifyOut,ascout,8);
			 sysDelayMs(10000);
			 break;
		 //case 0x40:
		 //memcpy(VerifyOut,DataIn,inlen);
		 //SM4_Mac1( MacKey,VerifyOut,inlen);
		 //break;
		 //case 0x41:
		 //memcpy(VerifyOut,DataIn,inlen);
		 //SM4_Mac2( MacKey,VerifyOut,inlen);
		 //break;
		 //case 0x42:
		 //memcpy(VerifyOut,DataIn,inlen);
		 //SM4_Mac3( MacKey,VerifyOut,inlen);
		 //break;
		 default: break;
	 }
  }

 int KcvVerify(uchar *PwdKeyValueIn,ushort PwdKeyLen ,uchar * KeyValueIn,
                  ushort KeyLen,uchar * KcvDataOut,uchar Mode,uchar MacMode) 
{
	uchar Dataout[9],Mode3Data[100],TempBuf[200];
	ushort Mode3DataLen,Len = 0;
	
	memset(Dataout,    0 , sizeof(Dataout));
	memset(Mode3Data, 0 , sizeof(Mode3Data));
		
	switch(Mode)
	{
		case 0x00: break;
		case 0x01:
			//int  Tdes(uchar * pszIn,uchar * pszOut,int iDataLen,uchar * pszKey,int iKeyLen,uchar ucMode)
			Tdes("\x00\x00\x00\x00\x00\x00\x00\x00", Dataout+1,8,KeyValueIn,KeyLen,ENCRYPT);
			Dataout[0] = 4;    
			memcpy(KcvDataOut, Dataout, 9);
			break;
		case 0x02:
			Tdes("\x12\x34\x56\x78\x90\x12\x34\x56", Dataout+1,8,KeyValueIn,KeyLen,ENCRYPT);
			Dataout[0] = 4;
			memcpy(KcvDataOut, Dataout, 9);
			break;	
		case 0x03:
			memset(Mode3Data, 0xee, sizeof(Mode3Data));
			Mode3DataLen=sizeof(Mode3Data)-PwdKeyLen;
			memcpy(Mode3Data, PwdKeyValueIn, PwdKeyLen);
			MAC_Arithmetic(MacMode,Mode3Data, sizeof(Mode3Data), KeyValueIn, KeyLen, Dataout);
			TempBuf[0] = Mode3DataLen;   //KCVData长度
			memcpy(TempBuf+1, Mode3Data+PwdKeyLen, Mode3DataLen);
			TempBuf[0+1+Mode3DataLen] = MacMode;  //计算Mac的模式
			TempBuf[0+2+Mode3DataLen] = 8;
			memcpy(TempBuf+3+Mode3DataLen, Dataout, 8);
			memcpy(KcvDataOut, TempBuf, Mode3DataLen+8+3);
			Len = Mode3DataLen+8+3;
			break;
	}
	return Len;
}
 int PedWriteKeyExCipherTest(char *Tilte,uchar sKeyTp,uchar sKeyIdx,uchar DKeyTp,uchar DKeyIdx,int iDKeyLen,uchar WriteMode, int CheckMode, int MacMode,int PreRet,uchar PortSendsFlag)
 {
	 int verLen = 0,Ret=255,ErrFlag = 1,EncryptFlag = 255,ValueLen = 0,Len = 0;
	 uchar KcvData[200],PwdKey[50],Value[50];
	 ST_KEY_INFO_EX  KeyInEx;
	 ST_KCV_INFO KcvIn;
	 uchar vector[32];
	 char Buf[500];
	 memset(Buf, 0x00,sizeof(Buf));
	 
	 memset(&KeyInEx,	 0x00,sizeof(KeyInEx));
	 memset(KcvData,	 0x00,sizeof(KcvData));
	 memset(&KcvIn,  0x00,sizeof(KcvIn));
	 memset(PwdKey,  0x00,sizeof(PwdKey));
	 memset(Value,	 0x00,sizeof(Value));
	 memset(vector,  0x00,sizeof(vector));
 
	 //api_pedErase();
 
	 if(DKeyTp == PED_TLK) KeyInEx.ucDstKeyType = PED_TLK;
	 else KeyInEx.ucDstKeyType = sKeyTp;
 
	 KeyInEx.ucSrcKeyIdx = 0;
	 
	 KeyInEx.ucDstKeyIdx = DKeyIdx;
	 KeyInEx.iDstKeyLen = iDKeyLen;
	 memcpy(KeyInEx.aucDstKeyValue,key_Data[10],KeyInEx.iDstKeyLen);
	 KeyInEx.ucWriteMode = 0;
	 KcvIn.iCheckMode = CheckMode;
	 verLen = KcvVerify("", KeyInEx.iDstKeyLen, KeyInEx.aucDstKeyValue, KeyInEx.iDstKeyLen,KcvData,KcvIn.iCheckMode,MacMode);
	 memcpy(KcvIn.aucCheckBuf,KcvData,9);
	 
	 Ret = pedWriteKeyEx_lib(&KeyInEx, &KcvIn);
	 if(Ret !=PED_RET_OK)
	 {
	     sysLOG(API_LOG_LEVEL_2, "  pedWriteKeyEx_lib faild\r\n");
		 return ErrFlag;
	 }
 
	 memset(&KeyInEx,	 0x00,sizeof(KeyInEx));
	 memset(KcvData,	 0x00,sizeof(KcvData));
	 memset(&KcvIn,  0x00,sizeof(KcvIn));
	 Ret = 255;
 
	 KeyInEx.ucSrcKeyIdx = DKeyIdx;
	 KeyInEx.ucSrcKeyType = sKeyTp;
	 
	 KeyInEx.ucDstKeyIdx = DKeyIdx;
	 KeyInEx.ucDstKeyType = DKeyTp;
	 KeyInEx.iDstKeyLen = iDKeyLen;
	 
	 KeyInEx.ucWriteMode = WriteMode;
 
	 KcvIn.iCheckMode = CheckMode;
 
	 if((WriteMode == 0x81)||(WriteMode == 0xC1))EncryptFlag = DECRYPT;
	 else if((WriteMode == 0x01)||(WriteMode == 0x41))EncryptFlag = ENCRYPT; 
	 
	 if((WriteMode == 0x81)||(WriteMode == 0x01))
	 {
		 Tdes(key_Data[11], PwdKey,  iDKeyLen, key_Data[10], iDKeyLen, EncryptFlag); 
	 }
	 else if((WriteMode == 0xC1)||(WriteMode == 0x41))
	 {   //int gmSm4(unsigned char *input, unsigned int  input_len, unsigned char *output, unsigned char *smkey, unsigned char *vector, unsigned char mode);
		 //api_gmSm4(key_Data[11],iDKeyLen,PwdKey,key_Data[10],vector,EncryptFlag);
	     gmSm4(key_Data[11],iDKeyLen,PwdKey,key_Data[10],vector,EncryptFlag);
	 }
	 
	 if((CheckMode==0x01)||(CheckMode==0x02)) {memcpy (Value, key_Data[11], iDKeyLen);ValueLen = iDKeyLen;}
	 else if(CheckMode==0x03) {memcpy (Value, key_Data[10], iDKeyLen);ValueLen = iDKeyLen;}
 
	 verLen = KcvVerify(PwdKey, iDKeyLen, Value, ValueLen,KcvData,CheckMode,MacMode);
	 
	 if((CheckMode==0x01)||(CheckMode==0x02)) Len = 9;
	 else if(CheckMode==0x03) Len = verLen;
	 memcpy(KcvIn.aucCheckBuf, KcvData, Len);
	 
	 memcpy(KeyInEx.aucDstKeyValue,PwdKey,KeyInEx.iDstKeyLen);
	 Ret = pedWriteKeyEx_lib(&KeyInEx, &KcvIn);
	 
	 if(Ret == PreRet)
	 {
		 ErrFlag = 0;
		 PortSendsFlag = 0;
	 }
	 else
	 {
		 ErrFlag = 1;
	 }
	 return ErrFlag;
 }

 int WriteKeyExCipherTMPADK(int Len,int Mode,int MacMode,int PreRet)
 {
   char WriteMode[4] ={0x01,0x81,0x41,0xC1};
	 char *WriteModeMess[4] ={"DES解密写入","DES加密写入","SM4解密写入","SM4加密写入"};
	 const char TempBuf[4] ={PED_TMK,PED_TPK,PED_TAK,PED_TDK,};
	 const char *TitleBuf[4] ={"写TMK","写TPK","写TAK","写TDK",};
	 uchar Ret[4] = {0xff,0xff,0xff,0xff}, WriteModeRet[4] = {0,0,0,0};
	 int i = 0,j = 0,k = 0,ErrFlag = 0,ORet;
	 uchar Buf[500];
 
	 memset(Buf, 0x00 , sizeof(Buf));
	 //for(k = 0;k < 4;k++)
	 {
		 //for(j = 0;j < 4;j++)
		 {
	 
			 //for(i = 1;i <= MAX_PWK_INDEX;i++)
			 for(i = 1;i <= 1;i++)
			 {
				 //Ret[j] = PedWriteKeyExCipherTest(TitleBuf[j],PED_TMK,1,TempBuf[j],i,Len,WriteMode[k] , Mode, MacMode, PreRet,0);
				 Ret[j] = PedWriteKeyExCipherTest(TitleBuf[j],0x60,1,TempBuf[j],i,Len,WriteMode[k] , Mode, MacMode, PreRet,0);
				 if(Ret[j] != 0)
				 {			 
					 sprintf(Buf," 写模式:%02x %s,密钥类型:%s,索引号:%d",WriteMode[k],WriteModeMess[k],TitleBuf[j],i);
					 //api_portSends(11, Buf,strlen(Buf));
					 //api_portSends(11, "\r\n",strlen("\r\n"));
					 WriteModeRet[k] = 1 ;
					 break;
				 }
				 sysDelayMs(1000);
			 }
		 }
	 }
	 return ErrFlag;
 }
 

 int PedCalcDESTest(ushort TDKLen,uchar KeyIdx,ushort DataInLen, uchar Mode,int PreRet)  
 {
	 uchar Key[]={0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79,0x89,0x9b,0xab,0xbc,0xcd,0xdf,0xef,0xe0,0x01,0x10,0x23,0x34,0x45,0x57,0x67,0x79};
	 uchar DataIn[1032];
	 uchar DataTempOut[1032], DataOut[1032];
	 ST_KEY_INFO  st_key_info;
	 ST_KCV_INFO  st_kcv_info;
	 int Ret,i,Idx = 1,ErrFlag = 1;
     unsigned char baRandom[100] = {0};
	 //api_pedErase(); //删除所有密钥
 
	 memset(&st_key_info, 0x00, sizeof(st_key_info));
	 memset(&st_kcv_info, 0x00, sizeof(st_kcv_info));
			 
	 st_key_info.ucSrcKeyIdx = 0x00;
	 st_key_info.ucDstKeyType = PED_TDK; 
	 
	 if(KeyIdx >100)Idx = 1;
	 else Idx = KeyIdx;
	 
	 st_key_info.ucDstKeyIdx = Idx ;
	 st_key_info.iDstKeyLen = TDKLen;		 
	 memcpy(st_key_info.aucDstKeyValue, Key, st_key_info.iDstKeyLen);
	 
	 if(pedWriteKey_lib(&st_key_info, &st_kcv_info)<0) 
	 {
		 //PEDMessage(api_pedWriteKey(&st_key_info, &st_kcv_info),"TDK");
		 sysLOG(API_LOG_LEVEL_2, "  pedWriteKey_lib faild\r\n");
		 return ErrFlag;
	 }
 
	 for(i=0; i<DataInLen/8+1; i++)
	 {
		 sysGetRandom_lib(8, DataIn+i*8);		 
	 }
	 
	 //Ret=pedCalcSym_lib(KeyIdx,DataIn,DataInLen, DataTempOut, Mode);
	 Ret=pedCalcSym_lib(KeyIdx,0,DataIn, DataTempOut, Mode);
	 //PEDMessage(Ret,"CalcDES");
	 if(Ret == PED_RET_OK)
	 {
		 Mode^=ENCRYPT;
		 
		 Ret=pedCalcSym_lib(KeyIdx, 0, DataTempOut,	DataOut, Mode);
 
		 if(!memcmp(DataIn, DataOut, DataInLen)) 
		 	//DispScrPrint(0, LINE3, G_FontSet, FDISP|CDISP, "校验成功");
		 	sysLOG(API_LOG_LEVEL_2, "  pedCalcSym_lib ok\r\n");
		 else 
		 {
			 Ret = 100;
			 //DispScrPrint(0, LINE3, G_FontSet, FDISP|CDISP, "校验失败");
			 sysLOG(API_LOG_LEVEL_2, "  pedCalcSym_lib faild\r\n");
		 }
	 }
 
	 if(Ret == PreRet)
		 ErrFlag = 0;
	 else
	 {
		 uchar errorbuf[10];
		 memset(errorbuf,0,sizeof(errorbuf));
		 sprintf(errorbuf,"Ret = %d",Ret);
		 //api_portSends(11, errorbuf,strlen(errorbuf));
	 }
	 return ErrFlag;
 }

 void StrXor(uchar *pucIn1,uchar *pucIn2,int iLen,uchar *pucOut)

{
	int iLoop;
	for(iLoop=0;iLoop<iLen;iLoop++)
	{
		pucOut[iLoop]=pucIn1[iLoop]^pucIn2[iLoop];
	}}
 int  tdes(uchar * pszIn,uchar * pszOut,int iDataLen,uchar * pszKey,int iKeyLen,uchar *IV, uchar ucMode)
 
 {	 
	 int iLoop;
	 
	 if (16==iKeyLen)
	 {
		 if(ucMode==ENCRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_descbc(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,IV,ENCRYPT);
		 calcDesEnc_lib(pszIn+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1); 
			  // api_descbc(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,IV,DECRYPT);
		calcDesDec_lib(pszOut+iLoop*8,8, pszOut+iLoop*8, pszKey+8, IV, 1); 
			 //api_descbc(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,IV,ENCRYPT);
	   calcDesEnc_lib(pszOut+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1); 
			 StrXor(pszOut+iLoop*8,pszIn+(iLoop+1)*8,8,pszIn+(iLoop+1)*8);
			 }
		 }
		 else if(ucMode==DECRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {			 
					 //api_descbc(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,IV,DECRYPT);	 
		   calcDesDec_lib(pszIn+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1);												   
					 //api_descbc(pszOut+iLoop*8,pszOut+iLoop*8,pszKey+8,IV,ENCRYPT);
		   calcDesEnc_lib(pszOut+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1); 																
					 //api_descbc(pszOut+iLoop*8,pszOut+iLoop*8,pszKey,IV,DECRYPT);
		   calcDesDec_lib(pszOut+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1); 	 
					 StrXor(pszOut+iLoop*8,pszIn+(iLoop-1)*8,8,pszOut+iLoop*8);
			 }
 
		 }
		 else 
		 {
			 return -2;
		 }
	 }
	 else if(8==iKeyLen)
	 {	 
		 if(ucMode==ENCRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_descbc(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,IV,ENCRYPT);
		 calcDesEnc_lib(pszIn+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1); 
				 StrXor(pszOut+iLoop*8,pszIn+(iLoop+1)*8,8,pszIn+(iLoop+1)*8);
			 }
		 }
		 else if(ucMode==DECRYPT)
		 {
			 for(iLoop=0;iLoop<iDataLen/8;iLoop++)
			 {
				 //api_descbc(pszIn+iLoop*8,pszOut+iLoop*8,pszKey,IV,DECRYPT);
		 calcDesDec_lib(pszIn+iLoop*8,8, pszOut+iLoop*8, pszKey, IV, 1); 
				 StrXor(pszOut+iLoop*8,pszIn+(iLoop-1)*8,8,pszOut+iLoop*8);
			 }
		 }
		 else 
		 {
			 return -2;
		 }
	 }
	 for(iLoop=0;iLoop<iDataLen%8;iLoop++)
	 {
		 pszOut[(iDataLen/8)*8+iLoop]=pszIn[(iDataLen/8)*8+iLoop]^0xff;
	 }
	 return 0;
 }

 
 //6-1  PedCalcDESDukptTest(Flag,8,1,8,0x00,DukptDeskey8a[1],PED_RET_OK);
 int PedCalcDESDukptTest(uchar KeyFlag, uchar KeyLen, uchar GroupIdx,ushort DataInLen, uchar Mode,uchar *KeyOut,int PreRet) 
 {
     sysLOG(API_LOG_LEVEL_2, " PedCalcDESDukptTest into\r\n");
		 int Ret,i,j = 0,ErrFlag = 1;
		 uchar KeyVarType = 0x01;
		 uchar DataIn[1032],DataInOut[1032]; 
		 uchar DataTempOut[1032], DataOut[1032],DataTempOut1[1032];
		 uchar KsnOut[20],KsnIn[10],KsnBuf[50];
		 uchar pucIV[9];
		 ST_KCV_INFO  KcvInfoIn;
		 uchar VerifyType = 255; //by CC
		 
		 memset(KsnOut, 	 0x00, sizeof(KsnOut));
		 memset(KsnBuf, 	 0x00, sizeof(KsnBuf));
		 memset(DataIn, 	 0x00, sizeof(DataIn));
		 memset(DataOut,		 0x00, sizeof(DataOut));
		 memset(pucIV,		 0x00, sizeof(pucIV));
		 memset(&KcvInfoIn, 	 0x00, sizeof(KcvInfoIn));
		 memcpy(pucIV,"\x00\x00\x00\x00\x00\x00\x00\x00",8);
		 
		 if(KeyFlag == 1)
		 {
			 //api_pedErase();	 
		 memcpy(KsnIn,"\xff\xff\x98\x76\x54\x32\x10\xe0\x00\x00",10);	 
			 Ret = pedWriteTiK_lib(GroupIdx, 0 ,KeyLen,key_Data[2],KsnIn,&KcvInfoIn);
			 if(Ret != PED_RET_OK){//PEDMessage(Ret,"WriteTIK");
		 return ErrFlag;}
		 }
		 for(i=0; i<DataInLen/8+1; i++)
		 {
			 sysGetRandom_lib(8,DataIn+i*8);	 
		 }
		 Ret = pedDukptDes_lib(GroupIdx,KeyVarType,pucIV,DataInLen,DataIn,DataOut,KsnOut,Mode);  
	   OSI_LOGI(0,"Case_12:ped DukptDes Return %d\r\n",Ret);
		 //PEDMessage(Ret,"CalcDesDukpt");
		 BcdToAsc(KsnBuf, KsnOut, 20);
		 //DispScrPrint(0, LINE5, 1, FDISP|CDISP, "%s",KsnBuf);
		 if (Ret == PED_RET_OK)
		 {
			 if(Mode == 0x00 || Mode == 0x02) VerifyType = ENCRYPT;
			 else VerifyType = DECRYPT;
			 if((Mode == 0x00 || Mode == 0x01))
			 {
				 Tdes(DataOut,DataTempOut,DataInLen, KeyOut,KeyLen,VerifyType);
				 //if(memcmp(DataIn,DataTempOut,DataInLen) == 0)
				 //  DispScrPrint(0, LINE3, G_FontSet, NOFDISP|CDISP, "У��ɹ�");
	 // 		 BcdToAsc(DataInOut, DataIn, 20);
	 // 		 BcdToAsc(DataTempOut1, DataTempOut, 20);
	 // 		 DispScrPrint(0, LINE3, 1, FDISP|CDISP, "%s:",DataInOut);
	 // 		 DispScrPrint(0, LINE4, 1, FDISP|CDISP, "%s:",DataTempOut1);}
		 // 	 else
				 //  DispScrPrint(0, LINE3, G_FontSet, FDISP|CDISP, "У��ʧ��");
	 // 			 BcdToAsc(DataInOut, DataIn, 20);
	 // 		 BcdToAsc(DataTempOut1, DataTempOut, 20);
	 // 		 DispScrPrint(0, LINE3, 1, FDISP|CDISP, "%s",DataInOut);
	 // 		 DispScrPrint(0, LINE4, 1, FDISP|CDISP, "%s",DataTempOut1);}
			 }
			 if((Mode == 0x02 || Mode == 0x03))
			 {
				 tdes(DataOut,DataTempOut,DataInLen, KeyOut,KeyLen,pucIV,VerifyType);
				 //if(memcmp(DataIn,DataTempOut,DataInLen) == 0)
					 //DispScrPrint(0, LINE3, G_FontSet, NOFDISP|CDISP, "У��ɹ�");
				 //else 
					 //DispScrPrint(0, LINE3, G_FontSet, FDISP|CDISP, "У��ʧ��");
			 }
		 }
		 if(Ret == PreRet)ErrFlag = 0;
		 else
		 {
			 uchar errorbuf[10];
			 memset(errorbuf,0,sizeof(errorbuf));
			 sprintf(errorbuf,"Ret = %d",Ret);
		 }
		return ErrFlag;


 }

 char *DukptDeskey8a[10] =
 {
	 "\x98\xCF\x68\xE9\x6A\xA4\x26\x25",
	 "\xD9\x70\x84\xF0\xDA\x56\x8A\x5F",
	 "\x8F\x16\xEA\x99\x1B\x8B\x51\x78",
	 "\x5C\x6D\x72\xF2\x72\xCC\x11\xF0",
	 "\x68\x32\xF5\x70\x14\xAC\xB7\xF7",
	 "\x2D\xF2\xA5\x26\x7C\xE0\x8E\xBA",
	 "\x0E\x8C\xB8\xD6\x9B\x8A\xF4\x4D",
	 "\xB1\x21\x45\x19\xE9\x86\x40\xC6",
	 "\xC6\xAC\x35\xD9\x65\xD9\x28\x5F",
	 "\x78\x50\xa9\x40\x14\xD8\x61\xb1",
 };

void pedTest()
{
    int iRet = 0;
	uchar Flag = 1;
	char *pcKey8 = "\x98\xCF\x68\xE9\x6A\xA4\x26\x25";
	uchar baDesKey[12] = {0};
	uchar KsnIn[10];
	ST_KCV_INFO  KcvInfoIn;
	 
	memcpy(baDesKey, pcKey8, 8);
	memset(&KcvInfoIn, 	 0x00, sizeof(KcvInfoIn));
	memcpy(KsnIn,"\xff\xff\x98\x76\x54\x32\x10\xe0\x00\x00",10);	
	sysLOG(API_LOG_LEVEL_2, " PedCalcDESDukptTest begin\r\n");
	//iRet = pedWriteTiK_lib(Flag, 0 ,8,key_Data[2],KsnIn,&KcvInfoIn);
	iRet = PedCalcDESDukptTest(Flag,8,1,8,0x00,DukptDeskey8a[1],PED_RET_OK);
	iRet = PedCalcDESDukptTest(0,8,1,512,0x00,DukptDeskey8a[0],PED_RET_OK);
	//int PedCalcDESDukptTest(uchar KeyFlag, uchar KeyLen, uchar GroupIdx,ushort DataInLen, uchar Mode,uchar *KeyOut,int PreRet) 
	//iRet = PedCalcDESDukptTest(Flag,8,1,8,0x00, baDesKey,PED_RET_OK);
	return;
	//pedWriteKey_lib;
	//myPedPlaintextTest("write TLK0",0,0,PED_TLK, 0x01,16,0x00);
	/*PedPlaintextTest("write TLK1",0,0,PED_TLK, 0x01,16,0x01);
	PedPlaintextTest("write TLK2",0,0,PED_TLK, 0x01,16,0x02);
	PedPlaintextTest("write TLK3",0,0,PED_TLK, 0x01,16,0x03);
	PedPlaintextTest("write TLK0 24",0,0,PED_TLK, 0x01,24,0x00);
	PedPlaintextTest("write TLK1 24",0,0,PED_TLK, 0x01,24,0x01);
	PedPlaintextTest("write TLK2 24",0,0,PED_TLK, 0x01,24,0x02);
	PedPlaintextTest("write TLK3 24",0,0,PED_TLK, 0x01,24,0x03);
	PedCiphertextTest("write TLK",1,1,1,1,16,0x03, 0x01);*/
	//WriteKeyDBufLen(0x01, 10,10,PED_RET_ERR_KCV_CHECK_FAIL);

	//pedWriteKeyEx_lib
	//WriteKeyExCipherTMPADK(16,0x03,0x01,PED_RET_OK);
	//40AD0006000000A12216A70000000010010010000000C2AE9E520B67107A45C443FBD3621A670000000000000000000300000054EEEEEEEEEEEEEEEE0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000DC
	//40AD0006000000A12216A70000000010010010000000C2AE9E520B67107A45C443FBD3621A670000000000000000000300000054EEEEEEEEEEEEEEEE0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000DC
	char *pc = "00000010010010000000C2AE9E520B67107A45C443FBD3621A670000000000000000000300000054EEEEEEEEEEEEEEEE0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    uchar pdata[900] = {0};
	ST_KEY_INFO_EX  KeyInEx;
	ST_KCV_INFO KcvIn;	
    StrToHex((uchar*)pc, strlen(pc), pdata);
	memcpy(&KeyInEx, pdata, sizeof(ST_KEY_INFO_EX));
    memcpy(&KcvIn, pdata + sizeof(ST_KEY_INFO_EX), sizeof(ST_KCV_INFO));
	iRet = pedWriteKeyEx_lib(&KeyInEx, &KcvIn);
    sysLOG(API_LOG_LEVEL_2, " pedWriteKeyEx_lib,iRet = %d, iRet=0x%x\r\n", iRet, iRet);
	//pedWriteTiK_lib

	//PedCalcDESTest(8,1, 8, DECRYPT,PED_RET_OK) ;
}
#endif

#if 0
int g_i32EncDecRet = 0;
void TestBigDataCalcEncDec(void)
{
    unsigned char pucInData[2049];
	unsigned char pucOutData[2049];
	
	memset(pucInData, 0x55, 2048);
	memset(pucOutData, 0x00, 2048);
	sysLOG(API_LOG_LEVEL_0, "api_pedBigDataCalcSym Start!%d\r\n", g_i32EncDecRet);
	g_i32EncDecRet = pedBigDataCalcSym_lib(20, 0, pucInData, 2048, pucOutData, 0x41);
	sysLOG(API_LOG_LEVEL_0, "api_pedBigDataCalcSymENC,g_i32Ret = %d, pucOutData=%s\r\n", g_i32EncDecRet, pucOutData);
	memset(pucInData, 0x00, 2048);
	g_i32EncDecRet = pedBigDataCalcSym_lib(20, 0, pucOutData, 2048, pucInData, 0x40);
	sysLOG(API_LOG_LEVEL_0, "api_pedBigDataCalcSymDEC,g_i32Ret = %d, pucInData=%s\r\n", g_i32EncDecRet, pucInData);
}
#endif