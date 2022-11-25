/********************************Copyright ( c ) ********************************
**					Vanstone Electronic (Beijing) Co., Ltd
**						https://www.vanstone.com.cn
**
**
** File Name:		hal_font.h
** Description:		调用文件系统中的GB2312字库相关接口
**
** Version:	1.0, 渠忠磊,2022-02-28
** a) 修改内容 1 //修改内容精确到函数，并添加相应说明
** b) 修改内容 2 //代码中只有修改后内容
**
** History:
** a) 完成内容 1
** b) 完成内容 2
******************************************************************************/


#ifndef _HAL_FONT_H_
#define _HAL_FONT_H_

#include "comm.h"

#define FONT_LOG_LEVEL_0		LOG_LEVEL_0
#define FONT_LOG_LEVEL_1		LOG_LEVEL_1
#define FONT_LOG_LEVEL_2		LOG_LEVEL_2
#define FONT_LOG_LEVEL_3		LOG_LEVEL_3
#define FONT_LOG_LEVEL_4		LOG_LEVEL_4
#define FONT_LOG_LEVEL_5		LOG_LEVEL_5


#define IN
#define OUT

/* Pubilc define ---------------------------------------------------------------------------*/ 

/**
  * @Character Display format info Define.
	*/

#define MAX_FONT_NUMS       8
#define MAX_FONT_WIDTH      48
#define MAX_FONT_HEIGHT     48


/**
  * #Fontlib driver error code definaton.
	*/
#define SINGLE_CODE_FONT_NON         	(-2501)  	/*SingleCodeFont*/
#define	MULTICODEFONT_CODE_FONT_NON	 	(-2502)  	/*MultiCodeFont*/
#define SINGLE_CODE_FONT_OVER         	(-2503)		/*SingleCodeFont*/
#define MULTICODEFONT_CODE_FONT_OVER    (-2504)  	/*MultiCodeFont*/
#define FONTS_BUF_OVERFLOW				(-2505)
#define ERROR_FONT_PARAM				(-2513)
/**
  * #System Font-type define.
  * @Corresponding to ST_FONTD -> encode_type.
  */
#define ENCODE_UNICODE     0x00
#define ENCODE_WEST        0x01u              /*西文字体                                    */
#define ENCODE_TAI         0x02u              /* 需要/最好 修改为enum类型，规范及类型检测   */
#define ENCODE_MID_EUROPE  0x03u     
#define ENCODE_VIETNAM     0x04u     
#define ENCODE_GREEK       0x05u     
#define ENCODE_BALTIC      0x06u     
#define ENCODE_TURKEY      0x07u     
#define ENCODE_HEBREW      0x08u    
#define ENCODE_RUSSIAN     0x09u     
#define ENCODE_GB2312      0x0Au     
#define ENCODE_GBK         0x0Bu     
#define ENCODE_GB18030     0x0Cu     
#define ENCODE_BIG5        0x0Du     
#define ENCODE_SHIFT_JIS   0x0Eu     
#define ENCODE_KOREAN      0x0Fu     
#define ENCODE_ARABIA      0x10u   
#define ENCODE_DIY  	     0x11u


/**
  * @The char display attribute[distinguish the single-code or multi-code].
  */
typedef enum{
	SINGLE_CHAR = 0,
	MULTI_CHAR = 1,
	ARABIC_CHAR = 2,
}CHAR_TYPE;
/**
  * @FONTLIB_TYPE
  * #Mark current font-lib is internal array-dot font or extern-hex font-lib.
  */
typedef enum{
	INTERNAL_FONTLIB = 0,
	EXTERN_FONTLIB,
}FONTLIB_TYPE;
/** 
  * @Font-lib Struct information.
	* @attention : this description must modified synchronization with Font development doc.
	* @Font-lib Format description:
	* #[1] Font-lib Head Description :
	*======================================================================================
	* | No.  |   Value  |      Description    |--|  No. |  Value   |      Description     |
	* |  0.  |     V    | Font-lib Exited     |--|  8.  | version  | Font-lib version     |  
	* |  1.  |     S	| Flog ;              |--|  9.  | number   | Sub font in font-lib |  
	* |  2.  |     T    | vst is vanstone.    |--|  10. | Reserved |        -             |  
	* |  3.  |     -    |       -             |--|  11. | Reserved |        -             |  
	* |  4.  |     F    |       -             |--|  12. | Reserved |        -             |  
	* |  5.  |     O    |       -             |--|  13. | Reserved |        -             |  
	* |  6.  |     N    |       -             |--|  14. | Reserved |        -             |    
	* |  7.  |     T    |       -             |--|  15. | Reserved |        -             |   
	*======================================================================================
	* | No.  |   Value  |      Description    |--|  No. |   Value  |      Description     |
	* |  0.  |  CharSet |    ST_FONT mumber   |--|  8.  |    -     |         -            |  
	* |  1.  |   Width  |    ST_FONT mumber   |--|  9.  |    -     |  Sub font Address    |  
	* |  2.  |  Height  |    ST_FONT mumber   |--|  10. |    -     |  offset (4 Byte).    |  
	* |  3.  |   Bold   |    ST_FONT mumber   |--|  11. |----------|----------------------|  
	* |  4.  |  Italic  |    ST_FONT mumber   |--|  12. |    -     |         -            |  
	* |  5.  | CodeType | ST_FONT_TABLE mumber|--|  13. |    -     |  Sub Font lib        |  
	* |  6.  | Reserved |        -            |--|  14. |    -     |  Length(4Byte)       |    
	* |  7.  | Reserved |        -            |--|  15. |----------|----------------------| 
	*======================================================================================
  */
#pragma pack(1)
typedef struct _ST_FONT{
	CHAR_TYPE     CharSet;          /* Record the sub-font is signal-char/multi-char/arabic-char...*/
	int Width;
	int Height;
	int Bold;
	int Italic;
	int EncodeSet;                  /* the sub-font encode type.                                   */
}ST_FONT;

#pragma pack()

/**
  * @ST_FONT_TABLE is used for construct a table which can record system Font-Lib total information.
  */
typedef struct{
	ST_FONT       st_font;            /* Record sub-font information in detail.                 */
	unsigned long fontlib_len;        /* Record sub-font length.                                */
	unsigned char* sub_fontlib_addr;  /* Record sub-font's address in extern Font-Lib.          */
	int32 sub_fontlib_addroffset;	  /* Record sub-font's address offset in extern Font-Lib.   */	
}ST_FONT_TABLE;


/**
  * @ST_CURRENT_FONT is used for record sub-font information which is current using on screen.
	* #Differentiate the single code & multi code's purpose is using for convent the screen display 
	*   full-shaped char(eg:assic code char) and helf-shaped char(eg: chinese code char) at the 
	*   same time.
  */
typedef struct{
  ST_FONT single_code_font;         /* Record single code font info which in extern Font-Lib */                      
  unsigned char* single_dot_addr;   /* Record single code font's address in extern Font-Lib  */
  unsigned long single_font_lib_len;/* Record single code font's total length                */
  ST_FONT multi_code_font;          /* Record multi code font info which in extern Font-Lib  */ 
  unsigned char* multi_dot_addr;    /* Record multi code font's address in extern Font-Lib   */
  unsigned long multi_font_lib_len; /* Record multi code font's total length.                */
  int32	multi_dot_addroffset;		/* Record multi code font's address offset in extern Font-Lib */
}ST_CUR_FONT;

typedef struct{
	int CodeType;
	unsigned long FontLibLen;
	ST_FONT Font;
	unsigned char *FontDotPtr;
}ST_FONT_TABLE_V10;

struct ExternFontLibAttr{
	unsigned int fontlib_type;               /* Mark extern font-lib exit or not                */
	unsigned int fontlib_ver;                /* Record the extern font-lib's version.           */
	unsigned int sub_font_nums;              /* Record the sub-FontLib's Num int Extern Fontlib */
	unsigned char* exfontlib_addr;           /* Record the extern font-lib start address in flash or sram.*/
	ST_CUR_FONT  lcd_cur_font;               /* Record Lcd Current font attribte/info.          */
};

typedef struct _GB2312FONTCURRENT{

	int GB2312FontCurrent_Multi;
	int *GB2312FontCurrent_FileName;
	int GB2312FontCurrentMulti_height;
	int GB2312FontCurrentMulti_width;
	

}GB2312FONTCURRENT;

#define GB2312FONTDIR				"/ext/FONT/"
#define GB2312FONT12X12FILENAME		"/ext/FONT/GFont12X12.fon"
#define GB2312FONT16X16FILENAME		"/ext/FONT/GFont16X16.fon"
#define GB2312FONT24X24FILENAME		"/ext/FONT/GFont24X24.fon"
#define GB2312FONTFILENAME			"/ext/FONT/GFont.fon"

typedef enum{

	FONTNONE = 0x00,
	FONTFLASH = 0x80,
	FONTFS = 0x81,
	
}FONTTYPE;

typedef enum{

	FONTVERSIONUNICODE = 0x01,
	FONTVERSIONGB2312 = 0x02,
	
}FONTVERSION;

extern struct ExternFontLibAttr FontAttr;
extern ST_FONT g_enum_font[MAX_FONT_NUMS*2];


void DotBufReverse(char *dot_buf,unsigned int len);

int hal_fontGetFontType(void);
int hal_fontSetFontType(unsigned char FontType);
int hal_fontGetDot(IN int reverse,ST_CUR_FONT *CurFont,IN unsigned char const *str,OUT char *dot_buf,OUT unsigned int *width,OUT unsigned int *heigh);
void hal_fontInit(FONTLIB_TYPE fontlib_exit_flag,unsigned char *fontlib_addr);
int hal_fontLoad(void);
int hal_fontSelect(ST_FONT *SingleCodeFont, ST_FONT *MultiCodeFont);
int hal_fontEnum(ST_FONT *Fonts,int MaxFontNums);



unsigned short hal_fontUnicode2GBcode(unsigned short iUnicode);
unsigned short hal_fontGBcode2Unicode(unsigned short iGBcode);


#endif



