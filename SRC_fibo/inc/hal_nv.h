/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_nv.h
** Description:		NV区相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_NV_H_
#define _HAL_NV_H_

#include "comm.h"


#define NV_LOG_LEVEL_0		LOG_LEVEL_0
#define NV_LOG_LEVEL_1		LOG_LEVEL_1
#define NV_LOG_LEVEL_2		LOG_LEVEL_2
#define NV_LOG_LEVEL_3		LOG_LEVEL_3
#define NV_LOG_LEVEL_4		LOG_LEVEL_4
#define NV_LOG_LEVEL_5		LOG_LEVEL_5


#define NV_SN_LEN		32//SN号
#define	NV_CID_LEN		24//客户标识CID
#define NV_TERM_LEN		8//内部机型TERM
#define NV_HW_LEN		8//硬件标识
#define NV_FTTYPE_LEN	8//字库标识

#define NV_LEN		80


#define SET_DEVICE_NUM		1
#define GET_DEVICE_NUM		0

int32 hal_nvReadHwVersionString(int8 *hw_data);
int32 hal_nvWriteHwVersionString(int8 *hw_data);

int16 hal_nvReadSN(int8 *sn_data);
int16 hal_snWriteSN(int8 *sn_data);
int16 hal_nvReadCustomerID(int8 *cid_data);
int16 hal_nvWriteCustomerID(int8 *cid_data, uint8 cid_len);
int16 hal_nvReadHwVersion(int8 *hw_data);
int16 hal_nvWriteHwVersion(int8 *hw_data, uint8 hw_len);
int16 hal_nvReadTerm(int8 *term_data);
int16 hal_nvWriteTerm(int8 *term_data, uint8 term_len);
int16 hal_nvReadFontType(int8 *fttype_data);
int16 hal_nvWriteFontType(int8 *fttype_data, uint8 fttype_len);



void SN_Hw_test(void);

void NV_test(void);


#endif

