/*
	RSAEURO.H - header file for RSAEURO cryptographic toolkit

	Copyright (c) J.S.A.Kapp 1994 - 1995.

	RSAEURO - RSA Library compatible with RSAREF(tm) 2.0.

	All functions prototypes are the Same as for RSAREF(tm).
	To aid compatiblity the source and the files follow the
	same naming comventions that RSAREF(tm) uses.  This should aid
				direct importing to your applications.

	This library is legal everywhere outside the US.  And should
	NOT be imported to the US and used there.

	This header file contains prototypes, and other definitions used
	in and by RSAEURO.

	Revision history
	0.90 First revision, implements all of RSAREF.H plus some minor
	changes.

	0.91 Second revision, Fixed a couple of problems.  SHS support
	added to header file, digest contexts altered.

	0.92 Third revision, add support at this level for the assembler
	routines and the implementation of some routines using the ANSI C
	equivalent.

	0.93 Fourth revision, Library details section added, MD4 details
	added to header file, digest contexts altered.
*/

#ifndef _RSAEURO_H_
#define _RSAEURO_H_

//#include <string.h>
//#include "md5.h"
//#include "nn.h"
#include "comm.h"



#ifdef __cplusplus
extern "C" {
#endif

/* RSA key lengths. */
#define MIN_RSA_MODULUS_BITS 508
/* 
	 PGP 2.6.2 Now allows 2048-bit keys changing below will allow this.
     It does lengthen key generation slightly if the value is increased.
*/
//#define MAX_RSA_MODULUS_BITS 1024
#define MAX_RSA_MODULUS_BITS 2048
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)

typedef struct {
    // 表示模长度 ,地位在前
	unsigned short int bits;                     /* length in bits of modulus */
    // 公钥模
  	unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
    // 高位在前，前补0 
  	unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* public exponent */
} R_RSA_PUBLIC_KEY;

typedef struct {
  	unsigned short int bits;                     					/* length in bits of modulus */
  	unsigned char modulus[MAX_RSA_MODULUS_LEN];  	/* modulus */
  	unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     	/* public exponent */  
  	unsigned char exponent[MAX_RSA_MODULUS_LEN]; 			/* private exponent */
  	unsigned char prime[2][MAX_RSA_PRIME_LEN];   		/* prime factors */
  	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];	/* exponents for CRT */
  	unsigned char coefficient[MAX_RSA_PRIME_LEN];          	/* CRT coefficient */
} R_RSA_PRIVATE_KEY;


/* Error codes. */
#define RE_CONTENT_ENCODING 0x0400
#define RE_DATA 0x0401
#define RE_DIGEST_ALGORITHM 0x0402
#define RE_ENCODING 0x0403
#define RE_KEY 0x0404
#define RE_KEY_ENCODING 0x0405
#define RE_LEN 0x0406
#define RE_MODULUS_LEN 0x0407
#define RE_NEED_RANDOM 0x0408
#define RE_PRIVATE_KEY 0x0409
#define RE_PUBLIC_KEY 0x040a
#define RE_SIGNATURE 0x040b
#define RE_SIGNATURE_ENCODING 0x040c
#define RE_ENCRYPTION_ALGORITHM 0x040d

/* Library details. */
#define RSAEURO_VER_MAJ 1
#define RSAEURO_VER_MIN 02
#define RSAEURO_IDENT "RSAEURO"
#define RSAEURO_DATE "21/08/94"

/* Internal Error Codes */
#define IDOK    0
#define IDERROR 1

/* Internal defs. */
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

/* Random structure. */
typedef struct {
	unsigned int bytesNeeded;		/* seed bytes required */
  	unsigned char state[16];             	/* state of object */
  	unsigned int outputAvailable;   	/* number byte available */
  	unsigned char output[16];         	/* output bytes */
} R_RANDOM_STRUCT;

/* RSA prototype key. */
typedef struct {
  	unsigned int bits;	/* length in bits of modulus */
  	int useFermat4;	/* public exponent (1 = F4, 0 = 3) */
} R_RSA_PROTO_KEY;


/* Standard library routines. */
#ifndef USE_ANSI
#define USE_ANSI
#endif

#ifdef USE_ANSI
#define R_memset(x, y, z)	memset(x, y, z)
#define R_memcpy(x, y, z)	memcpy(x, y, z)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _RSAEURO_H_ */

