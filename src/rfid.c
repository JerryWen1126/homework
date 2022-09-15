#include "../inc/rfid.h"
#include <stdbool.h>

// 1、串口初始化
int tty_init(int fd)
{
    // 1、定义串口配置结构体
    struct termios new_cfg;
    // 2、激活串口设置
    cfmakeraw(&new_cfg);
    // 3、本地连接和接收使能
    new_cfg.c_cflag |= CLOCAL | CREAD;
    // 4、设置波特率输入输出都为115200
    cfsetispeed(&new_cfg, B9600);
    cfsetospeed(&new_cfg, B9600);
    // 5、用数据位掩码清空数据位设置，设置为8个数据位
    new_cfg.c_cflag &= ~CSIZE;
    new_cfg.c_cflag |= CS8;
    // 6、无奇偶校验位
    new_cfg.c_cflag &= ~PARENB;
    // 7、设置一个停止位
    new_cfg.c_cflag &= ~CSTOPB;
    // 8、设置等待时间或者获取几个字符后退出read的阻塞状态
    new_cfg.c_cc[VTIME] = 0;
    new_cfg.c_cc[VMIN] = 1;
    // 9、清空串口缓冲区
    tcflush(fd, TCIFLUSH);
    // 10、配置激活
    tcsetattr(fd, TCSANOW, &new_cfg);

    return 0;
}
// 2、校验位数据获取
char get_bcc(int n, char* buf)
{
    int i;
    char BCC = 0;
    for (i = 0; i < n; i++) {
        BCC ^= buf[i];
    }
    return (~BCC);
}

// 3、查找范围内是否有卡的存在
int rfid_send_A(int tty1)
{
    char rbuf[8] = { 0 };
    char wbuf[8] = { 0 };
    wbuf[0] = 0x07; //数据帧长度
    wbuf[1] = 0x02; //使用ISO14443A协议
    wbuf[2] = 0x41; //使用A命令
    wbuf[3] = 0x01; //数据长度
    wbuf[4] = 0x52; //选择所有
    wbuf[5] = get_bcc(wbuf[0] - 2, wbuf); //校验位
    wbuf[6] = 0x03; //结束符

    while (1) {
        //清空缓冲区
        tcflush(tty1, TCIFLUSH);
        write(tty1, wbuf, 7);
        rbuf[2] = -1;
        read(tty1, rbuf, sizeof(rbuf));
        //判断rfid模块返回数据是否正确
        if (rbuf[2] == 0) {
            // printf("send a ok!\n");
            break;
        } else {
            sleep(1);
            printf("No card information around!\n");
        }
    }

    return 0;
}

// 4、获取范围卡号最大的卡号的信息
int rfid_send_B(int tty1)
{
    int cardid = 0;
    char qbuf[10] = { 0 };
    char wbuf[8] = { 0 };

    //防碰撞（获取卡号信息），
    wbuf[0] = 0x08; //数据帧长度
    wbuf[1] = 0x02; //使用ISO14443A协议
    wbuf[2] = 'B'; //使用B命令
    wbuf[3] = 0x02; //数据长度
    wbuf[4] = 0x93; //第一级防碰撞
    wbuf[5] = 0x00; //位计数为0
    wbuf[6] = get_bcc(wbuf[0] - 2, wbuf); //校验位
    wbuf[7] = 0x03; //结束符

    while (1) {
        tcflush(tty1, TCIFLUSH);
        write(tty1, wbuf, 8);
        qbuf[2] = -1;
        read(tty1, qbuf, sizeof(qbuf)); // scanf
        if (qbuf[2] == 0) {
            // printf("send b ok!\n");
            cardid = qbuf[4] << 0 | qbuf[5] << 8 | qbuf[6] << 16 | qbuf[7] << 24;
            return cardid;
        } else {
            printf("faield\n");
            rfid_send_A(tty1);
            sleep(1);
        }
    }

    return 0;
}

// main-获取ic卡号信息
int Get_IC_Code(void)
{

    // 1、打开串口1
    int tty1 = open("/dev/ttySAC1", O_RDWR | O_NOCTTY);
    if (tty1 == -1) {
        perror("open tty1 failed");
        return -1;
    }
    // 2、串口初始化
    tty_init(tty1);

    // 3、发送A命令，查找范围内是否有卡的存在
    rfid_send_A(tty1);

    // 4、发送B命令，获取范围卡号最大的卡号的信息
    return rfid_send_B(tty1);
}

void* rfid_login(void* arg)
{
    rfid_th_is_start = true;
	int id;
    printf("正在获取卡号\n");
	id = Get_IC_Code();
	printf("id =  0x%X\n",  id);
	if (id == 0xDCDCBC69)
	{
		printf("欢迎光临！！\n");
		rf_check = true;
	}
	else
	{
		printf("你给我滚！！！\n");
	}
    printf("获取结束\n");
    rfid_th_is_start = false;
    pthread_exit(NULL);
}
