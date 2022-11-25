#include "gpio.h"

uint8 g_iType_machine;
uint8 WIFI_PWR_EN_GPIO=0;
uint8 CHG_ING_GPIO=0;
uint8 CHG_EN_GPIO=0;
uint8 CAM2V8EN_GPIO=0;
uint8 CAMLEDEN_GPIO=0;
uint8 CBLCD_CS_GPIO=0;
uint8 CBLCD_WR_GPIO=0;
uint8 CBLCD_DATA_GPIO=0;
uint8 SEGLCD_BLEN_GPIO=0;
uint8 SEGLCDSPICSN_GPIO=0;
uint8 LED_RED_GPIO=0;
uint8 LED_BLUE_GPIO=0;
uint8 LED_YELLOW_GPIO=0;
void get_machine_type(void)
{
    
    if (g_iType_machine == 0)
    {
        sysLOG(BASE_LOG_LEVEL_1, "390 enter \r\n");
        WIFI_PWR_EN_GPIO=121; //√

        CHG_ING_GPIO=127; //√
        CHG_EN_GPIO=126; //√

        CAM2V8EN_GPIO=33;  //√
        CAMLEDEN_GPIO=13;  //√

        #if 0
        #define CAMRST_GPIO			72  //camera RST 
        #endif 


        //挂在在SE上
        #if 0
        #define CBLCD_CS_GPIO		37
        #define CBLCD_WR_GPIO		121
        #define CBLCD_DATA_GPIO		119
        #define SEGLCD_BLEN_GPIO	133//副屏的背光
        #define SEGLCDSPICSN_GPIO	37//副屏SPI片选
        #define LED_RED_GPIO		129
        #define LED_BLUE_GPIO		41
        #define LED_YELLOW_GPIO		6
        #endif

    }
    else if(g_iType_machine == 1)
    {
        sysLOG(BASE_LOG_LEVEL_1, "161 enter \r\n");
        /*-------USER_GPIO----------PIN_INDEX---START--*/

        WIFI_PWR_EN_GPIO=42;

        CHG_ING_GPIO=141;
        CHG_EN_GPIO=142;

        #if 0
        #define CAM2V8EN_GPIO		33
        #define CAMLEDEN_GPIO		13
        #define CAMRST_GPIO			72//camera RST
        #endif

        CBLCD_CS_GPIO=33;
        CBLCD_WR_GPIO=121;
        CBLCD_DATA_GPIO=119;

        SEGLCD_BLEN_GPIO=133;//副屏的背光
        SEGLCDSPICSN_GPIO=13;//副屏SPI片选

        LED_RED_GPIO=129;
        LED_BLUE_GPIO=41;
        LED_YELLOW_GPIO=6;

    }
    


}