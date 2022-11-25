/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_bmp.h
** Description:		BMP图片显示相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/
#ifndef _HAL_BMP_H_
#define _HAL_BMP_H_

#pragma once
 
#include<stdio.h>

#define BMP_LOG_LEVEL_0		0
#define BMP_LOG_LEVEL_1		1
#define BMP_LOG_LEVEL_2		2
#define BMP_LOG_LEVEL_3		3
#define BMP_LOG_LEVEL_4		4
#define BMP_LOG_LEVEL_5		5


typedef unsigned int DWORD;  // 4bytes
typedef unsigned short WORD;  // 2bytes
typedef signed long LONG;  // 4bytes
typedef unsigned char BYTE;  // 1bytes
 
 
#pragma pack(push)
#pragma pack(1)// 修改默认对齐值
/*位图文件文件头结构体*/
typedef struct tagBITMAPFILEHEADER {
	WORD bFileType;//文件类型
	DWORD bFileSize;//位图文件大小
	WORD bReserved1;//保留
	WORD bReserved2;//保留
	DWORD bPixelDataOffset;//从文件头开始到实际图像数据之间的偏移量，
}BITMAPFILEHEADER; //14bytes
#pragma pack(pop)
 
/*位图文件信息头结构体*/
typedef struct tagBITMAPINFOHEADER {
	DWORD bHeaderSize;  // 图像信息头总大小（40bytes）
	LONG bImageWidth;  // 图像宽度（像素）
	LONG bImageHeight;  // 图像高度
	WORD bPlanes;  // 应该是0
	WORD bBitsPerPixel;  // 像素位数，其值为1,4,8,16或32
	DWORD bCompression;  // 图像压缩方法
	DWORD bImageSize;  // 图像大小（字节）
	LONG bXpixelsPerMeter;  // 横向每米像素数
	LONG bYpixelsPerMeter;  // 纵向每米像素数
	DWORD bTotalColors;  // 使用的颜色索引总数，如果为0，则表示使用所有调色板项，如果像素位数大于8，则该字段没有意义
	DWORD bImportantColors;  // 重要颜色数，一般没什么用
}BITMAPINFOHEADER; //40bytes
 
/*位图文件调色板结构体*/
typedef struct tagRGBQUAD {
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
}RGBQUAD;
 
/*像素点RGB结构体*/
typedef struct tagRGB {
	BYTE blue;
	BYTE green;
	BYTE red;
}RGBDATA;

#if MAINTEST_FLAG
extern const unsigned char wxpp_logo[86454];
#endif

int hal_bmpWriteToScr(int left, int top, unsigned char *bmpdata, int bmpdatalen, unsigned int angle);
int hal_bmpWriteFileToScr(int left, int top, char *filename, unsigned int angle);
int hal_bmpRotate(unsigned char *bmpdata_in, unsigned char *bmpdata_out, unsigned int *width, unsigned int *height, unsigned int angle);


#endif



