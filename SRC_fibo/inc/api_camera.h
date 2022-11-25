
/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_camera.h
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

#ifndef _API_CAMERA_H_
#define _API_CAMERA_H_

#include "comm.h"


/*
*Function:		scanOpen_lib
*Description:	初始化扫码头模块
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int scanOpen_lib(void);

/*
*Function:		scanStart_lib
*Description:	启动扫描
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void scanStart_lib(void);

/*
*Function:		scanCheck_lib
*Description:	判断扫描是否完成
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-未完成；1-完成
*Others:
*/
int scanCheck_lib(void);

/*
*Function:		scanRead_lib
*Description:	获取扫码数据
*Input:			size-接收缓存大小
*Output:		*buf-接收缓存，不能为空
*Hardware:
*Return:		>0:成功返回实际读到的条码数据长度，<0-失败
*Others:
*/
int scanRead_lib(unsigned char *buf,unsigned int size);

/*
*Function:		scanClose_lib
*Description:	关闭扫码模块(模块下电，关闭通信口)
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void scanClose_lib(void);

#endif







