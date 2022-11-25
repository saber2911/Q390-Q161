#include "comm.h"


/**
 1. 初始化ring_buffer
 2. malloc开辟传入的buff_size大小的空间存放buffer
 3. read_offset write_offset valid_size均置为０
*/
int ring_buffer_init(ring_buffer_t *ring_buffer, unsigned int buff_size)
{
    ring_buffer->buffer = (unsigned char*)fibo_malloc(buff_size);
	if(NULL == ring_buffer->buffer)
	{
		sysLOG(BASE_LOG_LEVEL_1, " malloc error\r\n");	
		return -1;
	}
    memset(ring_buffer->buffer, 0, buff_size);

    ring_buffer->read_offset = 0;
    ring_buffer->write_offset = 0;
    ring_buffer->valid_size = 0;
    ring_buffer->total_size = buff_size;

	return 0;
}
/**
 *释放ring_buffer 
 */
void ring_buffer_delete(ring_buffer_t *ring_buffer)
{
    fibo_free(ring_buffer->buffer);
    ring_buffer->read_offset = 0;
    ring_buffer->write_offset = 0;
    ring_buffer->valid_size = 0;
}

/**
 *复位ring_buffer 
 */
void ring_buffer_deinit(ring_buffer_t *ring_buffer)
{
    memset(ring_buffer->buffer, 0, ring_buffer->total_size);
    ring_buffer->read_offset = 0;
    ring_buffer->write_offset = 0;
    ring_buffer->valid_size = 0;
}
unsigned int get_ringBuffer_length(ring_buffer_t *ringBuf)
{
	return ringBuf->total_size;
}
unsigned int get_ringBuffer_valid(ring_buffer_t *ringBuf)
{
	return ringBuf->valid_size;
}

unsigned int get_ringBuffer_freeSize(ring_buffer_t *ringBuf)
{
	return get_ringBuffer_length(ringBuf) - get_ringBuffer_valid(ringBuf); 
}

int clear_ringBuffer(ring_buffer_t *ringBuf, unsigned int avalid_len)
{
	if(avalid_len > ringBuf->total_size)
	{
		sysLOG(BASE_LOG_LEVEL_2, " avalid_len error\r\n");		
		return -1;
	}
	
	if(ringBuf->write_offset >= ringBuf->read_offset)
	{
		memset(ringBuf->buffer + ringBuf->read_offset, 0, avalid_len);
	}
	else
	{
		memset(ringBuf->buffer + ringBuf->read_offset, 0, ringBuf->total_size - ringBuf->read_offset);
		memset(ringBuf->buffer, 0, ringBuf->write_offset);
	}
    ringBuf->read_offset = ringBuf->write_offset = ringBuf->valid_size = 0;
	return 0;
}

/**
 * buffer_to_write:需要写入的数据的地址
 * size:需要写入的数据的大小
*/
int ring_buffer_write(unsigned char *buffer_to_write, unsigned int size, ring_buffer_t *ring_buffer)
{
    unsigned int write_offset = ring_buffer->write_offset;
    unsigned int total_size = ring_buffer->total_size;
    unsigned int first_write_size = 0;

    if (ring_buffer->valid_size + size > total_size) //ring_buffer->buffer未使用的总大小比需要写入的size小
    {
		sysLOG(BASE_LOG_LEVEL_2, " too short to save,total_size:%d valid_size:%d size:%d\r\n",ring_buffer->total_size, ring_buffer->valid_size, size);		        
        return -1;
    }

    if (size + write_offset <= total_size) //ring_buffer->buffer的后段未写入的空间不小于size
    {
        memcpy(ring_buffer->buffer + write_offset, buffer_to_write, size);
    }
    else //ring_buffer->buffer的后段未写入的空间小于size,这时候需要先在后面写入一部分，然后返回头部，从前面接着写入
    {
        first_write_size = total_size - write_offset;
        memcpy(ring_buffer->buffer + write_offset, buffer_to_write, first_write_size);
        memcpy(ring_buffer->buffer, buffer_to_write + first_write_size, size - first_write_size);
    }
    ring_buffer->write_offset += size;
    ring_buffer->write_offset %= total_size;
    ring_buffer->valid_size += size;

	sysLOG(BASE_LOG_LEVEL_4, " valid_size:%d\r\n",ring_buffer->valid_size);				

	return size;
}

int ring_buffer_read(ring_buffer_t *ring_buffer, unsigned char *buff, unsigned int size)
{
    unsigned int read_offset = ring_buffer->read_offset;
    unsigned int total_size = ring_buffer->total_size;
    unsigned int first_read_size = 0;

    if (size > ring_buffer->valid_size)
    {
		sysLOG(BASE_LOG_LEVEL_2, " not enough to read,valid size:%d read size:%d\r\n",ring_buffer->valid_size, size);		                
        return -1;
    }

    if (total_size - read_offset >= size)
    {
        memcpy(buff, ring_buffer->buffer + read_offset, size);
    }
    else
    {
        first_read_size = total_size - read_offset;
        memcpy(buff, ring_buffer->buffer + read_offset, first_read_size);
        memcpy(buff + first_read_size, ring_buffer->buffer, size - first_read_size);
    }

    ring_buffer->read_offset += size;
    ring_buffer->read_offset %= total_size;
    ring_buffer->valid_size -= size;
	sysLOG(BASE_LOG_LEVEL_4, " valid_size:%d\r\n",ring_buffer->valid_size);					
	return size;
}



/*
void create_ring_buffer(ring_buffer_t *ring_buf, unsigned char *buf, unsigned int buf_len)
{
	ring_buf->br         = 0;
	ring_buf->bw         = 0;
	ring_buf->btoRead    = 0;
	ring_buf->source     = buf;
	ring_buf->length     = buf_len;
	debug("create ring_buf->length = %d\r\n", ring_buf->length);
}
void clear_ring_buf(ring_buffer_t *ring_buf)
{
	ring_buf->br         = 0;
	ring_buf->bw         = 0;
	ring_buf->btoRead    = 0;
}
unsigned int get_ringBuffer_length(ring_buffer_t *ringBuf)
{
	return ringBuf->length;
}
unsigned int get_ringBuffer_btoRead(ring_buffer_t *ringBuf)
{
	return ringBuf->btoRead;
}

unsigned int write_ring_buffer(unsigned char *buffer, unsigned int size, ring_buffer_t *ring_buf)
{
	unsigned int len            = 0;
	unsigned int ring_buf_bw     = ring_buf->bw;
	unsigned int ring_buf_len    = ring_buf->length;
	unsigned char *ring_buf_source = ring_buf->source;
	
	if( (ring_buf_bw + size) <= ring_buf_len  ) //当前写位置+数据size <= ring_buf_len  
	{
		memcpy(ring_buf_source + ring_buf_bw, buffer, size);
	}
	else
	{
		len = ring_buf_len - ring_buf_bw;
		memcpy(ring_buf_source + ring_buf_bw, buffer, len);
		memcpy(ring_buf_source, buffer + len, size - len); //这里注意，因为从buffer开始的数据已经在第一个memcpy函数中
		                                                   //被拷贝了len这么多，所以剩下的数据应该从buffer+len开始。
	}

	ring_buf->bw = (ring_buf->bw + size) % ring_buf_len;
	ring_buf->btoRead += size;

	return size;
}
unsigned int read_ring_buffer(unsigned char *buffer, unsigned int size, ring_buffer_t *ring_buf)
{
	unsigned int len            = 0;
	unsigned int ring_buf_br     = ring_buf->br;
	unsigned int ring_buf_len    = ring_buf->length;
    unsigned char *ring_buf_source = ring_buf->source;

	if( (ring_buf_br + size ) <= ring_buf_len ) //当前读位置+数据size <= ring_buf_len 
	{
		memcpy(buffer, ring_buf_source + ring_buf_br, size);
	}
	else
	{
		len = ring_buf_len - ring_buf_br;
		memcpy(buffer, ring_buf_source + ring_buf_br, len);
		memcpy(buffer + len, ring_buf_source, size - len); //这里跟往ring_buffer思路类似，也只有这样才有环的概念了
	}

	ring_buf->br = (ring_buf->br + size) % ring_buf_len;
	ring_buf->btoRead -= size;

	return size;
}
*/

