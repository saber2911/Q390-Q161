/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_seload.h
** Description:		主要涉及610模组给SE升级VOS相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/


#ifndef _HAL_SELOAD_H_
#define _HAL_SELOAD_H_

#include "comm.h"


#define SELOAD_LOG_LEVEL_0		LOG_LEVEL_0
#define SELOAD_LOG_LEVEL_1		LOG_LEVEL_1
#define SELOAD_LOG_LEVEL_2		LOG_LEVEL_2
#define SELOAD_LOG_LEVEL_3		LOG_LEVEL_3
#define SELOAD_LOG_LEVEL_4		LOG_LEVEL_4
#define SELOAD_LOG_LEVEL_5		LOG_LEVEL_5



#define SELOAD_HANDSHAKE_SENDBYTE		'Q'
#define SELOAD_HANDSHAKE_RECVBYTE		'R'
#define SELOAD_READMACHINE_SENDBYTE		'T'
#define SELOAD_READMACHINE_RECVBYTE		0x10
#define SELOAD_NULL_SENDBYTE			'P'
#define SELOAD_PKG_STARTCODE			0x02
#define SELOAD_PKG_CMDLOAD				0xe4
#define SELOAD_PKG_CMDERASE				0xe3
#define SELOAD_PKG_CMDWRITE				0xe6		
#define SELOAD_ACKBYTE					0x06

#define SELOAD_SENDPKG_DATALEN			4*1024
#define SELOAD_RECVPKG_DATALEN			1*1024



/*********************ERR CODE*********************/
#define SELOAD_SUCCESS						0//成功
#define SELOAD_ANALYSISLENTHFAILED			-5301//解析失败
#define SELOAD_ANALYSISLRCFAILED			-5302//LRC失败
#define SELOAD_MALLOCFAILED					-5303//申请内存失败
#define SELOAD_SENDFAILED					-5304//发送失败
#define SELOAD_RECVFAILED					-5305//接收失败
#define SELOAD_HANDSHAKEFAILED				-5306//握手失败
#define SELOAD_READMACHINEFAILED			-5307//查询机型失败
#define SELOAD_CMDFAILED					-5308//指令非法
#define SELOAD_SIGNCHECKFAILED				-5309//签名校验失败
#define SELOAD_DATALENFAILED				-5310//长度错误
#define SELOAD_REBOOTVOSFAILED				-5311//重启失败


typedef enum{

	LOADVOS_START = 0,
		
	LOADVOS_HANDSHAKE,
	LOADVOS_ERASEFLASH,
	LOADVOS_LOAD,
	LOADVOS_ERASEVOS,
	LOADVOS_WRITEVOS,
	LOADVOS_REBOOTVOS,
	
	LOADVOS_END,
	
}LOADSTEP;
typedef struct _SE_PKG{

	uint8 startcode;
	uint8 cmd;
	uint32 datalen;
	uint8 *data;
	uint8 lrc;
}SE_PKG;


extern g_iUpdateVOSFlag;//下载VOS标志位

/*
*@Brief:		下载进度及状态读取回调接口
*@Param IN:		*loadvos_callback_P:回调接口指针
*@Param OUT:	NULL
*@Return:		NULL
*/
void hal_seldUpdateVOSCbReg(void (*loadvos_callback_P)(LOADSTEP step, int32 status, uint32 schedule, void *arg));

/*
*@Brief:		升级VOS接口
*@Param IN：		*data:升级VOS指针; datalen:VOS数据长度
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int32 hal_seldUpdateVOS(unsigned char *data, uint32 datalen);


void Test_SEload_UpdateVOS(void);

#endif

