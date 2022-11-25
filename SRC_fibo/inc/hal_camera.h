/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_camera.h
** Description:		摄像头相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_CAMERA_H_
#define _HAL_CAMERA_H_

#include "comm.h"



#define CAMERA_OK					0//成功
#define CAMERA_ERR_NOT_PRESENT		-7001//扫码模块不存在
#define CAMERA_ERR_NOT_OPEN			-7002//扫码模块未打开
#define CAMERA_ERR_PARAM			-7003//错误参数
#define CAMERA_ERR_NO_DATA			-7004//无数据
#define CAMERA_ERR_DATA_MISSING		-7005//缓冲区不够大，数据被截断
#define CAMERA_ERR_TIMEOUT			-7006//单次扫描超时
#define CAMERA_ERR_MEMORY			-7007//内存空间错误

struct _CameraScan_STR{

	uint8  state;//0-close;1-open
	uint32 height;//像素高度
	uint32 width;//像素宽度
	uint16 *pCamPreviewDataBuffer;
	camastae_t camastae;
	uint8  *Databuf;//保存从预览数据中提取的灰度信息
	uint8  Scanstate;
	uint8  *ScanCodebuffP;//扫描结果
	
}CameraScan_STR;

extern int g_iCamera_exist; //摄像头是否存在标志

extern struct _CameraScan_STR g_stCameraScan;

extern uint8 g_ui8CameraPreviewEn;

void hal_camInit(void);

int hal_camOpen(void);
void hal_camScanStart(void);
int hal_camScanCheck(void);
int hal_camScanRead(unsigned char *buf,unsigned int size);
void hal_camClose(void);


int hal_camSweepTest(void);

#endif




