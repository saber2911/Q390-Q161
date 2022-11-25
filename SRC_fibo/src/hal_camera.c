
/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_camera.c
** Description:		摄像头相关接口
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


uint8 g_ui8CameraPreviewEn = 0;//控制是否开启camera预览功能，如果=1开启，则全屏显示摄像头，不刷新屏幕其他内容
#define CAMERASCANCODE_LEN			1024

struct _CameraScan_STR g_stCameraScan; 

#define RANGE_LIMIT(x)	(x > 255 ? 255 : (x < 0 ? 0 : x ))
	
int g_iCamera_exist=0; //摄像头是否存在标志 0-无  1-有

barCodeEx_t sweep_codetype[12] = {
	{{"EAN13"}, 1},	//OK
	{{"EAN8"}, 1},	//OK
	{{"UPCA"}, 1},	//OK
	{{"UPCE"}, 1},	//OK
	{{"C128"}, 1},	//OK
	{{"C39"}, 1},	//OK
	{{"ITF25"}, 1},
	{{"CBAR"}, 1},
	{{"C93"}, 1},	//OK
	{{"PDF417"}, 1},//OK
	{{"QR"}, 1},	//OK
	{{"DM"}, 1},	//OK 
};



static void prvcamctx_init(camastae_t *st)
{
    memset(&st->CamDev, 0, sizeof(CAM_DEV_T));
    memset(&st->lcddev, 0, sizeof(lcdSpec_t));
    st->camTask = NULL;
    st->Decodestat = 0;
    st->Openstat = false;
    st->MemoryState = true;
    st->gCamPowerOnFlag = false;
    st->issnapshot = false;
    st->height = 0;
    st->width = 0;
    st->sweepsize=0;
    st->times=0;
    /*
       if (NULL == g_CamMutex)
       {
       g_CamMutex = osiMutexCreate();
       }
       */
}


/*
*Function:		hal_camInit
*Description:	初始化Camera
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_camInit(void)
{
	fibo_gpio_mode_set(CAM2V8EN_GPIO, GpioFunction1);
	fibo_gpio_cfg(CAM2V8EN_GPIO, GpioCfgOut);
	fibo_gpio_set(CAM2V8EN_GPIO, FALSE);

	fibo_gpio_mode_set(CAMLEDEN_GPIO, GpioFunction3);
	fibo_SetPwlLevel(0, 0);


	prvcamctx_init(&g_stCameraScan.camastae);

}



//YUV422_UYVY图像格式转RGB565
void YUV422toRGB565(const void *inBuff, void *outBuff)
{
	int i=0;
	int index=0;
	int rows = 0;
	int clos = 0;
	int u_value, v_value, y_value1 , y_value2, r, g, b;
	uint8 *yuv_buf;
	uint16 *rbg_buf;

	yuv_buf = (uint8 *)inBuff;
	rbg_buf = (uint16 *)outBuff;

	u_value = 0;
	v_value = 0;
	y_value1 = 0;
	y_value2 = 0;


	for (rows = 0; rows < 240   ; rows++)
	{
		for (clos = 0; clos < 320; clos++)
		{
			y_value1 = yuv_buf[index+1] & 0xff;			 				 // 第一个byte 的低四位 y1
			u_value = (yuv_buf[index]) & 0xff ;    	 					// 第二个byte 的高四位 u
			y_value2 = yuv_buf[index+3] & 0xff;							 // 第一个byte 的低四位 y2
			v_value = (yuv_buf[index+2]) & 0xff;						 // 第二个byte 的高四位 v

			r =  RANGE_LIMIT(y_value1 + (1.370705 * (v_value-128)));	
			g =  RANGE_LIMIT(y_value1 - (0.698001 * (v_value-128)) - (0.337633 * (u_value-128)));
			b =  RANGE_LIMIT(y_value1 + (1.732446 * (u_value-128)));

			
			rbg_buf[i++] = ( ( ( r  << 8 ) & 0xf800 ) | ( ( g << 3 ) & 0x7e0 ) | ( (b >> 3) & 0x1f ) );
			// //摄像头方向上下颠倒  储存顺序倒置 画面正常 
			// rbg_buf[(240-(rows+1))*320+clos] = ( ( ( r  << 8 ) & 0xf800 ) | ( ( g << 3 ) & 0x7e0 ) | ( (b >> 3) & 0x1f ) );
			index = index+4;
		}
	}
}

/*
*Function:		hal_camScanPreviewByScr
*Description:	摄像头预览
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功;other-失败
*Others:
*/
int hal_camScanPreviewByScr(void)
{
	lcdDisplay_t startPoint;
	lcdFrameBuffer_t dataBufferWin;
	uint16 *temp;
	temp = malloc (240 * 320 * 2 + 1);
	if (temp == NULL)
	{
		sysLOG(BASE_LOG_LEVEL_1, "<ERR> malloc failed \r\n");
		return ;
	}
	
	YUV422toRGB565(g_stCameraScan.pCamPreviewDataBuffer,temp);
	startPoint.x = 0;
	startPoint.y = 0;
	startPoint.height = g_stLcdConfig.COLORLCD_PIXHIGH;
	startPoint.width = g_stLcdConfig.COLORLCD_PIXWIDTH;
	dataBufferWin.buffer=g_stCameraScan.pCamPreviewDataBuffer;
	cus_export_api->fibo_genspi_lcd_send_buff(&startPoint, temp);
	free(temp);

	return 0;
}


/*
*Function:		hal_camOpen
*Description:	初始化扫码头模块
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-成功；<0-失败
*Others:
*/
int hal_camOpen(void)
{
	int iRet = -1;
	int i = 0;

	if(g_stCameraScan.state == 1)
	{
		return CAMERA_OK;
	}
	
	fibo_gpio_set(CAM2V8EN_GPIO, TRUE);
	
	fibo_SetPwlLevel(0, 0xFF);

	sweep_codetype[2].codeValue = 1;
	sweep_codetype[9].codeValue = 1;
	iRet = fibo_set_bar_code(sweep_codetype, 12);
	sysLOG(BASE_LOG_LEVEL_1, "fibo_set_bar_code iRet=%d\r\n", iRet);
	
	g_stCameraScan.Databuf = NULL;
	g_stCameraScan.ScanCodebuffP = NULL;
	
	g_stCameraScan.ScanCodebuffP = malloc(CAMERASCANCODE_LEN+1);
	if(g_stCameraScan.ScanCodebuffP == NULL)
	{
		iRet = CAMERA_ERR_NOT_OPEN;
		sysLOG(BASE_LOG_LEVEL_1, "<ERR> malloc for ScanCodebuffP failed\r\n");
		goto EXIT;
	}
	g_stCameraScan.height = 480;
	g_stCameraScan.width = 640;

	if(((g_stCameraScan.height == 60) && (g_stCameraScan.width == 80)) ||
		((g_stCameraScan.height == 120) && (g_stCameraScan.width == 160)) ||
		((g_stCameraScan.height == 240) && (g_stCameraScan.width == 320)) ||
		((g_stCameraScan.height == 480) && (g_stCameraScan.width == 640)))
	{
        if(!g_stCameraScan.camastae.Openstat)
        {
            prvcamctx_init(&g_stCameraScan.camastae);

            g_stCameraScan.camastae.sweepsize = g_stCameraScan.height * g_stCameraScan.width;
            g_stCameraScan.Databuf = (uint8_t *)malloc(g_stCameraScan.camastae.sweepsize + 32);
			if(g_stCameraScan.Databuf == NULL)
            {
            
				sysLOG(BASE_LOG_LEVEL_1, "<ERR> malloc for sweep code buf failed\r\n");
                iRet = CAMERA_ERR_NOT_OPEN;
				goto EXIT;
            }
			
			i = 0;
			do{
				iRet = fibo_camera_init();
				if(iRet == 0)
				{
					sysLOG(BASE_LOG_LEVEL_1, "<SUCC> Camera init succ, iRet=%d\r\n", iRet);
					break;
				}					
				else
	            {
	            	i++;
					if(i>3)
					{
						sysLOG(BASE_LOG_LEVEL_1, "<ERR> Camera init fail, i=%d iRet=%d\r\n", i, iRet);					
						iRet = CAMERA_ERR_NOT_OPEN;
						goto EXIT;
					}
	            }
				sysDelayMs(10);
				
			}while(1);
				
			iRet = fibo_camera_GetSensorInfo(&g_stCameraScan.camastae.CamDev);
			sysLOG(BASE_LOG_LEVEL_1, "fibo_camera_GetSensorInfo: %d, img_height:%d, img_width:%d, nPixcels:%d\r\n", \
				iRet, g_stCameraScan.camastae.CamDev.img_height, g_stCameraScan.camastae.CamDev.img_width, g_stCameraScan.camastae.CamDev.nPixcels);

            if(!g_stCameraScan.camastae.gCamPowerOnFlag)
            {
                //prvLcdInit();
                g_stCameraScan.camastae.height = g_stCameraScan.height;
                g_stCameraScan.camastae.width = g_stCameraScan.width;
                g_stCameraScan.camastae.times = g_stCameraScan.camastae.CamDev.img_width/g_stCameraScan.width;
                g_stCameraScan.camastae.gCamPowerOnFlag = true;
                g_stCameraScan.camastae.Openstat = true;
				
				sysLOG(BASE_LOG_LEVEL_1, "sweepCode:open height=%d,width=%d,times=%d,ulSweepSize=%d.\r\n", \
					g_stCameraScan.height,g_stCameraScan.width,g_stCameraScan.camastae.times,g_stCameraScan.camastae.sweepsize);
            }
			else

			{
				iRet = CAMERA_ERR_NOT_OPEN;
				sysLOG(BASE_LOG_LEVEL_1, "<ERR> g_stCameraScan.camastae.gCamPowerOnFlag=%d\r\n", g_stCameraScan.camastae.gCamPowerOnFlag);
				goto EXIT;
			}
        }
		else

		{
			sysLOG(BASE_LOG_LEVEL_1, "g_stCameraScan.camastae.Openstat=%d\r\n", g_stCameraScan.camastae.Openstat);
		}
    }

	iRet = fibo_camera_StartPreview();
	if(iRet < 0)
	{
		
		sysLOG(BASE_LOG_LEVEL_1, "<ERR> fibo_camera_StartPreview failed iRet=%d\r\n", iRet);
		iRet = CAMERA_ERR_NOT_OPEN;
		goto EXIT;
	}

	
	g_stCameraScan.state = 1;
	iRet = CAMERA_OK;
	
EXIT:

	if(iRet < 0)
	{
		if(g_stCameraScan.Databuf != NULL)
		{
			memset(g_stCameraScan.Databuf, 0, g_stCameraScan.camastae.sweepsize + 32);
			free(g_stCameraScan.Databuf);
			g_stCameraScan.Databuf = NULL;
		}
		if(g_stCameraScan.ScanCodebuffP != NULL)
		{
			memset(g_stCameraScan.ScanCodebuffP, 0, CAMERASCANCODE_LEN+1);
			free(g_stCameraScan.ScanCodebuffP);
			g_stCameraScan.ScanCodebuffP = NULL;
		}
	}
	return iRet;
	
}


/*
*Function:		hal_camScanStart
*Description:	启动扫描
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_camScanStart(void)
{
	

}


/*
*Function:		hal_camScanCheck
*Description:	判断扫描是否完成
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-未完成；1-完成
*Others:
*/
int hal_camScanCheck(void)
{
	sysLOG(BASE_LOG_LEVEL_1, " fibo_camera_GetPreviewBuf \r\n");

	g_stCameraScan.Scanstate = fibo_camera_GetPreviewBuf(&g_stCameraScan.pCamPreviewDataBuffer);
	if(g_stCameraScan.Scanstate == 0)
	{
		return 1;
	}
	else

	{
		return 0;
	}
}


/*
*Function:		hal_camScanRead
*Description:	获取扫码数据
*Input:			size-接收缓存大小
*Output:		*buf-接收缓存，不能为空
*Hardware:
*Return:		>0:成功返回实际读到的条码数据长度，<0-失败
*Others:
*/
int hal_camScanRead(unsigned char *buf,unsigned int size)
{
	int iRet = -1;
	uint32 scanlen = 0;
	int scantype;
	
	if(g_stCameraScan.state == 0)
	{
		return CAMERA_ERR_NOT_OPEN;
	}
	if(g_stCameraScan.ScanCodebuffP == NULL)
	{
		return CAMERA_ERR_MEMORY;
	}
	memset(g_stCameraScan.ScanCodebuffP, 0, CAMERASCANCODE_LEN+1);;
	iRet = fibo_sweep_code(&g_stCameraScan.camastae, g_stCameraScan.pCamPreviewDataBuffer, g_stCameraScan.Databuf, g_stCameraScan.ScanCodebuffP, &scanlen, &scantype);
	if(iRet == TRUE)
	{
		sysLOG(BASE_LOG_LEVEL_1, "<SUCC> fibo_sweep_code: %d, scanlen:%d, ScanCodebuffP:%s\r\n", \
					iRet, scanlen, g_stCameraScan.ScanCodebuffP);
		if(size < scanlen)
		{
			memcpy(buf, g_stCameraScan.ScanCodebuffP, size);
			iRet = CAMERA_ERR_DATA_MISSING;
			
		}
		else

		{
			memcpy(buf, g_stCameraScan.ScanCodebuffP, scanlen);
			iRet = scanlen;
		}
	}
	else


	{
		iRet = CAMERA_ERR_NO_DATA;
		//sysLOG(BASE_LOG_LEVEL_1, "<WARN> fibo_sweep_code: %d\r\n", iRet);		
	}
	
	memset(g_stCameraScan.pCamPreviewDataBuffer, 0, g_stCameraScan.camastae.CamDev.nPixcels);
    fibo_camera_PrevNotify((uint16_t *)g_stCameraScan.pCamPreviewDataBuffer);

	
	return iRet;
}


/*
*Function:		hal_camClose
*Description:	关闭扫码模块(模块下电，关闭通信口)
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_camClose(void)
{
	int iRet = -1;
	iRet = fibo_camera_StopPreview();
	sysLOG(BASE_LOG_LEVEL_1, "fibo_camera_StopPreview, iRet=%d\r\n", iRet);
	iRet = fibo_camera_deinit();
	sysLOG(BASE_LOG_LEVEL_1, "fibo_camera_deinit, iRet=%d\r\n", iRet);

	fibo_gpio_set(CAM2V8EN_GPIO, FALSE);
	
	fibo_SetPwlLevel(0, 0);

	prvcamctx_init(&g_stCameraScan.camastae);

	if(g_stCameraScan.Databuf != NULL)
	{
		memset(g_stCameraScan.Databuf, 0, g_stCameraScan.camastae.sweepsize + 32);
		free(g_stCameraScan.Databuf);
		g_stCameraScan.Databuf = NULL;
	}
	if(g_stCameraScan.ScanCodebuffP != NULL)
	{
		memset(g_stCameraScan.ScanCodebuffP, 0, CAMERASCANCODE_LEN+1);
		free(g_stCameraScan.ScanCodebuffP);
		g_stCameraScan.ScanCodebuffP = NULL;
	}
	g_stCameraScan.state = 0;
	
	if(g_ui8LcdType == 1)//关闭摄像头退出预览后刷新下图标
	{
		hal_clcdIconRefresh();
	}
}



/*******************************TEST************************************************/
#if MAINTEST_FLAG
static void hal_camThreadTest(void *param)
{
	int iRet;
	uint8 scanbufftmp[1024];
	int numtmp;
	sysLOG(BASE_LOG_LEVEL_1, "application thread enter, param 0x%x\r\n", param);

	memset(scanbufftmp, 0, sizeof(scanbufftmp));
	
	//hal_camOpen();
	//while(1){};
#if 1
	while(1)
	{

		do{

			sysDelayMs(200);

			
		}while(hal_keypadGetKey() != KEY0);
		
		
		if(g_ui8LcdType == 0)
		{
			hal_scrCls();
			hal_scrTextOut(0, 24, "scan START!\r\n");
		}
		else


		{
			if(FONTFS == hal_fontGetFontType())
				hal_scrSelectFont(&g_stSingleFont6X12, &g_stGBMultiFont12X12);
//			else
//				hal_scrSelectFont(&g_stSingleFont6X12, &g_stUniMultiFont12X12);
			hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
			hal_scrClrLine(1, 2);
			hal_scrPrintf("scan START!");
		}		
		sysDelayMs(500);
		g_ui8CameraPreviewEn = 1;
		iRet = hal_camOpen();
		sysLOG(BASE_LOG_LEVEL_1, "hal_camOpen: %d\r\n", iRet);
		hal_camScanStart();
		hal_scrCls();
		numtmp = 1000;
		if(iRet == 0)
		{
			do
			{
				numtmp --;
				if(hal_keypadGetKey() == KEY0)
				{
					break;
				}
				iRet = hal_camScanCheck();
				if(iRet == 1)
				{
					hal_camScanPreviewByScr();
					memset(scanbufftmp, 0, sizeof(scanbufftmp));
					iRet = hal_camScanRead(scanbufftmp, sizeof(scanbufftmp));
					sysLOG(BASE_LOG_LEVEL_1, "hal_camScanRead, iRet:%d, scanbufftmp:%s\r\n", iRet, scanbufftmp);
					if(iRet > 0)
					{
						g_ui8CameraPreviewEn = 0;
						if(g_ui8LcdType == 0)
						{
							hal_scrTextOut(0, 24, "scan SUCC, scanbufftmp:\r\n");
							hal_scrTextOut(0, 36, scanbufftmp);
						}
						else

						{
							hal_scrClsArea(0, 24,g_stLcdConfig.COLORLCD_PIXWIDTH-1,g_stLcdConfig.COLORLCD_PIXHIGH-1);
							//hal_clcdRefresh();
							if(FONTFS == hal_fontGetFontType())
								hal_scrSelectFont(&g_stSingleFont6X12, &g_stGBMultiFont12X12);
//							else
//								hal_scrSelectFont(&g_stSingleFont6X12, &g_stUniMultiFont12X12);
							hal_clcdSetAttrib(LCD_ATTR_FRONT_COLOR, BLACK);
							hal_scrPrint(64, 1, 0x80, "scanbufftmp:%s", scanbufftmp);
						}
						sysDelayMs(2000);

						break;
					}
				}
				else

				{
					sysLOG(BASE_LOG_LEVEL_1, "hal_camScanRead, iRet:%d\r\n", iRet);
				}
				sysDelayMs(5);
			}while(numtmp > 0);
			hal_camClose();
			g_ui8CameraPreviewEn =0;
			if(g_ui8LcdType == 1)
			{
				hal_scrClsArea(0, 24,g_stLcdConfig.COLORLCD_PIXWIDTH-1,g_stLcdConfig.COLORLCD_PIXHIGH-1);
			}
		}
		
		sysDelayMs(2000);
		
	}
#endif
	sysLOG(BASE_LOG_LEVEL_1, "sweep code osiThreadExit\r\n");
    //fibo_thread_delete();
}

int hal_camSweepTest(void)
{
	
     fibo_thread_create(hal_camThreadTest, "sweep code", 1024*64, NULL, OSI_PRIORITY_ABOVE_NORMAL);
          
}

#endif




