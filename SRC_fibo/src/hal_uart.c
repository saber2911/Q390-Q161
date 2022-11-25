/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_uart.c
** Description:		uart相关接口
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

/*
*Function:		hal_utSEUartRecv
*Description:	Uart0接收回调接口
*Input:			uart_port:端口号;*data:接受到的数据指针;len:接收到的数据长度;*arg:形参指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_utSEUartRecv(hal_uart_port_t uart_port, INT8 *data, UINT16 len, void *arg)
{
	uint8 *rP = NULL;
	int32 iRet = -1;

	sysLOG(UART_LOG_LEVEL_4, " len = %d\r\n",len);
	ring_buffer_write(data, len, &se_rev_ringbuff);
}


/*
*Function:		hal_utSEUartInit
*Description:	Uart0初始化
*Input:			baudrate:波特率
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_utSEUartInit(uint32_t baudrate)
{
	fibo_gpio_mode_set(SE_TXD_GPIO, GpioFunction1);
	fibo_gpio_mode_set(SE_RXD_GPIO, GpioFunction1);
	
	int port = SE_COMMUNICATION_PORT;
	hal_uart_config_t uart0_drvcfg = {0};
	int iRet;
	iRet =	fibo_hal_uart_deinit(port);
	sysLOG(UART_LOG_LEVEL_1, " fibo_hal_uart_deinit iRet = %d baudrate=%d\r\n",iRet,baudrate);

    uart0_drvcfg.baud = baudrate;//921600;//115200;
    uart0_drvcfg.parity = HAL_UART_NO_PARITY;
    uart0_drvcfg.data_bits = HAL_UART_DATA_BITS_8;
    uart0_drvcfg.stop_bits = HAL_UART_STOP_BITS_1;
    uart0_drvcfg.rx_buf_size = 5*1024;
    uart0_drvcfg.tx_buf_size = 5*1024;
	uart0_drvcfg.recv_timeout = 1;

    iRet = fibo_hal_uart_init(port, &uart0_drvcfg, hal_utSEUartRecv, NULL);
	sysLOG(UART_LOG_LEVEL_1, " fibo_hal_uart_init iRet = %d baudrate=%d \r\n",iRet,baudrate);
}




