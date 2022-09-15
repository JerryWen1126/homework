#include "../inc/main.h"
#include "../inc/udp.h"

int main(int argc, char const *argv[])
{
    // 创建udp连接
    Open_Send_Connect(IP_6818, IP_UBUNTU);
    Open_Recv_Connect(IP_UBUNTU);

    // 启动线程接收消息
    pthread_t Recv_From_Send_pt;
    Recv_From_Send_pt = start_udp_recv_th((void *)unlock);

    // 启动主循环
    while (1) {
        passwd_verify_page();
        goods_info_page();
    }

    shutdown_udp_recv_th(Recv_From_Send_pt);
    close_send_connect();
    close_recv_connect();

    return 0;
}
