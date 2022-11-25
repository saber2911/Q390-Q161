
#ifndef _RSA_H_
#define _RSA_H_

#include "comm.h"



extern R_RSA_PUBLIC_KEY g_stpuk;
// RSA��Կ�ṹ
/*
typedef struct 
{
	uint KeyBitLength;     // λ����
	uchar ModuleData[256]; // ��Կģ����
	uchar ExpData[256];    // ��Կָ��
}RSA_PUBLIC;
*/
// Public-Key���ܲ���
int RSAPublicDecrypt(unsigned char *output,         /* output block */
                         unsigned int *outputLen,       /* length of output block */
                         unsigned char *input,          /* input block */
                         unsigned int inputLen,         /* length of input block */
                         R_RSA_PUBLIC_KEY *publicKey);   /* RSA public key */

// Private-Key���ܲ���
int RSAPrivateEncrypt (unsigned char *output,         /* output block */
                           unsigned int *outputLen,        /* length of output block */
                           unsigned char *input,           /* input block */
                           unsigned int inputLen,          /* length of input block */
                           R_RSA_PRIVATE_KEY *privateKey);  /* RSA private key */

int RSARecover(unsigned char *Module, unsigned int ModuleLen, unsigned char *Exp,
				unsigned int ExpLen, unsigned char *DataIn, unsigned char *DataOut);

#endif

