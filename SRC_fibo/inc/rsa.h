
#ifndef _RSA_H_
#define _RSA_H_

#include "comm.h"



extern R_RSA_PUBLIC_KEY g_stpuk;
// RSA公钥结构
/*
typedef struct 
{
	uint KeyBitLength;     // 位长度
	uchar ModuleData[256]; // 公钥模数据
	uchar ExpData[256];    // 公钥指数
}RSA_PUBLIC;
*/
// Public-Key解密操作
int RSAPublicDecrypt(unsigned char *output,         /* output block */
                         unsigned int *outputLen,       /* length of output block */
                         unsigned char *input,          /* input block */
                         unsigned int inputLen,         /* length of input block */
                         R_RSA_PUBLIC_KEY *publicKey);   /* RSA public key */

// Private-Key加密操作
int RSAPrivateEncrypt (unsigned char *output,         /* output block */
                           unsigned int *outputLen,        /* length of output block */
                           unsigned char *input,           /* input block */
                           unsigned int inputLen,          /* length of input block */
                           R_RSA_PRIVATE_KEY *privateKey);  /* RSA private key */

int RSARecover(unsigned char *Module, unsigned int ModuleLen, unsigned char *Exp,
				unsigned int ExpLen, unsigned char *DataIn, unsigned char *DataOut);

#endif

