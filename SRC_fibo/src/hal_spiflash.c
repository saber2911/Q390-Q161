/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_spiflash.c
** Description:		SPI Flash相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"


#define sFLASH_ID 0xEF4018

#define SPI_FLASH_PageSize 256
#define SPI_FLASH_PerWritePageSize 256

#define W25x_WriteEnable 0x06
#define W25x_writeDisable 0x04
#define W25x_ReadStatusReg 0x05
#define W25x_WriteStatusReg 0x01
#define W25x_ReadData 0x03
#define W25x_FastReadData 0x0B
#define W25x_FastReadDual 0x3B
#define W25x_PageProgram 0x02
#define W25x_BlockErase 0xD8
#define W25x_SectorEarse 0x20
#define W25x_ChipErase 0xC7
#define W25x_PowerDone 0xB9
#define W25x_JedecDeviceID 0x9f

#define SPIBAUD		20*1000*1000
SPIHANDLE spiFd;

SPIFLASHINFO g_strSpiFlashInfo;

int g_iFlash_exist=0; //0-无， 1-有

void  FlashCsPinModeCheck()
{
	return;//@@@
	
	int iRet;
	uint8_t mode = 0;

	iRet = hal_JudgeBpVersion();
	if(iRet < 1)
	{
//		sysLOG_lib(SOCKET_LOG_LEVEL_2, "[%s] -%s- Line=%d:hal_JudgeBpVersion:%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return;
	}
	
	while(1)
	{
		if(NULL != cus_export_api)
		{
			iRet = cus_export_api->fibo_gpio_mode_get(FLASH_SPI_CS_PIN, &mode);
			sysLOG_lib(SOCKET_LOG_LEVEL_2, "[%s] -%s- Line=%d:cus_export_api\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
		}
		else
		{
			sysLOG_lib(SOCKET_LOG_LEVEL_2, "[%s] -%s- Line=%d:NULL == cus_export_api\r\n", filename(__FILE__), __FUNCTION__, __LINE__);
			iRet = 0;
			return;
		}
//		iRet = fibo_gpio_mode_get(FLASH_SPI_CS_PIN, &mode);
		if(iRet != 1)
		{
			sysLOG_lib(SPIFLASH_LOG_LEVEL_1, "[%s] -%s- Line=%d:FLASH_SPI_CS_PIN fibo_gpio_mode_get, iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			sysDelayMs(10);
			//writeErrinfo("FLASH_SPI_CS_PIN fibo_gpio_mode_get, iRet != 1\r\n");	
			continue;
		}
		else
		{ ; }
		
		if(mode != GpioFunction1)
		{
			iRet = fibo_gpio_mode_set(FLASH_SPI_CS_PIN, GpioFunction2);
			sysLOG_lib(SPIFLASH_LOG_LEVEL_1, "[%s] -%s- Line=%d:FLASH_SPI_CS_PIN fibo_gpio_mode_set, iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			if(iRet != 1)
			{
				sysDelayMs(10);
				continue;
			}
			else
			{ ;	}
		}
		else
		{
			break;
		}
		sysDelayMs(50);
	}

}



/*
*Function:		hal_flashErase
*Description:	擦除flash数据
*Input:			writeaddr:写入的flash地址; size:需要写入的长度
*Output:		NULL
*Hardware:
*Return:		>=0:成功写入的长度
*Others:
*/
int hal_flashErase(uint32_t writeaddr, uint32_t size)
{
	int iRet = -1;
	FlashCsPinModeCheck();
	
	iRet = fibo_ext_flash_erase(writeaddr, size);
	sysLOG(SPIFLASH_LOG_LEVEL_4, "fibo_ext_flash_erase, iRet=%d\r\n", iRet);
	return iRet;

}



/*
*Function:		hal_flashSectorErase
*Description:	按扇区擦除
*Input:			sectoraddr:需要擦除的扇区地址
*Output:		NULL
*Hardware:
*Return:		0:成功;<0:失败
*Others:
*/
int hal_flashSectorErase(uint32_t sectoraddr)
{
	int iRet;
	//FlashCsPinModeCheck();
	
	iRet = hal_flashErase(sectoraddr /*& 0x1000*/, 0x1000);
	sysLOG(SPIFLASH_LOG_LEVEL_4, "<SUCC> sectoraddr:0x%08x iRet=%d\r\n",sectoraddr,iRet);
	return iRet;
}


/*
*Function:		hal_flashWrite
*Description:	写flash数据
*Input:			*pbuff:数据指针; writeaddr:写入的flash地址; size:需要写入的长度
*Output:		NULL
*Hardware:
*Return:		>=0:成功写入的长度
*Others:
*/
int hal_flashWrite(uint8_t *pbuff, uint32_t writeaddr, uint16_t size)
{

	int iRet = -1;
	
	FlashCsPinModeCheck();
	iRet = fibo_ext_flash_write(writeaddr, pbuff, size);
	if(iRet == 0)
		return size;
	sysLOG(SPIFLASH_LOG_LEVEL_4, "fibo_ext_flash_write, iRet=%d\r\n", iRet);
	return 0;

}

/*
*Function:		hal_flashAllErase
*Description:	擦除flash，
*Input:			addr:擦除的起始地址；sector_num:需要擦出的扇区个数，一个扇区4Kbytes大小
*Output:		NULL
*Hardware:
*Return:		0-成功;<0-失败
*Others:
*/
int hal_flashAllErase(uint32 addr, uint32        sector_num)
{

	int iRet = -1;
	
	sysLOG(SPIFLASH_LOG_LEVEL_4, "<START> erase all flash,addr=%x, sector_num=%d\r\n", addr, sector_num);

	for(uint16 i=0; i<sector_num; i++)
	{
		iRet = hal_flashSectorErase(addr+i*4096);
	}
	sysLOG(SPIFLASH_LOG_LEVEL_4, "<SUCC>iRet=%d\r\n", iRet);
	return iRet;

}


/*
*Function:		hal_flashRead
*Description:	读flash
*Input:			ReadAddr:需要读取flash的地址;size:需要读到的长度
*Output:		*pbuff:读数据指针
*Hardware:
*Return:		成功读取的长度
*Others:
*/
int hal_flashRead(uint8_t *pbuff, uint32_t ReadAddr, uint16_t size)
{
	int iRet = -1;
	FlashCsPinModeCheck();

	iRet = fibo_ext_flash_read(ReadAddr, pbuff, size);
	sysLOG(SPIFLASH_LOG_LEVEL_4, "fibo_ext_flash_read, iRet=%d\r\n", iRet);
	return iRet;

}

/*
*Function:		hal_flashInit
*Description:	flash初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功
*Others:
*/
int hal_flashInit(void)
{
	int iRet = -1;
	uint32 flashid = 0, flashsize = 0;

	iRet = fibo_ext_flash_poweron(FLASH_SPI_PINSEL, FLASH_SPI_VAL, FLASH_SPI_DIV);
	sysLOG(SPIFLASH_LOG_LEVEL_1, "fibo_ext_flash_poweron, iRet=%d\r\n", iRet);
	iRet = fibo_ext_flash_init(FLASH_SPI_PINSEL);
	sysLOG(SPIFLASH_LOG_LEVEL_1, "fibo_ext_flash_init, iRet=%d\r\n", iRet);
	iRet = fibo_ext_flash_info(&flashid, &flashsize);
	sysLOG(SPIFLASH_LOG_LEVEL_1, "fibo_ext_flash_info, iRet=%d,flashid=%x,flashsize=%d\r\n", iRet, flashid, flashsize);

	g_strSpiFlashInfo.flash_id = flashid;
	g_strSpiFlashInfo.flash_size = flashsize;
	
	return 0;
}


/*************************************TEST*******************************************/

#if MAINTEST_FLAG

uint8_t recvdata[4096] = {0};
uint8_t sendata[4096] = {0};

void hal_flashRDTest(void)
{
    uint16_t i;
    uint16_t j = 0;

    for (i = 0; i < 100; i++)
    {
        memset(sendata, j, 4096);
        j = j + 1;
        if (j == 256)
            j = 0;
        hal_flashErase(4096*i, 4096);
		hal_flashWrite(sendata, 4096*i, 4096);
        uint32_t startTime = osiUpTimeUS();
		hal_flashRead(recvdata, 4096*i, 4096);
        uint32_t endtime = osiUpTimeUS() - startTime;
		sysLOG(SPIFLASH_LOG_LEVEL_1, "read take %d ms\r\n", endtime / 1000);
		sysLOG(SPIFLASH_LOG_LEVEL_1, "sendata:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",\
		sendata[0], sendata[1], sendata[2], sendata[3], sendata[4], sendata[2043], sendata[2044], sendata[2045], sendata[2046], sendata[2047]);
		sysLOG(SPIFLASH_LOG_LEVEL_1, "recvdata:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",\
		recvdata[0], recvdata[1], recvdata[2], recvdata[3], recvdata[4], recvdata[2043], recvdata[2044], recvdata[2045], recvdata[2046], recvdata[2047]);
        memset(recvdata, 0, 4096);
        sysDelayMs(2000);
    }
}

#define FLASHADDR_TMP		4096
void hal_spiflashRDTest(void)
{
    uint16_t i;
    uint16_t j = 2;
	RTC_time rtctimetmp;
	hal_sysGetRTC(&rtctimetmp);
	j = rtctimetmp.sec;

    for (i = 0; i < 10; i++)
    {
        memset(sendata, j, 4096);
        j = j + 1;
        if (j == 256)
            j = 1;
        //hal_flashSectorErase(FLASHADDR_TMP);
        //hal_flashWrite((uint8_t *)sendata, FLASHADDR_TMP, 4096);
        uint32_t startTime = osiUpTimeUS();
        hal_flashRead((uint8_t *)recvdata, FLASHADDR_TMP*i, 4096);
        uint32_t endtime = osiUpTimeUS() - startTime;
		sysLOG(SPIFLASH_LOG_LEVEL_1, "read take %d ms\r\n", endtime / 1000);
		//sysLOG(SPIFLASH_LOG_LEVEL_1, "sendata:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",\
		//sendata[0], sendata[1], sendata[2], sendata[3], sendata[4], sendata[2043], sendata[2044], sendata[2045], sendata[2046], sendata[2047]);
		sysLOG(SPIFLASH_LOG_LEVEL_1, "recvdata:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n",\
		recvdata[0], recvdata[1], recvdata[2], recvdata[3], recvdata[4], recvdata[2043], recvdata[2044], recvdata[2045], recvdata[2046], recvdata[2047]);
        memset(recvdata, 0, 4096);
        sysDelayMs(2000);
    }
}



static void hal_flashThreadTest(void *param)
{	

	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);

	//hal_spiflashRDTest();
	
//	hal_flashInit();
//	hal_flashRDTest();
    
    osiThreadExit();
}




int hal_flashTest(void)
{
	
     fibo_thread_create(hal_flashThreadTest, "hal_flashThreadTest", 1024*8, NULL, OSI_PRIORITY_ABOVE_NORMAL);
          
}


/******************flash性能测试***********************/

#define FILE_MAX_SIZE  256
#define MAX_TIME_TO_HANDLE 1024
extern unsigned char hal_keypadWaitOneKey(void);
static int err = 0; 
static int succ = 0;
void fileStressTest()
{
	
	int iRet;
	int fd;
	int i;
	char dataW[128];
	unsigned char writedata = 0;
	
	uint8_t *data = (uint8_t *)fibo_malloc(FILE_MAX_SIZE+32);

	uint8_t *dataCmp = (uint8_t *)fibo_malloc(FILE_MAX_SIZE+32);

	 err = 0; 
	 succ = 0;
	 hal_scrCls();
	 

	while(1)
	{
		sysLOG(BASE_LOG_LEVEL_1, " hal_fileGetFileSysFreeSize:%d  data:%p,dataCmp:%p\r\n", hal_fileGetFileSysFreeSize(),data,dataCmp);
		
		hal_scrClrLine(2, 3);
		hal_fileRemove("/ext/testfile.txt");
		sysDelayMs(20);
		//1.create a file in exfilesys
		fd = hal_fileOpen("/ext/testfile.txt", O_CREAT|O_RDWR);
		sysLOG(BASE_LOG_LEVEL_1, " hal_fileOpen,fd=%d\r\n", fd);
		if(fd < 0)
		{
			//fibo_free(data);
			//fibo_free(dataCmp);
			err++;
			goto exit;
		}
		//2.write big file 
		for(i=0;i<MAX_TIME_TO_HANDLE;i++)
		{
			//iRet = hal_fileSeek(fd, 0, SEEK_END);
			iRet = hal_fileSeek(fd, i*FILE_MAX_SIZE, SEEK_SET);
			sysLOG(BASE_LOG_LEVEL_1, " hal_fileSeek,iRet=%d\r\n", iRet);
			if(iRet < 0)
			{
				err++;
				goto exit;
			}
			writedata = i%256;
			memset(data,writedata,FILE_MAX_SIZE);
			iRet = hal_fileWrite(fd, data, FILE_MAX_SIZE);
			sysLOG(BASE_LOG_LEVEL_1, " hal_fileWrite,iRet=%d\r\n", iRet);
			if(iRet < 0)
			{
				err++;
				goto exit;
			}
			//sysDelayMs(10);
			hal_scrPrint(0, 2,0, "flash write:%d",i);	
			if(i%50 == 0)
			sysDelayMs(10);
			
		}
		//3.close
		iRet = hal_fileClose(fd);
		sysLOG(BASE_LOG_LEVEL_1, " hal_fileClose,iRet=%d\r\n", iRet);
		if(iRet < 0)
		{
			err++;
			goto exit;
		}	
		//4.read file cmp 

		fd = hal_fileOpen("/ext/testfile.txt", O_RDWR);
		sysLOG(BASE_LOG_LEVEL_1, " 2fileOpen_lib,fd=%d\r\n", fd);
		if(fd < 0)
		{
			err++;
			goto exit;
		}	
		
		for(i=0;i<MAX_TIME_TO_HANDLE;i++)
		{
			iRet = hal_fileSeek(fd, i*FILE_MAX_SIZE, SEEK_SET);
			sysLOG(BASE_LOG_LEVEL_1, " hal_fileSeek,iRet=%d\r\n", iRet);
			if(iRet < 0)
			{
				err++;
				goto exit;
			}
			writedata = i%256;
			memset(dataCmp,writedata,FILE_MAX_SIZE);
			iRet = hal_fileRead(fd, data, FILE_MAX_SIZE);
			sysLOG(BASE_LOG_LEVEL_1, " hal_fileRead,iRet=%d\r\n", iRet);
			if(iRet < 0)
			{
				err++;
				goto exit;
			}	
			if(0 != memcmp(data,dataCmp,FILE_MAX_SIZE))
			{
				err++;
				sysLOG(BASE_LOG_LEVEL_1, " memcmp fail data[0]:%d data[255]:%d cmp[0]:%d cmp[255]:%d\r\n",data[0],data[255],dataCmp[0],dataCmp[255]);
				goto exit;
			}
			hal_scrPrint(0, 3,0, "flash read:%d",i);
			if(i%50 == 0)
				sysDelayMs(10);
			
		}

		succ++;
		
	exit:

		//fibo_free(data);
		//fibo_free(dataCmp);
		
		iRet = hal_fileClose(fd);
		sysLOG(BASE_LOG_LEVEL_1, " f2ileClose_lib,iRet=%d filesize:%d\r\n", iRet,hal_fileGetFileSysFreeSize());

		sysLOG(BASE_LOG_LEVEL_1, " FILE_STRESS TEST,SUCC=%d err:%d\r\n", succ,err);
		hal_scrPrint(0, 4, 0, "test succ:%d,err:%d",succ,err);
		memset(dataW,0,sizeof(dataW));
		sprintf(dataW,"test succ:%d,err:%d",succ,err);
		if(hal_keypadGetKey() == KEYCANCEL)
			break;
		//"/ext/testfile.txt", O_CREAT|O_RDWR
		int fdd = hal_fileOpen("/fileStress.txt", O_CREAT|O_RDWR);
		hal_fileSeek(fdd, 0, SEEK_SET);
		hal_fileWrite(fdd, dataW, strlen(dataW)+1);
		hal_fileClose(fdd);
		sysDelayMs(50);
	}
	hal_keypadWaitOneKey();	

}

#endif


