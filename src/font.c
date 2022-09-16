#include "../inc/font.h"

static struct LcdDevice* lcd;

//1、打开lcd显示屏(字库)
void Font_Lcd_Open(void)
{
	//申请空间
	lcd = malloc(sizeof(struct LcdDevice));
	if(lcd == NULL){
		return ;
	} 
	//1打开设备
	lcd->fd = open("/dev/fb0", O_RDWR);
	if(lcd->fd < 0){
		perror("open lcd fail");
		free(lcd);
		return ;
	}
	//映射
	lcd->mp = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd->fd,0);

	return ;
}

/*
	2、显示字(可以指定字体大小，画板的x轴长度，画板的y轴长度，字在画板的x轴位置， 
       字在画板的y轴位置，文本，画在lcd的x轴位置，画在lcd的y轴位置，)
*/
void Font_Lcd_Show(s32 size, u32 bm_x, u32 bm_y, s32 f_bm_x,
                   s32 f_bm_y, char *text, int lcd_x, int lcd_y)
{			
	//1、打开字体	
	font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
	  
	//2、字体大小的设置
	fontSetSize(f,size);	//36
	
	//3、创建一个画板（点阵图）
	bitmap *bm = createBitmapWithInit(bm_x,bm_y,4,getColor(0,255, 255, 255));
	                                                    //画板的颜色ABGR
                
	//4、将字体画到画板上(点阵图上)
	fontPrint(f,bm,f_bm_x,f_bm_y,text,getColor(0,0,0,0),0); //72
							         //字体的颜色ABGR


	//5、把形成的画显示到LCD屏幕上
	show_font_to_lcd(lcd->mp,lcd_x,lcd_y,bm);

	//6、关闭字体，关闭画板
	fontUnload(f);
	destroyBitmap(bm);
		
}


//3、关闭lcd显示屏(字库)
void Font_Lcd_Close(void)
{
	//1、解除lcd映射
	int ret_m = munmap(lcd->mp, 800*480*4);
	if (-1 == ret_m){
		perror("munmap lcd error!\n");
		goto Label;
	}
	//2、关闭lcd显示屏文件
	int ret_c = close(lcd->fd);
	if (-1 == ret_c){
		perror("close lcd error!\n");
		goto Label;
	}	
Label:
	munmap(lcd->mp, 800*480*4);
	close(lcd->fd);

}


// int main(int argc, char const *argv[])
// {
// 	//1、打开lcd显示屏(字库)
// 	Font_Lcd_Open();

// 	//2、显示字
// 	int font_offset = 0;
// 	while(1)
// 	{
// 		Font_Lcd_Show(54, 800, 100, font_offset, 0, "美媛春肾宝，他好我也好！！", 100, 100);
// 		font_offset++;
// 		if (font_offset >= 800)
// 		{
// 			font_offset = 0;
// 		}
// 		usleep(1000); //1000微妙
// 	}


// 	//3、关闭lcd显示屏(字库)
// 	Font_Lcd_Close();
// 	return 0;
// }