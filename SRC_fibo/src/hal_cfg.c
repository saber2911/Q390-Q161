/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_cfg.c
** Description:		配置文件相关接口
**
** Version:	1.0, 渠忠磊,2022-02-24
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/

#include "comm.h"

#define PARAMCONFIG_JSON		"/app/ufs/paramcfg.json"

PDPCFG_STR g_stPdpCfg;

/*
*Function:		hal_cfgGetHwVerString
*Description:	读取硬件版本号
*Input:			*data:读取的内容,len:长度
*Output:		*outlen:读到的硬件版本号长度
*Hardware:
*Return:		硬件版本号的偏移量
*Others:
*/
int hal_cfgGetHwVerString(int8 *data, int len, uint8 *outlen)
{
	int iRet;
	int8 *readP = NULL;
	int8 *readendP = NULL;
	readP = MyStrStr(data, "MAIN_BOARD", 0, len);

	if(readP == NULL)
	{
		return -1;
	}
	iRet = readP - data + 12;
	
	readendP = MyStrStr(readP+12, "\"", 0, len-iRet);
	if(readendP == NULL)
	{
		return -2;
	}
	*outlen = readendP-(readP+12);
	
	return iRet;
}


/*
*Function:		hal_cfgReadCfgFile
*Description:	从文件中读取硬件版本号信息
*Input:			*cfgname:文件名
*Output:		NULL
*Hardware:
*Return:		<=0:失败，>0:读到的字节数
*Others:
*/
int hal_cfgReadCfgFile(int8 *cfgname)
{
	int iRet;
	int sizeofcfg;
	int8 *rP = NULL;
	int8 hwdata[32];
	uint8 hwlen = 0;
	
	sizeofcfg = hal_fileGetFileSize(cfgname);
	if(sizeofcfg <= 0)
	{
		return sizeofcfg;
	}
	
	rP = malloc(sizeofcfg+1);
	if(rP == NULL)
	{
		return -1;
	}
	
	memset(rP, 0, sizeofcfg+1);
	
	iRet = hal_fileReadPro(cfgname, 0, rP, sizeofcfg);
	if(iRet != sizeofcfg)
	{
		free(rP);
		return -2;
	}

	iRet = hal_cfgGetHwVerString(rP, sizeofcfg, &hwlen);
	if(iRet < 0)
	{
		free(rP);
		return iRet;
	}
	memset(hwdata, 0, sizeof(hwdata));
	memcpy(hwdata, rP+iRet, hwlen);
	free(rP);
	iRet = hal_nvWriteHwVersionString(hwdata);
	if(iRet < 0)
	{
		sysLOG(CFG_LOG_LEVEL_1, "<ERR>, hal_nvWriteHwVersionString, iRet:%d\r\n", iRet);
		return iRet;
	}
	return 4;
	
}


/*
*Function:		hal_cfgParCfgInit
*Description:	初始化PARAMCONFIG_JSON文件
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		0-succ; <0-failed
*Others:
*/
int hal_cfgParCfgInit(void)
{
	int iRet = -1;
	
	Vs_cJSON* root = NULL;

	iRet = fileExist_lib(PARAMCONFIG_JSON);
	if(1 == iRet)
		return 0;
	else
	{
		root = Vs_cJSON_CreateObject();
		Vs_cJSON_AddItemToObject(root, "comment", Vs_cJSON_CreateString( "This is the terminal configuration file, version V1.00"));
		
		// 打印控制台查看
		char *cPrint = Vs_cJSON_Print(root);
		char *cPrintUnformatted = Vs_cJSON_PrintUnformatted(root);

		iRet = hal_fileWritePro(PARAMCONFIG_JSON, 0, cPrint, strlen(cPrint));
		sysLOG(CFG_LOG_LEVEL_2, "cJSON_Print：\
		%s\
		", cPrint); 	// cJSON_Print：有做格式调整
		sysLOG(CFG_LOG_LEVEL_2, "cJSON_PrintUnformatted：\
		%s\
		", cPrintUnformatted);	// cJSON_PrintUnformatted：没有做格式调整
		// 返回的字符串指针需要自己释放
		free(cPrint);
		free(cPrintUnformatted);
		Vs_cJSON_Delete(root);
	}

	return 0;
}


/*
*Function:		hal_cfgWriteNtpEn
*Description:	设置pdp激活后是否做NTP对时的使能状态
*Input:			*value:0-不做对时,1-做NTP对时
*Output:		NULL
*Hardware:		NULL
*Return:		0-succ; <0-failed
*Others:
*/
int hal_cfgWriteNtpEn(int *value)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;

	if(*value < 0 || *value >1)
		return -3;
	
	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);

	
	Vs_cJSON *socket_node = Vs_cJSON_GetObjectItem(root, "socket");
    if(socket_node == 0)
	{
		
        // 定义对象
		Vs_cJSON *socket = Vs_cJSON_CreateObject();
		// 插入元素，对应值
		Vs_cJSON_AddItemToObject(socket, "ntpEn", Vs_cJSON_CreateNumber(*value));
		Vs_cJSON_AddItemToObject(root, "socket", socket);
    }
	else
	{
		Vs_cJSON *ntpEn = Vs_cJSON_GetObjectItem(socket_node, "ntpEn");
		if(ntpEn == 0)
		{
			Vs_cJSON_AddItemToObject(socket_node, "ntpEn", Vs_cJSON_CreateNumber(*value));
			Vs_cJSON_AddItemToObject(root, "socket", socket_node);

		}
		else
		{
			Vs_cJSON_ReplaceItemInObject(socket_node, "ntpEn", Vs_cJSON_CreateNumber(*value));
		}
	}

	// 打印控制台查看
	char *cPrint = Vs_cJSON_Print(root);

	fileRemove_lib(PARAMCONFIG_JSON);
	iRet = hal_fileWritePro(PARAMCONFIG_JSON, 0, cPrint, strlen(cPrint));
	sysLOG(CFG_LOG_LEVEL_2, "cJSON_Print：\
	%s\
	", cPrint);// cJSON_Print：有做格式调整
	// 返回的字符串指针需要自己释放
	free(cPrint);
	Vs_cJSON_Delete(root);

	if(NULL != rP)
		free(rP);

	return 0;
	
}


/*
*Function:		hal_cfgReadNtpEn
*Description:	读取pdp激活后是否做NTP对时的使能状态
*Input:			NULL
*Output:		*value:0-不做对时,1-做NTP对时
*Hardware:		NULL
*Return:		0-succ; <0-failed
*Others:
*/
int hal_cfgReadNtpEn(int *value)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;

	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);
	
	Vs_cJSON *socket_node = Vs_cJSON_GetObjectItem(root, "socket");
    if(socket_node == 0)
	{
		iRet = -4;
		goto exit;
    }
	else
	{
		Vs_cJSON *ntpEn_node = Vs_cJSON_GetObjectItem(socket_node, "ntpEn");
	    if(ntpEn_node == 0)
		{
			iRet = -5;
			goto exit;
	    }
		else
		{
			*value = ntpEn_node->valueint;
			iRet = 0;
		}
	}

exit:

	Vs_cJSON_Delete(root);
	
	if(NULL != rP)
		free(rP);

	return iRet;
}


/*
*Function:		hal_cfgWritePdpCfg
*Description:	写pdp拨号配置文件
*Input:			*pdpcfg-PDPCFG_STR指针, *apn-写入的apn指针; *username-写入的username指针; *password-写入的password指针
*Output:		NULL
*Hardware:		NULL
*Return:		>0:成功；其他:失败
*Others:
*/
int hal_cfgWritePdpCfg(PDPCFG_STR *pdpcfg, char *apn, char *username, char *password)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;

	if(apn != NULL){
		memset(pdpcfg->apn, 0, sizeof(pdpcfg->apn));
		memcpy(pdpcfg->apn, apn, strlen(apn));
	}
	else{
		memset(pdpcfg->apn, 0, sizeof(pdpcfg->apn));
		memcpy(pdpcfg->apn, "NULL", strlen("NULL"));
	}
	if(username != NULL){
		memset(pdpcfg->username, 0, sizeof(pdpcfg->username));
		memcpy(pdpcfg->username, username, strlen(username));
	}
	else{
		memset(pdpcfg->username, 0, sizeof(pdpcfg->username));
		memcpy(pdpcfg->username, "NULL", strlen("NULL"));
	}
	if(password != NULL){
		memset(pdpcfg->password, 0, sizeof(pdpcfg->password));
		memcpy(pdpcfg->password, password, strlen(password));
	}
	else{
		memset(pdpcfg->password, 0, sizeof(pdpcfg->password));
		memcpy(pdpcfg->password, "NULL", strlen("NULL"));
	}
	
	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);

	
	Vs_cJSON *network_node = Vs_cJSON_GetObjectItem(root, "network");
    if(network_node == 0)
	{
		
        // 定义对象
		Vs_cJSON *network = Vs_cJSON_CreateObject();
		// 插入元素，对应值
		Vs_cJSON_AddItemToObject(network, "apn", Vs_cJSON_CreateString(pdpcfg->apn));
		Vs_cJSON_AddItemToObject(network, "username", Vs_cJSON_CreateString(pdpcfg->username));
		Vs_cJSON_AddItemToObject(network, "password", Vs_cJSON_CreateString(pdpcfg->password));
		Vs_cJSON_AddItemToObject(root, "network", network);
    }
	else
	{
		Vs_cJSON *apn = Vs_cJSON_GetObjectItem(network_node, "apn");
		if(apn == 0)
		{
			Vs_cJSON_AddItemToObject(network_node, "apn", Vs_cJSON_CreateString(pdpcfg->apn));
			Vs_cJSON_AddItemToObject(root, "network", network_node);

		}
		else
		{
			Vs_cJSON_ReplaceItemInObject(network_node, "apn", Vs_cJSON_CreateString(pdpcfg->apn));
		}

		Vs_cJSON *username = Vs_cJSON_GetObjectItem(network_node, "username");
		if(username == 0)
		{
			Vs_cJSON_AddItemToObject(network_node, "username", Vs_cJSON_CreateString(pdpcfg->username));
			Vs_cJSON_AddItemToObject(root, "network", network_node);

		}
		else
		{
			Vs_cJSON_ReplaceItemInObject(network_node, "username", Vs_cJSON_CreateString(pdpcfg->username));
		}

		Vs_cJSON *password = Vs_cJSON_GetObjectItem(network_node, "password");
		if(password == 0)
		{
			Vs_cJSON_AddItemToObject(network_node, "password", Vs_cJSON_CreateString(pdpcfg->password));
			Vs_cJSON_AddItemToObject(root, "network", network_node);

		}
		else
		{
			Vs_cJSON_ReplaceItemInObject(network_node, "password", Vs_cJSON_CreateString(pdpcfg->password));
		}
	}

	// 打印控制台查看
	char *cPrint = Vs_cJSON_Print(root);

	fileRemove_lib(PARAMCONFIG_JSON);
	iRet = hal_fileWritePro(PARAMCONFIG_JSON, 0, cPrint, strlen(cPrint));
	sysLOG(CFG_LOG_LEVEL_2, "cJSON_Print：\
	%s\
	", cPrint);// cJSON_Print：有做格式调整
	// 返回的字符串指针需要自己释放
	free(cPrint);
	Vs_cJSON_Delete(root);

	if(NULL != rP)
		free(rP);

	return 0;
	
}


/*
*Function:		hal_cfgReadPdpCfg
*Description:	读取用户配置文件
*Input:			*pdpcfg-PDPCFG_STR指针
*Output:		NULL
*Hardware:		NULL
*Return:		>0:成功；其他:失败
*Others:
*/
int hal_cfgReadPdpCfg(PDPCFG_STR *pdpcfg)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;

	memset(pdpcfg->apn, 0, sizeof(pdpcfg));
	memset(pdpcfg->username, 0, sizeof(pdpcfg->username));
	memset(pdpcfg->password, 0, sizeof(pdpcfg->password));

	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);
	
	Vs_cJSON *network_node = Vs_cJSON_GetObjectItem(root, "network");
    if(network_node == 0)
	{
		iRet = -4;
		goto exit;
    }
	else
	{
		Vs_cJSON *apn_node = Vs_cJSON_GetObjectItem(network_node, "apn");
	    if(apn_node == 0)
		{
			iRet = -5;
			memcpy(pdpcfg->apn, "NULL", strlen("NULL"));
	    }
		else
		{
			memcpy(pdpcfg->apn, apn_node->valuestring, strlen(apn_node->valuestring));
			iRet = 0;
		}

		Vs_cJSON *username_node = Vs_cJSON_GetObjectItem(network_node, "username");
	    if(username_node == 0)
		{
			iRet = -5;
			memcpy(pdpcfg->username, "NULL", strlen("NULL"));
	    }
		else
		{
			memcpy(pdpcfg->username, username_node->valuestring, strlen(username_node->valuestring));
			iRet = 0;
		}

		Vs_cJSON *password_node = Vs_cJSON_GetObjectItem(network_node, "password");
	    if(password_node == 0)
		{
			iRet = -5;
			memcpy(pdpcfg->password, "NULL", strlen("NULL"));
	    }
		else
		{
			memcpy(pdpcfg->password, password_node->valuestring, strlen(password_node->valuestring));
			iRet = 0;
		}
	}

exit:

	Vs_cJSON_Delete(root);

	if(NULL != rP)
		free(rP);

	return iRet;
}



/*
*Function:		hal_cfgWriteTTSCfg
*Description:	写TTS语音库配置文件, 
*Input:			*ttspath_cn：TTS中文语音库存储路径; *ttspath_en:TTS英文语音库存储路径; *ttstype: tts语音播报类型,0-中文；1-英文
*Output:		NULL
*Hardware:		NULL
*Return:		>0:成功；其他:失败
*Others:		三个参数中又不需要设置的传NULL即可
*/
int hal_cfgWriteTTSCfg(char *ttspath_cn, char *ttspath_en, int *ttstype)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;
	int ttstypetmp = 0;
	unsigned char ttspath_cn_rP[256];
	unsigned char ttspath_en_rP[256];

	if(NULL == ttstype)
	{
		ttstypetmp = TTS_LIBTYPE_CN;
	}
	else
	{
		ttstypetmp = *ttstype;
	}

	memset(ttspath_cn_rP, 0, sizeof(ttspath_cn_rP));
	if(NULL == ttspath_cn)
	{
		sprintf(ttspath_cn_rP, "NULL");
	}
	else
	{
		memcpy(ttspath_cn_rP, ttspath_cn, strlen(ttspath_cn));
	}

	memset(ttspath_en_rP, 0, sizeof(ttspath_en_rP));
	if(NULL == ttspath_en)
	{
		sprintf(ttspath_en_rP, "NULL");
	}
	else
	{
		memcpy(ttspath_en_rP, ttspath_en, strlen(ttspath_en));
	}

	
	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);

	
	Vs_cJSON *ttslib_node = Vs_cJSON_GetObjectItem(root, "ttslib");
    if(ttslib_node == 0)
	{
		
        // 定义对象
		Vs_cJSON *ttslib = Vs_cJSON_CreateObject();
		// 插入元素，对应值
		Vs_cJSON_AddItemToObject(ttslib, "ttslibpath_cn", Vs_cJSON_CreateString(ttspath_cn_rP));
		Vs_cJSON_AddItemToObject(ttslib, "ttslibpath_en", Vs_cJSON_CreateString(ttspath_en_rP));
		Vs_cJSON_AddItemToObject(ttslib, "ttslibtype", Vs_cJSON_CreateNumber(ttstypetmp));
		Vs_cJSON_AddItemToObject(root, "ttslib", ttslib);
    }
	else
	{

		Vs_cJSON *ttslibpath_cn = Vs_cJSON_GetObjectItem(ttslib_node, "ttslibpath_cn");
		if(ttslibpath_cn == 0)
		{
			Vs_cJSON_AddItemToObject(ttslib_node, "ttslibpath_cn", Vs_cJSON_CreateString(ttspath_cn_rP));
			Vs_cJSON_AddItemToObject(root, "ttslib", ttslib_node);

		}
		else
		{
			if(NULL != ttspath_cn)
				Vs_cJSON_ReplaceItemInObject(ttslib_node, "ttslibpath_cn", Vs_cJSON_CreateString(ttspath_cn_rP));
		}
		
		Vs_cJSON *ttslibpath_en = Vs_cJSON_GetObjectItem(ttslib_node, "ttslibpath_en");
		if(ttslibpath_en == 0)
		{
			Vs_cJSON_AddItemToObject(ttslib_node, "ttslibpath_en", Vs_cJSON_CreateString(ttspath_en_rP));
			Vs_cJSON_AddItemToObject(root, "ttslib", ttslib_node);

		}
		else
		{
			if(NULL != ttspath_en)
				Vs_cJSON_ReplaceItemInObject(ttslib_node, "ttslibpath_en", Vs_cJSON_CreateString(ttspath_en_rP));
		}

		Vs_cJSON *ttslibtype = Vs_cJSON_GetObjectItem(ttslib_node, "ttslibtype");
		if(ttslibtype == 0)
		{
			Vs_cJSON_AddItemToObject(ttslib_node, "ttslibtype", Vs_cJSON_CreateNumber(ttstypetmp));
			Vs_cJSON_AddItemToObject(root, "ttslib", ttslib_node);

		}
		else
		{
			if(NULL != ttstype)
				Vs_cJSON_ReplaceItemInObject(ttslib_node, "ttslibtype", Vs_cJSON_CreateNumber(ttstypetmp));
		}
		

	}

	// 打印控制台查看
	char *cPrint = Vs_cJSON_Print(root);

	fileRemove_lib(PARAMCONFIG_JSON);
	iRet = hal_fileWritePro(PARAMCONFIG_JSON, 0, cPrint, strlen(cPrint));
	sysLOG(CFG_LOG_LEVEL_2, "cJSON_Print：\
	%s\
	", cPrint);// cJSON_Print：有做格式调整
	// 返回的字符串指针需要自己释放
	free(cPrint);
	Vs_cJSON_Delete(root);

	if(NULL != rP)
		free(rP);

	return 0;
	
}

/*
*Function:		hal_cfgReadTTSType
*Description:	读取TTS语音库类型
*Input:			NULL
*Output:		*ttstype:TTS_LIBTYPE_CN or TTS_LIBTYPE_EN
*Hardware:		NULL
*Return:		0:成功；其他:失败
*Others:
*/
int hal_cfgReadTTSType(int *ttstype)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;

	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);
	
	Vs_cJSON *ttslib_node = Vs_cJSON_GetObjectItem(root, "ttslib");
    if(ttslib_node == 0)
	{
		iRet = -4;
		goto exit;
    }
	else
	{
		Vs_cJSON *ttslibtype_node = Vs_cJSON_GetObjectItem(ttslib_node, "ttslibtype");
	    if(ttslibtype_node == 0)
		{
			iRet = -5;
			goto exit;
	    }
		else
		{
			*ttstype = ttslibtype_node->valueint;
			iRet = 0;
		}
	}

exit:

	Vs_cJSON_Delete(root);

	if(NULL != rP)
		free(rP);

	return iRet;
}



/*
*Function:		hal_cfgReadTTSPath
*Description:	读取中文TTS语音库存放路径
*Input:			ttstype:0-读取中文语音库路径; 1-读取英文语音库路
*Output:		*ttspath:中文语音库路径指针
*Hardware:		NULL
*Return:		>0:成功；其他:失败
*Others:
*/
int hal_cfgReadTTSPath(char *ttspath, int ttstype)
{
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;
	unsigned char ttslibpath_rP[256];

	memset(ttslibpath_rP, 0, sizeof(ttslibpath_rP));
	
	if(TTS_LIBTYPE_CN == ttstype)
	{
		memcpy(ttslibpath_rP, "ttslibpath_cn", sizeof("ttslibpath_cn"));
	}
	else if(TTS_LIBTYPE_EN == ttstype)
	{
		memcpy(ttslibpath_rP, "ttslibpath_en", sizeof("ttslibpath_en"));
	}
	else
	{
		return -1;
	}

	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);
	
	Vs_cJSON *ttslib_node = Vs_cJSON_GetObjectItem(root, "ttslib");
    if(ttslib_node == 0)
	{
		iRet = -4;
		goto exit;
    }
	else
	{
		Vs_cJSON *ttslibpath_cn_node = Vs_cJSON_GetObjectItem(ttslib_node, ttslibpath_rP);
	    if(ttslibpath_cn_node == 0)
		{
			iRet = -5;
			goto exit;
	    }
		else
		{
			if(memcmp(ttslibpath_cn_node->valuestring, "NULL", 4) == 0 && strlen(ttslibpath_cn_node->valuestring) == 4)
			{
				iRet = -6;
			}
			else
			{
				memcpy(ttspath, ttslibpath_cn_node->valuestring, strlen(ttslibpath_cn_node->valuestring));
				iRet = strlen(ttslibpath_cn_node->valuestring);
			}
		}
	}

exit:

	Vs_cJSON_Delete(root);

	if(NULL != rP)
		free(rP);

	return iRet;
}




/********************************TEST********************************/
void usercfgtest(void)
{
	int iRet = -1;
	int valtmp;
	char stringtmp[128];

	iRet = hal_cfgReadNtpEn(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadNtpEn, iRet=%d, valtmp=%d\r\n", iRet, valtmp);
	valtmp = 1;
	iRet = hal_cfgWriteNtpEn(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWriteNtpEn, iRet=%d\r\n", iRet);
	iRet = hal_cfgReadNtpEn(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadNtpEn, iRet=%d, valtmp=%d\r\n", iRet, valtmp);
	valtmp = 0;
	iRet = hal_cfgWriteNtpEn(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWriteNtpEn, iRet=%d\r\n", iRet);
	iRet = hal_cfgReadNtpEn(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadNtpEn, iRet=%d, valtmp=%d\r\n", iRet, valtmp);
	iRet = hal_cfgReadPdpCfg(&g_stPdpCfg);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadPdpCfg, iRet=%d, apn=%s, username=%s, password=%s\r\n", iRet, g_stPdpCfg.apn, g_stPdpCfg.username, g_stPdpCfg.password);
	iRet = hal_cfgWritePdpCfg(&g_stPdpCfg, "apn1", NULL, NULL);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWritePdpCfg, iRet=%d\r\n", iRet);
	iRet = hal_cfgReadPdpCfg(&g_stPdpCfg);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadPdpCfg, iRet=%d, apn=%s, username=%s, password=%s\r\n", iRet, g_stPdpCfg.apn, g_stPdpCfg.username, g_stPdpCfg.password);
	iRet = hal_cfgWritePdpCfg(&g_stPdpCfg, NULL, NULL, "password1");
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWritePdpCfg, iRet=%d\r\n", iRet);
	iRet = hal_cfgReadPdpCfg(&g_stPdpCfg);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadPdpCfg, iRet=%d, apn=%s, username=%s, password=%s\r\n", iRet, g_stPdpCfg.apn, g_stPdpCfg.username, g_stPdpCfg.password);
	iRet = hal_cfgWritePdpCfg(&g_stPdpCfg, "NULL", "NULL", "NULL");
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWritePdpCfg, iRet=%d\r\n", iRet);
	iRet = hal_cfgReadPdpCfg(&g_stPdpCfg);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadPdpCfg, iRet=%d, apn=%s, username=%s, password=%s\r\n", iRet, g_stPdpCfg.apn, g_stPdpCfg.username, g_stPdpCfg.password);
#if 0
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_CN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, stringtmp=%s, TTS_LIBTYPE_CN\r\n", iRet, stringtmp);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_EN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_EN\r\n", iRet);
	iRet = hal_cfgReadTTSType(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSType, iRet=%d, valtmp=%d\r\n", iRet, valtmp);

	valtmp = TTS_LIBTYPE_CN;
	iRet = hal_cfgWriteTTSCfg("/app/ufs/a.lib", NULL, &valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWriteTTSPath, iRet=%d, TTS_LIBTYPE_CN\r\n", iRet);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_CN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_CN\r\n", iRet);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_EN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_EN\r\n", iRet);
	iRet = hal_cfgReadTTSType(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSType, iRet=%d, valtmp=%d\r\n", iRet, valtmp);

	valtmp = TTS_LIBTYPE_EN;
	iRet = hal_cfgWriteTTSCfg(NULL, "/app/ufs/b.lib", &valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWriteTTSPath, iRet=%d, TTS_LIBTYPE_EN\r\n", iRet);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_CN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_CN\r\n", iRet);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_EN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_EN\r\n", iRet);
	iRet = hal_cfgReadTTSType(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSType, iRet=%d, valtmp=%d\r\n", iRet, valtmp);

	iRet = hal_cfgWriteTTSCfg(TTS_LIBPATH_CN, TTS_LIBPATH_EN, NULL);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgWriteTTSPath, iRet=%d\r\n", iRet);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_CN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_CN\r\n", iRet);
	memset(stringtmp, 0, sizeof(stringtmp));
	iRet = hal_cfgReadTTSPath(stringtmp, TTS_LIBTYPE_EN);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSPath, iRet=%d, TTS_LIBTYPE_EN\r\n", iRet);
	iRet = hal_cfgReadTTSType(&valtmp);
	sysLOG(CFG_LOG_LEVEL_2, "hal_cfgReadTTSType, iRet=%d, valtmp=%d\r\n", iRet, valtmp);
	
#endif
	
}


CONFIG_INI g_stRd_config;	//配置文件结构体
/*
*Function:		hal_reFresh_Cfg
*Description:	刷新硬件配置接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_reFresh_Cfg(void)
{
	int result =-3;
	if (strcmp(g_stRd_config.LCDDIRC,"00")==0 )//横屏
	{
		sysLOG(COLORLCD_LOG_LEVEL_1, "00 enter ");
		g_stLcdConfig.LCD_DIRECTION=0;
		g_stLcdConfig.LOGODEFAULTPOSITION_X=0;;
		g_stLcdConfig.LOGODEFAULTPOSITION_Y=32;
		g_stLcdConfig.COLORLOGODEFAULTPOSITION_X=100;
		g_stLcdConfig.COLORLOGODEFAULTPOSITION_Y=100;
		g_stLcdConfig.COLORICON_BATTERY_X=296;
		g_stLcdConfig.COLORICON_GPRS_X=12;
		g_stLcdConfig.COLORICON_BT_X=60;
		g_stLcdConfig.COLORICON_WIFI_X=36;
		g_stLcdConfig.COLORICON_USB_X=272;
		g_stLcdConfig.COLORICON_ICCARD_X=248;
		g_stLcdConfig.COLORICON_LOCK_X=224;
		g_stLcdConfig.COLORICON_TIME_X=130;
		g_stLcdConfig.COLORBIGBATTION_X=68;
		g_stLcdConfig.COLORBIGBATTION_Y=56;
		g_stLcdConfig.COLORLCD_PIXWIDTH=320;
		g_stLcdConfig.COLORLCD_PIXHIGH=240;
		g_stLcdConfig.COLORLCD_SUMPIXNUM=(g_stLcdConfig.COLORLCD_PIXWIDTH * g_stLcdConfig.COLORLCD_PIXHIGH);
		g_stLcdConfig.COLORLCD_PIXNUM=((g_stLcdConfig.COLORLCD_PIXWIDTH * g_stLcdConfig.COLORLCD_PIXHIGH)/8);
		g_stLcdConfig.COLORLCD_BLOCKWIDTH=320;
		g_stLcdConfig.COLORLCD_BLOCKHIGH=8;
		g_stLcdConfig.COLORLCD_BLOCKPIX=(g_stLcdConfig.COLORLCD_BLOCKWIDTH * g_stLcdConfig.COLORLCD_BLOCKHIGH);//一个block大小
		g_stLcdConfig.COLORLCD_BLOCKBUFNUM=(g_stLcdConfig.COLORLCD_SUMPIXNUM/g_stLcdConfig.COLORLCD_BLOCKPIX);//总共多少个block	
		g_stLcdConfig.ICON_BATTERY_X=116;
		g_stLcdConfig.ICON_GPRS_X=18;
		g_stLcdConfig.ICON_BT_X=42;
		g_stLcdConfig.ICON_WIFI_X=30;
		g_stLcdConfig.ICON_USB_X=104;
		g_stLcdConfig.ICON_ICCARD_X=92;
		g_stLcdConfig.ICON_LOCK_X=80;
		g_stLcdConfig.ICON_GPRSSIG_X=12;
		g_stLcdConfig.ICON_TIME_X=48;
		g_stLcdConfig.BIGBATTION_X=24;
		g_stLcdConfig.BIGBATTION_Y=16;
		g_stLcdConfig.LCD_X_PIXELMIN=0;//
		g_stLcdConfig.LCD_Y_PIXELMIN=0;
		g_stLcdConfig.LCD_X_PIXELMAX=127;//0-127,X像素最大值
		g_stLcdConfig.LCD_Y_PIXELMAX=95;//0-95,Y像素最大值
		g_stLcdConfig.LCD_LINEMAX=7;//0-7,包括图标这一行
		g_stLcdConfig.LCD_X_USERPIXELMIN=g_stLcdConfig.LCD_X_PIXELMIN;
		g_stLcdConfig.LCD_Y_USERPIXELMIN=12;//
		g_stLcdConfig.LCD_X_USERPIXELMAX=g_stLcdConfig.LCD_X_PIXELMAX;
		g_stLcdConfig.LCD_Y_USERPIXELMAX=g_stLcdConfig.LCD_Y_PIXELMAX;
		g_stLcdConfig.LCD_TEXTPIXEL=12;//字符占用12*12
		g_stLcdConfig.LCD_PIXWIDTH=128;
		g_stLcdConfig.LCD_PIXHIGH=96;
		g_stLcdConfig.LCD_SUMPIXNUM=(g_stLcdConfig.LCD_PIXWIDTH * g_stLcdConfig.LCD_PIXHIGH);
		g_stLcdConfig.LCD_PIXNUM=((g_stLcdConfig.LCD_PIXWIDTH * g_stLcdConfig.LCD_PIXHIGH)/8);
		g_stLcdConfig.LCD_BLOCKBYTE=8;//一个block实际存储为只占用了低4bit,实际上开辟空间一个block为8*LCD_BLOCKWIDTH
		g_stLcdConfig.LCD_BLOCKWIDTH=128;//一个block宽为128个像素点
		g_stLcdConfig.LCD_BLOCKHIGH=4;//一个block高为4个像素点
		g_stLcdConfig.LCD_BLOCKPIX=(g_stLcdConfig.LCD_BLOCKWIDTH * g_stLcdConfig.LCD_BLOCKHIGH);//一个block大小
		g_stLcdConfig.LCD_BLOCKBUFNUM=(g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKPIX);//总共多少个block
	}else if ( strcmp(g_stRd_config.LCDDIRC,"01")==0 )	//竖屏
	{
		sysLOG(COLORLCD_LOG_LEVEL_1, "01 enter ");
		g_stLcdConfig.LCD_DIRECTION=1;
		g_stLcdConfig.LOGODEFAULTPOSITION_X=0;
		g_stLcdConfig.LOGODEFAULTPOSITION_Y=40;
		g_stLcdConfig.COLORLOGODEFAULTPOSITION_X=55;
		g_stLcdConfig.COLORLOGODEFAULTPOSITION_Y=128;
		g_stLcdConfig.COLORICON_BATTERY_X=216;
		g_stLcdConfig.COLORICON_GPRS_X=12;
		g_stLcdConfig.COLORICON_BT_X=60;
		g_stLcdConfig.COLORICON_WIFI_X=36;
		g_stLcdConfig.COLORICON_USB_X=192;
		g_stLcdConfig.COLORICON_ICCARD_X=248;
		g_stLcdConfig.COLORICON_LOCK_X=224;
		g_stLcdConfig.COLORICON_TIME_X=90;
		g_stLcdConfig.COLORBIGBATTION_X=30;
		g_stLcdConfig.COLORBIGBATTION_Y=80;
		g_stLcdConfig.COLORLCD_PIXWIDTH=240;
		g_stLcdConfig.COLORLCD_PIXHIGH=320;
		g_stLcdConfig.COLORLCD_SUMPIXNUM=(g_stLcdConfig.COLORLCD_PIXWIDTH * g_stLcdConfig.COLORLCD_PIXHIGH);
		g_stLcdConfig.COLORLCD_PIXNUM=((g_stLcdConfig.COLORLCD_PIXWIDTH * g_stLcdConfig.COLORLCD_PIXHIGH)/8);
		g_stLcdConfig.COLORLCD_BLOCKWIDTH=240;
		g_stLcdConfig.COLORLCD_BLOCKHIGH=8;
		g_stLcdConfig.COLORLCD_BLOCKPIX=(g_stLcdConfig.COLORLCD_BLOCKWIDTH * g_stLcdConfig.COLORLCD_BLOCKHIGH);//一个block大小
		g_stLcdConfig.COLORLCD_BLOCKBUFNUM=(g_stLcdConfig.COLORLCD_SUMPIXNUM/g_stLcdConfig.COLORLCD_BLOCKPIX);//总共多少个block
		g_stLcdConfig.ICON_BATTERY_X=84;
		g_stLcdConfig.ICON_GPRS_X=18;
		g_stLcdConfig.ICON_BT_X=42;
		g_stLcdConfig.ICON_WIFI_X=30;
		g_stLcdConfig.ICON_USB_X=72;
		g_stLcdConfig.ICON_ICCARD_X=92;
		g_stLcdConfig.ICON_LOCK_X=80;
		g_stLcdConfig.ICON_GPRSSIG_X=12;
		g_stLcdConfig.ICON_TIME_X=30;
		g_stLcdConfig.BIGBATTION_X=8;
		g_stLcdConfig.BIGBATTION_Y=24;
		g_stLcdConfig.LCD_X_PIXELMIN=0;//
		g_stLcdConfig.LCD_Y_PIXELMIN=0;
		g_stLcdConfig.LCD_X_PIXELMAX=95;//0-95,Y像素最大值
		g_stLcdConfig.LCD_Y_PIXELMAX=127;//0-127,X像素最大值
		g_stLcdConfig.LCD_LINEMAX=9;//0-9,包括图标这一行
		g_stLcdConfig.LCD_X_USERPIXELMIN=g_stLcdConfig.LCD_X_PIXELMIN;
		g_stLcdConfig.LCD_Y_USERPIXELMIN=12;//
		g_stLcdConfig.LCD_X_USERPIXELMAX=g_stLcdConfig.LCD_X_PIXELMAX;
		g_stLcdConfig.LCD_Y_USERPIXELMAX=g_stLcdConfig.LCD_Y_PIXELMAX;
		g_stLcdConfig.LCD_TEXTPIXEL=12;//字符占用12*12
		g_stLcdConfig.LCD_PIXWIDTH=96;
		g_stLcdConfig.LCD_PIXHIGH=128;
		g_stLcdConfig.LCD_SUMPIXNUM=(g_stLcdConfig.LCD_PIXWIDTH * g_stLcdConfig.LCD_PIXHIGH);
		g_stLcdConfig.LCD_PIXNUM=((g_stLcdConfig.LCD_PIXWIDTH * g_stLcdConfig.LCD_PIXHIGH)/8);
		g_stLcdConfig.LCD_BLOCKBYTE=8;//一个block实际存储为只占用了低4bit,实际上开辟空间一个block为8*LCD_BLOCKWIDTH
		g_stLcdConfig.LCD_BLOCKWIDTH=96;//一个block宽为128个像素点
		g_stLcdConfig.LCD_BLOCKHIGH=4;//一个block高为4个像素点
		g_stLcdConfig.LCD_BLOCKPIX=(g_stLcdConfig.LCD_BLOCKWIDTH * g_stLcdConfig.LCD_BLOCKHIGH);//一个block大小
		g_stLcdConfig.LCD_BLOCKBUFNUM=(g_stLcdConfig.LCD_SUMPIXNUM/g_stLcdConfig.LCD_BLOCKPIX);//总共多少个block
	}
	// if ( strcmp(g_stRd_config.LCD,"00") == 0)				//黑白屏
	// {
	// 	g_ui8LcdType=0;
	// }
	// else if ( strcmp(g_stRd_config.LCD,"01") == 0)			//彩屏
	// {
	// 	g_ui8LcdType=1;
	// }
	
	if ( strcmp(g_stRd_config.CAMERA ,"NULL") == 0)			//无相机
	{
		g_iCamera_exist=0;
	}
	else if ( strcmp(g_stRd_config.CAMERA ,"01") == 0)		//有相机
	{
		g_iCamera_exist=1;
	}
	
	if ( strcmp(g_stRd_config.SPIFLASH ,"NULL") == 0)			//无flash
	{
		
		g_iFlash_exist=0;
	}
	else if ( strcmp(g_stRd_config.SPIFLASH ,"01") == 0)	//有flash
	{
		g_iFlash_exist=1;
	}

	if ( strcmp(g_stRd_config.CBLCD ,"00")==0)			//段码屏
	{
		g_ui8ScreenType=0;
		
	}
	else if ( strcmp(g_stRd_config.CBLCD ,"01")==0)		//点阵屏
	{
		g_ui8ScreenType=1;
	}

	if ( strcmp(g_stRd_config.TERMINAL_NAME ,"Q390")==0)			//机器类型Q390
	{
		sysLOG(COLORLCD_LOG_LEVEL_1, " type of machine is 390  ");
		g_iType_machine=0;
		get_machine_type();
	}
	else if ( strcmp(g_stRd_config.TERMINAL_NAME ,"Q161")==0)		//机器类型Q161
	{
		sysLOG(COLORLCD_LOG_LEVEL_1, " type of machine is 161  ");
		g_iType_machine=1;
		get_machine_type();
	}

	if (strcmp(g_stRd_config.KEYBOARD ,"00") == 0)					//AP
	{
		g_iTypeofKeyboard = 1;
	}
	else if (strcmp(g_stRd_config.KEYBOARD ,"01") == 0)				//SE
	{
		g_iTypeofKeyboard = 2;
	}
	else if (strcmp(g_stRd_config.KEYBOARD ,"NULL") == 0)			//无
	{
		g_iTypeofKeyboard = 0;
	}
	
	if (strcmp(g_stRd_config.LED ,"01") == 0)
	{
		g_iLed_exist = 1;
	}
	else if (strcmp(g_stRd_config.LED ,"NULL") == 0)				//无
	{
		g_iLed_exist = 0;
	}
	
	if (strcmp(g_stRd_config.TricolorLED ,"00") == 0)
	{
		g_iTricolorLED_exist = 1;
	}
	else if (strcmp(g_stRd_config.TricolorLED ,"NULL") == 0)		//无
	{
		g_iTricolorLED_exist = 0;
	}

	if (strcmp(g_stRd_config.GPS ,"00") == 0)				//AP端
	{
		g_iTypeofGPS = 1;
	}
	else if (strcmp(g_stRd_config.GPS ,"01") == 0)			//挂SE端
	{
		g_iTypeofGPS = 2;
	}
	else if (strcmp(g_stRd_config.GPS ,"02") == 0)			//挂AP端
	{
		g_iTypeofGPS = 3;
	}
	else if (strcmp(g_stRd_config.GPS ,"NULL") == 0)		//无
	{
		g_iTypeofGPS = 0;
	}

}


/*
*Function:		hal_throw_error
*Description:	抛出错误接口，防止没有json文件机器无法启动
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_throw_error(void)
{
	if ( (strcmp(g_stRd_config.LCDDIRC ,"00") != 0) && (strcmp(g_stRd_config.LCDDIRC ,"01") != 0) )
	{
		memcpy(g_stRd_config.LCDDIRC,"01",sizeof("01"));
		sysLOG(BASE_LOG_LEVEL_1, "lack of parameter of lcd \r\n");
	}
	

}

/*
*Function:		hal_config_Init
*Description:	读取json文件更改配置接口
*Input:			NULL
*Output:		NULL
*Hardware:
*Return:		NULL
*Others:
*/
void hal_config_Init(void)
{
	char *target[9]={
	"CBLCD",
	"Camera",
	"GPS",
	"LCDDirc",
	"SpiFlash",
	"Keyboard",
	"LED",
	"TricolorLED",
	"TERMINAL_NAME"
	};
	int iRet = -1;
	char *rP = NULL;
	Vs_cJSON* root = NULL;

	iRet = fileGetFileSize_lib(PARAMCONFIG_JSON);
	if(iRet < 0)
		return -1;

	rP = malloc(iRet);
	if(NULL == rP)
		return -2;

	memset(rP, 0, iRet);

	iRet = hal_fileReadPro(PARAMCONFIG_JSON, 0, rP, iRet);
	
	root = Vs_cJSON_Parse(rP);

	Vs_cJSON *terminal_node = Vs_cJSON_GetObjectItem(root, "terminal");

	for (size_t i = 0; i < 9; i++)
	{
		Vs_cJSON *node = Vs_cJSON_GetObjectItem(terminal_node, target[i]);
		if (node == NULL)
		{
			sysLOG(BASE_LOG_LEVEL_1, "match ini string failed :%s \r\n",target[i]);
		}
		else
		{
			switch (i)
			{
				case  0:
						memset(g_stRd_config.CBLCD,0,sizeof(g_stRd_config.CBLCD));
						memcpy(g_stRd_config.CBLCD,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.CBLCD :%s \r\n",g_stRd_config.CBLCD);
					break;
				case  1:
						memset(g_stRd_config.CAMERA,0,sizeof(g_stRd_config.CAMERA));
						memcpy(g_stRd_config.CAMERA,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.CAMERA :%s \r\n",g_stRd_config.CAMERA);
					break;
				case  2:
						memset(g_stRd_config.GPS,0,sizeof(g_stRd_config.GPS));
						memcpy(g_stRd_config.GPS,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.GPS :%s \r\n",g_stRd_config.GPS);
					break;
				case  3:
						memset(g_stRd_config.LCDDIRC,0,sizeof(g_stRd_config.LCDDIRC));
						memcpy(g_stRd_config.LCDDIRC,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.LCDDIRC :%s \r\n",g_stRd_config.LCDDIRC);
					break;
				case  4:
						memset(g_stRd_config.SPIFLASH,0,sizeof(g_stRd_config.SPIFLASH));
						memcpy(g_stRd_config.SPIFLASH,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.SPIFLASH :%s \r\n",g_stRd_config.SPIFLASH);
					break;
				case  5:
						memset(g_stRd_config.KEYBOARD,0,sizeof(g_stRd_config.KEYBOARD));
						memcpy(g_stRd_config.KEYBOARD,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.KEYBOARD :%s \r\n",g_stRd_config.KEYBOARD);
					break;
				case  6:
						memset(g_stRd_config.LED,0,sizeof(g_stRd_config.LED));
						memcpy(g_stRd_config.LED,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.LED :%s \r\n",g_stRd_config.LED);
					break;
				case  7:
						memset(g_stRd_config.TricolorLED,0,sizeof(g_stRd_config.TricolorLED));
						memcpy(g_stRd_config.TricolorLED,node->valuestring,sizeof(node->valuestring));
						sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.TricolorLED :%s \r\n",g_stRd_config.TricolorLED);
					break;
				case  8:
					memset(g_stRd_config.TERMINAL_NAME,0,sizeof(g_stRd_config.TERMINAL_NAME));
					memcpy(g_stRd_config.TERMINAL_NAME,node->valuestring,sizeof(node->valuestring));
					sysLOG(BASE_LOG_LEVEL_1, "g_stRd_config.TERMINAL_NAME :%s \r\n",g_stRd_config.TERMINAL_NAME);
				break;
				default:
					break;
			}
		}
	}
	hal_throw_error();
	hal_reFresh_Cfg();
	Vs_cJSON_Delete(root);
	free(rP);
}


