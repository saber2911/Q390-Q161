/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_security.h
** Description:		APP、L1Puk验签
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_SECURITY_H_
#define _HAL_SECURITY_H_

#include "comm.h"

#define SECURITY_LOG_LEVEL_0		LOG_LEVEL_0
#define SECURITY_LOG_LEVEL_1		LOG_LEVEL_1
#define SECURITY_LOG_LEVEL_2		LOG_LEVEL_2
#define SECURITY_LOG_LEVEL_3		LOG_LEVEL_3
#define SECURITY_LOG_LEVEL_4		LOG_LEVEL_4
#define SECURITY_LOG_LEVEL_5		LOG_LEVEL_5


#define uchar		unsigned char

#define SIGN_ERR_EFLAG		(-201)
#define SIGN_ERR_SP			(-202)
#define SIGN_ERR_ALG 		(-203)
#define SIGN_ERR_HASHTYPE 	(-203)
#define SIGN_ERR_ASYMTYPE 	(-204)
#define SIGN_ERR_I		 	(-205)
#define SIGN_ERR_H		 	(-206)
#define SIGN_ERR_F		 	(-207)
#define SIGN_ERR_KEYLEN		(-208)
#define SIGN_ERR_KEYTYPE	(-209)
#define SIGN_ERR_SFYPE		(-210)
#define SIGN_ERR_NOBUF		(-211)
#define SIGN_ERR_NOKEY		(-212)
#define SIGN_ERR_BADADDR	(-213)
#define SIGN_ERR_FILETYPE	(-214)
#define SIGN_ERR_FILEFMT	(-215)
#define SIGN_ERR_KEYEXP		(-216)
#define SIGN_ERR_KEYNA		(-218)
#define SIGN_ERR_NOFILE		(-219)

#define SIGN_ERR_ENCODE		(-220)

#define SIGN_PACKAGE_FLAG_STR ("SIGNED.VERSION01")
#define SIGN_PACKAGE_FLAG_LEN  16


#define SPF_ALG_SIZE		2
#define SPF_CORP_SIZE		96
#define SPF_ID_SIZE			32
#define SPF_TIME_SIZE		8
#define SPF_PUKID_SIZE		16

#define VOS_PUK_FILENAME		"/app/ufs/VOS_PUK.puk"
#define APP_PUK_FILENAME		"/app/ufs/APP_PUK.puk"
#define ROOT_PUK_FINENAME		"/app/ufs/ROOT_PUK.puk"
#define L1_PUK_FILENAME			"/app/ufs/L1_PUK.puk"

#pragma pack(1)
typedef struct sign_package_fmt //__attribute__ ((__packed__))
{
	union {
		unsigned char data[SPF_ALG_SIZE];
		struct {
			unsigned char as;//签名算法，0x00-不签名;0x01-RSA1024;0x02-RSA2048;0x03-RSA3072;0x04-RSA4096
			unsigned char ah;//散列/杂凑算法，0x00-不签名;0x01-SHA1;0x02-SHA224;0x03-SHA256;0x04-SHA384;0x05-SHA512
		}_t;
	}alg;//算法标识
	
	unsigned char puk_id[SPF_PUKID_SIZE];//公钥ID，验证签名的公钥索引
	
	union {
		unsigned char data[SPF_CORP_SIZE+SPF_ID_SIZE+SPF_TIME_SIZE];
		struct {
			unsigned char s_corp[SPF_CORP_SIZE];//签名公司名称
			unsigned char s_id[SPF_ID_SIZE];//签名操作者姓名
			unsigned char s_time[SPF_TIME_SIZE];//签名生成时间		
			}_t;
	}ex_info;//附加信息
	
	union {
		unsigned char data[10];
		struct {
			unsigned int f_len;//被签名的文件有效长度
			unsigned int d_len;//被签名的重要数据有效长度,
			unsigned char fmt;//被签名的文件格式
			unsigned char type;//被签名的文件类型				
			}_t;
	}f_info;//文件信息	
   
	unsigned int f_sig_len;//文件签名数据域的长度
	unsigned int d_sig_len;//重要数据签名数据域的长度
	
	unsigned char p_flag[SIGN_PACKAGE_FLAG_LEN];//“SIGNED.VERSION01”
}sign_package_fmt_t;
#pragma pack()

typedef struct sign_info 
{
	unsigned int f_addr;//被签名的原始文件内容地址
	unsigned int d_sign_addr;//附加的所有信息的长度总和，本字段（含）到签名包标识（含）的总长度，单位为字节
	unsigned int f_sign_addr;//重要数据签名
	unsigned int tspf_len;//签名部分数据长度
	sign_package_fmt_t tSpf;
}sign_info_t;

#define SKF_ALG_SIZE         2
#define SKF_CORP_SIZE  		96
#define SKF_ID_SIZE 		32
#define SKF_TIME_SIZE 		 8
#define SKF_SN_SIZE 		16

#pragma pack(1)
typedef struct signkey_fmt 
{

	unsigned char alg[SKF_ALG_SIZE];
	unsigned char issue_corp[SKF_CORP_SIZE];
	unsigned char issue_id[SKF_ID_SIZE];	
	unsigned char init_time[SKF_TIME_SIZE];
	unsigned char exp_time[SKF_TIME_SIZE];

	unsigned char usr_corp[SKF_CORP_SIZE];
	unsigned char usr_id[SKF_ID_SIZE];	

	unsigned char sn[SKF_SN_SIZE];	

	union {
		unsigned char _byte[2];
		unsigned short _us;
	}keylen;	

}signkey_fmt_t;

#pragma pack()

typedef struct
{
    unsigned short BitLen; // 表示模长度 ,低位在前
    unsigned char*  n; // 公钥模         
    unsigned char*  e;//[4]; // 高位在前，前补0 
}RSA_PUBLIC_KEY;


typedef struct signkey_struct
{
	signkey_fmt_t  tSkf;
	RSA_PUBLIC_KEY tPK;
	unsigned char*  cs;
}signkey_t;

  


//计算结构体中一个字段的偏移
#define offsetof(type, member) (unsigned long)(&((type *)0)->member) 

/*根据一个结构体变量中的一个域成员变量的指针来获取指向整个结构体变量的指针*/
#define container_of(ptr, type, member) (   \
				{ const typeof( ((type *)0)->member ) *__mptr = (ptr); \
				(type *)( (char *)__mptr - offsetof(type,member) );})

//#define unsigned int   uint32_t
#define BigLittleSwap32(A) ((((unsigned int)(A) & 0xff000000) >> 24) \
	|(((unsigned int)(A) & 0x00ff0000) >> 8) \
	|(((unsigned int)(A) & 0x0000ff00) << 8) \
	|(((unsigned int)(A) & 0x000000ff) << 24))


#define BigLittleSwap16(A) ((((unsigned short int)(A) & 0xff00) >> 8) \
	|(((unsigned short int)(A) & 0x00ff) << 8) )


// structure for download information.
typedef struct {
	unsigned int  addr_header; // Address of program image header
	unsigned int  addr_start;  // Address of program image
	unsigned int  addr_end;    // End address of program image
	unsigned int  addr_entry;  // Entry address to run program
}dl_info_t;


/*2.1.1.算法标识
签名算法值如下：
0x00：不签名
0x01：RSA-1024bit
0x02：RSA-2048bit
0x03：RSA-3072 bit
0x04：RSA-4096 bit
其他值保留，暂不支持
*/
/*
3)其中k为期望的编码后消息的八位组长度，
这里约定为签名密钥的字节单位长度。

*/
#define EM_LEN_1024b  128
#define EM_LEN_2048b  256
#define EM_LEN_3072b  384
#define EM_LEN_4096b  512



#define SFILE_FMT_BIN		0x00
#define SFILE_FMT_KERN		0x01
#define SFILE_FMT_APPELF	0x02
#define SFILE_FMT_ROOTFS	0x03

#define SFILE_TYPE_FIRMWARE_ADDR_OFFSET 0
#define SFILE_TYPE_APP_ADDR_OFFSET (1024)

/*	0x01：固件*/
#define SFILE_TYPE_FIRMWARE		0x01
/*	0x02：应用*/
#define SFILE_TYPE_APP			0x02
/*	0x03：固件公钥*/
#define SFILE_TYPE_FW_PUK       0x03
/*  0x04：验证应用公钥的厂家公钥*/
#define SFILE_TYPE_APP_MVPUK    0x04
/*  0x05：应用公钥*/
#define SFILE_TYPE_APP_PUK      0x05

enum{
    VOS_VERIFY_PUK = 1,     //VOS验签公钥
    APP_VERIFY_PUK = 2,     //APP验签公钥
    LOAD_PUKL2_PUK = 4,     //L2公钥导入验签公钥
    
};

/*
对于附录 0 中提到的六个散列函数， DigestInfo 值的 DER 编码 T 的值如下所示：
MD2: (0x)30 20 30 0c 06 08 2a 86 48 86 f7 0d 02 02 05 00 04 10 || H。
MD5: (0x)30 20 30 0c 06 08 2a 86 48 86 f7 0d 02 05 05 00 04 10 || H。 
SHA-1: (0x)30 21 30 09 06 05 2b 0e 03 02 1a 05 00 04 14 || H。
SHA-256: (0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01 05 00 04 20 || H。
SHA-384: (0x)30 41 30 0d 06 09 60 86 48 01 65 03 04 02 02 05 00 04 30 || H。
SHA-512: (0x)30 51 30 0d 06 09 60 86 48 01 65 03 04 02 03 05 00 04 40 || H。

散列算法值如下：
0x00：不签名
0x01：SHA1
0x02：SHA-224
0x03：SHA-256
0x04：SHA384
0x05：SHA512
其他值保留，暂不支持

*/
#define DigestInfo_DER_SHA1 	("\x30\x21\x30\x09\x06\x05\x2b\x0e\x03\x02\x1a\x05\x00\x04\x14")
#define ALGID_LEN_SHA1 			15
#define T_LEN_SHA1				(ALGID_LEN_SHA1+20)/*DigestInfo_DER_SHA1 len + sha1 len*/

#define DigestInfo_DER_SHA256	("\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20")
#define ALGID_LEN_SHA256 			19
#define T_LEN_SHA256			(ALGID_LEN_SHA256+32)/*DigestInfo_DER_SHA256 len + sha256 len*/

#define DigestInfo_DER_SHA384	("\x30\x41\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x02\x05\x00\x04\x30")
#define ALGID_LEN_SHA384 			19
#define T_LEN_SHA384			(ALGID_LEN_SHA384+48)/*DigestInfo_DER_SHA384 len + sha384 len*/

#define DigestInfo_DER_SHA512	("\x30\x51\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03\x05\x00\x04\x40")
#define ALGID_LEN_SHA512 			19
#define T_LEN_SHA512			(ALGID_LEN_SHA512+64)/*DigestInfo_DER_SHA512 len + sha512 len*/

//int SignVerify(uchar *addr, uint32_t size, int isSpiFlash);
int sysAppSignVerify(uchar *img_addr);
int sLoadPublickey( unsigned int filetype, char *id, signkey_t *pskt);

// structure for download information.
//int SignVerify_GetRawSize(uchar *img_addr,BIN_INFO* pbi, int isSpiFlash);

int iPedMemCmp(const void * cs, const void * ct, size_t count);

extern int ImageCalcSize(dl_info_t *image);

int WritePuk(char *filename);

void test_puk_loading(void);


#endif

