
#include "LC610N_api_picc.h"



/*
*@Brief:		对非接触卡模块上电并复位,检查复位后模块初始状态是否正常。 
*@Param IN:		无				
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int piccOpen_lib(void)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x01, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
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

int piccAttach( unsigned char  ucMode, unsigned char* pucOther)
{
	int iRet = 0;
	int iCmdLen = 6;
	int output_len = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x05, ucMode, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) {
		if(pucOther)
		{
		    output_len = retfrm.data[5]<<8 | retfrm.data[4];
			memcpy(pucOther, retfrm.data + 6 , output_len);
		}
		iRet = RET_RF_OK;
	}
	else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
    }
	else
	{
	    if(pucOther)
		{
			memcpy(pucOther, retfrm.data + 6 , 4);
	    }
		//iRet = -iRet;
	}
	fibo_free(retfrm.data);
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		按指定的模式搜寻 PICC 卡片;搜到卡片后,将其选中并激活。感应区内不允许多卡存在。 
*@Param IN:		ucMode[输入]  0x00 搜寻 A 型卡和 B 型卡一次, 此模式适用于需 要增强多卡检测功能的场合。该模式是符合 ISO14443 规范的寻卡模式； 
							 0x01 搜寻 A 型卡和 B 型卡一次；此模式为 EMV 寻卡模式，通常使用该模式； 
							 ‘a’或‘A’ 只搜寻 A 型卡一次； 
							 ‘b’或‘B’ 只搜寻 B 型卡一次； 
							 ‘m’或‘M’ 只搜寻 M1 卡一次； 
							 ‘c’或‘C’ 只搜寻 felica 卡一次； 
							 ‘d’或‘D’ 只搜寻身份证一次； 
							 其它值 保留 
 			 
*@Param OUT:		pucCardType[输出] 存放卡片类型的缓冲区指针，可为 NULL 目前均返回一字节的类型值 
								 ‘A’ 搜寻到 A 型卡 
									‘B’ 搜寻到 B 型卡 
								 ‘M’ 搜寻到 M1 卡 
								 ‘C’  搜寻到 Felica 卡 
								 ‘D’  搜寻到身份证 
					pucSerialInfo [输出] 存放卡片序列号的缓冲区指针，可为 NULL。 
					pucOther [输出] 存放详细错误代码、卡片响应信息等内容的缓冲区指针， 可为 NULL 

*@Return:		0:成功; <0:失败
*/
#if 1
int piccDetect_lib( unsigned char  ucMode, unsigned char* pucCardType, unsigned char* pucSerialInfo, unsigned char* pucOther )
{
	int iRet = 0;
	int iCmdLen = 6;
	int offset = 0;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x20, ucMode, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) 
	{
		if(pucCardType)
		{
			memcpy(pucCardType, retfrm.data + 6 , 1);
			offset = 7;
		}
		if(pucSerialInfo)
		{
			memcpy(pucSerialInfo, retfrm.data + offset , retfrm.data[7] + 1);
			offset += (retfrm.data[7] + 1);
		}
		if(pucOther)
		{
			sysLOG(API_LOG_LEVEL_2, "  offset :%d  retfrm.data[offset+1] = %d\r\n", offset, retfrm.data[offset]);
			memcpy(pucOther, retfrm.data+offset , retfrm.data[offset]);
		}
	    fibo_free(retfrm.data);
	    //激活卡片
	    //iRet = piccAttach(pucCardType, pucOther);
	    //iRet=piccAttach(ucMode, pucOther);
		iRet = RET_RF_OK;
	}
    else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
    }
	
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

#else
int piccDetect_lib( unsigned char  ucMode, unsigned char* pucCardType, unsigned char* pucSerialInfo, unsigned char* pucOther )
{
	int iRet = 0;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x03, ucMode, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) {
		if(pucCardType)
		{
			memcpy(pucCardType, retfrm.data + 6 , 1);
		}
		if(pucSerialInfo)
		{
			memcpy(pucSerialInfo, retfrm.data + 7 , retfrm.data[7] + 1);
		}
	    fibo_free(retfrm.data);
	    //激活卡片
	    //iRet = piccAttach(pucCardType, pucOther);
	    iRet=piccAttach(ucMode, pucOther);
		//iRet = RET_RF_OK;
	}
    else if(retfrm.length >= 10)
    {
        iRet = retfrm.data[6] | retfrm.data[7]<<8 | retfrm.data[8]<<16 | retfrm.data[9]<<24;
    }
	else
	{
	    if(pucOther)
		{
		    memcpy(pucOther, retfrm.data + 6 , 4);
	    }
		fibo_free(retfrm.data);
	}
	
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}
#endif
/*
*@Brief:		在指定的通道上,向卡片发送 APDU 格式的数据,并接收响应 
*@Param IN:		*ApduSend[输入] 发送给 PICC 卡命令数据结构
*@Param OUT:	ApduResp[输出] 
*@Return:		0:成功; <0:失败
*/
int piccIsoCommand_lib(APDU_SEND_LIB *ApduSend, APDU_RESP_LIB *ApduResp)
{
	int iRet = 0;
	int apduLen = 2+4+ApduSend->Lc+2;
	int iCmdLen = 6 + apduLen;
	
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x07, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 32);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, ApduSend, apduLen-2);//copy apdu data except le
	
	ucCmd[iCmdLen-1] = 0xFF & (ApduSend->Le >>8);//copy le to buffer little
	ucCmd[iCmdLen-2] = 0xFF & (ApduSend->Le);
#ifdef PRINT_API_CMD
	char* caShow = (char*) fibo_malloc(iCmdLen * 2 + 1);
	memset(caShow, 0, sizeof(caShow));
	HexToStr(ucCmd, iCmdLen, caShow);
	sysLOG(API_LOG_LEVEL_2, " le:%d, ucCmd = %s\r\n",ApduSend->Le, caShow);
	fibo_free(caShow);
#endif	
	Frame frm,retfrm;
	iRet = frameFactory(ucCmd,&frm,0x40, iCmdLen,0x01,0x00);
	fibo_free(ucCmd);
	if(iRet < 0) {
		goto RET_END;
	}
	iRet = transceiveFrame(frm, &retfrm, 2000);
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

/*
*@Brief:		依据指定的模式,向卡片发送停机指令；或者发送停活指令；或者复位载波，并 判断卡片是否已经移开感应区。
*@Param IN:		cid [输入] 用于指定卡片逻辑通道号;该通道号由PiccDetect( )的 CID 参数项输出,其取值范围为 0~14,目前取值均为 0。
				mode [输入]  ‘h’或‘H’ 意为 HALT，仅向卡片发送停活指令后就退出；该过程不执行卡移开检测 
								‘r’或‘R’ REMOVE， 向卡片发送停活指令，并执行卡移开检测； 
 								‘e’或‘E’ 符合 EMV 非接规范的移卡模式 复位载波，并执行卡移开检测 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccRemove_lib(unsigned char mode, unsigned char cid)
{
	int iRet = 0;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x11, mode, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) 
	{
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
*@Brief:		将载波关闭 5ms~10.1ms，场中所有的卡片均会掉电。复位后，卡片必须重新 PiccDetect 才能访问
*@Param IN:		无
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccReset_lib(void)
{
	int iRet = 0;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0x0d, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) 
	{
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
*@Brief:		关闭 PICC 模块
*@Param IN:		无
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccClose_lib(void)
{
	int iRet = 0;
	int iCmdLen = 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X02, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
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
	sysLOG(API_LOG_LEVEL_2, "  iRet = 0x%x, retfrm.length = %d\r\n", iRet, retfrm.length);
	if(0x9000 == iRet) 
	{
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
*@Brief:		控制 RF 模块的 4 个 LED 灯的点亮和熄灭状态。 
*@Param IN:		ucLedIndex[输入] 灯索引，参考LED_INDEX_ID，RED= 0x01,BLUE = 0x02,GREEN =0x03,YELLOW = 0x04
				ucOnOff[输入] 点亮或熄灭标志 0：熄灭 1：点亮 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
void piccLight_lib(uchar ucLedIndex,uchar ucOnOff)
{
	int iRet = 0;
	if((ucOnOff != 0) && (ucOnOff != 1) || (ucLedIndex < PICCLEDRED) || (ucLedIndex > LEDYELLOW))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}

	if(ucLedIndex > PICCLEDYELLOW)
	{
		iRet = hal_ledCtl(ucLedIndex, ucOnOff);
	}
	else
	{
	
		if(ucOnOff == 0)
		{
			ucOnOff = 2;//vos 2 熄灭
		}
		int iCmdLen = 6 + 1;
		unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X0a, ucOnOff, iCmdLen-6, (iCmdLen -6) >> 8};
		unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
		memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
		ucCmd[6] = ucLedIndex;
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
		if(0x9000 == iRet) 
		{
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
	}
	
RET_END:
	sysLOG(API_LOG_LEVEL_2, "  RET_END,iRet = %d\r\n", iRet);
	return iRet;
}

/*
*@Brief:		与卡片进行APDU数据交互,终端将数据直接发送给卡片,并接收卡片的应答数据。
*@Param IN:		iTxN[输入] 待发送的命令数据长度 
				*ucpSrc[输入] 待发送的命令数据 
*@Param OUT:	*ipRxN[输出] 接收到卡片的数据长度 
				*ucpDes[输出] 接收到的卡片数据 
*@Return:		0:成功; <0:失败
*/
int piccCmdExchange_lib(unsigned int iTxN,  uchar* ucpSrc, unsigned int* ipRxN,  uchar* ucpDes)
{
	int iRet = 0;
	int iCmdLen = 6 + iTxN;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X08, 0x00, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	memcpy(ucCmd + 6, ucpSrc, iTxN);
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
	if(0x9000 == iRet) 
	{
	    memcpy(ipRxN, retfrm.data + 4, 2);
	    if(retfrm.length >= (6 + *ipRxN))
        {
			memcpy(ucpDes, retfrm.data + 6, *ipRxN);
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

/*
*@Brief:		验证 M1 卡访问时读写相应模块需要提交的 A 密码或 B 密码
*@Param IN:		ucType [输入] ‘A’或‘a’提交的是 A 密码； 
								‘B’或‘b’提交的是 B 密码。 
				ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucPwd [输入] 指向提交的密码缓冲区。 6字节
				*pucSerialNo[ 输 入] 此参数未启用
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1Authority(unsigned char  ucType,        unsigned char  ucBlkNo, unsigned char *pucPwd, unsigned char *pucSerialNo )
{
	int iRet = 0;
	if((ucType != 'A') && (ucType != 'a') && (ucType != 'B') && (ucType != 'b'))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}
	if((ucBlkNo < 0) || (ucBlkNo > 63))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}	
	int iCmdLen = 6 + 1 + 1 + 6;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X0c, 0x01, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = ucType;
	ucCmd[7] = ucBlkNo;
	memcpy(ucCmd + 8, pucPwd, 6);
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
	if(0x9000 == iRet) 
	{
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
*@Brief:		读取 M1 卡指定块的内容(共 16 字节)。
*@Param IN:		ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucBlkValue[输出] 指向待存取块内容的缓冲区首址；该缓冲区至少应分配 16 字节。 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1ReadBlock(unsigned char ucBlkNo, unsigned char *pucBlkValue)
{
	int iRet = 0;
	if((ucBlkNo < 0) || (ucBlkNo > 63))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}	
	int iCmdLen = 6 + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X0c, 0x02, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = ucBlkNo;
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
	if(0x9000 == iRet) 
	{
	    memcpy(pucBlkValue, retfrm.data + 6, 16);
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
*@Brief:		向 M1 卡指定块写入指定的内容(共 16 字节)
*@Param IN:		ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucBlkValue[输入] 指向待存取块内容的缓冲区首址；该缓冲区至少应分配 16 字节。 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1WriteBlock(unsigned char ucBlkNo, unsigned char *pucBlkValue)
{
	int iRet = 0;
	if((ucBlkNo < 0) || (ucBlkNo > 63))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}	
	int iCmdLen = 6 + 1 + 16;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X0c, 0x03, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = ucBlkNo;
	memcpy(ucCmd + 7, pucBlkValue, 16);
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
	if(0x9000 == iRet) 
	{
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
*@Brief:		对 M1 卡的指定数据块 ucBlkNo 进行充/减值/备份操作，将操作后的值更新到 另一个指定的数据块 ucUpdateBlkNo。 
*@Param IN:		ucType [输入] ‘+’充值,加号 ‘—’减值,减号 ‘>’存/备份操作,大于号 
				ucBlkNo [输入] 用于指定访问的块号。0~63。 
				*pucValue [输入] 指向待存取块内容的缓冲区首址；该缓冲区至少应分配 4字节。 
				ucUpdateBlkNo [输入] 指定操作结果最终写入到的块号。 
*@Param OUT:	无
*@Return:		0:成功; <0:失败
*/
int piccM1Operate(unsigned char  ucType,       unsigned char  ucBlkNo,  unsigned char *pucValue,  unsigned char  ucUpdateBlkNo)
{
	int iRet = 0;
	if((ucBlkNo < 0) || (ucBlkNo > 63))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}
	if((ucType!='+') && (ucType!='-') && (ucType!='>'))
	{
		iRet = RET_RF_ERR_PARAM;
		goto RET_END;
	}
	int iCmdLen = 6 + 1 + 1 + 4 + 1;
	unsigned char ucCmdHead[6] = {0x00, 0xa3, 0X0c, 0x04, iCmdLen-6, (iCmdLen -6) >> 8};
	unsigned char* ucCmd = (unsigned char*) fibo_malloc(iCmdLen + 1);
	memcpy(ucCmd, ucCmdHead, sizeof(ucCmdHead));
	ucCmd[6] = ucType;
	ucCmd[7] = ucBlkNo;
	memcpy(ucCmd + 8, pucValue, 4);
	ucCmd[12] = ucUpdateBlkNo;
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
	if(0x9000 == iRet) 
	{
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
*@Brief:		验证M1卡访问时读写相应模块需要提交的A密码或B密码
*@Param IN:		pucPwd [输入]	4字节秘钥
*@Param OUT:	pucPack[输出]	2字节秘钥应答
*@Return:		0:成功; <0:失败
*/
int piccNtagAuthority_lib(unsigned char *pucPwd,  unsigned char *pucPack)
{
	return RET_RF_ERR_PARAM;
}

/*
*@Brief:		指定地址范围 读取卡片内容
*@Param IN:		ucStartAddr[输入]	起始地址页，范围0~0x2C
				ucStartEnd[输入]	起始地址页，范围0~0x2C
*@Param OUT:	pucPack[输出]	卡片的输出数据
*@Return:		0:成功; <0:失败
*/
int piccNtagRead_lib( unsigned char ucStartAddr, unsigned char ucEndaddr,	unsigned char *pucData)
{
	return RET_RF_ERR_PARAM;
}

/*
*@Brief:		指定地址写入数据
*@Param IN:		BlockNo [输入]	页地址
				pucPack [输入]	需写入内容，长度4字节
*@Param OUT:	
*@Return:		0:成功; <0:失败
*/
int piccNtagWrite_lib( unsigned char BlockNo, unsigned char *pucPack)
{
	return RET_RF_ERR_PARAM;
}


void apiPiccTest()
{
#if 1
	unsigned char  ucMode = 0;
	unsigned char* pucCardType[200] = {0};
	unsigned char* pucSerialInfo[200] = {0};
	unsigned char pucOther[200] = {0};
	int iRet ;

	APDU_SEND_LIB ApduSend;
	APDU_RESP_LIB ApduResp;

	piccOpen_lib();

	iRet = piccDetect_lib(ucMode, pucCardType, pucSerialInfo, pucOther);
    sysLOG(API_LOG_LEVEL_2, "  piccDetect_lib,iRet = %d, pucCardType=%s, pucSerialInfo=%s, pucOther=%s\r\n", iRet, pucCardType, pucSerialInfo, pucOther);
    if(iRet == 0)
	{
	    piccLight_lib(0x03, 1);
		ApduSend.Cmd[0] = 0x00;
		ApduSend.Cmd[1] = 0xa4;
		ApduSend.Cmd[2] = 0x04;
		ApduSend.Cmd[3] = 0x00;
		memset(ApduSend.DataIn, 0, sizeof(ApduSend.DataIn));
		strcpy((char*)ApduSend.DataIn, "1PAY.SYS.DDF01");
		memset(ApduResp.DataOut, 0, sizeof(ApduResp.DataOut));
		ApduSend.Lc = 14;
		ApduSend.Le = 256;
		iRet = piccIsoCommand_lib(&ApduSend, &ApduResp);
		sysLOG(API_LOG_LEVEL_2, "piccIsoCommand_lib, iRet = %d\r\n", iRet);
	}
	else
	{
	    piccLight_lib(0x01, 1);
	}

	
	piccClose_lib();

	piccLight_lib(0x03, 2);
	piccLight_lib(0x01, 2);
#else
    LedTwinkle_lib(2, 0, 50);
	LedTwinkle_lib(3, 0, 50);
	LedTwinkle_lib(4, 1, 50);
    sysDelayMs(2000);
    LedTwinkle_lib(3, 2, 0);
	LedTwinkle_lib(4, 2, 1);
#endif
}

/*
*@Brief:		LED定时闪烁及同异步控制。 
*@Param IN:		ledNum：[输入] 灯索引，参考LED_INDEX_ID,RED= 0x01,BLUE = 0x02,GREEN =0x03,YELLOW = 0x04	；
				type：type: 0-低频闪      1-高频闪 2-取消闪烁；
				count：闪烁次数，0：一直闪烁		
*@Param OUT:	无 
*@Return:		0:成功; <0:失败
*/
int LedTwinkle_lib(int ledNum, int type, int count)
{
	int iRet = RET_RF_ERR_PARAM;
	int iCmdLen = 0x0F;
	unsigned char OnTime[2]; 
	unsigned char Times[2];
	unsigned char blinkType = 0x01;
	unsigned char bFLICK = 0x01;//默认闪烁

	if(ledNum > PICCLEDYELLOW)
	{
		switch(type)
		{
			case 0:
				iRet = hal_ledRun(ledNum, 2, count);
			break;
			case 1:
				iRet = hal_ledRun(ledNum, 3, count);
			case 2:
				iRet = hal_ledRun(ledNum, 0, count);
			default:
				
			break;
		}
		
		if(iRet > 0)
			iRet = 0;
		else
			iRet = RET_RF_ERR_PARAM;

		return iRet;
	}
	
	if(count == 0)
	{
		blinkType = 0x02;
	}
	if(type == 1)
	{
		 OnTime[0] = 0xc8;
		 OnTime[1] = 0x00;
	}
	else if(type == 0)
	{
		 OnTime[0] = 0xDC;
		 OnTime[1] = 0x05;
	}
	else if(type == 2)
	{
	     bFLICK = 0x02;//取消闪烁
		 OnTime[0] = 0x00;
		 OnTime[1] = 0x00;
	}
	Times[0] = count & 0xff;
	Times[1] = count >> 8;
	unsigned char ucCmdHead[15] = {0x00, 0xa3, 0x0b, 0x02, iCmdLen-6, (iCmdLen -6) >> 8, bFLICK, ledNum, blinkType, OnTime[0], OnTime[1],OnTime[0], OnTime[1],Times[0],Times[1]};
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


