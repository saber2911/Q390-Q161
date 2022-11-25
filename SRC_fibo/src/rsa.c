/* RSA.C - RSA routines for RSAREF
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */

#include "comm.h"



//typedef struct {
//	unsigned short int bits;                     					/* length in bits of modulus */
//  	unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
//  	unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* public exponent *//
//} R_RSA_PUBLIC_KEY;

static int RSAPrivateBlock (unsigned char * output,unsigned int * outputLen, unsigned char * input,unsigned int inputLen,R_RSA_PRIVATE_KEY * privateKey);
static int RSAPublicBlock (uint8 * output, unsigned int * outputLen,uint8 * input, unsigned int inputLen, R_RSA_PUBLIC_KEY *publicKey);

int RSAPublicDecrypt(unsigned char *output,         /* output block */
                         unsigned int *outputLen,       /* length of output block */
                         unsigned char *input,          /* input block */
                         unsigned int inputLen,         /* length of input block */
                         R_RSA_PUBLIC_KEY *publicKey)   /* RSA public key */
{
	int status;
  	unsigned char pkcsBlock[MAX_RSA_MODULUS_LEN];
  	unsigned int i, modulusLen, pkcsBlockLen;

  	modulusLen = (publicKey->bits + 7) / 8;
  	if (inputLen > modulusLen)
    		return (RE_LEN);

  	if ((status = RSAPublicBlock(pkcsBlock, &pkcsBlockLen, input, inputLen, publicKey)))
    		return (status);

  	if (pkcsBlockLen != modulusLen)
    		return (RE_LEN);

  	i = 0;
  	*outputLen = modulusLen - i;

  	if (*outputLen > modulusLen)
    		return (RE_DATA);

  	R_memcpy ((uint8 *)output, (uint8 *)&pkcsBlock[i], *outputLen);

  	/* Zeroize potentially sensitive information.
   	 */
  	R_memset ((uint8 *)pkcsBlock, 0, sizeof (pkcsBlock));

  	return (0);
}

/* RSA private-key encryption, according to PKCS #1.
 */
int RSAPrivateEncrypt (unsigned char *output,         /* output block */
                          unsigned int *outputLen,        /* length of output block */
                          unsigned char *input,           /* input block */
                          unsigned int inputLen,          /* length of input block */
                          R_RSA_PRIVATE_KEY *privateKey)  /* RSA private key */
{
  	int status;
  	unsigned char pkcsBlock[MAX_RSA_MODULUS_LEN];
  	unsigned int i, modulusLen;

  	modulusLen = (privateKey->bits + 7) / 8;

  	if (inputLen  > modulusLen)
    		return (RE_LEN);

  	i = 0;
  	R_memcpy (pkcsBlock+i, (uint8 *)input, inputLen);

  	status = RSAPrivateBlock(output, outputLen, pkcsBlock, modulusLen, privateKey);

  	/* Zeroize potentially sensitive information.
   	  */
  	R_memset ((uint8 *)pkcsBlock, 0, sizeof (pkcsBlock));

  	return (status);
}

//unsigned char *output;                            /* output block */
//unsigned int *outputLen;                          /* length of output block */
//unsigned char *input;                             /* input block */
//unsigned int inputLen;                            /* length of input block */
//R_RSA_PUBLIC_KEY *publicKey;                      /* RSA public key */

static int RSAPublicBlock (uint8 * output, unsigned int * outputLen,uint8 * input, unsigned int inputLen, R_RSA_PUBLIC_KEY *publicKey)
{
	NN_DIGIT c[MAX_NN_DIGITS], e[MAX_NN_DIGITS], m[MAX_NN_DIGITS],
    		n[MAX_NN_DIGITS];
  	unsigned int eDigits, nDigits;

  	NN_Decode (m, MAX_NN_DIGITS, input, inputLen);
  	NN_Decode (n, MAX_NN_DIGITS, publicKey->modulus, MAX_RSA_MODULUS_LEN);
  	NN_Decode (e, MAX_NN_DIGITS, publicKey->exponent, 4);
  	nDigits = NN_Digits (n, MAX_NN_DIGITS);
  	eDigits = NN_Digits (e, MAX_NN_DIGITS);

  	if (NN_Cmp (m, n, nDigits) >= 0)
    		return (RE_DATA);

  	/* Compute c = m^e mod n.
   	 */
  	NN_ModExp (c, m, e, eDigits, n, nDigits);

  	*outputLen = (publicKey->bits + 7) / 8;
  	NN_Encode (output, *outputLen, c, nDigits);

  	/* Zeroize sensitive information.
   	 */
  	R_memset ((uint8 *)c, 0, sizeof (c));
  	R_memset ((uint8 *)m, 0, sizeof (m));

  	return (0);
}


/* Raw RSA private-key operation. Output has same length as modulus.

    Assumes inputLen < length of modulus.
    Requires input < modulus. 
*/
    //unsigned char *output;                            /* output block */
    //unsigned int *outputLen;                          /* length of output block */
    //unsigned char *input;                             /* input block */
    //unsigned int inputLen;                            /* length of input block */
    //R_RSA_PRIVATE_KEY *privateKey;                    /* RSA private key */

static int RSAPrivateBlock (unsigned char * output,unsigned int * outputLen, unsigned char * input,unsigned int inputLen,R_RSA_PRIVATE_KEY * privateKey)
{
  	NN_DIGIT c[MAX_NN_DIGITS], cP[MAX_NN_DIGITS], cQ[MAX_NN_DIGITS],
    		dP[MAX_NN_DIGITS], dQ[MAX_NN_DIGITS], mP[MAX_NN_DIGITS],
    		mQ[MAX_NN_DIGITS], n[MAX_NN_DIGITS], p[MAX_NN_DIGITS], q[MAX_NN_DIGITS],
    		qInv[MAX_NN_DIGITS], t[MAX_NN_DIGITS];
  	unsigned int cDigits, nDigits, pDigits;

    // Decodes character string b into a.
  	NN_Decode (c, MAX_NN_DIGITS, input, inputLen);

  // 潘平彬 2007年6月22号修改
  // 修改目的:为了适应不同模长的RSA公私钥对，即公私钥对
  // 在使用前不需要事先扩充到256字节。

  	NN_Decode(n, MAX_NN_DIGITS, privateKey->modulus, MAX_RSA_MODULUS_LEN);
  	NN_Decode(p, MAX_NN_DIGITS, privateKey->prime[0], MAX_RSA_PRIME_LEN);
  	NN_Decode(q, MAX_NN_DIGITS, privateKey->prime[1], MAX_RSA_PRIME_LEN);
  	NN_Decode(dP, MAX_NN_DIGITS, privateKey->primeExponent[0], MAX_RSA_PRIME_LEN);
  	NN_Decode(dQ, MAX_NN_DIGITS, privateKey->primeExponent[1], MAX_RSA_PRIME_LEN);
  	NN_Decode (qInv, MAX_NN_DIGITS, privateKey->coefficient, MAX_RSA_PRIME_LEN);

  	cDigits = NN_Digits (c, MAX_NN_DIGITS);
  	nDigits = NN_Digits (n, MAX_NN_DIGITS);
  	pDigits = NN_Digits (p, MAX_NN_DIGITS);

  	if (NN_Cmp (c, n, nDigits) >= 0)
    		return (RE_DATA);

  	/* Compute mP = cP^dP mod p  and  mQ = cQ^dQ mod q. (Assumes q has
     	     length at most pDigits, i.e., p > q.)
   	  */
  	NN_Mod (cP, c, cDigits, p, pDigits);
  	NN_Mod (cQ, c, cDigits, q, pDigits);
  	NN_ModExp (mP, cP, dP, pDigits, p, pDigits);
  	NN_AssignZero (mQ, nDigits);
  	NN_ModExp (mQ, cQ, dQ, pDigits, q, pDigits);

  	/* Chinese Remainder Theorem:
              m = ((((mP - mQ) mod p) * qInv) mod p) * q + mQ.
   	  */
  	if (NN_Cmp (mP, mQ, pDigits) >= 0)
    		NN_Sub (t, mP, mQ, pDigits);
  	else 
	{
    		NN_Sub (t, mQ, mP, pDigits);
    		NN_Sub (t, p, t, pDigits);
  	}

  	NN_ModMult (t, t, qInv, p, pDigits);
  	NN_Mult (t, t, q, pDigits);
  	NN_Add (t, t, mQ, nDigits);

  	*outputLen = (privateKey->bits + 7) / 8;
  	NN_Encode (output, *outputLen, t, nDigits);

  	/* Zeroize sensitive information.
   	  */
  	R_memset ((uint8 *)c, 0, sizeof (c));
  	R_memset ((uint8 *)cP, 0, sizeof (cP));
  	R_memset ((uint8 *)cQ, 0, sizeof (cQ));
  	R_memset ((uint8 *)dP, 0, sizeof (dP));
  	R_memset ((uint8 *)dQ, 0, sizeof (dQ));
  	R_memset ((uint8 *)mP, 0, sizeof (mP));
  	R_memset ((uint8 *)mQ, 0, sizeof (mQ));
  	R_memset ((uint8 *)p, 0, sizeof (p));
  	R_memset ((uint8 *)q, 0, sizeof (q));
  	R_memset ((uint8 *)qInv, 0, sizeof (qInv));
  	R_memset ((uint8 *)t, 0, sizeof (t));

  	return (0);
}


//R_RSA_PUBLIC_KEY g_rsapuk={
//.bits = 2048,
//.modulus = "\xD0\x1A\x4F\x92\x3A\x62\x9D\xC4\x52\x71\x93\x3E\xAE\x5D\x9A\x42,0x7E\x48\xF1\x66\xEF\x3D\xFA\x7A\x67\xAE\x30\xE0\xF9\x9A\xC7\x28,0xF8\x93\xFB\xF9\x69\x06\x6D\xC0\x5D\x91\x8A\xEB\xE9\xA2\xE1\x09,0x67\xED\xFB\x14\x2B\x71\x98\x84\xDF\xA8\x72\xBF\xBC\x41\x1D\x5C,0x20\x51\xA2\xDC\x2E\xFA\xE1\xC9\xBB\xED\xB3\xB5\x29\xA2\x37\x2F,0xB0\xA7\x38\x44\x51\x35\x00\xA6\xFC\xDD\x1D\x0F\x80\xE9\x7E\x9B,0xA9\xA6\xA9\xC4\x9B\x50\xBE\x14\x64\xC7\x86\xE6\x4C\x66\xF2\x2B,0x77\x04\x64\x45\x1A\x98\xC4\xA3\x2E\x74\xAE\x78\x6D\xAE\x20\xBF,0xA6\x6B\xE5\x12\xF1\xAE\x68\x18\x48\xC2\xA3\xC8\x76\xC5\xF3\x99,0x41\x0B\x04\x25\xFC\x1C\xED\x46\x6E\xC3\xDE\x1B\x7E\xA8\x4D\xF6,0x40\xB8\x7C\xAB\xB8\x86\x2D\x0D\x98\x08\xCE\x9C\xD4\xDD\xC6\x56,0x26\x4F\xBE\x98\x80\x2A\xB8\xFB\xE1\x05\xAE\x42\x61\x2C\x08\x44,0x94\x65\xD8\xAC\x1B\xD2\xF3\xE6\xB1\xB5\x42\x70\x07\x86\x13\xF1,0x04\x9C\x9B\x68\x2F\xA4\xA4\xF0\xB1\xAA\xFF\xF1\xD2\xFA\x57\xCF,0xE1\xB1\x7C\x8B\xC3\x96\x3E\xD8\xB6\xD5\x20\x82\x1F\x26\xEA\x4B,0x43\x72\x01\x09\x8E\x4F\x8D\xA5\xF7\x55\x2F\xA3\xAE\xEB\xE9\x31"
//.exponent = "\x00\x01\x00\x01",
//};
R_RSA_PUBLIC_KEY g_stpuk;

#if 0
uint8 const g_puk[]={
0xD0, 0x1A, 0x4F, 0x92, 0x3A, 0x62, 0x9D, 0xC4, 0x52, 0x71, 0x93, 0x3E, 0xAE, 0x5D, 0x9A, 0x42,
0x7E, 0x48, 0xF1, 0x66, 0xEF, 0x3D, 0xFA, 0x7A, 0x67, 0xAE, 0x30, 0xE0, 0xF9, 0x9A, 0xC7, 0x28,
0xF8, 0x93, 0xFB, 0xF9, 0x69, 0x06, 0x6D, 0xC0, 0x5D, 0x91, 0x8A, 0xEB, 0xE9, 0xA2, 0xE1, 0x09,
0x67, 0xED, 0xFB, 0x14, 0x2B, 0x71, 0x98, 0x84, 0xDF, 0xA8, 0x72, 0xBF, 0xBC, 0x41, 0x1D, 0x5C,
0x20, 0x51, 0xA2, 0xDC, 0x2E, 0xFA, 0xE1, 0xC9, 0xBB, 0xED, 0xB3, 0xB5, 0x29, 0xA2, 0x37, 0x2F,
0xB0, 0xA7, 0x38, 0x44, 0x51, 0x35, 0x00, 0xA6, 0xFC, 0xDD, 0x1D, 0x0F, 0x80, 0xE9, 0x7E, 0x9B,
0xA9, 0xA6, 0xA9, 0xC4, 0x9B, 0x50, 0xBE, 0x14, 0x64, 0xC7, 0x86, 0xE6, 0x4C, 0x66, 0xF2, 0x2B,
0x77, 0x04, 0x64, 0x45, 0x1A, 0x98, 0xC4, 0xA3, 0x2E, 0x74, 0xAE, 0x78, 0x6D, 0xAE, 0x20, 0xBF,
0xA6, 0x6B, 0xE5, 0x12, 0xF1, 0xAE, 0x68, 0x18, 0x48, 0xC2, 0xA3, 0xC8, 0x76, 0xC5, 0xF3, 0x99,
0x41, 0x0B, 0x04, 0x25, 0xFC, 0x1C, 0xED, 0x46, 0x6E, 0xC3, 0xDE, 0x1B, 0x7E, 0xA8, 0x4D, 0xF6,
0x40, 0xB8, 0x7C, 0xAB, 0xB8, 0x86, 0x2D, 0x0D, 0x98, 0x08, 0xCE, 0x9C, 0xD4, 0xDD, 0xC6, 0x56,
0x26, 0x4F, 0xBE, 0x98, 0x80, 0x2A, 0xB8, 0xFB, 0xE1, 0x05, 0xAE, 0x42, 0x61, 0x2C, 0x08, 0x44,
0x94, 0x65, 0xD8, 0xAC, 0x1B, 0xD2, 0xF3, 0xE6, 0xB1, 0xB5, 0x42, 0x70, 0x07, 0x86, 0x13, 0xF1,
0x04, 0x9C, 0x9B, 0x68, 0x2F, 0xA4, 0xA4, 0xF0, 0xB1, 0xAA, 0xFF, 0xF1, 0xD2, 0xFA, 0x57, 0xCF,
0xE1, 0xB1, 0x7C, 0x8B, 0xC3, 0x96, 0x3E, 0xD8, 0xB6, 0xD5, 0x20, 0x82, 0x1F, 0x26, 0xEA, 0x4B,
0x43, 0x72, 0x01, 0x09, 0x8E, 0x4F, 0x8D, 0xA5, 0xF7, 0x55, 0x2F, 0xA3, 0xAE, 0xEB, 0xE9, 0x31,
};

void stpuk_config()
{
    g_stpuk.bits = 2048;
    memcpy(g_stpuk.exponent,"\x00\x01\x00\x01",4);
    memcpy(g_stpuk.modulus,g_puk,sizeof(g_puk));
}
#endif
void stpuk_config_security(void)
{
	int rv;
	uint8_t puk_id[SPF_PUKID_SIZE];
	signkey_t skt;
	unsigned char cs[8];
	

    skt.tPK.n = g_stpuk.modulus;
    skt.tPK.e = g_stpuk.exponent;
    skt.cs    = cs;
        
	//鍙栧嚭鍏挜鍗冲彲									
	rv = sLoadPublickey(SFILE_TYPE_APP,puk_id, &skt);
	sysLOG(SECURITY_LOG_LEVEL_1, "sLoadPublickey return %d\n", rv);
	if(rv ) 
        goto err_finish;
    g_stpuk.bits = skt.tPK.BitLen;

err_finish:
	return rv;

}

#if 0
void rsa_calc_test()
{
    uint8 output[256];
    uint8 input[256];
    uint32 lenth;
    int iRet;
    int i;
    

    stpuk_config();
    
    CZ_memset(input,0x01,sizeof(input));
    
    iRet = RSAPublicDecrypt(output,         /* output block */
                            &lenth,         /* length of output block */
                            input,          /* input block */
                            sizeof(input),  /* length of input block */
                            &g_stpuk);     /* RSA public key */
    CZ_TRACE(1,"lenth = %d",lenth);
    
     for(i=0;i<sizeof(output);i++)
         CZ_TRACE(1, "%x",output[i]);

}
#endif


