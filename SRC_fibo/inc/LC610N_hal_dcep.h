#ifndef LC610N_HAL_DCEP_H
#define LC610N_HAL_DCEP_H

#include "comm.h"

#define DCEP_LOG_LEVEL_0			LOG_LEVEL_0
#define DCEP_LOG_LEVEL_1			LOG_LEVEL_1
#define DCEP_LOG_LEVEL_2			LOG_LEVEL_2
#define DCEP_LOG_LEVEL_3			LOG_LEVEL_3
#define DCEP_LOG_LEVEL_4			LOG_LEVEL_4
#define DCEP_LOG_LEVEL_5			LOG_LEVEL_5


#define  SEIDFIELNAME                       "/ext/seidfile"
#define  PARENTWALLFILENAME                 "/ext/parentwallfile"
#define  CONTEXTIDFILENAME                  "/ext/contextIdfile"
#define  WALLETIDFILENAME                   "/ext/walletIdfile"
#define  WALLETNAMEFILENAME                 "/ext/walletnamefile"
#define  DCEPWALLFILENAME                   "/ext/dcepwallfile"
#define  ADMBLNFILENAME                     "/ext/admBlnfile"
#define  LISTBLNFILENAME                    "/ext/listBlnfile"
#define  ADMBILLFILE                        "/ext/admBillfile"
#define  LISTBILLFILENAME                   "/ext/listbillfile"
#define  ATCFILENAME                        "/ext/atcfile"
#define  POWERDOWNRECOVERY                  "/ext/pDownRecovery"
#define  BACKUPFILE                         "/ext/backupFile"
#define  BACKUPLISTFILE                     "/ext/backupListFile"

#define  DCEP_ERR_Rand_PARAM                -7500
#define  DCEP_ERR_Rand_GET                  -7501
#define  DCEP_ERR_GET_SEID_PARAM            -7502
#define  DCEP_ERR_GET_SEID                  -7503
#define  DCEP_ERR_SET_SEID_PARAM            -7504
#define  DCEP_ERR_SET_SEID                  -7505
#define  DCEP_ERR_GET_PARENT_WALL_PARAM     -7506
#define  DCEP_ERR_GET_PARENT_WALL           -7507
#define  DCEP_ERR_SET_PARENT_WALL_PARAM     -7508
#define  DCEP_ERR_SET_PARENT_WALL           -7509
#define  DCEP_ERR_GET_CID_PARAM             -7510
#define  DCEP_ERR_GET_CID                   -7511
#define  DCEP_ERR_SET_CID_PARAM             -7512
#define  DCEP_ERR_SET_CID                   -7513
#define  DCEP_ERR_GET_WALL_PARAM            -7514
#define  DCEP_ERR_GET_WALL                  -7515
#define  DCEP_ERR_SET_WALL_PARAM            -7516
#define  DCEP_ERR_SET_WALL                  -7517
#define  DCEP_ERR_GET_WALLET_NAME_PARAM     -7518
#define  DCEP_ERR_GET_WALLET_NAME           -7519
#define  DCEP_ERR_SET_WALLET_NAME_PARAM     -7520
#define  DCEP_ERR_SET_WALLET_NAME           -7521 
#define  DCEP_ERR_GET_APPTYPE_PARAM         -7522
#define  DCEP_ERR_GET_APPTYPE               -7523
#define  DCEP_ERR_SET_APPTYPE_PARAM         -7524
#define  DCEP_ERR_SET_APPTYPE               -7525
#define  DCEP_ERR_GET_WLT_TXN_PARAM         -7526
#define  DCEP_ERR_GET_WLT_TXN_CD            -7527
#define  DCEP_ERR_SET_WLT_TXN_CD            -7528
#define  DCEP_ERR_GET_WLT_CRT_TIME_PARAM    -7529
#define  DCEP_ERR_GET_WLT_CRT_TIME          -7530
#define  DCEP_ERR_SET_WLT_CRT_TIME          -7531
#define  DCEP_ERR_GET_LIMAMT_PARAM          -7532
#define  DCEP_ERR_GET_LIMAMT                -7533
#define  DCEP_ERR_SET_LIMAMT_PARAM          -7534
#define  DCEP_ERR_SET_LIMAMT                -7535
#define  DCEP_ERR_GET_WAL_STATUS_PARAM      -7536
#define  DCEP_ERR_GET_WAL_STATUS            -7537
#define  DCEP_ERR_SET_WAL_STATUS_PARAM      -7538
#define  DCEP_ERR_SET_WAL_STATUS            -7539
#define  DCEP_ERR_GET_WAL_TYPE_PARAM        -7540
#define  DCEP_ERR_GET_WAL_TYPE              -7541
#define  DCEP_ERR_SET_WAL_TYPE              -7542
#define  DCEP_ERR_GET_WAL_LEVEL_PARAM       -7543
#define  DCEP_ERR_GET_WAL_LEVEL             -7544
#define  DCEP_ERR_SET_WAL_LEVEL             -7545
#define  DCEP_ERR_GET_WAL_INFO              -7546
#define  DCEP_ERR_SET_WAL_INFO_PARAM        -7547
#define  DCEP_ERR_SET_WAL_INFO              -7548
#define  DCEP_ERR_SAVE_BAL_VOUC_PARAM       -7549
#define  DCEP_ERR_SAVE_BAL_VOUC             -7550
#define  DCEP_ERR_GET_BAL_VOUC              -7551
#define  DCEP_ERR_GET_BAL_COUNT             -7552
#define  DCEP_ERR_GET_BAL                   -7553
#define  DCEP_ERR_DEL_BAL_VOUC              -7554
#define  DCEP_ERR_DEL_ALL_BAL_VOUC          -7555
#define  DCEP_ERR_BILL_SAVE_PARAM           -7556
#define  DCEP_ERR_BILL_SAVE                 -7557
#define  DCEP_ERR_BILL_GET                  -7558
#define  DCEP_ERR_BILL_GET_COUNT            -7559
#define  DCEP_ERR_BILL_DEL                  -7560
#define  DCEP_ERR_BILL_DEL_ALL              -7561
#define  DCEP_ERR_ATC_SET                   -7562
#define  DCEP_ERR_ATC_GET                   -7563
#define  DCEP_ERR_SAVE_CS                   -7564
#define  DCEP_ERR_GET_CS                    -7565
#define  DCEP_ERR_GET_BAL_VOUC_PARAM        -7566
#define  DCEP_ERR_DEL_BAL_VOUC_PARAM        -7567
#define  DCEP_ERR_DEL_CS                    -7568
#define  DCEP_ERR_SET_WLT_CRT_TIME_PARAM    -7569
#define  DCEP_ERR_GET_DATA_NON_EXIST        -7570
#define  DCEP_ERR_GET_BAL_PARAM             -7571
#define  DCEP_ERR_BILL_GET_PARAM            -7572
#define  DCEP_ERR_BAL_GET_INDEX_PARAM       -7573
#define  DCEP_ERR_BILL_GET_INDEX_PARAM      -7574
#define  DCEP_ERR_BAL_GET_INDEX             -7575
#define  DCEP_ERR_BILL_GET_INDEX            -7576
#define  DCEP_ERR_SET_BAL_PARAM             -7577
#define  DCEP_ERR_SET_BAL                   -7578
#define  DCEP_ERR_SET_BILL_PARAM            -7579
#define  DCEP_ERR_SET_BILL                  -7580
#define  DCEP_ERR_SET_WAL_TYPE_PARAM        -7581
#define  DCEP_ERR_SET_WAL_LEVEL_PARAM       -7582

#define WALLSEID                            0
#define WALLPRTWLTID                        1
#define WALLCID                             2
#define WALLWLTID                           3
#define WALLHWNAME                          4
#define WALLAPPTYPE                         5
#define WALLWLTTXCD                         6
#define WALLTM                              7
#define WALLLIMAMT                          8
#define WALLWALSTATUS                       9
#define WALLWALTYPE                         10
#define WALLWALLEVEL                        11
#define WALLINFO                            12

#define BALDATASIZE                         4*1024
#define MAXBALAMOUNT                        300
#define BALFILENAMELENTH                    4
#define BILLDATASIZE                        4*1024
#define MAXBILLAMOUNT                       300

#define MAXCIDLENTH                         128
#define MAXINFOLENTH                        1024

#define TYPE_TRANCHAIN_INDEX                "/ext/99997"
#define TYPE_BAL_COUNT_INDEX                "/ext/99998"

#define FILENOEXIST                         -1000  

typedef struct {
    unsigned char seid[32];                 //硬件钱包信息
    unsigned char parentWltId[16];           //母钱包ID
    unsigned char CID[16];                  //钱包关联码
    unsigned char wltID[16];                 //钱包ID
    unsigned char hwName[60];               //钱包名
    unsigned char appType;                  //应用标识               
    unsigned char wltTxnCd;                 //设备种类
    unsigned char tm[8];                    //时间
    unsigned char limAmt[6];                //限额
    unsigned char walStatus[8];                
    unsigned char WalType[8];
    unsigned char walLevel[8];
    unsigned char info[64];                 //钱包控制信息 待定
    unsigned int  seidLen;
    unsigned int  parentWltIdLen;
    unsigned int  wltIDLen;
    unsigned int  hwNameLen;
    unsigned int  tmLen;
    unsigned int  limAmtLen; 
    unsigned int  CIDLen;
    unsigned int  walStatusLen;
    unsigned int  walTypeLen;
    unsigned int  walLevelLen;
    unsigned int  infoLen;
}sDCEP_wall;

typedef struct {
    unsigned char dirOneFileCounter;
    unsigned char dirTwoFileCounter;
    unsigned char dirThrFileCounter;
    unsigned char dirFouFileCounter;
    unsigned char dirFivFileCounter;
    unsigned char dirOneFlag;
    unsigned char dirTwoFlag;
    unsigned char dirThrFlag;
    unsigned char dirFouFlag;
    unsigned char dirFivFlag;
    unsigned int blnTtlCnt;
    uint64_t balanceSum;
}sDCEP_VoucherHead;

typedef struct {
    unsigned char dirOneFileCounter;
    unsigned char dirTwoFileCounter;
    unsigned char dirThrFileCounter;
    unsigned char dirFouFileCounter;
    unsigned char dirFivFileCounter;
    unsigned char dirOneFlag;
    unsigned char dirTwoFlag;
    unsigned char dirThrFlag;
    unsigned char dirFouFlag;
    unsigned char dirFivFlag;
    unsigned int  billTtlCnt;
}sDCEP_BillHead;


typedef struct{
    unsigned char voucherSaveOpt;
    unsigned char voucherSetOpt;
    unsigned char voucherVGOpt;
    unsigned char voucherDelAllOpt;
    unsigned char voucherIdentifyFolderOpt;
    unsigned char billSaveOpt;
    unsigned char billSetOpt;
    unsigned char billVGOpt;
    unsigned char billDelAllOpt;
    unsigned char billIdentifyFolderOpt;
    unsigned char currencyStringSaveOpt;
    unsigned int voucherFileCnt;
    unsigned int billFileCnt;
    char PDOptFileName[19];
}DCEP_pwrDownOpt;

struct TLVNode{
	uint16_t Tag;				/*	T 	*/
	uint16_t Length;			/*	L 	*/
	unsigned char* Value;		/*	V 	*/
	unsigned char TagSize;
	unsigned char LengthSize;
	uint16_t MoreFlag;			/* Used In Sub */
	uint16_t SubFlag;			/* have Sub Node? */
	uint16_t SubCount;		
	struct TLVNode* Sub[256];
	struct TLVNode* Next;
};


/**
 * 解析数据,生成TLV结构
 * @param  buf  [数据]
 * @param  size [数据长度]
 * @return      [TLVNode]
 */
struct TLVNode* TLV_Parse(unsigned char* buf,int size);

/**
 * 合并src结构到target
 * @param target [目标TLVNode]
 * @param src    [源TLVNode]
 */
void TLV_Merge(struct TLVNode* target,struct TLVNode* src);

/**
 * 在node中查找tag.
 * @param  node [TLVNode]
 * @param  tag  [tag标签]
 * @return      [NULL - 未找到]
 */
struct TLVNode* TLV_Find(struct TLVNode* node,uint16_t tag);

/**
 * Free TLVNode Memory
 * @param node [node]
 */
void TLV_Free(struct TLVNode* node);

/**
 * DEBUG TLVNode
 * @param node [description]
 */
void TLV_Debug(struct TLVNode* node);

#endif