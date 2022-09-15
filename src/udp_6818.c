#include "../inc/udp.h"

// 打开与接收端的连接
int Open_Send_Connect(char* send_ip, char* recv_ip)
{
    udp_send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_send_sock == -1) {
        perror("创建套接字失败!\n");
        return -1;
    }

    // 2_1、定义ipv4地址结构体变量(填写发送端的手机用户信息)
    struct sockaddr_in udpaddr;
    udpaddr.sin_family = AF_INET; // IPV4协议
    udpaddr.sin_addr.s_addr = inet_addr(send_ip); //发送端自己的ip
    udpaddr.sin_port = htons(UDP_6818_SRVPORT); //发送端自己的port(1024-65535)

    // 2_2、绑定ip和端口号(插卡，绑定手机号码)
    int ret = bind(udp_send_sock, (struct sockaddr*)&udpaddr, sizeof(udpaddr));
    if (ret == -1) {
        perror("bind失败!\n");
        return -1;
    }

    // 3、定义i结构体变量(填写女朋友手机用户信息)
    recvaddr.sin_family = AF_INET; // IPV4协议
    recvaddr.sin_addr.s_addr = inet_addr(recv_ip); //接收端的ip
    recvaddr.sin_port = htons(UDP_UBUNTU_CLIPORT); //接收端的端口号
}

// 打开与发送端的连接
int Open_Recv_Connect(char* recv_ip)
{
    udp_recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_recv_sock == -1) {
        perror("创建套接字失败!\n");
        return -1;
    }

    // 2_1、定义ipv4地址结构体变量(填写发送端的手机用户信息)
    struct sockaddr_in udpaddr;
    udpaddr.sin_family = AF_INET; // IPV4协议
    udpaddr.sin_addr.s_addr = inet_addr(recv_ip); //发送端自己的ip
    udpaddr.sin_port = htons(UDP_6818_CLIPORT); //发送端自己的port(1024-65535)

    // 2_2、绑定ip和端口号(插卡，绑定手机号码)
    int ret = bind(udp_recv_sock, (struct sockaddr*)&udpaddr, sizeof(udpaddr));
    if (ret == -1) {
        perror("bind失败!\n");
        return -1;
    }

    return 0;
}

// 发送消息
void Send_To_Recv(char* arg)
{
    // printf("send: %s\n", arg);
    sendto(udp_send_sock, arg, sizeof(arg), 0, (struct sockaddr*)&recvaddr, sizeof(recvaddr));
}

// 接收消息
void* Recv_From_Send(void* arg)
{
    void (*p)(void) = (void (*)(void))arg;
    char recv_buf[128] = { 0 };
    while (is_udpRecv_th_alive == ALIVE) {
        bzero(recv_buf, sizeof(recv_buf));
        recvfrom(udp_recv_sock, recv_buf, sizeof(recv_buf), MSG_WAITALL, (struct sockaddr*)&sendaddr, &addrsize);
        // printf("recv: %s\n", recv_buf);
        if (!strncmp(recv_buf, "ulck", 4)) {
            p();
        }
    }
}

pthread_t start_udp_recv_th(void* task)
{
    if (is_udpRecv_th_alive == ALIVE) {
        printf("目前只支持创建一个线程！");
        return NULL;
    }

    is_udpRecv_th_alive = ALIVE;
    pthread_t Recv_From_Send_pt;
    pthread_create(&Recv_From_Send_pt, NULL, Recv_From_Send, task);

    return Recv_From_Send_pt;
}

void shutdown_udp_recv_th(pthread_t pt)
{
    is_udpRecv_th_alive = DEAD;
    pthread_join(pt, NULL);
}


// 关闭udp的连接
int close_send_connect(void)
{
    close(udp_send_sock);
}

int close_recv_connect(void)
{
    close(udp_recv_sock);
}
