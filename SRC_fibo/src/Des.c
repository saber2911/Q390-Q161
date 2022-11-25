//#include <stdio.h>
//#include <wtypes.h>
#include "Des.h"

static void Byte8ToBit64(OUT BYTE * Bit64, IN const BYTE * Byte8)
{
	for (int i = 0; i < 8; i++)
	{
		for ( int j = 0; j < 8; j++)
		{
			char tmp = Byte8[i];
			Bit64[i * 8 + 7 - j] = (tmp >> j) & 0x01;
		}
	}
}

const BYTE PC1[] = {
	57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22, 14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4
};

const BYTE PC2[] = {
	14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10, 23, 19, 12, 4, 26, 8, 16, 7, 27, 20, 13, 2, 41,
	52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48, 44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

static void DesSubKey(OUT BYTE *pbSubKey, IN const BYTE *pbKey)
{
	BYTE i, j;
	BYTE LS[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
	BYTE KEY[64] = {0};
	BYTE C0[30] = {0};
	BYTE D0[30] = {0};

	Byte8ToBit64(KEY, pbKey);
	for (i = 0; i < 28; i++)
	{
		C0[i] = KEY[PC1[i] - 1];
	}
	for (i = 0; i < 28; i++)
	{
		D0[i] = KEY[PC1[i + 28] - 1];
	}
	for (i = 0; i < 16; i++)
	{
		C0[28] = C0[0];
		C0[29] = C0[1];
		D0[28] = D0[0];
		D0[29] = D0[1];
		for (j = 0; j < 28; j++)
		{
			C0[j] = C0[j + LS[i]];
			D0[j] = D0[j + LS[i]];
		}
		for (j = 0; j < 56; j++)
		{
			if (j < 28)
			{
				KEY[j] = C0[j];
			}
			else
			{
				KEY[j] = D0[j - 28];
			}
		}
		for (j = 0; j < 48; j++)
		{
			pbSubKey[i * 48 + j] = KEY[PC2[j] - 1];
		}
	}
}

const unsigned char IP[] = {
	58 - 1, 50 - 1, 42 - 1, 34 - 1, 26 - 1, 18 - 1, 10 - 1, 2 - 1, 60 - 1, 52 - 1, 44 - 1, 36 - 1, 28 - 1, 20 - 1, 12 - 1, 4 - 1,
	62 - 1, 54 - 1, 46 - 1, 38 - 1, 30 - 1, 22 - 1, 14 - 1, 6 - 1, 64 - 1, 56 - 1, 48 - 1, 40 - 1, 32 - 1, 24 - 1, 16 - 1, 8 - 1,
	57 - 1, 49 - 1, 41 - 1, 33 - 1, 25 - 1, 17 - 1, 9 - 1, 1 - 1, 59 - 1, 51 - 1, 43 - 1, 35 - 1, 27 - 1, 19 - 1, 11 - 1, 3 - 1,
	61 - 1, 53 - 1, 45 - 1, 37 - 1, 29 - 1, 21 - 1, 13 - 1, 5 - 1, 63 - 1, 55 - 1, 47 - 1, 39 - 1, 31 - 1, 23 - 1, 15 - 1, 7 - 1
};

const unsigned char IP_1[] = {
	40 - 1, 8 - 1, 48 - 1, 16 - 1, 56 - 1, 24 - 1, 64 - 1, 32 - 1, 39 - 1, 7 - 1, 47 - 1, 15 - 1, 55 - 1, 23 - 1, 63 - 1, 31 - 1,
	38 - 1, 6 - 1, 46 - 1, 14 - 1, 54 - 1, 22 - 1, 62 - 1, 30 - 1, 37 - 1, 5 - 1, 45 - 1, 13 - 1, 53 - 1, 21 - 1, 61 - 1, 29 - 1,
	36 - 1, 4 - 1, 44 - 1, 12 - 1, 52 - 1, 20 - 1, 60 - 1, 28 - 1, 35 - 1, 3 - 1, 43 - 1, 11 - 1, 51 - 1, 19 - 1, 59 - 1, 27 - 1,
	34 - 1, 2 - 1, 42 - 1, 10 - 1, 50 - 1, 18 - 1, 58 - 1, 26 - 1, 33 - 1, 1 - 1, 41 - 1, 9 - 1, 49 - 1, 17 - 1, 57 - 1, 25 - 1
};

const unsigned char E[] = {
	32 - 1, 1 - 1, 2 - 1, 3 - 1, 4 - 1, 5 - 1, 4 - 1, 5 - 1, 6 - 1, 7 - 1, 8 - 1, 9 - 1, 8 - 1, 9 - 1, 10 - 1, 11 - 1, 12 - 1, 13 - 1,
	12 - 1, 13 - 1, 14 - 1, 15 - 1, 16 - 1, 17 - 1, 16 - 1, 17 - 1, 18 - 1, 19 - 1, 20 - 1, 21 - 1, 20 - 1, 21 - 1, 22 - 1, 23 - 1,
	24 - 1, 25 - 1, 24 - 1, 25 - 1, 26 - 1, 27 - 1, 28 - 1, 29 - 1, 28 - 1, 29 - 1, 30 - 1, 31 - 1, 32 - 1, 1 - 1
};

const unsigned char P[] = {
	16 - 1, 7 - 1, 20 - 1, 21 - 1, 29 - 1, 12 - 1, 28 - 1, 17 - 1, 1 - 1, 15 - 1, 23 - 1, 26 - 1, 5 - 1, 18 - 1, 31 - 1, 10 - 1, 
	2 - 1, 8 - 1, 24 - 1, 14 - 1, 32 - 1, 27 - 1, 3 - 1, 9 - 1, 19 - 1, 13 - 1, 30 - 1, 6 - 1, 22 - 1, 11 - 1, 4 - 1, 25 - 1
};

const BYTE S[] = {
	//S1
	14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7, 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
	4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0, 15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13,

	//S2
	15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10, 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
	0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15, 13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9,

	//S3
	10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8, 13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
	13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7, 1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12,

	//S4
	7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15, 13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
	10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4, 3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14,

	//S5
	2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9, 14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
	4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14, 11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3,

	//S6
	12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11, 10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
	9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6, 4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13,

	//S7
	4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1, 13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
	1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2, 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12,

	//S8
	13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7, 1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
	7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8, 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11
};


static void DesCrypt(BYTE *subkey, BYTE *pbOut, BYTE *pbIn, const BOOL bDecrypt)
{
	BYTE *T  = NULL; 
	BYTE *W  = NULL; 
	BYTE *L0 = NULL;  
	BYTE *R0 = NULL; 
	BYTE Q[8]	 = {0};
	BYTE FRK[64] = {0};
	BYTE LR[64]	 = {0};

	Byte8ToBit64(FRK, pbIn);

	int i = 0;
	for (i = 0; i < 64; i++)
	{
		LR[i] = FRK[IP[i]];
	}

	if (bDecrypt)
	{
		subkey += 720;
	}

	int j = 0;
	for (int k = 0; k < 16; k++)
	{
		if (k % 2)
		{
			R0 = LR;
			L0 = LR + 32;
		}
		else
		{
			L0 = LR;
			R0 = LR + 32;
		}

		for (i = 0; i < 48; i++)
		{
			FRK[i] = R0[E[i]];
		}

		for (i = 0; i < 48; i++)
		{
			FRK[i] ^= *subkey++;
		}

		if (bDecrypt)
		{
			subkey -= 96;
		}

		for (i = 0; i < 8; i++)
		{
			char PS1 = FRK[i * 6] * 2 + FRK[i * 6 + 5];
			char PS2 = 0;
			for (j = 1; j < 5; j++)
			{
				PS2 += FRK[i * 6 + j] * (1 << (4 - j));
			}
			Q[i] = S[(int)64 * i + (int)16 * PS1 + PS2];
		}

		T = FRK + 32;
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 4; j++)
			{
				char tmp = Q[i];
				T[i * 4 + 3 - j] = tmp >> j & 0x01;
			}
		}

		W = FRK;
		for (i = 0; i < 32; i++)
		{
			*W++ = T[P[i]];
		}
		W = FRK;
		for (i = 0; i < 32; i++)
		{
			*L0++ ^= *W++;
		}
	}

	L0 = R0 + 32;
	for (i = 0; i < 32; i++)
	{
		j = *L0;
		*L0++ = *R0;
		*R0++ = j;
	}

	for (i = 0; i < 64; i++)
	{
		FRK[i] = LR[IP_1[i]];
	}

	for (i = 0; i < 8; i++)
	{
		for (j = 0, pbOut[i] = 0; j < 8; j++)
		{
			pbOut[i] += FRK[i * 8 + j] * (1 << (7 - j));
		}
	}
}

BOOL DesECB(
	IN const BOOL bDecrypt, 
	IN const BYTE *pbKey, 
	IN const BYTE *pbIn, 
	IN const unsigned int nInLen, 
	OUT BYTE *pbOut)
{
	if (0 != (nInLen % 8))
	{
		return FALSE;
	}

	BYTE SubKey[800] = {0};
	BYTE buf[16] = {0}; 
	BYTE tmp[16] = {0};
	memset(SubKey, 0, sizeof(SubKey));
	memset(buf, 0, sizeof(buf));
	memset(tmp, 0, sizeof(tmp));

	DesSubKey(SubKey, pbKey);

	for (unsigned int i = 0; i < nInLen / 8; i++)
	{
		memcpy(buf, pbIn + i * 8, 8);
		DesCrypt(SubKey, tmp, buf, bDecrypt);
		memcpy(pbOut + i * 8, tmp, 8);
	}

	return TRUE;
}

BOOL DesCBC(
	IN const BOOL bDecrypt, 
	const BYTE *pbKey, 
	const BYTE *pbIn, 
	const unsigned int nInLen, 
	BYTE *pbOut, 
	IN OUT BYTE *IV)
{

	if (0 != (nInLen % 8))
	{
		return FALSE;
	}

	BYTE SubKey[800] = {0};
	BYTE buf[16] = {0}; 
	BYTE tmp[16] = {0};

	memset(SubKey, 0, 800);
	memset(buf, 0, 16);
	memset(tmp, 0, 16);

	DesSubKey(SubKey, pbKey);

	for (unsigned int i = 0; i < nInLen / 8; i++)
	{
		if (bDecrypt)
		{
			memcpy(buf, pbIn + i * 8, 8);
			DesCrypt(SubKey, tmp, buf, bDecrypt);
			for (unsigned int j = 0; j < 8; j++)
			{
				tmp[j] ^= IV[j];
			}
			memcpy(IV, buf, 8);
		}
		else
		{
			for (unsigned int j = 0; j < 8; j++)
			{
				buf[j] = pbIn[i * 8 + j] ^ IV[j];
			}
			DesCrypt(SubKey, tmp, buf, bDecrypt);
			memcpy(IV, tmp, 8);
		}
		memcpy(pbOut + i * 8, tmp, 8);
	}

	return TRUE;
}

BOOL Des3ECB(
	IN const BOOL bDecrypt, 
	IN const BYTE *pbKey,
	IN const BYTE *pbIn,
	IN const unsigned int nInLen,
	OUT BYTE *pbOut)
{
	if (0 != (nInLen % 8))
	{
		return FALSE;
	}

	BYTE SubKey1[800] = {0};
	BYTE SubKey2[800] = {0};
	BYTE SubKey3[800] = {0};
	BYTE buf[16] = {0};
	BYTE tmp[16] = {0};


	memset(SubKey1, 0, 800);
	memset(SubKey2, 0, 800);
	memset(SubKey3, 0, 800);

	memset(buf, 0, 16);
	memset(tmp, 0, 16);

	DesSubKey(SubKey1, pbKey);
	DesSubKey(SubKey2, pbKey + 8);
	DesSubKey(SubKey3, pbKey + 16);

	for (unsigned int i = 0; i < nInLen / 8; i++)
	{
		if (!bDecrypt)
		{
			memcpy(buf, pbIn + i * 8, 8);
			DesCrypt(SubKey1, tmp, buf, bDecrypt);
			DesCrypt(SubKey2, buf, tmp, !bDecrypt);
			DesCrypt(SubKey3, tmp, buf, bDecrypt);
			memcpy(pbOut + i * 8, tmp, 8);
		}
		else
		{
			memcpy(buf, pbIn + i * 8, 8);
			DesCrypt(SubKey3, tmp, buf, bDecrypt);
			DesCrypt(SubKey2, buf, tmp, !bDecrypt);
			DesCrypt(SubKey1, tmp, buf, bDecrypt);
			memcpy(pbOut + i * 8, tmp, 8);
		}
	}

	return TRUE;
}

BOOL Des3CBC(
	IN const BOOL bDecrypt,
	IN const BYTE *pbKey,
	IN const BYTE *pbIn,
	IN const unsigned int nInlen,
	OUT BYTE *pbOut,
	OUT BYTE *IV)
{
	if (0 != (nInlen % 8))
	{
		return FALSE;
	}

	BYTE SubKey1[800] = {0}; 
	BYTE SubKey2[800] = {0};
	BYTE SubKey3[800] = {0};
	BYTE buf[16] = {0}; 
	BYTE tmp[16] = {0};

	memset(SubKey1, 0, 800);
	memset(SubKey2, 0, 800);
	memset(SubKey3, 0, 800);
	memset(buf, 0, 16);
	memset(tmp, 0, 16);

	DesSubKey(SubKey1, pbKey);
	DesSubKey(SubKey2, pbKey + 8);
	DesSubKey(SubKey3, pbKey + 16);

	for (unsigned int i = 0; i < nInlen / 8; i++)
	{
		if (!bDecrypt)
		{
			for (int j = 0; j < 8; j++)
			{
				buf[j] = pbIn[i * 8 + j] ^ IV[j];
			}
			DesCrypt(SubKey1, tmp, buf, bDecrypt);
			DesCrypt(SubKey2, buf, tmp, !bDecrypt);
			DesCrypt(SubKey3, tmp, buf, bDecrypt);
			memcpy(IV, tmp, 8);
		}
		else
		{
			memcpy(buf, pbIn + i * 8, 8);
			DesCrypt(SubKey3, tmp, buf, bDecrypt);
			DesCrypt(SubKey2, buf, tmp, !bDecrypt);
			DesCrypt(SubKey1, tmp, buf, bDecrypt);
			for (int j = 0; j < 8; j++)
			{
				tmp[j] ^= IV[j];
			}
			memcpy(IV, pbIn + i * 8, 8);
		}
		memcpy(pbOut + i * 8, tmp, 8);
	}

	return TRUE;
}