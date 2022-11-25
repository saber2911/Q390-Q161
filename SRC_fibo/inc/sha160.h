#ifndef _SHA160_H
#define	_SHA160_H

#include "comm.h"


#define	SHA_INIT_MODE	1
#define	SHA_UPDATE_MODE	2
#define SHA_FINAL_MODE	3

#define HASH_MODE_INIT              SHA_INIT_MODE //0x00
#define HASH_MODE_UPDATE            SHA_UPDATE_MODE //0x01
#define HASH_MODE_FINAL             SHA_FINAL_MODE //0x02

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int	  DWORD;
typedef unsigned long	  DDWORD;
typedef char		  CHAR;
typedef unsigned char	  INT8U;
typedef struct {
    DWORD   nblocks;
    WORD    count;
    BYTE    blocksize;
    BYTE    padlength;
    DWORD   h[16];
    BYTE    buf[128];
} sha1_context;

typedef struct {
    DWORD   nblocks;
    WORD    count;
    BYTE    blocksize;
    BYTE    padlength;
    DWORD   h[16];
    BYTE    buf[128];
} sha256_context;

#define SHA256_RESULT_LENGTH    32
/*
// 用于HASH计算的上下文结构，我们按SHA512的最大需求定义的。
*/
typedef struct {
    DWORD nblocks;
    WORD  count;
    BYTE  blocksize;
    BYTE  padlength;
    DWORD h[16];
    BYTE  buf[128];
} hash_context;

#define HASH_SHA1               0x10
#define SHA1_RESULT_LENGTH      20

#define HASH_SHA256             0x20

void SHA_start(uint8 b_mode,uint8 *in_temp,int lenth,uint8 *out_temp);

WORD saSha256(BYTE *pbData, WORD wDataLen, BYTE *pbResult, BYTE bMode);




#endif
