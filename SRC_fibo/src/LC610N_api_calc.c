
#include "comm.h"
#include "LC610N_api_calc.h"



/*
*@Brief:		Des 加密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 8 字节 DES 密钥 
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcDesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint8_t *IV, int mode)
{
    int iRet = ERR_CRYPTO_ERR;
	if(lenth % 8 != 0)
    {
    	iRet = ERR_CRYPTO_LENGTH;
    	goto RET_END;
    }
	
    if(mode == 0)
    {
        if(DesECB(FALSE, deskey, input, lenth, output))
    	iRet = 0;
    }
    else if(mode == 1)
    {
	    if(IV == NULL)
	    {
	    	iRet = ERR_CRYPTO_INVALID_PARAMS;
	    	goto RET_END;
	    }
        if(DesCBC(FALSE, deskey, input, lenth, output, IV))
    	iRet = 0;
    }
	else
	{
    	iRet = ERR_CRYPTO_MODE;
    }

RET_END:
	return iRet;
}


/*
*@Brief:		Des 解密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 8 字节 DES 密钥 
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcDesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint8_t *IV, int mode)
{
    int iRet = ERR_CRYPTO_ERR;
	if(lenth % 8 != 0)
    {
    	iRet = ERR_CRYPTO_LENGTH;
    	goto RET_END;
    }
	
    if(mode == 0)
    {
        if(DesECB(TRUE, deskey, input, lenth, output))
    	iRet = 0;
    }
    else if(mode == 1)
    {
	    if(IV == NULL)
	    {
	    	iRet = ERR_CRYPTO_INVALID_PARAMS;
	    	goto RET_END;
	    }
        if(DesCBC(TRUE, deskey, input, lenth, output, IV))
    	iRet = 0;
    }
	else
	{
    	iRet = ERR_CRYPTO_MODE;
    }

RET_END:
	return iRet;
}


/*
*@Brief:		3Des 加密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 16/24 字节 DES 密钥
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcTdesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode)
{
    uint8_t  bKey[25] = {0};
    int iRet = ERR_CRYPTO_ERR;
	if(lenth % 8 != 0)
    {
    	iRet = ERR_CRYPTO_LENGTH;
    	goto RET_END;
    }
	memset(bKey, 0, sizeof(bKey));
	if(keyLen == 24)
	{
	    memcpy(bKey, deskey, 24);
	}
	else if(keyLen == 16)
	{
	    memcpy(bKey, deskey, 16);
        memcpy(bKey+16, deskey, 8);
	}
	else
	{
	    return ERR_CRYPTO_LENGTH;
	}

    if(mode == 0)
    {
        if(Des3ECB(FALSE, bKey, input, lenth, output))
    	iRet = 0;
#if 0
    sysDelayMs(500);
	char* caShow = (char*) fibo_malloc(24 * 2 + 1);
	HexToStr(input, lenth, caShow);
	sysLOG(API_LOG_LEVEL_2, "input = %s\r\n", caShow);
	sysDelayMs(500);
	memset(caShow, 0, 49);
	HexToStr(bKey, 24, caShow);
	sysLOG(API_LOG_LEVEL_2, "bKey = %s\r\n", caShow);
	sysDelayMs(500);
	for(int i = 0; i < 8; i++)
	{
	    sysLOG(API_LOG_LEVEL_2, "output[%d] = 0x%x\r\n", i, output[i]);
	}
	fibo_free(caShow);
#endif
    }
    else if(mode == 1)
    {
	    if(IV == NULL)
	    {
	    	iRet = ERR_CRYPTO_INVALID_PARAMS;
	    	goto RET_END;
	    }
        if(Des3CBC(FALSE, bKey, input, lenth, output, IV))
    	iRet = 0;
    }
	else
	{
    	iRet = ERR_CRYPTO_MODE;
    }

RET_END:

	return iRet;
}


/*
*@Brief:		3Des 解密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 16/24 字节 DES 密钥
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcTdesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode)
{
	uint8_t  bKey[25] = {0};
    int iRet = ERR_CRYPTO_ERR;
	if(lenth % 8 != 0)
    {
    	iRet = ERR_CRYPTO_LENGTH;
    	goto RET_END;
    }
	memset(bKey, 0, sizeof(bKey));
	if(keyLen == 24)
	{
	    memcpy(bKey, deskey, 24);
	}
	else if(keyLen == 16)
	{
	    memcpy(bKey, deskey, 16);
        memcpy(bKey+16, deskey, 8);
	}
	else
	{
	    return ERR_CRYPTO_LENGTH;
	}
	
    if(mode == 0)
    {
        if(Des3ECB(TRUE, bKey, input, lenth, output))
    	iRet = 0;
    }
    else if(mode == 1)
    {
	    if(IV == NULL)
	    {
	    	iRet = ERR_CRYPTO_INVALID_PARAMS;
	    	goto RET_END;
	    }
        if(Des3CBC(TRUE, bKey, input, lenth, output, IV))
    	iRet = 0;
    }
	else
	{
    	iRet = ERR_CRYPTO_MODE;
    }

RET_END:
	return iRet;
}


/*
*@Brief:		Aes加密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，16字节倍数 
                *deskey [输入] 16/24/32 字节 
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcAesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode)
{
	int iRet = ERR_CRYPTO_ERR;
	int output_len = 0;
	int key_len = keyLen;
	int input_len = lenth;
	int iIvLen = 0;
	if((lenth % 16 != 0) || ((keyLen != 16) && (keyLen != 24) && (keyLen != 32)))
    {
    	iRet = ERR_CRYPTO_LENGTH;
    	goto RET_END;
    }
	if((mode == 1)  && (IV == NULL))
    {
    	iRet = ERR_CRYPTO_MODE;
    	goto RET_END;
    }
	if(mode == 1)
	{
		iIvLen = 16;
	}

	int cmd_len = 6 + 2 + input_len + 1 + key_len + 1 + iIvLen + 1;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x02, 0x00, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = lenth;
	*(uccmd + 7) = lenth >> 8;
	memcpy(uccmd + 8, input, lenth);
	*(uccmd + 8 + lenth) = keyLen;
	memcpy(uccmd + 8 + lenth + 1, deskey, keyLen);
	if(mode == 0)
	{
	    *(uccmd + 8 + lenth + 1 + keyLen) = 0;
		*(uccmd + 8 + lenth + 1 + keyLen + 1) = 0;
	}
	else if(mode == 1)
	{
		*(uccmd + 8 + lenth + 1 + keyLen) = 16;
		memcpy(uccmd + 8 + lenth + 1 + keyLen + 1, IV, 16);
		*(uccmd + 8 + lenth + 1 + keyLen + 17) = 1;
	}
	else
	{
	    free(uccmd);
	    iRet = ERR_CRYPTO_MODE;
    	goto RET_END;
	}
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(uccmd);
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
		memcpy(output, retfrm.data + 6, output_len);
		iRet = 0;
	}
	else if(iRet == 0xa006)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
	else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}


/*
*@Brief:		Aes解密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，16字节倍数 
                *deskey [输入] 16/24/32 字节 
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcAesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode)
{
	int iRet = ERR_CRYPTO_ERR;
	int output_len = 0;
	int key_len = keyLen;
	int input_len = lenth;
	int iIvLen = 0;
	if((lenth % 16 != 0) || ((keyLen != 16) && (keyLen != 24) && (keyLen != 32)))
    {
    	iRet = ERR_CRYPTO_LENGTH;
    	goto RET_END;
    }
	if((mode == 1)  && (IV == NULL))
    {
    	iRet = ERR_CRYPTO_MODE;
    	goto RET_END;
    }
	if(mode == 1)
	{
		iIvLen = 16;
	}

	int cmd_len = 6 + 2 + input_len + 1 + key_len + 1 + iIvLen + 1;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x02, 0x01, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = lenth;
	*(uccmd + 7) = lenth >> 8;
	memcpy(uccmd + 8, input, lenth);
	*(uccmd + 8 + lenth) = keyLen;
	memcpy(uccmd + 8 + lenth + 1, deskey, keyLen);
	if(mode == 0)
	{
	    *(uccmd + 8 + lenth + 1 + keyLen) = 0;
		*(uccmd + 8 + lenth + 1 + keyLen + 1) = 0;
	}
	else if(mode == 1)
	{
		*(uccmd + 8 + lenth + 1 + keyLen) = 16;
		memcpy(uccmd + 8 + lenth + 1 + keyLen + 1, IV, 16);
		*(uccmd + 8 + lenth + 1 + keyLen + 17) = 1;
	}
	else
	{
		free(uccmd);
		iRet = ERR_CRYPTO_MODE;
    	goto RET_END;
	}
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(uccmd);
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
		memcpy(output, retfrm.data + 6, output_len);
		iRet = 0;
	}
	else if(iRet == 0xa006)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
	else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}


/*
*@Brief:		进行 RSA 加解密运
*@Param IN:		*Modul[输入] 存放 RSA 运算的模缓冲区指针，RSA 模数 n = p*q。按高 位在前， 低位在后的顺序存储。  
				ModulLen[输入] 模长(1～256)，单位：字节。 
				*Exp[输入] 存放 RSA 运算的指数缓冲区指针，指数 e /d。按高位在 前， 低位在后的顺序存储。。 
				ExpLen[输入] 指数长度（以 BYTE 为单位） 
				*DataIn[输入] 输入数据缓冲区指针，长度与模长度同
*@Param OUT:	*DataOut [输出] 输出数据缓冲区指针，长度与模长度同  
*@Return:		0:成功; <0:失败
*/
int calcRsaRecover_lib(uchar *Modul, uint ModuleLen, uchar *Exp,  uint ExpLen, uchar *DataIn, uchar *DataOut)
{
	int output_len = 0;

	int cmd_len = 6 + 2 + ModuleLen + 2 + ExpLen + ModuleLen;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x04, 0x00, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = ModuleLen;
	*(uccmd + 7) = ModuleLen >> 8;
	memcpy(uccmd + 8, Modul, ModuleLen);
	*(uccmd + 8 + ModuleLen) = ExpLen;
	*(uccmd + 8 + ModuleLen + 1) = ExpLen >> 8;
	memcpy(uccmd + 8 + ModuleLen + 2, Exp, ExpLen);
	memcpy(uccmd + 8 + ModuleLen + 2 + ExpLen, DataIn, ModuleLen);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(uccmd);
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
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		memcpy(DataOut, retfrm.data + 6, output_len);
#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(output_len * 2 + 1);
		HexToStr(DataOut, output_len, caShow);
		sysLOG(API_LOG_LEVEL_4, "DataOut = %s\r\n", caShow);
		fibo_free(caShow);
#endif
		iRet = 0;
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
	else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}

/*
*@Brief:		RSA 生成密钥对
*@Param IN:		modulus_size [输入] 密钥位数,最大支持 2048 
				e_type [输入] E 位数，支持 3 或 65537. 
*@Param OUT:	*pbModulus [输出] 公钥模数。 
				*pdwPubExp [输出] 公钥指数。 
				*pbD [输出] 私钥指数 
*@Return:		0:成功; <0:失败
*/
int calcRsaKeyPairGen_lib (uint32_t modulus_size,  uint32_t e_type, uint8_t *pbModulus,  uint32_t *pdwPubExp, uint8_t *pbD)
{
	int output_len = 0;

	int cmd_len = 6 + 2 + 4;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x04, 0x01, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = modulus_size;
	*(uccmd + 7) = modulus_size >> 8;
	memcpy(uccmd+8, &e_type, 4);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(uccmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 10000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		if(output_len != (modulus_size / 8 * 2 + 4))
		{
		    iRet = ERR_CRYPTO_ERR;
		}
		else
		{
			memcpy(pbModulus, retfrm.data + 6, modulus_size/8);
			memcpy(pdwPubExp, retfrm.data + 6 + modulus_size/8, 4);
			memcpy(pbD, retfrm.data + 6 + modulus_size/8 + 4, modulus_size/8);
			iRet = 0;
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
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}

/*
*@Brief:		计算安全的 hash 值
*@Param IN:		*DataIn[输入] 输入数据  
				DataInLen[输入] 输入数据长度(以字节为单位) 
				Mode[输入] 0   SHA_TYPE_160  1   SHA_TYPE_224 2   SHA_TYPE_256 3   SHA_TYPE_384 4   SHA_TYPE_512 目前只支持SHA1\SHA256
*@Param OUT:	*DataOut  [输出] 输出数据缓冲区指针   
*@Return:		0:成功; <0:失败
*/
int calcSha_lib( const uchar*DataIn, int DataInLen, uchar* ShaOut, int Mode)
{
    if((Mode != 0) && (Mode != 2))
    {
    	return ERR_CRYPTO_MODE;
    }
	int output_len = 0;

	int cmd_len = 6 + 1 + 4 + DataInLen;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x03, 0x00, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = Mode;
	memcpy(uccmd+7, &DataInLen, 4);
	//*(uccmd + 7) = DataInLen;
	//*(uccmd + 8) = DataInLen >> 8;
	memcpy(uccmd + 11, DataIn, DataInLen);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(uccmd);
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
		if(output_len != 16)
		{
			iRet = ERR_CRYPTO_ERR;
		}
		else
		{
			memcpy(ShaOut, retfrm.data + 6, output_len);
			iRet = 0;
		}
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
	else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;
}

/*
*@Brief:		RSA 生成密钥对
*@Param IN:		exponent [输入] 公钥指数 支持 3 或 65537 
				nbits [输入]  RSA模数位数。受限于CPU的硬件算法库，目前只支持1024及2048位
*@Param OUT:	*key [输出] RSA私钥缓存。 
*@Return:		0:成功; <0:失败
*/
uint32_t calcRsaGenKey_lib(rsa_private_key_t *key, uint32_t exponent, uint32_t nbits)
{
	int output_len = 0;

	int cmd_len = 6 + 2 + 4;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x04, 0x02, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = nbits;
	*(uccmd + 7) = nbits >> 8;
	memcpy(uccmd+8, &exponent, 4);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(uccmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 10000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		if(output_len != (nbits*9/16 + 4))
		{
			iRet = ERR_CRYPTO_ERR;
		}
		else
		{
		    memset(key, 0, sizeof(rsa_private_key_t));
			memcpy(key->e,  retfrm.data + 6, 4);
			memcpy(key->p,  retfrm.data + 10, nbits/16);
			memcpy(key->q,  retfrm.data + 10 + nbits/16, nbits/16);
			memcpy(key->n,  retfrm.data + 10 + nbits/8, nbits/8);
			memcpy(key->d,  retfrm.data + 10 + nbits/4, nbits/8);
			memcpy(key->dp, retfrm.data + 10 + nbits*3/8, nbits/16);
			memcpy(key->dq, retfrm.data + 10 + nbits*7/16, nbits/16);
			memcpy(key->qp, retfrm.data + 10 + nbits*8/16, nbits/16);
			key->bytes = nbits / 8;// nbits*9/16 + 8;
			iRet = 0;
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
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x, output_len=%d\r\n", iRet, iRet, output_len);
	return iRet;

}

/*
*@Brief:		RSA私钥运算，使用CRT算法。CRT为中国剩余定理，用于加快RSA的运算
*@Param IN:		*input[输入] 输入数据缓存  
				*key[输入] RSA私钥 由calcRsaGenKey_lib接口生成的key
*@Param OUT:	*output [输出] 输出数据缓冲区指针，长度与模长度同  
*@Return:		0:成功; <0:失败
*/
uint32_t calcRsaPrivateCrt_lib(uint8_t *output, uint8_t *input, rsa_private_key_t *key)
{
	int output_len = 0;
	int nbits = 0;
	if((key == NULL) || (output == NULL) || (input == NULL))
	{
	    return ERR_CRYPTO_LENGTH;
	}
    nbits = key->bytes * 8;// (key->bytes - 8) * 16 / 9;
	if((nbits != 1024) && (nbits != 2048))
	{
	    return ERR_CRYPTO_LENGTH;
	}
	//uint8_t baKeyData[nbits*9/16 +5];
	uint8_t *baKeyData = (uint8_t*) fibo_malloc(nbits*9/16 +5);
	memset(baKeyData, 0, nbits*9/16 +5);
	memcpy(baKeyData, key->e, 4);
	memcpy(baKeyData + 4, key->p, nbits/16);
	memcpy(baKeyData + 4 + nbits/16, key->q,  nbits/16);
	//memcpy(baKeyData + 4 + nbits/8, key->n,  nbits/8);
	//memcpy(baKeyData + 4 + nbits/4, key->d,  nbits/8);
	memcpy(baKeyData + 4 + nbits/8, key->dp, nbits/16);
	memcpy(baKeyData + 4 + nbits*3/16, key->dq, nbits/16);
	memcpy(baKeyData + 4 + nbits*4/16, key->qp, nbits/16);
	int cmd_len = 6 + 2 + nbits/8 + nbits*5/16 +4;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x04, 0x03, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = nbits;
	*(uccmd + 7) = nbits >> 8;
	memcpy(uccmd + 8, input, nbits/8);
	memcpy(uccmd + 8 + nbits/8, baKeyData, nbits*5/16 +4);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(baKeyData);
	fibo_free(uccmd);
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
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		if(output_len != nbits/8)
		{
			iRet = ERR_CRYPTO_ERR;
		}
		else
		{
			memcpy(output, retfrm.data + 6, output_len);
#ifdef PRINT_API_CMD
			char* caShow = (char*) fibo_malloc(output_len * 2 + 1);
			HexToStr(output, output_len, caShow);
			sysLOG(API_LOG_LEVEL_4, "output = %s\r\n", caShow);
			fibo_free(caShow);
#endif
			iRet = 0;
		}
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
	else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}

/*
*@Brief:		RSA公钥运算
*@Param IN:		*input[输入] 输入数据缓存  
				*key[输入] RSA公钥 结构体的前3个参数数据有效
*@Param OUT:	*output [输出] 输出数据缓冲区指针，长度与模长度同  
*@Return:		0:成功; <0:失败
*/
uint32_t calcRsaPublic_lib(uint8_t *output, uint8_t *input, rsa_public_key_t *key)
{
	int output_len = 0;
	int nbits = 0;
	if((key == NULL) || (output == NULL) || (input == NULL))
	{
	    return ERR_CRYPTO_LENGTH;
	}
    nbits = key->bytes * 8;// (key->bytes - 8) * 16 / 9;
	if((nbits != 1024) && (nbits != 2048))
	{
	    return ERR_CRYPTO_LENGTH;
	}
	uint8_t *baKeyData = (uint8_t*) fibo_malloc(nbits/8 +5);//[nbits/8 +5];
	memset(baKeyData, 0, nbits/8 +5);
	memcpy(baKeyData, key->e, 4);
	memcpy(baKeyData + 4,  key->n,  nbits/8);
	int cmd_len = 6 + 2 + nbits/8 + nbits/8 +4;
	unsigned char cmd_head[6] = {0x00, 0xf2, 0x04, 0x04, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* uccmd = (unsigned char*) fibo_malloc(cmd_len + 1);
	memcpy(uccmd, cmd_head, sizeof(cmd_head));
	*(uccmd + 6) = nbits;
	*(uccmd + 7) = nbits >> 8;
	memcpy(uccmd + 8, input, nbits/8);
	memcpy(uccmd + 8 + nbits/8, baKeyData, nbits/8 +4);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(baKeyData, nbits/8 +4, caShow);
	sysLOG(API_LOG_LEVEL_4, "baKeyData = %s\r\n", caShow);
    memset(caShow, 0, sizeof(caShow));
	HexToStr(uccmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(uccmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(baKeyData);
	fibo_free(uccmd);
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
		output_len = retfrm.data[5]<<8 | retfrm.data[4];
		if(output_len != nbits/8)
		{
			iRet = ERR_CRYPTO_ERR;
		}
		else
		{
			memcpy(output, retfrm.data + 6, output_len);
#ifdef PRINT_API_CMD
			char* caShow = (char*) fibo_malloc(output_len * 2 + 1);
			HexToStr(output, output_len, caShow);
			sysLOG(API_LOG_LEVEL_4, "output = %s\r\n", caShow);
			fibo_free(caShow);
#endif
			iRet = 0;
		}
	}
	else if(iRet == 0x9399)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }
	else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_1, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}

#if 0
static unsigned char aasc_to_bcd(unsigned char ucAsc)
{
	unsigned char ucBcd = 0;

	if ((ucAsc >= '0') && (ucAsc <= '9'))
		ucBcd = ucAsc - '0';
	else if ((ucAsc >= 'A') && (ucAsc <= 'F'))
		ucBcd = ucAsc - 'A' + 10;
	else if ((ucAsc >= 'a') && (ucAsc <= 'f'))
		ucBcd = ucAsc - 'a' + 10;
	else if ((ucAsc > 0x39) && (ucAsc <= 0x3f))
		ucBcd = ucAsc - '0';
	else 
 		ucBcd = 0x0f;
	
	return ucBcd;
}

void AscToBcd(unsigned char  * sBcdBuf,     char  * sAscBuf, int iAscLen)
{
	int i, j;

	if((sBcdBuf == NULL) || (sAscBuf == NULL) || (iAscLen < 0))
		return;
	
	j = 0;

	for (i = 0; i < (iAscLen + 1) / 2; i++) 
	{
		sBcdBuf[i] = aasc_to_bcd(sAscBuf[j++]) << 4;
		sBcdBuf[i] |= (j >= iAscLen) ? 0x00 : aasc_to_bcd(sAscBuf[j++]);
	}
}
#endif

#if 0

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

int DesCBCTest(uchar *InputData, uchar *PreResult , uchar *DesKey,uchar *IV,int Mode,int PreRet)
{   
	uchar OutPutData[20],PreRes[40],OutPut[40];
	int Ret = 255;
	
  
	memset(OutPutData,0,sizeof(OutPutData));
	memset(PreRes,0,sizeof(PreRes));
	memset(OutPut,0,sizeof(OutPut));
	
  if(Mode==1)//加密
   Ret =calcDesEnc_lib(InputData,8, OutPutData, DesKey, IV,1);
  if(Mode==0)//解密
   Ret =calcDesDec_lib (InputData,8, OutPutData,DesKey, IV,1);

  //OSI_LOGI(0,"Case_16:return %d\r\n",Ret);
//	Ret = api_descbc(InputData,OutPutData,DesKey,IV,Mode);
	if((0 == Ret) && (0 == PreRet))
	{

		BcdToAsc(PreRes, PreResult,16);
		BcdToAsc(OutPut, OutPutData,16);
    sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s %s\r\n",OutPut,PreRes);
		if(0 == memcmp(OutPutData,PreResult,8))
		{
			Ret = 0;
			//api_ScrPrint(0,2,0, "cmp result %d",Ret);
		}
		else
		{
			Ret = 1;	
			//api_ScrPrint(0,2,0, "cmp result %d",Ret);
		}
	}	
	if (Ret == PreRet)
	{
		return 0;
	}
	else
	{	
		if(0 == Ret)
		{
			return 200;
		}
		else
		{
			return Ret;
		}
	}
}

int TDesCBCTest(uchar *InputData, uchar *PreResult , uchar *DesKey,int keyLen,uchar *IV,int Mode,int PreRet)
{   
	uchar OutPutData[20],PreRes[40],OutPut[40];
	int Ret = 255;
	
  
	memset(OutPutData,0,sizeof(OutPutData));
	memset(PreRes,0,sizeof(PreRes));
	memset(OutPut,0,sizeof(OutPut));
	
   if(Mode==1)//加密
   //calcTdesEnc_lib(uint8_t *input,int lenth,uint8_t *output,uint8_t *deskey,uint32_t keyLen,uint8_t *IV，int mode);
     Ret=calcTdesEnc_lib(InputData,16,OutPutData,DesKey,keyLen,IV,1);
   if(Mode==0)//解密
   //calcTdesDec_lib (uint8_t *input,int lenth,uint8_t *output,uint8_t *deskey,uint32_t keyLen,uint8_t *IV，int mode);
     Ret=calcTdesDec_lib(InputData,16,OutPutData,DesKey,keyLen,IV,1);

  //OSI_LOGI(0,"Case_16:return %d\r\n",Ret);
//	Ret = api_descbc(InputData,OutPutData,DesKey,IV,Mode);
	if((0 == Ret) && (0 == PreRet))
	{

		BcdToAsc(PreRes, PreResult,32);
		BcdToAsc(OutPut, OutPutData,32);
    sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s, %s\r\n",OutPut,PreRes);
		if(0 == memcmp(OutPutData,PreResult,16))
		{
			Ret = 0;
			//api_ScrPrint(0,2,0, "cmp result %d",Ret);
		}
		else
		{
			Ret = 1;	
			//api_ScrPrint(0,2,0, "cmp result %d",Ret);
		}
	}	
	if (Ret == PreRet)
	{
		return 0;
	}
	else
	{	
		if(0 == Ret)
		{
			return 200;
		}
		else
		{
			return Ret;
		}
	}
}


int Case_1601005(void)
{
	int Mode = 1;
	uchar input[40],output[40],deskey[40],preResult[40],IV[40];
	int Ret = -1,PreRet = 0,allRet = -1,i;

	memset(input,0,sizeof(input));
	memset(output,0,sizeof(output));
	memset(deskey,0,sizeof(deskey));
	memset(preResult,0,sizeof(preResult));
	memset(IV,0,sizeof(IV));

	AscToBcd (input, "0102030405060708", 16);
	AscToBcd (deskey, "0102030405060708", 16);
	AscToBcd(preResult, "B073DC3FB209536D", 16);
	AscToBcd(IV, "0102030405060708", 16);
	PreRet = 0;
	for(i = 0;i<3;i++)
	{
	    AscToBcd(IV, "0102030405060708", 16);
        Ret = DesCBCTest(input,preResult,deskey,IV,Mode,PreRet);
		if(Ret != 0)
		{
			allRet = Ret ;
			//api_ScrPrint(0,3,0, "i= %d",i);
			sysDelayMs(2000);
			break;
		}
		if(99 == i)
		{
			allRet = 0;
		}
	}
	return allRet;
}
int Case_1603001(void)
{
	int Mode = 1;
	uchar input[40],output[40],deskey[40],preResult[40],IV[40];
	int Ret = -1,PreRet = 0;

	memset(input,0,sizeof(input));
	memset(output,0,sizeof(output));
	memset(deskey,0,sizeof(deskey));
	memset(preResult,0,sizeof(preResult));
	memset(IV,0,sizeof(IV));
	AscToBcd (input, "01020304050607080102030405060708", 32);
	AscToBcd (deskey, "01020304050607080102030405060708", 32);
	AscToBcd(preResult, "B073DC3FB209536D", 16);
	AscToBcd(IV, "0102030405060708", 16);
	PreRet = 0;
	//mode = 1;
 //int TDesCBCTest(uchar *InputData, uchar *PreResult , uchar *DesKey,int keyLen,uchar *IV,int Mode,int PreRet)
	Ret = TDesCBCTest(input,preResult,deskey,16,IV,Mode,PreRet);
	return Ret;
}
int RSATest(uchar * DataIn, uchar * PreResult , uchar * Modul,unsigned int ModulLen,uchar * Exp,unsigned int ExpLen,int PreRet)
{   
	uchar DateOut[300],PreRes[520],OutPut[520];
	int Ret = 255;

	memset(DateOut,0,sizeof(DateOut));
	memset(PreRes,0,sizeof(PreRes));
	memset(OutPut,0,sizeof(OutPut));	
	Ret = calcRsaRecover_lib(Modul,ModulLen,Exp,ExpLen,DataIn,DateOut);
	hal_scrPrint(0,3,0, "Ret_frist = %d",Ret);	
    BcdToAsc(PreRes, PreResult,ModulLen*2);
		BcdToAsc(OutPut, DateOut,ModulLen*2);
  hal_scrPrint(0,4,0, "DateOut: %s",OutPut);
  hal_scrPrint(0,6,0, "PreResult %s",PreRes);
	if((0 == Ret)&&(0 == PreRet))
	{
		if(0 == memcmp(DateOut,PreResult,ModulLen))
		{
			Ret = 0;
			//hal_scrPrint(0,5,0, "cmp result_first %d",Ret);
		}
		else
		{
			Ret = 1;	
			 //hal_scrPrint(0,6,0, "cmp result_second %d",Ret);
		}
	}
	//hal_scrPrint(0,4,1, "Ret_second: %d",Ret);
	//CalculateMessage(Ret);
	if (Ret == PreRet)
	{
		return 0;
	}
	else
	{	
		if(0 == Ret)
		{
			return 200;
		}
		else
		{
			return Ret;
		}
	}
}

int Case_1604001(void)
{
	int Ret = -1,PreRet = 0;
	uint ModulLen,ExpLen;
	uchar Modul[600],Exp[600],DataIn[600],preResult[600],pbD[600];

	memset(Modul,0,sizeof(Modul));
	memset(Exp,0,sizeof(Exp));
	memset(DataIn,0,sizeof(DataIn));
	memset(preResult,0,sizeof(preResult));
	memset(pbD,0,sizeof(pbD));

	AscToBcd(Modul, "B240706080AF07ABF7A59DEA3FD90906EFE4DA6D13CAC9A78AC64AF2D61ED7563DE9CF4892B30DA554EB77D44717F7AA93F24A70E82035BF26E04A422F78EE73", 128);
	AscToBcd(Exp,"03",2);
	AscToBcd(DataIn,"11111111111111111111111111111111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",128);
	AscToBcd(preResult,"15923F0DD1B3F6BC287695D9D5A81CD7C5652F1C536B34710DA172137E64C507A6F22F67E9BE729729FA36E2182ED90BE98104DF34004C3A282E9AD989E92BCB",128);
	AscToBcd(pbD,"76D5A04055CA051D4FC3BE9C2A90B0AF4A98919E0D31DBC5072EDCA1E4148F8DB7178C18348854ABFA3DC8F7073F7805021156BA562A8E8B3BAA86D8582B0F7B",128);
	
	ModulLen = 64;
	ExpLen = 1;
	PreRet = 0;
	
	Ret = RSATest(DataIn,preResult,Modul,ModulLen,Exp,ExpLen,PreRet);

	if(Ret != 0)
	{
		return Ret;
	}

	Ret = RSATest(preResult,DataIn,Modul,ModulLen,pbD,ModulLen,PreRet);

	return Ret;
}

int Case_1607004(void)
{	
	//return RSA_Gen_Verify0(1024);
 int Ret = -1,Ret1 = -1,PreRet = 0,j = 0;
	uint e_type;
  uchar Modulus[200],D[200];
  uchar DataIn[100];
	uint pbModulus[260],pdwPubExp[100],pbD[260];
 
  uchar DateOut[300],OutPut[520],DateOut1[300],OutPut1[520];
   
	memset(pbModulus,0,sizeof(pbModulus));
	memset(pdwPubExp,0,sizeof(pdwPubExp));
	memset(pbD,0,sizeof(pbD));
  
  memset(DateOut,0,sizeof(DateOut));
  memset(DateOut1,0,sizeof(DateOut1));
	memset(OutPut,0,sizeof(OutPut));
  memset(OutPut1,0,sizeof(OutPut1));
  memset(DataIn,0,sizeof(DataIn));
  AscToBcd(DataIn,"11111111111111111111111111111111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",128);
  memset(Modulus,0,sizeof(Modulus));
	memset(D,0,sizeof(D));
	//memset(e_type,0,sizeof(e_type));
	PreRet = 0;
	//DispScrPrint(0, LINE2, G_FontSet, NOFDISP|CDISP, "生成密钥测试");
	e_type = 65537;
  
	Ret = calcRsaKeyPairGen_lib(512,e_type,pbModulus,pdwPubExp,pbD);
  BcdToAsc(D,pbD,128);BcdToAsc(Modulus,pbModulus,128);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:calcRsa Modulus= %s\r\n",Modulus);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:calcRsa D= %s\r\n",D);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:calcRsa Ret= %d\r\n",Ret);

  if(0 == Ret)
  {
     Ret = calcRsaRecover_lib(pbModulus,64,pbD,64,DataIn,DateOut);
     hal_scrPrint(0,3,0, "Ret_frist = %d",Ret);	
		 BcdToAsc(OutPut, DateOut,128);
     sysLOG(BASE_LOG_LEVEL_0, "Case_16:OutPut=%s\r\n",OutPut);
     if(0 == Ret)
	    {
		    //Ret = calcRsaRecover_lib(pbModulus,64,pdwPubExp,4,DateOut,DateOut1);
		Ret = calcRsaRecover_lib(pbModulus,64,&e_type,4,DateOut,DateOut1);
        BcdToAsc(OutPut1, DateOut1,128);
        sysLOG(BASE_LOG_LEVEL_0, "Case_16:OutPut1=%s\r\n",OutPut1);
        //sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s\r\n",PreRes);
        if(0 == Ret)
        {
          if(0 == memcmp(DateOut1,DataIn,64)) 
           Ret = 0;
          else
           Ret=-1;
        }
	   }
  }
  else if(Ret != 0)
	{
		//DispScrPrint(0, LINE3, G_FontSet, NOFDISP|CDISP, "生成密钥失败");
		//api_ScrPrint(0,2,2,"GenKeyErr %d",Ret);
		return Ret;
	}
 

	return Ret;
}

int Case_1607006(void)
{	
 int Ret = -1,Ret1 = -1,PreRet = 0,j = 0;
	uint e_type;
  uchar Modulus[520],D[520];
  uchar DataIn[260];
	uint pbModulus[300],pdwPubExp[100],pbD[300];
 
  uchar DateOut[300],OutPut[520],DateOut1[300],OutPut1[520];
   
	memset(pbModulus,0,sizeof(pbModulus));
	memset(pdwPubExp,0,sizeof(pdwPubExp));
	memset(pbD,0,sizeof(pbD));
  
  memset(DateOut,0,sizeof(DateOut));
  memset(DateOut1,0,sizeof(DateOut1));
	memset(OutPut,0,sizeof(OutPut));
  memset(OutPut1,0,sizeof(OutPut1));
  memset(DataIn,0,sizeof(DataIn));
  AscToBcd(DataIn,"11111111111111111111111111111111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",512);
  memset(Modulus,0,sizeof(Modulus));
	memset(D,0,sizeof(D));
	//memset(e_type,0,sizeof(e_type));
	PreRet = 0;
	//DispScrPrint(0, LINE2, G_FontSet, NOFDISP|CDISP, "生成密钥测试");
	e_type = 65537;
  
	Ret = calcRsaKeyPairGen_lib(2048,e_type,pbModulus,pdwPubExp,pbD);
  BcdToAsc(D,pbD,512);BcdToAsc(Modulus,pbModulus,512);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:calcRsa return %s\r\n",Modulus);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:calcRsa return %s\r\n",D);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:calcRsa return %d\r\n",Ret);
  
 	if(0 == Ret)
  {
     Ret = calcRsaRecover_lib(pbModulus,256,pbD,256,DataIn,DateOut);
     hal_scrPrint(0,3,0, "Ret_frist = %d",Ret);	
		 BcdToAsc(OutPut, DateOut,512);
     sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s\r\n",OutPut);
     if(0 == Ret)
	    {
		    Ret = calcRsaRecover_lib(pbModulus,256,pdwPubExp,4,DateOut,DateOut1);
        BcdToAsc(OutPut1, DateOut1,512);
        sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s\r\n",OutPut1);
        //sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s\r\n",PreRes);
        if(0 == Ret)
        {
          if(0 == memcmp(DateOut1,DataIn,256)) 
           Ret = 0;
          else
           Ret=-1;
        }
	   }
  }
  else if(Ret != 0)
	{
		//DispScrPrint(0, LINE3, G_FontSet, NOFDISP|CDISP, "生成密钥失败");
		//api_ScrPrint(0,2,2,"GenKeyErr %d",Ret);
		return Ret;
	}
 

	return Ret;
}


int AesecbTest(uchar * InputData, int inputlen,uchar * PreResult , uchar * aeskey, int keylen,int Mode,int PreRet)
{   
	uchar OutPutData[20],PreRes[40],OutPut[40];
	int Ret = 255;
	int outputlen = 512;
	
	memset(OutPutData,0,sizeof(OutPutData));
	memset(PreRes,0,sizeof(PreRes));
	memset(OutPut,0,sizeof(OutPut));
	
  if(Mode==1)
    //int calcAesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);
    Ret =calcAesEnc_lib(InputData,inputlen, OutPutData, aeskey, keylen, NULL,0);
  else if(Mode==0)
    //int calcAesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);
    Ret =calcAesDec_lib(InputData,inputlen, OutPutData, aeskey, keylen, NULL,0);
	//Ret = api_aesecb(InputData,inputlen, OutPutData, outputlen, aeskey,keylen,Mode);
    sysLOG(BASE_LOG_LEVEL_0, "Case_16:%d\r\n",Ret);
    BcdToAsc(PreRes, PreResult,16);
		BcdToAsc(OutPut, OutPutData,16);
    sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s %s\r\n",OutPut,PreRes);
	if((0 == Ret) && (0 == PreRet))
	{
	
		
		if(0 == memcmp(OutPutData,PreResult,8))
		{
			Ret = 0;
			//api_ScrPrint(0,2,0, "cmp result %d",Ret);
		}
		else
		{
			Ret = 1;	
		}
	}
	//api_ScrPrint(0,2,1, "Ret: %d",Ret);
	//CalculateMessage(Ret);
	if (Ret == PreRet)
	{
		return 0;
	}
	else
	{	
		if(0 == Ret)
		{
			return 200;
		}
		else
		{
			return Ret;
		}
	}
}

int AesCBCTest(uchar * InputData,  int inputlen, uchar * PreResult , uchar * aeskey, int keylen,uchar * IV,int IVlen,int Mode,int PreRet)
{   
  uchar OutPutData[20],PreRes[40],OutPut[40];
	int Ret = 255;
	int outputlen = 512;
	memset(OutPutData,0,sizeof(OutPutData));
	memset(PreRes,0,sizeof(PreRes));
	memset(OutPut,0,sizeof(OutPut));
	if(Mode==1)
    //int calcAesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);
    Ret =calcAesEnc_lib(InputData,inputlen, OutPutData, aeskey, keylen, IV,1);
  else if(Mode==0)
    //int calcAesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);
    Ret =calcAesDec_lib(InputData,inputlen, OutPutData, aeskey, keylen, IV,1);
	//Ret = api_aescbc(InputData,inputlen, OutPutData,outputlen,aeskey,keylen,IV,IVlen,Mode);
  
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:%d\r\n",Ret);
	BcdToAsc(PreRes, PreResult,16);
	BcdToAsc(OutPut, OutPutData,16);
  sysLOG(BASE_LOG_LEVEL_0, "Case_16:%s %s\r\n",OutPut,PreRes);
	if((0 == Ret) && (0 == PreRet))
	{
	if(0 == memcmp(OutPutData,PreResult,8))
	{
		Ret = 0;	
	}
	else
	{
		Ret = 1;	
	}
	}
	//api_ScrPrint(0,2,1, "Ret: %d",Ret);
	//CalculateMessage(Ret);
	if (Ret == PreRet)
	{
		return 0;
	}
	else
	{	
		if(0 == Ret)
		{
			return 200;
		}
		else
		{
			return Ret;
		}
	}
	
}

int Case_1605001(void)
{
	int Mode = 1;
	uchar input[40],output[40],aeskey[40],preResult[40];
	int Ret = -1,PreRet = 0;

	memset(input,0,sizeof(input));
	memset(output,0,sizeof(output));
	memset(aeskey,0,sizeof(aeskey));
	memset(preResult,0,sizeof(preResult));

	AscToBcd (input, "01020304050607080102030405060708", 32);
	AscToBcd (aeskey, "01020304050607080102030405060708", 32);
	AscToBcd(preResult, "E66AC2F214E2AF137922C1291C57F00D", 32);
	PreRet = 0;
	Ret = AesecbTest(input,16,preResult,aeskey,16,Mode,PreRet);

	return Ret;
	
}
int Case_1605004(void)
{
	int Mode = 1;
	uchar input[40],output[40],aeskey[40],preResult[40],IV[40];
	int Ret = -1,PreRet = 0;

	memset(input,0,sizeof(input));
	memset(output,0,sizeof(output));
	memset(aeskey,0,sizeof(aeskey));
	memset(preResult,0,sizeof(preResult));
	memset(IV,0,sizeof(IV));

	AscToBcd (input, "01020304050607080102030405060708", 32);
	AscToBcd (aeskey, "01020304050607080102030405060708", 32);
	AscToBcd(preResult, "C39E93D8CE44BEF6AADF2959FD066DB8", 32);
	AscToBcd(IV, "01020304050607080102030405060708", 32);
	PreRet = 0;
	Ret = AesCBCTest(input,16,preResult ,aeskey,16,IV,16,Mode,PreRet);

	return Ret;
}


void calcTest()
{
	Case_1605001();
	Case_1605004();
	//return;
	int Ret = -1,PreRet = 0;
	uint ModulLen,ExpLen;
	uchar Modul[600],Exp[600],pbD[600];
	uchar DataIn[600], Result[600], output[600];
	rsa_private_key_t key;
	rsa_public_key_t pubkey;

	memset(Modul,0,sizeof(Modul));
	memset(Exp,0,sizeof(Exp));
	memset(pbD,0,sizeof(pbD));
	memset(DataIn,0,sizeof(DataIn));
	memset(Result,0,sizeof(Result));
	memset(output,0,sizeof(output));
	for(int i=0;i<256;i++)
	{
		DataIn[i] = i+10;
	}
	//AscToBcd(DataIn, "B240706080AF07ABF7A59DEA3FD90906EFE4DA6D13CAC9A78AC64AF2D61ED7563DE9CF4892B30DA554EB77D44717F7AA93F24A70E82035BF26E04A422F78EE73B240706080AF07ABF7A59DEA3FD90906EFE4DA6D13CAC9A78AC64AF2D61ED7563DE9CF4892B30DA554EB77D44717F7AA93F24A70E82035BF26E04A422F78EE73B240706080AF07ABF7A59DEA3FD90906EFE4DA6D13CAC9A78AC64AF2D61ED7563DE9CF4892B30DA554EB77D44717F7AA93F24A70E82035BF26E04A422F78EE73B240706080AF07ABF7A59DEA3FD90906EFE4DA6D13CAC9A78AC64AF2D61ED7563DE9CF4892B30DA554EB77D44717F7AA93F24A70E82035BF26E04A422F78EE73", 512);
    Ret =  calcRsaGenKey_lib(&key, 0x10001, 2048);
	if(Ret != 0) return;
#if 0//def PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(300 * 2 + 1);
	HexToStr(key.e, 4, caShow);
	sysLOG(API_LOG_LEVEL_2, "key.e = %s\r\n", caShow);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(key.n, 256, caShow);
	sysLOG(API_LOG_LEVEL_2, "key.n = %s\r\n", caShow);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(key.d, 256, caShow);
	sysLOG(API_LOG_LEVEL_2, "key.d = %s\r\n", caShow);

	memset(caShow, 0, sizeof(caShow));
	HexToStr(DataIn, 256, caShow);
	sysLOG(API_LOG_LEVEL_2, "key.d = %s\r\n", caShow);
	fibo_free(caShow);
#endif
	pubkey.bytes = key.bytes;
	memcpy(pubkey.e, key.e, 4);
	memcpy(pubkey.n, key.n, key.bytes);
	Ret = calcRsaPublic_lib(Result, DataIn, &pubkey);
	if(Ret != 0) return;
	Ret =  calcRsaPrivateCrt_lib(output, Result, &key);
    memset(output,0,sizeof(output));
	Ret =  calcRsaRecover_lib(key.n, key.bytes, key.d, key.bytes, Result, output);
	
	//uint32_t calcRsaPublic_lib(uint8_t *output, uint8_t *input, rsa_public_key_t *key)
	//AscToBcd(DataIn, "B240706080AF07ABF7A59DEA3FD90906EFE4DA6D13CAC9A78AC64AF2D61ED7563DE9CF4892B30DA554EB77D44717F7AA93F24A70E82035BF26E04A422F78EE73", 128);
	//Ret =  calcSha_lib(DataIn, 64, Result, 2);
	//Ret =  calcRsaKeyPairGen_lib (2048,  3, Modul,  Exp,  pbD);
	//Ret =  calcRsaKeyPairGen_lib (1024,  65537, Modul,  Exp,  pbD);
    return Ret;
}

#endif

