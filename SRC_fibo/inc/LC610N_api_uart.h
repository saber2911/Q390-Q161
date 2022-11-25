#ifndef LC610N_API_UART_H
#define LC610N_API_UART_H

#include "comm.h"

/*typedef enum
{
P_DEBUG = 0,
P_APP,
P_WIFI,
P_GPRS,
P_BT = 7,
P_USB = 10,
P_RS232 = 11,
P_SER_DEFAULT = 0xFF
}SER_PORTNUM_t;*/

int portOpenEx(SER_PORTNUM_t emPort,char *Attr);
int portCloseEx(SER_PORTNUM_t emPort);
int portFlushBufEx(SER_PORTNUM_t emPort);
int portSendsEx(SER_PORTNUM_t emPort,uchar *str, ushort str_len);
int portRecvsEx(SER_PORTNUM_t emPort, uchar *pszBuf, ushort usBufLen, ushort usTimeoutMs);
#endif