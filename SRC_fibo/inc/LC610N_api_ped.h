#ifndef LC610N_API_PED_H
#define LC610N_API_PED_H


#include "comm.h"

//#define PRINT_API_CMD


#define   PED_TLK 0x01//0x1100 //TLK 
#define   PED_TMK 0x10//0x2100 //TMK 
#define   PED_TPK 0x21//0x3101 //PIN Key 
#define   PED_TAK 0x22//0x3102 //MAC Key 
#define   PED_TDK 0x23//0x3103 //DES Key 
#define   PED_TIK 0x40//0x3310 //DUKPT Key 


#define  ushort unsigned short
#define  uchar  unsigned char

#pragma pack(1)
/**
密钥信息，用于写入密钥信息时传入密钥的相关信息
*/
typedef struct
{
    /** 发散该密钥的源密钥的密钥类型，
    PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK,
    不得低于ucDstKeyType所在的密钥级别*/
    unsigned char ucSrcKeyType;
    /** 发散该密钥的源密钥索引，
    索引一般从1开始，如果该变量为0，
    则表示这个密钥的写入是明文形式*/
    unsigned short ucSrcKeyIdx;
    /** 目的密钥的密钥类型，
    PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK
    */
    unsigned char ucDstKeyType;
    /*目的密钥索引*/
    unsigned short ucDstKeyIdx;
    /**目的密钥长度，8,16,24*/
    int iDstKeyLen;
    /**写入密钥的内容*/
    unsigned char aucDstKeyValue[24];
}ST_KEY_INFO;
#pragma pack()
#pragma pack(1)
typedef struct  _ST_KEY_INFO_EX
{
    /* 发散该密钥的源密钥的密钥类型，
    PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK,
    不得低于ucDstKeyType所在的密钥级别*/
    unsigned char ucSrcKeyType;
    /* 发散该密钥的源密钥索引，
    索引一般从1开始，如果该变量为0，
    则表示这个密钥的写入是明文形式*/
    unsigned short ucSrcKeyIdx;
    /* 目的密钥的密钥类型，
    PED_GMK,PED_TMK,PED_TPK,PED_TAK,PED_TDK
    */
    unsigned char ucDstKeyType;
    /*目的密钥索引*/
    unsigned short ucDstKeyIdx;
    /*目的密钥长度，8,16,24*/
    int iDstKeyLen;
    /*写入密钥的内容*/
    unsigned char aucDstKeyValue[24];
    /*写入密钥的模式，加密，解密，des or gm*/
    unsigned char ucWriteMode;
} ST_KEY_INFO_EX;
#pragma pack()

#pragma pack(1)
/**密钥的kcv校验信息*/
typedef struct
{
    /**
    KCV 模式
    */
    int iCheckMode;
    /**
    kcv数据，根据iCheckMode进行解析
    */
    unsigned char aucCheckBuf[128];
} ST_KCV_INFO;
#pragma pack()


//密钥数据结构
typedef struct{  
	uchar ucSrcKeyType;  /* 发 散 该 密 钥 的 源 密 钥 的 密 钥 类 型 , PED_TLK,PED_TMK. */ 
	uchar ucDstKeyType;  /* 目的密钥类型：PED_TAESK */ 
	uchar ucSrcKeyIdx;   /*发散该密钥的源密钥索引,索引一般从1开始,如果该变 量为 0,则表示这个密钥的写入是明文形式*/ 
	uchar ucDstKeyIdx;   /*目的密钥索引*/ 
	int iDstKeyLen;   /* 目的密钥长度，16,24,32 */ 
	uchar aucDstKeyValue[32];  /*目的密钥内容*/ 
}ST_AES_KEY_INFO; 


typedef struct { 
	int iModulusLen;       /* 模位数长度 the length of modulus bits 。 */ 
	uchar aucModulus[512]; /* 模 , 模长小于 512字节时 , 靠右存储 , 左补 0x00 。 */ 
	int iExponentLen;      /* 指数位长度 */ 
	uchar aucExponent[512];/* 指数 , 小于 512字节时 , 左补 0x00 。 */ 
	uchar aucKeyInfo[128]; /* 密钥信息 , 由应用自定义。 */ 
}ST_RSA_KEY; 


//PED 模块的错误代码范围为-4500  ~ -4999
#define PED_RET_OK 0 //D 函数返回正确 
#define PED_RET_ERR_NO_KEY -4501 //不存在 
#define PED_RET_ERR_KEYIDX_ERR -4502 //索引错，参数索引不在范围内 
#define PED_RET_ERR_DERIVE_ERR -4503 //写入时，源密钥类型错或层次比目 的密钥低 
#define PED_RET_ERR_CHECK_KEY_FAIL   -4504 //验证失败 
#define PED_RET_ERR_NO_PIN_INPUT -4505 //没输入 PIN 
#define PED_RET_ERR_INPUT_CANCEL  -4506 //用户取消输入 PIN 
#define PED_RET_ERR_WAIT_INTERVAL   -4507 //函数调用小于最小间隔时间 
#define PED_RET_ERR_CHECK_MODE_ERR -4508 //KCV 模式错，不支持 
#define PED_RET_ERR_NO_RIGHT_USE   -4509 //无权使用该密钥，PED 当前密钥标签值 和要使用的密钥标签值不相等 
#define PED_RET_ERR_KEY_TYPE_ERR   -4510  //密钥类型错 
#define PED_RET_ERR_EXPLEN_ERR    -4511   //期望 PIN 的长度字符串错 
#define PED_RET_ERR_DSTKEY_IDX_ERR -4512   //目的密钥索引错，不在范围内 
#define PED_RET_ERR_SRCKEY_IDX_ERR -4513   //源密钥索引错，不在范围内或者写入密  钥时，源密钥类型的值大于目的密钥类型，都 会返回该密钥 
#define PED_RET_ERR_KEY_LEN_ERR   -4514  //密钥长度错 
#define PED_RET_ERR_INPUT_TIMEOUT   -4515 //输入 PIN 超时 
#define PED_RET_ERR_NO_ICC           -4516 //IC 卡不存在 
#define PED_RET_ERR_ICC_NO_INIT     -4517  //IC 卡未初始化 
#define PED_RET_ERR_GROUP_IDX_ERR   -4518 //DUKPT 组索引号错 
#define PED_RET_ERR_PARAM_PTR_NULL  -4519 //指针参数非法为空 
#define PED_RET_ERR_TAMPERED  -4520 //PED 已受攻击 
#define PED_RET_ERROR  -4521 //PED 通用错误 
#define PED_RET_ERR_NOMORE_BUF -4522 //没有空闲的缓冲 
#define PED_RET_ERR_NEED_ADMIN -4523 //需要取得高级权限 
#define PED_RET_ERR_DUKPT_OVERFLOW -4524 //DUKPT 已经溢出 
#define PED_RET_ERR_KCV_CHECK_FAIL -4525 //KCV 校验失败 
#define PED_RET_ERR_SRCKEY_TYPE_ERR -4526 //写入密钥时，源密钥 id 的密钥类型和 源密钥类型不匹配 
#define PED_RET_ERR_UNSPT_CMD  -4527 //命令不支持 
#define PED_RET_ERR_COMM_ERR  -4528 //通讯错误 
#define PED_RET_ERR_NO_UAPUK  -4529 //没有用户认证公钥 
#define PED_RET_ERR_ADMIN_ERR -4530 //取系统敏感服务失败 
#define PED_RET_ERR_DOWNLOAD_INACTIVE -4531 //PED 处于下载非激活状态 
#define PED_RET_ERR_KCV_ODD_CHECK_FAIL  -4532 //KCV 奇校验失败 
#define PED_RET_ERR_PED_DATA_RW_FAIL  -4533 //读取 PED 数据失败 
#define PED_RET_ERR_ICC_CMD_ERR -4534 //IC 卡操作错误脱机明文、密文密码验 证 
#define PED_RET_ERR_KEY_VALUE_INVALID -4535 //写入的密钥全零或者，有组分相等， 16/24 字节密钥存在两个组分相等的情 况 
#define PED_RET_ERR_KEY_VALUE_EXIST -4536 //已存在相同的密钥值的密钥 
#define PED_RET_ERR_UART_PARAM_INVALID -4537 //串口参数不支持  
#define PED_RET_ERR_KEY_INDEX_NOT_SELECT_OR_NOT_MATCH  -4538 //密钥索引没有选择或者和选择的密钥 索引不相等 
#define PED_RET_ERR_INPUT_CLEAR -4539 //用户按 CLEAR 键退出输入 PIN 
#define PED_RET_ERR_LOAD_TRK_FAIL  -4540  
#define PED_RET_ERR_TRK_VERIFY_FAIL  -4541  
#define PED_RET_ERR_MSR_STATUS_INVALID  -4542  
#define PED_RET_ERR_NO_FREE_FLASH -4543  
#define PED_RET_ERR_DUKPT_NEED_INC_KSN  -4544 //DUKPT KSN 需要先加 1 
#define PED_RET_ERR_KCV_MODE_ERR  -4545 //KCV MODE 错误 
#define PED_RET_ERR_DUKPT_NO_KCV  -4546 //NO KCV 
#define PED_RET_ERR_PIN_BYPASS_BYFUNKEY -4547 //按 FN/ATM4 键取消 PIN 输入 
#define PED_RET_ERR_MAC_ERR     -4548 //数据 MAC 校验错 
#define PED_RET_ERR_CRC_ERR     -4549 //数据 CRC 校验错 
#define PED_RET_DATA_LEN_ERR     -4550 //数据长度错 
#define PED_RET_ERR_MODIFY_PWDA1 -4551  
#define PED_RET_ERR_MODIFY_PWDA2 -4552  
#define PED_RET_ERR_MODIFY_PWDB1 -4553  
#define PED_RET_ERR_MODIFY_PWDB2 -4554  
#define PED_RET_ERR_PWDA_EQU_PWDB -4555  
#define PED_RET_ERR_ADMIN_LOGA_ERR -4556  
#define PED_RET_ERR_ADMIN_LOGB_ERR -4557  
#define PED_RET_ERR_FORCE_MODIFY -4558  
#define PED_RET_ERR_VERFIY_PWDA -4559  
#define PED_RET_ERR_VERFIY_PWDB -4560  
#define PED_RET_ERR_TLK_ILLEGAL -4561  
#define PED_RET_ERR_TLK_ALL_ZERO -4562  
#define PED_RET_ERR_TLK_XOR_ALL_ZERO -4563  
#define PED_RET_ERR_TLK_NO_EQUAL -4564  
#define PED_RET_ERR_TRK_ILLEGAL -4565  
#define PED_RET_ERR_TRK_ALL_ZERO -4566  
#define PED_RET_ERR_TRK_XOR_ALL_ZERO -4567  
#define PED_RET_ERR_TRK_NO_EQUAL -4568  
#define PED_RET_ERR_PINBLOCK_FREQUENT -4569  
#define PED_RET_ERR_KEY_KCV_TAB_NULL  -4575  
#define PED_RET_ERR_PED_CFG_RW_FAIL -4576 //读取 PED 配置数据失败 
#define PED_RET_NOT_INIT  -4577 //PED 未初始化 
#define PED_RET_NOT_FMT  -4578 //PED 未格式化 
#define PED_RET_ERR_MODE_ERR  -4579 //PED 模式错误 
#define PED_RET_ERR_DSTKEY_TYPE_ERR -4580 //目的密钥 id 的密钥类型和目的密钥类 型不匹配 
#define PED_RET_ERR_LOG_OPEN_FILE_ERR -4581  
#define PED_STATE_ERR -4582  
#define PED_RET_CALC_ERR -4583 //PED 计算失败 
#define PED_RET_ERR_EXPLENIN_ERR -4584 //期望 PIN 的字符串错误 
#define PED_USER_NO_PRE_OPERATE -4585 //前置操作不正确 
#define PED_USER_ERASE_ERR -4590 //FLASH 擦除错误 
#define PED_USER_NO_LOG_CODE -4591 //无有效挑战码 
#define PED_USER_NO_FOUND -4592 //找不到用户 
#define PED_USER_NO_LOGIN -4593 //用户未登录 
#define PED_USER_RECORD_ERR -4594 //用户记录错误 
#define PED_USER_PWD_INPUT_ERROR -4595 //输入不正确 
#define PED_CERT_INDEX_ERR -4600 //证书索引号无效 
#define PED_CERT_NO_FOUND_ERR -4601 //证书不存在 
#define PED_CERT_LEN_ERR -4602 //证书数据长度不正确 
#define PED_CERT_WAIT_READ -4603 //证书有剩余数据等待读取 
#define PED_CERT_WAIT_WRITE -4604 //证书有剩余数据等待写入 
#define PED_RET_ERR_NO_MODIFY -4610  
#define PED_RET_ERR_TYPE -4615  
#define PED_RET_ERR_INJECTKEY_LEN -4616  
#define PED_RET_ERR_AESKEY_LEN -4617  
#define PED_RET_ERR_AES -4618  
#define PED_CALC_TYPE_NO_SUPPORT -4619  


int pedWriteKey_lib(ST_KEY_INFO * KeyInfoIn, ST_KCV_INFO * KcvInfoIn);

int pedWriteKeyEx_lib(ST_KEY_INFO_EX * KeyInfoIn, ST_KCV_INFO * KcvInfoIn);

int pedWriteTiK_lib(uchar GroupIdx, uchar SrcKeyIdx, uchar KeyLen, uchar * KeyValueIn, uchar * KsnIn, ST_KCV_INFO * KcvInfoIn);

int pedWriteMK_lib(uchar key_no, uchar key_len, uchar *key_data, ST_KCV_INFO *KcvInfoIn, uchar mode, uchar mkey_no);

int pedWriteWK_lib(uchar key_no, uchar key_len, uchar *key_data, ST_KCV_INFO *KcvInfoIn, uchar mode, uchar mkey_no);

int pedKeyExist_lib(unsigned char ucKeyType, unsigned short usKeyID);

int pedGetMac_lib(uchar KeyIdx, uchar *DataIn, ushort DataInLen, uchar *MacOut, uchar Mode);

int pedCalcSym_lib (unsigned char key_no, unsigned char key_part, unsigned char *indata, unsigned char *outdata, unsigned char mode);
int pedBigDataCalcSym_lib(unsigned char key_no, unsigned char key_part, unsigned char *indata, unsigned short usDataLen, unsigned char *outdata, unsigned char mode);
int pedGetDukptKSN_lib(uchar GroupIdx,  uchar * KsnOut);

int pedDukptIncreaseKsn_lib(uchar GroupIdx);

int pedGetMacDukpt_lib(uchar GroupIdx, uchar* DataIn, ushort DatainLen, uchar * MacOut, uchar * KsnOut, uchar Mode);

int pedDukptDes_lib(uchar GroupIdx, uchar KeyVarType, uchar * pucIV, ushort DataInLen, uchar * DataIn, uchar * DataOut, uchar * KsnOut , uchar Mode);

int pedGetKcv_lib(uchar KeyType, uchar KeyIdx, ST_KCV_INFO *KcvInfoOut);

int pedWriteRSAKey_lib (uchar RSAKeyIndex, ST_RSA_KEY* pstRsakeyIn);

int PeCalcRSA_lib(uchar RSAKeyIndex, uchar *pucDataIn, ushort DataInLen, uchar * pucDataOut, uchar* pucKeyInfoOut);

int pedErase_lib(void);

int pedGetVer_lib(uchar * VerInfoOut);

int pedGetPinBlock_lib(unsigned char ucKeyIdx, 
                  unsigned char *pucExpPinLenIn,  
                  unsigned char *pucDataIn, unsigned short uiDataInLen, 
                  unsigned char *pucPinBlockOut, 
                  unsigned char ucMode,   unsigned int uiTimeOutMs); 
int pedGetPinDukpt_lib(unsigned char ucGroupIdx, unsigned char *pucExpPinLenIn,	unsigned char *pucDataIn, unsigned short uiDataInLen, unsigned char *pucKsnOut, 
			unsigned char *pucPinBlockOut, unsigned char ucMode, unsigned int uiTimeOutMs);

#endif
