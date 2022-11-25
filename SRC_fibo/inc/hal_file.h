/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_file.h
** Description:		File system related interfaces
**
** Version:	1.0, 渠忠磊,2022-02-23
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_FILE_H_
#define _HAL_FILE_H_

#include "comm.h"

#define FILE_LOG_LEVEL_0		LOG_LEVEL_0
#define FILE_LOG_LEVEL_1		LOG_LEVEL_1
#define FILE_LOG_LEVEL_2		LOG_LEVEL_2
#define FILE_LOG_LEVEL_3		LOG_LEVEL_3
#define FILE_LOG_LEVEL_4		LOG_LEVEL_4
#define FILE_LOG_LEVEL_5		LOG_LEVEL_5



#define FILE_TBL_RET_OK				0		//操作成功
#define FILE_EXIST					-5101	//文件已存在
#define FILE_NOEXIST				-5102	//文件不存在
#define FILE_MEM_OVERFLOW			-5103	//空间不足
#define FILE_TOO_MANY_FILES			-5104	//文件太多
#define FILE_INVALID_HANDLE			-5105	//无效的文件句柄
#define FILE_INVALID_MODE			-5106	//无效的模式
#define FILE_NOT_OPENED				-5108	//文件没有打开
#define FILE_OPENED					-5109	//文件已被打开
#define FILE_END_OVERFLOW			-5110	//修改后的文件指针超出文件末尾
#define FILE_TOP_OVERFLOW			-5111	//修改后的文件指针小于0
#define FILE_NO_PERMISSION			-5112	//超出访问权限
#define FILE_NAME_LENERR			-5113	//文件名长度不合法
#define FILE_OPEN_MAX_FILE			-5114	//已打开的文件数目达到最大值
#define FILE_PARA_INVALID			-5115	//无效参数
#define FILE_BUFFPARA_OVERFLOW		-5116	//接收缓存空间不足
#define FILE_UNZIP_ERR				-5117	//文件解压失败



#define EXT_FS_DIR  	"/ext"
/*FONTFLASH方案，字库直接存放于spiflash中*/
#define EXT_FS_FLASH_ADDR_START 			(4*1024*1024)

/*FONTFS方案，字库存放于文件系统中*/
#define EXT_FS_FLASH_ADDR_START_FONTFS		(0*1024*1024)


int hal_fileOpen(char *pchFileName, unsigned int ucMode);
int hal_fileRead(int iFileId, char *pucOutData, unsigned int iReadLen);
int hal_fileWrite(int iFileId, char *pucInData, unsigned int iInLen);
int hal_fileClose(int iFileId);
int hal_fileSeek(int iFileId, int lOffset, int iMode);
int hal_fileRemove(char *pchFileName);
long hal_fileGetFileSysFreeSize(void);
long hal_fileGetFileSize(char *pchFileName);
int hal_fileExist(char *pchFileName);
int hal_fileRename(char *pchOldFileName, char *pchNewFileName);
int hal_fileMkdir(char *pchDirName);
int hal_fileRmdir(char *pchDirName);
int hal_fileGetFileListCB(char *pchDirName,void (*cb)(const char *pchDirName, uint32 size, uint8 filetype, void *arg), void *args);

/*
*@Brief:		zip文件解压
*@Param IN:		file_path:待解压文件路径名, zip_path:解压文件存储路径
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int hal_fileunZip(const char *file_path,const char *zip_path);
int32 hal_fileReadPro(int8 *filename, uint32 offset, int8 *buf, uint32 len);
int32 hal_fileWritePro(int8 *filename, uint32 offset, int8 *buf, uint32 len);



void filetest(void);



#endif

