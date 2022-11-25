#ifndef LC610N_API_CALC_H
#define LC610N_API_CALC_H


#include "comm.h"



//SHA_TYPE 数值表 
#define SHA_TYPE_160 0 //SHA1-160 
#define SHA_TYPE_224 1 //SHA2-224 
#define SHA_TYPE_256 2 //SHA2-256 
#define SHA_TYPE_384 3 //SHA2-384 
#define SHA_TYPE_512 4 //SHA2-512 


//加解密运算的错误代码范围为-1000  ~ -1499。 
#define ERR_CRYPTO_NOTSUPPORT -1000 //不支持该功能 
#define ERR_CRYPTO_MODE -1001 //模式错误 
#define ERR_CRYPTO_LENGTH -1002 //长度错误     
#define RSA_RET_MODULUS_LENGTH_ERR -1018 //RSA 运算时模长错误，为 0 或者过大 
#define RSA_RET_EXP_LENGTH_ERR -1019 //RSA 运算时指数长度错误，为 0 或者过大 
#define RSA_RET_EXP_MODULUS_LENGTH_ERR -1020 //RSA 运算时指数长度大于模长度， 错误 
#define RSA_RET_DATA_ERR -1021 //RSA 运算时输入数据大于等于模 数，错误 
#define ERR_CRYPTO_INVALID_PARAMS -1022 //参数错误 
#define ERR_CRYPTO_DATA -1023 //RSA 签名与验签数据错误 
#define ERR_CRYPTO_LEN -1024 //RSA 签名与验签长度错误 
#define ERR_CRYPTO_E_TYPE -1025 //E 模式错误

#define ERR_CRYPTO_ERR -1499 //未知错误

#if 0
//RSA 密钥结构: 
//公钥结构
 typedef struct { 
    unsigned short int bits;                   /* length in bits of modulus */ 
    unsigned char modulus[MAX_RSA_MODULUS_LEN];     /* modulus */ 
    unsigned char exponent[MAX_RSA_MODULUS_LEN];    /* public exponent */ 
} R_RSA_PUBLIC_KEY;

//私钥结构
 typedef struct { 
    unsigned short int bits;                    /* length in bits of modulus */ 
	unsigned char modulus[MAX_RSA_MODULUS_LEN];     /* modulus */ 
	unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent */     
	unsigned char exponent[MAX_RSA_MODULUS_LEN];   /* private exponent */       
	unsigned char prime[2][MAX_RSA_PRIME_LEN];     /* prime factors */ 
	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];     /* exponents for CRT */     
	unsigned char coefficient[MAX_RSA_PRIME_LEN];   /* CRT coefficient */ 
} R_RSA_PRIVATE_KEY; 
#endif

#define MH_MIN_RSA_MODULUS_BITS         1024
#define MH_MAX_RSA_MODULUS_BITS         2048
#define MH_MAX_RSA_MODULUS_BYTES      ((MH_MAX_RSA_MODULUS_BITS + 7) / 8)
#define MH_MAX_RSA_PRIME_BITS            ((MH_MAX_RSA_MODULUS_BITS + 1) / 2)
#define MH_MAX_RSA_PRIME_BYTES          ((MH_MAX_RSA_PRIME_BITS + 7) / 8)

typedef struct
{
    uint32_t bytes;                         //字节长度
    uint8_t e[4];                           //公钥指数
    uint8_t n[MH_MAX_RSA_MODULUS_BYTES];    //模数n
    uint8_t n_c[MH_MAX_RSA_MODULUS_BYTES];  //模数n参数C
    uint8_t n_q[4];                         //模数n参数Q
}rsa_public_key_t;

typedef struct
{
    uint32_t bytes;                                //字节长度
    uint8_t e[4];                                  //公钥指数
    uint8_t p[MH_MAX_RSA_PRIME_BYTES];      //素数p
    uint8_t q[MH_MAX_RSA_PRIME_BYTES];      //素数q
    uint8_t n[MH_MAX_RSA_MODULUS_BYTES];   //模数n
    uint8_t d[MH_MAX_RSA_MODULUS_BYTES];   //私钥指数
    uint8_t dp[MH_MAX_RSA_PRIME_BYTES];      //d mod (p-1)
    uint8_t dq[MH_MAX_RSA_PRIME_BYTES];      //d mod (q-1)
    uint8_t qp[MH_MAX_RSA_PRIME_BYTES];      //q^-1 mod p 

    uint8_t n_c[MH_MAX_RSA_MODULUS_BYTES];  //模数n参数C
    uint8_t n_q[4];                                 //模数n参数Q
    uint8_t p_c[MH_MAX_RSA_PRIME_BYTES];    //素数p参数C
    uint8_t p_q[4];                                //素数p参数Q
    uint8_t q_c[MH_MAX_RSA_PRIME_BYTES];    //素数q参数C
    uint8_t q_q[4];                               //素数q参数Q
}rsa_private_key_t;


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
int calcDesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint8_t *IV, int mode); 

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
int calcDesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint8_t *IV, int mode);

/*
*@Brief:		3Des 加密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 16/24 字节 DES 密钥 此接口目前只支持24字节密钥
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcTdesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);

/*
*@Brief:		3Des 解密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 16/24 字节 DES 密钥 此接口目前只支持24字节密钥
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcTdesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);

/*
*@Brief:		Aes加密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 16/24 字节 DES 密钥 此接口目前只支持24字节密钥
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcAesEnc_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);

/*
*@Brief:		Aes解密运算
*@Param IN:		*input [输入] 输入数据 
				lenth [输入] 输入数据长度，8 字节倍数 
                *deskey [输入] 16/24 字节 DES 密钥 此接口目前只支持24字节密钥
				*IV [输入] 初始化向量，ECB IV 为 NULL。 
				mode [输入] 0－ECB;1－CBC 				
*@Param OUT:	output [输出] 输出数据 
*@Return:		0:成功; <0:失败
*/
int calcAesDec_lib(uint8_t *input,int lenth, uint8_t *output, uint8_t *deskey, uint32_t keyLen, uint8_t *IV, int mode);

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
int calcRsaRecover_lib(uchar *Modul, uint ModuleLen, uchar *Exp,  uint ExpLen, uchar *DataIn, uchar *DataOut);

/*
*@Brief:		RSA 生成密钥对
*@Param IN:		modulus_size [输入] 密钥位数,最大支持 2048 
				e_type [输入] E 位数，支持 3 或 65537. 
*@Param OUT:	*pbModulus [输出] 公钥模数。 
				*pdwPubExp [输出] 公钥指数。 
				*pbD [输出] 私钥指数 
*@Return:		0:成功; <0:失败
*/
int calcRsaKeyPairGen_lib (uint32_t modulus_size,  uint32_t e_type, uint8_t *pbModulus,  uint32_t *pdwPubExp, uint8_t *pbD); 

/*
*@Brief:		计算安全的 hash 值
*@Param IN:		*DataIn[输入] 输入数据  
				DataInLen[输入] 输入数据长度(以字节为单位) 
				Mode[输入] 0   SHA_TYPE_160  1   SHA_TYPE_224 2   SHA_TYPE_256 3   SHA_TYPE_384 4   SHA_TYPE_512 
*@Param OUT:	*DataOut  [输出] 输出数据缓冲区指针   
*@Return:		0:成功; <0:失败
*/
int calcSha_lib( const uchar*DataIn, int DataInLen, uchar* ShaOut, int Mode);
/*
*@Brief:		RSA 生成密钥对
*@Param IN:		exponent [输入] 公钥指数 支持 3 或 65537 
				nbits [输入]  RSA模数位数。受限于CPU的硬件算法库，目前只支持1024及2048位
*@Param OUT:	*key [输出] RSA私钥缓存。 
*@Return:		0:成功; <0:失败
*/
uint32_t calcRsaGenKey_lib(rsa_private_key_t *key, uint32_t exponent, uint32_t nbits);
/*
*@Brief:		RSA私钥运算，使用CRT算法。CRT为中国剩余定理，用于加快RSA的运算
*@Param IN:		*input[输入] 输入数据缓存  
				*key[输入] RSA私钥 由calcRsaGenKey_lib接口生成的key
*@Param OUT:	*output [输出] 输出数据缓冲区指针，长度与模长度同  
*@Return:		0:成功; <0:失败
*/
uint32_t calcRsaPrivateCrt_lib(uint8_t *output, uint8_t *input, rsa_private_key_t *key);

/*
*@Brief:		RSA公钥运算
*@Param IN:		*input[输入] 输入数据缓存  
				*key[输入] RSA公钥 结构体的前3个参数数据有效
*@Param OUT:	*output [输出] 输出数据缓冲区指针，长度与模长度同  
*@Return:		0:成功; <0:失败
*/
uint32_t calcRsaPublic_lib(uint8_t *output, uint8_t *input, rsa_public_key_t *key);

#endif
