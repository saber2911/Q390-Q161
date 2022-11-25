#include "comm.h"


static hash_context hd;

static void sha256_encode( BYTE *from, BYTE *to, int len );
static void sha256_transform( sha256_context *hd, BYTE *msg );

 void sha1_init( sha1_context *hd )
{
    hd->h[0] = 0x67452301;
    hd->h[1] = 0xefcdab89;
    hd->h[2] = 0x98badcfe;
    hd->h[3] = 0x10325476;
    hd->h[4] = 0xc3d2e1f0;
}

/*
// prototype: static sha1_transform( IN sha1_context *hd, IN BYTE *msg );
// parameters:
//      hd: HASH计算上下文结构
//      msg: 输入待计算的数据
// return value:
//      无
// remark:
//      对msg指向的一个SHA1原文数据块进行变换计算。
*/
static void sha1_transform( sha1_context *hd, BYTE *msg )
{
    DWORD a,b,c,d,e,tm;
    DWORD x[16];

    /* get values from the chaining vars */
    a = hd->h[0];
    b = hd->h[1];
    c = hd->h[2];
    d = hd->h[3];
    e = hd->h[4];

#ifdef _BIG_ENDIAN_HOST
    memcpy( x, msg, 64 );
#else
    {
        BYTE i;
        BYTE *p2;

        for(i=0, p2=(BYTE*)x; i < 16; i++, p2 += 4 ) {
            p2[3] = *msg++;
            p2[2] = *msg++;
            p2[1] = *msg++;
            p2[0] = *msg++;
        }
    }
#endif


#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L
#define F1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )
#define F2(x,y,z)   ( x ^ y ^ z )
#define F3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )
#define F4(x,y,z)   ( x ^ y ^ z )

#define rol(x, n) (((x) << (n)) | ((x) >> (sizeof(DWORD)*8-(n))))
#define M(i) ( tm =   x[i&0x0f] ^ x[(i-14)&0x0f] \
		    ^ x[(i-8)&0x0f] ^ x[(i-3)&0x0f] \
	       , (x[i&0x0f] = rol(tm,1)) )

#define R(a,b,c,d,e,f,k,m)  do { e += rol( a, 5 )     \
				      + f( b, c, d )  \
				      + k	      \
				      + m;	      \
				 b = rol( b, 30 );    \
			       } while(0)
    R( a, b, c, d, e, F1, K1, x[ 0] );
    R( e, a, b, c, d, F1, K1, x[ 1] );
    R( d, e, a, b, c, F1, K1, x[ 2] );
    R( c, d, e, a, b, F1, K1, x[ 3] );
    R( b, c, d, e, a, F1, K1, x[ 4] );
    R( a, b, c, d, e, F1, K1, x[ 5] );
    R( e, a, b, c, d, F1, K1, x[ 6] );
    R( d, e, a, b, c, F1, K1, x[ 7] );
    R( c, d, e, a, b, F1, K1, x[ 8] );
    R( b, c, d, e, a, F1, K1, x[ 9] );
    R( a, b, c, d, e, F1, K1, x[10] );
    R( e, a, b, c, d, F1, K1, x[11] );
    R( d, e, a, b, c, F1, K1, x[12] );
    R( c, d, e, a, b, F1, K1, x[13] );
    R( b, c, d, e, a, F1, K1, x[14] );
    R( a, b, c, d, e, F1, K1, x[15] );
    R( e, a, b, c, d, F1, K1, M(16) );
    R( d, e, a, b, c, F1, K1, M(17) );
    R( c, d, e, a, b, F1, K1, M(18) );
    R( b, c, d, e, a, F1, K1, M(19) );
    R( a, b, c, d, e, F2, K2, M(20) );
    R( e, a, b, c, d, F2, K2, M(21) );
    R( d, e, a, b, c, F2, K2, M(22) );
    R( c, d, e, a, b, F2, K2, M(23) );
    R( b, c, d, e, a, F2, K2, M(24) );
    R( a, b, c, d, e, F2, K2, M(25) );
    R( e, a, b, c, d, F2, K2, M(26) );
    R( d, e, a, b, c, F2, K2, M(27) );
    R( c, d, e, a, b, F2, K2, M(28) );
    R( b, c, d, e, a, F2, K2, M(29) );
    R( a, b, c, d, e, F2, K2, M(30) );
    R( e, a, b, c, d, F2, K2, M(31) );
    R( d, e, a, b, c, F2, K2, M(32) );
    R( c, d, e, a, b, F2, K2, M(33) );
    R( b, c, d, e, a, F2, K2, M(34) );
    R( a, b, c, d, e, F2, K2, M(35) );
    R( e, a, b, c, d, F2, K2, M(36) );
    R( d, e, a, b, c, F2, K2, M(37) );
    R( c, d, e, a, b, F2, K2, M(38) );
    R( b, c, d, e, a, F2, K2, M(39) );
    R( a, b, c, d, e, F3, K3, M(40) );
    R( e, a, b, c, d, F3, K3, M(41) );
    R( d, e, a, b, c, F3, K3, M(42) );
    R( c, d, e, a, b, F3, K3, M(43) );
    R( b, c, d, e, a, F3, K3, M(44) );
    R( a, b, c, d, e, F3, K3, M(45) );
    R( e, a, b, c, d, F3, K3, M(46) );
    R( d, e, a, b, c, F3, K3, M(47) );
    R( c, d, e, a, b, F3, K3, M(48) );
    R( b, c, d, e, a, F3, K3, M(49) );
    R( a, b, c, d, e, F3, K3, M(50) );
    R( e, a, b, c, d, F3, K3, M(51) );
    R( d, e, a, b, c, F3, K3, M(52) );
    R( c, d, e, a, b, F3, K3, M(53) );
    R( b, c, d, e, a, F3, K3, M(54) );
    R( a, b, c, d, e, F3, K3, M(55) );
    R( e, a, b, c, d, F3, K3, M(56) );
    R( d, e, a, b, c, F3, K3, M(57) );
    R( c, d, e, a, b, F3, K3, M(58) );
    R( b, c, d, e, a, F3, K3, M(59) );
    R( a, b, c, d, e, F4, K4, M(60) );
    R( e, a, b, c, d, F4, K4, M(61) );
    R( d, e, a, b, c, F4, K4, M(62) );
    R( c, d, e, a, b, F4, K4, M(63) );
    R( b, c, d, e, a, F4, K4, M(64) );
    R( a, b, c, d, e, F4, K4, M(65) );
    R( e, a, b, c, d, F4, K4, M(66) );
    R( d, e, a, b, c, F4, K4, M(67) );
    R( c, d, e, a, b, F4, K4, M(68) );
    R( b, c, d, e, a, F4, K4, M(69) );
    R( a, b, c, d, e, F4, K4, M(70) );
    R( e, a, b, c, d, F4, K4, M(71) );
    R( d, e, a, b, c, F4, K4, M(72) );
    R( c, d, e, a, b, F4, K4, M(73) );
    R( b, c, d, e, a, F4, K4, M(74) );
    R( a, b, c, d, e, F4, K4, M(75) );
    R( e, a, b, c, d, F4, K4, M(76) );
    R( d, e, a, b, c, F4, K4, M(77) );
    R( c, d, e, a, b, F4, K4, M(78) );
    R( b, c, d, e, a, F4, K4, M(79) );

    /* update chainig vars */
    hd->h[0] += a;
    hd->h[1] += b;
    hd->h[2] += c;
    hd->h[3] += d;
    hd->h[4] += e;
}

static void HASH_vTransform(BYTE Algo, BYTE *buf)
//----------------------------------------------------------------------
//  Update History:
//  02/25/2004 - V2.00 - LCS :  First official release
//----------------------------------------------------------------------
//     Input Parameters:	Algo: 算法选择标识，在hash.h中有定义
//                          buf:  参与计算的实际数据
//    Output Parameters:    none
//  Modified Parameters:    none
//----------------------------------------------------------------------
//  Description:
//
//  本函数进行实际的HASH计算。
//
//----------------------------------------------------------------------
{
    switch( Algo )
    {
    case HASH_SHA1:
        sha1_transform( (sha1_context *)&hd, buf );
        break;
    case HASH_SHA256:
        sha256_transform( (sha256_context *)&hd, buf );
        break;
    default:
        break;
    }
}

static void HASH_vUpdate(BYTE Algo, BYTE *inbuf, WORD inlen)
//----------------------------------------------------------------------
//  Update History:
//  02/25/2004 - V2.00 - LCS :  First official release
//----------------------------------------------------------------------
//     Input Parameters:    Algo: 算法选择标识，在hash.h中有定义
//                          inbuf: 输入数据的缓冲区指针
//                          inlen: 输入数据的长度
//    Output Parameters:    none
//  Modified Parameters:    none
//----------------------------------------------------------------------
//  Description:
//
//  本函数负责把数据流整理成数据块，并调用HASH_vTransform()函数进行变换
//  我们不对inlen的值进行限制，本函数可以处理不定长数据块的情况。
//
//----------------------------------------------------------------------
{
    int blocksize = hd.blocksize;

    if( hd.count == blocksize ) 
    { /* flush the buffer */
        HASH_vTransform( Algo, hd.buf );
        hd.count = 0;
        hd.nblocks++;
    }
    if( !inbuf )
        return;
    if( hd.count ) 
    {
        for( ; inlen && hd.count < blocksize; inlen-- )
            hd.buf[hd.count++] = *inbuf++;
        
        HASH_vUpdate( Algo, NULL, 0 );
        if( !inlen )
            return;
    }

    while( inlen >= blocksize )
    {
        HASH_vTransform( Algo, inbuf );
        hd.count = 0;
        hd.nblocks++;
        inlen -= blocksize;
        inbuf += blocksize;
    }
    for( ; inlen && hd.count < blocksize; inlen-- )
        hd.buf[hd.count++] = *inbuf++;
}


static void HASH_vFinal(BYTE Algo)
//----------------------------------------------------------------------
//  Update History:
//  02/25/2004 - V2.00 - LCS :  First official release
//----------------------------------------------------------------------
//     Input Parameters:    Algo: 算法选择标识，在hash.h中有定义
//    Output Parameters:    none
//  Modified Parameters:    none
//----------------------------------------------------------------------
//  Description:
//
//  本函数负责处理剩余不足一块的数据，完成补齐，进行计算并最后整理计算
//  结果。由于HASH_vWrite()函数所采用的算法，剩余数据肯定是保存在上下文
//  结构中的数据缓冲区里。
//
//----------------------------------------------------------------------
{
    DWORD t, msb, lsb;
    int l;

    HASH_vUpdate(Algo, NULL, 0); /* flush */;

    t = hd.nblocks;

    /* multiply by 64 to make a byte count */
    lsb = t << 6; msb = t >> 26;

    /* add the count */
    t = lsb;
    if( (lsb += hd.count) < t )
        msb++;

    /* multiply by 8 to make a bit count */
    t = lsb;
    lsb <<= 3;
    msb <<= 3;
    msb |= t >> 29;

    l = hd.blocksize - hd.padlength;

    if( hd.count < l ) { /* enough room */
        hd.buf[hd.count++] = 0x80; /* pad */
        while( hd.count < l )
            hd.buf[hd.count++] = 0;  /* pad */
    }
    else { /* need one extra block */
        hd.buf[hd.count++] = 0x80; /* pad character */
        while( hd.count < hd.blocksize )
            hd.buf[hd.count++] = 0;
        HASH_vUpdate(Algo, NULL, 0);  /* flush */;
        memset(hd.buf, 0, l ); /* fill next block with zeroes */
    }

    /* append the 64 bit count */
    hd.buf[56] = (INT8U)(msb >> 24);
    hd.buf[57] = (INT8U)(msb >> 16);
    hd.buf[58] = (INT8U)(msb >>  8);
    hd.buf[59] = (INT8U)(msb      );
    hd.buf[60] = (INT8U)(lsb >> 24);
    hd.buf[61] = (INT8U)(lsb >> 16);
    hd.buf[62] = (INT8U)(lsb >>  8);
    hd.buf[63] = (INT8U)(lsb      );

    HASH_vTransform(Algo, hd.buf);
}
static void sha1_encode( BYTE *from, BYTE *to, int len )
{
#ifdef _BIG_ENDIAN
    memcpy( to, from, len );
#else
    int i,l;
    BYTE * p2;

    l = len>>2;
    for(i=0, p2=from; i < l; i++, p2 += 4 ) {
        *to++ = p2[3];
        *to++ = p2[2];
        *to++ = p2[1];
        *to++ = p2[0];
    }
#endif
}
static BYTE HASH_bEncode(BYTE Algo, void *Outbuf)
//----------------------------------------------------------------------
//  Update History:
//  02/25/2004 - V2.00 - LCS :  First official release
//  06/30/2004 - V2.01 - LCS :  本函数不再负责整理数据，HAL层自己负责
//                              把计算结果的字节序调整好。
//  09/21/2004 - V2.02 - LCS :  本函数要根据选择的算法调用HAL层整理计算
//                              结果。06/30/2004的修改未完善。
//  09/24/2004 - V2.03 - LCS :  本函数要求HAL层把encode的结果直接放到
//                              Outbuf中，保持hd->h中为原始计算结果。
//  09/09/2011 - V2.04 - LCS :  本函数返回摘要的长度（字节数）。
//----------------------------------------------------------------------
//     Input Parameters:    Algo: 选择的HASH算法
//                          Outbuf: 数据输出缓冲区
//    Output Parameters:    摘要的长度（字节数）
//  Modified Parameters:    none
//----------------------------------------------------------------------
//  Description:
//
//  本函数负责把HASH_CONTEXT中的h[]域整理之后拷贝到Outbuf中。
//
//  09/24/2004 - V2.02 - LCS :
//  hash_context->h[]中必须总是保持HASH计算的原始结果，这样当本函数被重
//  复调用时就不会导致hash_context->h[]中的数据被反复倒序。
//
//----------------------------------------------------------------------
{
    switch( Algo )
    {
    case HASH_SHA1:
        sha1_encode((INT8U *)hd.h,Outbuf,SHA1_RESULT_LENGTH);
        return SHA1_RESULT_LENGTH;
    case HASH_SHA256:
        sha256_encode((INT8U *)hd.h,Outbuf,SHA256_RESULT_LENGTH);
        return SHA256_RESULT_LENGTH;
    default:
        return 0;
    }
}

void SHA_start(uint8 b_mode,uint8 *in_temp,int lenth,uint8 *out_temp)
{
	
	
	switch(b_mode)
	{
		case SHA_INIT_MODE:
			hd.nblocks = hd.count = 0;
			hd.blocksize = 64;
			hd.padlength = 8;
			sha1_init((sha1_context *)&hd);
		break;
		case SHA_UPDATE_MODE:
			HASH_vUpdate(HASH_SHA1, in_temp, lenth);
		break;
		case SHA_FINAL_MODE:
			HASH_vFinal(HASH_SHA1);
			HASH_bEncode(HASH_SHA1, out_temp);
		break;
		defult:
		break;
	}	
}

/*
// prototype: static void sha256_init(IN sha256_context *hd);
// parameters:
//      hd: HASH计算上下文结构
// return value:
//      无
// remark:
//      在HASH上下文结构中填入SHA256的初始向量。
*/
static void sha256_init( sha256_context *hd )
{
    hd->h[0] = 0x6a09e667;
    hd->h[1] = 0xbb67ae85;
    hd->h[2] = 0x3c6ef372;
    hd->h[3] = 0xa54ff53a;
    hd->h[4] = 0x510e527f;
    hd->h[5] = 0x9b05688c;
    hd->h[6] = 0x1f83d9ab;
    hd->h[7] = 0x5be0cd19;
}
/*
// prototype: static sha256_transform( IN sha256_context *hd, IN BYTE *msg );
// parameters:
//      hd: HASH计算上下文结构
//      msg: 输入待计算的数据
// return value:
//      无
// remark:
//      对msg指向的一个SHA256原文数据块进行变换计算。
*/
static void sha256_transform( sha256_context *hd, BYTE *msg )
{
    DWORD a,b,c,d,e,f,g,h;
    DWORD w[64];
    int t;
    static const DWORD k[]=
    {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    /* get values from the chaining vars */
    a = hd->h[0];
    b = hd->h[1];
    c = hd->h[2];
    d = hd->h[3];
    e = hd->h[4];
    f = hd->h[5];
    g = hd->h[6];
    h = hd->h[7];

#ifdef BIG_ENDIAN_HOST
    memcpy( w, data, 64 );
#else
    {
        int i;
        BYTE *p2;

        for(i=0, p2=(BYTE*)w; i < 16; i++, p2 += 4 )
        {
            p2[3] = *msg++;
            p2[2] = *msg++;
            p2[1] = *msg++;
            p2[0] = *msg++;
        }
    }
#endif

#define ROTR(x,n) (((x)>>(n)) | ((x)<<(32-(n))))
#define Ch(x,y,z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sum0(x) (ROTR((x),2) ^ ROTR((x),13) ^ ROTR((x),22))
#define Sum1(x) (ROTR((x),6) ^ ROTR((x),11) ^ ROTR((x),25))
#define S0(x) (ROTR((x),7) ^ ROTR((x),18) ^ ((x)>>3))
#define S1(x) (ROTR((x),17) ^ ROTR((x),19) ^ ((x)>>10))

    for(t=16;t<64;t++)
        w[t] = S1(w[t-2]) + w[t-7] + S0(w[t-15]) + w[t-16];

    for(t=0;t<64;t++)
    {
        DWORD t1,t2;

        t1=h+Sum1(e)+Ch(e,f,g)+k[t]+w[t];
        t2=Sum0(a)+Maj(a,b,c);
        h=g;
        g=f;
        f=e;
        e=d+t1;
        d=c;
        c=b;
        b=a;
        a=t1+t2;
        /* printf("t=%d a=%08lX b=%08lX c=%08lX d=%08lX e=%08lX f=%08lX g=%08lX h=%08lX\n",t,a,b,c,d,e,f,g,h); */
    }

    /* update chaining vars */
    hd->h[0] += a;
    hd->h[1] += b;
    hd->h[2] += c;
    hd->h[3] += d;
    hd->h[4] += e;
    hd->h[5] += f;
    hd->h[6] += g;
    hd->h[7] += h;
}
/*
// prototype: static void sha256_encode( BYTE *from, BYTE *to, int len );
// parameters:
//      from: 待调序的摘要值在这里
//      to: 输出的摘要值存到这里
//      len: 摘要值长度
// return value:
//      无
// remark:
//      对from指向的的摘要值调序后输出。
*/
static void sha256_encode( BYTE *from, BYTE *to, int len )
{
#ifdef BIG_ENDIAN_HOST
    memcpy( to, from, len );
#else
    int i,l;
    BYTE * p2;

    l = len>>2;
    for(i=0, p2=from; i < l; i++, p2 += 4 ) {
        *to++ = p2[3];
        *to++ = p2[2];
        *to++ = p2[1];
        *to++ = p2[0];
    }
#endif
}

static void HASH_vInit(BYTE Algo)
//----------------------------------------------------------------------
//  Update History:
//  02/25/2004 - V2.00 - LCS :  First official release
//  09/24/2004 - V2.01 - LCS :  本函数原型修改，返回一个BOOL值。
//----------------------------------------------------------------------
//     Input Parameters:    Algo: 算法选择标识，在algorithm.h中有定义
//    Output Parameters:    TRUE/FALSE: 返回FALSE时表示算法不被支持。
//  Modified Parameters:    none
//----------------------------------------------------------------------
//  Description:
//
//  本函数负责把HASH_CONTEXT结构进行初始化。
//
//  09/24/2004 - V2.01 - LCS :
//  既然HASH算法是可裁剪的，我们理应能判断参数Algo是否被支持，并给出返回
//  值和错误代码SW1/SW2说明情况。
//
//----------------------------------------------------------------------
{
    //------------------------------------------------------------------
    // 调用HAL层获取HASH CONTEXT。
    //------------------------------------------------------------------

    //------------------------------------------------------------------
    // 进一步初始化内部成员。
    //------------------------------------------------------------------
    hd.nblocks = hd.count = 0;

    switch( Algo )
    {
    case HASH_SHA1:
        hd.blocksize = 64;
        hd.padlength = 8;
        sha1_init((sha1_context *)&hd);
        break;
    case HASH_SHA256:
        hd.blocksize = 64;
        hd.padlength = 8;
        sha256_init((sha256_context *)&hd);
        break;
    default:
        break;
    }
}
    
/*
// prototype: WORD saSha256(IN BYTE *pbData, IN WORD wDataLen, OUT BYTE *pbResult, IN BYTE bMode);
// parameters:
//		pbData: 要Hash的数据
//		wDataLen: 数据长度.
//		pbResult: HASH结果. 当bMode=2,且函数返回OK时有效.
//		bMode: 模式, 0: Init; 1:Update; 2:Final;
// return value:
//		ERR_SUCCESS: 成功;
//		其它值: 错误码
// remarks: 对数据进行SHA256计算.
*/
WORD saSha256(BYTE *pbData, WORD wDataLen, BYTE *pbResult, BYTE bMode)
{
    switch(bMode)
    {
    case HASH_MODE_INIT:
        HASH_vInit(HASH_SHA256);
        break;
    case HASH_MODE_UPDATE:
        //CZ_TRACE(1,"sha256");
        //CZ_TRACE(1,"%s",pbData);
        HASH_vUpdate(HASH_SHA256, pbData, wDataLen);
        break;
    case HASH_MODE_FINAL:
        HASH_vFinal(HASH_SHA256);
        HASH_bEncode(HASH_SHA256, pbResult);
        break;
    }

    return 0;
}


#if 0
void main()
{
	unsigned char result[20];
	char temp[256];//={"\x00\x00\x00\x00\x00\x00\x00\x00"};
	int lenth = 0;
	int i;
	while(1){
	memset(temp,0,sizeof(lenth));
	printf("plsase input data:\r\n");
	scanf("%s",temp);
	lenth = strlen(temp);
	//init hd struct
	hd.nblocks = hd.count = 0;
	hd.blocksize = 64;
	hd.padlength = 8;
	sha1_init((sha1_context *)&hd);

	HASH_vUpdate(HASH_SHA1, temp, lenth);

	HASH_vFinal(HASH_SHA1);
	memset(result,0,sizeof(result));
	HASH_bEncode(HASH_SHA1, result);
	//
	for(i=0;i<sizeof(result);i++)
	printf("%x",result[i]);

	printf("\r\n");
	}


}

#endif