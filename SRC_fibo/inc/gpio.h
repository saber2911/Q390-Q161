#ifndef _gpio_H_
#define _gpio_H_
#include "comm.h"

extern uint8 g_iType_machine;
extern uint8 WIFI_PWR_EN_GPIO;
extern uint8 CHG_ING_GPIO;
extern uint8 CHG_EN_GPIO;
extern uint8 CAM2V8EN_GPIO;
extern uint8 CAMLEDEN_GPIO;
extern uint8 CBLCD_CS_GPIO;
extern uint8 CBLCD_WR_GPIO;
extern uint8 CBLCD_DATA_GPIO;
extern uint8 SEGLCD_BLEN_GPIO;
extern uint8 SEGLCDSPICSN_GPIO;
extern uint8 LED_RED_GPIO;
extern uint8 LED_BLUE_GPIO;
extern uint8 LED_YELLOW_GPIO;
extern uint8 SEGLCD_BLEN_GPIO;
extern uint8 g_iType_machine;
void get_machine_type(void);

#endif