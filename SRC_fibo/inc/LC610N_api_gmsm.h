#ifndef LC610N_API_GMSM_H
#define LC610N_API_GMSM_H


#include "comm.h"

//char caShow[2048];

#define API_LOG_LEVEL_0		LOG_LEVEL_0
#define API_LOG_LEVEL_1		LOG_LEVEL_1
#define API_LOG_LEVEL_2		LOG_LEVEL_2
#define API_LOG_LEVEL_3		LOG_LEVEL_3
#define API_LOG_LEVEL_4		LOG_LEVEL_4
#define API_LOG_LEVEL_5		LOG_LEVEL_5



#define ERR_GM_INIT -1500 //模块初始化失败 
#define ERR_GM_NOINIT -1501 //模块未初始化 
#define ERR_GM_STATE -1502 //模块状态错误 
#define ERR_GM_COMM -1503 //与模块通讯失败 
#define ERR_GM_PARAM -1504 //参数不正确 
#define ERR_GM_BUSY -1505 //设备忙 
#define ERR_GM_NOT_PRESENT -1506 //设备不存在 
#define ERR_GM_LRC -1507 //LRC 校验错 
#define ERR_GM_STATUS_WORD_HAVE_RETURN_DATA -1508 //还有字节需要返回 
#define ERR_GM_STATUS_WORD_LENTH_ERROR -1509 //长度错误 
#define ERR_GM_STATUS_WORD_RETURN_DATA_ERROR -1510 //返回数据错误 
#define ERR_GM_STATUS_WORD_LE_OR_LC_ERROR -1511 //Le 或者 LC 错误 
#define ERR_GM_STATUS_WORD_P1_OR_P2_ERROR -1512 //P1 P2 错误 
#define ERR_GM_STATUS_WORD_LRC_ERROR -1513 //LRC 校验失败 
#define ERR_GM_STATUS_WORD_OTHER_ERROR -1514 //其他错误 
#define ERR_GM_SENDAPDU_TIMEOUT -1515 //发送命令超时 
#define ERR_GM_INQUIRE_TIMEOUT -1516 //查询引脚状态超时 
#define ERR_GM_GETRESPONSE_TIMEOUT -1517 //获取应答超时 
#define ERR_GM_SM2_ENC -1518 //SM2 加密失败 
#define ERR_GM_SM2_DEC -1519 //SM2 解密失败 
#define ERR_GM_SM2_SIGN -1520 //SM2 签名失败 
#define ERR_GM_SM2_VERIFY -1521 //SM2 验签失败 
#define ERR_GM_SM2_GEN -1522 //SM2 生成密钥失败 
#define ERR_GM_SM3_RESULT -1523 //SM3 计算出错 

/*
*@Brief:		国密模块上电以及初始化
*@Param IN:		*params: 曲线参数缓冲指针,至少 128 字节。
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int gmSm2Init_lib(unsigned char *params);
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
   unsigned char Mode);
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
  unsigned char *sign);
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
	int msg_len);
/*
*@Brief: 	    导出密钥对
*@Param IN:	    bPara  [输入] bPara = 1 获取私钥 ,bPara = 2 获取公钥的 px ,bPara = 3 获取公钥的 py  
*@Param OUT:    *sOutput [输出] 获取的密钥 
*@Return:	  0:成功; <0:失败
*/
int gmSm2ExportPk_lib(uint8_t bPara,uint8_t *sOutput);
/*
*@Brief: 	    使用 SM3hash 算法获得 32 bytes hash 值。
*@Param IN:	    *input  [输入] 待计算的数据 
				input_len [输入] 待计算的数据长度  
*@Param OUT:    *output  [输出] 32 字节的 HASH 值 
*@Return:	  0:成功; <0:失败
*/  
int gmSm3_lib(unsigned char *input, unsigned int  input_len, unsigned char *output);
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
int gmSm4_lib(unsigned char *input, unsigned int  input_len, unsigned char *output, unsigned char *smkey, unsigned char *vector, unsigned char mode);
/*
*@Brief: 	    使用 SM2 私钥，对输入的 E 值，进行签名。
*@Param IN:	    *E[输入] SM2 签名与验签算法中的 E 值。 
				PrivateKey[输入] SM2 的私钥 
*@Param OUT:    *Sign[输出] SM2 签名后的数据
*@Return:	  0:成功; <0:失败
*/
int gmSm2SignWithE_lib(unsigned char *E, unsigned char *PrivateKey, unsigned char *Sign);
/*
*@Brief: 	    使用 SM2 公钥及 E 值，对签名数据进行验签 
*@Param IN:	    *E[输入] SM2 签名与验签算法中的 E 值。 
				*Sign [输入] SM2 的签名数据
				*PublicKey [输入] SM2 的公钥
*@Param OUT:    
*@Return:	  0:成功; <0:失败
*/
int gmSm2VerifyWithE_lib(unsigned char *E,	unsigned char *Sign, unsigned char *PublicKey);

#endif
