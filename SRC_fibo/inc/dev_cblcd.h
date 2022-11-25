/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		dev_cblcd.h
** Description:		断码屏相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _DEV_CBLCD_H_
#define _DEV_CBLCD_H_


#include "comm.h"


/*
*Function:		dev_cblcdSendBit
*Description:	发送一个bit数据
*Input:			val:bit数值
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdSendBit(BOOL val);

/*
*Function:		dev_cblcdSYSDIS
*Description:	关闭系统振荡器和LCD 偏压发生器,时钟停止后LCD显示空白，LCD OFF之后，可以调用此接口降低功耗
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdSYSDIS(void);

/*
*Function:		dev_cblcdSYSEN
*Description:	只打开系统振荡器。如果调用了DR_CblcdSYSDIS后再次调用此接口前要调用DR_CblcdON，不然不显示
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdSYSEN(void);

/*
*Function:		dev_cblcdOFF
*Description:	关闭LCD偏压发生器
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdOFF(void);

/*
*Function:		dev_cblcdON
*Description:	打开LCD偏压发生器
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdON(void);

/*
*Function:		dev_cblcdBIAS
*Description:	设置偏压系数和COM端口，默认COM端口是4个
*Input:			val:2-偏压系数1/2; 3-偏压系数1/3
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdBIAS(uint8 val);

/*
*Function:		dev_cblcdInit
*Description:	断码屏初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdInit(void);

/*
*Function:		dev_cblcdData
*Description:	发送4bits数据
*Input:			data:数据内容
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdData(uint8 data);

/*
*Function:		dev_cblcdAddr
*Description:	发送6bits地址
*Input:			val:地址内容
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int dev_cblcdAddr(uint8 val);

#endif


