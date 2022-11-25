
/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_tts.c
** Description:		TTS播报相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/


#include "comm.h"

#define	TTS_VOLUME_FEATURE	4


struct _TTS_S g_stTTS_s;
uint32 g_ui32SemTTSPlayHandle;//tts播报和清队列之间的信号量,


/*
*Function:		hal_ttsAmpGpioInit
*Description:	初始化外部功放IO
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ttsAmpGpioInit(void)
{
	fibo_gpio_mode_set(AMP_CTRL_GPIO, GpioFunction0);
	fibo_gpio_cfg(AMP_CTRL_GPIO, GpioCfgOut);
	fibo_gpio_set(AMP_CTRL_GPIO, false);
}



/*
*Function:		hal_ttsInit
*Description:	TTS初始化
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ttsInit(void)
{
	int iRet;
	//hal_ttsAmpGpioInit();
	//fibo_external_PA_enable_level(AMP_CTRL_GPIO);
	fibo_external_PA_enable_fun(2,AMP_CTRL_GPIO);
	iRet = fibo_get_volume(TTS_VOLUME_FEATURE);
	if(iRet >= 0)
		g_stTTS_s.tts_volume = iRet;
	
	
	g_stTTS_s.tts_enable = 1;
	g_stTTS_s.tts_orderin = g_stTTS_s.tts_orderout = 0;
	g_stTTS_s.tts_ordercnt = 0;
	g_stTTS_s.tts_status = 0;
	fibo_tts_voice_speed(3000);
	fibo_tts_voice_pitch(0);
	g_ui32SemTTSPlayHandle = fibo_sem_new(1);


}

/*
*Function:		hal_ttsAmpCtl
*Description:	外部功放使能控制接口
*Input:			AMP_value：TRUE-使能;FALSE-失能
*Output:		NULL
*Hardware:
*Return:		返回使能结果
*Others:
*/
uint8 hal_ttsAmpCtl(uint8 AMP_value)
{
#if 0
	uint8 ret;
	fibo_gpio_get(AMP_CTRL_GPIO, &ret);
	if(ret != AMP_value)
	{
		fibo_gpio_set(AMP_CTRL_GPIO, AMP_value);
		fibo_gpio_get(AMP_CTRL_GPIO, &ret);
	}
	
	sysLOG(TTS_LOG_LEVEL_2, "hal_ttsAmpCtl:%d\r\n", ret);
	return ret;
#endif
}


/*
*Function:		hal_ttsTransVolume
*Description:	音量等级转换
*Input:			volume:支持1-5级
*Output:		NULL
*Hardware:
*Return:		返回转换后调用厂家接口所需要的值
*Others:
*/
static uint8 hal_ttsTransVolume(uint8 volume)
{
	switch(volume)
	{
		case 1:
			return 2;
		break;
		case 2:
			return 4;
		break;
		case 3:
			return 5;
		break;
		case 4:
			return 6;
		break;
		case 5:
			return 7;
		break;
		default:
			return g_stTTS_s.tts_volume;
		break;
	}
}


/*
*Function:		hal_ttsSetVolume
*Description:	设置TTS音量大小：1-5
*Input:			volume:音量值
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ttsSetVolume(uint8 volume)
{
	uint8 volumetmp;
	
	volumetmp = hal_ttsTransVolume(volume);
	g_stTTS_s.tts_volume = volumetmp;
	if(fibo_tts_is_playing() == TRUE)
	{	
		fibo_set_volume(AUDIO_PLAY_VOLUME, volumetmp);	
	}
	else
	{
		fibo_set_audio_volume(TTS_VOLUME_FEATURE, volumetmp);		
	}
	
	sysLOG(TTS_LOG_LEVEL_1, "volume:%d, g_stTTS_s.tts_volume:%d\r\n", volume, g_stTTS_s.tts_volume);
}


unsigned char one2nine_gb2312[32] = {
	0xC1, 0xE3, 0xD2, 0xBB, 0xB6, 0xFE, 0xC8, 0xFD, 0xCB, 0xC4, 0xCE, 0xE5, 0xC1, 0xF9, 0xC6, 0xDF, 
	0xB0, 0xCB, 0xBE, 0xC5};
unsigned char one2nine_utf8[32] = {
	0xE9, 0x9B, 0xB6, 0xE4, 0xB8, 0x80, 0xE4, 0xBA, 0x8C, 0xE4, 0xB8, 0x89, 0xE5, 0x9B, 0x9B, 0xE4, 
	0xBA, 0x94, 0xE5, 0x85, 0xAD, 0xE4, 0xB8, 0x83, 0xE5, 0x85, 0xAB, 0xE4, 0xB9, 0x9D};
unsigned char one2nine_ucs2little[32] = {
	0xF6, 0x96, 0x00, 0x4E, 0x8C, 0x4E, 0x09, 0x4E, 0xDB, 0x56, 0x94, 0x4E, 0x6D, 0x51, 0x03, 0x4E, 
	0x6B, 0x51, 0x5D, 0x4E};


/*
*Function:		hal_ttsArabicnumber2Sinogram
*Description:	ASCII格式的阿拉伯数字转中文
*Input:			num:阿拉伯数字，encode:输出的中文编码格式，0:UTF-8; 1:GB2312; 2:UNICODE little endian
*Output:		*data:输出转换后的中文字符地址
*Hardware:
*Return:		>=0-succ,输出转换后的字节个数;other-failed
*Others:
*/
int hal_ttsArabicnumber2Sinogram(uint8 num, uint8 *data, int8 encode)
{
	int iRet = -1;
	uint8 numtmp = 0;
	if(num>=0x30 && num<=0x39)
	{
		numtmp = num - 0x30;
		switch(encode)
		{
			case 0://utf8
				memcpy(data, &one2nine_utf8[numtmp*3], 3);
				iRet = 3;			
			break;
			case 1://gb2312
				memcpy(data, &one2nine_gb2312[numtmp*2], 2);
				iRet = 2;
			break;
			case 2://unicode
				memcpy(data, &one2nine_ucs2little[numtmp*2], 2);
				iRet = 2;
			break;
			default://others

			break;
		}
	}
	sysLOG(TTS_LOG_LEVEL_5, "iRet=%d, num=%d, data:%x,%x,%x\r\n", iRet, num, *data, *(data+1), *(data+2));

	return iRet;
}
	

/*
*Function:		hal_ttsPlay
*Description:	TTS播报
*Input:			*tts_content:tts播报内容指针; readmode:读数字方式，0-自动判断；1-按号码方式；2-按数值方式；
*				encode:播报内容编码格式，0:UTF-8; 1:GB2312; 2:UNICODE
*Output:		NULL
*Hardware:
*Return:		-1:失败；0:成功
*Others:
*/
int32 hal_ttsPlay(int8 *tts_content, uint8 readmode, int8 encode)
{
	int iRet = -1;
	int8 *rP = NULL;
	int8 *rP1 = NULL;
	int lentmp = 0;
	int8 encodetmp = 0;
	uint8 sinogramtmp[8];

	if(encode == 2)
	{
		while(1)
		{
			if((*(tts_content+lentmp) == 0) && (*(tts_content+lentmp+1) == 0))
			{
				break;
			}
			lentmp += 2;
		}
	}
	else
	{
		lentmp = strlen(tts_content);
	}

	rP = malloc(lentmp*3);
	if(rP == NULL)
	{
		return -1;
	}
	memset(rP, 0, lentmp*3);


	if(encode == 2)//unicode only support little endian
	{
		if((uint8)*tts_content == 0xFE && (uint8)*(tts_content+1) == 0xFF)//big endian->little endian
		{
			int8 aa;
			for(int i=0; i<lentmp/2; i++)
			{
				aa = *(tts_content+i*2+1);
				*(tts_content+i*2+1) = *(tts_content+i*2);
				*(tts_content+i*2) = aa;
			}
		}
		
	}
	
	if(readmode == 1)//按号码方式读取
	{
		if(encode == 0)//utf8
		{
			int j = 0;
			for(int i=0; i<lentmp; i++)
			{
				if((uint8)*(tts_content+i)>=0x30 && (uint8)*(tts_content+i)<=0x39)
				{
					if((i != 0 && (uint8)*(tts_content+i-1) != 0x2E) && ((uint8)*(tts_content+i-1)<0x30 || (uint8)*(tts_content+i-1)>0x39))
					{
						*(rP+i+j) = 0x3A;//用冒号将文本和数字隔开
						j += 1;
					}

					memset(sinogramtmp, 0, sizeof(sinogramtmp));
					iRet = hal_ttsArabicnumber2Sinogram(*(tts_content+i), sinogramtmp, encode);

					if((uint8)*(tts_content+i+1)==0x2E &&((uint8)*(tts_content+i+2)>=0x30 && (uint8)*(tts_content+i+2)<=0x39))
					{
						
						if(iRet == 3)
						{
							memcpy(rP+i+j, sinogramtmp, iRet);
							i += 1;
							j += 2;
						}
						else
						{
							*(rP+i+j) = *(tts_content+i);
							i += 1;
						}
						*(rP+i+j) = 0xE7;
						j += 1;
						*(rP+i+j) = 0x82;
						j += 1;
						*(rP+i+j) = 0xB9;
						
					}
					else
					{
						if(iRet == 3)
						{
							memcpy(rP+i+j, sinogramtmp, iRet);
							j += 2;
						}
						else
						{
							*(rP+i+j) = *(tts_content+i);
							j += 1;
							*(rP+i+j) = 0x20;
						}
					}
					
				}
				else
				{
					*(rP+i+j) = *(tts_content+i);
				}
			}
			lentmp += j;
		}
		else if(encode == 1)//GB2312
		{
			int j = 0;
			for(int i=0; i<lentmp; i++)
			{
				if((uint8)*(tts_content+i)>=0x30 && (uint8)*(tts_content+i)<=0x39)
				{
					if((i != 0 && (uint8)*(tts_content+i-1) != 0x2E) && ((uint8)*(tts_content+i-1)<0x30 || (uint8)*(tts_content+i-1)>0x39))
					{
						*(rP+i+j) = 0x3A;//用冒号将文本和数字隔开
						j += 1;
					}

					memset(sinogramtmp, 0, sizeof(sinogramtmp));
					iRet = hal_ttsArabicnumber2Sinogram(*(tts_content+i), sinogramtmp, encode);

					if((uint8)*(tts_content+i+1)==0x2E &&((uint8)*(tts_content+i+2)>=0x30 && (uint8)*(tts_content+i+2)<=0x39))
					{
						if(iRet == 2)
						{
							memcpy(rP+i+j, sinogramtmp, iRet);
							i += 1;
							j += 1;
						}
						else
						{
							*(rP+i+j) = *(tts_content+i);
							i += 1;
						}
						*(rP+i+j) = 0xB5;
						j += 1;
						*(rP+i+j) = 0xE3;
						
					}
					else
					{
						if(iRet == 2)
						{
							memcpy(rP+i+j, sinogramtmp, iRet);
							j += 1;
						}
						else
						{
							*(rP+i+j) = *(tts_content+i);
							j += 1;
							*(rP+i+j) = 0x20;
						}
					}
					
				}
				else
				{
					*(rP+i+j) = *(tts_content+i);
				}
			}
			lentmp += j;
		}
		else if(encode == 2)//unicode
		{
			int j = 0;
			for(int i=0; i<lentmp/2; i++)
			{
			
				if(((uint8)*(tts_content+i*2+1)==0x00) && ((uint8)*(tts_content+i*2)>=0x30 && (uint8)*(tts_content+i*2)<=0x39))
				{
					if((i != 0) && ((uint8)*(tts_content+i*2-2)<0x30 || (uint8)*(tts_content+i*2-2)>0x39))
					{
						if(!((uint8)*(tts_content+i*2-2) == 0x2E && (uint8)*(tts_content+i*2-1) == 0x00))
						{
							*(rP+i*2+j) = 0x3A;//用冒号将文本和数字隔开，dump有点问题暂时关闭
							*(rP+i*2+1+j) = 0x00;
							j += 2;
						}
					}

					memset(sinogramtmp, 0, sizeof(sinogramtmp));
					iRet = hal_ttsArabicnumber2Sinogram(*(tts_content+i*2), sinogramtmp, encode);

					if((((uint8)*(tts_content+i*2+3)==0x00) && ((uint8)*(tts_content+i*2+2)==0x2E)) && \
						(((uint8)*(tts_content+i*2+5)==0x00) && ((uint8)*(tts_content+i*2+4)>=0x30 && (uint8)*(tts_content+i*2+4)<=0x39)))
					{
						if(iRet == 2)
						{
							memcpy(rP+i*2+j, sinogramtmp, iRet);
							i += 1;
						}
						else
						{
							*(rP+i*2+j) = *(tts_content+i*2);
							*(rP+i*2+1+j) = *(tts_content+i*2+1);
							i += 1;
						}
						*(rP+i*2+j) = 0xB9;
						*(rP+i*2+j+1) = 0x70;		
						
					}
					else
					{
						if(iRet == 2)
						{
							memcpy(rP+i*2+j, sinogramtmp, iRet);
						}
						else
						{
							*(rP+i*2+j) = *(tts_content+i*2);
							*(rP+i*2+1+j) = *(tts_content+i*2+1);
							j += 2;
							*(rP+i*2+j) = 0x20;
							*(rP+i*2+j+1) = 0x00;	
						}
					}
				}
				else
				{
					*(rP+i*2+j) = *(tts_content+i*2);
					*(rP+i*2+1+j) = *(tts_content+i*2+1);
				}
				
			}
		
			lentmp += j;
		}
	}
	else
	//{
		//memcpy(rP, tts_content, lentmp);
	//}
	{
		if(encode == 0)//utf8
		{
			int j = 0;
			for(int i=0; i<lentmp; i++)
			{
				if((uint8)*(tts_content+i)>=0x30 && (uint8)*(tts_content+i)<=0x39)
				{
//					if((i != 0 && (uint8)*(tts_content+i-1) != 0x2E) && ((uint8)*(tts_content+i-1)<0x30 || (uint8)*(tts_content+i-1)>0x39))
//					{
//						*(rP+i+j) = 0x3A;//用冒号将文本和数字隔开
//						j += 1;
//					}
					*(rP+i+j) = *(tts_content+i);
					
				}
				else
				{
					*(rP+i+j) = *(tts_content+i);
				}
			}
			lentmp += j;
		}
		else if(encode == 1)//GB2312
		{
			int j = 0;
			for(int i=0; i<lentmp; i++)
			{
				if((uint8)*(tts_content+i)>=0x30 && (uint8)*(tts_content+i)<=0x39)
				{
//					if((i != 0 && (uint8)*(tts_content+i-1) != 0x2E) && ((uint8)*(tts_content+i-1)<0x30 || (uint8)*(tts_content+i-1)>0x39))
//					{
//						*(rP+i+j) = 0x3A;//用冒号将文本和数字隔开
//						j += 1;
//					}
					*(rP+i+j) = *(tts_content+i);
				}
				else
				{
					*(rP+i+j) = *(tts_content+i);
				}
			}
			lentmp += j;
		}
		else if(encode == 2)//unicode
		{
			int j = 0;
			for(int i=0; i<lentmp/2; i++)
			{
			
				if(((uint8)*(tts_content+i*2+1)==0x00) && ((uint8)*(tts_content+i*2)>=0x30 && (uint8)*(tts_content+i*2)<=0x39))
				{
					if((i != 0) && ((uint8)*(tts_content+i*2-2)<0x30 || (uint8)*(tts_content+i*2-2)>0x39))
					{
//						if(!((uint8)*(tts_content+i*2-2) == 0x2E && (uint8)*(tts_content+i*2-1) == 0x00))
//						{
//							*(rP+i*2+j) = 0x3A;//用冒号将文本和数字隔开，dump有点问题暂时关闭
//							*(rP+i*2+1+j) = 0x00;
//							j += 2;
//						}
					}
					*(rP+i*2+j) = *(tts_content+i*2);
					*(rP+i*2+1+j) = *(tts_content+i*2+1);
					
				}
				else
				{
					*(rP+i*2+j) = *(tts_content+i*2);
					*(rP+i*2+1+j) = *(tts_content+i*2+1);
				}
				
			}
		
			lentmp += j;
		}
	}

	rP1 = malloc(lentmp*2+1);
	if(rP1 == NULL)
	{
		free(rP);
		return -2;
	}
	memset(rP1, 0, lentmp*2+1);
	
	HexToStr(rP, lentmp, rP1);
	switch (encode)
	{
		case 0:
			iRet = fibo_tts_start((uint8 *)rP1, CTTS_STRING_ENCODE_TYPE_UTF8);
		break;
		case 1:
			iRet = fibo_tts_start((uint8 *)rP1, CTTS_STRING_ENCODE_TYPE_GB2312);
		break;
		case 2:
			iRet = fibo_tts_start((uint8 *)rP1, CTTS_STRING_ENCODE_TYPE_UNICODE);
		break;
	}
	sysLOG(TTS_LOG_LEVEL_4, "fibo_tts_start=%d, encode=%d\r\n", iRet, encode);
	free(rP);
	free(rP1);
	return iRet;

}


/*
*Function:		hal_ttsQueuePlay
*Description:	TTS队列播报写入接口
*Input:			*tts_content:播报内容指针, *tts_vol:播报音量指针，不设置则为NULL，*readmode:读数字方式指针，0-自动判断；1-按号码方式；2-按数值方式，NULL-不设置
*				encode:播报内容编码格式，0:UTF-8; 1:GB2312; 2:UNICODE
*Output:		NULL
*Hardware:
*Return:		0:成功；-1:待播报队列已满，失败，-2:队列播报关闭
*Others:
*/
int hal_ttsQueuePlay(int8 *tts_content, uint8 *tts_vol, uint8 *readmode, int8 encode)
{
	int iRet = 0;
	if(g_stTTS_s.tts_enable == 1)//允许往队列里放
	{
		if(g_stTTS_s.tts_ordercnt < TTS_LOOP_NUM)//未缓存满
		{
			if(tts_vol == NULL)//未设置音量
			{
				g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin] = 0xFF;
			}
			else
			{
				g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin] = *tts_vol;		
				sysLOG(TTS_LOG_LEVEL_4, "g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin]:%d, tts_vol:%d\r\n", g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin], *tts_vol);
			}
			if(readmode == NULL)
			{
				g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin] = 0xFF;
			}
			else
			{
				g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin] = *readmode;		
				sysLOG(TTS_LOG_LEVEL_4, "g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin]:%d, readmode:%d\r\n", g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin], *readmode);
			}
			
			g_stTTS_s.tts_encode[g_stTTS_s.tts_orderin] = encode;

			g_stTTS_s.tts_Amount[g_stTTS_s.tts_orderin] = 0xFFFFFFFF;

			int lentmp = 0;
			if(encode == 2)
			{
				while(1)
				{
					if((*(tts_content+lentmp) == 0) && (*(tts_content+lentmp+1) == 0))
					{
						break;
					}
					lentmp += 2;
					if(lentmp >255)
					{
						lentmp = 255;
						break;
					}
				}
				
			}
			else
			{
				lentmp = strlen(tts_content);
			}
			if(lentmp > 255)lentmp = 255;
			sysLOG(TTS_LOG_LEVEL_4, "lentmp=%d\r\n", lentmp);
			memset(g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin], 0, sizeof(g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin]));
			memcpy(g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin], tts_content, lentmp);
			//sprintf(&g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin][strlen(tts_content)], "\r\n");
			g_stTTS_s.tts_orderin++;
			if(g_stTTS_s.tts_orderin==TTS_LOOP_NUM)g_stTTS_s.tts_orderin=0;
			g_stTTS_s.tts_ordercnt+=1;

			iRet = 0;
			sysLOG(TTS_LOG_LEVEL_4, "iRet:%d, g_stTTS_s.tts_encode[g_stTTS_s.tts_orderin]:%d, encode:%d, g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderin:%d, g_stTTS_s.tts_orderout:%d\r\n", iRet, g_stTTS_s.tts_encode[g_stTTS_s.tts_orderin], encode, g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderin, g_stTTS_s.tts_orderout);

		}
		else
		{
			iRet = -1;
			sysLOG(TTS_LOG_LEVEL_1, "<WARN> iRet:%d, g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderin:%d, g_stTTS_s.tts_orderout:%d\r\n", iRet, g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderin, g_stTTS_s.tts_orderout);
			
		}
	}
	else//播报队列关闭，不允许放
	{
		iRet = -2;	
		sysLOG(TTS_LOG_LEVEL_1, "<WARN> iRet:%d, g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderin:%d, g_stTTS_s.tts_orderout:%d\r\n", iRet, g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderin, g_stTTS_s.tts_orderout);
	}
	
	return iRet;

}



/*
*Function:		hal_ttsQueuePlay
*Description:	TTS队列播报写入接口
*Input:			*tts_content:播报内容指针, *tts_vol:播报音量指针，不设置则为NULL，*readmode:读数字方式指针，0-自动判断；1-按号码方式；2-按数值方式，NULL-不设置
*				encode:播报内容编码格式，0:UTF-8; 1:GB2312; 2:UNICODE
*Output:		NULL
*Hardware:
*Return:		0:成功；-1:待播报队列已满，失败，-2:队列播报关闭
*Others:
*/
int hal_ttsQueuePlayAmount(int8 *tts_content, uint8 *tts_vol, uint8 *readmode, int8 encode, int32 *amount)
{
	int iRet = 0;
	if(g_stTTS_s.tts_enable == 1)//允许往队列里放
	{
		if(g_stTTS_s.tts_ordercnt < TTS_LOOP_NUM)//未缓存满
		{
			if(tts_vol == NULL)//未设置音量
			{
				g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin] = 0xFF;
			}
			else
			{
				g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin] = *tts_vol;		
				sysLOG(TTS_LOG_LEVEL_4, "g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin]:%d, tts_vol:%d\r\n", g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderin], *tts_vol);
			}
			if(readmode == NULL)
			{
				g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin] = 0xFF;
			}
			else
			{
				g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin] = *readmode;		
				sysLOG(TTS_LOG_LEVEL_4, "g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin]:%d, readmode:%d\r\n", g_stTTS_s.tts_readmode[g_stTTS_s.tts_orderin], *readmode);
			}
			
			g_stTTS_s.tts_encode[g_stTTS_s.tts_orderin] = encode;

			if(amount == NULL)
			{
				g_stTTS_s.tts_Amount[g_stTTS_s.tts_orderin] = 0xFFFFFFFF;
			}
			else
			{
				g_stTTS_s.tts_Amount[g_stTTS_s.tts_orderin] = *amount;		
				sysLOG(TTS_LOG_LEVEL_4, "g_stTTS_s.tts_Amount[g_stTTS_s.tts_orderin]:%d, amount:%d\r\n", g_stTTS_s.tts_Amount[g_stTTS_s.tts_orderin], *amount);
			}

			int lentmp = 0;
			if(encode == 2)
			{
				while(1)
				{
					if((*(tts_content+lentmp) == 0) && (*(tts_content+lentmp+1) == 0))
					{
						break;
					}
					lentmp += 2;
					if(lentmp >255)
					{
						lentmp = 255;
						break;
					}
				}
				
			}
			else
			{
				lentmp = strlen(tts_content);
			}
			if(lentmp > 255)lentmp = 255;
			sysLOG(TTS_LOG_LEVEL_4, "lentmp=%d\r\n", lentmp);
			memset(g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin], 0, sizeof(g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin]));
			memcpy(g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin], tts_content, lentmp);
			//sprintf(&g_stTTS_s.tts_contenttemp[g_stTTS_s.tts_orderin][strlen(tts_content)], "\r\n");
			g_stTTS_s.tts_orderin++;
			if(g_stTTS_s.tts_orderin==TTS_LOOP_NUM)g_stTTS_s.tts_orderin=0;
			g_stTTS_s.tts_ordercnt+=1;

			iRet = 0;
			sysLOG(TTS_LOG_LEVEL_4, "iRet:%d, g_stTTS_s.tts_encode[g_stTTS_s.tts_orderin]:%d, encode:%d, g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderin:%d, g_stTTS_s.tts_orderout:%d\r\n", iRet, g_stTTS_s.tts_encode[g_stTTS_s.tts_orderin], encode, g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderin, g_stTTS_s.tts_orderout);

		}
		else
		{
			iRet = -1;
			sysLOG(TTS_LOG_LEVEL_1, "<WARN> iRet:%d, g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderin:%d, g_stTTS_s.tts_orderout:%d\r\n", iRet, g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderin, g_stTTS_s.tts_orderout);
			
		}
	}
	else//播报队列关闭，不允许放
	{
		iRet = -2;	
		sysLOG(TTS_LOG_LEVEL_1, "<WARN> iRet:%d, g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderin:%d, g_stTTS_s.tts_orderout:%d\r\n", iRet, g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderin, g_stTTS_s.tts_orderout);
	}
	
	return iRet;

}



/*
*Function:		hal_ttsLoopHandle
*Description:	TTS循环调用的播报接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ttsLoopHandle(void)
{
	int iRet;
	uint8 tts_orderouttmp;
	if(g_stTTS_s.tts_status > 0)//播报过
	{
		if(g_stTTS_s.tts_ordercnt == 0 && fibo_tts_is_playing() == FALSE)//队列里没有待播报内容并且现在未在播报
		{
			if(g_stTTS_s.tts_status == 1)//刚结束
			{
				g_stTTS_s.tts_closetick = hal_sysGetTickms() + TTS_AMPOPEN_TIME;
				g_stTTS_s.tts_status = 2;
				
				sysLOG(TTS_LOG_LEVEL_4, "g_stTTS_s.tts_closetick:%lld\r\n", g_stTTS_s.tts_closetick);
			}
			else if(g_stTTS_s.tts_status == 2)
			{
				if(hal_sysGetTickms() > g_stTTS_s.tts_closetick)
				{
					hal_ttsAmpCtl(FALSE);
					int irrt= fibo_tts_stop();
					sysLOG(TTS_LOG_LEVEL_4, "fibo_tts_stop:%d\r\n", irrt);
					
					g_stTTS_s.tts_status = 0;
					sysLOG(TTS_LOG_LEVEL_4, "hal_ttsAmpCtl(FALSE), hal_sysGetTickms():%lld\r\n", hal_sysGetTickms());
				}
			}
		}	
	}
	if(g_stTTS_s.tts_ordercnt != 0)//有待播报的内容
	{
		
		if(fibo_get_audio_status())//正在播放音频文件，先停止播放
		{
			fibo_audio_stop();
		}
		
		//sysLOG(TTS_LOG_LEVEL_2, "g_stTTS_s.tts_ordercnt:%d, fibo_tts_is_playing():%d, fibo_get_audio_status():%d\r\n", g_stTTS_s.tts_ordercnt, fibo_tts_is_playing(), fibo_get_audio_status());
		if((fibo_tts_is_playing() == FALSE) && (fibo_get_audio_status() == FALSE))//未正在播放
		{
			int8 encodetmp = 0;
			if(g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderout] != 0xFF)
			{
				hal_ttsSetVolume(g_stTTS_s.tts_volumetemp[g_stTTS_s.tts_orderout]);
			}
			
			if(g_stTTS_s.tts_encode[g_stTTS_s.tts_orderout] != 0xFF)
			{
				encodetmp = g_stTTS_s.tts_encode[g_stTTS_s.tts_orderout];
			}
			else
			{
				encodetmp = 0;//默认是采用UTF-8格式
			}
			iRet = fibo_sem_try_wait(g_ui32SemTTSPlayHandle, 1000);
			if(iRet == FALSE)
			{	
				sysLOG(TTS_LOG_LEVEL_3, "<WARN> fibo_sem_try_wait, iRet:%d\r\n", iRet);
			}
			/*
			*先把tts_orderout和tts_ordercnt的值修改了，防止DR_TtsPlay有延时导致调用clear清队列时，
			*clear已经把tts_ordercnt变为0了，然后放进去一条之后，由tts_ordercnt-=1，最后一条不能正常播报出来。
			*/
			g_stTTS_s.tts_orderout+=1;
			if(g_stTTS_s.tts_orderout==TTS_LOOP_NUM)g_stTTS_s.tts_orderout=0;
			g_stTTS_s.tts_ordercnt-=1;
			if(g_stTTS_s.tts_orderout == 0)
			{
				tts_orderouttmp = TTS_LOOP_NUM-1;
			}
			else
			{
				tts_orderouttmp = g_stTTS_s.tts_orderout-1;
			}

			/*如果有传递金额，需要打开并显示断码屏*/
			if(g_stTTS_s.tts_Amount[tts_orderouttmp] != 0xFFFFFFFF)
			{
				hal_cblcdOpen(TRUE);
				dpyDispAmount_lib(g_stTTS_s.tts_Amount[tts_orderouttmp]);
			}
			
			hal_ttsAmpCtl(TRUE);
			if(g_stTTS_s.tts_readmode[tts_orderouttmp] != 0xFF)
			{
				hal_ttsPlay(g_stTTS_s.tts_contenttemp[tts_orderouttmp], g_stTTS_s.tts_readmode[tts_orderouttmp], encodetmp);
			}
			else
			{
				hal_ttsPlay(g_stTTS_s.tts_contenttemp[tts_orderouttmp], 0, encodetmp);
			}
			/*
			*清队列和播报加一个互斥锁Sem_TTSPlay_mutex。
			*有概率调用DR_TtsQueueClear和DR_TtsQueuePlay时走在了这里，
			*DR_TtsQueueClear把tts_s.tts_orderout清0了，然后又放进去一条播报，tts_s.tts_orderout=1
			*这时走到下面tts_s.tts_ordercnt-=1时又把tts_s.tts_ordercnt减成了0，导致放进去的最后一条没有播报出来！！！
			*/
			
			sysLOG(TTS_LOG_LEVEL_5, "g_stTTS_s.tts_ordercnt:%d, g_stTTS_s.tts_orderout:%d, tts_orderouttmp:%d\r\n", g_stTTS_s.tts_ordercnt, g_stTTS_s.tts_orderout, tts_orderouttmp);
			fibo_sem_signal(g_ui32SemTTSPlayHandle);
			g_stTTS_s.tts_status = 1;
		}
		
	}


}


/*
*Function:		hal_ttsQueueClear
*Description:	清播报队列
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_ttsQueueClear(void)
{

	int iRet;
	
	sysLOG(TTS_LOG_LEVEL_4, "hal_ttsQueueClear start!\r\n");
	iRet = fibo_sem_try_wait(g_ui32SemTTSPlayHandle, 2000);
	if(iRet == FALSE)
	{	
		sysLOG(TTS_LOG_LEVEL_3, "<WARN> fibo_sem_try_wait, iRet:%d\r\n", iRet);
	}
	//读指针等于写指针，并且个数清0

	//g_stTTS_s.tts_orderin = 0;
	g_stTTS_s.tts_orderout = g_stTTS_s.tts_orderin;
	g_stTTS_s.tts_ordercnt = 0;
	
	sysLOG(TTS_LOG_LEVEL_4, "hal_ttsQueueClear end!\r\n");
	fibo_sem_signal(g_ui32SemTTSPlayHandle);
}


/*
*Function:		hal_ttsGetSpare
*Description:	获取TTS队列剩余空间
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		-1:获取失败; -2:队列播报关闭; >=0剩余空间个数;
*Others:
*/
int hal_ttsGetSpare(void)
{
	int iRet = -1;
	if(g_stTTS_s.tts_enable == 1)//允许往队列里放
	{
		iRet = TTS_LOOP_NUM - g_stTTS_s.tts_ordercnt;

	}
	else
	{
		iRet = -2;
	}

	return iRet;
}




/*
*Function:		hal_ttsSetLanguage
*Description:	选择TTS语言
*Input:			ttslangtype:0-中文; 1-英文
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int hal_ttsSetLanguage(unsigned char ttslangtype)
{
	int iRet = -1;

	int ttstypetmp = ttslangtype;
	unsigned char ttspath[256];

	if(TTS_LIBTYPE_CN != ttslangtype && TTS_LIBTYPE_EN != ttslangtype)
	{
		return -1;
	}

	memset(ttspath, 0, sizeof(ttspath));
	iRet = hal_cfgReadTTSPath(ttspath, ttstypetmp);
	if(iRet < 0)
	{
		if(TTS_LIBTYPE_CN == ttstypetmp)
		{
			memcpy(ttspath, TTS_LIBPATH_CN, strlen(TTS_LIBPATH_CN));
			hal_cfgWriteTTSCfg(ttspath, NULL, &ttstypetmp);
		}
		else if(TTS_LIBTYPE_EN == ttstypetmp)
		{
			memcpy(ttspath, TTS_LIBPATH_EN, strlen(TTS_LIBPATH_EN));
			hal_cfgWriteTTSCfg(NULL, ttspath, &ttstypetmp);
		}
	}
	else
	{
		hal_cfgWriteTTSCfg(NULL, NULL, &ttstypetmp);
	}
	
	iRet = cus_export_api->fibo_set_ext_ttslib_path(ttspath);
	iRet = cus_export_api->fibo_set_ext_ttsplay_choren((uint8)ttstypetmp);

	sysLOG(TTS_LOG_LEVEL_1, "ttstypetmp=%d, ttspath:%s\r\n", ttstypetmp, ttspath);

	return iRet;
}


/*
*Function:		hal_ttsSelLanguage
*Description:	读取TTS语言
*Input:			NULL
*Output:		*ttslangtype:0-中文; 1-英文
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int hal_ttsGetLanguage(unsigned char *ttslangtype)
{
	int iRet = -1;
	int ttstypetmp = 0;

	iRet = cus_export_api->fibo_get_ext_ttsplay_choren();
	if(iRet >= 0)
	{
		*ttslangtype = iRet;
		iRet = 0;
	}

	return iRet;
	
}


/*
*Function:		hal_ttsSetLibPath
*Description:	设置TTS语音库路径
*Input:			*libpathcn：中文语音库路径; *libpathen:英文语音库路径
*Output:		NULL
*Hardware:
*Return:		0-成功，<0-失败;
*Others:
*/
int hal_ttsSetLibPath(unsigned char *libpathcn, unsigned char *libpathen)
{
	int iRet = -1;

	iRet = hal_cfgWriteTTSCfg(libpathcn, libpathen, NULL);

	return iRet;
}

/*
*Function:		hal_ttsGetLibPath
*Description:	读取TTS语音库路径
*Input:			ttstype:0-中文；1-英文
*Output:		*libpath：读到的语音库路径;
*Hardware:
*Return:		>=0-成功，<0-失败;
*Others:
*/
int hal_ttsGetLibPath(char *libpath, int ttstype)
{
	int iRet = -1;

	iRet = hal_cfgReadTTSPath(libpath, ttstype);
	if(iRet < 0)
	{
		if(TTS_LIBTYPE_CN == ttstype)
			memcpy(libpath, TTS_LIBPATH_CN, strlen(TTS_LIBPATH_CN));
		else if(TTS_LIBTYPE_EN == ttstype)
			memcpy(libpath, TTS_LIBPATH_EN, strlen(TTS_LIBPATH_EN));

		iRet = 0;
	}

	return iRet;
}

/*
*Function:		hal_ttsLibInit
*Description:	初始化TTS语音库
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		>=0-成功，<0-失败;
*Others:
*/
int hal_ttsLibInit(void)
{
	int iRet = -1;
	int ttslibtype = 0;
	unsigned char ttslibpathtmp[256];

	memset(ttslibpathtmp, 0, sizeof(ttslibpathtmp));
	
	iRet = hal_cfgReadTTSType(&ttslibtype);
	if(iRet < 0)
	{
		ttslibtype = TTS_LIBTYPE_CN;
	}

	
	iRet = hal_ttsSetLanguage((unsigned char)ttslibtype);

	return iRet;

}



#if MAINTEST_FLAG

void hal_ttsTest(void)
{
	uint8 value;
	uint8 readmode;
	int32 amounttest = 0;
	
//	hal_ttsQueuePlay("欢迎使用艾体威尔支付", NULL, NULL, 0);
	hal_ttsQueuePlay("welcome", NULL, NULL, 0);

//	ttsQueuePlayAmount_lib("支付宝到账20元", &value, NULL, 0, NULL);
//	amounttest = 1000;
//	ttsQueuePlayAmount_lib("支付宝到账10元", &value, NULL, 0, &amounttest);
//	amounttest = 8888;
//	ttsQueuePlayAmount_lib("支付宝到账88.88元", &value, NULL, 0, &amounttest);
//	amounttest = 999900;
//	ttsQueuePlayAmount_lib("支付宝到账9999元", &value, NULL, 0, &amounttest);

}

uint8 ttsloopflag = 1;

void hal_ttsLoopTest(void)
{
	uint8 value;
	value = 5;
	switch(ttsloopflag){

		case 1:
			hal_ttsQueuePlay("轻轻的我走了            正如我轻轻的来", &value, NULL, 0);
			ttsloopflag = 2;
		break;
		case 2:
			hal_ttsQueuePlay("我轻轻的招手            作别西天的云彩", &value, NULL, 0);
			ttsloopflag = 3;
		break;
		case 3:
			hal_ttsQueuePlay("那河畔的金柳            是夕阳中的新娘", &value, NULL, 0);
			ttsloopflag = 4;
		break;
		case 4:
			hal_ttsQueuePlay("波光里的艳影            在我的心头荡漾", &value, NULL, 0);
			ttsloopflag = 1;
		break;
	

	}
	
}


void hal_ttsLoopPlayThread(void *param)
{
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);
	
	while(1)
	{
		
		hal_ttsLoopTest();
		sysDelayMs(4000);
		
	}
    
}


#endif

