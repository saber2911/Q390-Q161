#include "LC610N_api_icc.h"


int iccInit(unsigned char ucV)
{
	int iRet = ERR_SCC_PARAM;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa2, 0x01, ucV, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		初始化 IC 卡功能，并选定电压 
*@Param IN:		slot[输入] 	bit 0~3 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2 
							bit 4~7 上电电压 1 --1.8V 2 -- 3.0V 3-5.0V      0 默认：5.0V 
							bit 8~11 PPS 自适应操作 1 -- PPS 其它--NO PPS 
							bit 12~15 上电复位数据速率 默认--9600 1 -- 38400 
							bit 16~19 协议模式 默认--EMV 模式 1 -- ISO7816 模式 
*@Param OUT:	ATR [输出] 卡片复位应答。(至少需要 33bytes 的空间) 
							其内容为长度(1 字节)+复位应答内容[输出] 
*@Return:		0:成功; <0:失败
*/
int iccPowerUp_lib(unsigned int slot, unsigned char *atr)
{
	int iRet = ERR_SCC_PARAM;
	int iSlot = slot & 0xF;
	int iV = (slot >> 4) & 0xF;
	sysLOG(API_LOG_LEVEL_2, " iSlot=%d, iV=%d\r\n", iSlot, iV);
	if((iSlot < 0) || (iSlot > 2) || (iV < 0) || (iV > 3))
	{
		return ERR_SCC_PARAM;
	}
	//
	iRet = iccInit(iV);
	if(iRet != ERR_SCC_SUCCESS)
	{
		return iRet;
	}		
	
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa2, 0x07, iSlot, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	if(0x9000 == iRet) {
		int output_len = retfrm.data[5]<<8 | retfrm.data[4];
		memcpy(atr, retfrm.data + 6 , output_len);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		对指定卡座中的卡片下电
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int iccClose_lib(unsigned int slot)
{
	int iRet = ERR_SCC_PARAM;
	if((slot < 0) || (slot > 2))
	{
		return ERR_SCC_PARAM;
	}
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa2, 0x08, slot, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		设置 IccIsoCommand 函数是否自动发送 GET RESPONSE 指令。 
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2
				autoresp [输入] 标志：1：自动发送;0：不自动发送;其他：无效
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int iccAutoResp_lib(unsigned char slot, uchar autoresp)
{
	int iRet = ERR_SCC_PARAM;
	if((slot < 0) || (slot > 2) || (autoresp < 0) || (autoresp > 1))
	{
		return ERR_SCC_PARAM;
	}
	int iCmdLen = 6 + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xa2, 0x0c, slot, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = autoresp;

#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

#if 1
/*
*@Brief:		IC 卡操作函数。该函数支持 IC 卡通用接口协议(T=0 及 T=1)。
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2
				ApduSend[输入] 发送给 IC 卡命令数据结构
*@Param OUT:	ApduResp[输出]
*@Return:		0:成功; <0:失败
*/
int iccIsoCommand_lib(uchar slot, APDU_SEND_LIB *ApduSend, APDU_RESP_LIB *ApduResp)
{
	int iRet = ERR_SCC_PARAM;
	if((slot < 0) || (slot > 2))
	{
		return ERR_SCC_PARAM;
	}
	int iCmdLen = 6 + sizeof(APDU_SEND_LIB);
	unsigned char ucCmdHead[6] = {0x00, 0xa2, 0x09, slot, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, ApduSend, sizeof(APDU_SEND_LIB));
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif	
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
	fibo_free(frm.data);
	if(iRet <0) {
		goto RET_END;
	}
	iRet=retfrm.data[2]<<8 | retfrm.data[3];
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) {
		if(ApduResp)
		{
			memcpy(&ApduResp->LenOut, retfrm.data + 6 , 2);
			sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, ApduResp->LenOut = %d\r\n", iRet, ApduResp->LenOut);
			memcpy(ApduResp->DataOut, retfrm.data + 8 , ApduResp->LenOut);
			ApduResp->SWA = retfrm.data[8 + ApduResp->LenOut];
			ApduResp->SWB = retfrm.data[8 + ApduResp->LenOut + 1];
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}
#endif

/*
*@Brief:		检查指定的卡座内是否有卡
*@Param IN:		slot[输入] 	slot - 需要初始化卡通道号 低 4bit 有效 0 -- IC 卡 1 -- PSAM1 2 -- PSAM2
*@Param OUT:	
*@Return:		0:有卡插入; <0:失败
*/
int iccGetPresent_lib(uchar slot)
{
	int iRet = ERR_SCC_PARAM;
	if((slot < 0) || (slot > 2))
	{
		return ERR_SCC_PARAM;
	}
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa2, 0x03, slot, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));

#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, "  ucCmd = %s\r\n", caShow);
	fibo_free(caShow);
#endif

	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 1000);
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
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}



void apiiccTest()
{

}

