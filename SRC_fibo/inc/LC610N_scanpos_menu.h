#ifndef LC610N_SCANPOS_MENU_H
#define LC610N_SCANPOS_MENU_H

#include "comm.h"

#define __MENU_DEBUG		0

int iMenuFlag;//菜单状态 0 未进入，1 已进入菜单， 2 已退出菜单

void SelectMode(void);

typedef struct{
    unsigned char   cmd[4];
    int             lc;
    unsigned char   data_in[512];
    int             le;
}ST_APDU_REQ;
typedef struct{
    unsigned short  len_out;//modify by lbb
    unsigned char   data_out[512];
    unsigned char   swa;
    unsigned char   swb;
}ST_APDU_RSP;
#endif

