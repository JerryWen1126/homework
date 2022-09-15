/*
	自定义的头文件，打算把常用的头文件都包含进来
*/
#ifndef _UDP_H
#define _UDP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h> 
#include <sys/mman.h>
#include <linux/input.h>  //输入子系统模型有关的头文件 
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_6818_SRVPORT 60011
#define UDP_6818_CLIPORT 60012
#define UDP_UBUNTU_SRVPORT 60013
#define UDP_UBUNTU_CLIPORT 60014

#define IP_6818 "192.168.1.3"
#define IP_UBUNTU "192.168.1.3"

#define ALIVE 1
#define DEAD 0

static int udp_send_sock;
static struct sockaddr_in recvaddr;
static int is_udpRecv_th_alive = DEAD;

static int udp_recv_sock;
static struct sockaddr_in sendaddr;
static int addrsize;

// 1、建立客户端与服务器的连接
int Open_Send_Connect(char *send_ip, char *recv_ip);
// 2、客户端发送信息给服务器
void Send_To_Recv(char* arg);

// 1、建立客户端与服务器的连接
int Open_Recv_Connect(char *recv_ip);
// 2、客户端发送信息给服务器
void *Recv_From_Send(void *arg);

pthread_t start_udp_recv_th(void* task);
void shutdown_udp_recv_th(pthread_t pt);

// 3、关闭udp连接
int close_send_connect(void);
int close_recv_connect(void);

#endif /* _UDP_H */