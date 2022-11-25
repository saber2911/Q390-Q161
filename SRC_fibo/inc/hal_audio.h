/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_audio.h
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

#ifndef _HAL_AUDIO_H_
#define _HAL_AUDIO_H_

#include "comm.h"

#define AUDIO_LOG_LEVEL_0		LOG_LEVEL_0
#define AUDIO_LOG_LEVEL_1		LOG_LEVEL_1
#define AUDIO_LOG_LEVEL_2		LOG_LEVEL_2


#define LD_AUDIO_OK						0	//成功
#define LDERR_AUDIO_GENERIC				-1	//失败
#define LDERR_AUDIO_FILENOTEXIST	    -2	//音频文件不存在
#define LDERR_AUDIO_TTSPLAYING    	    -3	//TSS占用


/*
*Function:		hal_audioFilePlay
*Description:	播放语音文件
*Input:			audioFileName:音频文件名称,只播放/FFS/目录下音频文件
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioFilePlay(char *audioFileName);

/*
*Function:		hal_audioFilePlayPath
*Description:	播放语音文件
*Input:			audioFileName:音频文件名称带路径，如："/ext/app/data/scanok.mp3"
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioFilePlayPath(char *audioFileName);

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
int hal_audioPlayList(const char fname[][128], int32_t num);

/*
*Function:		hal_audioPlay
*Description:	播放文件系统中的音频文件。
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioPlay(void);

/*
*Function:		hal_audioPause
*Description:	暂停正在播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioPause(void);

/*
*Function:		hal_audioResume
*Description:	恢复播放正在暂停播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioResume(void);

/*
*Function:		hal_audioStop
*Description:	停止正在播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int hal_audioStop(void);

/*
*Function:		hal_audioGetStatus
*Description:	查询当前audio状态的接口，繁忙或空闲
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		true 忙 false 空闲
*Others:
*/
bool hal_audioGetStatus(void);


#endif


