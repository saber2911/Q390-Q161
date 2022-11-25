#include "LC610N_hal_dcep.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int BalVoucherFileInit(void);    
//#include <assert.h>
//#include "tlvparse.h"

unsigned char g_DCEPwallFileInitFlag = 0;        //0：DCEPWALLFILENAME文件没有初始化操作；1：已初始化
unsigned char g_admBlnFileInitFlag = 0;          //0：余额凭证管理文件没有初始化操作；1：已初始化
unsigned char g_listBlnFileInitFlag = 0;         //0：余额凭证列表文件没有初始化操作；1：已初始化
unsigned char g_admBillFileInitFlag = 0;         //0: 交易记录管理文件没有初始化操作；1：已初始化
unsigned char g_listBillFileInitFlag = 0;        //0：交易记录列表文件没有初始化操作；1：已初始化
unsigned char g_atcFileInitFlag = 0;             //0：atc文件没有初始化操作；1：已初始化
unsigned char currencyStringInitFlag = 0;        //0：币串文件没有初始化操作；1：已初始化


/*无符号长整形转字符型*/
char * ultoaUsr(unsigned long value, char *string, int radix)
{
    char tmp[33];
    char *tp = tmp;
    long i;
    unsigned long v = value;
    char *sp;

    if (radix > 36 || radix <= 1)
    {
        //__set_errno(EDOM);
        return 0;
    }

    while (v || tp == tmp)
    {
        i = v % radix;
        v = v / radix;
        if (i < 10)
        *tp++ = i+'0';
        else
        *tp++ = i + 'a' - 10;
    }
    if (string == 0)
    string = (char *)malloc((tp-tmp)+1);
    sp = string;

    while (tp > tmp)
    *sp++ = *--tp;
    *sp = 0;
    return string;
}

//#define inline __inline
/**
 * 检查bit位是否为真
 * @param  value [待检测值]
 * @param  bit   [1~8位 低~高]
 * @return       [1 真 0 假]
 */
static inline int CheckBit(unsigned char value, int bit)
{
	unsigned char bitvalue[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	if((bit >= 1)&&(bit <= 8))
	{
		 if(value & bitvalue[bit-1])
		 	return(1);
		 else
		 	return(0);
	}
	else
	{
		//dprintf("FILE: %s LINE: %d -- 传入的函数参数错误! bit=[%d]\n", 	__FILE__, __LINE__, bit);
		return(-1);
	}
}

/**
 * Create TLVNode
 * @return  [description]
 */
static struct TLVNode* TLV_CreateNode(void)
{
	struct TLVNode* node = (struct TLVNode *)malloc(sizeof(*node));
	memset(node,0,sizeof(*node));
	return node;
}

/**
 * Free TLVNode Memory
 * @param node [node]
 */
void TLV_Free(struct TLVNode* node)
{
	int i;
	if(node == NULL)
	{
		return;
	}

	if(node->Value)
		free(node->Value);

	for(i=0;i<node->SubCount;i++)
	{
		TLV_Free(node->Sub[i]);
		node->Sub[i] = NULL;
	}

	if(node->Next)
	{
		TLV_Free(node->Next);
		node->Next = NULL;
	}

	free(node);
}

/**
 * 打印TLV子结点信息(深度优先)
 * @param parent [description]
 * @param step   [description]
 */
static void TLV_DebugSubNode(struct TLVNode* parent,int step)
{
	int i,j;
	for(i=0;i<parent->SubCount;i++)
	{
		struct TLVNode* sub = parent->Sub[i];
		//dprintf("[%d]SUBTAG=%04X LEN=[%d] ADDR=[%p]\n",step,sub->Tag,sub->Length,sub);
		for(j=0;j<sub->Length;j++)
		{
			//dprintf("%02X ",sub->Value[j]);
		}
		//dprintf("\n");
		//dprintf("[%d]MoreFlag=%d SubFlag=%d\n",step,sub->MoreFlag,sub->SubFlag);
		//dprintf("---------------------------\n");

		if(sub->SubFlag == 1)
		{
			TLV_DebugSubNode(sub,step+1);
		}
	}
}

/**
 * DEBUG TLVNode
 * @param node [description]
 */
void TLV_Debug(struct TLVNode* node)
{
	int i;
	//dprintf("\n************************BEGIN TLV DEBUG************************\n");
	//dprintf("TAG=%04X LEN=[%d] ADDR=[%p]\n",node->Tag,node->Length,node);
	for(i=0;i<node->Length;i++)
	{
		//dprintf("%02X ",node->Value[i]);
	}
	//dprintf("\n");
	//dprintf("MoreFlag=%d SubFlag=%d\n",node->MoreFlag,node->SubFlag);
	if(node->SubFlag == 1)
	{
		TLV_DebugSubNode(node,1);
	}
	if(node->Next)
	{
		//dprintf("Next=[%p]\n",node->Next);
	}
	//dprintf("************************END TLV DEBUG************************\n");
}

/**
 * 解析一条数据到TLVNode结构
 * @param  buf  [description]
 * @param  size [description]
 * @param  sflag [description]
 * @return      [description]
 */
static struct TLVNode* TLV_Parse_One(unsigned char* buf,int size)
{
	int index = 0;
	int i;
	uint16_t tag1,tag2,tagsize;
	uint16_t len,lensize;
	unsigned char* value;
	struct TLVNode* node = TLV_CreateNode();

	tag1 = tag2 = 0;
	tagsize = 1;
	tag1 = buf[index++];
	if((tag1 & 0x1f) == 0x1f)
	{
		tagsize++;
		tag2 = buf[index++];
		//tag2 b8 must be 0!
	}
	
	if(tagsize == 1)
		node->Tag = tag1;
	else
		node->Tag = (tag1<<8) + tag2;
	
	node->TagSize = tagsize;

	//SubFlag
	node->SubFlag = CheckBit(tag1,6);

	//L zone
	len = 0;
	lensize = 1;
	len = buf[index++];
	if(CheckBit(len,8) == 0)
	{
		node->Length = len;
	}
	else
	{
		lensize = len & 0x7f;
		len = 0;
		len = (uint16_t)buf[index++];
		for(i=1;i<lensize;i++)
		{
			//不确定长度是否是小端编码?
			//len += (uint16_t)buf[index++] << (i*8);//wait to fix here
			len = (len << (i*8))+(uint16_t)buf[index++];
		}
		//fix lensize
		lensize++;
	}
	node->Length = len;
	node->LengthSize = lensize;

	//V zone
	value = (unsigned char *)malloc(len);
	memcpy(value,buf+index,len);
	node->Value = value;
	index += len;

	if(index < size)
	{
		node->MoreFlag = 1;
	}
	else if(index == size)
	{
		node->MoreFlag = 0;
	}
	else
	{
		//dprintf("Parse Error! index=%d size=%d\n",index,size);
	}

	return node;
}


/**
 * [TLV_Parse_More description]
 * @param  parent [description]
 * @return        [1 more 0 over]
 */
static int TLV_Parse_SubNodes(struct TLVNode* parent)
{
	//check have more
	int sublen = 0;
	int i;

	//have no subtag!
	if(parent->SubFlag == 0)
		return 0;

	for(i=0;i<parent->SubCount;i++)
	{
		sublen += (parent->Sub[i]->TagSize + parent->Sub[i]->Length + parent->Sub[i]->LengthSize);
	}

	if(sublen < parent->Length)
	{
		struct TLVNode* subnode = TLV_Parse_One(parent->Value+sublen,parent->Length-sublen);
		parent->Sub[parent->SubCount++] = subnode;
		return subnode->MoreFlag;
	}
	else
	{
		return 0;
	}
}


/**
 * 解析子段内嵌数据(广度优先)
 * @param parent [description]
 */
static void TLV_Parse_Sub(struct TLVNode* parent)
{
	int i;
	if(parent->SubFlag != 0)
	{
		//parse sub nodes.
		while(TLV_Parse_SubNodes(parent) != 0)
		{

		}
		
		for(i=0;i<parent->SubCount;i++)
		{
			if(parent->Sub[i]->SubFlag != 0)
			{
				TLV_Parse_Sub(parent->Sub[i]);
			}
		}
	}
}


/**
 * 解析数据,生成TLV结构
 * @param  buf  [数据]
 * @param  size [数据长度]
 * @return      [TLVNode]
 */
struct TLVNode* TLV_Parse(unsigned char* buf,int size)
{
	struct TLVNode* node = TLV_Parse_One(buf,size);
	TLV_Parse_Sub(node);

	return node;
}

/**
 * 拷贝一个节点到另一个结点(不包含Next)
 * @param dst [description]
 * @param src [description]
 */
static void TLV_Copy(struct TLVNode* dst,struct TLVNode* src)
{
	int i;
	memcpy(dst,src,sizeof(*src));
	dst->Value = (unsigned char*)malloc(src->Length);
	memcpy(dst->Value,src->Value,src->Length);

	for(i=0;i<src->SubCount;i++)
	{
		dst->Sub[i] = TLV_CreateNode();
		TLV_Copy(dst->Sub[i],src->Sub[i]);
	}
}

/**
 * 合并src结构到target
 * @param target [目标TLVNode]
 * @param src    [源TLVNode]
 */
void TLV_Merge(struct TLVNode* target,struct TLVNode* src)
{
	int i;
	int found = 0;
	struct TLVNode* tmpnode = target;
	//assert(target != NULL);
	while(tmpnode)
	{
		//只比较一级tag是否相同,子tag必须是不同的
		if(tmpnode->Tag == src->Tag)
		{
			found = 1;
			break;
		}

		if(tmpnode->Next)
			tmpnode = tmpnode->Next;
		else
			break;
	}

	if(found == 0)
	{
		tmpnode->Next = TLV_CreateNode();
		TLV_Copy(tmpnode->Next,src);
	}
	else
	{
		for(i=0;i<src->SubCount;i++)
		{
			tmpnode->Sub[tmpnode->SubCount] = TLV_CreateNode();
			TLV_Copy(tmpnode->Sub[tmpnode->SubCount],src->Sub[i]);
			tmpnode->SubCount++;
			tmpnode->SubFlag = 1;
		}
	}
}


/**
 * 在node中查找tag.
 * @param  node [TLVNode]
 * @param  tag  [tag标签]
 * @return      [NULL - 未找到]
 */
struct TLVNode* TLV_Find(struct TLVNode* node,uint16_t tag)
{
	int i;
	struct TLVNode* tmpnode;
	if(node->Tag == tag)
	{
		return node;
	}
	for(i=0;i<node->SubCount;i++)
	{
		tmpnode = NULL;
		tmpnode = TLV_Find(node->Sub[i],tag);
		if(tmpnode != NULL)
			return tmpnode;
	}
	if(node->Next)
	{
		tmpnode = NULL;
		tmpnode = TLV_Find(node->Next,tag);
		if(tmpnode != NULL)
			return tmpnode;
	}

	return NULL;
}

//#define TYPE_TRANCHAIN_INDEX	"hw_bal_voucher99997" 
//#define TYPE_BAL_COUNT_INDEX	"hw_bal_voucher99998" //99998 
//#define TYPE_BILL_COUNT_INDEX	"hw_bal_voucher99999" //99999 

//#define TYPE_BANLANCE_INDEX		"hw_bal_voucher" 

long bytesToLong(unsigned char *data,int len)
{
	int i;
	long blng;
	blng=data[0];
	for(i=1;i<len;i++)
	{
		blng = (blng << (8))+(long)data[i];
	}
	return blng;
}
long long bytesToLonglong(unsigned char *data,int len)
{
	int i;
	long long blng;
	blng=data[0];
	for(i=1;i<len;i++)
	{
		blng = (blng << (8))+ data[i];
	}
	return blng;
}

int longlongTobyte(long long Value, uint8_t *data,int size)
{
	int i;
	for(i=0;i<size;i++)
		data[size-i-1] = (Value >> (i*8))&0xFF;
	
	return i;
}

int getBalVoucherIndexTable1(unsigned int *indexTable) 
{
	/*unsigned char data[32];// = new byte[4096];
	int len = 0;
	int count = 0;
	int i;
	int fd = 0;// 

	fd = lfs2_fileOpen(TYPE_BAL_COUNT_INDEX,FILE_READWRITE);
	if(fd < 0)
	{
		return -1;
	}
	len = lfs2_fileRead(fd, data, 8);//read lenth
    if(len != 8 )
	{
		lfs2_fileClose(fd); 
		return -1;
	}
    count = bytesToLong(data, 8);

    for (i = 0; i<count; i++) 
	{
		lfs2_fileSeek(fd,(i+1)*8,FILE_SEEK_SET);
		memset(data,0,sizeof(data));
		len = lfs2_fileRead(fd, data, 8);
		if(len != 8 )
		{
			lfs2_fileClose(fd); 
			return -1;
		}
        indexTable[i] = bytesToLong(data, 8);
    }
	lfs2_fileClose(fd);
	return count;*/
    int iRet = 0;
    sDCEP_VoucherHead ADMVoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];

    memset(&ADMVoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 
    iRet = readBalVoucherAdmblnFile(voucherInfo, &ADMVoucherHead);
    if(iRet != 0)
    {
        return -1;
    }
    memcpy(indexTable, voucherInfo, sizeof(voucherInfo));

    return ADMVoucherHead.blnTtlCnt;
 }

extern struct TLVNode* TLV_Parse(unsigned char* buf,int size);
 
int getTagValuebyIndex(unsigned int index,unsigned char *value)
{
	struct TLVNode* node0;
	struct TLVNode* found;
	char ffs[128];
    char fileName[19];
    char* bufChar;
	unsigned char buf[2048];
	int size;
	int i;
	int fd = 0;//NULL; /* 需要注意 */
    int iRet = 0; 
    int fileSize = 0;
    unsigned char aucBalData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    uint32_t dir = 0;
	memset(ffs,0,sizeof(ffs));
    memset(buf,0,sizeof(buf));
	memset(aucBalData, 0x00, sizeof(aucBalData));
	/*sprintf(ffs,"%s%d",TYPE_BANLANCE_INDEX,index);	
	fd = lfs2_fileOpen(ffs,FILE_READWRITE);
	if(fd < 0)
	{
		return -1;
	}
	memset(buf,0,sizeof(buf));
	size = lfs2_fileRead(fd, buf,sizeof(buf));
	lfs2_fileClose(fd);
	if(size <= 0)
		return -1;*/

    memset(fileName, 0x00, sizeof(fileName));
    dir = index >> 28;
    if (dir == 0)
    {
        memcpy(fileName, "/ext/00/", 8);
    }
    else if (dir == 1)
    {
        memcpy(fileName, "/ext/01/", 8);
    }
    else if (dir == 2)
    {
        memcpy(fileName, "/ext/02/", 8);
    }
    else if (dir == 3)
    {
        memcpy(fileName, "/ext/03/", 8);
    }
    else
    {
        memcpy(fileName, "/ext/04/", 8);
    }

    bufChar = &fileName[8];
    ultoaUsr(index, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_4, " fileindex=%d, fileName=%s\r\n", index, fileName);
    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBalVoucherfileInit error=%d\r\n", iRet);
        return iRet;
    }

    fileSize = fibo_sfile_size(fileName);
    if(fileSize <= 0)
    {
        return fileSize;
    }

    //sysLOG(BASE_LOG_LEVEL_1, " getBalVoucherfileSize=%d\r\n", fileSize);
    iRet = fibo_sfile_read(fileName, aucBalData, fileSize);
    if(iRet != fileSize)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBalVoucherfileRead error=%d\r\n", iRet);
        if(iRet > 0)
        {
            iRet = -1;
        }
        return iRet; 
    }

    iRet = calcTdesDec_lib(aucBalData, fileSize, buf, ucKey, strlen(ucKey), NULL, ECB);
    if (iRet != 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "getBalVoucher desDec iRet=%d\r\n", iRet);
        return -2;
    }
    fileSize -= buf[fileSize - 8] - 8;
    node0 = TLV_Parse(buf,fileSize);
	TLV_Debug(node0);

	found = TLV_Find(node0,0x5A);
	
	if(found)
	{
		//dprintf("FOUND! 9f4d\n");
		for(i=0;i<found->Length;i++)
		{
			value[i] = found->Value[i];
			//dprintf("%02X ",found->Value[i]);
		}
		//memcpy(value,found->Value,6);
		TLV_Free(node0);
		return 0;
	}
	TLV_Free(node0);
	return -3;
}

/**
    * 获取币串
    *
    * @return
    */
int  getTranChain(unsigned char *chianValue) 
{
	struct TLVNode* node0;
	struct TLVNode* found;
	char ffs[128];
	unsigned char buf[2048];
	int size;
	int i;
    unsigned char aucBalData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int iRet = 0;
    int i32Len = 0;

	memset(chianValue,0,6);//清空余额值

	memset(buf,0,sizeof(buf));
	memset(aucBalData, 0x00, sizeof(aucBalData));

    size = hal_getCurrencyString(aucBalData);

	if(size == FILENOEXIST)
		return 0;
	if(size <=0 )//判断如果长度为1 且文件内容值为0，表示文件为空
	{
		return -1;
	}
	else         //读取tag值并返回
	{
        iRet = calcTdesDec_lib(aucBalData, size, buf, ucKey, strlen(ucKey), NULL, ECB);
        if(iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "getBalVoucher desDec iRet=%d\r\n", iRet);
            return -2;
        }
        i32Len = size- buf[size-8] - 8;

		node0 = TLV_Parse(buf,i32Len);
		TLV_Debug(node0);
		found = TLV_Find(node0,0x5A);
		if(found)
		{
			//dprintf("FOUND! 9f4d\n");
			for(i=0;i<found->Length;i++)
			{
				chianValue[i] = found->Value[i];
				//dprintf("%02X ",found->Value[i]);
			}
			TLV_Free(node0);
			return 0;
		}	
		TLV_Free(node0);
	}
	return -3;
}

int getBalVoucherBal(unsigned char *voucher)
{
	int count;
	int i;
	int iRet;
	char fliename[128];
	unsigned int indexTable[MAXBALAMOUNT];
	unsigned char value[6];
	long long balance = 0;
	
	//1.get index

	iRet = getBalVoucherIndexTable1(indexTable);
	if(iRet < 0)
		return iRet;	

	//2.loop file find vocher
	count = iRet;
	for(i=0;i<count;i++)
	{	//2.1 get 0x5A tag		
		iRet = getTagValuebyIndex(indexTable[i],value);
		if(iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " i=%d getBalVoucherfileInit error=%d\r\n", i, iRet);
            return iRet;
        }
			
		//2.2 make balance & sum
		balance += bytesToLonglong(value,6);
	}

	//for(i=0;i<6;i++)
	//	voucher[5-i] = (balance >> (i*8))&0xFF;
	longlongTobyte(balance, voucher, 6);

	return 0;

}

int hal_dcepGetBalance(unsigned char *BalV)
{
	unsigned char voucher[6],chain[6];
	int iRetV,iRetC;
	int i;
	long long balance = 0;
	
	memset(BalV,0, 6);

	memset(voucher,0,sizeof(voucher));
	iRetV = getBalVoucherBal(voucher);
	if(iRetV == 0)
	{
		balance = bytesToLonglong(voucher,6);
	}
	iRetC = getTranChain(chain);
	if(iRetC == 0)
	{
		balance += bytesToLonglong(chain,6);
	}

	if(iRetV || iRetC)
	{
		return -1;
	}
	else
	{
		//for(i=0;i<6;i++)
		//	BalV[6-i] = (balance >> (i*8))&0xFF;
		longlongTobyte(balance, BalV, 6);
	}

	return 0;
}

int hal_readDCEPWall(unsigned char* ucDcepWallParam)  
{
    int iRet = 0;
    int i32FileSize = 0;

    i32FileSize = fibo_sfile_size(DCEPWALLFILENAME);
    if(i32FileSize < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " hal_readDCEPWall error=%d\r\n", iRet);
        return i32FileSize;
    }
    
    iRet = fibo_sfile_read(DCEPWALLFILENAME, ucDcepWallParam, i32FileSize);
    if(iRet != i32FileSize)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " readDCEPWallread error=%d\r\n", iRet);
        if(iRet > 0)
        {
            iRet = -1;
        }
        return iRet; 
    }
    
    return iRet;
}

int hal_writeDCEPWall(unsigned char* ucDcepWallParam, int i32Len)
{
    int iRet = 0;
    int writeSize = 0;  
    
    writeSize = fibo_sfile_write(DCEPWALLFILENAME, ucDcepWallParam, i32Len);
    if(writeSize != i32Len)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " writeDCEPWallWrite error=%d\r\n", iRet);
        iRet = writeSize;
        if(iRet > 0)
        {
            iRet = -1;
        }
        return iRet;
    }
    
    return writeSize;
}

/*
*@Brief:		保存一个余额凭证
*@Param IN:		balVoucher:余额凭证数据; lenth:存储长度
*@Param OUT:	NULL
*@Return:		>=0:成功,返回储存索引; <0:失败
*/
int hal_saveBalVoucher(DCEP_pwrDownOpt *sPwrDownOpt, uint32_t *voucherInfo, sDCEP_VoucherHead *VoucherHead, uint8_t* balVoucher, int lenth)
{
    int iRet = 0;
    int fileindex = 0;
    char fileName[19];
    char* bufChar;
    int fd = 0;
    uint64_t balance = 0;

    if((VoucherHead->blnTtlCnt > 0) && (VoucherHead->blnTtlCnt < MAXBALAMOUNT))
    {
        fileindex = voucherInfo[VoucherHead->blnTtlCnt-1] + 1;
    }
    else if(VoucherHead->blnTtlCnt == 0)
    {
        fileindex = 0;
    }
    else
    {
        return -1;
    }

    memset(fileName, 0x00, sizeof(fileName));
    fileindex &= ~(7 << 28);
    if(VoucherHead->dirOneFileCounter < 200)
    {
        VoucherHead->dirOneFileCounter++;
        sPwrDownOpt->voucherIdentifyFolderOpt = 1;
        memcpy(fileName, "/ext/00/", 8);
        if(VoucherHead->dirOneFlag == 0)
        {
            if(fibo_filedir_exist("/ext/00") != 1)
            {
                iRet = fibo_file_mkdir("/ext/00");
                if(iRet != 0)
                {
                    sysLOG(BASE_LOG_LEVEL_1, "saveError iRet=%d\r\n",iRet);
                    return -2;
                }
            }
            
            VoucherHead->dirOneFlag = 1;
        }
    }
    else if(VoucherHead->dirTwoFileCounter < 200)
    {
        fileindex |= 1<<28;
        VoucherHead->dirTwoFileCounter++;
        sPwrDownOpt->voucherIdentifyFolderOpt = 2;
        memcpy(fileName, "/ext/01/", 8);
        if(VoucherHead->dirTwoFlag == 0)
        {
            if(fibo_filedir_exist("/ext/01") != 1)
            {
                iRet = fibo_file_mkdir("/ext/01");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
                    return -3;
                }
            }
            VoucherHead->dirTwoFlag = 1;
        }
    }
    else if(VoucherHead->dirThrFileCounter < 200)
    {
        fileindex |= 2<<28;
        VoucherHead->dirThrFileCounter++;
        sPwrDownOpt->voucherIdentifyFolderOpt = 3;
        memcpy(fileName, "/ext/02/", 8);
        if(VoucherHead->dirThrFlag == 0)
        {
            if(fibo_filedir_exist("/ext/02") != 1)
            {
                iRet = fibo_file_mkdir("/ext/02");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
                    return -4;
                }
            }
            VoucherHead->dirThrFlag = 1;
        }
    }
    else if(VoucherHead->dirFouFileCounter < 200)
    {
        fileindex |= 3<<28;
        VoucherHead->dirFouFileCounter++;
        sPwrDownOpt->voucherIdentifyFolderOpt = 4;
        memcpy(fileName, "/ext/03/", 8);
        if(VoucherHead->dirFouFlag == 0)
        {
            if(fibo_filedir_exist("/ext/03") != 1)
            {
                iRet = fibo_file_mkdir("/ext/03");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
                    return -5;
                }
            }
            VoucherHead->dirFouFlag = 1;
        }
    }
    else if(VoucherHead->dirFivFileCounter < 200)
    {
        fileindex |= 4<<28;
        VoucherHead->dirFivFileCounter++;
        sPwrDownOpt->voucherIdentifyFolderOpt = 5;
        memcpy(fileName, "/ext/04/", 8);
        if(VoucherHead->dirFivFlag == 0)
        {
            if(fibo_filedir_exist("/ext/04") != 1)
            {
                iRet = fibo_file_mkdir("/ext/04");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
                    return -6;
                }
            } 
            VoucherHead->dirFivFlag = 1;
        }
    }
    else{
        return -7;
    }
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_4, "fileindex=%d,fileName=%s\r\n", fileindex,fileName);

    sPwrDownOpt->voucherSaveOpt = 1;
    memcpy(sPwrDownOpt->PDOptFileName, fileName, sizeof(fileName));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -8;
    }

    fd = fileOpen_lib(fileName, O_RDWR | O_CREAT);
    if(fd < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        goto Exit;
    } 
    iRet = fileClose_lib(fd);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        goto Exit;
    }

    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        goto Exit;
    }

    iRet = fibo_sfile_write(fileName,balVoucher, lenth);
    if(iRet != lenth)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        goto Exit; 
    }
    
    return fileindex;

Exit: 
    if(fibo_file_exist(fileName) == 1)
    {
        fileRemove_lib(fileName);
    }
    memset(sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -9;
    }
    return -10;
}

/*
*@Brief:		读取指定位置的余额凭证
*@Param IN:		index：索引; BalData: 余额凭证数据
*@Param OUT:	NULL
*@Return:		>0:成功;余额凭证数据长度 其他:失败
*/
uint8_t* hal_getBalVoucher(int index, uint32_t *voucherInfo, sDCEP_VoucherHead VoucherHead, uint8_t* BalData)
{
    int iRet = 0;
    int fileindex = 0;
    char fileName[19];
    int pos = 0;
    char* bufChar;
    char flag = 0;
    int fileSize = 0;
    uint32_t dir = 0;
    
    if(VoucherHead.blnTtlCnt > 0)
    {
        for(pos = 0; pos < VoucherHead.blnTtlCnt; pos++)
        {
            if (index == voucherInfo[pos])
            {
                flag = 1;
                break;
            }
        }
    }
    else
    {
        return -1;
    }
    
    if(flag == 1)
    {
        fileindex = index;
    }
    else{
        return -2;
    }

    fileindex = index;
    memset(fileName, 0x00, sizeof(fileName));
     dir = fileindex >> 28;
     if(dir == 0)
     {
         memcpy(fileName, "/ext/00/", 8);
     }
     else if(dir == 1)
     {
         memcpy(fileName, "/ext/01/", 8);
     }
     else if(dir == 2)
     {
         memcpy(fileName, "/ext/02/", 8);
     }
     else if(dir == 3)
     {
         memcpy(fileName, "/ext/03/", 8);
     }
     else
     {
         memcpy(fileName, "/ext/04/", 8);
     }
    
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_4, " fileindex=%d, fileName=%s\r\n", fileindex,fileName);
    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBalVoucherfileInit error=%d\r\n", iRet);
        return -3;
    }

    fileSize = fibo_sfile_size(fileName);
    if(fileSize <= 0)
    {
        return -4;
    }
    
    iRet = fibo_sfile_read(fileName, BalData, fileSize);
    if(iRet != fileSize)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBalVoucherfileRead error=%d\r\n", iRet);
        return -5; 
    }

    return fileSize;
}

/*
*@Brief:		获取余额凭证个数或获取余额
*@Param IN:		mode: 0:获取余额凭证个数; 1:获取余额
*@Param OUT:	NULL
*@Return:		>=0:成功,返回余额凭证个数或余额; <0:失败
*/
uint64_t hal_getBalVoucherData(char mode, sDCEP_VoucherHead VoucherHead)
{
    int iRet = 0;

    if(mode)
    {
        return VoucherHead.balanceSum;
    }
    else
    {
        return VoucherHead.blnTtlCnt;
    }
}

/*
*@Brief:		更新余额凭证
*@Param IN:		uiIndex:索引；pbBalVoucher：更新数据；uiLen:更新数据长度
*@Param OUT:	NULL
*@Return:		=0:成功; <0:失败
*/
int hal_setBalVoucher(int index, uint32_t *voucherInfo, sDCEP_VoucherHead VoucherHead, uint8_t* pbBalVoucher, uint32_t uiLen)
{
    int iRet = 0;
    int fileindex = 0;
    char fileName[19];
    int pos = 0;
    char* bufChar;
    char flag = 0;
    unsigned char backData[4096];
    int fd = 0;
    int fileSize = 0;
    uint32_t dir = 0;
    
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(backData, 0x00, sizeof(backData));

    if(VoucherHead.blnTtlCnt > 0)
    {
        for(pos = 0; pos < VoucherHead.blnTtlCnt; pos++)
        {
            if (index == voucherInfo[pos])
            {
                flag = 1;
                break;
            }
        }
    }
    else
    {
        return -1;
    }
    
    if(flag == 1)
    {
        fileindex = index;
    }
    else{
        return -2;
    }

    fileindex = index;
    memset(fileName, 0x00, sizeof(fileName));
     dir = fileindex >> 28;
     if(dir == 0)
     {
         memcpy(fileName, "/ext/00/", 8);
     }
     else if(dir == 1)
     {
         memcpy(fileName, "/ext/01/", 8);
     }
     else if(dir == 2)
     {
         memcpy(fileName, "/ext/02/", 8);
     }
     else if(dir == 3)
     {
         memcpy(fileName, "/ext/03/", 8);
     }
     else
     {
         memcpy(fileName, "/ext/04/", 8);
     }
    
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_4, " fileindex=%d, fileName=%s\r\n", fileindex,fileName);
    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBalVoucherfileInit error=%d\r\n", iRet);
        return -3;
    }

    fileSize = fibo_sfile_size(fileName);
    if(fileSize <= 0)
    {
        return -4;
    }

    if(uiLen >= fileSize)
    {
        iRet = fibo_sfile_write(fileName, pbBalVoucher, uiLen);
        if(iRet != uiLen)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " saveBalVoucfileWrite error=%d\r\n", iRet);
            return -5; 
        }
    }
    else{
        if(fibo_file_exist(BACKUPFILE) == 1)
        {
            iRet = fileRemove_lib(BACKUPFILE);
            if(iRet != 0)
            { 
                return -6;
            }
        }
        fd = fileOpen_lib(BACKUPFILE, O_RDWR | O_CREAT);
        if(fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Open error=%d\r\n", fd);
            return -7;
        } 
        iRet = fileClose_lib(fd);
        if(iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Close error=%d\r\n", iRet);
            return -8;
        }
        iRet = fibo_sfile_init(BACKUPFILE);
        if(iRet < 0)
        {
            return -9;
        } 
        if(fibo_sfile_write(BACKUPFILE, pbBalVoucher, uiLen) != uiLen)
        {
            return -10;
        }

        sPwrDownOpt.voucherSetOpt = 1;
        memcpy(sPwrDownOpt.PDOptFileName, fileName, sizeof(fileName));
        iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
        if (iRet != sizeof(DCEP_pwrDownOpt))
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
            return -11;
        }

        iRet = fileRemove_lib(fileName);
        if(iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " delBalVoucfileRemove error=%d\r\n", iRet);
            return -12;
        }
        
        fd = fileOpen_lib(fileName, O_RDWR | O_CREAT);
        if(fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " Open error=%d\r\n", fd);
            return -13;
        } 
        iRet = fileClose_lib(fd);
        if(iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " Close error=%d\r\n", iRet);
            return -14;
        }

        iRet = fibo_sfile_init(fileName);
        if(iRet < 0)
        {
            return -15;
        } 
    
        iRet = fibo_sfile_write(fileName, pbBalVoucher, uiLen);
        if(iRet != uiLen)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " saveBalVoucfileWrite error=%d\r\n", iRet);
            return -16; 
        }

        memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
        iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
        if (iRet != sizeof(DCEP_pwrDownOpt))
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
            return -17;
        }
    }
    return 0;
}
/*
*@Brief:		获取余额凭证索引表
*@Param IN:		NULL
*@Param OUT:	iIndexTable：索引表
*@Return:		=0:成功; <0:失败
*/
int hal_getBalVoucherIndexTable(int32_t* iIndexTable, uint32_t *voucherInfo, sDCEP_VoucherHead VoucherHead)
{
    int iRet = 0;

    if(VoucherHead.blnTtlCnt > 0)
    {
        memcpy(iIndexTable, voucherInfo, VoucherHead.blnTtlCnt*sizeof(int));
        return VoucherHead.blnTtlCnt;
    }
    else{
        return -1;
    }
}
/*
*@Brief:		删除指定位置的余额凭证
*@Param IN:		index：索引
*@Param OUT:	NULL
*@Return:		>=0:成功; <0:失败
*/
int hal_deletBalVoucher(int index, DCEP_pwrDownOpt *sPwrDownOpt, uint32_t *voucherInfo, sDCEP_VoucherHead *VoucherHead)
{
    int iRet = 0;
    int len = 0;
    int pos = 0;
    char* bufChar;
    char flag = 0;
    int fileindex = 0;
    uint32_t dir = 0;
    char fileName[19];
    uint64_t balance = 0;

    if(VoucherHead->blnTtlCnt > 0)
    {
        //fileindex = voucherInfo[pos];
        for (pos = 0; pos < VoucherHead->blnTtlCnt; pos++)
        {
            if (index == voucherInfo[pos])
            {
                sysLOG(DCEP_LOG_LEVEL_1, " blnTtlCnt=%d pos=%d index=%d voucherInfo[pos]=%d\r\n",VoucherHead->blnTtlCnt, pos, index, voucherInfo[pos]);
                flag = 1;
                break;
            }
        }
    }
    else
    {
        sysLOG(DCEP_LOG_LEVEL_2, " blnTtlCnt=%d pos=%d index=%d voucherInfo[pos]=%d\r\n",VoucherHead->blnTtlCnt, pos, index, voucherInfo[pos]);
        return -1;
    }
    
    if(flag == 1)
    {
        fileindex = index;
    }
    else{
        return -2;
    }
    
    memset(fileName, 0x00, sizeof(fileName));
    dir = fileindex >> 28;
     if(dir == 0)
     {
        sPwrDownOpt->voucherIdentifyFolderOpt = 1; 
         memcpy(fileName, "/ext/00/", 8);
         VoucherHead->dirOneFileCounter--;
     }
     else if(dir == 1)
     {
         sPwrDownOpt->voucherIdentifyFolderOpt = 2; 
         memcpy(fileName, "/ext/01/", 8);
         VoucherHead->dirTwoFileCounter--;
     }
     else if(dir == 2)
     {
         sPwrDownOpt->voucherIdentifyFolderOpt = 3; 
         memcpy(fileName, "/ext/02/", 8);
         VoucherHead->dirThrFileCounter--;
     }
     else if(dir == 3)
     {
         sPwrDownOpt->voucherIdentifyFolderOpt = 4; 
         memcpy(fileName, "/ext/03/", 8);
         VoucherHead->dirFouFileCounter--;
     }
     else
     {
         sPwrDownOpt->voucherIdentifyFolderOpt = 5; 
         memcpy(fileName, "/ext/04/", 8);
         VoucherHead->dirFivFileCounter--;
     }
    
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);

    iRet = fibo_sfile_write(BACKUPLISTFILE, voucherInfo, sizeof(uint32_t)*MAXBALAMOUNT);
    if (iRet != sizeof(uint32_t)*MAXBALAMOUNT)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
        return -3;
    }

    VoucherHead->blnTtlCnt -= 1;

    sPwrDownOpt->voucherVGOpt = 1;
    memcpy(sPwrDownOpt->PDOptFileName, fileName, sizeof(fileName));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -4;
    }
    
    return pos;
}

/*
*@Brief:		删除全部余额凭证
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功; <0:失败
*/
int hal_deleteAllBalVoucher(DCEP_pwrDownOpt *sPwrDownOpt, uint32_t *voucherInfo, sDCEP_VoucherHead *VoucherHead)
{
    int iRet = 0;
    int i =0;
    uint32_t dir = 0;
    char fileName[19];
    char* bufChar;

    if(VoucherHead->blnTtlCnt == 0)
    {
        return -1;
    }

    sPwrDownOpt->voucherDelAllOpt = 1;
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Error iRet=%d\r\n",iRet);
        return -2;
    }

    for(i = 0; i<VoucherHead->blnTtlCnt; i++)
    {
        memset(fileName, 0x00, sizeof(fileName));

        dir = voucherInfo[i] >> 28;
        if(dir == 0)
        {
            memcpy(fileName, "/ext/00/", 8);
            VoucherHead->dirOneFileCounter--;
        }
        else if(dir == 1)
        {
            memcpy(fileName, "/ext/01/", 8);
            VoucherHead->dirTwoFileCounter--;
        }
        else if(dir == 2)
        {
            memcpy(fileName, "/ext/02/", 8);
            VoucherHead->dirThrFileCounter--;
        }
        else if(dir == 3)
        {
            memcpy(fileName, "/ext/03/", 8);
            VoucherHead->dirFouFileCounter--;
        }
        else
        {
            memcpy(fileName, "/ext/04/", 8);
            VoucherHead->dirFivFileCounter--;
        }
        
        bufChar = &fileName[8];
        ultoaUsr(voucherInfo[i], bufChar, 10);
        
        if(fibo_file_exist(fileName) == 1)
        {
            iRet = fileRemove_lib(fileName);
            if(iRet != 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, " delAllBalVoucfileRemove error=%d\r\n", iRet);
                return -3;
            }
        }
    }

    VoucherHead->blnTtlCnt = 0;
    //memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    VoucherHead->dirOneFileCounter = 0;
    VoucherHead->dirOneFileCounter = 0;
    VoucherHead->dirThrFileCounter = 0;
    VoucherHead->dirFivFileCounter = 0;
    VoucherHead->dirFivFileCounter = 0;

    return 0;
}

/*
*@Brief:		保存币串
*@Param IN:		Data:余额凭证数据; lenth:存储长度
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int hal_saveCurrencyString(uint8_t* Data, int lenth)
{
    int iRet = 0;
    int fd = 0;
    uint64_t balance = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo));
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    if (fibo_file_exist(BACKUPFILE) == 1)
    {
        iRet = fileRemove_lib(BACKUPFILE);
        if (iRet != 0)
        {
            return -1;
        }
    }
    fd = fileOpen_lib(BACKUPFILE, O_RDWR | O_CREAT);
    if (fd < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Open error=%d\r\n", fd);
        return -2;
    }
    iRet = fileClose_lib(fd);
    if (iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Close error=%d\r\n", iRet);
        return -3;
    }
    iRet = fibo_sfile_init(BACKUPFILE);
    if (iRet < 0)
    {
        return -4;
    }
    if (fibo_sfile_write(BACKUPFILE, Data, lenth) != lenth)
    {
        return -5;
    }

    sPwrDownOpt.voucherSetOpt = 1;
    memcpy(sPwrDownOpt.PDOptFileName, TYPE_TRANCHAIN_INDEX, strlen(TYPE_TRANCHAIN_INDEX));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n", iRet);
        return -6;
    }

    iRet = fibo_file_exist(TYPE_TRANCHAIN_INDEX);
    if(iRet > 0) 
    {
        iRet = fileRemove_lib(TYPE_TRANCHAIN_INDEX);
        if(iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXRemove error=%d\r\n", iRet);
            return -7;
        }
    }

    fd = fileOpen_lib(TYPE_TRANCHAIN_INDEX, O_RDWR | O_CREAT);
    if(fd < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXOpen error=%d\r\n", fd);
        return -8;
    } 
    iRet = fileClose_lib(fd);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXClose error=%d\r\n", iRet);
        return -9;
    }

    iRet = fibo_sfile_init(TYPE_TRANCHAIN_INDEX);
    if(iRet < 0)
    {
        return -10;
    }
    currencyStringInitFlag = 1;
   
    iRet = fibo_sfile_write(TYPE_TRANCHAIN_INDEX, Data, lenth);
    if(iRet != lenth)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXwrite error=%d\r\n", iRet);
        return -11; 
    }

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n", iRet);
        return -12;
    }
    return 0;
}

/*
*@Brief:		获取币串
*@Param IN:		Data:币串数据
*@Param OUT:	NULL
*@Return:		0:成功;返回币串长度 <0:失败
*/
int hal_getCurrencyString(uint8_t* Data)
{
    int iRet = 0;
   
    int fileSize = 0;
    char* bufChar;
    int fd = 0;
    uint64_t balance = 0;
    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];
    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 

    iRet = fibo_file_exist(TYPE_TRANCHAIN_INDEX);
    if(iRet < 0)
    {
        return FILENOEXIST;
    }

    if(currencyStringInitFlag == 0)
    {
        iRet = fibo_sfile_init(TYPE_TRANCHAIN_INDEX);
        if(iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXInit error=%d\r\n", iRet);
            return -1;
        }
        currencyStringInitFlag = 1;
    }
    
    fileSize = fibo_sfile_size(TYPE_TRANCHAIN_INDEX);
    if(fileSize <= 0)
    {
        return -2;
    }
    
    iRet = fibo_sfile_read(TYPE_TRANCHAIN_INDEX, Data, fileSize);
    if(iRet != fileSize)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXread error=%d\r\n", iRet);
        return -3; 
    }
    
    return fileSize;
}

/*
*@Brief:	    
*@Param IN:		删除币串
*@Param OUT:	NULL
*@Return:		0:成功; <0:失败
*/
int hal_delCurrencyString()
{
    int iRet = 0;

    iRet = fibo_file_exist(TYPE_TRANCHAIN_INDEX);
    if(iRet < 0)
    {
        return FILENOEXIST;
    }

    iRet = fileRemove_lib(TYPE_TRANCHAIN_INDEX);
    if(iRet != 0)
    {
         sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXRemove error=%d\r\n", iRet);
         return -1;
    }
    currencyStringInitFlag = 0;
    return 0;
}

/*
*@Brief:		保存交易记录
*@Param IN:		bill:交易记录数据; lenth:交易记录长度
*@Param OUT:	NULL
*@Return:		n>=0:成功,返回索引; <0:失败
*/
int hal_saveBill(DCEP_pwrDownOpt *sPwrDownOpt, uint32_t *billInfo, sDCEP_BillHead *BillHead, uint8_t* bill, int lenth)
{
    int iRet = 0;
    uint32_t billTtlCnt = 0;
    int fd = 0;
    char fileName[19];
    char* bufChar;
    int fileindex = 0;
  
    billTtlCnt = billInfo[0];
    if((billTtlCnt > 0) && (billTtlCnt < MAXBILLAMOUNT))
    {
        fileindex = billInfo[billTtlCnt] + 1;
    }
    else if(billTtlCnt == 0)
    {
        fileindex = 0;
    }
    else
    {
        return -1;
    }

    fileindex &= ~(7 << 28);
    memset(fileName, 0x00, sizeof(fileName));
    if(BillHead->dirOneFileCounter < 200)
    {
        BillHead->dirOneFileCounter++;
        sPwrDownOpt->billIdentifyFolderOpt = 1;
        memcpy(fileName, "/ext/20/", 8);
        if(BillHead->dirOneFlag == 0)
        {
            if(fibo_filedir_exist("/ext/20") != 1)
            {
                iRet = fibo_file_mkdir("/ext/20");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, " saveBill error=%d\r\n", iRet);
                    return -2;
                }
            }
            
            BillHead->dirOneFlag = 1;
        }
    }
    else if(BillHead->dirTwoFileCounter < 200)
    {
        fileindex |= 1<<28;
        BillHead->dirTwoFileCounter++;
        sPwrDownOpt->billIdentifyFolderOpt = 2;
        memcpy(fileName, "/ext/21/", 8);
        if(BillHead->dirTwoFlag == 0)
        {
            if(fibo_filedir_exist("/ext/21") != 1)
            {
                iRet = fibo_file_mkdir("/ext/21");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, " saveBill error=%d\r\n", iRet);
                    return -3;
                }
            }
            
            BillHead->dirTwoFlag = 1;
        }
    }
    else if(BillHead->dirThrFileCounter < 200)
    {
        fileindex |= 2<<28;
        BillHead->dirThrFileCounter++;
        sPwrDownOpt->billIdentifyFolderOpt = 3;
        memcpy(fileName, "/ext/22/", 8);
        if(BillHead->dirThrFlag == 0)
        {
            if(fibo_filedir_exist("/ext/22") != 1)
            {
                iRet = fibo_file_mkdir("/ext/22");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, " saveBill error=%d\r\n", iRet);
                    return -4;
                }
            }
            
            BillHead->dirThrFlag = 1;
        }
    }
    else if(BillHead->dirFouFileCounter < 200)
    {
        fileindex |= 3<<28;
        BillHead->dirFouFileCounter++;
        sPwrDownOpt->billIdentifyFolderOpt = 4;
        memcpy(fileName, "/ext/23/", 8);
        if(BillHead->dirFouFlag == 0)
        {
            if(fibo_filedir_exist("/ext/23") != 1)
            {
                iRet = fibo_file_mkdir("/ext/23");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, " saveBill error=%d\r\n", iRet);
                    return -5;
                }
            }
            
            BillHead->dirFouFlag = 1;
        }
    }
    else if(BillHead->dirFivFileCounter < 200)
    {
        fileindex |= 4<<28;
        BillHead->dirFivFileCounter++;
        sPwrDownOpt->billIdentifyFolderOpt = 5;
        memcpy(fileName, "/ext/24/", 8);
        if(BillHead->dirFivFlag == 0)
        {
            if(fibo_filedir_exist("/ext/24") != 1)
            {
                iRet = fibo_file_mkdir("/ext/24");
                if(iRet != 0)
                {
                    sysLOG(DCEP_LOG_LEVEL_2, " saveBill error=%d\r\n", iRet);
                    return -6;
                }
            }
            BillHead->dirFivFlag = 1;
        }
    }
    else{
        return -7;
    }

    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_1, " fileindex=%d, fileName=%s\r\n",fileindex, fileName);

    iRet = fibo_sfile_write(BACKUPLISTFILE, billInfo, sizeof(uint32_t)*(MAXBILLAMOUNT+1));
    if (iRet != sizeof(uint32_t)*MAXBALAMOUNT)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
        return -8;
    }
   
    sPwrDownOpt->billSaveOpt = 1;
    sPwrDownOpt->billFileCnt = billInfo[0];
    memcpy(sPwrDownOpt->PDOptFileName, fileName, sizeof(fileName));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
         return -9;
    }

    fd = fileOpen_lib(fileName, O_RDWR | O_CREAT);
    if(fd < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " billCreatOpen error=%d\r\n", fd);
        goto Exit;
    } 
    iRet = fileClose_lib(fd);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " billCreatClose error=%d\r\n", iRet);
        goto Exit;
    }
    
    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " billCreatInit error=%d\r\n", iRet);
        goto Exit;
    }

    iRet = fibo_sfile_write(fileName, bill, lenth);
    if(iRet != lenth)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " saveBillFileWrite error=%d\r\n", iRet);
        goto Exit; 
    }
   
    billInfo[0]++;
    billInfo[billTtlCnt+1] = fileindex;
    BillHead->billTtlCnt = billInfo[0];
    return fileindex;
Exit: 
    if(fibo_file_exist(fileName) == 1)
    {
        fileRemove_lib(fileName);
    }
    sPwrDownOpt->billSaveOpt = 0;
    memset(sPwrDownOpt->PDOptFileName, 0x00, sizeof(fileName));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -10;
    }
    return -11;
}

/*
*@Brief:		读取交易记录
*@Param IN:		index：索引；billData：交易记录数据
*@Param OUT:	NULL
*@Return:		=0:成功; 其他:失败
*/
int hal_getBill(int index, uint32_t *billInfo, sDCEP_BillHead BillHead, uint8_t* billData)
{
    int iRet = 0;
    uint32_t billTtlCnt = 0;
    char fileName[19];
    int pos = 0;
    char* bufChar;
    char flag = 0;
    int fileSize = 0;
    int fileindex = 0;
    uint32_t dir = 0;

    billTtlCnt = billInfo[0];
    if(billTtlCnt > 0)
    {
        //fileindex = billInfo[pos+1];
        for(pos=0; pos<billTtlCnt; pos++)
        {
            if(index ==  billInfo[pos+1])
            {
                flag = 1;
                break;
            }
        }
    }
    else
    {
        return -1;
    }
    
    if(flag == 1)
    {
        fileindex = index;
    }
    else{
        return -2;
    }

    memset(fileName, 0x00, sizeof(fileName));
    dir = fileindex >> 28;
     if(dir == 0)
     {
         memcpy(fileName, "/ext/20/", 8);
     }
     else if(dir == 1)
     {
         memcpy(fileName, "/ext/21/", 8);
     }
     else if(dir == 2)
     {
         memcpy(fileName, "/ext/22/", 8);
     }
     else if(dir == 3)
     {
         memcpy(fileName, "/ext/23/", 8);
     }
     else
     {
         memcpy(fileName, "/ext/24/", 8);
     }
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_1, " fileindex=%d, fileName=%s\r\n",fileindex, fileName);
    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBillfileInit error=%d\r\n", iRet);
        return -3;
    }

    fileSize = fibo_sfile_size(fileName);
    if(fileSize < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBillfileSize error=%d\r\n", iRet);
        return -4;
    }

    iRet = fibo_sfile_read(fileName, billData, fileSize);
    if(iRet != fileSize)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBillfileRead error=%d\r\n", iRet);
        return -5; 
    }

    return fileSize;
}

int hal_setBill(int index, uint32_t *billInfo, uint8_t* pbBill, uint32_t uiLen)
{
    int iRet = 0;
    uint32_t billTtlCnt = 0;
    char fileName[19];
    int pos = 0;
    char* bufChar;
    char flag = 0;
    unsigned char backData[4096];
    int fd = 0;
    int fileSize = 0;
    int fileindex = 0;
    uint32_t dir = 0;
    DCEP_pwrDownOpt sPwrDownOpt;

    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(backData, 0x00, sizeof(backData));

    billTtlCnt = billInfo[0];
    if(billTtlCnt > 0)
    {
        //fileindex = billInfo[pos+1];
        for(pos=0; pos<billTtlCnt; pos++)
        {
            if(index ==  billInfo[pos+1])
            {
                flag = 1;
                break;
            }
        }
    }
    else
    {
        return -1;
    }
    
    if(flag == 1)
    {
        fileindex = index;
    }
    else{
        return -2;
    }

    memset(fileName, 0x00, sizeof(fileName));
    dir = fileindex >> 28;
     if(dir == 0)
     {
         memcpy(fileName, "/ext/20/", 8);
     }
     else if(dir == 1)
     {
         memcpy(fileName, "/ext/21/", 8);
     }
     else if(dir == 2)
     {
         memcpy(fileName, "/ext/22/", 8);
     }
     else if(dir == 3)
     {
         memcpy(fileName, "/ext/23/", 8);
     }
     else
     {
         memcpy(fileName, "/ext/24/", 8);
     }
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);
    sysLOG(DCEP_LOG_LEVEL_1, " fileindex=%d, fileName=%s\r\n",fileindex, fileName);
    iRet = fibo_sfile_init(fileName);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBillfileInit error=%d\r\n", iRet);
        return -3;
    }

    fileSize = fibo_sfile_size(fileName);
    if(fileSize < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " getBillfileSize error=%d\r\n", iRet);
        return -4;
    }

    if(uiLen > fileSize)
    {
        iRet = fibo_sfile_write(fileName, pbBill, uiLen);
        if(iRet != uiLen)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " saveBalVoucfileWrite error=%d\r\n", iRet);
            return -5; 
        }
    }
    else{
        if(fibo_sfile_read(fileName, backData, fileSize) != fileSize)
        {
            return -6;
        }

        if(fibo_file_exist(BACKUPFILE) == 1)
        {
            iRet = fileRemove_lib(BACKUPFILE);
            if(iRet != 0)
            { 
                return -7;
            }
        }
        fd = fileOpen_lib(BACKUPFILE, O_RDWR | O_CREAT);
        if(fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Open error=%d\r\n", fd);
            return -8;
        } 
        iRet = fileClose_lib(fd);
        if(iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Close error=%d\r\n", iRet);
            return -9;
        }
        iRet = fibo_sfile_init(BACKUPFILE);
        if(iRet < 0)
        {
            return -10;
        } 
        if(fibo_sfile_write(BACKUPFILE, pbBill, uiLen) != uiLen)
        {
            return -11;
        }

        sPwrDownOpt.billSetOpt = 1;
        memcpy(sPwrDownOpt.PDOptFileName, fileName, sizeof(fileName));
        iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
        if (iRet != sizeof(DCEP_pwrDownOpt))
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
            return -12;
        }
        iRet = fileRemove_lib(fileName);
        if(iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " delBillfileRemove error=%d\r\n", iRet);
            return -13;
        }

        fd = fileOpen_lib(fileName, O_RDWR | O_CREAT);
        if(fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXOpen error=%d\r\n", fd);
            return -14;
        } 
        iRet = fileClose_lib(fd);
        if(iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " TYPE_TRANCHAIN_INDEXClose error=%d\r\n", iRet);
            return -15;
        }

        iRet = fibo_sfile_init(fileName);
        if(iRet < 0)
        {
            return -16;
        }

        iRet = fibo_sfile_write(fileName, pbBill, uiLen);
        if(iRet != uiLen)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " saveBalVoucfileWrite error=%d\r\n", iRet);
            return -17; 
        } 
        sPwrDownOpt.billSetOpt = 0;
        memcpy(sPwrDownOpt.PDOptFileName, fileName, sizeof(fileName));
        iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
        if (iRet != sizeof(DCEP_pwrDownOpt))
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
            return -18;
        }
    }

    return 0;
}

/*
*@Brief:		获取交易记录个数
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功,交易记录个数; <0:失败
*/
int hal_getBillCount(uint32_t *billInfo, sDCEP_BillHead BillHead)
{
    int iRet = 0;
    uint32_t blnTtlCnt = 0;

    iRet = fibo_sfile_read(LISTBILLFILENAME, &blnTtlCnt, sizeof(uint32_t));
    if(iRet != sizeof(uint32_t))
    {
        sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEread error=%d\r\n", iRet);
        return -1; 
    }

    return blnTtlCnt;
}

/*
*@Brief:		设置交易记录
*@Param IN:		NULL
*@Param OUT:	iIndexTable：索引表
*@Return:		=0:成功; <0:失败
*/
int hal_getBillIndexTable(int32_t* iIndexTable)
{
    int iRet = 0;
    uint32_t blnTtlCnt = 0;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    memset(billInfo, 0x00, sizeof(billInfo));

    iRet = fibo_sfile_read(LISTBILLFILENAME, billInfo, sizeof(billInfo));
    if(iRet != sizeof(billInfo))
    {
        sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEread error=%d\r\n", iRet);
        return -1; 
    }

    if(billInfo[0] > 0)
    {
        memcpy(iIndexTable, &billInfo[1], billInfo[0]*sizeof(int));
        return billInfo[0];
    }
    else{
        return -2;
    }
    
}

/*
*@Brief:		删除指定位置的交易记录
*@Param IN:		index：索引
*@Param OUT:	NULL
*@Return:		=0:成功; 其他:失败
*/
int hal_deleteBill(int index, DCEP_pwrDownOpt *sPwrDownOpt, uint32_t *billInfo, sDCEP_BillHead *BillHead)
{
    int iRet = 0;
    int len = 0;
    int pos = 0;
    char* bufChar;
    char flag = 0;
    uint32_t billTtlCnt = 0;
    int fileindex = 0;
    uint32_t dir = 0;
    char fileName[19];
    
    uint32_t billInfoTrs[MAXBILLAMOUNT+1];
    memset(billInfoTrs, 0x00, sizeof(billInfoTrs)); 

    billTtlCnt = billInfo[0];
    if(billTtlCnt > 0)
    {
        //fileindex = billInfo[pos+1];
        for(pos=0; pos<billTtlCnt; pos++)
        {
            if(index ==  billInfo[pos+1])
            {
                flag = 1;
                break;
            }
        }
    }
    else
    {
        return -1;
    }

    if(flag == 1)
    {
        fileindex = index;
    }
    else{
        return -2;
    }

    memset(fileName, 0x00, sizeof(fileName));
    dir = fileindex >> 28;
     if(dir == 0)
     {
         sPwrDownOpt->billIdentifyFolderOpt = 1;
         memcpy(fileName, "/ext/20/", 8);
         BillHead->dirOneFileCounter--;
     }
     else if(dir == 1)
     {
         sPwrDownOpt->billIdentifyFolderOpt = 2;
         memcpy(fileName, "/ext/21/", 8);
         BillHead->dirTwoFileCounter--;
     }
     else if(dir == 2)
     {
         sPwrDownOpt->billIdentifyFolderOpt = 3;
         memcpy(fileName, "/ext/22/", 8);
         BillHead->dirThrFileCounter--;
     }
     else if(dir == 3)
     {
         sPwrDownOpt->billIdentifyFolderOpt = 4;
         memcpy(fileName, "/ext/23/", 8);
         BillHead->dirFouFileCounter--;
     }
     else
     {
         sPwrDownOpt->billIdentifyFolderOpt = 5;
         memcpy(fileName, "/ext/24/", 8);
         BillHead->dirFivFileCounter--;
     }
    bufChar = &fileName[8];
    ultoaUsr(fileindex, bufChar, 10);  
    sysLOG(DCEP_LOG_LEVEL_1, " fileindex=%d, fileName=%s\r\n",fileindex, fileName);
   
    iRet = fibo_sfile_write(BACKUPLISTFILE, billInfo, sizeof(uint32_t)*(MAXBILLAMOUNT+1));
    if (iRet != sizeof(uint32_t)*MAXBALAMOUNT)
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
        return -3;
    }
    sPwrDownOpt->billFileCnt = billInfo[0];
    sPwrDownOpt->billVGOpt = 1;
    memcpy(sPwrDownOpt->PDOptFileName, fileName, sizeof(fileName));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n",iRet);
        return -3;
    }

    billInfo[0]--;
    BillHead->billTtlCnt = billInfo[0];
    memcpy(billInfoTrs, &billInfo[pos+2], (billTtlCnt-pos)*sizeof(uint32_t));
    memcpy(&billInfo[pos+1], billInfoTrs, (billTtlCnt-pos)*sizeof(uint32_t));

    return 0;
}

/*
*@Brief:		删除所有交易记录
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		=0:成功; 其他:失败
*/
int hal_deleteAllBill(DCEP_pwrDownOpt *sPwrDownOpt, uint32_t *billInfo, sDCEP_BillHead *BillHead)
{
    int iRet = 0;
    int i =0;
    uint32_t fileindex = 0;
    uint32_t dir = 0;
    char fileName[19];
    char* bufChar;

    if(billInfo[0] == 0) 
    {
        return -1;
    }

    sPwrDownOpt->billDelAllOpt = 1;
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_2, "Error iRet=%d\r\n",iRet);
        return -2;
    }

    for(i = 0; i<billInfo[0]; i++)
    {
        memset(fileName, 0x00, sizeof(fileName));
        dir = billInfo[i+1] >> 28;
        if(dir == 0)
        {
            memcpy(fileName, "/ext/20/", 8);
            BillHead->dirOneFileCounter--;
        }
        else if(dir == 1)
        {
            memcpy(fileName, "/ext/21/", 8);
            BillHead->dirTwoFileCounter--;
        }
        else if(dir == 2)
        {
            memcpy(fileName, "/ext/22/", 8);
            BillHead->dirThrFileCounter--;
        }
        else if(dir == 3)
        {
            memcpy(fileName, "/ext/23/", 8);
            BillHead->dirFouFileCounter--;
        }
        else
        {
            memcpy(fileName, "/ext/24/", 8);
            BillHead->dirFivFileCounter--;
        }
        bufChar = &fileName[8];
        ultoaUsr(billInfo[i+1], bufChar, 10);
        sysLOG(DCEP_LOG_LEVEL_1, " billInfo[i+1]=%d, fileName=%s\r\n",billInfo[i+1], fileName);
        iRet = fileRemove_lib(fileName);
        if(iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " delAllBillfileRemove error=%d\r\n", iRet);
            return -3;
        }
    }

    memset(billInfo, 0x00, sizeof(billInfo)); 
    //memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));  
    BillHead->dirOneFileCounter = 0;
    BillHead->dirTwoFileCounter = 0;
    BillHead->dirThrFileCounter = 0;
    BillHead->dirFouFileCounter = 0;
    BillHead->dirFivFileCounter = 0;

    return 0;
}

/*
*@Brief:		写入ATC值
*@Param IN:		atc：atc值
*@Param OUT:	NULL
*@Return:		=0:成功; 其他:失败
*/
int hal_setATC(unsigned char* pucAtc, int i32Len)
{
    int iRet = 0;

    iRet = fibo_sfile_write(ATCFILENAME, pucAtc, i32Len);
    if(iRet != i32Len)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " ATCFILENAMEWrite error=%d\r\n", iRet);
        if(iRet > 0)
        {
            iRet = -1;
        }
        return iRet; 
    }

    return 0;
}

/*
*@Brief:		获取ATC值
*@Param IN:		NULL
*@Param OUT:	NULL
*@Return:		>=0:成功，ATC值; <0:失败
*/
int hal_getATC(unsigned char* pucAtc)
{
    int iRet = 0;
    int i32FileSize = 0;

    i32FileSize = fibo_sfile_size(ATCFILENAME);
    if(i32FileSize <= 0)
    {
        sysLOG(DCEP_LOG_LEVEL_1, " i32FileSize=%d\r\n", i32FileSize);
        return -1;
    }

    iRet = fibo_sfile_read(ATCFILENAME, pucAtc, i32FileSize);
    if(iRet != i32FileSize)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " ATCFILENAMEread error=%d\r\n", iRet);
        if(iRet > 0)
        {
            iRet = -2;
        }
        return iRet; 
    }

    return i32FileSize;
}

int hal_dcepWallInit()
{
    int fd = 0;
    int iRet = 0;
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    uint32_t ui32Len = 0;

    iRet = fibo_file_exist(DCEPWALLFILENAME);
    if(iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
         fd = fileOpen_lib(DCEPWALLFILENAME, O_RDWR | O_CREAT);
         if (fd < 0)
         {
             sysLOG(DCEP_LOG_LEVEL_2, " writeDCEPWallOpen error=%d\r\n", fd);
             return fd;
         }
         iRet = fileClose_lib(fd);
         if (iRet < 0)
         {
             sysLOG(DCEP_LOG_LEVEL_2, " writeDCEPWallClose error=%d\r\n", iRet);
             return iRet;
         }
         iRet = fibo_sfile_init(DCEPWALLFILENAME);
         if (iRet < 0)
         {
             sysLOG(DCEP_LOG_LEVEL_2, " writeDCEPWallInit error=%d\r\n", iRet);
             return iRet;
         }
         memset(aucBalData, 0x00, sizeof(aucBalData));
         memset(aucBalData, 0x00, sizeof(aucOutData));
         ui32Len = sizeof(sDCEP_wall);
          if((sizeof(sDCEP_wall) % 8) != 0)
          {
              ui32Len += (8 - (sizeof(sDCEP_wall) % 8));
              aucBalData[ui32Len] = (8 - (sizeof(sDCEP_wall) % 8));
          }

          iRet = calcTdesEnc_lib(aucBalData, ui32Len + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
          if (iRet != 0)
          {
              sysLOG(DCEP_LOG_LEVEL_2, "setDcepWallData desEnc iRet=%d\r\n", iRet);
              return -1;
          }

          iRet = hal_writeDCEPWall(aucOutData, ui32Len + 8);
          if (iRet != (ui32Len + 8))
          {
              return -2;
          }
    }
    else{
        iRet = fibo_sfile_init(DCEPWALLFILENAME);
         if (iRet < 0)
         {
             sysLOG(DCEP_LOG_LEVEL_2, " writeDCEPWallInit error=%d\r\n", iRet);
             return iRet;
         }
    }
    return 0;
}

int hal_voucherInit(void)
{
    int fd = 0;
    int iRet = 0;

    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];

    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    uint32_t ui32Len = 0;
    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 
    
    iRet = fibo_file_exist(ADMBLNFILENAME);
    if (iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        fd = fileOpen_lib(ADMBLNFILENAME, O_RDWR | O_CREAT);
        if (fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ADMBLNFILENAMEOpen error=%d\r\n", fd);
            return fd;
        }
        iRet = fileClose_lib(fd);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ADMBLNFILENAMEClose error=%d\r\n", iRet);
            return iRet;
        }
        iRet = fibo_sfile_init(ADMBLNFILENAME);
        if (iRet < 0)
        {
            return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        memset(aucBalData, 0x00, sizeof(aucBalData));
        memset(aucBalData, 0x00, sizeof(aucOutData));
        ui32Len = sizeof(sDCEP_VoucherHead);
        if ((sizeof(sDCEP_VoucherHead) % 8) != 0)
        {
            ui32Len += (8 - (sizeof(sDCEP_VoucherHead) % 8));
            aucBalData[ui32Len] = (8 - (sizeof(sDCEP_VoucherHead) % 8));
        }
        
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        iRet = calcTdesEnc_lib(aucBalData, ui32Len + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
        if (iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveBalInit desEnc iRet=%d\r\n", iRet);
            return -1;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        iRet = fibo_sfile_write(ADMBLNFILENAME, aucOutData, ui32Len+8);
        if (iRet != (ui32Len+8))
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ADMBLNFILENAMEWrite error=%d\r\n", iRet);
            if (iRet > 0)
            {
                iRet = -2;
            }
            return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }
    else
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        iRet = fibo_sfile_init(ADMBLNFILENAME);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " fibo_sfile_init error=%d\r\n", iRet);
            return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }

    iRet = fibo_file_exist(LISTBLNFILENAME);
    if (iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        fd = fileOpen_lib(LISTBLNFILENAME, O_RDWR | O_CREAT);
        if (fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBLNFILENAMEOpen error=%d\r\n", fd);
            return fd;
        }
        iRet = fileClose_lib(fd);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBLNFILENAMEClose error=%d\r\n", iRet);
            return iRet;
        }
        iRet = fibo_sfile_init(LISTBLNFILENAME);
        if (iRet < 0)
        {
            return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        memset(aucBalData, 0x00, sizeof(aucBalData));
        memset(aucBalData, 0x00, sizeof(aucOutData));
        ui32Len = sizeof(voucherInfo);
        if ((sizeof(voucherInfo) % 8) != 0)
        {
            ui32Len += (8 - (sizeof(voucherInfo) % 8));
            aucBalData[ui32Len] = (8 - (sizeof(voucherInfo) % 8));
        }

        iRet = calcTdesEnc_lib(aucBalData, ui32Len + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
        if (iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveBalInit desEnc iRet=%d\r\n", iRet);
            return -3;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        iRet = fibo_sfile_write(LISTBLNFILENAME, aucOutData, ui32Len + 8);
        if (iRet != ui32Len + 8)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBLNFILENAMEWrite error=%d\r\n", iRet);
            if (iRet > 0)
            {
                iRet = -4;
            }
            return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }
    else
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        iRet = fibo_sfile_init(LISTBLNFILENAME);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " fibo_sfile_init error=%d\r\n", iRet);
            return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }

    return 0;
}

int hal_billdcepInit()
{
    int fd = 0;
    int iRet = 0;

    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    uint32_t ui32Len = 0;
    
    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));

    iRet = fibo_file_exist(LISTBILLFILENAME);
    if (iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        fd = fileOpen_lib(LISTBILLFILENAME, O_RDWR | O_CREAT);
        if (fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ATCFILENAMEFileOpen error=%d\r\n", fd);
            return iRet;
        }
        iRet = fileClose_lib(fd);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ATCFILENAMEClose error=%d\r\n", iRet);
             return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, " hal_saveBill003\r\n");
        iRet = fibo_sfile_init(LISTBILLFILENAME);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEinit error=%d\r\n", iRet);
             return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, " hal_saveBill004\r\n");
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        memset(aucBalData, 0x00, sizeof(aucBalData));
        memset(aucBalData, 0x00, sizeof(aucOutData));
        ui32Len = sizeof(billInfo);
        if ((sizeof(billInfo) % 8) != 0)
        {
            ui32Len += (8 - (sizeof(billInfo) % 8));
            aucBalData[ui32Len] = (8 - (sizeof(billInfo) % 8));
        }
    
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        iRet = calcTdesEnc_lib(aucBalData, ui32Len + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
        if (iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveBalInit desEnc iRet=%d\r\n", iRet);
            return -1;
        }

        iRet = fibo_sfile_write(LISTBILLFILENAME, aucOutData, ui32Len+8);
        if (iRet != ui32Len+8)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEWrite error=%d\r\n", iRet);
            return -2;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }
    else
    {
        sysLOG(DCEP_LOG_LEVEL_4, " hal_saveBill002\r\n");
        iRet = fibo_sfile_init(LISTBILLFILENAME);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEinit error=%d\r\n", iRet);
             return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }

    iRet = fibo_file_exist(ADMBILLFILE);
    if (iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        fd = fileOpen_lib(ADMBILLFILE, O_RDWR | O_CREAT);
        if (fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ATCFILENAMEFileOpen error=%d\r\n", fd);
             return iRet;
        }
        iRet = fileClose_lib(fd);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " ATCFILENAMEClose error=%d\r\n", iRet);
             return iRet;
        }
        iRet = fibo_sfile_init(ADMBILLFILE);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEinit error=%d\r\n", iRet);
             return iRet;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        memset(aucBalData, 0x00, sizeof(aucBalData));
        memset(aucBalData, 0x00, sizeof(aucOutData));
        ui32Len = sizeof(sDCEP_BillHead);
        if ((sizeof(sDCEP_BillHead) % 8) != 0)
        {
            ui32Len += (8 - (sizeof(sDCEP_BillHead) % 8));
            aucBalData[ui32Len] = (8 - (sizeof(sDCEP_BillHead) % 8));
        }

        iRet = calcTdesEnc_lib(aucBalData, ui32Len + 8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
        if (iRet != 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "saveBalInit desEnc iRet=%d\r\n", iRet);
            return -3;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
        iRet = fibo_sfile_write(ADMBILLFILE, aucOutData, ui32Len + 8);
        if (iRet != ui32Len + 8)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEWrite error=%d\r\n", iRet);
            return -4;
        }
        //sysLOG(BASE_LOG_LEVEL_1, "decpInit\r\n");
    }
    else
    {
        sysLOG(DCEP_LOG_LEVEL_4, "decpInit\r\n");
        iRet = fibo_sfile_init(ADMBILLFILE);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, " LISTBILLFILENAMEinit error=%d\r\n", iRet);
             return iRet;
        }
    }
    return 0;
}

int hal_atcInit()
{
    int fd = 0;
    int iRet = 0;

    fd = fileOpen_lib(ATCFILENAME, O_RDWR | O_CREAT);
    if (fd < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " setATCFileOpen error=%d\r\n", fd);
        return fd;
    }
    iRet = fileClose_lib(fd);
    if (iRet < 0)
    {
        sysLOG(DCEP_LOG_LEVEL_2, " setATCFileClose error=%d\r\n", iRet);
        return iRet;
    }
    iRet = fibo_sfile_init(ATCFILENAME);
    if (iRet < 0)
    {
        return iRet;
    }
    return 0;
}

int hal_backupListFileInit()
{
    uint32_t backData[MAXBALAMOUNT];
    int iRet = 0;
    int fd = 0;

    memset(backData, 0x00, sizeof(backData));
    iRet = fibo_file_exist(BACKUPLISTFILE);
    if(iRet < 0)
    {
        fd = fileOpen_lib(BACKUPLISTFILE, O_RDWR | O_CREAT);
        if (fd < 0)
        {
             goto exit;
        }
        iRet = fileClose_lib(fd);
        if (iRet < 0)
        {
            goto exit;
        }
        iRet = fibo_sfile_init(BACKUPLISTFILE);
        if (iRet < 0)
        {
             goto exit;
        }
        iRet = fibo_sfile_write(BACKUPLISTFILE, backData, sizeof(backData));
        if (iRet != sizeof(backData))
        {
             goto exit;
        }
    }
    else
	{
        iRet = fibo_sfile_init(BACKUPLISTFILE);
        if (iRet < 0)
        {
            goto exit;
        }
    }

exit:

	sysLOG(DCEP_LOG_LEVEL_1, "iRet=%d\r\n", iRet);
	return iRet;

	
}

int hal_pwrDownOpt()
{
    int fd = 0;
    int iRet = 0;
    int fileindex = 0;
    unsigned char backData[4096];
    int fileSize = 0;
    unsigned char aucBalData[4096];
    unsigned char aucOutData[4096];
    unsigned char ucKey[]="asdcvdffff1234598760qwer\0";
    int i32Len = 0;
    uint32_t backListData[MAXBALAMOUNT];
    uint32_t billTtlCnt = 0;

    sDCEP_VoucherHead VoucherHead;
    uint32_t voucherInfo[MAXBALAMOUNT];
    DCEP_pwrDownOpt sPwrDownOpt;
    uint32_t billInfo[MAXBILLAMOUNT+1];
    sDCEP_BillHead BillHead;
    
    memset(billInfo, 0x00, sizeof(billInfo));
    memset(&BillHead, 0x00, sizeof(sDCEP_BillHead));
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    memset(&VoucherHead, 0x00, sizeof(sDCEP_VoucherHead));
    memset(voucherInfo, 0x00, sizeof(voucherInfo)); 
    memset(backData, 0x00, sizeof(backData));

    iRet = fibo_file_exist(POWERDOWNRECOVERY);
    if(iRet < 0)
    {
        fd = fileOpen_lib(POWERDOWNRECOVERY, O_RDWR | O_CREAT);
        if (fd < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
            return iRet;
        }
        iRet = fileClose_lib(fd);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
             return iRet;
        }
        iRet = fibo_sfile_init(POWERDOWNRECOVERY);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
            return iRet;
        }
        iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
        if (iRet != sizeof(DCEP_pwrDownOpt))
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
            return -1;
        }
    }
    else{
        iRet = fibo_sfile_init(POWERDOWNRECOVERY);
        if (iRet < 0)
        {
            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
            return iRet;
        }

        iRet = fibo_sfile_read(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
        if(sPwrDownOpt.voucherSaveOpt != 0)
        {
            switch (sPwrDownOpt.voucherSaveOpt)
            {
                case 1:
                    if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
                    {
                        //sysLOG(BASE_LOG_LEVEL_1, "DelSaveFile!\r\n");
                        fileRemove_lib(sPwrDownOpt.PDOptFileName);
                    }
                    goto Exit;
                    break;
                case 2:
                    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
                    if(iRet == 0)
                    {
                        if(sPwrDownOpt.voucherFileCnt = VoucherHead.blnTtlCnt)   //断电前已保存
                        {
                            sysLOG(DCEP_LOG_LEVEL_4, "SaveFile!\r\n");
                            goto Exit;
                        }
                        VoucherHead.blnTtlCnt += 1;
                        if(sPwrDownOpt.voucherIdentifyFolderOpt == 1)
                        {
                            VoucherHead.dirOneFileCounter++;
                            VoucherHead.dirOneFlag = 1;
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 2)
                        {
                            VoucherHead.dirTwoFileCounter++;
                            VoucherHead.dirTwoFlag = 1;
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 3)
                        {
                            VoucherHead.dirThrFileCounter++;
                            VoucherHead.dirThrFlag = 1;
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 4)
                        {
                            VoucherHead.dirFouFileCounter++;   
                            VoucherHead.dirFouFlag = 1;            
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 5)
                        {
                            VoucherHead.dirFivFileCounter++;
                            VoucherHead.dirFivFlag = 1;
                        }
                        iRet = writeBalVoucherAdmblnFile(sPwrDownOpt, voucherInfo, VoucherHead);
                        if(iRet < 0)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                            return iRet;
                        } 
                        sysLOG(DCEP_LOG_LEVEL_1, "recoverSaveFile!\r\n");
                        return 0;
                    }
                    else{
                        sysLOG(DCEP_LOG_LEVEL_1, "DelSaveFile!\r\n");
                        fileRemove_lib(sPwrDownOpt.PDOptFileName);
                        goto Exit;
                    }
                    break;
                default:
                    break;
            }
        }
        else if(sPwrDownOpt.voucherSetOpt != 0)
        {
            if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
            {
                fileRemove_lib(sPwrDownOpt.PDOptFileName);
            }
            fd = fileOpen_lib(sPwrDownOpt.PDOptFileName, O_RDWR | O_CREAT);
            if(fd < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -2;
            } 
            iRet = fileClose_lib(fd);
            if(iRet < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -3;
            }
            iRet = fibo_sfile_init(BACKUPFILE);
            if(iRet < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return iRet;
            } 
            fileSize = fibo_sfile_size(BACKUPFILE);
            if(fileSize <= 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -4;
            }
            if(fibo_sfile_read(BACKUPFILE, backData, fileSize) != fileSize)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -5;
            }
            if(fibo_sfile_write(sPwrDownOpt.PDOptFileName, backData, fileSize) != fileSize)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -6;
            }
            fileRemove_lib(BACKUPFILE);
            goto Exit;
        }
        else if(sPwrDownOpt.voucherVGOpt != 0)
        {
            switch(sPwrDownOpt.voucherVGOpt)
            {
                case 1:
                    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
                    if(iRet == 0)
                    {
                        memset(backListData, 0x00, sizeof(backListData));
                        if(fibo_sfile_read(BACKUPLISTFILE, backListData, sizeof(uint32_t)*MAXBALAMOUNT) == sizeof(uint32_t)*MAXBALAMOUNT)
                        {
                            if(memcmp(voucherInfo, backListData, sizeof(uint32_t)*MAXBALAMOUNT) != 0)
                            {
                                 sysLOG(DCEP_LOG_LEVEL_1, "voucherInfo modified!\r\n");
                                 memcpy(voucherInfo, backListData, sizeof(uint32_t)*MAXBALAMOUNT);
                            }
                            else{
                                sysLOG(DCEP_LOG_LEVEL_1, "voucherInfo unmodified!\r\n");
                            }
                            goto Exit;
                        }
                        else{
                            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                            return iRet;
                        }
                    }
                    else{
                        sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                        return iRet;
                    }
                   
                break;
                case 2:
                    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
                    if(iRet == 0)
                    {
                        if(sPwrDownOpt.voucherFileCnt = VoucherHead.blnTtlCnt)   //断电前已保存
                        {
                            if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
                            {
                                iRet = fileRemove_lib(sPwrDownOpt.PDOptFileName);
                            }
                            sysLOG(DCEP_LOG_LEVEL_1, "DelFile OK!\r\n");
                            goto Exit;
                        }
                        if(sPwrDownOpt.voucherIdentifyFolderOpt == 1)
                        {
                            VoucherHead.dirOneFileCounter--;
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 2)
                        {
                            VoucherHead.dirTwoFileCounter--;
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 3)
                        {
                            VoucherHead.dirThrFileCounter--;
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 4)
                        {
                            VoucherHead.dirFouFileCounter--;             
                        }
                        else if(sPwrDownOpt.voucherIdentifyFolderOpt == 5)
                        {
                            VoucherHead.dirFivFileCounter--;
                        }
                        VoucherHead.blnTtlCnt -= 1;
                        memset(aucBalData, 0x00, sizeof(aucBalData));
                        memset(aucBalData, 0x00, sizeof(aucOutData));
                        
                        memcpy(aucBalData, &VoucherHead, sizeof(sDCEP_VoucherHead));
                        i32Len = sizeof(sDCEP_VoucherHead);
                        if(sizeof(sDCEP_VoucherHead)%8 != 0)
                        {
                            i32Len += (8-(sizeof(sDCEP_VoucherHead) % 8));
                            aucBalData[i32Len] = (8-(sizeof(sDCEP_VoucherHead) % 8));
                        }

                        iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
                        if(iRet != 0)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "Error=%d\r\n", iRet);
                            return -7;
                        }
                        iRet = fibo_sfile_write(ADMBLNFILENAME, aucOutData, i32Len+8);
                        if(iRet != i32Len+8)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "Error=%d\r\n", iRet);
                            return -8;
                        }
                        if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
                        {
                            iRet = fileRemove_lib(sPwrDownOpt.PDOptFileName);
                        }
                    }
                    else{
                        sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                        return iRet;
                    }
                    goto Exit;
                break;
                default:
                break;
            }
        }
        else if(sPwrDownOpt.voucherDelAllOpt != 0)
        {
            switch(sPwrDownOpt.voucherDelAllOpt)
            {
                case 1:
                    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
                    if(iRet == 0)
                    {
                        iRet = hal_deleteAllBalVoucher(&sPwrDownOpt,voucherInfo, &VoucherHead);
                        if(iRet < 0)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "saveError iRet=%d\r\n", iRet);
                            return -9;
                        }
                        memset(voucherInfo, 0x00, sizeof(voucherInfo));
                        iRet = writeBalVoucherAdmblnFile(sPwrDownOpt, voucherInfo, VoucherHead);
                        if(iRet != 0)
                        {
                            return -10;
                        }
                        return 0;
                    }
                break;
                case 2:
                    iRet = readBalVoucherAdmblnFile(voucherInfo, &VoucherHead);
                    if(iRet == 0)
                    {
                        VoucherHead.blnTtlCnt = 0;
                        VoucherHead.dirOneFileCounter = 0;
                        VoucherHead.dirOneFileCounter = 0;
                        VoucherHead.dirThrFileCounter = 0;
                        VoucherHead.dirFivFileCounter = 0;
                        VoucherHead.dirFivFileCounter = 0;
                        memset(voucherInfo, 0x00, sizeof(voucherInfo));
                        iRet = writeBalVoucherAdmblnFile(sPwrDownOpt, voucherInfo, VoucherHead);
                        if(iRet != 0)
                        {
                            return -11;
                        }
                        return 0;
                    }
                break;
                default:
                break;
            }
        }
        else if(sPwrDownOpt.billSaveOpt != 0)
        {
            switch(sPwrDownOpt.billSaveOpt)
            {
                case 1:
                    if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
                    {
                        memset(backListData, 0x00, sizeof(backListData));
                        fileRemove_lib(sPwrDownOpt.PDOptFileName);
                        if (readBillAdmblnFile(billInfo, &BillHead) == 0)
                        {
                            if(fibo_sfile_read(BACKUPLISTFILE, backListData, sizeof(uint32_t)*(MAXBILLAMOUNT+1)) == sizeof(uint32_t)*(MAXBILLAMOUNT+1))
                            {
                                if(memcmp(billInfo, backListData, sizeof(uint32_t)*(MAXBILLAMOUNT+1)) != 0)
                                {
                                    sysLOG(DCEP_LOG_LEVEL_1, "billInfo modified!\r\n");
                                    memcpy(billInfo, backListData, sizeof(uint32_t)*(MAXBILLAMOUNT+1));
                                }
                                else{
                                    sysLOG(DCEP_LOG_LEVEL_1, "billInfo unmodified!\r\n");
                                }
                                goto Exit;
                            }
                            else{
                                return iRet;
                            }
                        }
                        else{
                            return iRet;
                        }
                    }
                    goto Exit;
                break;
                case 2:
                    if(readBillAdmblnFile(billInfo, &BillHead) == 0)
                    {
                        if(BillHead.billTtlCnt == (sPwrDownOpt.billFileCnt+1))
                        {
                            sysLOG(DCEP_LOG_LEVEL_1, "SaveFile!\r\n");
                            goto Exit;
                        }
                        if(sPwrDownOpt.billIdentifyFolderOpt == 1)
                        {
                            BillHead.dirOneFileCounter++;
                            BillHead.dirOneFlag = 1;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 2)
                        {
                            BillHead.dirTwoFileCounter++;
                            BillHead.dirTwoFlag = 1;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 3)
                        {
                            BillHead.dirThrFileCounter++;
                            BillHead.dirThrFlag = 1;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 4)
                        {
                            BillHead.dirFouFileCounter++;
                            BillHead.dirFouFlag = 1;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 5)
                        {
                            BillHead.dirFivFileCounter++;
                            BillHead.dirFivFlag = 1;
                        }
                        iRet =  writeBillAdmblnFile(sPwrDownOpt, billInfo, BillHead);
                        if(iRet != 0)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                            return iRet;
                        }
                        return 0;
                    }
                    else{
                        fileRemove_lib(sPwrDownOpt.PDOptFileName);
                        goto Exit;
                    }
                break;
                default:
                break;

            }
        }
        else if(sPwrDownOpt.billSetOpt != 0)
        {
            if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
            {
                fileRemove_lib(sPwrDownOpt.PDOptFileName);
            }
            fd = fileOpen_lib(sPwrDownOpt.PDOptFileName, O_RDWR | O_CREAT);
            if(fd < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -12;
            } 
            iRet = fileClose_lib(fd);
            if(iRet < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -13;
            }
            iRet = fibo_sfile_init(BACKUPFILE);
            if(iRet < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return iRet;
            } 
            fileSize = fibo_sfile_size(BACKUPFILE);
            if(fileSize <= 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -14;
            }
            if(fibo_sfile_read(BACKUPFILE, backData, fileSize) != fileSize)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -15;
            }
            if(fibo_sfile_write(sPwrDownOpt.PDOptFileName, backData, fileSize) != fileSize)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -16;
            }
            fileRemove_lib(BACKUPFILE);
            goto Exit;
        }
        else if(sPwrDownOpt.billVGOpt != 0)
        {
            switch(sPwrDownOpt.billVGOpt)
            {
                case 1:
                    if (readBillAdmblnFile(billInfo, &BillHead) == 0)
                    {
                        if (fibo_sfile_read(BACKUPLISTFILE, backListData, sizeof(uint32_t) * (MAXBILLAMOUNT + 1)) == sizeof(uint32_t) * (MAXBILLAMOUNT + 1))
                        {
                            if (memcmp(billInfo, backListData, sizeof(uint32_t) * (MAXBILLAMOUNT + 1)) != 0)
                            {
                                sysLOG(DCEP_LOG_LEVEL_1, "billInfo modified!\r\n");
                                memcpy(billInfo, backListData, sizeof(uint32_t) * (MAXBILLAMOUNT + 1));
                            }
                            else
                            {
                                sysLOG(DCEP_LOG_LEVEL_1, "billInfo unmodified!\r\n");
                            }
                            goto Exit;
                        }
                        else
                        {
                            return iRet;
                        }
                    }
                    else
                    {
                        return iRet;
                    }
                    
                break;
                case 2:
                    iRet = readBillAdmblnFile(billInfo, &BillHead);
                    if(iRet == 0)
                    {
                        if(sPwrDownOpt.billFileCnt == (BillHead.billTtlCnt+1))
                        {
                            if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
                            {
                                iRet = fileRemove_lib(sPwrDownOpt.PDOptFileName);
                            }
                            sysLOG(DCEP_LOG_LEVEL_1, "DelFile OK!\r\n");
                            goto Exit;
                        }
                        if(sPwrDownOpt.billIdentifyFolderOpt == 1)
                        {
                            BillHead.dirOneFileCounter--;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 2)
                        {
                            BillHead.dirTwoFileCounter--;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 3)
                        {
                            BillHead.dirThrFileCounter--;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 4)
                        {
                            BillHead.dirFouFileCounter--;
                        }
                        else if(sPwrDownOpt.billIdentifyFolderOpt == 5)
                        {
                            BillHead.dirFivFileCounter--;
                        }
                        memset(aucBalData, 0x00, sizeof(aucBalData));
                        memset(aucBalData, 0x00, sizeof(aucOutData));

                        memcpy(aucBalData, &BillHead, sizeof(sDCEP_BillHead));
                        i32Len = sizeof(sDCEP_BillHead);
                        if(sizeof(sDCEP_BillHead)%8 != 0)
                        {
                            i32Len += (8-(sizeof(sDCEP_BillHead) % 8));
                            aucBalData[i32Len] = (8-(sizeof(sDCEP_BillHead) % 8));
                        }

                        iRet = calcTdesEnc_lib(aucBalData, i32Len+8, aucOutData, ucKey, strlen(ucKey), NULL, ECB);
                        if(iRet != 0)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "Error=%d\r\n", iRet);
                            return -17;
                        }
                        iRet = fibo_sfile_write(ADMBILLFILE, aucOutData, i32Len+8);
                        if(iRet != i32Len+8)
                        {
                            sysLOG(DCEP_LOG_LEVEL_2, "Error=%d\r\n", iRet);
                            return -18; 
                        }
                        if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
                        {
                            iRet = fileRemove_lib(sPwrDownOpt.PDOptFileName);
                        }
                    }
                    else
                    {
                        return iRet;
                    }
                    goto Exit;
                break;
                default:
                break;
            }
        }
        else if(sPwrDownOpt.billDelAllOpt != 0)
        {
            switch(sPwrDownOpt.billDelAllOpt)
            {
                case 1:
                    if(readBillAdmblnFile(billInfo, &BillHead) == 0)
                    {
                        iRet = hal_deleteAllBill(&sPwrDownOpt, billInfo, &BillHead);
                        if(iRet != 0)
                        {
                            return -19;
                        }
                        iRet =  writeBillAdmblnFile(sPwrDownOpt, billInfo, BillHead);
                        if(iRet != 0)
                        {
                            return -20;
                        }
                        return 0;
                    }
                break;
                case 2:
                     if(readBillAdmblnFile(billInfo, &BillHead) == 0)
                     {
                        BillHead.dirOneFileCounter = 0;
                        BillHead.dirTwoFileCounter = 0;
                        BillHead.dirThrFileCounter = 0;
                        BillHead.dirFouFileCounter = 0;
                        BillHead.dirFivFileCounter = 0;
                        iRet =  writeBillAdmblnFile(sPwrDownOpt, billInfo, BillHead);
                        if(iRet != 0)
                        {
                            return -21;
                        }
                        return 0;
                     }
                break;
                default:
                break;
            }
        }
        else if(sPwrDownOpt.currencyStringSaveOpt != 0)
        {
            if(fibo_file_exist(sPwrDownOpt.PDOptFileName) == 1)
            {
                fileRemove_lib(sPwrDownOpt.PDOptFileName);
                goto Exit;
            }
            fd = fileOpen_lib(sPwrDownOpt.PDOptFileName, O_RDWR | O_CREAT);
            if(fd < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -22;
            } 
            iRet = fileClose_lib(fd);
            if(iRet < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -23;
            }
            iRet = fibo_sfile_init(BACKUPFILE);
            if(iRet < 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return iRet;
            } 
            fileSize = fibo_sfile_size(BACKUPFILE);
            if(fileSize <= 0)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -24;
            }
            if(fibo_sfile_read(BACKUPFILE, backData, fileSize) != fileSize)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -25;
            }
            if(fibo_sfile_write(sPwrDownOpt.PDOptFileName, backData, fileSize) != fileSize)
            {
                sysLOG(DCEP_LOG_LEVEL_2, "Error!\r\n");
                return -26;
            }
            fileRemove_lib(BACKUPFILE);
            goto Exit;
        }
    }
Exit:
    memset(&sPwrDownOpt, 0x00, sizeof(DCEP_pwrDownOpt));
    iRet = fibo_sfile_write(POWERDOWNRECOVERY, &sPwrDownOpt, sizeof(DCEP_pwrDownOpt));
    if (iRet != sizeof(DCEP_pwrDownOpt))
    {
        sysLOG(DCEP_LOG_LEVEL_1, "saveError iRet=%d\r\n", iRet);
        return -27;
    }
    return 0;
}
