/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_load.h
** Description:		下载及外部通信相关接口
**
** Version:	1.0, 渠忠磊,2022-03-01
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#ifndef _HAL_LOAD_H_
#define _HAL_LOAD_H_

#include "comm.h"

#define LOAD_LOG_LEVEL_0		LOG_LEVEL_0
#define LOAD_LOG_LEVEL_1		LOG_LEVEL_1
#define LOAD_LOG_LEVEL_2		LOG_LEVEL_2
#define LOAD_LOG_LEVEL_3		LOG_LEVEL_3
#define LOAD_LOG_LEVEL_4		LOG_LEVEL_4
#define LOAD_LOG_LEVEL_5		LOG_LEVEL_5


#define APP_NOSIGN_EN			0//1-允许不签名的APP下载
#define APP_OLDSIGN_EN			0//1-允许老式方案签名的APP下载
#define APP_NEWSIGN_EN			1//1-允许艾体正式签名方案的APP下载

#define FILETYPE_PUK			0xA6
#define FILETYPE_APP			0xA7
#define FILETYPE_FONT			0xA8
#define FILETYPE_PARAMFILE		0xA9

#define FILETYPE_PARAMTYPE_CFGFILE		0x06//配置文件
#define FILETYPE_PARAMTYPE_APPFILE		0x04//应用自定义文件，默认放到/ext/app/data中
#define FILETYPE_PARAMTYPE_AUDIOFILE	0x05//音频文件
#define FILETYPE_PARAMTYPE_FILE			0x07//带绝对路径的文件名



#define APP_DIR		"/ext/app/data/"

typedef struct _LD_PKG{
	uint8 start_code;
	uint8 pkg_index;
	uint8 cmd_type;
	uint8 cmd;
	uint16 datalen;
	uint8 re_code;
	uint8 data[8*1024];
	uint8 lrc;
}LD_PKG;

#define BININFO_HEAD_SIZE		8
#define BININFO_FORMAT_SIZE		4
#define BININFO_COMPA_SIZE		16
#define BININFO_RESINFO_SIZE	960
#define BININFO_HASHV_SIZE		32

typedef struct _BinInfoAA66{

	uint8 head[BININFO_HEAD_SIZE];//头部标识
	uint8 format[BININFO_FORMAT_SIZE];//签名方式
	uint32 appsize;//原生APP大小
	uint8 compatible[BININFO_COMPA_SIZE];//兼容性控制
	uint8 reservedinfo[BININFO_RESINFO_SIZE];//预留信息
	uint8 hashvalue[BININFO_HASHV_SIZE];//哈希值

}BinInfoAA66;


typedef struct _APP_S{
	uint32 sizeAPP;//APP大小
	uint32 sizeAPP_recvd;//已经收到的APP字节个数
	int iPackNo;//帧序
	int8 g_DesAppName[64];//存放即将写入的APP文件
	int8 SHA_value[20];//计算后的哈希值
	int8 SHA_value_recvd[20];//接收到的哈希值
	int8 MD5_value[16];
	int8 MD5_value_recvd[16];
    uint32 g_signLen;
    uint32 g_isSignExist;
    int8 signVal[256];
	int appflag;//0-Q181原来的签名方案,1-APP正式签名方案
	BinInfoAA66 bininfoAA66;
}APP_S;

typedef struct _LOADFILE_S{

	uint32 sizeFile;//文件大小
	uint32 sizeFile_recvd;//已经收到的字节数
	int iPackNo;//帧序
	uint8 paramtypeFile;//参数文件类型，参数文件CFG为0x06, 应用自定义文件为0x04，音频文件0x05，绝对路径文件0x07
	uint8 numFile;//应用编号，该文件属于的应用编号
	int8 nameFile[64];//存放正在写入的FILE名字
	int8 nameFile_recvd[64];//存放即将写入的FILE名字
	uint8 typefile;//文件类型,0xA6-公钥,0xA7-APP,A8-font,A9-paramfile
	uint32 fontwriteP;//写入字库时的写指针
	int8 tmpnameFile[64];//存放下载结束前的缓存名字
	
}LOADFILE_S;


#define LD_STARTCODE		0x02


#define LD_CMD_TYPE			0x80

#define	LD_CMDCODE_TEST					0x00//测试
#define LD_CMDCODE_BAUD					0xA1//设置通信波特率，直接反LD_OK即可，实际上啥都没做
#define LD_CMDCODE_POSINFO				0xA2//查询pos终端信息
#define LD_CMDCODE_APPINFO				0xA3//查询应用信息
#define LD_CMDCODE_REBUILDFILESYS		0xA4//重建POS终端文件系统，啥都没有做，直接反LD_OK
#define LD_CMDCODE_SETTIME				0xA5//设置POS终端时间，BCD码
#define LD_CMDCODE_STARTLOADPUK			0xA6//发起下载用户公钥(US_PUK/UA_PUK)文件请求，1-USPUK;2-UAPUK
#define LD_CMDCODE_STARTLOADAPP			0xA7//发起下载APP请求
#define LD_CMDCODE_STARTLOADFONT		0xA8//发起下载字库文件请求
#define LD_CMDCODE_STARTLOADPARAM		0xA9//发起下载参数文件请求
#define LD_CMDCODE_LOADFILE				0xAB//以非压缩方式下载文件
#define LD_CMDCODE_WRITEFILE			0xAD//写入文件
#define LD_CMDCODE_LOADDONE				0xBF//下载完成
#define LD_CMDCODE_STARTLOADFILE		0xD1//发起下载数据文件请求
#define LD_CMDCODE_PUK					0xB1//下载公钥

#define LD_CMDCODE_STARTGETPEDTLOG    	0xC1//发起获取ped测试log
#define LD_CMDCODE_READPEDTESTLOG    	0xC2//读取ped测试log
#define LD_CMDCODE_DELETEPEDTESTLOG    	0xC3//删除ped测试log文件




#define LD_CMDCODE_READSN				0x80//读SN
#define	LD_CMDCODE_WRITESN				0x81//写SN
#define	LD_CMDCODE_READAPPVER			0x82//读APP版本信息
#define	LD_CMDCODE_BATMILLIVOLT			0x83//读电池电压，单位mV
#define	LD_CMDCODE_LOGLEVEL				0x84//设置log等级
#define	LD_CMDCODE_WIFIAT				0x85//透传WIFI的AT指令
//#define LD_CMDCODE_LDAT					0x86//透传模组AT指令
#define	LD_CMDCODE_WRITEHWVER			0x87//写硬件标识
#define	LD_CMDCODE_READHWVER			0x88//读硬件标识
#define	LD_CMDCODE_WRITECID				0x89//写客户标识
#define	LD_CMDCODE_READCID				0x8A//读客户标识
//#define LD_CMDCODE_LOADAPP				0x8B//下载APP
#define	LD_CMDCODE_READDRVER			0x8C//读驱动层版本信息
//#define	LD_CMDCODE_UPDATEAPP			0x8D//更新APP
#define LD_CMDCODE_WIFIPWR				0x8E//设置WiFi上下电
#define LD_CMDCODE_NREBOOT				0x8F//下载应用时的重启指令，不做真正的重启，直接回复LD_OK即可
#define LD_CMDCODE_REBOOT				0x90//重启
#define	LD_CMDCODE_WRITEHWVERSTRING		0x91//写硬件版本号
#define	LD_CMDCODE_READHWVERSTRING		0x92//读硬件版本号
#define LD_CMDCODE_CLRQUICKPARAM		0x93//清除快充记录
#define LD_CMDCODE_GETCELLINFO          0x94//获取电子围栏
#define LD_CMDCODE_WRITEKEY             0x95//写入密钥
#define LD_CMDCODE_GETDATAMD5           0x96//获取MD5

#define LD_CMDCODE_AUDIOPLAY            0x97//播放音频文件
#define LD_CMDCODE_AUDIOPAUSE           0x98//暂停播放音频文件
#define LD_CMDCODE_AUDIORESUME          0x99//恢复播放音频文件
#define LD_CMDCODE_AUDIOSTOP            0x9A//停止播放音频文件
#define	LD_CMDCODE_TERMINALNAME			0x9B//读取终端名字

#define	LD_CMDCODE_CLOSEPORT			0x9C//关掉串口(PORT0-PORT5)
#define	LD_CMDCODE_READTERM				0x9D//读内部机型数据
#define	LD_CMDCODE_WRITETERM			0x9E//写内部机型数据
#define	LD_CMDCODE_READFTTYPE			0x9F//读字库类型
#define	LD_CMDCODE_WRITEFTTYPE			0xA0//写字库类型


#define LD_OK						0x00	//成功
#define LDERR_GENERIC				0x01	//失败
#define LDERR_HAVEMOREDATA			0x02	//还有下一个包,请求的数据未能一次全部返回
#define LDERR_UNSUPPORTEDBAUDRATE	0x03	//波特率不支持
#define LDERR_INVALIDTIME			0x04	//非法时间
#define LDERR_CLOCKHWERR			0x05	//时钟硬件错误
#define LDERR_INVALIDSIGNATURE		0x06	//验证签名失败
#define LDERR_TOOMANYAPPLICATIONS	0x07	//应用太多，不能下载更多应用
#define LDERR_TOOMANYFILES			0x08	//文件太多，不能下载更多文件
#define LDERR_APPLCIATIONNOTEXIST	0x09	//指定应用不存在
#define LDERR_UNKNOWNAPPTYPE		0x0A	//不可识别的应用类型
#define LDERR_SIGTYPEERR			0x0B	//签名的数据类型和下载请求类型不一致
#define LDERR_SIGAPPERR				0x0C	//签名的数据所属的应用名和下载请求应用名不一致
#define LDERR_REPEAT_APPNAME		0x0D	//下载的子应用与主应用重名或者下载的主应用与子应用重名
#define LDERR_WRITEFILEFAIL			0x10	//文件写入失败
#define LDERR_NOSUFFICIENTSPACE		0x11	//没有足够的空间
#define LDERR_DECOMPRESS_ERR		0xA0	//数据解压缩失败
#define LDERR_APP_TOO_BIG			0xA1	//应用文件太大
#define LDERR_FILE_EXIST			0xA2	//文件已存在
#define LDERR_DATA_LEN  			0xA3	//数据长度不正确
#define LDERR_ENCRYPT  			    0xA4	//加密失败
#define LDERR_DECRYPT  			    0xA5	//解密失败
#define LDERR_UNSUPPORTEDCMD		0xFF	//不支持该命令

#define LD_POSTYPE_SE				0x18//0x18//SE机型码
#define LD_POSTYPE_LC610N			0x19//0x19//610机型码

#define LD_POSSHAKEHAND				0x30//握手
#define LD_POSSHAKEHAND_SUCC		0x31//握手成功
#define LD_POSTYPEREAD				0x32//读取机型
#define LD_POSTYPEREAD_SUCC			0x33//机型读取成功


extern LD_PKG g_stRecvPkg;
extern LD_PKG g_stSendPkg;

extern APP_S g_stAppStr;


int hal_ldIsSignFlagExist(int8 *data);
int hal_ldWriteSignInfo(uint8 *data,uint32 dataLen,uint32 lastLen);
void hal_ldAPPStrFormat(APP_S *ap_sP);


int32 hal_ldRecvDataAnalysis(uint8 *data, int16 datalen, LD_PKG *ld_pkg_recv, int16 *usedlen);
void hal_ldProcessHandle(void);
int32 hal_ldRecvDataHandle(LD_PKG *ld_pkg_recv, LD_PKG *ld_pkg_send);
int32 hal_ldUpdateSignApp(int8 *ldappname, uint32 offset, int8 *sign_name);



#endif


