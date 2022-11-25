
#include "comm.h"
#include "LC610N_api_gmsm.h"



/*
*@Brief:		国密模块上电以及初始化
*@Param IN:		*params: 曲线参数缓冲指针,至少 128 字节。
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int gmSm2Init_lib(unsigned char *params)
{
    int iRet = ERR_GM_NOINIT;
	int iCmdLen = 6;//暂不处理params数据长度
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 6);
	unsigned char ucCmdHead[6] = {0x00, 0xe5, 0x00, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	memcpy(ucCmd, ucCmdHead, 6);
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);	
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
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
		iRet = 0;
	}else {
		iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_4, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;
}

/*
*@Brief:		使用 SM2 公钥加密数据或私钥解密数据
*@Param IN:		*Key  [输入] 密钥数据。Sm2 算法的私钥长度为 32 字节，公钥长度为 64 字节。 
							Mode=0 时，Key 为 96 字节，数据结构为：64 字节公钥 （X+Y） +32 字节私钥。 
							Mode=1 时，Key 为 64 字节公钥（X+Y）
				KeyLen [输入] 密钥数据长度 
				*Input [输入] 待解密或加密的数据 
				InputLen [输入] 待解密或加密的数据长度 
 
				Mode [输入] 0x00 使用 SM2 私钥解密数据 
							0x01 使用 SM2 公钥加密数据 
*@Param OUT:	*Output [输出] 解密或加密后的数据 
				*OutputLen [输出] 解密或加密后的数据长度
*@Return:		0:成功; <0:失败
*/
int gmSm2_lib(unsigned char * Key,  
   unsigned int KeyLen, 
   unsigned char *Input,  
   unsigned int InputLen,  
   unsigned char *Output,  
   unsigned int *OutputLen,  
   unsigned char Mode)
{
	int output_len = 0;
	int key_len = KeyLen;
	int input_len = InputLen;
	if(((KeyLen != 64) && (KeyLen != 96)) || (InputLen <=0) || ((Mode != 0) && (Mode != 1)))
	{
		return ERR_GM_PARAM;
	}

	int cmd_len = 6 + 1 + key_len + 2 + input_len + 1;
	unsigned char sm2_cmd_head[6] = {0x00, 0xe5, 0x02, 0x00, cmd_len-6, (cmd_len -6) >> 8};
	unsigned char* sm2_cmd = (unsigned char*) fibo_malloc(cmd_len);
	memcpy(sm2_cmd, sm2_cmd_head, sizeof(sm2_cmd_head));
	*(sm2_cmd + 6) = key_len;
	memcpy(sm2_cmd + 7, Key, key_len);
	*(sm2_cmd + 7 + key_len) = input_len;
	*(sm2_cmd + 7 + key_len + 1) = input_len >> 8;
	memcpy(sm2_cmd + 7 + key_len + 2, Input, input_len);
	*(sm2_cmd + 7 + key_len + 2 + input_len) = Mode;
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(sm2_cmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(sm2_cmd,&frm,0x40, cmd_len,0x01,0x00);
	fibo_free(sm2_cmd);
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
		memcpy(Output, retfrm.data + 6, output_len);
	    *OutputLen = output_len;
		iRet = 0;
	}
	else if(iRet == 0xa003)
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
	sysLOG(API_LOG_LEVEL_4, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
	return iRet;

}

/*
*@Brief: 	    使用 SM2 私钥签名数据，获得签名信息
*@Param IN:	    *user_id  [输入] 签名者信息 
				userid_len [输入] 签名者信息长度，长度最长为 32 字节 
				*public_key [输入] 公钥数据缓存指针，公钥长度为 64 字节 
				*private_Key[输入] 私钥数据缓存指针。私钥长度为 32 字节 
				*msg [输入] 待签名数据 
				msg_len [输入] 待签名数据长度，任意长度
*@Param OUT:    *sign [输出]  签名信息缓存指针，空间大小至少为 64 字节 
*@Return:	   0:成功; <0:失败
*/
int gmSm2Sign_lib(unsigned char *user_id,  
   int userid_len,	
   const unsigned char *public_key, 
   unsigned char *private_Key,	
   unsigned char *msg, 
   int msg_len, 
   unsigned char *sign)
{
   if((userid_len > 32) || (userid_len <= 0) || (msg_len <= 0))
   {
	   return ERR_GM_PARAM;
   }
   int cmd_len = 6 + 1 + userid_len + 1 + 64 + 1 + 32 + 2 + msg_len;
   unsigned char sm2_sign_cmd_head[] = {0x00, 0xe5, 0x03, 0x00, cmd_len-6, (cmd_len-6) >> 8};
   unsigned char* sm2_sign_cmd = (unsigned char*) fibo_malloc(cmd_len + 1);
   memcpy(sm2_sign_cmd, sm2_sign_cmd_head, sizeof(sm2_sign_cmd_head));
   *(sm2_sign_cmd + 6) = userid_len;
   memcpy(sm2_sign_cmd + 7, user_id, userid_len);
   *(sm2_sign_cmd + 7 + userid_len) = 0x40;
   memcpy(sm2_sign_cmd + 7 + userid_len + 1, public_key, 64);
   *(sm2_sign_cmd + 7 + userid_len + 65) = 0x20;
   memcpy(sm2_sign_cmd + 7 + userid_len + 66, private_Key, 32);
   *(sm2_sign_cmd + 7 + userid_len + 98) = msg_len;
   *(sm2_sign_cmd + 7 + userid_len + 99) = (msg_len >> 8);
   memcpy(sm2_sign_cmd + 7 + userid_len + 100, msg, msg_len);
#ifdef PRINT_API_CMD
   char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
   HexToStr(sm2_sign_cmd, cmd_len, caShow);
   sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif	
   int iRet=0;
   Frame frm,retfrm;
   iRet = frameFactory(sm2_sign_cmd,&frm,0x40, cmd_len,0x01,0x00);
   fibo_free(sm2_sign_cmd);
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
	   memcpy(sign,retfrm.data + 6, 64);
	   iRet = 0;
   }else {
	   iRet = -iRet;
   }
   fibo_free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
   return iRet;
}
   
/*
*@Brief: 	   使用 SM2 公钥验证签名
*@Param IN:	   *user_id  [输入] 签名者信息 
			   userid_len [输入] 签名者信息长度，长度最长为 32 字节 
			   *public_key [输入] 公钥数据缓存指针，公钥长度为 64 字节 
			   *signed_data[输入] 签名信息
			   *msg [输入] 待签名数据 
			   msg_len [输入] 待签名数据长度，任意长度
*@Param OUT:   
*@Return:	  0:成功; <0:失败
*/
int gmSm2Verify_lib(unsigned char *user_id,	
	int userid_len,  
	const unsigned char *public_key, 
	unsigned char *signed_data,  
	unsigned char *msg, 
	int msg_len)
{
   if((userid_len > 32) || (msg_len <= 0))
   {
	   return ERR_GM_PARAM;
   }

   int cmd_len = 6 + 1 + userid_len + 2 + 128 + 2 + msg_len;
   unsigned char sm2_verify_cmd_head[] = {0x00, 0xe5, 0x04, 0x00, cmd_len-6, (cmd_len-6) >> 8};
   unsigned char* sm2_verify_cmd = (unsigned char*) fibo_malloc(cmd_len);
   memcpy(sm2_verify_cmd, sm2_verify_cmd_head, sizeof(sm2_verify_cmd_head));
   *(sm2_verify_cmd + 6) = userid_len;
   memcpy(sm2_verify_cmd + 7, user_id, userid_len);
   *(sm2_verify_cmd + 7 + userid_len) = 0x40;
   memcpy(sm2_verify_cmd + 7 + userid_len + 1, public_key, 64);
   *(sm2_verify_cmd + 7 + userid_len + 65) = 0x40;
   memcpy(sm2_verify_cmd + 7 + userid_len + 66, signed_data, 64);
   *(sm2_verify_cmd + 7 + userid_len + 130) = msg_len;
   *(sm2_verify_cmd + 7 + userid_len + 131) = (msg_len >> 8);
   memcpy(sm2_verify_cmd + 7 + userid_len + 132, msg, msg_len);
#ifdef PRINT_API_CMD
   char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);   
   HexToStr(sm2_verify_cmd, cmd_len, caShow);
   sysLOG(API_LOG_LEVEL_4, "ucCmd = %s\r\n", caShow);
   fibo_free(caShow);
#endif

   int iRet=0;
   Frame frm,retfrm;
   iRet = frameFactory(sm2_verify_cmd,&frm,0x40, cmd_len,0x01,0x00);
   fibo_free(sm2_verify_cmd);
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
	   iRet = 0;
   }else {
	   iRet = -iRet;
   }
   fibo_free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
   return iRet;
}

   

/*
*@Brief: 	    导出密钥对
*@Param IN:	    bPara  [输入] bPara = 1 获取私钥 ,bPara = 2 获取公钥的 px ,bPara = 3 获取公钥的 py  
*@Param OUT:    *sOutput [输出] 获取的密钥 
*@Return:	  0:成功; <0:失败
*/
int gmSm2ExportPk_lib(uint8_t bPara,uint8_t *sOutput)
{
	if((bPara > 3) || (bPara <= 0))
	{
		return ERR_GM_PARAM;
	}

    unsigned char sm2_exportpk_cmd[] = {0x00, 0xe5, 0x05, 0x00, 0x01, 0x00, bPara};
    int output_len = 0;

    int iRet=0;
    Frame frm,retfrm;
    iRet = frameFactory(sm2_exportpk_cmd,&frm,0x40, sizeof(sm2_exportpk_cmd),0x01,0x00);
    if(iRet < 0) {
        goto RET_END;
    }
    iRet = transceiveFrame(frm, &retfrm, 1000);
	free(frm.data);
    if(iRet <0) {
        goto RET_END;
    }
    iRet=retfrm.data[2]<<8 | retfrm.data[3];
    if(0x9000 == iRet) {
        output_len = retfrm.data[5]<<8 | retfrm.data[4];
        memcpy(sOutput,retfrm.data + 6, output_len);
        iRet = 0;
    }else {
        iRet = -iRet;
    }
   free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "	RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
   return iRet;

}

/*
*@Brief: 	    使用 SM3hash 算法获得 32 bytes hash 值。
*@Param IN:	    *input  [输入] 待计算的数据 
				input_len [输入] 待计算的数据长度  
*@Param OUT:    *output  [输出] 32 字节的 HASH 值 
*@Return:	  0:成功; <0:失败
*/
int gmSm3_lib(unsigned char *input, unsigned int  input_len, unsigned char *output)
{
	if((input_len <= 0) || (input_len > 1024))
	{
		return ERR_GM_PARAM;
	}

	int output_len = 0;

	int cmd_len = 6 + 2 + input_len;
	unsigned char sm3_cmd_head[6] = {0x00, 0xe5, 0x06, 0x00, cmd_len-6, (cmd_len-6) >> 8};
	unsigned char* sm3_cmd = (unsigned char*) fibo_malloc(cmd_len);
	memcpy(sm3_cmd, sm3_cmd_head, sizeof(sm3_cmd_head));
	*(sm3_cmd + 6) = input_len;
	*(sm3_cmd + 7) = input_len >> 8;
	memcpy(sm3_cmd + 8, input, input_len);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
	HexToStr(sm3_cmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(sm3_cmd,&frm,0x40,cmd_len,0x01,0x00);
	fibo_free(sm3_cmd);
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
		memcpy(output,retfrm.data + 6, output_len);
		iRet = 0;
	}else {
		iRet = -iRet;
	}
    fibo_free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "	RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
   return iRet;

}

/*
*@Brief: 	    Sm4 加解密运算
*@Param IN:	    *input  [输入] 待计算的数据 
				input_len [输入] 待计算的数据长度
				*smkey  [输入] 密钥数据 
				*vector  [输入] 初始化向量，大小为 16 字节； 对于 ECB 可以为 NULL。 
				mode  [输入] 模式： 0x00 ECB 解密 0x01 ECB 加密 0x02 CBC 解密 0x03 CBC 加密 	
*@Param OUT:    *output  [输出] 加密或解密后的数据
*@Return:	  0:成功; <0:失败
*/
int gmSm4_lib(unsigned char *input, unsigned int  input_len, unsigned char *output, unsigned char *smkey, unsigned char *vector, unsigned char mode) 
{
	if(input_len <= 0 || input_len > 4096 || mode > 0x03 || mode < 0x00)
	{
		return ERR_GM_PARAM;
	}
	if(vector == NULL && (mode == 0x03 || mode == 0x02))
	{
		return ERR_GM_PARAM;
	}

	int output_len = 0;

	int cmd_len = 6 + 2 + input_len + 2 + 16 + 1 + 16 + 1;
	if(vector == NULL) {
		cmd_len -= 16;
	}

	unsigned char sm4_cmd_head[6] = {0x00, 0xe5, 0x07, 0x00, cmd_len-6, (cmd_len-6) >> 8};
	unsigned char* sm4_cmd = (unsigned char*) fibo_malloc(cmd_len);
	memcpy(sm4_cmd, sm4_cmd_head, sizeof(sm4_cmd_head));
	*(sm4_cmd + 6) = input_len;
	*(sm4_cmd + 7) = input_len >> 8;
	memcpy(sm4_cmd + 8, input, input_len);
	*(sm4_cmd + 8 + input_len) = 0x10;
	*(sm4_cmd + 8 + input_len + 1) = 0x00;
	memcpy(sm4_cmd + 8 + input_len + 2, smkey, 16);
	if(vector != NULL) {
		*(sm4_cmd + 8 + input_len + 18) = 0x10;
		memcpy(sm4_cmd + 8 + input_len + 19, vector, 16);
		*(sm4_cmd + 8 + input_len + 35) = mode;
	}else {
		*(sm4_cmd + 8 + input_len + 18) = 0x00;
		*(sm4_cmd + 8 + input_len + 19) = mode;
	}
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);	
	HexToStr(sm4_cmd, cmd_len, caShow);
	sysLOG(API_LOG_LEVEL_4, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	int iRet=0;
	Frame frm,retfrm;
	iRet = frameFactory(sm4_cmd,&frm,0x40,cmd_len,0x01,0x00);
	fibo_free(sm4_cmd);
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
		memcpy(output,retfrm.data + 6, output_len);
		iRet = 0;
	}
	else if(iRet == 0xa006)
    {
        if(retfrm.length >= 10)
        {
            iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
        }
    }else {
		iRet = -iRet;
	}
    fibo_free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "	RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
   return iRet;
}

/*
*@Brief: 	    使用 SM2 私钥，对输入的 E 值，进行签名。
*@Param IN:	    *E[输入] SM2 签名与验签算法中的 E 值。 
				PrivateKey[输入] SM2 的私钥 
*@Param OUT:    *Sign[输出] SM2 签名后的数据
*@Return:	  0:成功; <0:失败
*/
int gmSm2SignWithE_lib(unsigned char *E, unsigned char *PrivateKey, unsigned char *Sign)
{
   int cmd_len = 6 + 1 + 32 + 1 + 32;
   unsigned char sm2_sign_cmd_head[] = {0x00, 0xe5, 0x03, 0x01, cmd_len-6, (cmd_len-6) >> 8};
   unsigned char* sm2_sign_cmd = (unsigned char*) fibo_malloc(cmd_len + 1);
   memcpy(sm2_sign_cmd, sm2_sign_cmd_head, sizeof(sm2_sign_cmd_head));
   *(sm2_sign_cmd + 6) = 32;
   memcpy(sm2_sign_cmd + 7, E, 32);
   *(sm2_sign_cmd + 7 + 32) = 32;
   memcpy(sm2_sign_cmd + 7 + 32 + 1, PrivateKey, 32);
#ifdef PRINT_API_CMD
   char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);
   HexToStr(sm2_sign_cmd, cmd_len, caShow);
   sysLOG(API_LOG_LEVEL_4, "	ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif	
   int iRet=0;
   Frame frm,retfrm;
   iRet = frameFactory(sm2_sign_cmd,&frm,0x40, cmd_len,0x01,0x00);
   fibo_free(sm2_sign_cmd);
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
	   memcpy(Sign,retfrm.data + 6, 64);
	   iRet = 0;
   }else {
	   iRet = -iRet;
   }
   fibo_free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "	RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
   return iRet;

}


/*
*@Brief: 	    使用 SM2 公钥及 E 值，对签名数据进行验签 
*@Param IN:	    *E[输入] SM2 签名与验签算法中的 E 值。 
				*Sign [输入] SM2 的签名数据
				*PublicKey [输入] SM2 的公钥
*@Param OUT:    
*@Return:	  0:成功; <0:失败
*/
int gmSm2VerifyWithE_lib(unsigned char *E,	unsigned char *Sign, unsigned char *PublicKey)
{
   int cmd_len = 6 + 1 + 32 + 1 + 64 + 1 + 64;
   unsigned char sm2_verify_cmd_head[] = {0x00, 0xe5, 0x04, 0x01, cmd_len-6, (cmd_len-6) >> 8};
   unsigned char* sm2_verify_cmd = (unsigned char*) fibo_malloc(cmd_len);
   memcpy(sm2_verify_cmd, sm2_verify_cmd_head, sizeof(sm2_verify_cmd_head));
   *(sm2_verify_cmd + 6) = 32;
   memcpy(sm2_verify_cmd + 7, E, 32);
   *(sm2_verify_cmd + 7 + 32) = 64;
   memcpy(sm2_verify_cmd + 7 + 32 + 1, PublicKey, 64);
   *(sm2_verify_cmd + 7 + 32 + 65) = 64;
   memcpy(sm2_verify_cmd + 7 + 32 + 66, Sign, 64);
#ifdef PRINT_API_CMD
    char* caShow = (char*) fibo_malloc(cmd_len * 2 + 1);   
   HexToStr(sm2_verify_cmd, cmd_len, caShow);
   sysLOG(API_LOG_LEVEL_4, "	ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

   int iRet=0;
   Frame frm,retfrm;
   iRet = frameFactory(sm2_verify_cmd,&frm,0x40, cmd_len,0x01,0x00);
   fibo_free(sm2_verify_cmd);
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
	   iRet = 0;
   }else {
	   iRet = -iRet;
   }
   fibo_free(retfrm.data);
RET_END:
   sysLOG(API_LOG_LEVEL_4, "	RET_END,iRet = %d, iRet = 0x%x\r\n", iRet, iRet);
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

#define ErrFlag_True		0x00
#define ErrFlag_False		0x01


unsigned int G_Case14Fun_Len;
unsigned int G_Case14FunID_Len;
uchar OutPut[2050],VerData[2050];
uchar t_OutPut[2050],t_VerData[2050];

//GmSm2Test
int GmSm2Test1(char *PulKey,char PulKeyLen,char *PrvKey,char PrvKeyLen,char *InputData,int DataLen,int  *OutPutLen,char Mode1,char Mode2,int PreRet )

{
	int Ret = 255;
  int ErrFlag = ErrFlag_False;
	memset(OutPut,	0x00,sizeof(OutPut));
	memset(VerData,	0x00, sizeof(VerData));
	memset(t_OutPut,	0x00,sizeof(t_OutPut));
	memset(t_VerData,	0x00, sizeof(t_VerData));
	
	//PortOpen(0,"115200,8,n,1");
	Ret = gmSm2Init_lib(NULL);//相当于打开国密
  OSI_LOGI(0,"Case_14:gmSm2Init_lib: Ret = %d",Ret);
	if(Ret == 0)
	{
		//公钥加密数据
		Ret = gmSm2_lib(PulKey, PulKeyLen, InputData , DataLen ,OutPut,OutPutLen, Mode1);
		//DispScrPrint(0, LINE3, G_FontSet, FDISP|CDISP, "Ret = %d",Ret);
    	OSI_LOGI(0,"Case_14:PulKey:Ret=%d",Ret);
		if(Ret == 0)
	    {
	    // DispScrPrint(0, LINE4, G_FontSet, FDISP|CDISP, "PubKey Enc Succ");
	   	 OSI_LOGI(0,"Case_14:PubKey Enc Succ");
	    }
		else 
			OSI_LOGI(0,"Case_14:PubKey Enc faild");
		BcdToAsc(t_OutPut, OutPut, (*OutPutLen)*2);

   		gmSm2Init_lib(NULL);
		//私钥解密数据
		Ret = gmSm2_lib(PrvKey, PrvKeyLen, OutPut, *OutPutLen, VerData, &DataLen, Mode2);
    	OSI_LOGI(0,"Case_14:PrvKey: Ret=%d",Ret);
		if(Ret == 0)
		{
			Ret = memcmp(VerData,InputData,DataLen);
			if(Ret == 0) 
      OSI_LOGI(0,"Case_14:PriKey Dec Succ");
     // DispScrPrint(0, LINE4, G_FontSet, FDISP|CDISP, "PriKey Dec Succ");
			else  
      OSI_LOGI(0,"Case_14:PrvKey:Ret=%d",Ret);
      //GmMessage(Ret, "", LINE3);
		}
		//else  
    //GmMessage(Ret, "", LINE3);
    OSI_LOGI(0,"Case_14: Ret=%d",Ret);
	}
	else  //GmMessage(Ret, "", LINE3);
  OSI_LOGI(0,"Case_14: finish: Ret=%d",Ret);
	if(Ret == PreRet) ErrFlag = ErrFlag_True;
	return ErrFlag;
}

int Case_1404007(void)
{
	char InputData[70],PulKey[200],PrvKey[200];
	int DataLen = 32,OutPutLen = 0;
	int Ret = 255;
	
	
	memset(InputData,	0x00,sizeof(InputData));
	memset(PrvKey,	0x00, sizeof(PrvKey));
	memset(PulKey,	0x00, sizeof(PulKey));

	AscToBcd(InputData,"1111111111111111111111111111111111111111111111111111111111111112", 64);
	AscToBcd(PulKey,"ec91818de0b7012173f51c3375436e43b6a9a26abd6dbcb79f851cdeaf7a0f6ccbf4b5a15fb87e60fc0b3a923d12e866364a935ffb30842bc9139ebd2ddce961",128);
	AscToBcd(PrvKey,"ec91818de0b7012173f51c3375436e43b6a9a26abd6dbcb79f851cdeaf7a0f6ccbf4b5a15fb87e60fc0b3a923d12e866364a935ffb30842bc9139ebd2ddce961c56a2b58a094ef2441037945bab1398cc0df9fc4f99e9a602cd86fc2c388ad", 190);
	
	return GmSm2Test1(PulKey,64,PrvKey,95,InputData,DataLen,&OutPutLen,0x01,0x00,ERR_GM_STATUS_WORD_LENTH_ERROR);
}

void gmSm2Test()
{
    Case_1404007();
	return;
	
	unsigned char baRandom[100] = {0};
	unsigned char InputData[40],PulKey[200],PrvKey[200], OutData[200], user_id[20], msg[100], sign[200], Preresult[200], SmKey[100], vector[100];
	int DataLen = 16,OutPutLen = 0;
	int iRet = 0;
	memset(InputData,	0x00,sizeof(InputData));
	memset(PrvKey,	0x00, sizeof(PrvKey));
	memset(PulKey,	0x00, sizeof(PulKey));
	memset(OutData,	0x00, sizeof(OutData));
	memset(user_id,	0x00, sizeof(user_id));
	memset(msg,	0x00, sizeof(msg));
	memset(sign,	0x00, sizeof(sign));

	char* caShow = (char*) fibo_malloc(1024 * 2 + 1);

	sysGetRandom_lib(8, baRandom);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(baRandom, 8, caShow);
	sysLOG(API_LOG_LEVEL_2, " sysGetRandom_lib baRandom = %s\r\n", caShow);
	
	gmSm2Init_lib(NULL);

	AscToBcd(InputData,"11111111111111111111111111111111",32);
	//AscToBcd(PulKey,"ec91818de0b7012173f51c3375436e43b6a9a26abd6dbcb79f851cdeaf7a0f6ccbf4b5a15fb87e60fc0b3a923d12e866364a935ffb30842bc9139ebd2ddce961",128);
	//AscToBcd(PrvKey,"ec91818de0b7012173f51c3375436e43b6a9a26abd6dbcb79f851cdeaf7a0f6ccbf4b5a15fb87e60fc0b3a923d12e866364a935ffb30842bc9139ebd2ddce961c56a2b58a094ef2441037945bab1398cc0df9fc4f99e9a602cd86fc2c388ad0c", 192);

	memset(OutData, 0x00, sizeof(OutData));
	iRet = gmSm2ExportPk_lib(2, OutData);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 32, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm2ExportPk_lib pubx = %s\r\n", caShow);
    memcpy(PulKey, OutData, 32);
	
	memset(OutData, 0x00, sizeof(OutData));
	iRet = gmSm2ExportPk_lib(3, OutData);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 32, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm2ExportPk_lib puby = %s\r\n", caShow);
	memcpy(PulKey+32, OutData, 32);

	memset(OutData, 0x00, sizeof(OutData));
	iRet = gmSm2ExportPk_lib(1, OutData);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 32, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm2ExportPk_lib prv = %s\r\n", caShow);
	memcpy(PrvKey, PulKey, 64);
	memcpy(PrvKey + 64, OutData, 32);

	memset(caShow, 0, sizeof(caShow));
	HexToStr(PulKey, 64, caShow);
	sysLOG(API_LOG_LEVEL_2, " PulKey = %s\r\n", caShow);

	iRet = gmSm2_lib(PulKey, 64, InputData, 16, OutData, &OutPutLen, 1);
	if(iRet == 0)
	{
	    memset(caShow, 0, sizeof(caShow));
	    HexToStr(OutData, OutPutLen, caShow);
	    sysLOG(API_LOG_LEVEL_2, " gmSm2_lib 1 OutData = %s\r\n", caShow);

		memset(caShow, 0, sizeof(caShow));
	    HexToStr(PrvKey, 96, caShow);
	    sysLOG(API_LOG_LEVEL_2, " PrvKey = %s\r\n", caShow);
		gmSm2_lib(PrvKey, 96, OutData, OutPutLen, OutData, &OutPutLen, 0);
		memset(caShow, 0, sizeof(caShow));
	    HexToStr(OutData, OutPutLen, caShow);
	    sysLOG(API_LOG_LEVEL_2, " gmSm2_lib 0 OutData = %s\r\n", caShow);
	}

	memcpy(user_id,	"\x11",1);
	memcpy(msg, "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x12",32);

	iRet = gmSm2Sign_lib(user_id,  1, PulKey, PrvKey + 64, msg, 32, sign);
	memset(caShow, 0, sizeof(caShow));
    HexToStr(sign, 64, caShow);
    sysLOG(API_LOG_LEVEL_2, " gmSm2Sign_lib sign = %s\r\n", caShow);

	iRet = gmSm2Verify_lib(user_id,  1, PulKey, sign, msg, 32);
	if(iRet == 0)
	{
	    sysLOG(API_LOG_LEVEL_2, " gmSm2Verify_lib ok\r\n");
	}
	else
	{
	    sysLOG(API_LOG_LEVEL_2, " gmSm2Verify_lib faild\r\n");
	}

	memset(InputData,	0x00,sizeof(InputData));
	memset(OutData,	0x00, sizeof(OutData));
	AscToBcd(InputData,"F1BC1DFF9B4C9268D5FD31FF72FD07040527267CC33F7BCFCAE366848F8FA7EF53F4324E2C375D6D3C49F8071F1729227D7D994DEE32293C8D6FFA6A10F73CAC", 128);	
	AscToBcd(Preresult,"700A86D08F86618902A10FBE2821888ECAF7632932DE0C72E9CADAE9AE901ACA",64);
	iRet = gmSm3_lib(InputData, 64, OutData);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 32, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm3 OutData = %s\r\n", caShow);
	if(memcmp(OutData,Preresult,32)==0)
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm3 ok\r\n");
	}
	else
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm3 faild\r\n");
	}

	memset(InputData,	0x00,sizeof(InputData));
	memset(OutData, 0x00, sizeof(OutData));
	memset(SmKey,	0x00, sizeof(SmKey));
	memcpy(InputData,"\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef",16);
	memcpy(SmKey,"\x01\x23\x45\x67\x89\xab\xcd\xef\x01\x23\x45\x67\x89\xab\xcd\xef",16);
	memcpy(Preresult,"\xF5\x7A\x93\x36\xB5\x07\xC3\x1C\xDD\xD3\x9A\x66\xAA\x99\xF6\xC2",16);
	/*
	*@Brief:		Sm4 加解密运算
	*@Param IN: 	*input	[输入] 待计算的数据 
					input_len [输入] 待计算的数据长度
					*smkey	[输入] 密钥数据 
					*vector  [输入] 初始化向量，大小为 16 字节； 对于 ECB 可以为 NULL。 
					mode  [输入] 模式： 0x00 ECB 解密 0x01 ECB 加密 0x02 CBC 解密 0x03 CBC 加密	
	*@Param OUT:	*output  [输出] 加密或解密后的数据
	*@Return:	  0:成功; <0:失败
	*/
	//0x01 ECB 加密
	iRet = gmSm4_lib(InputData, 16, OutData, SmKey, NULL, 1); 
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 16, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm4 1 OutData = %s\r\n", caShow);
	if(memcmp(OutData,Preresult,16)==0)
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 1 ok\r\n");
	}
	else
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 1 faild\r\n");
	}
	//0x00 ECB 解密
	iRet = gmSm4_lib(OutData, 16, OutData, SmKey, NULL, 0); 
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 16, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm4 0 OutData = %s\r\n", caShow);
	if(memcmp(OutData,InputData,16)==0)
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 0 ok\r\n");
	}
	else
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 0 faild\r\n");
	}

	memset(InputData,	0x00,sizeof(InputData));
	memset(OutData, 0x00, sizeof(OutData));
	memset(SmKey,	0x00, sizeof(SmKey));
	memset(vector,	0x00, sizeof(vector));

	AscToBcd(InputData,"A3FBE4D1D76E6804EBC297FA25AAE007",32);
	AscToBcd(SmKey,"FE19B6CF10F034A70CE2FBE59B2F090E", 32);
	AscToBcd(Preresult,"9D90BCD83744333251330F0D281680C6",32);
	AscToBcd(vector,"62E521F2572F77EA5B0BBC1A36B42016",32);

	//0x03 CBC 加密
	iRet = gmSm4_lib(InputData, 16, OutData, SmKey, vector, 3); 
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 16, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm4 3 OutData = %s\r\n", caShow);
	if(memcmp(OutData,Preresult,16)==0)
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 3 ok\r\n");
	}
	else
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 3 faild\r\n");
	}
	//0x02 CBC 解密
	iRet = gmSm4_lib(OutData, 16, OutData, SmKey, vector, 2); 
	memset(caShow, 0, sizeof(caShow));
	HexToStr(OutData, 16, caShow);
	sysLOG(API_LOG_LEVEL_2, " gmSm4 2 OutData = %s\r\n", caShow);
	if(memcmp(OutData,InputData,16)==0)
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 2 ok\r\n");
	}
	else
	{
		sysLOG(API_LOG_LEVEL_2, " gmSm4 2 faild\r\n");
	}

    fibo_free(caShow);
}
#endif
