/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_usb.h
** Description:		USB相关接口
**
** Version:	1.0, 渠忠磊,2022-03-09
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_USB_H_
#define _HAL_USB_H_ 

#include "comm.h"

#define PORT_LOG_LEVEL_0		LOG_LEVEL_0
#define PORT_LOG_LEVEL_1		LOG_LEVEL_1
#define PORT_LOG_LEVEL_2		LOG_LEVEL_2
#define PORT_LOG_LEVEL_3		LOG_LEVEL_3
#define PORT_LOG_LEVEL_4		LOG_LEVEL_4
#define PORT_LOG_LEVEL_5		LOG_LEVEL_5



extern unsigned char first_find_Q;
extern struct _BUFF_STRUCT g_stUsbBuffStr;//USB数据缓存结构体

#define USBRECVBUFFSIZE		9*1024
#define USBTIMEOUT			500

#define	SHAKE_REQUEST		            'Q'
#define	SHAKE_REPLY	                    'R'
#define CMD_POS_NAME                    'T'
#define CMD_POS_NAME_REPLY              0x10
#define CMD_TRASHY                      'P'
#define	ACK		                        0x06
#define	STX		                        0x02

#define	CMD_ENQ		                    0xE1    //enquire terminal's  type 
#define	CMD_DOWNLOAD_MON	            0xE3    //download monitor to sdram   
#define	CMD_RUN_AT_SDRAM	            0xE4    //run monitor on sdram
#define	CMD_WRITE_TO_FLASH	            0xE6    // write monitor to flash
#define	CMD_CLEAR_FAT                   0xE7    //clear fat
#define	CMD_ERASE_SEK		            0xE8    //ERASE SEK
#define	CMD_UPDATE_SN		            0xE9    //update sn
#define	CMD_UPDATE_EXSN	                0xEA    //UPDATE EXT SN
#define	CMD_UPDATE_MACADDR	            0xEB    //UPDATE MAC ADDRESS
#define CMD_WRITE_CFGFILE_TO_FLASH      0xED
#define CMD_DOWNLOAD_CFGFILE            0xEC
#define CMD_READ_INFO                   0xEE    //Read SN,EXSN
#define CMD_ERASE_FLASH_LABEL           0xEF    //恢复bootloader下载

int hal_portUsbSEHandle(void);
int hal_portUsbSECommitHandle(void);
int hal_portUsbWiFiHandle(void);

int hal_portUsbInit(void);
int hal_portUsbOpen(void);
int hal_portUsbClose(void);
int hal_portUsbFlushBuff(void);
int hal_portUsbSend(char *data, int len);
int hal_portUsbRecv(char *data, int len, uint32 timeout);

int hal_portOpen(SER_PORTNUM_t emPort, char *Attr);
int hal_portClose(SER_PORTNUM_t emPort);
int hal_portFlushBuf(SER_PORTNUM_t emPort);
int hal_portSends(SER_PORTNUM_t emPort, uchar *str, ushort str_len);
int hal_portRecvs(SER_PORTNUM_t emPort, uchar *pszBuf, ushort usBufLen, ushort usTimeoutMs);

void hal_portUSBTest(void);


#endif

