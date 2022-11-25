#ifndef _LC610N_SE_COMMUNICATION_H_
#define _LC610N_SE_COMMUNICATION_H_

#include "comm.h"
#include "LC610N_ringbuffer.h"


#define COM_OK						0
#define COM_DATA_NULL               -401
#define COM_SEND_ERROR              -402
#define COM_RECEIVE_HEAD_TIMEOUT    -403
#define COM_RECEIVE_BODY_TIMEOUT    -404
#define COM_RECEIVE_FRAME_ERROR     -405
#define COM_RECEIVE_CHECKSUM_ERR    -406
#define COM_RECEIVE_NULL            -407
#define COM_RECEIVE_HEAD_ERROR      -408
#define IO_READ_ERROR               -409
#define COM_BEEN_LOCKED             -410
#define COM_BEEN_STOPPED            -411
#define COM_WAIT_IO_TIMEOUT         -412
#define COM_SEND_ERR_SE_BUSY        -413
#define COM_MALLOC_ERROR	-414
#define COM_ISR_HANDLE       -415
	

#define HEADSIZE 6
#define FRAME_SEND_TIMES 5
#define SE_COMMUNICATION_PORT	UART2_PORT
#define SE_FRAMEREV_BUFF_SIZE	(5*1024)

#define DEFAULT_RATE  921600
#define DOWNLOAD_RATE  115200 


typedef struct
{
	//int headsize=6;
	unsigned char tag;
	unsigned short length;
	unsigned char sequence;
	unsigned char state;
	unsigned char reserve;

	unsigned char *data;
	unsigned char checksum;
} Frame;

extern char  is_se_sleepping;
extern ring_buffer_t se_rev_ringbuff;

void scanpos_se_comm_init(void);
int test_serialport(unsigned char *pData, int dataLen, unsigned char *pBuffer, int *bufferLen);


#endif

