#ifndef __FONT_H
#define __FONT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <strings.h>

#define color u32
#define getColor(a, b, c, d) (a|b<<8|c<<16|d<<24)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef struct stbtt_fontinfo
{
   void           * userdata;
   unsigned char  * data;             // pointer to .ttf file
   int              fontstart;        // offset of start of font

   int numGlyphs;                     // number of glyphs, needed for range checking

   int loca,head,glyf,hhea,hmtx,kern; // table locations as offset from start of .ttf
   int index_map;                     // a cmap mapping for our chosen character encoding
   int indexToLocFormat;              // format needed to map from glyph index to glyph
} stbtt_fontinfo;

typedef struct{
	u32 height;
	u32 width;
	u32 byteperpixel;
	u8 *map;
}bitmap;

typedef struct{
	stbtt_fontinfo *info;
	u8 *buffer;
	float scale;
}font;

//lcd设备结构体
struct LcdDevice
{
	int fd;
	unsigned int *mp; //保存映射首地址
};

//1、初始化字库 
font *fontLoad(char *fontPath);
//2、设置字体的大小 
void fontSetSize(font *f, s32 pixels);
//3.设置字体输出框的大小
bitmap *createBitmap(u32 width, u32 height, u32 byteperpixel);
//4、可以指定输出框的颜色
bitmap *createBitmapWithInit(u32 width, u32 height, u32 byteperpixel, color c);
//5、把字体输出到输出框中
void fontPrint(font *f, bitmap *screen, s32 x, s32 y, char *text, color c, s32 maxWidth);
//6、把输出框的所有信息显示到LCD屏幕中 
void show_font_to_lcd(unsigned int *p,int px,int py,bitmap *bm);
//7、关闭字体库
void fontUnload(font *f);
//8、关闭bitmap
void destroyBitmap(bitmap *bm);


void Font_Lcd_Open(void);			//1、打开lcd显示屏(字库)
/*
	2、显示字(可以指定字体大小，画板的x轴长度，画板的y轴长度，字在画板的x轴位置， 
       字在画板的y轴位置，文本，画在lcd的x轴位置，画在lcd的y轴位置，)
*/
void Font_Lcd_Show(s32 size, u32 bm_x, u32 bm_y, s32 f_bm_x,
                   s32 f_bm_y, char *text, int lcd_x, int lcd_y);

void Font_Lcd_Close(void);			//4、打开lcd显示屏(字库)

#endif