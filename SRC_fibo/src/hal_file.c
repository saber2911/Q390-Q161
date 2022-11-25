/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_file.c
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

extern INT32 fibo_file_getFreeSize_ex(const char *dir_path);

typedef void (*Filelist_cb)(const char *pchDirName, uint32 size, uint8 filetype, void *arg);

Filelist_cb filelistcb = NULL;


#define EXTNAMEHEAD_USR		"/ext/"
#define EXTNAMEHEAD_USR1	"/ext"


/*
*
*@Brief:		判断文件名字是否有效，如果挂载spiFlash失败，则不可操作外部文件系统
*@Param IN：		*pchFileName：带绝对路径的文件名字指针
*@Return:		0-成功，<0-失败
*/
int fileJudgeFilename(char *pchFileName)
{
	int iRet = 0;
	iRet = memcmp(pchFileName, EXTNAMEHEAD_USR, strlen(EXTNAMEHEAD_USR));
	if(iRet == 0)//spiflash file
	{
		if(g_i32SpiFlashStatus == 0)return -1;
		else return 0;
	}
	iRet = memcmp(pchFileName, EXTNAMEHEAD_USR1, strlen(EXTNAMEHEAD_USR1));
	if(iRet == 0 && strlen(pchFileName) == 4)//spiflash file
	{
		if(g_i32SpiFlashStatus == 0)return -1;
		else return 0;
	}
	
	return 0;
}


/*
*Function:		hal_fileOpen
*Description:	打开或创建指定名称的文件
*Input:			*pchFileName:文件名; ucMode:打开文件选项，
*				O_RDONLY-只读;O_WRONLY-只写;O_RDWR-可读写;O_CREAT-文件不存在则创建;
*				O_EXCL与O_CREAT联合使用，若文件存在则创建失败
*Output:		NULL
*Hardware:
*Return:		>=0:成功，打开文件的句柄；<0:失败
*Others:
*/
int hal_fileOpen(char *pchFileName, unsigned int ucMode)
{
	int iRet = -1;

	if(pchFileName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	//if(0 == memcmp(EXT_FS_DIR,pchFileName,strlen(EXT_FS_DIR)))
	{
		FlashCsPinModeCheck();
	}

	iRet = fileJudgeFilename(pchFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_file_open(pchFileName, ucMode);
	sysLOG(FILE_LOG_LEVEL_2, "hal_fileOpen pchFileName:%s,iRet=%d\r\n", pchFileName, iRet);
	if(iRet < 0)
	{
		iRet = FILE_NOEXIST;
	}

exit:
	
	return iRet;
}


/*
*Function:		hal_fileRead
*Description:	读取文件中指定长度内容，一次不超过1024个字节
*Input:			iFileId:打开的文件句柄;        iReadLen:要读取的数据长度
*Output:		*pucOutData:数据缓冲指针;
*Hardware:
*Return:		>=0:成功读取的字节数; <0:失败
*Others:
*/
int hal_fileRead(int iFileId, char *pucOutData, unsigned int iReadLen)
{
	int iRet = -1;
	uint cnt = 0;
	uint endlen = 0;
	int readlentmp = 0;

	cnt = iReadLen/1024;
	endlen = iReadLen%1024;

	if(iFileId < 0 || pucOutData == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	FlashCsPinModeCheck();
	
	for(uint8 i=0; i<cnt; i++)
	{
		iRet = fibo_file_read(iFileId, (unsigned char *)pucOutData+i*1024, 1024);
		if(iRet < 0)
		{
			iRet = FILE_NOT_OPENED;
			goto exit;
		}
		readlentmp += iRet;
	}
	if(endlen != 0)
	{
		iRet = fibo_file_read(iFileId, (unsigned char *)pucOutData+cnt*1024, endlen);
		if(iRet < 0)
		{
			iRet = FILE_NOT_OPENED;
			goto exit;
		}
		readlentmp += iRet;
	}
	iRet = readlentmp;

exit:

	return iRet;

}

/*
*Function:		hal_fileWrite
*Description:	向文件中写入指定长度的数据
*Input:			iFileId:打开的文件句柄; *pucInData:要写入的数据缓冲区指针; iInLen:要写入的字节数
*Output:		NULL
*Hardware:
*Return:		>=0:成功写入的字节数; <0:失败
*Others:
*/
int hal_fileWrite(int iFileId, char *pucInData, unsigned int iInLen)
{
	int iRet = -1;
	uint cnt = 0;
	uint endlen = 0;
	int writelentmp = 0;

	cnt = iInLen/1024;
	endlen = iInLen%1024;

	if(iFileId < 0 || pucInData == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}
	
	FlashCsPinModeCheck();
	
	for(uint8 i=0; i<cnt; i++)
	{
		iRet = fibo_file_write(iFileId, (unsigned char *)pucInData+i*1024, 1024);
		if(iRet < 0)
		{
			iRet = FILE_NOT_OPENED;
			goto exit;
		}
		writelentmp += iRet;
	}
	if(endlen != 0)
	{
		iRet = fibo_file_write(iFileId, (unsigned char *)pucInData+cnt*1024, endlen);
		if(iRet < 0)
		{
			iRet = FILE_NOT_OPENED;
			goto exit;
		}
		writelentmp += iRet;
	}
	iRet = writelentmp;

exit:

	return iRet;
	
}

/*
*Function:		hal_fileClose
*Description:	关闭文件句柄
*Input:			iFileId:已打开的文件句柄
*Output:		NULL
*Hardware:
*Return:		0:成功；<0:失败
*Others:
*/
int hal_fileClose(int iFileId)
{
	int iRet = -1;

	if(iFileId < 0)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	FlashCsPinModeCheck();
	
	iRet = fibo_file_close(iFileId);
	if(iRet < 0)
	{
		iRet = FILE_NOEXIST;
		goto exit;
	}
	iRet = FILE_TBL_RET_OK;

exit:

	return iRet;
	
}

/*
*Function:		hal_fileSeek
*Description:	移动文件指针到指定的位置
*Input:			iFileId:已打开的文件句柄; lOffset:从指定位置的偏移量，可为负数; 
*				iMode:移动指针的起始位置，SEEK_CUR-指针当前位置; SEEK_SET-文件开始位置; SEEK_END-文件末尾位置
*Output:		NULL
*Hardware:
*Return:		>=0:返回操作成功后文件指针位置; <0:失败
*Others:
*/
int hal_fileSeek(int iFileId, int lOffset, int iMode)
{
	int iRet = -1;

	if(iFileId < 0)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	FlashCsPinModeCheck();
	
	iRet = fibo_file_seek(iFileId, lOffset, iMode);
	if(iRet < 0)
	{
		iRet = FILE_NOT_OPENED;
		goto exit;
	}

exit:

	return iRet;
	
}

/*
*Function:		hal_fileRemove
*Description:	删除文件
*Input:			*pchFileName:文件名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_fileRemove(char *pchFileName)
{
	int iRet = -1;

	if(pchFileName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}

	iRet = fibo_file_delete(pchFileName);
	if(iRet < 0)
	{
		iRet = FILE_NOEXIST;
		goto exit;
	}
	
	iRet = FILE_TBL_RET_OK;

exit:

	return iRet;
	
}


/*
*Function:		hal_fileGetFileSysFreeSize
*Description:	获取文件系统剩余存储空间大小
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0:文件系统剩余空间大小; <0:失败
*Others:
*/
long hal_fileGetFileSysFreeSize(void)
{
	long iRet = -1;

	FlashCsPinModeCheck();

	iRet = fibo_file_getFreeSize_ex(EXT_FS_DIR);
	if(iRet < 0)
	{
		iRet = FILE_MEM_OVERFLOW;
		goto exit;
	}

exit:

	return iRet;
}


/*
*Function:		hal_fileInGetFileSysFreeSize
*Description:	获取内部文件系统剩余存储空间大小
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0:文件系统剩余空间大小; <0:失败
*Others:
*/
long hal_fileInGetFileSysFreeSize(void)
{
	long iRet = -1;

	iRet = fibo_file_getFreeSize();
	if(iRet < 0)
	{
		iRet = FILE_MEM_OVERFLOW;
		goto exit;
	}

exit:

	return iRet;
}

/*
*Function:		hal_fileGetFileSize
*Description:	获取文件大小
*Input:			*pchFileName:文件名
*Output:		NULL
*Hardware:
*Return:		>=0:文件大小; <0:失败
*Others:
*/
long hal_fileGetFileSize(char *pchFileName)
{
	long iRet = -1;
	
	if(pchFileName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}
	
	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_file_getSize(pchFileName);
	if(iRet < 0)
	{
		iRet = FILE_NOEXIST;
		goto exit;
	}

exit:

	return iRet;
}

/*
*Function:		hal_fileExist
*Description:	判断文件是否存在
*Input:			*pchFileName:文件名
*Output:		NULL
*Hardware:
*Return:		1:文件存在; <0:文件不存在
*Others:
*/
int hal_fileExist(char *pchFileName)
{
	int iRet = -1;

	if(pchFileName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_file_exist(pchFileName);
	if(iRet < 0)
	{
		iRet = FILE_NAME_LENERR;
		goto exit;
	}

exit:

	return iRet;
}


/*
*Function:		hal_fileRename
*Description:	文件改名
*Input:			*pchOldFileName:原文件名; *pchNewFileName:新文件名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_fileRename(char *pchOldFileName, char *pchNewFileName)
{
	int iRet = -1;
	
	int *rP = NULL;
	int lentmp = 0;
	
	if(pchOldFileName == NULL || pchNewFileName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}
	
	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchOldFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}

	iRet = fileJudgeFilename(pchNewFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}

#if 1

	iRet = fileGetFileSize_lib(pchOldFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileGetFileSize_lib,iRet=%d\r\n", iRet);
		goto exit;
	}
	lentmp = iRet;

	rP = malloc(lentmp+1);
	if(rP == NULL)
	{
		sysLOG(FILE_LOG_LEVEL_2, "malloc err,iRet=%d\r\n", iRet);
		return -2;
	}
	memset(rP, 0, lentmp+1);
	iRet = hal_fileReadPro(pchOldFileName, 0, rP, lentmp);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "DR_ReadFile err,iRet=%d\r\n", iRet);
		goto exit;
	}
	iRet = fileRemove_lib(pchOldFileName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileRemove_lib err,iRet=%d\r\n", iRet);
		goto exit;
	}

	iRet = hal_fileWritePro(pchNewFileName, 0, rP, lentmp);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "DR_WriteFile err,iRet=%d\r\n", iRet);
		goto exit;
	}

	iRet = FILE_TBL_RET_OK;

#endif


#if 0	
	iRet = fibo_file_rename(pchOldFileName, pchNewFileName);
	if(iRet < 0)
	{
		iRet = FILE_NAME_LENERR;
		goto exit;
	}

	iRet = FILE_TBL_RET_OK;
#endif

exit:

	if(NULL != rP)
		free(rP);
	
	return iRet;
}


/*
*Function:		hal_fileMkdir
*Description:	创建一个文件夹（全路径）
*Input:			*pchDirName:文件夹路径名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_fileMkdir(char *pchDirName)
{
	int iRet = -1;
	
	if(pchDirName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}

	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchDirName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_file_mkdir(pchDirName);
	if(iRet < 0)
	{
		iRet = FILE_NAME_LENERR;
		goto exit;
	}
	iRet = FILE_TBL_RET_OK;

exit:

	return iRet;
}


/*
*Function:		hal_fileRmdir
*Description:	删除指定的文件夹
*Input:			*pchDirName:文件夹路径名
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_fileRmdir(char *pchDirName)
{
	int iRet = -1;
	
	if(pchDirName == NULL)
	{
		iRet = FILE_PARA_INVALID;
		goto exit;
	}
	
	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchDirName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_file_rmdir(pchDirName);
	if(iRet < 0)
	{
		iRet = FILE_NAME_LENERR;
		goto exit;
	}

	iRet = FILE_TBL_RET_OK;

exit:

	return iRet;
}

/*
*Function:		hal_fileunZip
*Description:	zip文件解压
*Input:			file_path:待解压文件路径名, zip_path:解压文件存储路径
*Output:		NULL
*Hardware:
*Return:		0:成功; <0:失败
*Others:
*/
int hal_fileunZip(const char *file_path,const char *zip_path)
{
	int iRet;

	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(file_path);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}

	iRet = fileJudgeFilename(zip_path);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_file_unzip(file_path,zip_path);
	if(iRet < 0)
	{
		iRet = FILE_UNZIP_ERR;
		goto exit;
	}

	iRet = FILE_TBL_RET_OK;

exit:

	return iRet;

}


/*
*Function:		hal_fileListCB
*Description:	文件列表回调接口
*Input:			*pchDirName:读到的带绝对路径的文件列表; size:文件大小; *arg:入参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void *hal_fileListCB(const char *pchDirName, uint32 size, void *arg)
{
	int iRet = -1;
	sysLOG(FILE_LOG_LEVEL_4, "size = %d,pchDirName:%s, arg=%x\r\n", size, pchDirName, arg);
	iRet = fibo_file_differdir(pchDirName);
	filelistcb(pchDirName, size, iRet, arg);
}




/*
*Function:		hal_fileGetFileListCB
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
int hal_fileGetFileListCB(char *pchDirName,void (*cb)(const char *pchDirName, uint32 size, uint8 filetype, void *arg), void *args)
{
	int iRet = -1;

	uint32 uTime = 0;
	
	filelistcb = cb;

	FlashCsPinModeCheck();

	iRet = fileJudgeFilename(pchDirName);
	if(iRet < 0)
	{
		sysLOG(FILE_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto exit;
	}
	
	iRet = fibo_get_file_list(pchDirName, hal_fileListCB, args);
	if(iRet != 0)
	{
		iRet = FILE_NO_PERMISSION;
		goto exit;
	}
	iRet = FILE_TBL_RET_OK;

exit:

	return iRet;
	
}


/*
*Function:		hal_fileReadPro
*Description:	读文件系统中的内容到buf中,包括open read close等过程
*Input:			*filename：全路径文件名称; offset:开始读文件中的内容的偏移量；len:期望读到的字节数
*Output:		*buf：文件读取之后缓存指针
*Hardware:
*Return:		<0:失败；>0:实际读取到的字节数
*Others:
*/
int32 hal_fileReadPro(int8 *filename, uint32 offset, int8 *buf, uint32 len)
{
	int32 iRet = -1;	
	int32 fd;
	int32 fsize = -1;
	int i;
	int count;
	fsize = hal_fileGetFileSize(filename);
	if(fsize < 0)
	{
		return fsize;
	}
	if((fsize - offset) < 0)
	{
		return (fsize - offset);
	}
	if((fsize-offset) > len)
	{
		fsize = len;
	}
	else
	{
		fsize -= offset;
	}
	count = fsize/1024+1;

	fd = hal_fileOpen(filename, O_RDONLY);
	if(fd < 0)
	{
		return fd;
	}
	iRet = hal_fileSeek(fd, offset, SEEK_SET);
	if(iRet < 0)
	{
		return iRet;
	}
	for(i=0; i<count; i++)
	{
		if(i == count-1)
		{
			if(fsize%1024 != 0)
			{
				iRet = hal_fileRead(fd, buf+(i*1024), fsize%1024);
			}
		}
		else

		{
			iRet = hal_fileRead(fd, buf+(i*1024), 1024);
		}
		if(iRet < 0)
		{
			hal_fileClose(fd);
			return iRet;
		}
	}
	hal_fileClose(fd);
	return fsize;
	
}


/*
*Function:		hal_fileWritePro
*Description:	写buf中的内容到文件系统中,包括open write close等步骤
*Input:			*filename：全路径文件名称; offset: 写文件中的偏移量；*buf：写入数据的指针；len:期望写入的字节数
*Output:		NULL
*Hardware:
*Return:		<0:失败；>0:实际写入的字节数
*Others:
*/
int32 hal_fileWritePro(int8 *filename, uint32 offset, int8 *buf, uint32 len)
{
	int32 iRet = -1;	
	int32 fd;
	int32 fsize = -1;
	int i;
	int count;
	
	fsize = len;
	
	count = fsize/1024+1;

	fd = hal_fileOpen(filename, O_RDWR | O_CREAT);
	if(fd < 0)
	{
		return fd;
	}
	iRet = hal_fileSeek(fd, offset, SEEK_SET);
	if(iRet < 0)
	{
		return iRet;
	}
	for(i=0; i<count; i++)
	{
		if(i == count-1)
		{
			if(fsize%1024 != 0)
			{
				iRet = hal_fileWrite(fd, buf+(i*1024), fsize%1024);
			}
		}
		else

		{
			iRet = hal_fileWrite(fd, buf+(i*1024), 1024);
		}
		if(iRet < 0)
		{
			hal_fileClose(fd);
			return iRet;
		}
	}
	hal_fileClose(fd);
	return fsize;
	
}





/************************TEST***********************************/
#if MAINTEST_FLAG

void filegetlistcbtest(const char *pchDirName, uint32 size, uint8 filetype, void *arg)
{
	sysLOG(FILE_LOG_LEVEL_1, "size = %d,filetype=%d,pchDirName:%s, arg=%s\r\n", size, filetype, pchDirName, arg);

}


void filetest(void)
{
	int iRet = -1;

	uint8 *rP = "hello world";
	
	iRet = hal_fileGetFileListCB("/app/ufs", filegetlistcbtest, rP);
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileGetFileListCB,iRet=%d\r\n", iRet);
	sysDelayMs(5000);
	iRet = hal_fileGetFileListCB("/app/data", filegetlistcbtest, rP);
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileGetFileListCB,iRet=%d\r\n", iRet);

}
//#define EX_FILESYSTEM_TEST
#ifdef EX_FILESYSTEM_TEST

/*
*@Brief:		测试文件系统大文件 创建和改写 删除
*@Param IN:		
*				
*				
*				
*
*@Return:		
*/


void maxFileWriteTest()
{
	int iRet;
	int fd;
	int i;
	uint8_t *data = fibo_malloc(2048);

	
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileGetFileSysFreeSize:%d\r\n", hal_fileGetFileSysFreeSize());
	//1.create a file in exfilesys
	fd = hal_fileOpen("/ext/testfile.txt", O_CREAT|O_RDWR);
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileOpen,fd=%d\r\n", fd);
	if(fd < 0)
	{
		fibo_free(data);
		return;
	}
	//2.write big file 
	for(i=0;i<1024;i++)
	{
		iRet = hal_fileSeek(fd, 0, SEEK_END);
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileSeek,iRet=%d\r\n", iRet);
		if(iRet < 0)
		{
			goto exit;
		}
		memset(data,1,2048);
		iRet = hal_fileWrite(fd, data, 2048);
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileWrite,iRet=%d\r\n", iRet);
		if(iRet < 0)
		{
			goto exit;
		}		
	}
	//3.close
	iRet = hal_fileClose(fd);
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileClose,iRet=%d\r\n", iRet);
	if(iRet < 0)
	{
		goto exit;
	}	
	//4.seek start change something

	fd = hal_fileOpen("/ext/testfile.txt", O_RDWR);
	sysLOG(FILE_LOG_LEVEL_1, "2fileOpen_lib,fd=%d\r\n", fd);
	if(fd < 0)
	{
		goto exit;
	}	
	iRet = hal_fileSeek(fd, 0, SEEK_SET);
	sysLOG(FILE_LOG_LEVEL_1, "2fileSeek_lib,iRet=%d\r\n", iRet);
	if(iRet < 0)
	{
		goto exit;
	}	
	iRet = hal_fileWrite(fd, "\xAA\x55", 2);
	sysLOG(FILE_LOG_LEVEL_1, "2fileWrite_lib,iRet=%d\r\n", iRet);
	if(iRet < 0)
	{
		goto exit;
	}	
exit:

	fibo_free(data);
	iRet = hal_fileClose(fd);
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileClose,iRet=%d filesize:%d\r\n", iRet,hal_fileGetFileSysFreeSize());



}

void secondDirFsTest()
{
		int iRet;
		int fd;
		int i;
		uint8_t *data = fibo_malloc(2048);
	
		
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileGetFileSysFreeSize:%d\r\n", hal_fileGetFileSysFreeSize());

		iRet = hal_fileMkdir("/ext/app");
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileMkdir:%d\r\n", iRet);

		//1.create a file in exfilesys
		fd = hal_fileOpen("/ext/app/testfile.txt", O_CREAT|O_RDWR);
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileOpen,fd=%d\r\n", fd);
		if(fd < 0)
		{
			fibo_free(data);
			return;
		}
		//2.write big file 
		for(i=0;i<2;i++)
		{
			iRet = hal_fileSeek(fd, 0, SEEK_END);
			sysLOG(FILE_LOG_LEVEL_1, "hal_fileSeek,iRet=%d\r\n", iRet);
			if(iRet < 0)
			{
				goto exit;
			}
			memset(data,1,2048);
			iRet = hal_fileWrite(fd, data, 2048);
			sysLOG(FILE_LOG_LEVEL_1, "hal_fileWrite,iRet=%d\r\n", iRet);
			if(iRet < 0)
			{
				goto exit;
			}		
		}
		//3.close
		iRet = hal_fileClose(fd);
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileClose,iRet=%d\r\n", iRet);
		if(iRet < 0)
		{
			goto exit;
		}	
		//4.seek start change something
	
		fd = hal_fileOpen("/ext/app/testfile.txt", O_RDWR);
		sysLOG(FILE_LOG_LEVEL_1, "2fileOpen_lib,fd=%d\r\n", fd);
		if(fd < 0)
		{
			goto exit;
		}	
		iRet = hal_fileSeek(fd, 0, SEEK_SET);
		sysLOG(FILE_LOG_LEVEL_1, "2fileSeek_lib,iRet=%d\r\n", iRet);
		if(iRet < 0)
		{
			goto exit;
		}	
		iRet = hal_fileWrite(fd, "\xAA\x55", 2);
		sysLOG(FILE_LOG_LEVEL_1, "2fileWrite_lib,iRet=%d\r\n", iRet);
		if(iRet < 0)
		{
			goto exit;
		}	
	exit:

		fibo_free(data);
		iRet = hal_fileClose(fd);
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileClose,iRet=%d filesize:%d\r\n", iRet,hal_fileGetFileSysFreeSize());
	

}

void bigNumfileOpenAndGetFileList()
{
	int iRet;
	int fd;
	int i;
	//uint8_t *data = fibo_malloc(2048);

	char rp[]="SFFS";
	
	char fileName[]={"/ext/app/testfile0.txt"};
	
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileGetFileSysFreeSize:%d\r\n", hal_fileGetFileSysFreeSize());

	iRet = hal_fileMkdir("/ext/app");
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileMkdir:%d\r\n", iRet);

	//1.create a file in exfilesys
	for(i=0;i<20;i++)
	{	
		fileName[17] = i+'0';
		fd = hal_fileOpen(fileName, O_CREAT|O_RDWR);
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileOpen,fd=%d,filename:%s\r\n", fd, fileName);
		if(fd < 0)
		{
			return;
		}
		iRet = hal_fileClose(fd);
	}
	//2.get file list 
	iRet = hal_fileGetFileListCB("/ext/app", filegetlistcbtest, rp);
	sysLOG(FILE_LOG_LEVEL_1, "hal_fileGetFileListCB,iRet=%d\r\n", iRet);


}

void clearFileWrite()
{
	char fileName[]={"/ext/app/testfile0.txt"};
	int i;
	int iRet;
	for(i=0;i<20;i++)
	{	
		fileName[17] = i+'0';
		iRet = hal_fileRemove(fileName);
		
		sysLOG(FILE_LOG_LEVEL_1, "hal_fileRemove file:%s,iRet=%d\r\n", fileName, iRet);
	}

	
}

static void hal_sysThreadFiletest(void *param)
{
	uint8_t keyVal;

	sysLOG(FILE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
	
	hal_keypadFlush();	

    while(1)
    {
    	keyVal = hal_keypadGetKey();
		switch(keyVal)
		{
			case '0':
				deleteTestFile();
			break;
			case '1':
				maxFileWriteTest();
			break;
			case '2':
				secondDirFsTest();
			break;
			case '3':
				bigNumfileOpenAndGetFileList();
			break;
			case '4':
				clearFileWrite();
			break;
			default:
				break;
		}
		sysLOG(FILE_LOG_LEVEL_1, "key is : %x\r\n", keyVal);
		delay_ms(1000);
	}

}


#endif

#endif



