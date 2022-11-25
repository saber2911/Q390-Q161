#include "comm.h"

int enc_get_utf8_size(const char *pInput)
 {
     unsigned char c = *((unsigned char *)pInput);
     if(c< 0x80) return 0;                // 0xxxxxxx 返回0
     if(c>=0x80 && c<0xC0) return -1;     // 10xxxxxx 返回-1
     if(c>=0xC0 && c<0xE0) return 2;      // 110xxxxx 返回2
     if(c>=0xE0 && c<0xF0) return 3;      // 1110xxxx 返回3
     if(c>=0xF0 && c<0xF8) return 4;      // 11110xxx 返回4
     if(c>=0xF8 && c<0xFC) return 5;      // 111110xx 返回5
     if(c>=0xFC) return 6;                // 1111110x 返回6
 }
                      

                     
 // #c---
 /*****************************************************************************
  * 将一个字符的UTF8编码转换成Unicode(UCS-2和UCS-4)编码.
  *
  * 参数:
  *    pInput      指向输入缓冲区, 以UTF-8编码
  *    Unic        指向输出缓冲区, 其保存的数据即是Unicode编码值,
  *                类型为unsigned long .
  *
  * 返回值:
  *    成功则返回该字符的UTF8编码所占用的字节数; 失败则返回0.
  *
  * 注意:
  *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
  *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
  *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
  ****************************************************************************/


int enc_utf8_to_unicode_one(IN const char* pInput, OUT unsigned char *Unic)
{

    // b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...
    char b1, b2, b3, b4, b5, b6;
 
    *Unic = 0x0; // 把 *Unic 初始化为全零
    int utfbytes = enc_get_utf8_size(pInput);
    unsigned char *pOutput = (unsigned char *) Unic;
 
    switch ( utfbytes )
    {
        case 0:
            *pOutput     = *pInput;
            utfbytes    += 1;         
            break;
        case 2:
            b1 = *pInput;
            b2 = *(pInput + 1);
            if ( (b2 & 0xE0) != 0x80 )
                return 0;
            *(pOutput+1)     = (b1 << 6) + (b2 & 0x3F);
            *(pOutput) = (b1 >> 2) & 0x07;
            break;
        case 3:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )
                return 0;
            *(pOutput+1)  = (b2 << 6) + (b3 & 0x3F);
            *(pOutput) = (b1 << 4) + ((b2 >> 2) & 0x0F);
            break;
        case 4:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) )
                return 0;
            *(pOutput+2)   = (b3 << 6) + (b4 & 0x3F);
            *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
            *(pOutput) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);
            break;
        case 5:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )
                return 0;
            *(pOutput+3)  = (b4 << 6) + (b5 & 0x3F);
            *(pOutput+2) = (b3 << 4) + ((b4 >> 2) & 0x0F);
            *(pOutput+1) = (b2 << 2) + ((b3 >> 4) & 0x03);
            *(pOutput) = (b1 << 6);
            break;
        case 6:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            b6 = *(pInput + 5);
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)
                    || ((b6 & 0xC0) != 0x80) )
                return 0;
            *(pOutput+3) = (b5 << 6) + (b6 & 0x3F);
            *(pOutput+2) = (b5 << 4) + ((b6 >> 2) & 0x0F);
            *(pOutput+1) = (b3 << 2) + ((b4 >> 4) & 0x03);
            *(pOutput+0) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            break;
        default:
            return 0;
            break;
    }
 
    return utfbytes;
}

unsigned int strUTF8tostrUnicode(char *str, unsigned int len, OUT unsigned char *ucStr)
{
    int i;
    unsigned char recBytes;
    unsigned char *p = ucStr;
    unsigned char *datap = str;
    unsigned char count = 0;
    
    if(datap[0] == '\0')
        return count;  
    
    for(i=0;i<len;i++)
    {                 
        recBytes = enc_utf8_to_unicode_one(datap,p);      
        if(recBytes == 1)
        {          
            p[1] = p[0];
            p[0] = 0;
            p+=2;               
        }
        else
        {
            p+=2;
        }
        
        datap+=recBytes;
              
        count+=2;
        if(count > MAX_UTF8_STRING_LENTH)
            return MAX_UTF8_STRING_LENTH;
        
        if(datap[0] == '\0')
           return count; 
        
    }

    return count;
}

 
 //
 /*****************************************************************************
  * 将一个UTF8编码格式的字符串转换成MG22660模块支持的Unicode编码格式.
  *
  * 参数:
  *    pInput      指向输入缓冲区, 以UTF-8编码
  *    ulen        传入字符串的长度
  *    Unic        指向输出缓冲区, 其保存的数据即是Unicode编码值,
  *                类型为unsigned long .
  *
  * 返回值:
  *    成功则返回转换的字节数; 失败则返回0.
  *
  * 注意:
  *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
  *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
  *        根据模块支持情况，当前转码的数据按照大端方式输出。(低地址存高位)
  ****************************************************************************/

int formatUtf8StingtoTTScode(IN char *sInput, IN unsigned int ulen, OUT unsigned char *ucStr)
{
    unsigned int lenth;
    unsigned char buf[MAX_UTF8_STRING_LENTH];
    memset(buf,0,sizeof(buf));
    lenth = strUTF8tostrUnicode(sInput,ulen,buf);
    HexToStr(buf,lenth,ucStr);
    lenth = lenth * 2;
    return lenth;
}

void demo()
{
	int8 ret;
	unsigned char temp[256];
	memset(temp, 0x00, sizeof(temp));
	int strlenth = formatUtf8StingtoTTScode("微信收款1.23元",CZ_strlen("微信收款1.23元"),temp);
	sysLOG(2, "unicode %s\r\n",temp);
	//ret = CZ_TtsStart(temp, strlenth, CODE_FORMAT_UNICODE); 
}