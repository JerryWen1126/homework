#ifndef __RFID_H
#define __RFID_H

/*
	1、头文件
	2、宏定义
	3、函数声明
	....
*/
#include <termios.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdbool.h>

bool rf_check;  // RFID确认标志位
bool rfid_th_is_start;
bool is_camera_open;    //是否启动摄像头
char ch[128];	//存放显示的字


//1、串口初始化
int tty_init(int fd);

//2、校验位数据获取
char get_bcc(int n,char *buf);

//3、查找范围内是否有卡的存在
int rfid_send_A(int tty1);

//4、获取范围卡号最大的卡号的信息
int rfid_send_B(int tty1);

//main-获取ic卡号信息
int Get_IC_Code(void);

void* rfid_login(void*);

void *rfid_goods_check(void *);

#endif /* __RFID_H */




