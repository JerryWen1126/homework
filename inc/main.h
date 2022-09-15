#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

// 全局变量”轮询数组“：用于线程间通信
bool poll_array[11];
bool check;
bool delete;
int false_cnt;
bool islocked;
pthread_t ts_pt;
pthread_t rf_pt;


enum type {
    Login,
    Lock
};

int show_color(int lcd_x, int lcd_y, int color_w, int color_h, unsigned int color);
void* touch_screen_for_pad(void *);
int all_bmp(const char* path, int x, int y, int w, int h);
void show_passwd(const char* path, int row, int column);

void login();
void logout();
void lock(void);
void unlock(void);
void notify_ubuntu(int type);


#endif