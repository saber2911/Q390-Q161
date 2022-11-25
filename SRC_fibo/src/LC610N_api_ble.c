/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "comm.h"


#define RING_BUF_SIZE 1024
ring_buffer_t btringRead;
//ring_buffer_t btringWrite;

ring_buffer_t *bt_ringRead;
//ring_buffer_t *bt_ringWrite;

typedef enum
{
	GATT_DISCONNECT = 0x00,
	GATT_CONNECT,
}GATT_CONNECT_STAT;



#define BT_8910_TP_UUID 0xffe0
#define BT_8910_TP_UUID_CHAR 0xffe1
#define BT_8910_FEEDBACK_CHAR 0x8910
#define ATT_UUID_BAS 0x180F //Battery Service
#define ATT_UUID_BATTERY_LEVEL 0x2A19 //Battery level characteristic

static gatt_chara_def_short_t wifi_mgr_chara = {{
	ATT_CHARA_PROP_READ | ATT_CHARA_PROP_WRITE | ATT_CHARA_PROP_INDICATE,
	0,
	0,
	ATT_UUID_BATTERY_LEVEL&0xff,
	ATT_UUID_BATTERY_LEVEL>>8
}};
static gatt_chara_def_short_t wifi_mgr_chara1 = {{
	ATT_CHARA_PROP_READ | ATT_CHARA_PROP_WRITE | ATT_CHARA_PROP_INDICATE,
	0,
	0,
	BT_8910_TP_UUID_CHAR&0xff,
	BT_8910_TP_UUID_CHAR>>8
}};


static gatt_chara_def_short_t wifi_mgr_chara2 = {{
	ATT_CHARA_PROP_READ | ATT_CHARA_PROP_NOTIFY,
	0,
	0,
	BT_8910_FEEDBACK_CHAR&0xff,
	BT_8910_FEEDBACK_CHAR>>8
}};

typedef struct
{
    UINT16 configurationBits;
    UINT16 aclHandle;
} gatt_chara_ccb_t;

static UINT8 fibo_8910_tp_data_uuid[16]= {0xf5,0x89,0x9b,0x5f,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,BT_8910_TP_UUID&0xff, BT_8910_TP_UUID>>8,0x00,0x00};
static gatt_chara_ccb_t fibo_8910_feedback_client_desc = {0x0000, 0x00}; //client characteristic configuration descriptor, bit0: notification enable, bit1:indications enable, other:RFU

char wifimgr_value[512] = "qazxswedcvfr01234567890123456789tgbnhyujmkiolp";
char wifimgr_ccc_cfg[100] = {0};
char device_name[100] = "8910_ble";
char notify_test[100] = "notify_test";
static UINT8 fibo_8910_bas_value = 20;

uint8_t test_service[2] = {ATT_UUID_BAS&0xff, ATT_UUID_BAS >> 8};
uint8_t add_service[2] = {0x180d&0xff, 0x180d >> 8};

UINT16 acl_handle = 0;
UINT8 gatt_connect_status = GATT_DISCONNECT;

static UINT8 fibo_8910_tp_data_value[244] = {0};

static UINT32 g_fibo_ble_queue = 0; 


char temp_data[512] ={0};
int test_data[128] = {0};

#define BT_8910_TP_UUID 0xf3f4
uint8_t bt_8910_tp_data_uuid[16]= {0xfb,0x35,0x9b,0x5f,0x80,0x00,0x00,0x80,0x00,0x10,0x00,0x00,BT_8910_TP_UUID&0xff, BT_8910_TP_UUID>>8,0x00,0x00};
uint8_t bt_8910_tp_data_uuid1[16]= {0x16,0x14,0xDE,0x54,0xDA,0xF2,0xFF,0x3C,0xFB,0xDC,0xBA,0xC9,0x80, 0x54,0x1D,0x11};

UINT8 data_write_callback(void *param);
UINT8 wifi_changed_cb(void *param);
UINT8 wifimgr_value_read_cb(void *param);
UINT8 fibo_8910_feedback_client_cb(void *param);
UINT8 fibo_8910_bas_write_cb(void *param);
UINT8 fibo_8910_tp_data_write_cb(void *param);
uint8_t fibo_send_notify(uint16_t datalen, uint8_t *data);




gatt_element_t config_wifi_service[]={

	{
		//creat service and fill UUID
	    sizeof(test_service),
	   	ATT_PM_READABLE,
	    {ATT_UUID_PRIMARY},
	    ATT_FMT_SHORT_UUID | ATT_FMT_GROUPED,
	    //ATT_FMT_GROUPED,
	    (void *)test_service,
	    NULL,
	    NULL
	},
	{
		//creat chara and fill permission
	    sizeof(wifi_mgr_chara),
		ATT_PM_READABLE,
	    {ATT_UUID_CHAR},
	    ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
	    (void *)&wifi_mgr_chara,
	    NULL,//cb
	    NULL//read callback
	},
	{
        .length = sizeof(fibo_8910_bas_value),
        .permisssion = ATT_PM_READABLE,
        .uuid.uuid_s = ATT_UUID_BATTERY_LEVEL,
        .fmt = ATT_FMT_WRITE_NOTIFY | ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH | ATT_FMT_CONFIRM_NOTITY,
        .attValue = (void *)&fibo_8910_bas_value,
        .cb = fibo_8910_bas_write_cb,
        .read_cb = NULL
    },
	{
	    sizeof(wifimgr_value),
		ATT_PM_READABLE | ATT_PM_WRITEABLE,
	    ATT_UUID_CLIENT,
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY | ATT_FMT_FIXED_LENGTH,
	    (void *)wifimgr_value,
	    data_write_callback,
	    NULL
	},
	{
		//des
	    sizeof(fibo_8910_tp_data_uuid),
	    ATT_PM_READABLE,
	    {ATT_UUID_PRIMARY},
	    ATT_FMT_GROUPED,
	    (void *)fibo_8910_tp_data_uuid,
	    //wifi_changed_cb,
	    NULL,
	    NULL
	},
	{
	    sizeof(wifi_mgr_chara1),
		ATT_PM_READABLE,
	    {ATT_UUID_CHAR},
	    ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
	    (void *)&wifi_mgr_chara1,
	    NULL,
	    NULL
	},
	{
        .length = sizeof(fibo_8910_tp_data_value),
        .permisssion = ATT_PM_WRITEABLE, //|ATT_PM_R_ENCRYPTION_REQUIRED,
        .uuid.uuid_s = BT_8910_TP_UUID_CHAR,
        .fmt = ATT_FMT_WRITE_NOTIFY | ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH | ATT_FMT_CONFIRM_NOTITY,
        .attValue = (void *)&fibo_8910_tp_data_value,
        .cb = fibo_8910_tp_data_write_cb,
        .read_cb = wifimgr_value_read_cb
    },
	{
		//creat chara and fill permission
	    sizeof(wifi_mgr_chara2),
		ATT_PM_READABLE,
	    {ATT_UUID_CHAR},
	    ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
	    (void *)&wifi_mgr_chara2,
	    wifi_changed_cb,//cb
	    wifimgr_value_read_cb//read callback
	},
	//feedback char,Unknown Characteristic ,UUID:8910
    /*{
        .length = sizeof(fibo_8910_feedback_char),
        .permisssion = ATT_PM_READABLE,
        .uuid.uuid_s = ATT_UUID_CHAR,
        .fmt = ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
        .attValue = (void *)&fibo_8910_feedback_char,
        .cb = NULL,
        .read_cb = NULL
    },*/
    {
		//fill chara value 
	    sizeof(wifimgr_value),
		ATT_PM_READABLE,
	    BT_8910_FEEDBACK_CHAR,
	    ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
	    (void *)wifimgr_value,
	    NULL,
	    wifimgr_value_read_cb
	},
	{
        .length = sizeof(fibo_8910_feedback_client_desc),
        .permisssion = ATT_PM_READABLE | ATT_PM_WRITEABLE,
        .uuid.uuid_s = ATT_UUID_CLIENT,
        .fmt = ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY | ATT_FMT_FIXED_LENGTH,
        .attValue = (void *)&fibo_8910_feedback_client_desc,
        .cb = fibo_8910_feedback_client_cb,
        .read_cb = NULL
	},
	/*{
	    sizeof(device_name),
		ATT_PM_READABLE,
	    {0x2a00},
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY | ATT_FMT_FIXED_LENGTH,
	    (void *)device_name,
	    NULL,
	    NULL
	},
	{
	    sizeof(wifimgr_ccc_cfg),
	    ATT_PM_READABLE | ATT_PM_WRITEABLE,
	    {ATT_UUID_CLIENT},
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY|ATT_FMT_FIXED_LENGTH,
	    (void *)wifimgr_ccc_cfg,
	    //wifi_changed_cb,
	    NULL,
	    NULL
	},
	{
		//creat service and fill UUID
	    sizeof(add_service),
	   	ATT_PM_READABLE,
	    {ATT_UUID_PRIMARY},
	    ATT_FMT_SHORT_UUID | ATT_FMT_GROUPED,
	    (void *)add_service,
	    NULL,
	    NULL
	},
	{
		//creat chara and fill permission
	    sizeof(wifi_mgr_chara2),
		ATT_PM_READABLE,
	    {ATT_UUID_CHAR},
	    ATT_FMT_SHORT_UUID | ATT_FMT_FIXED_LENGTH,
	    (void *)&wifi_mgr_chara2,
	    wifi_changed_cb,//cb
	    wifimgr_value_read_cb//read callback
	},
	
	{
		//des
	    sizeof(wifimgr_ccc_cfg),
	    ATT_PM_READABLE | ATT_PM_WRITEABLE,
	    {ATT_UUID_CLIENT},
	    ATT_FMT_SHORT_UUID | ATT_FMT_WRITE_NOTIFY|ATT_FMT_FIXED_LENGTH,
	    (void *)wifimgr_ccc_cfg,
	    wifi_changed_cb,
	    NULL
	},*/

};

UINT8 fibo_8910_bas_write_cb(void *param)
{
    OSI_LOGI(0, "[AUTO_BLE]bt_8910_bas_write_cb bt_8910_bas_value =%x ", fibo_8910_bas_value);
    return 0;
}
#if 0
UINT8 fibo_8910_tp_data_write_cb(void *param)
{
    uint8 data[256] = {0};
    uint16 data_len = 0;
    static int num = 0;
    //UINT8 datalen = 0;
    gatt_srv_write_notify_t *pAttr = (gatt_srv_write_notify_t *)param;
    //datalen = strlen((const char *)fibo_8910_tp_data_value);
    OSI_LOGI(0, "[AUTO_BLE]fibo_8910_bas_write_cb len=%d", pAttr->valueLen);
    for(int i = 0; i < pAttr->valueLen; i++)
    {
        OSI_PRINTFI("[AUTO_BLE]fibo_8910_bas_write_cb[%d]=%x",i,fibo_8910_tp_data_value[i]);
    }
    sprintf(data, "%dqazxswedcvfrtgbnhyujmkiolp0123456789", num);
	fibo_taskSleep(1000);
    fibo_send_notify(strlen(data), data);
    num++;
    memset(fibo_8910_tp_data_value, 0, sizeof(fibo_8910_tp_data_value));
    return 0;
}
#else
UINT8 fibo_8910_tp_data_write_cb(void *param)
{
    int iRet;
    gatt_srv_write_notify_t *pAttr = (gatt_srv_write_notify_t *)param;

	iRet = ring_buffer_write((uint8_t *)fibo_8910_tp_data_value, pAttr->valueLen, bt_ringRead);
	
    memset(fibo_8910_tp_data_value, 0, sizeof(fibo_8910_tp_data_value));
    return 0;
}

#endif
UINT8 data_write_callback(void *param)
{
	//UINT8 datalen = 0;
	OSI_PRINTFI("[AUTO_BLE][%s:%d]wifimgr_value=%s", __FUNCTION__, __LINE__,wifimgr_value);
	memset(wifimgr_value,0,512);
	memcpy(wifimgr_value,"abcd123456",10);
	
	test_data[127]= 1;
	
	OSI_PRINTFI("[AUTO_BLE][%s:%d] current thread ID = 0x%x", __FUNCTION__, __LINE__,fibo_thread_id());
	
    if(param != NULL)
    {
		gatt_srv_write_notify_t *pAttr = (gatt_srv_write_notify_t *)param;
		OSI_PRINTFI("[AUTO_BLE][%s:%d]pAttr->->valueLen = %d,test_data[7999] = %d", __FUNCTION__, __LINE__,pAttr->valueLen,test_data[127]);
		int test = 1;
		fibo_queue_put(g_fibo_ble_queue, (void *)&test, 0);	

		fibo_taskSleep(1000);
		test = 2;
		fibo_queue_put(g_fibo_ble_queue, (void *)&test, 0);	
	}

	return 0;
	
}

UINT8 fibo_8910_feedback_client_cb(void *param)
{
    UINT8 value[2] = {0, 0};
    gatt_srv_write_notify_t *msg = (gatt_srv_write_notify_t *)param;
    OSI_LOGI(0, "bt_8910_feedback_client_cb, msg->valueLen=%x,configurationBits=%x",
             msg->valueLen, fibo_8910_feedback_client_desc.configurationBits);

    if (msg == NULL || msg->attribute == NULL || msg->attribute->attValue == NULL)
    {
        return 0;
    }

    if (msg->valueLen > 2)
    {
        msg->valueLen = 2;
    }
    if (msg->valueLen)
    {
        memcpy(value, msg->attribute->attValue, msg->valueLen);
    }

    if (fibo_8910_feedback_client_desc.configurationBits & 0x01)
    {
        //app_bt_send_notify(ble_handle, ((bt_8910_tp_data_char.value[2]<<8)|bt_8910_tp_data_char.value[1]),
        //                             sizeof(bt_8910_feedback_value), &bt_8910_feedback_value[0]);
    }

    return 0;
}


UINT8 read_notify_callback(void *param)
{
	OSI_PRINTFI("[AUTO_BLE][%s:%d]device_name=%s", __FUNCTION__, __LINE__,device_name);
  
	return 0;
	
}



UINT8 wifi_changed_cb(void *param)
{
	OSI_PRINTFI("[AUTO_BLE][%s:%d] param = %p,wifimgr_value=%s", __FUNCTION__, __LINE__,param,wifimgr_value);
	gatt_le_data_info_t notify;
	notify.att_handle = wifi_mgr_chara1.value[2] << 8 | wifi_mgr_chara1.value[1];
	memcpy((char *)&device_name[0],"notify_test",strlen((char *)"notify_test"));
	OSI_PRINTFI("[AUTO_BLE][%s:%d] device_name = %s", __FUNCTION__, __LINE__,param,device_name);
	notify.length = sizeof(device_name);
	notify.data = (UINT8 *)&(device_name)[0];
	notify.acl_handle = acl_handle;
	return 0;
}
UINT8 wifimgr_value_write_cb(void *param)
{
    OSI_PRINTFI("[AUTO_BLE][%s:%d] param = %p,wifimgr_value=%s", __FUNCTION__, __LINE__,param,wifimgr_value);
	return 0;
}

UINT8 wifimgr_value_read_cb(void *param)
{
    uint8 data[256] = {0};
    uint16 data_len = 0;
    static int num = 0;

    if(param != NULL)
    {
		gatt_srv_write_notify_t *pAttr = (gatt_srv_write_notify_t *)param;
		OSI_PRINTFI("[AUTO_BLE][%s:%d]pAttr->->valueLen = %d", __FUNCTION__, __LINE__,pAttr->valueLen);
	}
	memset(wifimgr_value,0,sizeof(wifimgr_value));
	memcpy(wifimgr_value,"abcd123456",10);
    if (num % 2 == 0)
    {
        config_wifi_service[8].length = 10;
    }
    else
    {
        config_wifi_service[8].length = 5;
    }
    
    OSI_PRINTFI("[AUTO_BLE][%s:%d] param = %p,wifimgr_value=%s", __FUNCTION__, __LINE__,param,wifimgr_value);
	
	OSI_PRINTFI("[AUTO_BLE][%s:%d] current thread ID = 0x%x", __FUNCTION__, __LINE__,fibo_thread_id());
    num++;
	return 0;
}

uint8_t fibo_send_notify(uint16_t datalen, uint8_t *data)
{
	memset(fibo_8910_tp_data_value,0,sizeof(fibo_8910_tp_data_value));
	memcpy(fibo_8910_tp_data_value,data,datalen);
	gatt_le_data_info_t notify;
	notify.att_handle = wifi_mgr_chara2.value[2] << 8 | wifi_mgr_chara2.value[1];
	notify.length = datalen;
	notify.data = (UINT8 *)&fibo_8910_tp_data_value[0];
	notify.acl_handle = acl_handle;//no effect ,one connect acl_handle
	
	fibo_ble_notify((gatt_le_data_info_t *)&notify,GATT_NOTIFICATION);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] %s,handle=%x,lenth=%d", __FUNCTION__, __LINE__,data,notify.att_handle,datalen);
	return 0;
}

uint8_t fibo_send_indicator(uint16_t datalen, uint8_t *data)
{
	memset(device_name,0,sizeof(device_name));
	memcpy(device_name,data,datalen);
	gatt_le_data_info_t notify;
	notify.att_handle = wifi_mgr_chara1.value[2] << 8 | wifi_mgr_chara1.value[1];
	notify.length = datalen;
	notify.data = (UINT8 *)&device_name[0];
	notify.acl_handle = acl_handle;//no effect ,one connect acl_handle
	fibo_ble_notify((gatt_le_data_info_t *)&notify,GATT_INDICATION);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] %s,handle=%x", __FUNCTION__, __LINE__,data,notify.att_handle);
	return 0;
}





static void sig_res_callback(GAPP_SIGNAL_ID_T sig, va_list arg)
{
    switch (sig)
    {
	   
		//fibo_PDPActive /fibo_asyn_PDPActive  pdp active status report
		case GAPP_SIG_PDP_ACTIVE_IND:
		{
			UINT8 cid = (UINT8)va_arg(arg, int);
			OSI_PRINTFI("sig_res_callback  cid = %d", cid);
			va_end(arg);

		}
		break;

		//fibo_PDPRelease /fibo_asyn_PDPRelease pdp deactive status report
		case GAPP_SIG_PDP_RELEASE_IND:
		{
			UINT8 cid = (UINT8)va_arg(arg, int);
			OSI_PRINTFI("sig_res_callback  cid = %d", cid);
			va_end(arg);

		}
		break;

		//GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND
		case GAPP_SIG_PDP_ACTIVE_OR_DEACTIVE_FAIL_IND:
		{
		
			UINT8 cid = (UINT8)va_arg(arg, int);
			UINT8 state = (UINT8)va_arg(arg, int);
			OSI_PRINTFI("[%s-%d]cid = %d,state = %d", __FUNCTION__, __LINE__,cid,state);
			va_end(arg);
		}
		break;

		//PDP in active state, deactive indicator received from modem  
		case GAPP_SIG_PDP_DEACTIVE_ABNORMALLY_IND:
		{
		
			UINT8 cid = (UINT8)va_arg(arg, int);
			OSI_PRINTFI("[%s-%d]cid = %d", __FUNCTION__, __LINE__,cid);
			va_end(arg);
		}
		break;

		case GAPP_SIG_BLE_SET_ADV_IND:
		{
			UINT8 type = (UINT8)va_arg(arg, int);
			UINT8 state = (UINT8)va_arg(arg, int);
			OSI_PRINTFI("[AUTO_BLE][%s:%d]type=%d,state=%d", __FUNCTION__, __LINE__,type,state);				
		}
		break;
		case GAPP_SIG_BT_ON_IND: 
		{

			OSI_PRINTFI("[AUTO_BLE][%s:%d]GAPP_SIG_BT_ON_IND", __FUNCTION__, __LINE__);	

		}
		break;
		case GAPP_SIG_BLE_CONNECT_IND:
		{
			int connect_id = (int)va_arg(arg, int);
			int state = (int)va_arg(arg, int);
			UINT8 *addr = (UINT8 *)va_arg(arg, UINT8 *);
			UINT8 reason = (UINT8 )va_arg(arg, int);
			va_end(arg);
			OSI_PRINTFI("[AUTO_BLE][%s:%d]type=%d,state=%d,%p,%d", __FUNCTION__, __LINE__,connect_id,state,addr,reason);
			if(addr != NULL)
			{
				OSI_PRINTFI("[AUTO_BLE][%s:%d]type=%d,state=%d,%s,%d", __FUNCTION__, __LINE__,connect_id,state,addr,reason);
			}

            acl_handle = (int)(connect_id);
			if(state == 0)
			{
				//fibo_ble_enable_dev(1); // open broadcast
			}
		}
		break;

		
	    default:
	    {
	        break;
	    }
    }
    OSI_LOGI(0, "test");
}

static FIBO_CALLBACK_T user_callback = {
    .fibo_signal = sig_res_callback};


static void fibo_mtu_exchange_result_cb(UINT16 handle, UINT16 mtu)
{
	OSI_PRINTFI("[AUTO_BLE][%s:%d] handle = %d,mtu = %d", __FUNCTION__, __LINE__,handle,mtu);
}

static fibo_ble_btgatt_callback_t fibo_ble_btgatt_callback={
	.client = NULL,
	.server = NULL,
	.mtu_exchange_result_cb = fibo_mtu_exchange_result_cb,
};



static void prvThreadEntry(void *param)
{
    OSI_LOGI(0, "application thread enter, param 0x%x", param);
	UINT32 size;


	OSI_PRINTFI("[AUTO_BLE][%s:%d] current thread ID = 0x%x", __FUNCTION__, __LINE__,fibo_thread_id());

	fibo_bt_onoff(1);
	fibo_taskSleep(2000);
	char name_set[28] = {0};
	memset(name_set,0,28);
	memcpy(name_set,"8910_ble",sizeof("8910_ble"));
	int ret = fibo_ble_set_read_name(0,name_set,0); // set ble name 
    fibo_ble_client_server_int(&fibo_ble_btgatt_callback);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] ret = %d", __FUNCTION__, __LINE__,ret);

	
	fibo_taskSleep(2000);

	char addr[18] = {0};
	
	char name[41] = {0};
	fibo_ble_set_read_name(1,name,0);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] name = %s", __FUNCTION__, __LINE__,name);
	
	size = sizeof(config_wifi_service)/sizeof(gatt_element_t);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] size = %d", __FUNCTION__, __LINE__,size);
	fibo_ble_add_service_and_characteristic(config_wifi_service,size); //create serive and characteristic
	fibo_taskSleep(2000);


	fibo_ble_enable_dev(1); // open broadcast

	fibo_taskSleep(2000);
#if 0
	char adv_data[20]={0};
	
	fibo_ble_enable_dev(0); // close broadcast
    memcpy(adv_data,"0303E7FE0DFF0102123456123456123456123456",36);
	fibo_ble_set_dev_data(18,adv_data); // set broadcast data

	fibo_ble_enable_dev(1); // open broadcast
#endif

#if 0

    fibo_taskSleep(20000);
	fibo_ble_enable_dev(0); // close broadcast
    memcpy(adv_data,"050209010205",12);
	fibo_ble_set_dev_data(6,adv_data); // set broadcast data

	fibo_ble_enable_dev(1); // open broadcast



	fibo_taskSleep(20000);
	fibo_ble_enable_dev(0); // close broadcast
    fibo_ble_set_dev_param(6,60,80,0,0,-1,7,0,NULL);
	fibo_ble_enable_dev(1); // open broadcast
#endif

	while(1)
	{
        OSI_LOGI(0, "hello world %d");
        fibo_taskSleep(10000);
        /*sprintf(data, "%dqazxswedcvfrtgbnhyujmkiolp0123456789", num);
        fibo_send_notify(strlen(data), data);
        num++;*/
	}

	test_printf();
    fibo_thread_delete();
}


static void ull_wan_ble_drv_int(void)
{
	UINT32 size;
		
	fibo_bt_onoff(1);
	fibo_taskSleep(2000);
	char name_set[28] = {0};
	memset(name_set,0,28);
	memcpy(name_set,"8910_ble",sizeof("8910_ble"));
	int ret = fibo_ble_set_read_name(0,name_set,0); // set ble name 
	fibo_ble_client_server_int(&fibo_ble_btgatt_callback);
	
		
	fibo_taskSleep(2000);
	
	char addr[18] = {0};
		
	char name[41] = {0};
	fibo_ble_set_read_name(1,name,0);
	//OSI_PRINTFI("[AUTO_BLE][%s:%d] name = %s", __FUNCTION__, __LINE__,name);
		
	size = sizeof(config_wifi_service)/sizeof(gatt_element_t);
	//OSI_PRINTFI("[AUTO_BLE][%s:%d] size = %d", __FUNCTION__, __LINE__,size);
	fibo_ble_add_service_and_characteristic(config_wifi_service,size); //create serive and characteristic
	fibo_taskSleep(2000);
	
	
	fibo_ble_enable_dev(1); // open broadcast
	
	fibo_taskSleep(2000);
#if 0
		char adv_data[20]={0};
		
		fibo_ble_enable_dev(0); // close broadcast
		memcpy(adv_data,"0303E7FE0DFF0102123456123456123456123456",36);
		fibo_ble_set_dev_data(18,adv_data); // set broadcast data
	
		fibo_ble_enable_dev(1); // open broadcast
#endif
	
#if 0
	
		fibo_taskSleep(20000);
		fibo_ble_enable_dev(0); // close broadcast
		memcpy(adv_data,"050209010205",12);
		fibo_ble_set_dev_data(6,adv_data); // set broadcast data
	
		fibo_ble_enable_dev(1); // open broadcast
	
	
	
		fibo_taskSleep(20000);
		fibo_ble_enable_dev(0); // close broadcast
		fibo_ble_set_dev_param(6,60,80,0,0,-1,7,0,NULL);
		fibo_ble_enable_dev(1); // open broadcast
#endif


}




void ull_ble_task(void *pArg)
{
	fibo_taskSleep(5000);

	ull_wan_ble_drv_int();


	while(1)
	{
        //ull_wan_ble_drv_int();
        //UllOsLog(ULL_LOG_TEST, "%s:BLE open",__FUNCTION__);
        OSI_LOGI(0, "hello world %d");
        fibo_taskSleep(10000);
	}
	
   // fibo_thread_delete();

}


void fibo_ble_task(void *param)
{
	while(1)
	{
	    int value = 0;
		fibo_queue_get(g_fibo_ble_queue, (void *)&value, 0);
		switch(value)
		{
           case 1: //send notification
           fibo_send_notify(6,"notify");
		   break;
		   case 2: //send indicator
		   fibo_send_indicator(strlen("inidicator"),"inidicator");
		   break;
		   default:
		   	OSI_PRINTFI("[AUTO_BLE][%s:%d]", __FUNCTION__, __LINE__);
		   break;
		}
	}
	return;

}
#define BLUETOOTH_OK                             0
#define BLUETOOTH_ERR_NOT_OPEN					(-6000)				   	/*蓝牙未打开*/
#define BLUETOOTH_ERR_INVALID_PARAM             (-6001)                	/*函数入参错误*/
#define BLUETOOTH_ERR_OPREATE_NOTPERMIT         (-6002)                	/*操作不允许*/
#define BLUETOOTH_ERR_SUPPORT	                (-6003)	               	/*命令不支持*/

#define BLUETOOTH_ERR_BLE_SET_MAC				(-6030)				   	/*设置蓝牙BLE的MAC地址失败*/
#define BLUETOOTH_ERR_BLE_SET_NAME				(-6031)				   	/*设置蓝牙BLE的名称失败*/
#define BLUETOOTH_ERR_BLE_SET_PAIRINGMODE		(-6032)				   	/*设置蓝牙BLE的配对模式失败*/
#define BLUETOOTH_ERR_POWERON_FAIL				(-6033)				   	/*模块上电异常*/
#define BLUETOOTH_ERR_DISCONNECT				(-6034)				   	/*蓝牙未连接*/
#define BLUETOOTH_ERR_INIT						(-6035)					/*模块初始化异常*/
#define BLUETOOTH_ERR_NONVRAM					(-6036)					/*没有NVRAM数据*/
#define BLUETOOTH_ERR_CONFIGFILE				(-6037)					/*配置文件不存在或文件中的蓝牙参数错误*/
#define BLUETOOTH_ERR_12MOUT					(-6038)					/*12M输出设置异常*/
#define BLUETOOTH_ERR_SET_FLOW					(-6039)					/*启用流控失败*/
#define BLUETOOTH_ERR_RESET						(-6040)					/*模块复位失败*/

//操作指令

typedef enum{
	eBtCmdGetName = 0, /*获取蓝牙设备名*/
	eBtCmdSetName , /*修改蓝牙设备名*/
	eBtCmdGetMAC, /*获取蓝牙设备 MAC*/
	eBtCmdGetSN, /*获取蓝牙设备 SN*/
	eBtCmdReset, /*复位蓝牙模块*/
	eBtCmdDropLink, /*断开连接*/
	eBtCmdGetLinkStatus, /*获取连接状态 */
	eBtCmdSetPairMode, /*选择配对模式*/
	eBtCmdGetPairMode, /*获取配对模式*/
	eBtCmdSetPinCode, /*设置 PinCode*/
	eBtCmdGetPinCode, /*获取 PinCode*/
	eBtCmdClearRecv, /*清除接收缓存*/
}E_BtCtlCmd;


typedef struct
{
	char cInitOK;					/**0 --- 初始化上电失败  1---初始化上电成功**/
	char cOpenState;				/** 1 --- Open  0 --- Close**/
	int iErrCode;					/**初始化错误码**/
}BT_STRUCT;
BT_STRUCT g_stBt;

#define BLUETOOTH_NAME_MAX_LEN 	22
#define BT_QUEUE_SIZE			1024

/**
 * [Function]       api_blueToothInit
 * [Description]    模块初始化
 * [param]          无
 * [return]         
 * [modify]                
 */
int api_blueToothInit(void)
{
	g_stBt.cInitOK = 0;
	g_stBt.cOpenState = 0;
	g_stBt.iErrCode = 0;

	g_stBt.cInitOK = 1;

	bt_ringRead = &btringRead;
	//bt_ringWrite = &btringWrite;
	return 0;
}


//open 和 close 需成对出现
int btOpen_lib()
{
	int iRet;
	unsigned char temp[32];
	if(g_stBt.cOpenState == 1)
		return 0;
	iRet = ring_buffer_init(bt_ringRead, RING_BUF_SIZE);	
	OSI_PRINTFI(" AUTO_BLE [%s:%d] bt_ringRead ret = %d", __FUNCTION__, __LINE__,iRet);
	//if(g_fibo_ble_queue == 0)
    {
		//g_fibo_ble_queue = fibo_queue_create(20, sizeof(int));
	}	
	//iRet = ring_buffer_init(bt_ringWrite, 512);
	//OSI_PRINTFI(" AUTO_BLE [%s:%d] bt_ringWrite ret = %d", __FUNCTION__, __LINE__,iRet);
	
	//open bt function
	fibo_bt_onoff(1);

	
	//set device name
	char name_set[28] = {0};
	memset(name_set,0,sizeof(name_set));
	memset(temp,0,sizeof(temp));
	iRet = sysReadSn_lib(0x0055FFAA, (unsigned char *)temp);
	if(iRet >=4)
	{
		sprintf(name_set,"Q360_%s",temp+iRet-4);		
	}
	else
	{
		memcpy(name_set,"Q360_FFFF",sizeof("Q360_FFFF"));
	}
	
	iRet = fibo_ble_set_read_name(0,name_set,0); // set ble name 

	sysLOG(1, "BLE_NAME:%s",name_set);

	
    fibo_ble_client_server_int(&fibo_ble_btgatt_callback);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] ret = %d", __FUNCTION__, __LINE__,iRet);

	fibo_taskSleep(100);

	char name[41] = {0};
	fibo_ble_set_read_name(1,name,0);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] name = %s", __FUNCTION__, __LINE__,name);
	
	int size = sizeof(config_wifi_service)/sizeof(gatt_element_t);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] size = %d", __FUNCTION__, __LINE__,size);
	fibo_ble_add_service_and_characteristic(config_wifi_service,size); //create serive and characteristic
	fibo_taskSleep(100);

	fibo_ble_enable_dev(1); // open broadcast

	g_stBt.cOpenState = 1;

	return 0;
	
}

int btClose_lib()
{
	int iRet;
	iRet = fibo_bt_onoff(0);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] iRet = %d", __FUNCTION__, __LINE__,iRet);
	g_stBt.cOpenState = 0;
}

//
int btSend_lib(unsigned char *pucData, unsigned int uiLen)
{
	int iRet;
	fibo_send_notify(uiLen, pucData);
	OSI_PRINTFI("AUTO_BLE [%s:%d] iRet = %d", __FUNCTION__, __LINE__,iRet);
	return iRet;
}

int btRecv_lib(unsigned char *pucBuff, unsigned int uiSize, unsigned int uiTimeOut)
{
    unsigned long long uiStartTime = 0;
    int iRet;
    int i;
    unsigned char ucGetChar;
    
    if(uiTimeOut == 0)   
		uiTimeOut = 2;
    
    uiStartTime = hal_sysGetTickms();
    i = 0;
    do{
        if(hal_sysGetTickms() > uiStartTime+uiTimeOut)
        {
            break;
        }        
        iRet =  ring_buffer_read(bt_ringRead,&ucGetChar,1);
        if(iRet <= 0)
        {
        	sysDelayMs(10);
            continue;
        }
        else
        {
            pucBuff[i++] = ucGetChar;
            if(i==uiSize)
                break;
        }       
     }while(1);		
    OSI_PRINTFI("[AUTO_BLE][%s:%d] rev size= %d", __FUNCTION__, __LINE__,i);
    return i;
}
/**
 * [Function]       api_blueToothCtrl
 * [Description]    对蓝牙设置进行控制
 * [param]          iCmd        命令，E_BtCtlCmd内所有参数
                    pArgIn      命令需要输入的参数
                    iSizeIn     输入数组长度
                    pArgOut     命令返回的参数
                    iSizeOut    提供的返回参数大小
 * [return]         
 * [modify]                
 */
int btCtrl_lib(unsigned int uiCmd, void *pArgIn, unsigned int uiSizeIn, void *pArgOut, unsigned int uiSizeOut)
{
    int i = 0;
    int iRet = 0;
	//char cMode = 0x00;

	if(1 != g_stBt.cInitOK)
	{
		return g_stBt.iErrCode;
	}
	switch(uiCmd)
	{
	
		case eBtCmdGetName:
		{
			if((pArgOut == NULL) /*|| (uiSizeOut != sizeof(ConfigArg_t))*/)
			{
				return BLUETOOTH_ERR_INVALID_PARAM;
			}
			return hal_blueToothReadName(pArgOut,uiSizeOut);		
		}
		case eBtCmdSetName:
		{
			return hal_blueToothModifyName(pArgIn, uiSizeIn);
			break;
		}
		case eBtCmdGetLinkStatus:
		{
			return hal_blueToothGetLinkStatus();
		}
		case eBtCmdClearRecv:
		{
			hal_blueToothClearRecv();
			break;
		}
		case eBtCmdGetMAC:
			hal_btGetMac(pArgOut,uiSizeOut);

		break;
		default :
			iRet = BLUETOOTH_ERR_SUPPORT;
		break;
	}

	return iRet;

}


/**
 * [Function]       hal_blueToothReadName
 * [Description]    读取蓝牙名
 * [param]          pucBuff       数据指针
                    iSize         数据长度
 * [return]         0     设置成功
 * [modify]                
 */
int hal_blueToothReadName(uint8_t *pucBuff,int iSize)
{
	char name_set[28];
	memset(name_set,0,28);
	int ret = fibo_ble_set_read_name(1,name_set,0); // set ble name 

	iSize = iSize > strlen(name_set) ? strlen(name_set) : iSize;

	memcpy(pucBuff,name_set,iSize);
	
	return ret;
}

/**
 * [Function]       hal_blueToothModifyName
 * [Description]    修改蓝牙名
 * [param]          pucData       数据指针
                    iLen         数据长度
 * [return]         0     设置成功
 * [modify]                
 */
int hal_blueToothModifyName(unsigned char *pucData, int iLen)
{
	int iSizeOut;
	int ret;
	uint8_t name[23];
	iSizeOut = iLen > strlen(pucData) ? strlen(pucData) : iLen;//取两个中 小的作为长度值写入

	if (iSizeOut > BLUETOOTH_NAME_MAX_LEN)
		iSizeOut = BLUETOOTH_NAME_MAX_LEN;
	
	memset(name , 0 ,sizeof(name));
	memcpy(name,pucData,iSizeOut);
	
	ret = fibo_ble_set_read_name(0,pucData,0); // set ble name 

	return ret;
}
/*< state >：输出 BLE 连接
状态，输出状态范围 0---6 0 表示初始状态。
1 表示连接状态。
2 表示断开状态。
其他表示中间态
< addr >：查询与之连接
状态的地址，外部长度需
大于 18。
*/
/**
 * [Function]       hal_blueToothGetLinkStatus
 * [Description]    获取蓝牙连接状态
 * [param]          
 * [return]         
 * [modify]                
 */
int hal_blueToothGetLinkStatus(void)
{
	int iRet;
	int status;
	char addr[18];
	memset(addr,0,sizeof(addr));
	iRet = fibo_ble_get_connect_state(&status, addr);
	if(iRet == 0)
	{
		return status;
	}
	return -1;
}


/**
 * [Function]       hal_blueToothClearRecv
 * [Description]    清除接收缓存
 * [param]          
 * [return]         
 * [modify]                
 */
int hal_blueToothClearRecv()
{
	
	return clear_ringBuffer(bt_ringRead,RING_BUF_SIZE);
}

/**
 * [Function]       hal_btGetMac
 * [Description]    获取MAC地址
 * [param]          pucBuff       数据指针
                    iSize         数据长度
 * [return]         0     设置成功
 * [modify]                
 */
int hal_btGetMac(unsigned char *pucBuff, int iSize)
{
	int iRet,lenth;
	char recv[18];
	

	lenth = iSize<17 ? iSize:17;
	
	//memset(recv, 0, sizeof(recv));
	//iRet = fibo_ble_set_read_addr(1,0,recv);
	//memcpy(pucBuff,recv,lenth);
	//OSI_PRINTFI("[AUTO_BLE][%s:%d] GET public mac= %s", __FUNCTION__, __LINE__,pucBuff);

	memset(recv, 0, sizeof(recv));
	iRet = fibo_ble_set_read_addr(1,1,recv);
	memcpy(pucBuff,recv,lenth);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] GET random mac= %s", __FUNCTION__, __LINE__,pucBuff);
	return iRet;
}

/**
 * [Function]       hal_btSetMac
 * [Description]    设置MAC地址
 * [param]          pucBuff       数据指针
                    iSize         数据长度
 * [return]         0     设置成功
 * [modify]                
 */
int hal_btSetMac(unsigned char *pucBuff, int iSize)
{
	int iRet;

	iRet = fibo_ble_set_read_addr(0,0,pucBuff);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] GET public mac= %s", __FUNCTION__, __LINE__,pucBuff);

	iRet = fibo_ble_set_read_addr(0,1,pucBuff);
	OSI_PRINTFI("[AUTO_BLE][%s:%d] GET random mac= %s", __FUNCTION__, __LINE__,pucBuff);
	return iRet;
}


void fibo_ble_task_test(void *param)
{
	int iRet;
	unsigned char *data;
	int lenth;
	int total = 0;
	unsigned char senddata[128];
	int i;
		
	OSI_LOGI(0, "AUTO_BLE fibo_ble_task_test");
	iRet = api_blueToothInit();

	btOpen_lib();
	data = (uint8_t *)malloc(256);
	memset(data,0,256);
	while(1)
	{
		
		lenth = btRecv_lib(data,32,10);
		if(total >= 20)
		{
			
			memset(senddata, 0, sizeof(senddata));
			sprintf(senddata,"rec:%s",data);
			btSend_lib(senddata, total+4);
			OSI_PRINTFI("AUTO_BLE btRecv_lib total:%d, last str data:%s" ,total, data);
			total = 0;
			memset(data,0,256);
		}
		else if(lenth > 0)
		{
			total+=lenth;
		}
		else
		{
			OSI_PRINTFI("AUTO_BLE btRecv_lib waitting data total=%d",total);
		}				
		sysDelayMs(10);
	}
	
}


#if 0
void * appimg_enter(void *param)
{
    OSI_LOGI(0, "application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

	if(g_fibo_ble_queue == 0)
    {
		g_fibo_ble_queue = fibo_queue_create(20, sizeof(int));
	}

    //fibo_thread_create(prvThreadEntry, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	fibo_thread_create(fibo_ble_task, "fibo_ble_task", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	fibo_thread_create(fibo_ble_task_test, "fibo_ble_task_test", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}

void * appimg_enter(void *param)
{
    OSI_LOGI(0, "application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

	if(g_fibo_ble_queue == 0)
    {
		g_fibo_ble_queue = fibo_queue_create(20, sizeof(int));
	}

    fibo_thread_create(ull_ble_task, "mythread", 1024*4, NULL, OSI_PRIORITY_NORMAL);
	fibo_thread_create(fibo_ble_task, "fibo_ble_task", 1024*4, NULL, OSI_PRIORITY_NORMAL);
    return (void *)&user_callback;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
#endif
