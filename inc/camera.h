#ifndef __CAMERA_H
#define __CAMERA_H

//一、函数头文件
#include <stdio.h>
#include "jpeglib.h"
#include "yuyv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <linux/input.h>
#include "jpeg.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include "font.h"


bool is_snap;
bool is_camera_open;    //是否启动摄像头
char ch[128];	//存放显示的字

//二、函数声明
//1、打开摄像头
void Camera_Open(char *pathname_camera);
//2、运行摄像头
void Camera_Show(int lcd_x, int lcd_y);
//3、关闭摄像头
void Camera_Close(void);


//三、宏定义、变量声明



#endif /* __CAMERA_H */