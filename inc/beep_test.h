#ifndef _BEEP_TEST_H					//预编译指令，作用是：防止递归包含以下内容
#define _BEEP_TEST_H

//一、函数头文件
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

//二、函数声明
void BEEP_Open(void);			//1、打开蜂鸣器
void BEEP_Button(int button);	//2、蜂鸣器开
void BEEP_Close(void);			//3、关闭蜂鸣器


//三、宏定义、变量声明、结构体等
#define GEC6818_BUZZER_ON _IO('B',1)
#define GEC6818_BUZZER_OFF _IO('B',2)


#endif	/*beep_test.h*/
