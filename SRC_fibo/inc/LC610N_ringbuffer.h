#ifndef _LC610N_RINGBUFFER_
#define _LC610N_RINGBUFFER_

#include "comm.h"


//#define RINGBUFFERSIZE (24*48*4)

typedef struct
{
    //unsigned char buffer[RINGBUFFERSIZE];      //存放实际的数据
	unsigned char *buffer;
	unsigned int read_offset;  //读取地址相对buffer的偏移量
    unsigned int write_offset; //写入地址相对buffer的偏移量
    unsigned int valid_size;   //buffer的有效size
    unsigned int total_size;   //buffer的总大小，即init时malloc的size
} ring_buffer_t;



int ring_buffer_init(ring_buffer_t *ring_buffer, unsigned int buff_size);
void ring_buffer_deinit(ring_buffer_t *ring_buffer);
void ring_buffer_delete(ring_buffer_t *ring_buffer);
unsigned int get_ringBuffer_length(ring_buffer_t *ringBuf);
unsigned int get_ringBuffer_valid(ring_buffer_t *ringBuf);
int ring_buffer_write(unsigned char *buffer_to_write, unsigned int size, ring_buffer_t *ring_buffer);
int ring_buffer_read(ring_buffer_t *ring_buffer, unsigned char *buff, unsigned int size);
unsigned int get_ringBuffer_freeSize(ring_buffer_t *ringBuf);
int clear_ringBuffer(ring_buffer_t *ringBuf, unsigned int avalid_len);




#endif

