#ifndef LC610N_API_ICC_H
#define LC610N_API_ICC_H


#include "comm.h"

//IC 卡的错误代码范围为-2100  ~ -2499。 
#define ERR_SCC_SUCCESS 0 //成功 
#define ERR_SCC_DATA_LENGTH -2400 //数据长度错误 
#define ERR_SCC_PARITY -2401 //奇偶错误 
#define ERR_SCC_PARAMETER -2402 //参数值为空 
#define ERR_SCC_SLOT -2403 //卡通道错误 
#define ERR_SCC_PROTOCOL -2404 //协议错误 
#define ERR_SCC_CARD_OUT -2405 //无卡 
#define ERR_SCC_NO_INIT -2406 //没有初始化 
#define ERR_SCC_MESS_TIMEOUT -2407 //卡通讯超时 
#define ERR_SCC_VCC_MODE -2408 //电压模式错误 
#define ERR_SCC_MESS -2409 //通信失败 
#define ERR_ERR_SCC_TMOT -2410 //复位总超时 
#define ERR_ERR_SCC_MEMF -2411 //内存分配失败 
#define ERR_SCC_READ -2412 //读取字节错误 
#define ERR_SCC_WRITE -2413 //发送字节错误 
#define ERR_SCC_PARAM -2414 //参数错误 
#define ERR_NOT_SUPPORT_PAR -2415 //参数不支持 
//卡复位详细错误代码 
#define ERR_SCC_ATR_TS -2100 //正反向约定(TS)错误 
#define ERR_SCC_ATR_TCK -2101 //复位校验(TCK)错误 
#define ERR_SCC_ATR_TIMEOUT -2102 //复位等待超时 
#define ERR_SCC_ATR_TA1 -2103 //TA1 错误 
#define ERR_SCC_ATR_TA2 -2104 //TA2 错误 
#define ERR_SCC_ATR_TA3 -2105 //TA3 错误 
#define ERR_SCC_ATR_TB1 -2106 //TB1 错误 
#define ERR_SCC_ATR_TB2 -2107 //TB2 错误 
#define ERR_SCC_ATR_TB3 -2108 //TB3 错误 
#define ERR_SCC_ATR_TC1 -2109 //TC1 错误 
#define ERR_SCC_ATR_TC2 -2110 //TC2 错误 
#define ERR_SCC_ATR_TC3 -2111 //TC3 错误 
#define ERR_SCC_ATR_TD1 -2112 //TD1 错误 
#define ERR_SCC_ATR_TD2 -2113 //TD2 错误 
#define ERR_SCC_ATR_LENGTH -2114 //ATR 数据长度错误 
#define ERR_SCC_TS_TIMEOUT -2115 //字符间隔超时 
//T=0 卡片通信错误详细代码 
#define ERR_SCC_T0_TIMEOUT -2200 //等待卡片响应超时 
#define ERR_SCC_T0_MORE_SEND -2201 //重发错误 
#define ERR_SCC_T0_MORE_RECV -2202 //重收错误 
#define ERR_SCC_T0_PARITY -2203 //字符奇偶错误 
#define ERR_SCC_T0_INVALID_SW -2204 //状态字节无效 
//T=1 卡片通信错误详细代码 
#define ERR_SCC_T1_BWT -2300 //字组等待超时(BWT)错误 
#define ERR_SCC_T1_CWT -2301 //字符等待超时(CWT)错误 
#define ERR_SCC_T1_ABORT -2302 //异常(ABORT)通信错误 
#define ERR_SCC_T1_EDC -2303 //字组校验码(EDC)错误 
#define ERR_SCC_T1_SYNCH -2304 //同步通信错误 
#define ERR_SCC_T1_EGT -2305 //字符保护时间(EG )错误 
#define ERR_SCC_T1_BGT -2306 //字组保护时间(BG )错误 
#define ERR_SCC_T1_NAD -2307 //NAD 错误 
#define ERR_SCC_T1_PCB -2308 //PCB 错误 
#define ERR_SCC_T1_LENGTH -2309 //LEN 错误 
#define ERR_SCC_T1_IFSC -2310 //IFSC 错误 
#define ERR_SCC_T1_IFSD -2311 //IFSD 错误 
#define ERR_SCC_T1_MORE -2312 //重收发错误字组多次 错误 
#define ERR_SCC_T1_PARITY -2313 //字符奇偶错误 
#define ERR_SCC_T1_INVALID_BLOCK -2314 //无效的字组 


/*
*@Brief:		初始化 IC 卡功能，并选定电压 
*@Param IN:		slot[输入] 	bit 0~3 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2 
							bit 4~7 上电电压 1 --1.8V 2 -- 3.0V 默认：5.0V 
							bit 8~11 PPS 自适应操作 1 -- PPS 其它--NO PPS 
							bit 12~15 上电复位数据速率 默认--9600 1 -- 38400 
							bit 16~19 协议模式 默认--EMV 模式 1 -- ISO7816 模式 
*@Param OUT:	ATR [输出] 卡片复位应答。(至少需要 33bytes 的空间) 
							其内容为长度(1 字节)+复位应答内容[输出] 
*@Return:		0:成功; <0:失败
*/
int iccPowerUp_lib(unsigned int slot, unsigned char *atr);

/*
*@Brief:		对指定卡座中的卡片下电
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int iccClose_lib(unsigned int slot);

/*
*@Brief:		设置 IccIsoCommand 函数是否自动发送 GET RESPONSE 指令。 
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2
				autoresp [输入] 标志：1：自动发送;0：不自动发送;其他：无效
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int iccAutoResp_lib(unsigned char slot, uchar autoresp);

/*
*@Brief:		IC 卡操作函数。该函数支持 IC 卡通用接口协议(T=0 及 T=1)。
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2
				ApduSend[输入] 发送给 IC 卡命令数据结构
*@Param OUT:	ApduResp[输出]
*@Return:		0:成功; <0:失败
*/
int iccIsoCommand_lib(uchar slot, APDU_SEND_LIB *ApduSend, APDU_RESP_LIB *ApduResp);

/*
*@Brief:		检查指定的卡座内是否有卡
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2
*@Param OUT:	
*@Return:		0:有卡插入; <0:失败
*/
int iccGetPresent_lib(uchar slot);


#endif
