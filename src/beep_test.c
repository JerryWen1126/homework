#include "../inc/beep_test.h"

static int beep_fd = 0;

//1、打开蜂鸣器
void BEEP_Open(void)
{
	beep_fd = open("/dev/buzzer_drv",O_RDWR);
	if(beep_fd < 0)
	{
		printf("open beep error\n");
		return ;
	}
	
	BEEP_Button(0);
	return ;
}

//2、蜂鸣器开
void BEEP_Button(int button)
{
	if (button == 1)
		ioctl(beep_fd,GEC6818_BUZZER_ON);
	else if(button == 0)
		ioctl(beep_fd,GEC6818_BUZZER_OFF);
	//ioctl相当于read和write
	return ;
}

//3、关闭蜂鸣器
void BEEP_Close(void)
{
	close(beep_fd);
	return ;
}

// int main(int argc, char const *argv[])
// {
// 	//1、打开蜂鸣器
// 	BEEP_Open();

// 	//2、控制蜂鸣器
// 	while(1)
// 	{
// 		sleep(1);
// 		BEEP_Button(1);
// 		sleep(1);
// 		BEEP_Button(0);
// 	}
	

// 	//3、关闭蜂鸣器
// 	BEEP_Close();

// 	return 0;
// }