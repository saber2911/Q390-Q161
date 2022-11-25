/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_spiflash.h
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

#ifndef _HAL_SPIFLASH_H_
#define _HAL_SPIFLASH_H_

#include "comm.h"



#define SPIFLASH_LOG_LEVEL_0		LOG_LEVEL_0
#define SPIFLASH_LOG_LEVEL_1		LOG_LEVEL_1
#define SPIFLASH_LOG_LEVEL_2		LOG_LEVEL_2
#define SPIFLASH_LOG_LEVEL_3		LOG_LEVEL_3
#define SPIFLASH_LOG_LEVEL_4		LOG_LEVEL_4
#define SPIFLASH_LOG_LEVEL_5		LOG_LEVEL_5


/*
*spiflashID:
*
*EN25S64:0x17381C;
*GD25LQ64:0x1760C8;
*W25Q64JWIQ:0x1760EF;
*W25Q64JWIM:0x1780EF;
*
*/

#define FLASH_EN25S64_ID		0x17381C
#define FLASH_GD25LQ64_ID		0x1760C8
#define FLASH_W25Q64JWIQ_ID		0x1760EF
#define FLASH_W25Q64JWIM_ID		0x1780EF

#define FLASH_SPI_PINSEL		0//0-lcd,1-PCM
#define FLASH_SPI_VAL			1//0-1.8V,1-3.2V
#define FLASH_SPI_DIV			2//1-31MHZ,2-41MHZ,3-62MHZ,4-71MHZ,5-83MHZ,6-90MHZ,7-100MHZ,8-111MHZ,9-125MHZ
#define FLASH_SPI_MODE			0//0-dual_spi,1-quad_spi(dafult)

typedef struct _SPIFLASHINFO{

	uint32 flash_id;
	uint32 flash_size;
	uint32 flash_mountaddr;
	uint32 flash_mountsize;

}SPIFLASHINFO;

extern int g_iFlash_exist;
extern SPIFLASHINFO g_strSpiFlashInfo;

int hal_flashSectorErase(uint32_t sectoraddr);
int hal_flashAllErase(uint32 addr, uint32        sector_num);
int hal_flashWrite(uint8_t *pbuff, uint32_t writeaddr, uint16_t size);
int hal_flashRead(uint8_t *pbuff, uint32_t ReadAddr, uint16_t size);
int hal_flashInit(void);

int hal_flashTest(void);

void  FlashCsPinModeCheck();


#endif



