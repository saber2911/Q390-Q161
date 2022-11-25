

#ifndef LC610N_API_CALC_H
#define LC610N_API_CALC_H

//open 和 close 需成对出现
/**
 * [Function]       btOpen_lib
 * [Description]    模块打开
 * [param]          无
 * [return]         
 * [modify]                
 */
int btOpen_lib();
/**
 * [Function]       btClose_lib
 * [Description]    模块关闭
 * [param]          无
 * [return]         
 * [modify]                
 */
int btClose_lib();

/**
 * [Function]       btSend_lib
 * [Description]    蓝牙发送数据
 * [param]          pucData    数据指针
                    uiLen      数据长度
 * [return]         
 * [modify]                
 */
int btSend_lib(unsigned char *pucData, unsigned int uiLen);
/**
 * [Function]       btRecv_lib
 * [Description]    蓝牙发送数据
 * [param]          pucBuff              接收缓存指针
                    uiSize               缓存大小
                    uiTimeOut           超时时间，单位ms
 * [return]         
 * [modify]                
 */
int btRecv_lib(unsigned char *pucBuff, unsigned int uiSize, unsigned int uiTimeOut);
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
int btCtrl_lib(unsigned int uiCmd, void *pArgIn, unsigned int uiSizeIn, void *pArgOut, unsigned int uiSizeOut);


#endif

