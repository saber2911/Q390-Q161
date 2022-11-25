#ifndef _DES_H_
#define _DES_H_
#include "comm.h"

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

typedef unsigned char   BYTE;

BOOL DesECB(
	IN const BOOL bDecrypt, 
	IN const BYTE *pbKey, 
	IN const BYTE *pbIn, 
	IN const unsigned int nInLen, 
	OUT BYTE *pbOut);

BOOL DesCBC(
	IN const BOOL bDecrypt, 
	const BYTE *pbKey, 
	const BYTE *pbIn, 
	const unsigned int nInLen, 
	BYTE *pbOut, 
	IN OUT BYTE *IV);

BOOL Des3ECB(
	IN const BOOL bDecrypt, 
	IN const BYTE *pbKey,
	IN const BYTE *pbIn,
	IN const unsigned int nInLen,
	OUT BYTE *pbOut);

BOOL Des3CBC(
	IN const BOOL bDecrypt,
	IN const BYTE *pbKey,
	IN const BYTE *pbIn,
	IN const unsigned int nInlen,
	OUT BYTE *pbOut,
	OUT BYTE *IV);

#endif//_DES_H_