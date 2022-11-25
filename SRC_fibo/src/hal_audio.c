/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_audio.c
** Description:		audio播报相关接口
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
*Function:		hal_audioFilePlay
*Description:	播放语音文件
*Input:			audioFileName:音频文件名称,只播放/FFS/目录下音频文件
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioFilePlay(char *audioFileName)
{
	int32 iRet = 0;
	int32 iFileSize = 0;
	int8 nameFile[256];
	memset(nameFile, 0, sizeof(nameFile));
	memcpy(nameFile, "/FFS/", sizeof("/FFS/"));
    memcpy(&(nameFile[strlen(nameFile)]), audioFileName, strlen(audioFileName));
	if(1 != fibo_file_exist(nameFile))
	{
	    iRet =  LDERR_AUDIO_FILENOTEXIST;
		goto RET_END;
	}
	iFileSize = fibo_file_getSize(nameFile);
	if(iFileSize <= 0)
	{
	    iRet =  LDERR_AUDIO_FILENOTEXIST;
		goto RET_END;
	}
	iRet = fileJudgeFilename(nameFile);
	if(iRet < 0)
	{
		sysLOG(AUDIO_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto RET_END;
	}
	if((g_stTTS_s.tts_ordercnt == 0) && (!fibo_get_audio_status()))
	{
		iRet = fibo_audio_play(1, audioFileName);
	}
	else
	{
	    iRet = LDERR_AUDIO_TTSPLAYING;
	}
RET_END:
	sysLOG(AUDIO_LOG_LEVEL_2, "nameFile:%s,iFileSize:%d,iRet:%d\r\n", nameFile, iFileSize, iRet);
	return iRet;
}


/*
*Function:		hal_audioFilePlayPath
*Description:	播放语音文件
*Input:			audioFileName:音频文件名称带路径，如："/ext/app/data/scanok.mp3"
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioFilePlayPath(char *audioFileName)
{
	int32 iRet = 0;
	int32 iFileSize = 0;
	if(1 != fibo_file_exist(audioFileName))
	{
	    iRet =  LDERR_AUDIO_FILENOTEXIST;
		goto RET_END;
	}
	iFileSize = fibo_file_getSize(audioFileName);
	if(iFileSize <= 0)
	{
	    iRet =  LDERR_AUDIO_FILENOTEXIST;
		goto RET_END;
	}
	iRet = fileJudgeFilename(audioFileName);
	if(iRet < 0)
	{
		sysLOG(AUDIO_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto RET_END;
	}
	if((g_stTTS_s.tts_ordercnt == 0) && (!fibo_get_audio_status()))
	{
	    
		fibo_audio_path_play(1, audioFileName, NULL);
	}
	else
	{
	    iRet = LDERR_AUDIO_TTSPLAYING;
	}
RET_END:
	sysLOG(AUDIO_LOG_LEVEL_2, "audioFileName:%s,iFileSize:%d,iRet:%d\r\n", audioFileName, iFileSize, iRet);
	return iRet;
}


/*
*Function:		hal_audioPlayList
*Description:	播放音频列表文件
*Input:			Fname[][128]：传入文件名<包含绝对路径>，长度不超过128字节<含路径>
*				Num：传入文件个数
*Output:		NULL
*Hardware:
*Return:		成功-返回0
*				其他错误：-3TTS占用，-1--未知错误，         1-格式错误，2-参数错误，4-读取错误，5-文件个数小于等于0    ，    6-播放错误
*Others:
*/
int hal_audioPlayList(const char fname[][128], int32_t num)
{
	int32 iRet = LDERR_GENERIC;
	if((g_stTTS_s.tts_ordercnt == 0) && (!fibo_get_audio_status()))
	{
	    iRet = fibo_audio_list_play(fname, num);
	}
	else
	{
	    iRet = LDERR_AUDIO_TTSPLAYING;
	}
	sysLOG(AUDIO_LOG_LEVEL_2, "iRet:%d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_audioPlay
*Description:	播放文件系统中的音频文件。
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioPlay(void)
{
	int32 iRet = LDERR_GENERIC;
	int32 iFileSize = 0;
	int8 nameFile[64];
	memset(nameFile, 0, sizeof(nameFile));
    strcpy(nameFile, "/FFS/audioplay.mp3");
	if(1 != fibo_file_exist(nameFile))
	{
	    iRet =   LDERR_AUDIO_FILENOTEXIST;
		goto RET_END;
	}
	iFileSize = fibo_file_getSize(nameFile);
	if(iFileSize <= 0)
	{
	    iRet =  LDERR_AUDIO_FILENOTEXIST;
		goto RET_END;
	}
	iRet = fileJudgeFilename(nameFile);
	if(iRet < 0)
	{
		sysLOG(AUDIO_LOG_LEVEL_2, "fileJudgeFilename,iRet=%d\r\n", iRet);
		goto RET_END;
	}
	if((g_stTTS_s.tts_ordercnt == 0) && (!fibo_get_audio_status()))
	{
		iRet = fibo_audio_play(1, "audioplay.mp3");
	}
	else
	{
	    iRet = LDERR_AUDIO_TTSPLAYING;
	}
	
RET_END:
	sysLOG(AUDIO_LOG_LEVEL_2, "nameFile:%s,iFileSize:%d,iRet:%d\r\n", nameFile, iFileSize, iRet);
	return iRet;
}


/*
*Function:		hal_audioPause
*Description:	暂停正在播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioPause(void)
{
	int32 iRet = LDERR_AUDIO_GENERIC;
	iRet = fibo_audio_pause();
	sysLOG(AUDIO_LOG_LEVEL_2, "iRet:%d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_audioResume
*Description:	恢复播放正在暂停播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioResume(void)
{
	int32 iRet = LDERR_AUDIO_GENERIC;
	iRet = fibo_audio_resume();
	sysLOG(AUDIO_LOG_LEVEL_2, "iRet:%d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_audioStop
*Description:	停止正在播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioStop(void)
{
	int32 iRet = LDERR_AUDIO_GENERIC;
	iRet = fibo_audio_stop();
	sysLOG(AUDIO_LOG_LEVEL_2, "iRet:%d\r\n", iRet);
	return iRet;
}


/*
*Function:		hal_audioGetStatus
*Description:	查询当前audio状态的接口，繁忙或空闲
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		true 忙 false 空闲
*Others:
*/
bool hal_audioGetStatus(void)
{
    bool blRet = FALSE;
	if((g_stTTS_s.tts_ordercnt != 0) || (fibo_get_audio_status()))
	{
	    blRet = TRUE;
	}
	sysLOG(AUDIO_LOG_LEVEL_2, "blRet:%d\r\n", blRet);
	return blRet;
}



/******************************TEST******************************/

#if MAINTEST_FLAG

int audioTest(void)
{
	int32 iRet = 0;

    iRet = hal_audioFilePlay("audioplay.mp3");
#if 0
	iRet = hal_audioPlay();
	if(0 == iRet)
	{
	    sysDelayMs(5000);
	    hal_audioPause();
		hal_ttsQueuePlay("暂停播报广告", NULL, NULL, 0);
		sysDelayMs(5000);
		hal_ttsQueuePlay("恢复播报广告", NULL, NULL, 0);
		sysDelayMs(5000);
		hal_audioResume();
		sysDelayMs(5000);
		hal_audioStop();
		hal_ttsQueuePlay("停止播报广告", NULL, NULL, 0);
	}
#endif
	return iRet;
}

#endif



