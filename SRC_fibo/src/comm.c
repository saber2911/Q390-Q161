/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_comm.c
** Description:		通用接口
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

volatile uint8 g_ui8LogLevel = 2;
volatile static int g_iBuffMutex = 0;

int Vsnprintf(char *str, size_t size, const char *format, ...)
{
	int n;
	va_list vp;

//	if (size < 2)
//        return 0;
	
	va_start(vp, format);
	vsnprintf((char *)str, size, format, vp);
	va_end(vp);
	//return (n < size) ? n : (size - 1);
}

/*
*Function:		sysLogSet
*Description:	设置log等级
*Input:			level: log输出等级
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void sysLogSet(LOG_LEVEL level)
{
	g_ui8LogLevel = level;	
	sysLOG(COMM_LOG_LEVEL_1, "g_ui8LogLevel:%d\r\n", g_ui8LogLevel);
}

/*
*Function:		sysLogGet
*Description:	获取log等级
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		log输出等级
*Others:
*/
LOG_LEVEL sysLogGet(void)
{	
	sysLOG(COMM_LOG_LEVEL_5, "g_ui8LogLevel:%d\r\n", g_ui8LogLevel);
	return g_ui8LogLevel;
}

char syslogBuffDebug[4096] = {0};


//void sysLOG(uint8 level, const char *fmt, ...)
//{
//
//	if(level <= g_ui8LogLevel)
//	{
//		va_list vp;
//		
//		
//		memset(syslogBuffDebug, 0, 4096);
//		va_start(vp, fmt);
//		vsnprintf((int8 *)syslogBuffDebug,4095, fmt, vp);
//		va_end(vp);
//		
//		sysLOG_Debug(level, "%s", syslogBuffDebug);
//
//	}
//
//}


///*
//*Function:		hal_commConvert
//*Description:	二进制取反
//*Input:			*src:二进制数据
//*				length:待转换的二进制数据长度
//*Output:		*dst:取反后的二进制数据
//*Hardware:
//*Return:		0:success
//*Others:
//*/
//int hal_commConvert(unsigned char *dst, const unsigned char *src, int length)
//{
//        int i;
//        for(i=0; i<length; i++)
//        {
//                dst[i] = src[i]^0xFF;
//        }
//        return 0;
//}


/*
*Function:		HexToDec
*Description:	十六进制转为十进制
*Input:			*hex:待转换的十六进制数据
*				length:十六进制数据长度
*Output:		NULL
*Hardware:
*Return:		转换后的十进制数据
*Others:		十六进制每个字符位所表示的十进制数的范围是0 ~255，进制为256,左移8位(<<8)等价乘以256
*/
unsigned long HexToDec(const unsigned char *hex, int length)
{
    int i;
    unsigned long rslt = 0;
    for(i=0; i<length; i++)
    {
        rslt += (unsigned long)(hex[i])<<(8*(length-1-i));
                                                        
    }
    return rslt;
}


/*
*Function:		DecToHex
*Description:	十进制转十六进制
*Input:			dec:待转换的十进制数据
*				length:转换后的十六进制数据长度
*Output:		*hex:转换后的十六进制数据
*Hardware:
*Return:		0:success
*Others:		原理同十六进制转十进制
*/
int DecToHex(int dec, unsigned char *hex, int length)
{
    int i;
    for(i=length-1; i>=0; i--)
    {
        hex[i] = (dec%256)&0xFF;
        dec /= 256;
    }
    return 0;
}


/*
*Function:		ClaimPower
*Description:	求权
*Input:			base:进制基数
*				times:权级数
*Output:		NULL
*Hardware:
*Return:		当前数据位的权
*Others:		
*/
static unsigned long ClaimPower(int base, int times)
{
    int i;
    unsigned long rslt = 1;
    for(i=0; i<times; i++)
        rslt *= base;
    return rslt;
}


/*
*Function:		BcdToDec
*Description:	BCD转10进制
*Input:			*bcd:待转换的BCD码
*				length:BCD码数据长度
*Output:		NULL
*Hardware:
*Return:		转换后的十进制数值
*Others:		
*/
unsigned long  BcdToDec(const unsigned char *bcd, int length)
{
     int i, tmp;
     unsigned long dec = 0;
     for(i=0; i<length; i++)
     {
        tmp = ((bcd[i]>>4)&0x0F)*10 + (bcd[i]&0x0F);   
        dec += tmp * ClaimPower(100, length-1-i);          
     }
     return dec;
}


/*
*Function:		DecToBcd
*Description:	十进制转BCD码
*Input:			Dec:待转换的十进制数据
*				length:BCD码数据长度
*Output:		*Bcd:转换后的BCD码
*Hardware:
*Return:		0-success
*Others:		
*/
int DecToBcd(int Dec, unsigned char *Bcd, int length)
{
     int i;
     int temp;
     for(i=length-1; i>=0; i--)
     {
         temp = Dec%100;
         Bcd[i] = ((temp/10)<<4) + ((temp%10) & 0x0F);
         Dec /= 100;
     }
     return 0;
}


/*
*Function:		HexToStr
*Description:	hex转string
*Input:			*inchar:hex指针; len:输入长度;
*Output:		*outtxt:字符串指针
*Hardware:
*Return:		字符串长度
*Others:		
*/
unsigned int HexToStr(char *inchar, unsigned int len, char *outtxt)
{
	unsigned char hbit,lbit;
	unsigned int i;
	for(i=0;i<len;i++)
	{
		hbit = (*(inchar+i)&0xf0)>>4;
		lbit = *(inchar+i)&0x0f;
		if (hbit>9) outtxt[2*i]='A'+hbit-10;
		else outtxt[2*i]='0'+hbit;
		if (lbit>9) outtxt[2*i+1]='A'+lbit-10;
		else outtxt[2*i+1]='0'+lbit;
	}
	outtxt[2*i] = 0;
	return 2*i;
}


/*
*Function:		StrToHex
*Description:	string转hex
*Input:			*strs:tring指针
*Output:		*hex:hex指针
*Hardware:
*Return:		hex字节长度
*Others:		
*/
unsigned int StrToHex(char *str, char *hex)
{
	unsigned char ctmp, ctmp1,half;
	unsigned int num=0;
	do{
		do{
			half = 0;
			ctmp = *str;
			if(!ctmp) break;
			str++;
		}while((ctmp == 0x20)||(ctmp == 0x2c)||(ctmp == '\t'));
		if(!ctmp) break;
		if(ctmp>='a') ctmp = ctmp -'a' + 10;
		else if(ctmp>='A') ctmp = ctmp -'A'+ 10;
		else ctmp=ctmp-'0';
		ctmp=ctmp<<4;
		half = 1;
		ctmp1 = *str;
		if(!ctmp1) break;
		str++;
		if((ctmp1 == 0x20)||(ctmp1 == 0x2c)||(ctmp1 == '\t'))
		{
			ctmp = ctmp>>4;
			ctmp1 = 0;
		}
		else if(ctmp1>='a') ctmp1 = ctmp1 - 'a' + 10;
		else if(ctmp1>='A') ctmp1 = ctmp1 - 'A' + 10;
		else ctmp1 = ctmp1 - '0';
		ctmp += ctmp1;
		*hex = ctmp;
		hex++;
		num++;
	}while(1);
	if(half)
	{
		ctmp = ctmp>>4;
		*hex = ctmp;
		num++;
	}
	return(num);


} 


/*
*Function:		BuffLeftMove
*Description:	数组循环左移一位
*Input:			*buffer:数组指针; buf_len:数组长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void BuffLeftMove(int8 *buffer , int32 buf_len)
{
	int32 i ;
	uint8 tmp = buffer[0];
	for(i = 1 ; i < buf_len ; i++)
	{
		buffer[i-1] = buffer[i] ;
	}
	buffer[buf_len-1] = tmp ;
}
 

/*
*Function:		BuffRightMove
*Description:	数组循环右移一位
*Input:			*buffer:数组指针; buf_len:数组长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void BuffRightMove(int8 *buffer , int32 buf_len)
{
	int32 i ;
	uint8 tmp = buffer[buf_len - 1];
	for(i = buf_len ; i > 0 ; i--)
	{
		buffer[i] = buffer[i-1] ; 
	}
	buffer[0] = tmp ;
}
 

/*
*Function:		BuffTurnOver
*Description:	数组翻转
*Input:			*buffer:数组指针; buf_len:数组长度
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void BuffTurnOver(int8 *buffer , int32 buf_len)
{
	int32 i, size = buf_len ; 
	uint8 tmp; 
	for(i = 0 ; i < size/2 ; i++)
	{
		tmp = buffer[i] ; 
		buffer[i] = buffer[size-1-i] ;
		buffer[size-1-i] = tmp;
	}
}

/*
*Function:		BuffOrder
*Description:	Buff中的数值从小到达排序
*Input:			*buf:需要排序的数据指针;len:数据个数
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void BuffOrder(int32 *buf, uint32 len)
{
	uint32 i, j;
	int32 datatmp;
	for(j = 0; j< (len-1); j++)
	{
		for(i = 0; i < (len-j-1); i++)
		{
			if(*(buf+i) > *(buf+i+1))
			{
				datatmp = *(buf+i);
				*(buf+i) = *(buf+i+1);
				*(buf+i+1) = datatmp;
			}
				
		}
	}
}


/*
*Function:		CalLRC
*Description:	LRC计算
*Input:			*pStr:输入数据指针; lLength:数据长度
*Output:		NULL
*Hardware:
*Return:		LRC结果
*Others:		
*/
BYTE CalLRC(BYTE *pStr, DWORD lLength)
{
 BYTE bValue=0;
 for(DWORD i=0; i<lLength; i++)
  bValue ^= pStr[i];
 
 return bValue;
}


/*
*Function:		MyStrStr
*Description:	在buf里查找字符串,,有时间了实现出来可以writeP在readP之前的功能
*Input:			*pcSrc:数据指针; *pcDes:要查找的字符串指针; readP:读指针位置; writeP:写指针位置
*Output:		NULL
*Hardware:
*Return:		NULL:未找到; 其他:要查找的*pcDes字符串地址
*Others:		
*/
char* MyStrStr(char *pcSrc, char *pcDes, uint32 readP, uint32 writeP)
{
 char *pcStrH = 0;
 const char *bp = NULL;
 const char *sp = NULL;
 int iLen = 0;
 int iSrcLen = 0;
 int iSubLen = strlen(pcDes);
 
 if(0 == iSubLen)
 {
  return NULL;
 }

 iSrcLen = writeP - readP;
 if((iSrcLen <= 0) || (iSrcLen < iSubLen))
 {
  return NULL;
 }

 pcStrH = pcSrc;

 while(1)
 {
  bp = pcStrH;
  
  sp = pcDes;
 
  do
  {
   if(!*sp)
    return pcStrH;
  }while(*bp++==*sp++);
  
  pcStrH++;
  iLen++;

  iSrcLen = writeP - readP;
  if((iLen >= iSrcLen) || ((iSrcLen - iLen) < iSubLen))
  {
   return NULL;
  }
 }

 return NULL;
}


/*
*Function:		CBuffFormat
*Description:	格式化BUFF_STRUCT结构体
*Input:			*buff_struct:BUFF_STRUCT指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void CBuffFormat(BUFF_STRUCT *buff_struct)
{
	buff_struct->read_P = 0;
	buff_struct->write_P = 0;
	buff_struct->count = 0;
	buff_struct->invailddatalen = 0;
	if((int)(buff_struct->bufflen) > 0)
	{
		memset(buff_struct->buff, 0, buff_struct->bufflen);
	}
	else
	{
		sysLOG(COMM_LOG_LEVEL_1, "<ERR> buff_struct->bufflen:%d\r\n", buff_struct->bufflen);
	}
}


/*
*Function:		CBuffInit
*Description:	初始化BUFF_STRUCT结构体
*Input:			*buff_struct:BUFF_STRUCT指针
*Output:		NULL
*Hardware:
*Return:		<0:失败; 0:成功
*Others:		
*/
int CBuffInit(BUFF_STRUCT *buff_struct, uint32 bufflen)
{
	int iRet = -1;
	
	buff_struct->read_P = 0;
	buff_struct->write_P = 0;
	buff_struct->count = 0;
	buff_struct->buff = malloc(bufflen);
	if(buff_struct->buff == NULL)
	{
		buff_struct->bufflen = 0;
		return iRet;
	}
	buff_struct->bufflen = bufflen;
	memset(buff_struct->buff, 0, buff_struct->bufflen);
	iRet = 0;
	return iRet;
}

/*
*Function:		CBuffGetWrittenLen
*Description:	读取buff已写入的长度
*Input:			*buff_struct:BUFF_STRUCT指针
*Output:		NULL
*Hardware:
*Return:		读到的已写入长度
*Others:		
*/
int CBuffGetWrittenLen(BUFF_STRUCT *buff_struct)
{
	return buff_struct->count;
}

/*
*Function:		CBuffClose
*Description:	关闭BUFF_STRUCT结构体并释放资源
*Input:			*buff_struct:BUFF_STRUCT指针
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:		
*/
void CBuffClose(BUFF_STRUCT *buff_struct)
{
	
	if(buff_struct->buff != NULL)
	{
		CBuffFormat(buff_struct);
		free(buff_struct->buff);
	}
    else
    {
    	sysLOG(COMM_LOG_LEVEL_1, "<WARN> CBuffClose is NULL\r\n");
    }
}


/*
*Function:		CBuffRead
*Description:	读buf中的内容，如果需要读取的长度大于缓存长度，则只读取缓存长度
*Input:			len:需要读取的长度；*buff_struct:BUFF_STRUCT指针;
*Output:		*data:读取数据指针
*Hardware:
*Return:		>=0-实际读取到的字节数；<0-读取失败
*Others:		
*/
int32 CBuffRead(BUFF_STRUCT *buff_struct, int8 *data, uint32 len)
{
	int32 iRet = -1;
	int32 readlen;
	int8 *rP = NULL;
	if(len > buff_struct->bufflen)
		return -2;
	
	rP = malloc(len+1);
	if(rP == NULL)
	{		
		return iRet;
	}
	memset(rP, 0, len+1);

	while(g_iBuffMutex)
	{
    	sysLOG(COMM_LOG_LEVEL_5, "CBuffRead mutex:%d \r\n",g_iBuffMutex);		
		sysDelayMs(1);
	}
	
	if(buff_struct->count >= len)//以缓存的数据长度够用
	{
		readlen = len;
		if((buff_struct->bufflen - buff_struct->read_P) >= readlen)//
		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, readlen);
			memset(buff_struct->buff+buff_struct->read_P, 0, readlen);
			buff_struct->read_P += readlen;
			buff_struct->count -= readlen;
		}
		else
		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, buff_struct->bufflen-buff_struct->read_P);
			memcpy(rP+buff_struct->bufflen-buff_struct->read_P, buff_struct->buff, readlen-(buff_struct->bufflen-buff_struct->read_P));
			memset(buff_struct->buff+buff_struct->read_P, 0, buff_struct->bufflen-buff_struct->read_P);
			memset(buff_struct->buff, 0, readlen-(buff_struct->bufflen-buff_struct->read_P));
			buff_struct->read_P = readlen-(buff_struct->bufflen-buff_struct->read_P);
			buff_struct->count -= readlen;
		}
		iRet = readlen;
	}
	else//已缓存的数据长度不够用
	{
		readlen = buff_struct->count;
		if((buff_struct->bufflen - buff_struct->read_P) >= readlen)
		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, readlen);
			memset(buff_struct->buff+buff_struct->read_P, 0, readlen);
			buff_struct->read_P += readlen;
			buff_struct->count =0;
		}
		else

		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, buff_struct->bufflen-buff_struct->read_P);
			memcpy(rP+buff_struct->bufflen-buff_struct->read_P, buff_struct->buff, readlen-(buff_struct->bufflen-buff_struct->read_P));
			memset(buff_struct->buff+buff_struct->read_P, 0, buff_struct->bufflen-buff_struct->read_P);
			memset(buff_struct->buff, 0, readlen-(buff_struct->bufflen-buff_struct->read_P));
			buff_struct->read_P = readlen-(buff_struct->bufflen-buff_struct->read_P);
			buff_struct->count = 0;
		}
		iRet = readlen;
	}
	memcpy(data, rP, iRet);
	free(rP);
	
	return iRet;
}


/*
*Function:		CBuffWrite
*Description:	写数据到buf中，如写入的数据长度大于剩余空间则只写入剩余空间的内容剩下的丢弃
*Input:			*data:写入数据指针；len:写入数据长度；*buff_struct:BUFF_STRUCT指针
*Output:		NULL
*Hardware:
*Return:		>=0-实际写入的数据长度；<0-写入失败
*Others:		
*/
int32 CBuffWrite(BUFF_STRUCT *buff_struct, int8 *data, uint32 len)
{
	int32 iRet = -1;
	int32 writelen;

	g_iBuffMutex = 1;
	
	if((buff_struct->bufflen - buff_struct->count) >= len)//空间够用
	{
		writelen = len;
		if((buff_struct->bufflen - buff_struct->write_P) >= writelen)
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, writelen);
			buff_struct->count += writelen;
			buff_struct->write_P += writelen;
		}
		else
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, (buff_struct->bufflen - buff_struct->write_P));
			memcpy(buff_struct->buff, data+(buff_struct->bufflen - buff_struct->write_P), (writelen - (buff_struct->bufflen - buff_struct->write_P)));
			buff_struct->count += writelen;
			buff_struct->write_P = writelen - (buff_struct->bufflen - buff_struct->write_P);
		}
		iRet = writelen;
	}
	else//空间不够用，则只copy剩余空间的内容
	{
		writelen = buff_struct->bufflen - buff_struct->count;
		if((buff_struct->bufflen - buff_struct->write_P) >= writelen)
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, writelen);
			buff_struct->count += writelen;
			buff_struct->write_P += writelen;
		}
		else
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, (buff_struct->bufflen - buff_struct->write_P));
			memcpy(buff_struct->buff, data+(buff_struct->bufflen - buff_struct->write_P), (writelen - (buff_struct->bufflen - buff_struct->write_P)));
			buff_struct->count += writelen;
			buff_struct->write_P = writelen - (buff_struct->bufflen - buff_struct->write_P);
		}
		iRet = writelen;
	}
	g_iBuffMutex = 0;

	return iRet;
}


/*
*Function:		CBuffFiFoWrite
*Description:	写数据到buf中，如写入的数据长度大于剩余空间则将之前的覆盖掉，循环写入
*Input:			*data:写入数据指针；len:写入数据长度；*buff_struct:BUFF_STRUCT指针
*Output:		NULL
*Hardware:
*Return:		>=0-实际写入的数据长度；<0-写入失败
*Others:		
*/
int32 CBuffFiFoWrite(BUFF_STRUCT *buff_struct, int8 *data, uint32 len)
{
	int32 iRet = -1;
	int32 writelen;
	if((buff_struct->bufflen - buff_struct->count) >= len)//空间够用
	{
		writelen = len;
		if((buff_struct->bufflen - buff_struct->write_P) >= writelen)
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, writelen);
			buff_struct->count += writelen;
			buff_struct->write_P += writelen;
		}
		else
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, (buff_struct->bufflen - buff_struct->write_P));
			memcpy(buff_struct->buff, data+(buff_struct->bufflen - buff_struct->write_P), (writelen - (buff_struct->bufflen - buff_struct->write_P)));
			buff_struct->count += writelen;
			buff_struct->write_P = writelen - (buff_struct->bufflen - buff_struct->write_P);
		}
		iRet = writelen;
	}
	else//空间不够用，则将之前的覆盖掉，循环写入
	{
		writelen = len;
		if((buff_struct->bufflen - buff_struct->write_P) >= writelen)
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, writelen);
			buff_struct->count = buff_struct->bufflen;
			buff_struct->write_P += writelen;
			buff_struct->read_P = buff_struct->write_P;
		}
		else
		{
			memcpy(buff_struct->buff+buff_struct->write_P, data, (buff_struct->bufflen - buff_struct->write_P));
			memcpy(buff_struct->buff, data+(buff_struct->bufflen - buff_struct->write_P), (writelen - (buff_struct->bufflen - buff_struct->write_P)));
			buff_struct->count = buff_struct->bufflen;
			buff_struct->write_P = writelen - (buff_struct->bufflen - buff_struct->write_P);
			buff_struct->read_P = buff_struct->write_P;
		}
		iRet = writelen;
	}

	return iRet;
}


/*
*Function:		CBuffReadStr
*Description:	从一个buff读内容到另一个buff中,如果原地址buff的长度不够则只读取原地址已缓存的长度；
*				如果目的buff的空间不够，则只读取目的buff剩余空间长度的内容
*Input:			*buff_struct_sor:源地址，*buff_struct_des:目的地址，len:读取长度
*Output:		NULL
*Hardware:
*Return:		<0:失败，>0:实际读取的长度
*Others:		
*/
int CBuffReadStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len)
{
	int iRet;
	int *rP = NULL;
	
	rP = malloc(len+1);
	if(rP == NULL)
	{
		return -1;
	}
	memset(rP, 0, len+1);
	iRet = CBuffCopy(buff_struct_sor, rP, len);//1,先复制出来，防止目的地址存不了len长度的内容
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	iRet = CBuffWrite(buff_struct_des, rP, iRet);//2,写入数据到目的地址
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	free(rP);
	CBuffClean(buff_struct_sor, iRet);//3,擦除已经成功写入到目的地址长度的数据
	return iRet;
}


/*
*Function:		CBuffFiFoReadStr
*Description:	从一个buff读内容到另一个buff中,如果原地址buff的长度不够则只读取原地址已缓存的长度；
*				如果目的buff的空间不够，则将之前的覆盖掉，循环写入
*Input:			*buff_struct_sor:源地址，*buff_struct_des:目的地址，len:读取长度
*Output:		NULL
*Hardware:
*Return:		<0:失败，>0:实际读取的长度
*Others:		
*/
int CBuffFiFoReadStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len)
{
	int iRet;
	int *rP = NULL;
	
	rP = malloc(len+1);
	if(rP == NULL)
	{
		return -1;
	}
	memset(rP, 0, len+1);
	iRet = CBuffRead(buff_struct_sor, rP, len);//1,先全部读出来即可
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	iRet = CBuffFiFoWrite(buff_struct_des, rP, iRet);//2,写入数据到目的地址
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	free(rP);
	return iRet;
}


/*
*Function:		CBuffClean
*Description:	清除buff中长度为len的内容
*Input:			*buff_struct:BUFF_STRUCT指针, len:需要清除的长度
*Output:		NULL
*Hardware:
*Return:		<0:失败；>=0:成功，实际清除的长度
*Others:		
*/
int CBuffClean(BUFF_STRUCT *buff_struct, uint32 len)
{
	int32 iRet = -1;
	int32 cleanlen;
	
	if(len == 0)
		return 0;
	
	if(buff_struct->count >= len)//以缓存的数据长度够用
	{
		cleanlen = len;
		if((buff_struct->bufflen - buff_struct->read_P) >= cleanlen)//
		{
			memset(buff_struct->buff+buff_struct->read_P, 0, cleanlen);
			buff_struct->read_P += cleanlen;
			buff_struct->count -= cleanlen;
		}
		else
		{
			memset(buff_struct->buff+buff_struct->read_P, 0, buff_struct->bufflen-buff_struct->read_P);
			memset(buff_struct->buff, 0, cleanlen-(buff_struct->bufflen-buff_struct->read_P));
			buff_struct->read_P = cleanlen-(buff_struct->bufflen-buff_struct->read_P);
			buff_struct->count -= cleanlen;
		}
		iRet = cleanlen;
	}
	else//已缓存的数据长度不够用
	{
		cleanlen = buff_struct->count;
		if((buff_struct->bufflen - buff_struct->read_P) >= cleanlen)
		{
			buff_struct->read_P += cleanlen;
			buff_struct->count =0;
		}
		else
		{
			buff_struct->read_P = cleanlen-(buff_struct->bufflen-buff_struct->read_P);
			buff_struct->count = 0;
		}
		iRet = cleanlen;
	}
	
	return iRet;
}


/*
*Function:		CBuffCopy
*Description:	复制buf中的内容，复制完毕后原路径的缓存不清除，如果需要读取的长度大于缓存长度，则只读取缓存长度
*Input:			len:需要读取的长度；*buff_struct:BUFF_STRUCT指针;
*Output:		*data:读取数据指针
*Hardware:
*Return:		>=0-实际读取到的字节数；<0-读取失败
*Others:		
*/
int32 CBuffCopy(BUFF_STRUCT *buff_struct, int8 *data, uint32 len)
{
	int32 iRet = -1;
	int32 readlen;
	int8 *rP = NULL;
	
	rP = malloc(len+1);
	if(rP == NULL)
	{		
		return iRet;
	}
	memset(rP, 0, len+1);

	if(buff_struct->count >= len)//以缓存的数据长度够用
	{
		readlen = len;
		if((buff_struct->bufflen - buff_struct->read_P) >= readlen)//
		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, readlen);
		}
		else
		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, buff_struct->bufflen-buff_struct->read_P);
			memcpy(rP+buff_struct->bufflen-buff_struct->read_P, buff_struct->buff, readlen-(buff_struct->bufflen-buff_struct->read_P));
		}
		iRet = readlen;
	}
	else//已缓存的数据长度不够用
	{
		readlen = buff_struct->count;
		if((buff_struct->bufflen - buff_struct->read_P) >= readlen)
		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, readlen);
		}
		else

		{
			memcpy(rP, buff_struct->buff+buff_struct->read_P, buff_struct->bufflen-buff_struct->read_P);
			memcpy(rP+buff_struct->bufflen-buff_struct->read_P, buff_struct->buff, readlen-(buff_struct->bufflen-buff_struct->read_P));
		}
		iRet = readlen;
	}
	memcpy(data, rP, iRet);
	free(rP);
	
	return iRet;
}


/*
*Function:		CBuffCopyStr
*Description:	从一个buff复制内容到另一个buff中，如果原地址buff的长度不够则只读取原地址已缓存的长度；
*				如果目的buff的空间不够，则只读取目的buff剩余空间长度的内容
*Input:			*buff_struct_sor:源地址，*buff_struct_des:目的地址，len:复制长度
*Output:		NULL
*Hardware:
*Return:		<0:失败，>0:实际复制的长度
*Others:		
*/
int CBuffCopyStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len)
{
	int iRet;
	int *rP = NULL;
	
	rP = malloc(len+1);
	if(rP == NULL)
	{
		return -1;
	}
	memset(rP, 0, len+1);
	iRet = CBuffCopy(buff_struct_sor, rP, len);
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	iRet = CBuffWrite(buff_struct_des, rP, iRet);
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	free(rP);
	return iRet;
}


/*
*Function:		CBuffFiFoCopyStr
*Description:	从一个buff复制内容到另一个buff中，如果原地址buff的长度不够则只读取原地址已缓存的长度；
*				如果目的buff的空间不够，则则将之前的覆盖掉，循环写入
*Input:			*buff_struct_sor:源地址，*buff_struct_des:目的地址，len:复制长度
*Output:		NULL
*Hardware:
*Return:		<0:失败，>0:实际复制的长度
*Others:		
*/
int CBuffFiFoCopyStr(BUFF_STRUCT *buff_struct_des, BUFF_STRUCT *buff_struct_sor, uint32 len)
{
	int iRet;
	int *rP = NULL;
	
	rP = malloc(len+1);
	if(rP == NULL)
	{
		return -1;
	}
	memset(rP, 0, len+1);
	iRet = CBuffCopy(buff_struct_sor, rP, len);
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	iRet = CBuffFiFoWrite(buff_struct_des, rP, iRet);
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	free(rP);
	return iRet;
}


/*
*Function:		GetNumFromAscii
*Description:	从ASCII中获取数值，可为负数
*Input:			*buff:Ascii地址，offset:Ascii在buff中的偏移量; *endchar:数值结束字符; lenth:数值长度; 
*    			mode:读取数值结束的模式，0-以识别到结束字符为结束标志，1-以识别的长度lenth为结束标志
*Output:		NULL
*Hardware:
*Return:		获取的数值，可为负数
*Others:		
*/
int GetNumFromAscii(char *buff, uint32 offset, char *endchar, unsigned char lenth, unsigned char mode)
{
	int iRet = -1;
	int offsettmp = 0;
	int lenthtmp = 0;
	uint8 i = 0, j = 0, k = 0;
	int valuetmp = 0;
	char valuebufftmp[128];
	char * pcTemp = NULL;

	memset(valuebufftmp, 0, sizeof(valuebufftmp));

	if(mode == 0)//按结束字符查找
	{
		if(*(buff+offset) == *endchar)//end
		{
			return 0;//没有数据
		}
		if(*(buff+offset) == 0x2D)
		{
			offsettmp = offset+1;
		}
		else
		{
			offsettmp = offset;
		}
		while(1)
		{
			if(*(buff+offsettmp+i) == *endchar)//end
			{
				break;
			}
			if(*(buff+offsettmp+i)<0x30 || *(buff+offsettmp+i)>0x39)
			{
				break;
			}
			valuebufftmp[i] = *(buff+offsettmp+i)-0x30;
			sysLOG(COMM_LOG_LEVEL_4, "GetNumFromAscii, i:%d, valuebufftmp[i]:%d\r\n", i, valuebufftmp[i]);
			i++;
		}
	}
	else if(mode == 1)
	{
		if(lenth > 0)
		{
			if(*(buff+offset) == 0x2D)
			{
				offsettmp = offset+1;
				lenthtmp = lenth-1;
			}
			else
			{
				offsettmp = offset;
				lenthtmp = lenth;
			}

			for(i=0; i<lenthtmp; i++)
			{
				if(*(buff+offsettmp+i)<0x30 || *(buff+offsettmp+i)>0x39)
				{
					break;
				}
				valuebufftmp[i] = *(buff+offsettmp+i)-0x30;
				sysLOG(COMM_LOG_LEVEL_4, "GetNumFromAscii, i:%d, valuebufftmp[i]:%d\r\n", i, valuebufftmp[i]);
			}
		}
		else
		{
			return 0;//没有数据
		}
	}
	else if(3 == mode)
	{
		pcTemp = strstr(buff+offset, endchar);
		if(pcTemp)
		{
			memset(valuebufftmp, 0, sizeof(valuebufftmp));
			lenthtmp = pcTemp - (buff+offset);
			if(lenthtmp % 2)
			{
				valuebufftmp[0] =0x30;
				memcpy(valuebufftmp+1, buff+offset, lenthtmp);
			}
			else
			{
				memcpy(valuebufftmp, buff+offset, lenthtmp);
			}
			lenthtmp = StrToHex((char*)valuebufftmp, (char*)valuebufftmp);
			BuffTurnOver((char*)valuebufftmp, lenthtmp);
			memcpy(&valuetmp, valuebufftmp, lenthtmp);
		}
		else
		{
			valuetmp = 0;
		}
		goto RET_END;
	}
	if(i > 0)
	{
		valuetmp=valuebufftmp[i-1];
		for(k=i-1; k>0; k--)
		{
			j++;
			valuetmp += valuebufftmp[k-1]*pow(10,j); 
		}

		if(offsettmp == (offset+1))
		{
			valuetmp = 0-valuetmp;
		}
	}
RET_END:
	sysLOG(COMM_LOG_LEVEL_1, "GetNumFromAscii, valuetmp:%d, valuetmp:0x%x\r\n", valuetmp, valuetmp);

	return valuetmp;
 
}



/*
*Function:		Hysteresis
*Description:	迟滞接口,
*				mode=0时,	当activityV与lastV的差值相比，下降差值>deviationDown或者上升差值>deviationUp，则返回值为activityV，否则返回值为lastV;
*				mode=1时	,	当activityV上升并越过临界值criticalV，与criticalV相比，差值>deviationUp时，返回值为activityV，否则返回值为lastV;
*							当activityV下降并越过临界值criticalV，与criticalV相比，差值>deviationDown时，返回值为activityV，否则返回值为lastV;
*Input:			activityV:当前值; lastV:上一次的值; deviationDown:向下减小的差值; deviationUp:向上增加的差值
*				criticalV:临界值; mode:0-所有值都要做迟滞计算; 1-只在临界值上下做迟滞计算
*Output:		NULL
*Hardware:
*Return:		返回迟滞后的值
*Others:
*/
float Hysteresis(float activityV, float lastV, float deviationDown, float deviationUp, float criticalV, uint8 mode)
{
	float lastvtemp;
	float batvtemp;

	lastvtemp = lastV;

	if(mode == 0)
	{
		
		if(activityV <= (lastvtemp - deviationDown)) batvtemp = activityV;
		else if(activityV >= (lastvtemp + deviationUp)) batvtemp = activityV;
		else batvtemp = lastV;
		
	}
	else if(mode == 1)
	{
		if(activityV > criticalV && lastV < criticalV)//判断上升是否越过临界值
		{
		
			if(activityV >= (criticalV + deviationUp)) batvtemp = activityV;
			else 
			{
				if(activityV > (lastV+deviationUp*2))//当这一次数据大于上一次数据+变化量*2，则更新值为activityV
				{
					batvtemp = activityV;
				}
				else
				{
					batvtemp = lastV;
				}
			}
		}
		else if(activityV < criticalV && lastV > criticalV)//判断下降是否越过临界值
		{
			if(activityV <= (criticalV - deviationDown)) batvtemp = activityV;
			else
			{
				if(activityV < (lastV - deviationDown*2))//当这一次数据小于上一次数据-变化量*2，则更新值为activityV
				{
					batvtemp = activityV;
				}
				else
				{
					batvtemp = lastV;
				}
			}
		}
		else//不在临界值附近的是多少就返回多少即可
		{
			batvtemp = activityV;
		}
		
	}
	else
	{
		batvtemp = activityV;
	}

	return batvtemp;
}




void Delayss(int ss)
{
	int cc = 0;
	for(int i = 0; i < ss; i++)
	{
		for(int j = 0; j < ss; j++)
		{
			cc++;
		}
	}
}


/*****************************TEST*****************************/

#if MAINTEST_FLAG
void testBCD(void)
{
	int iRet;
	int ret;
	uint8 bcdtmp[6] = {0x19, 0x03, 0x12, 0x23, 0x59, 0x12};
	uint8 dectmp[7];
	memset(dectmp, 0, sizeof(dectmp));
	for(uint8 i=0; i<6; i++)
	{
		dectmp[i]= BcdToDec(&bcdtmp[i], 1);
		sysLOG(1, "bcdtmp[i]:%d, dectmp[i]:%d\r\n", bcdtmp[i], dectmp[i]);
			
	}
	memset(bcdtmp, 0, sizeof(bcdtmp));
	for(uint8 j=0; j<6; j++)
	{
		sysLOG(1, "bcdtmp[j]:%d, dectmp[j]:%d\r\n", bcdtmp[j], dectmp[j]);
		DecToBcd(dectmp[j], &bcdtmp[j], 1);
		sysLOG(1, "bcdtmp[j]:%d, dectmp[j]:%d\r\n", bcdtmp[j], dectmp[j]);
	}
	
	memset(dectmp, 0, sizeof(dectmp));
	iRet = hal_sysGetTime(dectmp);
	sysLOG(1, "hal_sysGetTime, iRet=%d, %d,%d,%d,%d,%d,%d,%d\r\n", iRet, dectmp[0], dectmp[1], dectmp[2], dectmp[3], dectmp[4], dectmp[5], dectmp[6]);
	iRet = hal_sysSetTime(bcdtmp);
	sysLOG(1, "hal_sysSetTime, iRet=%d\r\n", iRet);
	memset(dectmp, 0, sizeof(dectmp));
	iRet = hal_sysGetTime(dectmp);
	sysLOG(1, "hal_sysGetTime, iRet=%d, %d,%d,%d,%d,%d,%d,%d\r\n", iRet, dectmp[0], dectmp[1], dectmp[2], dectmp[3], dectmp[4], dectmp[5], dectmp[6]);

}

#endif

