#ifndef LC610N_API_PICC_H
#define LC610N_API_PICC_H


#include "comm.h"


//IC 卡的错误代码范围为-4000  ~ -4499。 
#define RET_RF_OK 0 //操作成功 
#define RET_RF_ERR_PARAM -4001 //参数错误 
#define RET_RF_ERR_NO_OPEN -4002 //射频模块未开启 
#define RET_RF_DET_ERR_NO_CARD -4003 //未搜寻到卡片(感应区内无指定类型的 卡片) 
#define RET_RF_DET_ERR_COLL -4004 //感应区内卡片过多(出现通讯冲突) 
#define RET_RF_DET_ERR_PROTOCOL -4006 //协议错误(卡片应答中出现违反协议规 定的数据) 
#define RET_RF_ERR_NOT_ACT -4019 //卡片未激活 
#define RET_RF_ERR_MULTI_CARD -4020 //多卡冲突 
#define RET_RF_ERR_TIMEOUT -4021 //超时无响应 
#define RET_RF_ERR_PROTOCOL -4022 //协议错误 
#define RET_RF_ERR_TRANSMIT -4023 //通信传输错误 
#define RET_RF_ERR_AUTH -4024 //M1 卡认证失败 
#define RET_RF_ERR_NO_AUTH -4025 //扇区未认证 
#define RET_RF_ERR_VAL -4026 //数值块数据格式有误 
#define RET_RF_ERR_NOT_REMOVE -4027 //卡片仍在感应区内 
#define RET_RF_ERR_STATUS -4028 //卡片状态错误(如 A/B 卡调用 M1 卡接 口, 或 M1 卡调用 PiccIsoCommand 接 口) 
#define RET_RF_ERR_CRC -4034 //CRC 校验错误 
#define RET_RF_ERR_FRAMING -4035 //帧错误 
#define RET_RF_ERR_PARITY -4036 //奇偶错误 
#define RET_RF_ERR_USER_CANCEL -4039 //用户取消 
#define RET_RF_HW_ERR -4255 //接口芯片不存在或异常 



//PICC 参数信息结构 
typedef struct{  
	unsigned char drv_ver[5];  		/* 驱动程序的版本信息,如：”1.01A”;只能读取,写入无效 */ 
	unsigned char drv_date[12];  	/* 驱 动 程 序 的 日 期 信 息 , 如：”2006.08.25”; 只能读取 */ 
	unsigned char a_conduct_w;  	/* A 卡输出电导写入允许：1--允许,其它 —不允许 */ 
	unsigned char a_conduct_val;  	/* A 卡输出电导控制变量*/ 
	unsigned char m_conduct_w;  	/* M1 卡输出电导写入允许 */ 
	unsigned char m_conduct_val;  	/* M1 卡输出电导控制变量*/ 
	unsigned char b_modulate_w;  	/* B 卡调制指数写入允许：*/ 
	unsigned char b_modulate_val;  	/* B 卡调制指数控制变量*/ 
	unsigned char  card_buffer_w;  	/*卡片接收缓冲区大小写入允许：*/ 
	unsigned short  card_buffer_val;  /*卡片接收缓冲区大小参数(单位：字节), 有效值 1~256.大于 256 时,将以 256 写 入;设为 0 时,将不会写入 */ 
	unsigned char wait_retry_limit_w; 	/* S(WTX)响应发送次数写入允许（暂不 适用） */ 
	unsigned short wait_retry_limit_val; /* S(WTX)响应最大重试次数（暂不适 用） */ 
	unsigned char card_type_check_w;  /*卡片类型检查写入允许 */ 
	unsigned char card_type_check_val;  /* 0-检查卡片类型,其他－不检查卡片类 型(默认为检查卡片类型) */ 
	unsigned char card_RxThreshold_w;  	/*B 卡片接收灵敏度写入允许：1--允许， 其它值--不允许。该值不可读 */ 
	unsigned char card_RxThreshold_val; /*B 卡片接收灵敏度 */ 
	unsigned char f_modulate_w;      /* felica 调制指数写入允许 */ 
	unsigned char f_modulate_val; 	 /* felica 调制指数 */ 
	unsigned char a_modulate_w; 	 /* A 卡调制指数写入允许：1--允许，其 它值—不允许*/ 
	unsigned char a_modulate_val; 	 /*A 卡调制指数控制变量*/ /*A 卡接收灵敏度检查写入允许：1--允 许，其它值--不允许*/ 
	unsigned char a_card_RxThreshold_w; 	/*A 卡接收灵敏度*/  
	unsigned char a_card_RxThreshold_val; 
	unsigned char a_card_antenna_gain_w;    /*A 卡的天线增益*/ 
	unsigned char a_card_antenna_gain_val; 
	
	unsigned char b_card_antenna_gain_w;    /*B 卡的天线增益*/ 
	unsigned char b_card_antenna_gain_val; 
	
	unsigned char f_card_antenna_gain_w;    /*Felica 的天线增益*/ 
	unsigned char f_card_antenna_gain_val; 
	
	unsigned char f_card_RxThreshold_w;     /*Felica 的接收灵敏度*/ 
	unsigned char f_card_RxThreshold_val; 
	
	unsigned char f_conduct_w;  /*Felica 的电导率*/ 
	unsigned char f_conduct_val; 
	
	unsigned char user_control_w;     /*paypass 认证需求，指定按键值，按下 时强制退出交易*/ 
	unsigned char user_control__key_val;     
	unsigned char b_modulate_level_w ; 
	unsigned char b_modulate_level_val[1]; 
	
	unsigned char reserved[70]; /*保留字节，用于将来扩展；写入时应全清零*/ 
}PICC_PARA;  



/*
*@Brief:		对非接触卡模块上电并复位,检查复位后模块初始状态是否正常。 
*@Param IN:		无				
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int piccOpen_lib(void);

/*
*@Brief:		按指定的模式搜寻 PICC 卡片;搜到卡片后,将其选中并激活。感应区内不允许多 卡存在。 
*@Param IN:		ucMode[输入]  0x00 搜寻 A 型卡和 B 型卡一次, 此模式适用于需 要增强多卡检测功能的场合。该模式是符合 ISO14443 规范的寻卡模式； 
							 0x01 搜寻 A 型卡和 B 型卡一次；此模式为 EMV 寻卡模式，通常使用该模式； 
							 ‘a’或‘A’ 只搜寻 A 型卡一次； 
							 ‘b’或‘B’ 只搜寻 B 型卡一次； 
							 ‘m’或‘M’ 只搜寻 M1 卡一次； 
							 ‘c’或‘C’ 只搜寻 felica 卡一次； 
							 ‘d’或‘D’ 只搜寻身份证一次； 
							 其它值 保留 
 			 
*@Param OUT:		pucCardType[输出] 存放卡片类型的缓冲区指针，可为 NULL 目前均返回一字节的类型值 
								 ‘A’ 搜寻到 A 型卡 
									‘B’ 搜寻到 B 型卡 
								 ‘M’ 搜寻到 M1 卡 
								 ‘C’  搜寻到 Felica 卡 
								 ‘D’  搜寻到身份证 
					pucSerialInfo [输出] 存放卡片序列号的缓冲区指针，可为 NULL。 
					pucOther [输出] 存放详细错误代码、卡片响应信息等内容的缓冲区指针， 可为 NULL 

*@Return:		0:成功; <0:失败
*/
int piccDetect_lib( unsigned char  ucMode, unsigned char* pucCardType, unsigned char* pucSerialInfo, unsigned char* pucOther );

/*
*@Brief:		在指定的通道上,向卡片发送 APDU 格式的数据,并接收响应 
*@Param IN:		*ApduSend[输入] 发送给 PICC 卡命令数据结构
*@Param OUT:	ApduResp[输出] 
*@Return:		0:成功; <0:失败
*/
int piccIsoCommand_lib(APDU_SEND_LIB *ApduSend, APDU_RESP_LIB *ApduResp);

/*
*@Brief:		依据指定的模式,向卡片发送停机指令；或者发送停活指令；或者复位载波，并 判断卡片是否已经移开感应区。
*@Param IN:		cid [输入] 用于指定卡片逻辑通道号;该通道号由PiccDetect( )的 CID 参数项输出,其取值范围为 0~14,目前取值均为 0。
				mode [输入]  ‘h’或‘H’ 意为 HALT，仅向卡片发送停活指令后就退出；该过程不执行卡移开检测 
								‘r’或‘R’ REMOVE， 向卡片发送停活指令，并执行卡移开检测； 
 								‘e’或‘E’ 符合 EMV 非接规范的移卡模式 复位载波，并执行卡移开检测 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccRemove_lib(unsigned char mode, unsigned char cid);

/*
*@Brief:		将载波关闭 5ms~10.1ms，场中所有的卡片均会掉电。复位后，卡片必须重新 PiccDetect 才能访问
*@Param IN:		无
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccReset_lib(void);

/*
*@Brief:		关闭 PICC 模块
*@Param IN:		无
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccClose_lib(void);

/*
*@Brief:		控制 RF 模块的 4 个 LED 灯的点亮和熄灭状态。 
*@Param IN:		ucLedIndex[输入] 灯索引，RED= 0x01,BLUE = 0x02,GREEN =0x03,YELLOW = 0x04
				ucOnOff[输入] 点亮或熄灭标志 0：熄灭 1：点亮 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
void piccLight_lib(uchar ucLedIndex,uchar ucOnOff);

/*
*@Brief:		与卡片进行APDU数据交互,终端将数据直接发送给卡片,并接收卡片的应答数据。
*@Param IN:		iTxN[输入] 待发送的命令数据长度 
				*ucpSrc[输入] 待发送的命令数据 
*@Param OUT:	*ipRxN[输出] 接收到卡片的数据长度 
				*ucpDes[输出] 接收到的卡片数据 
*@Return:		0:成功; <0:失败
*/
int piccCmdExchange_lib(unsigned int iTxN,  uchar* ucpSrc, unsigned int* ipRxN,  uchar* ucpDes); 

/*
*@Brief:		验证 M1 卡访问时读写相应模块需要提交的 A 密码或 B 密码
*@Param IN:		ucType [输入] ‘A’或‘a’提交的是 A 密码； 
								‘B’或‘b’提交的是 B 密码。 
				ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucPwd [输入] 指向提交的密码缓冲区。 6字节
				*pucSerialNo[ 输 入] 此参数未启用
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1Authority(unsigned char  ucType,        unsigned char  ucBlkNo, unsigned char *pucPwd, unsigned char *pucSerialNo ); 

/*
*@Brief:		读取 M1 卡指定块的内容(共 16 字节)。
*@Param IN:		ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucBlkValue[输出] 指向待存取块内容的缓冲区首址；该缓冲区至少应分配 16 字节。 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1ReadBlock(unsigned char ucBlkNo, unsigned char *pucBlkValue); 

/*
*@Brief:		向 M1 卡指定块写入指定的内容(共 16 字节)
*@Param IN:		ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucBlkValue[输入] 指向待存取块内容的缓冲区首址；该缓冲区至少应分配 16 字节。 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1WriteBlock(unsigned char ucBlkNo, unsigned char *pucBlkValue); 

/*
*@Brief:		对 M1 卡的指定数据块 ucBlkNo 进行充/减值/备份操作，将操作后的值更新到 另一个指定的数据块 ucUpdateBlkNo。 
*@Param IN:		ucType [输入] ‘+’充值,加号 ‘—’减值,减号 ‘>’存/备份操作,大于号 
				ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucValue [输入] 指向待存取块内容的缓冲区首址；该缓冲区至少应分配 16 字节。 
				ucUpdateBlkNo [输入] 指定操作结果最终写入到的块号。 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1Operate(unsigned char  ucType,       unsigned char  ucBlkNo,  unsigned char *pucValue,  unsigned char  ucUpdateBlkNo); 

/*
*@Brief:		验证M1卡访问时读写相应模块需要提交的A密码或B密码
*@Param IN:		pucPwd [输入]	4字节秘钥
*@Param OUT:	pucPack[输出]	2字节秘钥应答
*@Return:		0:成功; <0:失败
*/
int piccNtagAuthority_lib(unsigned char *pucPwd,  unsigned char *pucPack);

/*
*@Brief:		指定地址范围 读取卡片内容
*@Param IN:		ucStartAddr[输入]	起始地址页，范围0~0x2C
				ucStartEnd[输入]	起始地址页，范围0~0x2C
*@Param OUT:	pucPack[输出]	卡片的输出数据
*@Return:		0:成功; <0:失败
*/
int piccNtagRead_lib( unsigned char ucStartAddr, unsigned char ucEndaddr,	unsigned char *pucData);

/*
*@Brief:		指定地址写入数据
*@Param IN:		BlockNo [输入]	页地址
				pucPack [输入]	需写入内容，长度4字节
*@Param OUT:	
*@Return:		0:成功; <0:失败
*/
int piccNtagWrite_lib( unsigned char BlockNo, unsigned char *pucPack);

/*
*@Brief:		LED定时闪烁及同异步控制。 
*@Param IN:		ledNum：[输入] 灯索引，RED= 0x01,BLUE = 0x02,GREEN =0x03,YELLOW = 0x04	；
				type：type: 0-低频闪      1-高频闪 2-取消闪烁；
				count：闪烁次数，0：一直闪烁		
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int LedTwinkle_lib(int ledNum, int type, int count);


#endif
