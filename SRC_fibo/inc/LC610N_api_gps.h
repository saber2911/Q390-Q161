#ifndef LC610N_API_GPS_H
#define LC610N_API_GPS_H

#include "comm.h"

#define GPS_OK 					0		//操作成功
#define GPS_ERR_NO_DATA 		-8050	//没有数据
#define GPS_ERR_RECEIVE_DATA 	-8051	//接收数据错误
#define GPS_ERR_GET_MSG 		-8052	//获取卫星信息错误
#define GPS_ERR_NO_ELT 			-8053	//没上电
#define GPS_ERR_TIME_OUT 		-8054	//操作超时,未获取到状态
#define GPS_ERR_TASK_CREAT 		-8055	//创建GPS线程失败
#define GPS_ERR_TASK_DELETE 	-8056	//删除GPS线程失败
#define GPS_ERR_NO_GPS 			-8057	//无GPS





extern int g_iTypeofGPS; //0-无，01-AP端内置，02-挂载SE端，03-挂载AP端
typedef struct {
    uint8_t Lon_area;   //东/西经
    uint8_t Lat_area;   //南/北纬
		double Lon;     //GPS Latitude and longitude  //经度
		double Lat;         //纬度
}GPS_Coordinates;

typedef struct {
	uint8_t GPNoSv;       //使用GPS卫星数,MAX=12
	uint8_t GBNoSv;       //使用北斗卫星数,MAX=12
	uint8_t	sv[72];		  //卫星号
	uint8_t elv[72];      //卫星的仰角
	uint8_t	cno[72];      //卫星的载噪比
	uint16_t az[72];      //卫星的方位角
}GPSGSAData;
int getGPSCoordinates_lib(GPS_Coordinates *GpsData);
int GetGSV_lib(GPSGSAData *GpsGSVData);
int gpsPowerUp_lib(void);
int gpsClose_lib(void);
int gpsQueryCrtState_lib(void);
int gpsQueryCrtState(void);
int gpsClose(void);
char *gps_strptime(const char *buf, const char *fmt, struct tm *tm);
void gnss_init(void);
int gpsPowerUp(void);
int gnss_poweron(void);
int gnss_poweroff(void);
#endif