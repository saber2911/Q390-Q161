#include "LC610N_api_gps.h"
unsigned char gpsPowerUpFlag = 0; 

UINT32 gps_task = NULL; 

int8 * GnssPowerStatusCheck = 	"AT+GTGPSPOWER?\r\n";
int8 * GnssPowerOnSet = 		"AT+GTGPSPOWER=1\r\n";
int8 * GnssPowerOffSet = 		"AT+GTGPSPOWER=0\r\n";
int8 * GnssInfoCheck = 			"AT+GTGPS?\r\n";

int g_iTypeofGPS=0; //1-GPS在AP端 ，2-GPS在SE端，3-无
static uint8     g_Gps_Power_Status = 0; 


/*
*@Brief:		查询GPS当前状态
*@Param IN:		无
*@Param OUT:	无 
*@Return:		0:完成上电; <0:没有完成上电
*/
int gpsQueryCrtState(void)
{
	if(g_iTypeofGPS == 0) return GPS_ERR_NO_GPS;
	int iRet = 0;
	char *retP = NULL;
	unsigned long long uTime;

	iRet = fibo_sem_try_wait(Sem_AT_signal, 5000);
	if(FALSE == iRet)
	{
		sysLOG(AT_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR>fibo_sem_try_wait,iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return -3;

	}
	
	CBuffFormat(&g_stATCbBuffStruct);
	
	g_ui8ATFlag = 2;
	uTime = hal_sysGetTickms()+5000;
		
	iRet = hal_atSend(GnssPowerStatusCheck, strlen(GnssPowerStatusCheck));

	while(1)
	{
		retP = MyStrStr(g_stATCbBuffStruct.buff, "OK", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
		if(retP != NULL)
		{
			break;
		}
		if(hal_sysGetTickms() > uTime)
		{
			break;
		}
		sysDelayMs(100);
	}
	
	g_ui8ATFlag = 0;
	
	if(retP == NULL)//未接收到
	{
		iRet =  GPS_ERR_TIME_OUT;
		goto exit;
	}
	
	retP = MyStrStr(g_stATCbBuffStruct.buff, "1", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
	if(iRet == NULL)
	{
		iRet = GPS_ERR_NO_ELT;
		goto exit;
	}
	iRet = GPS_OK;

exit:

	fibo_sem_signal(Sem_AT_signal);
	return iRet;
}


static void gnss_thread(void *param)
{
    sysLOG(API_LOG_LEVEL_4,"gnss thread enter, param 0x%x", param);
    int iRet = 0;
	char *retP = NULL;
	unsigned long long uTime;
	
	uTime = hal_sysGetTickms()+10000;

	while(1)
	{
		retP = gpsQueryCrtState();
		if(retP == GPS_OK)
		{
			break;
		}
		if(hal_sysGetTickms() > uTime)
		{
			goto exit;
		}
		sysDelayMs(100);
	}
	
    while(g_Gps_Power_Status)
    {   

		iRet = fibo_sem_try_wait(Sem_AT_signal, 5000);
		if(FALSE == iRet)
		{
			sysLOG(AT_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR>fibo_sem_try_wait,iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
			sysDelayMs(2000);
			continue;
		}
		
		CBuffFormat(&g_stATCbBuffStruct);
		
		g_ui8ATFlag = 2;
		uTime = hal_sysGetTickms()+5000;
	
		iRet = hal_atSend(GnssInfoCheck, strlen(GnssInfoCheck));
        while(1)
		{
			retP = MyStrStr(g_stATCbBuffStruct.buff, "OK", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
			if(retP != NULL)
			{
				break;
			}
			if(hal_sysGetTickms() > uTime)
			{
				sysLOG(API_LOG_LEVEL_4,"GPS check OK time on");
				break;
			}
			sysDelayMs(100);
		}
		if(retP != NULL){
			#if 0
			retP = MyStrStr(g_stATCbBuffStruct.buff, "$GNRMC", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
			if(retP != NULL){
				retP = MyStrStr(retP, ",", retP, g_stATCbBuffStruct.write_P);
				if(retP != NULL){
					DR_LOG_D("%s",retP);
					retP = MyStrStr(retP+1, ",", retP+1, g_stATCbBuffStruct.write_P);
					if(retP != NULL){
						DR_LOG_D("%s",retP);
						if(retP[1]=='A'){
							DR_LOG_D("GPS check OK");
							fibo_gpio_set(LED_STA_PIN, TRUE);
							fibo_gpio_set(LED_CHG_PIN, TRUE);
						}
					}
				}
			}
			#else
			char * buff = NULL;  //缓冲区中存协议数据
			retP = MyStrStr(g_stATCbBuffStruct.buff, "$GNRMC", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
			if(retP != NULL){
				sysLOG(API_LOG_LEVEL_4,"%s",retP);
				uint32 buff_len = strlen(retP) - 2;//消除OK
				sysLOG(API_LOG_LEVEL_4,"buff_len:%d",buff_len);
				int gpscount = 0;
				unsigned int count = 0;  //定义存储帧数据的个数
				//init_gps_message(get_gps_message()); //初始化库内部定义的gps_message
				for(int i=0; i<buff_len; i++)
				{			
					if(get_nmea_frame(retP[i], nmea_buff, &gpscount)) //按字符解析，如果得到一帧数据，则为真
					{
//						DR_LOG_D("nmea_buff:%s, gpscount=%d\n", nmea_buff, gpscount);
						nema_message_parse(nmea_buff, get_gps_message(), gpscount); //对一帧数据进行解析
						//printf_GPS_message(get_gps_message());
						//sysDelayMs(10);
					}
				}
			}
			#endif
    	}

		fibo_sem_signal(Sem_AT_signal);
		sysDelayMs(2000);
    }
 exit:
 gpsClose();
 fibo_thread_delete();
}


void gnss_init(void)
{
	init_gps_message(get_gps_message()); //初始化库内部定义的gps_message		
}

/*
*@Brief:		GPS上电
*@Param IN:		无
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int gpsPowerUp(void)
{
	if(g_iTypeofGPS == 0) return GPS_ERR_NO_GPS;
	int iRet = GPS_OK;
	gnss_init();	
	gnss_poweron();
	iRet = fibo_thread_create_ex(gnss_thread, "GNSS_thread", 1024*16, NULL, OSI_PRIORITY_NORMAL, &gps_task);
	sysLOG(API_LOG_LEVEL_2,"created GNSS task iRet:%d\r\n",iRet);
	if(iRet < 0){
		sysLOG(API_LOG_LEVEL_2,"created GNSS task failed\r\n");
		gnss_poweroff();
		iRet = GPS_ERR_TASK_CREAT;
	}else{
		sysLOG(API_LOG_LEVEL_2,"created GNSS task success\r\n");
	}

	if(iRet == GPS_OK){
		g_Gps_Power_Status = 1;
		sysLOG(API_LOG_LEVEL_2,"gpsPowerUp ok\r\n");
	}
	return iRet;
}

/*
*@Brief:		GPS上电
*@Param IN:		无
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int gpsPowerUp_lib(void)
{
	if (g_iTypeofGPS == 1)
	{
		int iRet = gpsPowerUp();
		return iRet;
	}
	else if (g_iTypeofGPS == 2)
	{
		int iRet = RET_RF_ERR_PARAM;
		int iCmdLen = 6;
		int output_len = 0;
		unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x30, 0x02, iCmdLen-6, (iCmdLen -6) >> 8};
		unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
		memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
		memset(caShow, 0, sizeof(caShow));
		HexToStr(ucCmd, iCmdLen, caShow);
		sysLOG(API_LOG_LEVEL_5, "  ucCmd = %s\r\n", caShow);
		fibo_free(caShow);
	#endif

		Frame frm,retfrm;
		
		iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		fibo_free(ucCmd);
		if(iRet < 0) {
			goto RET_END;
		}
		iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		gpsPowerUpFlag = 1;
		fibo_free(frm.data);
		if(iRet <0) {
			goto RET_END;
		}
		iRet=retfrm.data[2]<<8 | retfrm.data[3];
		if(0x9000 == iRet) {
			iRet = RET_RF_OK;
		}
		else if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
		else
		{
			iRet = -iRet;
		}
		fibo_free(retfrm.data);
	RET_END:
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		return iRet;
	}
	
	
}

/*
*@Brief:		GPS下电
*@Param IN:		无
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int gpsClose(void)
{
	if(g_iTypeofGPS == 0) return GPS_ERR_NO_GPS;
	int iRet = GPS_OK;
	fibo_specify_thread_delete(gps_task);
	iRet = gnss_poweroff();
	if(iRet == GPS_OK) g_Gps_Power_Status = 0;
	return iRet;
}

/*
*@Brief:		GPS下电
*@Param IN:		无
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int gpsClose_lib(void)
{
	if (g_iTypeofGPS == 1)
	{
		int iRet = gpsClose();
		return iRet;
	}
	else if (g_iTypeofGPS == 2)
	{
		int iRet = RET_RF_ERR_PARAM;
		int iCmdLen = 6;
		int output_len = 0;
		unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x30, 0x03, iCmdLen-6, (iCmdLen -6) >> 8};
		unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
		memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
		memset(caShow, 0, sizeof(caShow));
		HexToStr(ucCmd, iCmdLen, caShow);
		sysLOG(API_LOG_LEVEL_5, "  ucCmd = %s\r\n", caShow);
		fibo_free(caShow);
	#endif

		Frame frm,retfrm;
		
		iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
		fibo_free(ucCmd);
		if(iRet < 0) {
			goto RET_END;
		}
		iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
		fibo_free(frm.data);
		if(iRet <0) {
			goto RET_END;
		}
		iRet=retfrm.data[2]<<8 | retfrm.data[3];
		if(0x9000 == iRet) {
			iRet = RET_RF_OK;
			gpsPowerUpFlag = 0;
		}
		else if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
		else
		{
			iRet = -iRet;
		}
		fibo_free(retfrm.data);
	RET_END:
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		return iRet;
	}
	
	
}

/*
*@Brief:		查询GPS当前状态
*@Param IN:		无
*@Param OUT:	无 
*@Return:		0:完成上电; <0:没有完成上电
*/
int gpsQueryCrtState_lib(void)
{
	if (g_iTypeofGPS == 1)
	{
		int iRet = gpsQueryCrtState();
		sysLOG(API_LOG_LEVEL_2, " GPS status iRet:%d" ,iRet);
		return iRet;
	}
	else if (g_iTypeofGPS == 2)
	{
		int iRet = RET_RF_ERR_PARAM;
		int iCmdLen = 6;
		int output_len = 0;
		unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x30, 0x04, iCmdLen-6, (iCmdLen -6) >> 8};
		unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
		memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
		memset(caShow, 0, sizeof(caShow));
		HexToStr(ucCmd, iCmdLen, caShow);
		sysLOG(API_LOG_LEVEL_5, "  ucCmd = %s\r\n", caShow);
		fibo_free(caShow);
	#endif

		Frame frm,retfrm;
		
		iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
		fibo_free(ucCmd);
		if(iRet < 0) {
			goto RET_END;
		}
		iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
		fibo_free(frm.data);
		if(iRet <0) {
			goto RET_END;
		}
		iRet=retfrm.data[2]<<8 | retfrm.data[3];
		if(0x9000 == iRet) {
			iRet = RET_RF_OK;
		}
		else if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
		else
		{
			iRet = -iRet;
		}
		fibo_free(retfrm.data);
	RET_END:
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		return iRet;
	}
	
	
}

/*
*@Brief:		获取GPS经度，纬度 ，南/北纬，东/西经
*@Param IN:		经纬度坐标结构体GpsData->Lon_area：东/西经 GpsData->Lon：经度 GpsData->Lat_area：南北纬 GpsData->Lat：纬度
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int getGPSCoordinates(GPS_Coordinates *GpsData)
{
	if(g_iTypeofGPS == 0) return GPS_ERR_NO_GPS;
	int iRet = GPS_OK;
	gps_message * message = get_gps_message();
	if(message->data_val == false)
	{
		iRet = GPS_ERR_NO_DATA;
	}else{
		GpsData->Lon = message->longitude;
		GpsData->Lat = message->latitude;
		GpsData->Lon_area = message->long_direc;
		GpsData->Lat_area = message->lat_direc;
	}
	
	return iRet;
}

/*
*@Brief:		获取GPS经度，纬度 ，南/北纬，东/西经
*@Param IN:		经纬度坐标结构体GpsData->Lon_area：东/西经 GpsData->Lon：经度 GpsData->Lat_area：南北纬 GpsData->Lat：纬度
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int getGPSCoordinates_lib(GPS_Coordinates *GpsData)
{
	if (g_iTypeofGPS == 1)
	{
		if(g_iTypeofGPS == 0) return GPS_ERR_NO_GPS;
	int iRet = GPS_OK;
	gps_message * message = get_gps_message();
	if(message->data_val == false)
	{
		iRet = GPS_ERR_NO_DATA;
	}else{
		GpsData->Lon = message->longitude;
		GpsData->Lat = message->latitude;
		GpsData->Lon_area = message->long_direc;
		GpsData->Lat_area = message->lat_direc;
	}
	
	return iRet;
	}
	else if (g_iTypeofGPS == 2)
	{
		int iRet = RET_RF_ERR_PARAM;
		int iCmdLen = 6;
		int output_len = 0;
		unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x30, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
		unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
		memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

	#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
		memset(caShow, 0, sizeof(caShow));
		HexToStr(ucCmd, iCmdLen, caShow);
		sysLOG(API_LOG_LEVEL_5, "  ucCmd = %s\r\n", caShow);
		fibo_free(caShow);
	#endif

		Frame frm,retfrm;
		
		iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
		fibo_free(ucCmd);
		if(iRet < 0) {
			goto RET_END;
		}
		iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
		fibo_free(frm.data);
		if(iRet <0) {
			goto RET_END;
		}
		iRet=retfrm.data[2]<<8 | retfrm.data[3];
		if(0x9000 == iRet) {
			output_len = retfrm.data[5]<<8 | retfrm.data[4];
			GpsData->Lon_area = retfrm.data[6];
			GpsData->Lon =  (retfrm.data[10]<<24);
			GpsData->Lon += (retfrm.data[9]<<16);
			GpsData->Lon += (retfrm.data[8]<<8);
			GpsData->Lon += retfrm.data[7];
			GpsData->Lon /= 1000000; 
			GpsData->Lat_area = retfrm.data[11];
			GpsData->Lat =  (retfrm.data[15]<<24);
			GpsData->Lat += (retfrm.data[14]<<16);
			GpsData->Lat += (retfrm.data[13]<<8);
			GpsData->Lat += retfrm.data[12];
			GpsData->Lat /= 1000000;

			iRet = RET_RF_OK;
		}
		else if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
		else
		{
			iRet = -iRet;
		}
		fibo_free(retfrm.data);
	RET_END:
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		return iRet;
	}
	
	
}

int GetGSV_lib(GPSGSAData *GpsGSVData)
{
	if (g_iTypeofGPS == 1)
	{
		if(g_iTypeofGPS == 0) return GPS_ERR_NO_GPS;
		int iRet = GPS_OK;
		int gpgsv_num = get_gpgsv_num();
		int gbgsv_num = get_gbgsv_num();
		if(gpgsv_num = 0 && gpgsv_num == 0)
		{
			iRet = GPS_ERR_NO_DATA;
		}else{
			int i = 0;
			gps_gsv* gpgsv = get_gpgsv();
			gps_gsv* gbgsv = get_gbgsv();
			GpsGSVData->GPNoSv = gpgsv_num;
			GpsGSVData->GBNoSv = gbgsv_num;
			for(i = 0;i<gpgsv_num;i++){
				GpsGSVData->sv[i] = gpgsv->prn_number;
				GpsGSVData->elv[i] = gpgsv->elevation;
				GpsGSVData->az[i] = gpgsv->azimuth;
				GpsGSVData->cno[i] = gpgsv->db;
			}
			for(i = gpgsv_num;i<gpgsv_num+gbgsv_num;i++){
				GpsGSVData->sv[i] = gbgsv->prn_number;
				GpsGSVData->elv[i] = gbgsv->elevation;
				GpsGSVData->az[i] = gbgsv->azimuth;
				GpsGSVData->cno[i] = gbgsv->db;
			}
		}
		return iRet;
	}
	else if (g_iTypeofGPS == 2)
	{
		int iRet = RET_RF_ERR_PARAM;
		int iCmdLen = 6;
		int output_len = 0;
		unsigned char i = 0;
		unsigned char ucCmdHead[6] = {0x00, 0xe2, 0x30, 0x01, iCmdLen-6, (iCmdLen -6) >> 8};
		unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
		memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	#ifdef PRINT_API_CMD
		char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
		memset(caShow, 0, sizeof(caShow));
		HexToStr(ucCmd, iCmdLen, caShow);
		sysLOG(API_LOG_LEVEL_5, "  ucCmd = %s\r\n", caShow);
		fibo_free(caShow);
	#endif
		Frame frm,retfrm;
		iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
		fibo_free(ucCmd);
		if(iRet < 0) {
			goto RET_END;
		}
		iRet = transceiveFrame(frm, &retfrm, 1000);  //发送数据包并接收SE返回数据包
		fibo_free(frm.data);
		if(iRet <0) {
			goto RET_END;
		}
		iRet=retfrm.data[2]<<8 | retfrm.data[3];
		if(0x9000 == iRet) {
			memset(GpsGSVData, 0x00, sizeof(GPSGSAData));
			GpsGSVData->GPNoSv = retfrm.data[6];
			GpsGSVData->GBNoSv = retfrm.data[7];
			for(i=0; i<GpsGSVData->GPNoSv+GpsGSVData->GBNoSv; i++)
			{
				GpsGSVData->sv[i] = retfrm.data[i*5+8];
				GpsGSVData->elv[i] = retfrm.data[i*5+9];
				GpsGSVData->az[i] = retfrm.data[i*5+10];
				GpsGSVData->az[i] += (retfrm.data[i*5+11]<<8);
				GpsGSVData->cno[i] = retfrm.data[i*5+12];
			}
			iRet = RET_RF_OK;
		}
		else if(retfrm.length >= 10)
		{
			iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
		}
		else
		{
			iRet = -iRet;
		}
		fibo_free(retfrm.data);
	RET_END:
		sysLOG(API_LOG_LEVEL_4, "  RET_END,iRet = %d\r\n", iRet);
		return iRet;
	}
	
	
}
#if 0
void GPS_test()
{
	int iRet = 0;

	GPS_Coordinates s_GPSData;
	iRet = getGPSCoordinates_lib(&s_GPSData);
	if(iRet == 0)
	{
		sysLOG(API_LOG_LEVEL_2, "  s_GPSData.Lon = %d s_GPSData.Lon_area = %f\r\n", s_GPSData.Lon, s_GPSData.Lon_area);
		sysLOG(API_LOG_LEVEL_2, "  s_GPSData.Lat = %d s_GPSData.Lat_area = %f\r\n", s_GPSData.Lat, s_GPSData.Lat_area);
	}
	else
	{
		sysLOG(API_LOG_LEVEL_2, "  GPS error! iRet= %d\r\n", iRet);
	}
	
}
#endif
int gnss_poweron(void)
{
	int iRet = 0;
	//char *retP = NULL;
	//unsigned long long uTime;

	iRet = fibo_sem_try_wait(Sem_AT_signal, 5000);
	if(FALSE == iRet)
	{
		sysLOG(AT_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR>fibo_sem_try_wait,iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return -3;
	}
	
	CBuffFormat(&g_stATCbBuffStruct);
	
	g_ui8ATFlag = 2;
	//uTime = DR_GetSystick_ms()+15000;
	
	hal_atSend(GnssPowerOnSet, strlen(GnssPowerOnSet));

	fibo_sem_signal(Sem_AT_signal);
	
	//为保证时效，不作操作成功校验，后续可通过是否上电成功接口查询
/*
	while(1)
	{
		retP = MyStrStr(g_stATCbBuffStruct.buff, "OK", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
		if(retP != NULL)
		{
			break;
		}
		if(DR_GetSystick_ms() > uTime)
		{
			break;
		}
		Sleep(100);
	}
*/
	g_ui8ATFlag = 0;
	
	/*if(retP == NULL)//未接收到
	{
		DR_LOG_D("gnss power on time out\r\n");
		return GPS_ERR_TIME_OUT;
	}else{
		DR_LOG_D("gnss power on ok\r\n");
	}
	
	return iRet;*/		
}

int gnss_poweroff(void)
{
	int iRet = 0;
	char *retP = NULL;
	unsigned long long uTime;

	iRet = fibo_sem_try_wait(Sem_AT_signal, 5000);
	if(FALSE == iRet)
	{
		sysLOG(AT_LOG_LEVEL_2, "[%s] -%s- Line=%d:<ERR>fibo_sem_try_wait,iRet=%d\r\n", filename(__FILE__), __FUNCTION__, __LINE__, iRet);
		return -3;
	}
	
	CBuffFormat(&g_stATCbBuffStruct);
	
	g_ui8ATFlag = 2;
	uTime = hal_sysGetTickms()+5000;
	
	iRet = hal_atSend(GnssPowerOffSet, strlen(GnssPowerOffSet));

	while(1)
	{
		retP = MyStrStr(g_stATCbBuffStruct.buff, "OK", g_stATCbBuffStruct.read_P, g_stATCbBuffStruct.write_P);
		if(retP != NULL)
		{
			break;
		}
		if(hal_sysGetTickms() > uTime)
		{
			break;
		}
		sysDelayMs(100);
	}
	
	g_ui8ATFlag = 0;
	
	if(retP == NULL)//未接收到
	{
		sysLOG(API_LOG_LEVEL_4,"gnss power off time out\r\n");
		iRet = GPS_ERR_TIME_OUT;
		goto exit;
	}
exit:

	fibo_sem_signal(Sem_AT_signal);
	return iRet;		
}
