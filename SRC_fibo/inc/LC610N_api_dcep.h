#ifndef LC610N_API_DCEP_H
#define LC610N_API_DCEP_H

#include "comm.h"

#define  ECB    0
#define  CBC    1
#define ERR_DCEP_OK 		0 //成功

#define ERR_DCEP_PARAM 		-1601 //参数不正确
#define ERR_DCEP_CERT_DIR   -1602 //创建证书目录失败
#define ERR_DCEP_CERT_WRITE -1603 //写入证书失败
#define ERR_DCEP_CERT_READ  -1604 //读取证书失败
#define ERR_DCEP_PRIKEY_WRITE 	-1605 //写入证书私钥失败
#define ERR_DCEP_PRIKEY_READ  	-1606 //读取证书私钥失败
#define ERR_DCEP_SM4KEY_WRITE 	-1607 //写SM4密钥失败
#define ERR_DCEP_SM4KEY_READ  	-1608 //读取SM4密钥失败
#define ERR_DCEP_FILE_NOTEXITST -1609 //文件不存在
#define ERR_DCEP_CERT_GETINFO   -1610 //解析证书错误
#define ERR_DCEP_CERT_VERIFY    -1611 //验签失败



#define ERR_DCEP_ERR   		-1699 //未知错误

#define LISTBLNFILELEN      sizeof(uint32_t)*MAXBALAMOUNT
#define LISTBILLFILELEN      sizeof(uint32_t)*(MAXBILLAMOUNT+1)
/*
*@Brief:		解析证书
*@Param IN:		[in]pbInBuffer	 证书16进制数据
				[in]iLen	     证书数据长度
*@Param OUT:	[out]pbPKout     证书64字节公钥数据，如果不解析传NULL
                [out]piLenout    证书公钥长度，如果不解析传NULL
                [out]pbtbsCer    证书tbs数据，如果不解析传NULL
                [out]pitbsCerLen 证书tbs长度，如果不解析传NULL
                [out]pbSig    	 证书64字节签名值数据，如果不解析传NULL
                [out]piSigLen    证书签名值长度，如果不解析传NULL
*@Return:		true:成功; false:失败
*/
bool getInfoFormCertBuf(unsigned char * pbInBuffer, int iLen, unsigned char * pbPKout, int* piLenout, unsigned char * pbtbsCer, int* pitbsCerLen, unsigned char * pbSig, int* piSigLen);


/*
*@Brief:		写入证书
*@Param IN:		[in]index	索引号  (目前索引999为个人证书)
				[in]certBuffer	证书内容
				[in]len	证书长度				
*@Param OUT:	
*@Return:		>=0:成功; <0:失败
*/
int setCert(int iIndex, UINT8* pbTlvData, int iLen);

/*
*@Brief:		读取证书
*@Param IN:		[in]iIndex	索引号  (目前索引999为个人证书)
*@Param OUT:	[out]pbOut 证书数据,如果输入NULL为获取证书数据长度,返回证书数据长度
*@Return:		>0成功 为证书数据长度,	 <0	错误
*/
int getCert(int iIndex, uint8 *pbOut);

/*
*@Brief:		生成P10证书请求数据
*@Param IN:		[in]PCX500NameSubject	证书主体信息
*@Param OUT:	[out]pbOut 返回证书请求数据
*@Return:		>0成功,返回数据长度,	<0 错误
*/
int getP10CertReq(char* PCX500NameSubject, uint8 *pbOut);

/*
*@Brief:		用指定索引的证书校验传入的证书
*@Param IN:		[in]iIndex	上级证书索引
				[in]pbCert	待校验证书
				[in]iCertLen	待校验证书长度
*@Param OUT:	
*@Return:		0	成功    <0	失败
*/
int verifyCert(int iIndex, uint8* pbCert, int iCertLen);
/*
*@Brief:		用指定索引的证书校验传入的数据的签名
*@Param IN:		[in]index	证书索引
				[in]sign	待校验数据的签名数据
				[in]iSignLen 待校验数据的签名数据长度 64字节
				[in]data	待校验数据
				[in]len	数据长度
*@Param OUT:	
*@Return:		0	成功    <0	失败
*/
int verifyByCert(int iIndex, uint8* pbSign, int iSignLen, uint8* pbData, int iLen);
/*
*@Brief:		用指定索引的证书对数据签名
*@Param IN:		[in]index	证书索引
				[in]data	待签名数据
				[in]len	数据长度
*@Param OUT:	[out]pcOut	签名值数据
*@Return:		>	成功 返回签名值数据长度   
                <0	失败
*/
int signByCert(int iIndex, uint8* pbData, int iLen, uint8 *pbOut);

/*
*@Brief:		用证书对数据加密 
*@Param IN:		[in]iMode	加密模式 0：C1C2C3      	1：C1C3C2 目前只支持1
				[in]pbCert	证书数据
				[in]iCertLen	证书数据长度
				[in]pbData	待加密数据
				[in]iLen	    待加密数据数据长度，不大于1024字节
*@Param OUT:	[out]pbOut	    密文数据,分配的空间要大于iLen+96字节
*@Return:		>0 成功,返回密文数据长度     
                <0 失败
*/
int encrypByCert(int iMode, uint8* pbCert, int iCertLen, uint8* pbData, int iLen, uint8 *pbOut);

/*
*@Brief:		用指定证书对数据解密
*@Param IN:		[in]mode	解密模式 0：C1C2C3      	1：C1C3C2 目前只支持1
				[in]index	证书索引
				[in]data	密文数据
				[in]iLen	    数据长度，不大于1024+96
*@Param OUT:	[out]pcOut	    明文数据，要分配大于iLen-96字节空间
*@Return:		>0 成功,返回明文数据长度
				<0 失败
*/
int decrypByCert(int iMode, int iIndex, uint8* pbData, int iLen, uint8* pbOut);
/**
 * sm2非对称加密算法
 *
 * @param key		密钥数据。
 *					Sm2算法的私钥长度为32字节，公钥长度为64字节。
 *					Mode=0时，Key为96字节，数据结构为：64字节公钥（X+Y） +32字节私钥。
 *					Mode=1时，Key为64字节公钥（X+Y）
 * @param keyLen    密钥数据长度
 * @param input 	待加密或解密的数据
 * @param inputLen  待加密或解密的数据长度,不大于1024字节
 * @param output    加密或解密后的数据
 * @param mode		0x00：使用SM2 私钥解密数据
 *					0x01：使用SM2公钥加密数据
 * @return 大于0表示成功，此时返回值表示加密或解密后的数据长度，小于0失败
 */
int sm2(uint8* pbKey, int iKeyLen, uint8* pbInput, int iInputLen, uint8* pbOutput, uint8 bMode);
/**
 * 使用sm2算法签名数据
 *
 * @param user_id	  签名者信息
 * @param user_id_len 签名者信息长度,长度最长为32字节
 * @param public_key  公钥数据，64字节
 * @param pbPrivate_key 私钥数据，32字节
 * @param pbMsg		  待签名数据
 * @param msg_len	  待签名数据长度，不大于1024字节
 * @param out_buf	  输出缓冲区
 * @return 大于0表示成功,并返回签名数据长度，小于0失败
 */
int sm2Sign(uint8* pbUser_id, int iUser_id_len, uint8* pbPublic_key, uint8* pbPrivate_key, uint8* pbMsg, int iMsg_len, uint8* pbOut_buf);
/**
 * 使用sm2算法验证签名
 *
 * @param user_id	  签名者信息
 * @param user_id_len 签名者信息长度,长度最长为32字节
 * @param public_key  公钥数据，64字节
 * @param signed_data 签名信息，64字节
 * @param msg		  待签名数据
 * @param msg_len	  待签名数据长度，不大于1024字节
 * @return =0表示成功，小于0失败
 */
int sm2Verify(uint8* pbUser_id, int iUser_id_len, uint8* pbPublic_key, uint8* pbSigned_data,	 uint8* pbMsg, int iMsg_len); 
/**
* sm3散列算法，获取不大于1024字节的数据的32字节hash值
*
* @param input	 待散列数据，不大于1024字节
* @param input_len 待散列数据长度
* @param out_buf	 输出缓冲区，不小于32字节
* @return 大于0表示成功,返回结果数据长度，小于0失败
*/
int sm3(uint8* pbInput, int iInput_len, uint8* pbOut_buf);

/**
 * sm4对称分组加密算法，加密块与秘钥均为16字节，每次加密的数据量最大为1024字节，
 * 当待加密数据大于1024字节且mode为ECB时，可以分多次调用
 *
 * @param input 	待加密数据，必须为16字节的整数倍，不大于1024字节
 * @param input_len 待加密数据长度
 * @param out_buf	加密数据缓冲区，由于sm1为对称分组算法，此缓冲区长度应大于等于待加密数据长度
 * @param pbKey 	16字节秘钥
 * @param vector	初始化向量，16字节，当mode为ECB时，可以为null
 * @param mode		模式，0为ECB解密，1为ECB加密，2为CBC解密，3位CBC加密
 * @return 大于0表示成功,返回数据长度，小于0失败
 */
int sm4(uint8* pbInput, int iInput_len, uint8* pbOut_buf, uint8* pbKey, uint8* pbVector, int iMode);
/*
*@Brief:		保存SM4密钥 
*@Param IN:		[in] index	保存的索引
				[in]key	    16字节密钥
				[in]iKeyLen	密钥长度16字节
*@Param OUT:	
*@Return:	>0成功，返回写入密钥长度，<0失败	
*/
int writeSm4Key(int iIndex, uint8* pbKey, int iKeyLen);
/*
*@Brief:		用保存的SM4密钥CBC加密 
*@Param IN:		[in]iIndex	密钥索引
				[in]iv	初始向量 16字节
				[in]data	待加密数据
				[in]iLen	数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pbOut	密文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回密文数据长度，小于等于0失败
*/
int sm4CBCEncryp(int iIndex, uint8* pbIv, uint8* pbData, int iLen, uint8* pbOut);
/*
*@Brief:		用保存的SM4密钥CBC解密 
*@Param IN:		[in] iIndex	密钥索引
				[in]pbIv	初始向量  16字节
				[in]data	待解解数据 
				[in]len	    数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pbOut	明文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回明文数据长度，小于0失败
*/
int sm4CBCDecryp(int iIndex, uint8* pbIv, uint8* pbData, int iLen, uint8* pbOut);
/*
*@Brief:		用保存的SM4密钥ECB加密 
*@Param IN:		[in] iIndex	密钥索引
				[in]data	待加密数据
				[in]iLen	数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pbOut	密文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回密文数据长度，小于0失败
*/
int sm4ECBEncryp(int iIndex, uint8* pbData, int iLen, uint8* pbOut);
/*
*@Brief:		用保存的SM4密钥ECB解密 
*@Param IN:		[in] index	密钥索引
				[in]data	待解密数据
				[in]len	数据长度 必须为16字节的整数倍，不大于1024字节
*@Param OUT:	[out]pcOut	明文数据，空间不能小于数据长度len
*@Return:		大于0表示成功,返回明文数据长度，小于0失败
*/
int sm4ECBDecryp(int iIndex, uint8* pbData, int iLen, uint8* pbOut);

/*
*@Brief:		生成硬件随机数
*@Param IN:		uiLen:  获取随机数的长度
*@Param OUT:	pbData: 随机数缓存地址
*@Return:		>0:成功,获取到的字节数; <0:失败
*/
int getRandom(uint8_t* pbData, int uiLen);

/*
*@Brief:		读取硬件钱包信息
*@Param IN:		NULL
*@Param OUT:	pcSeid: 获取的pcSeid内容，长度最大32字节
*@Return:		>0:成功,读取的pcSeid长度; <0:失败
*/
int getSEId(char *pcSeid);

/*
*@Brief:		设置硬件钱包SEID信息
*@Param IN:		pcSeid:设置的硬件钱包SEID; uiLen硬件钱包长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setSEId(char *pcSeid, uint32_t uiLen);

/*
*@Brief:		获取母钱包ID
*@Param IN:		NULL
*@Param OUT:	parentWltId: 获取的母钱包ID
*@Return:		>0:成功,母钱包ID长度; <0:失败
*/
int getParentWalletId(uint8_t* pbParentWltId);

/*
*@Brief:		设置母钱包ID
*@Param IN:		parentWltId：母钱包ID（16字节）; uiLen:parentWltId长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setParentWalletId(uint8_t* pbParentWltId, uint32_t uiLen);

/*
*@Brief:		获取钱包关联码
*@Param IN:		NULL
*@Param OUT:	pbCID: 钱包关联码
*@Return:		>0:成功,CID字节数; <0:失败
*/
int getContextId(uint8_t* pbCID);

/*
*@Brief:		设置钱包关联码
*@Param IN:		pbCID：钱包关联码; uiLen:pbCID长度,uiLen<16
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setContextId(uint8_t* pbCID, uint32_t uiLen);

/*
*@Brief:		获取钱包ID
*@Param IN:		NULL
*@Param OUT:	pbWltID: 读取到的硬件钱包ID
*@Return:		>0:成功,钱包ID长度; <0:失败
*/
int getWalletId(uint8_t* pbWltID);

/*
*@Brief:		设置钱包ID
*@Param IN:		pbWltID: 钱包ID; uiLen:pbWltID长度，uiLen<16
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletId(uint8_t* pbWltID, uint32_t uiLen);

/*
*@Brief:		获取钱包名字
*@Param IN:		NULL
*@Param OUT:	pcHwName: 钱包名,最大60字节
*@Return:		>0:成功,返回钱包名字字节数; <0:失败
*/
int getWalletName(char *pcHwName);

/*
*@Brief:		设置钱包名
*@Param IN:		pcHwName: 钱包名,最大60字节;   uiLen：pcHwName长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletName(char *pcHwName, uint32_t uiLen);

/*
*@Brief:		获取应用标识
*@Param IN:		NULL
*@Param OUT:	pAppType: 标识内容
*@Return:		>0:成功,返回标识内容字节数; <0:失败
*/
int getAppType(uint8_t* pbAppType);

/*
*@Brief:		设置应用标识
*@Param IN:		bAppType：应用方标识,1字节
*@Param OUT:	NULL
*@Return:		=0:成功,返回写入字节个数; <0:失败
*/
int setAppType(uint8_t bAppType);

/*
*@Brief:		获取钱包设备种类
*@Param IN:		NULL
*@Param OUT:	pbWltTxnCd: 设备种类（1字节）
*@Return:		>0:成功,返回钱包设备种类字节数; <0:失败
*/
int getWltTxnCd(uint8_t* pbWltTxnCd);

/*
*@Brief:		设置钱包设备种类
*@Param IN:		bWltTxnCds：钱包设备种类,1字节
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWltTxnCd(uint8_t bWltTxnCd);

/*
*@Brief:		获取钱包开立时间
*@Param IN:		NULL
*@Param OUT:	pbTm: 钱包开立时间(ISO 时间：YYYYMMDDHHMMSS)
*@Return:		>0:成功,返回时间节数; <0:失败
*/
int getCreateTime(uint8_t * pbTm);

/*
*@Brief:		设置钱包开立时间
*@Param IN:		pbTm：钱包开立时间(ISO 时间：YYYYMMDDHHMMSS); uiLen：pbTm长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setCreateTime(uint8_t * pbTm, uint32_t uiLen);

/*
*@Brief:		获取钱包限额
*@Param IN:		NULL
*@Param OUT:	pbLimAmt：钱包额度（6字节）
*@Return:		>0:成功,返回字节数; <0:失败
*/
int getWalletLimit(uint8_t* pbLimAmt);

/*
*@Brief:		设置钱包额度
*@Param IN:		pbLimAmt：钱包限额; uiLen: pbLimAmt长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletLimit(uint8_t * pbLimAmt, uint32_t uiLen);

/*
*@Brief:		获取钱包状态
*@Param IN:		NULL
*@Param OUT:	pbWalStatus:钱包状态
*@Return:		>0:成功; <0:失败
*/
int getWalletStatus(uint8_t* pbWalStatus);

/*
*@Brief:		设置钱包状态
*@Param IN:		pbWalStatus：钱包状态; uiLen：pbWalStatus长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletStatus(uint8_t * pbWalStatus, uint32_t uiLen);

/*
*@Brief:		获取钱包类型
*@Param IN:		NULL
*@Param OUT:	pbWalType：钱包类型
*@Return:		>0:成功; <0:失败
*/
int getWalletType(uint8_t* pbWalType);

/*
*@Brief:		设置钱包类型
*@Param IN:		pbWalType：钱包类型; uiLen:pbWalType长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletType(uint8_t* pbWalType, uint32_t uiLen);

/*
*@Brief:		获取钱包等级
*@Param IN:		NULL
*@Param OUT:	pbWalLevel：钱包等级
*@Return:		>0:成功; <0:失败
*/
int getWalletLevel(uint8_t* pbWalLevel);

/*
*@Brief:		设置钱包等级
*@Param IN:		pbWalLevel：钱包等级; uiLen：pbWalLevel长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setWalletLevel(uint8_t *pbWalLevel, uint32_t uiLen);

/*
*@Brief:		获取钱包控制信息
*@Param IN:		NULL
*@Param OUT:	pbInfo：钱包控制信息
*@Return:		>0:成功,返回钱包控制信息字节数; <0:失败
*/
int getCtrlInfo(uint8_t* pbInfo);

/*
*@Brief:		设置钱包控制信息
*@Param IN:		info：钱包控制信息;uiLen:info长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setCtrlInfo(uint8_t* pbInfo, uint32_t uiLen);

/*
*@Brief:		保存一个余额凭证
*@Param IN:		bMode：0：余额凭证，1：币串;pbBalVoucher:余额凭证数据; uiLen:存储长度
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int saveBalVoucher(uint8_t bMode, uint8_t * pbBalVoucher, uint32_t uiLen);

/*
*@Brief:		读取指定位置的余额凭证
*@Param IN:		bMode：0：余额凭证，1：币串; uiIndex：索引
*@Param OUT:	pbBalData:余额凭证数据
*@Return:		>0:成功,余额凭证数据长度; 其他:失败
*/
int getBalVoucher(uint8_t bMode, uint32_t uiIndex, uint8_t* pbBalData);

/*
*@Brief:		获取余额凭证个数
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功,余额凭证个数; <0:失败
*/
int getBalVoucherCount();

/*
*@Brief:		获取余额
*@Param IN:		NULL
*@Param OUT:	pbBalData:余额
*@Return:		>0：成功,返回余额字节数; <:失败
*/
int getBalance(uint8_t*  pbBalData);

/*
*@Brief:		删除指定位置的余额凭证
*@Param IN:		uiIndex：索引
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteBalVoucher(uint32_t uiIndex);

/*
*@Brief:		删除全部余额凭证
*@Param IN:		bMode：0：余额凭证，1：币串
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteAllBalVoucher(uint8_t bMode);

/*
*@Brief:		保存交易记录
*@Param IN:		pbBill:交易记录数据; uiLen:交易记录长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int saveBill(uint8_t* pbBill, uint32_t uiLen);

/*
*@Brief:		读取交易记录
*@Param IN:		uiPos：索引
*@Param OUT:	pbBillData:交易记录数据
*@Return:		>0:成功,交易记录数据长度; 其他:失败
*/
int getBill(uint32_t uiPos, uint8_t* pbBillData);

/*
*@Brief:		获取交易记录个数
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功,交易记录个数; <0:失败
*/
int getBillCount();

/*
*@Brief:		删除指定位置的交易记录
*@Param IN:		uiPos：索引
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteBill(uint32_t uiPos);

/*
*@Brief:		删除所有交易记录
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int deleteAllBill();

/*
*@Brief:		写入ATC值
*@Param IN:		atc：atc值
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setATC(long lAtc);

/*
*@Brief:		获取ATC值
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功，ATC值; <0:失败
*/
long getATC();

/*
*@Brief:		获取交易记录索引表
*@Param IN:		NULL
*@Param OUT:	iIndexTable：索引表
*@Return:		>0:成功; <0:失败
*/
int getBillIndexTable(int32_t* iIndexTable);

/*
*@Brief:		获取余额凭证索引表
*@Param IN:		NULL
*@Param OUT:	iIndexTable：索引表
*@Return:		>0:成功; <0:失败
*/
int getBalVoucherIndexTable(int32_t* iIndexTable);

/*
*@Brief:		更新余额凭证
*@Param IN:		uiIndex:索引；pbBalVoucher：更新数据；uiLen:更新数据长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setBalVoucher(uint32_t uiIndex, uint8_t *pbBalVoucher, uint32_t uiLen);

/*
*@Brief:		更新交易记录
*@Param IN:		uiPos：索引；pbBill:交易记录数据；uiLen：交易记录数据长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int setBill(uint32_t uiPos, uint8_t* pbBill, uint32_t uiLen);

//void DCEPWalltest(void);
int dcepInit();
#endif
