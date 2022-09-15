#include "../inc/udp.h"

static int udp_send_sock;
static struct sockaddr_in recvaddr;

static int udp_recv_sock;
static struct sockaddr_in sendaddr;
static int addrsize;

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
    udpaddr.sin_port = htons(UDP_UBUNTU_SRVPORT); //发送端自己的port(1024-65535)

    // 2_2、绑定ip和端口号(插卡，绑定手机号码)
    int ret = bind(udp_send_sock, (struct sockaddr*)&udpaddr, sizeof(udpaddr));
    if (ret == -1) {
        perror("bind失败!\n");
        return -1;
    }

    recvaddr.sin_family = AF_INET; // IPV4协议
    recvaddr.sin_addr.s_addr = inet_addr(recv_ip); //接收端的ip
    recvaddr.sin_port = htons(UDP_6818_CLIPORT); //接收端的端口号
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
    udpaddr.sin_port = htons(UDP_UBUNTU_CLIPORT); //发送端自己的port(1024-65535)

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
    printf("send: %s\n", arg);
    sendto(udp_send_sock, arg, sizeof(arg), 0, (struct sockaddr*)&recvaddr, sizeof(recvaddr));
}

// 接收消息
void* Recv_From_Send(void* arg)
{
    int log_fd = (long int)arg;
    char recv_buf[128] = { 0 };

    while (1) {
        bzero(recv_buf, sizeof(recv_buf));
        recvfrom(udp_recv_sock, recv_buf, sizeof(recv_buf), MSG_WAITALL, (struct sockaddr*)&sendaddr, &addrsize);
        
        // 写入日志文件
        // printf("recv: %s\n", recv_buf);
        if (!strncmp(recv_buf, "0", 1)) {
            write(log_fd, "login\n", 6);
        } else if (!strncmp(recv_buf, "1", 1)) {
            write(log_fd, "lock\n", 5);
        } else {
            printf("前端数据错误!\n");
        }
    }
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

int main()
{
    long int log_fd = open("./1.txt", O_WRONLY | O_CREAT | O_APPEND);

    // 创建udp连接
    Open_Send_Connect(IP_UBUNTU, IP_6818);
    Open_Recv_Connect(IP_6818);

    // 启动线程接收消息
    pthread_t Recv_From_Send_pt;
    pthread_create(&Recv_From_Send_pt, NULL, Recv_From_Send, (void *)log_fd);

    char send_buf[128] = {0};
    printf("%s\n", "发送ulck可解锁系统");
    while(1) {
        memset(send_buf, 0, sizeof(send_buf));
        scanf("%s", send_buf);
        Send_To_Recv(send_buf);
    }

    // 关闭连接
    Close_Connect();
    pthread_join(Recv_From_Send_pt, NULL);

    return 0;
}