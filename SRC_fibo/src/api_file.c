/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_file.c
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

#include "comm.h"



/*
*Function:		fileOpen_lib
*Description:	打开或创建指定名称的文件
*Input:			*pchFileName:文件名; ucMode:打开文件选项，
*				O_RDONLY-只读;O_WRONLY-只写;O_RDWR-可读写;O_CREAT-文件不存在则创建;
*				O_EXCL与O_CREAT联合使用，若文件存在则创建失败
*Output:		NULL
*Hardware:
*Return:		>=0:成功，打开文件的句柄；<0:失败
*Others:
*/
int fileOpen_lib(char *pchFileName, unsigned int ucMode)
{
	int iRet = -1;

	iRet = hal_fileOpen(pchFileName, ucMode);

	return iRet;
}



/*
*Function:		fileRead_lib
*Description:	读取文件中指定长度内容，一次不超过1024个字节
*Input:			iFileId:打开的文件句柄;        iReadLen:要读取的数据长度
*Output:		*pucOutData:数据缓冲指针;
*Hardware:
*Return:		>=0:成功读取的字节数; <0:失败
*Others:
*/
int fileRead_lib(int iFileId, char *pucOutData, unsigned int iReadLen)
{
	int iRet = -1;

	iRet = hal_fileRead(iFileId, pucOutData, iReadLen);

	return iRet;

}


/*
*Function:		fileWrite_lib
*Description:	向文件中写入指定长度的数据
*Input:			iFileId:打开的文件句柄; *pucInData:要写入的数据缓冲区指针; iInLen:要写入的字节数
*Output:		NULL
*Hardware:
*Return:		>=0:成功写入的字节数; <0:失败
*Others:
*/
int fileWrite_lib(int iFileId, char *pucInData, unsigned int iInLen)
{
	int iRet = -1;

	iRet = hal_fileWrite(iFileId, pucInData, iInLen);

	return iRet;
}

/*
*Function:		fileClose_lib
*Description:	关闭文件句柄
*Input:			iFileId:已打开的文件句柄
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int fileClose_lib(int iFileId)
{
	int iRet = -1;

	iRet = hal_fileClose(iFileId);

	return iRet;
}


/*
*Function:		fileSeek_lib
*Description:	移动文件指针到指定的位置
*Input:			iFileId:已打开的文件句柄; lOffset:从指定位置的偏移量，可为负数; 
*				iMode:移动指针的起始位置，SEEK_CUR-指针当前位置; SEEK_SET-文件开始位置; SEEK_END-文件末尾位置
*Output:		NULL
*Hardware:
*Return:		>=0:返回操作成功后文件指针位置; <0:失败
*Others:
*/
int fileSeek_lib(int iFileId, int lOffset, int iMode)
{
	int iRet = -1;

	iRet = hal_fileSeek(iFileId, lOffset, iMode);
	
	return iRet;
}


/*
*Function:		fileRemove_lib
*Description:	删除文件
*Input:			*pchFileName:文件名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int fileRemove_lib(char *pchFileName)
{
	int iRet = -1;

	iRet = hal_fileRemove(pchFileName);
	
	return iRet;
}


/*
*Function:		fileGetFileSysFreeSize_lib
*Description:	获取文件系统剩余存储空间大小
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0:文件系统剩余空间大小; <0:失败
*Others:
*/
long fileGetFileSysFreeSize_lib(void)
{
	long iRet = -1;

	iRet = hal_fileGetFileSysFreeSize();
	
	return iRet;
}


/*
*Function:		fileInGetFileSysFreeSize_lib
*Description:	获取内部文件系统剩余存储空间大小
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0:文件系统剩余空间大小; <0:失败
*Others:
*/
long fileInGetFileSysFreeSize_lib(void)
{
	long iRet = -1;

	iRet = hal_fileInGetFileSysFreeSize();

	return iRet;
}


/*
*Function:		fileGetFileSize_lib
*Description:	获取文件大小
*Input:			*pchFileName:文件名
*Output:		NULL
*Hardware:
*Return:		>=0:文件大小; <0:失败
*Others:
*/
long fileGetFileSize_lib(char *pchFileName)
{
	long iRet = -1;
	
	iRet = hal_fileGetFileSize(pchFileName);
	
	return iRet;
}


/*
*Function:		fileExist_lib
*Description:	判断文件是否存在
*Input:			*pchFileName:文件名
*Output:		NULL
*Hardware:
*Return:		1:文件存在; <0:文件不存在
*Others:
*/
int fileExist_lib(char *pchFileName)
{
	int iRet = -1;

	iRet = hal_fileExist(pchFileName);
	
	return iRet;
}



/*
*Function:		fileRename_lib
*Description:	文件改名
*Input:			*pchOldFileName:原文件名; *pchNewFileName:新文件名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int fileRename_lib(char *pchOldFileName, char *pchNewFileName)
{
	int iRet = -1;
	
	iRet = hal_fileRename(pchOldFileName, pchNewFileName);

	return iRet;
}

/*
*Function:		fileMkdir_lib
*Description:	创建一个文件夹（全路径）
*Input:			*pchDirName:文件夹路径名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int fileMkdir_lib(char *pchDirName)
{
	int iRet = -1;
	
	iRet = hal_fileMkdir(pchDirName);
	
	return iRet;
}


/*
*Function:		fileRmdir_lib
*Description:	删除指定的文件夹
*Input:			*pchDirName:文件夹路径名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int fileRmdir_lib(char *pchDirName)
{
	int iRet = -1;
	
	iRet = hal_fileRmdir(pchDirName);

	return iRet;
}


/*
*Function:		fileunZip_lib
*Description:	zip文件解压
*Input:			file_path:待解压文件路径名, zip_path:解压文件存储路径
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int fileunZip_lib(const char *file_path,const char *zip_path)
{
	int iRet;

	iRet = hal_fileunZip(file_path, zip_path);

	return iRet;

}


/*
*Function:		fileGetFileListCB_lib
*Description:	获取当前应用的文件名列表
*Input:			*pchDirName:需要查看的文件目录，绝对路径
*				void (*cb):读取目录列表回调接口
*				回调中：*pchDirName:读到的带绝对路径的文件列表;size:文件大小;filetype:文件类型,0-文件,1-文件夹;*arg:入参指针
*				*args:参数指针
*Output:		NULL
*Hardware:
*Return:		0：成功; 其他：失败
*Others:
*/
int fileGetFileListCB_lib(char *pchDirName,void (*cb)(const char *pchDirName, uint32 size, uint8 filetype, void *arg), void *args)
{
	int iRet = -1;

	iRet = hal_fileGetFileListCB(pchDirName, cb, args);

	return iRet;
	
}


/************************TEST***********************************/




