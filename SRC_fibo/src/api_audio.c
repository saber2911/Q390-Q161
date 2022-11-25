/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		api_audio.c
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
*Function:		audioFilePlay_lib
*Description:	播放语音文件
*Input:			audioFileName:音频文件名称,只播放/FFS/目录下音频文件
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int audioFilePlay_lib(char *audioFileName)
{
	return hal_audioFilePlay(audioFileName);
}


/*
*Function:		audioFilePlayPath_lib
*Description:	播放语音文件
*Input:			audioFileName:音频文件名称带路径，如："/ext/app/data/scanok.mp3"
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int audioFilePlayPath_lib(char *audioFileName)
{
	return hal_audioFilePlayPath(audioFileName);
}


/*
*Function:		audioPlayList_lib
*Description:	播放音频列表文件
*Input:			Fname[][128]：传入文件名<包含绝对路径>，长度不超过128字节<含路径>
*				Num：传入文件个数
*Output:		NULL
*Hardware:
*Return:		成功-返回0
*				其他错误：-3TTS占用，-1--未知错误，         1-格式错误，2-参数错误，4-读取错误，5-文件个数小于等于0    ，    6-播放错误
*Others:
*/
int audioPlayList_lib(const char fname[][128], int32_t num)
{
	return hal_audioPlayList(fname, num);
}


/*
*Function:		audioPlay_lib
*Description:	播放文件系统中的音频文件。
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int audioPlay_lib(void)
{
	return hal_audioPlay();
}


/*
*Function:		audioPause_lib
*Description:	暂停正在播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int audioPause_lib(void)
{
	return hal_audioPause();
}


/*
*Function:		audioResume_lib
*Description:	恢复播放正在暂停播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int audioResume_lib(void)
{
	return hal_audioResume();
}


/*
*Function:		audioStop_lib
*Description:	停止正在播放的音频文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0 成功 <0 失败
*Others:
*/
int audioStop_lib(void)
{
	return hal_audioStop();
}


/*
*Function:		getAudioStatus_lib
*Description:	查询当前audio状态的接口，繁忙或空闲
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		true 忙 false 空闲
*Others:
*/
bool getAudioStatus_lib(void)
{
	return hal_audioGetStatus();
}






