
/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_tts.h
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

#ifndef _HAL_TTS_H_
#define _HAL_TTS_H_

#include "comm.h"


#define TTS_LOG_LEVEL_0		LOG_LEVEL_0
#define TTS_LOG_LEVEL_1		LOG_LEVEL_1
#define TTS_LOG_LEVEL_2		LOG_LEVEL_2
#define TTS_LOG_LEVEL_3		LOG_LEVEL_3
#define TTS_LOG_LEVEL_4		LOG_LEVEL_4
#define TTS_LOG_LEVEL_5		LOG_LEVEL_5



#define TTS_DEBUG_ENABLE	1//DEBUG调试时语音播报
#define TTS_LOOP_NUM		64//tts可以缓存64个播报队列
#define TTS_AMPOPEN_TIME	3000//tts不播报3s后关闭功放

#define TTS_LIBPATH			"/ext/TTS/"
#define TTS_LIBPATH_CN		"/ext/TTS/Resource_4M.irf"
#define TTS_LIBPATH_EN		"/ext/TTS/Resource_4M_EN.irf"
#define TTS_LIBTYPE_CN		0
#define TTS_LIBTYPE_EN		1


struct _TTS_S{
	uint8 tts_enable;//TTS播报队列使能
	uint8 tts_orderin;//TTS播报队列写指针
	uint8 tts_orderout;//TTS播报队列读指针
	uint8 tts_ordercnt;//TTS播报队列已缓存个数
	uint8 tts_status;//tts未播报为0，调用播报接口后为1，播报完毕后为2
	unsigned long long tts_closetick;//延时关闭功放计时
	uint8 tts_volume;//缓存音量值;
	uint8 tts_volumetemp[TTS_LOOP_NUM];//音量
	uint8 tts_readmode[TTS_LOOP_NUM];//读数字方式
	int8 tts_encode[TTS_LOOP_NUM];//内容编码方式
	int8 tts_contenttemp[TTS_LOOP_NUM][256];//待播报的内容
	int32 tts_Amount[TTS_LOOP_NUM];//断码屏金额,单位分，如果为0xFFFFFFFF则表示不需要显示
	
}TTS_S;


extern struct _TTS_S g_stTTS_s;


void hal_ttsSetVolume(uint8 volume);
int hal_ttsQueuePlay(int8 *tts_content, uint8 *tts_vol, uint8 *readmode, int8 encode);
int hal_ttsQueuePlayAmount(int8 *tts_content, uint8 *tts_vol, uint8 *readmode, int8 encode, int32 *amount);
void hal_ttsLoopHandle(void);
void hal_ttsInit(void);
void hal_ttsQueueClear(void);
int hal_ttsGetSpare(void);


int hal_ttsSetLanguage(unsigned char ttslangtype);
int hal_ttsGetLanguage(unsigned char *ttslangtype);
int hal_ttsSetLibPath(unsigned char *libpathcn, unsigned char *libpathen);
int hal_ttsGetLibPath(char *libpath, int ttstype);

int hal_ttsLibInit(void);


void hal_ttsTest(void);
void hal_ttsLoopTest(void);
void hal_ttsLoopPlayThread(void *param);




#endif

