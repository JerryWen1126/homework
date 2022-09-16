#include "../inc/main.h"
#include "../inc/udp.h"
#include "../inc/camera.h"
#include "../inc/jpeg.h"
#include "../inc/rfid.h"

#define OFF_DIG_X 5
#define OFF_DIG_Y 15

//---全局变量---
//轮询数组:11位为true代表触摸屏线程不允许向轮询数组写哪个数字被摸
bool poll_array[11] = {0};
//确认与删除标志位
bool check = false;
bool rf_check = false; // RFID确认标志位
bool delete = false;
int passwd[6] = {0}; //这个如果放主函数里passwd[0]会变成7368034，我不到啊
int false_cnt = 0;
bool islocked = 0;
bool is_ts_th_pad_alive = 1;
bool is_ts_th_good_alive = 1;
bool input = false;            //延迟显示星号的输入标志位
bool rfid_th_is_start = false; //  RFID是否已经启动
bool is_snap = false;   //是否开始拍照
bool is_camera_open = false;    //是否启动摄像头
pthread_t ts_pt;
pthread_t rf_pt;

//显示任意大小的BMP图片
// x,y：显示的坐标
// w,h：图片的宽高
int all_bmp(const char *path, int x, int y, int w, int h)
{
    //判断要显示的图片是否合法
    if (x + w > 800 || y + h > 480)
    {
        perror("要显示的图片不合法");
        printf(">>>%s\n", path);
        return 0;
    }

    //获取图片显示的最大坐标
    int max_x = x + w;
    int max_y = y + h;

    // 1.打开BMP图片文件
    int bmp_fd = open(path, O_RDONLY);
    if (bmp_fd == -1)
    {
        perror("BMP图片文件打开失败!");
        return -1;
    }

    // 2.跳过54字节数据
    lseek(bmp_fd, 54, SEEK_SET);

    // 3.读取BMP图片文件数据
    int i, j;
    char bmp_buf[w * h * 3];
    int m_x = (4 - w * 3 % 4) % 4;

    for (i = 0; i < h; i++)
    {
        read(bmp_fd, &bmp_buf[i * w * 3], w * 3);
        lseek(bmp_fd, m_x, SEEK_CUR);
    }

    // 4.将24位图转化为32位图
    char lcd_buf[w * h * 4];
    for (i = 0, j = 0; i < w * h * 3; i += 3, j += 4)
    {
        lcd_buf[j] = bmp_buf[i];
        lcd_buf[j + 1] = bmp_buf[i + 1];
        lcd_buf[j + 2] = bmp_buf[i + 2];
        lcd_buf[j + 3] = 0;
    }

    // 5.上下颠倒
    char show_buf[w * h * 4];
    for (i = 0; i < h; i++)
    {
        for (j = 0; j < w * 4; j++)
        {
            show_buf[i * w * 4 + j] = lcd_buf[(h - i - 1) * w * 4 + j];
        }
    }

    // 5.打开LCD显示屏文件
    int lcd_fd = open("/dev/fb0", O_RDWR);
    if (lcd_fd == -1)
    {
        perror("LCD显示屏打开失败!");
        return -1;
    }

    // 6.内存映射
    char *lcd = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (lcd == (void *)-1)
    {
        perror("内存映射失败!");
        close(lcd_fd);
        close(bmp_fd);
        return -1;
    }

    // 7.向LCD显示屏写入数据
    int k = 0;
    for (i = y; i < max_y; i++)
    {
        for (j = x * 4; j < max_x * 4; j++)
        {
            lcd[i * 800 * 4 + j] = show_buf[k];
            k++;
        }
    }

    //关闭内存映射
    munmap(lcd, 800 * 480 * 4);
    //关闭LCD显示屏文件
    close(lcd_fd);
    //关闭BMP图片文件
    close(bmp_fd);

    return 0;
}

//在屏幕上某区域显示颜色
int show_color(int lcd_x, int lcd_y, int color_w, int color_h, unsigned int color)
{
    // 1、打开LCD显示屏文件（以只写形式打开）
    int lcd_fd = open("/dev/fb0", O_RDWR);
    if (-1 == lcd_fd)
    {
        perror("Open lcd error");
        return -1;
    }

    // 2.1、向LCD显示屏文件写文件数据
    /*     unsigned int lcd_buf[800 * 480] = {0};
        unsigned int i;
        for (i = 0; i < 800 * 480; i++)
        {
            lcd_buf[i] = color;
        };
        ssize_t ret_w = write(lcd_fd, lcd_buf, sizeof(lcd_buf));
        if (ret_w == -1)
        {
            perror("Write lcd error");
            return -1;
        }
        printf("Have written %d bytes\n", ret_w); */

    // 2.2、使用mmap函数编写
    unsigned int *lcd_p = mmap(NULL,
                               800 * 480 * 4,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               lcd_fd,
                               (off_t)0);
    unsigned int *lcd_temp_p = lcd_p;
    if (lcd_p == (void *)-1)
    {
        perror("mmap lcd error");
        return -1;
    }
    // 2.2.1、指定任意位置显示
    lcd_p = lcd_p + lcd_x;
    lcd_p = lcd_p + lcd_y * 800;
    // 2.2.2、任意大小显示颜色
    int x, y;
    for (y = 0; y < color_h; y++)
    {
        for (x = 0; x < color_w; x++)
        {
            *(lcd_p + y * 800 + x) = color;
        }
    }

    // 3、解除映射，关闭LCD显示屏文件
    // 3.1、解除映射
    int ret_m = munmap(lcd_temp_p, 800 * 480 * 4);
    if (ret_m == -1)
    {
        perror("munmap error");
        return -1;
    }

    // 3、关闭LCD显示屏文件
    int ret_c = close(lcd_fd);
    if (ret_c == -1)
    {
        perror("Close lcd error");
        return -1;
    }

    return 0;
}

//触摸屏线程
void *touch_screen_for_pad(void *arg)
{
    int ts_fd = open("/dev/input/event0", O_RDONLY);
    if (ts_fd == -1)
    {
        perror("打开文件失败！\n");
        return NULL;
    }

    struct input_event ts_buf;
    int ts_x = 0;
    int ts_y = 0;
    int touch = 0;

    while (is_ts_th_pad_alive == true)
    {
        while (is_ts_th_pad_alive == true)
        {
            memset(&ts_buf, 0, sizeof(ts_buf));
            read(ts_fd, &ts_buf, sizeof(ts_buf));

            // 获取x
            if (ts_buf.type == EV_ABS && ts_buf.code == ABS_X)
            {
                ts_x = ts_buf.value;
            }
            // 获取y
            if (ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)
            {
                ts_y = ts_buf.value;
            }
            // 获取触摸值
            if (ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH)
            {
                touch = ts_buf.value;
                break;
            }
        }
        // printf("(%d, %d, %d)\n", ts_x, ts_y, touch); // touch：按下是1，松开是0

        //如果松开
        if (!touch && is_ts_th_pad_alive)
        {
            all_bmp("./img/pad.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
        }
        //按下确认
        else if (ts_x > 632 && ts_x < 696 && ts_y > 120 && ts_y < 184 && touch == 1)
        {
            all_bmp("./img/checkp.bmp", 632, 120, 64, 64); // 检查按钮
            check = true;
        }
        //按下删除
        else if (ts_x > 632 && ts_x < 696 && ts_y > 288 && ts_y < 352 && touch == 1)
        {
            all_bmp("./img/backp.bmp", 632, 288, 64, 64); // 删除按钮
            delete = true;
        }
        //按下0
        else if (ts_x > 632 && ts_x < 696 && ts_y > 204 && ts_y < 268 && touch == 1)
        {
            all_bmp("./img/pad0.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[0] = true;
            }
        }
        //按下1
        else if (ts_x > 550 && ts_x < 614 && ts_y > 288 && ts_y < 352 && touch == 1)
        {
            all_bmp("./img/pad1.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[1] = true;
            }
        }
        //按下2
        else if (ts_x > 550 && ts_x < 614 && ts_y > 204 && ts_y < 268 && touch == 1)
        {
            all_bmp("./img/pad2.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[2] = true;
            }
        }
        //按下3
        else if (ts_x > 550 && ts_x < 614 && ts_y > 120 && ts_y < 184 && touch == 1)
        {
            all_bmp("./img/pad3.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[3] = true;
            }
        }
        //按下4
        else if (ts_x > 486 && ts_x < 532 && ts_y > 288 && ts_y < 352 && touch == 1)
        {
            all_bmp("./img/pad4.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[4] = true;
            }
        }
        //按下5
        else if (ts_x > 486 && ts_x < 532 && ts_y > 204 && ts_y < 268 && touch == 1)
        {
            all_bmp("./img/pad5.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[5] = true;
            }
        }
        //按下6
        else if (ts_x > 486 && ts_x < 532 && ts_y > 120 && ts_y < 184 && touch == 1)
        {
            all_bmp("./img/pad6.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[6] = true;
            }
        }
        //按下7
        else if (ts_x > 404 && ts_x < 468 && ts_y > 288 && ts_y < 352 && touch == 1)
        {
            all_bmp("./img/pad7.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[7] = true;
            }
        }
        //按下8
        else if (ts_x > 404 && ts_x < 468 && ts_y > 204 && ts_y < 268 && touch == 1)
        {
            all_bmp("./img/pad8.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[8] = true;
            }
        }
        //按下9
        else if (ts_x > 404 && ts_x < 468 && ts_y > 120 && ts_y < 184 && touch == 1)
        {
            all_bmp("./img/pad9.bmp", 386, 0, 414, 480);
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            if (poll_array[10] != true)
            {
                poll_array[9] = true;
            }
        }
    }

    int ret_c = close(ts_fd);
    if (ret_c == -1)
    {
        perror("Close ts error");
        return NULL;
    }

    pthread_exit(NULL);
    return NULL;
}

void *touch_screen_for_goods(void *arg)
{
    int ts_fd = open("/dev/input/event0", O_RDONLY);
    if (ts_fd == -1)
    {
        perror("打开文件失败！\n");
        return NULL;
    }

    struct input_event ts_buf;
    int ts_x = 0;
    int ts_y = 0;
    int touch = 0;

    while (is_ts_th_good_alive == true)
    {
        while (is_ts_th_good_alive == true)
        {
            memset(&ts_buf, 0, sizeof(ts_buf));
            read(ts_fd, &ts_buf, sizeof(ts_buf));

            // 获取x
            if (ts_buf.type == EV_ABS && ts_buf.code == ABS_X)
            {
                ts_x = ts_buf.value;
            }
            // 获取y
            if (ts_buf.type == EV_ABS && ts_buf.code == ABS_Y)
            {
                ts_y = ts_buf.value;
            }
            // 获取触摸值
            if (ts_buf.type == EV_KEY && ts_buf.code == BTN_TOUCH)
            {
                touch = ts_buf.value;
                break;
            }
        }
        printf("(%d, %d, %d)\n", ts_x, ts_y, touch); // touch：按下是1，松开是0

        //如果松开
        if (!touch && is_ts_th_good_alive)
        {
            // all_bmp("./img/background.bmp", 0, 0, 800, 480);
            all_bmp("./img/back_to_pad.bmp", 0, 0, 75, 75); //返回主界面
            all_bmp("./img/snap.bmp", 363, 350, 75, 75);    // 拍照按钮
        }
        //按下返回
        else if (ts_x > 0 && ts_x < 75 && ts_y > 0 && ts_y < 75 && touch == 1)
        {
            printf("按下返回\n");
            is_camera_open = false;
        }
        else if (ts_x > 363 && ts_x < 438 && ts_y > 350 && ts_y < 425 && touch == 1)
        {
            printf("按下拍照\n");
            int i;
            is_snap = true;
            for (i = 0; i < 10; i++)
            {
                all_bmp("./img/background.bmp", 0, 0, 800, 480);
            }
            is_snap = false;
        }
    }

    int ret_c = close(ts_fd);
    if (ret_c == -1)
    {
        perror("Close ts error");
        return NULL;
    }

    pthread_exit(NULL);
    return NULL;
}

//经计算后的屏幕分区函数
void show_passwd(const char *path, int row, int column)
{
    all_bmp(path, OFF_DIG_X + (row - 1) * 75, OFF_DIG_Y + 75 * (6 - column), 75, 75);
}

void passwd_verify_page(void)
{
    printf("开始验证密码\n");
    //初始化
    memset(poll_array, 0, 11);
    memset(passwd, 0, 6);
    check = false;
    rf_check = false;
    false_cnt = 0;
    islocked = 0;
    is_ts_th_pad_alive = 1;
    input = false;

    int ret_p = pthread_create(&ts_pt, NULL, touch_screen_for_pad, NULL);
    if (ret_p != 0)
    {
        perror("MAIN:touch screen:Pthread create error");
        exit(1);
    }
    else
    {
        printf("按键线程创建成功！");
    }

    // 启动RFID线程检测ID卡
    if (rfid_th_is_start == false)
    {
        ret_p = pthread_create(&rf_pt, NULL, rfid_login, NULL);
        if (ret_p != 0)
        {
            perror("MAIN:rfid:Pthread create error");
            exit(2);
        }
    }

    // 主线程负责图片的驱动
    // 初始化界面
    all_bmp("./img/pad.bmp", 386, 0, 414, 480);   // 初始界面
    all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
    all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
    show_color(0, 0, 386, 480, 0x00FFFF00);       // 密码层黄底
    show_color(145, 0, 95, 480, 0x00FFFFFF);      // 输入框白底

    int i, row = 1, column = 1;
    char bmp_name[8];
    //光标闪烁延迟
    int set_delay = 1e7, cur;
    // *显示延迟
    int x_delay = 1e7, x_cur;
    while (true)
    {
        if (islocked == false)
        {
            //负责光标闪烁
            if (column < 7)
            {
                if (cur == set_delay)
                {
                    show_passwd("./img/white.bmp", 3, column);
                    cur = 0;
                }
                else if (cur == set_delay / 2)
                {
                    show_passwd("./img/cursor.bmp", 3, column);
                }
                cur++;
            }

            // 负责界面图片更新
            // 循环检查轮询数组
            //显示星号计数器
            x_cur++;

            for (i = 0; i < 10; i++)
            {
                if (poll_array[i] == true)
                {
                    x_cur = 0;
                    // 密码数码距离顶部底部各15pixel显示效果更好
                    // 可显示5行，距离左边右边各偏移5pixel显示效果最好

                    // if (column > 6)  // 关了轮询数组写标志位，不可能执行到这里，故注释
                    // {
                    //     break;
                    // }
                    passwd[column - 1] = i;
                    //获取digital.bmp文件名
                    sprintf(bmp_name, "./img/%d.bmp", i);
                    show_passwd(bmp_name, 3, column);

                    //输入标志位置高
                    input = true;

                    if (column > 1)
                    {
                        show_passwd("./img/x.bmp", 3, column - 1);
                    }

                    column++;
                    if (column == 7)
                    {
                        poll_array[10] = true; //轮询数组关闭写
                        // show_passwd("./img/x.bmp", 3, column - 1);
                    }
                    poll_array[i] = false; //轮询数组关闭数字标志位
                }
            }
            // 检查密码数组
            if (delete == true)
            {
                if (column < 7)
                {
                    show_passwd("./img/white.bmp", 3, column); // 消除光标残影
                }
                if (column - 1 > 0)
                {
                    show_passwd("./img/white.bmp", 3, --column);
                    poll_array[10] = false;
                }

                delete = false;
            }
            else if (check == true)
            {
                show_color(145, 0, 95, 480, 0x00FFFFFF); // 输入框白底
                if (column == 7)
                {
                    // 检查密码
                    char pwd[6];
                    int l;
                    for (l = 0; l < 6; l++)
                    {
                        pwd[l] = '0' + passwd[l];
                    }
                    pwd[l] = '\0';
                    if (!strcmp(pwd, "114514"))
                    {
                        all_bmp("./img/pwd_true.bmp", 255, 0, 115, 480);

                        // 错误计数器清零
                        false_cnt = 0;

                        // TODO
                        if (rfid_th_is_start == true)
                        {
                            pthread_cancel(rf_pt);
                            rfid_th_is_start = false;
                        }

                        // 退出按键检测线程
                        is_ts_th_pad_alive = 0;

                        // 退出这个页面，进入下个页面
                        // goods_info_page();

                        break;
                    }
                    else
                    {
                        all_bmp("./img/pwd_false.bmp", 255, 0, 115, 480);

                        // 如果错误次数达到三则锁死系统
                        // 同时通知ubuntu
                        printf("密码错误\n");
                        false_cnt = (false_cnt + 1) % 3;
                        if (false_cnt == 0)
                        {
                            printf("累计错误三次\n");
                            lock();
                            notify_ubuntu(Lock);
                        }
                    }
                }
                else
                {
                    all_bmp("./img/pwd_false.bmp", 255, 0, 115, 480);

                    // 如果错误次数达到三则锁死系统
                    // 同时通知ubuntu
                    printf("密码错误\n");
                    false_cnt = (false_cnt + 1) % 3;
                    if (false_cnt == 0)
                    {
                        printf("累计错误三次\n");
                        lock();
                        notify_ubuntu(Lock);
                    }
                    check = false;
                }
            }
            else if (rf_check == true) // RFID识别登录
            {
                rf_check = false;
                printf("RFID check success\n");
                notify_ubuntu(Login);

                // 退出按键检测线程
                is_ts_th_pad_alive = 0;

                // 退出这个页面，进入下个页面
                // goods_info_page();

                break;
            }
            //定时器显示星号
            if ((x_cur >= x_delay) && (input == true))
            {
                show_passwd("./img/x.bmp", 3, column - 1);
                printf("show *\n");
                input = false;
                x_cur = 0;
            }
        }
        else
        {
            // 锁死界面
            show_color(0, 0, 386, 480, 0x00FFFFFF);
            all_bmp("./img/lock.bmp", 110, 47, 130, 385);

            while (islocked == true)
                ;

            all_bmp("./img/pad.bmp", 386, 0, 414, 480);   // 初始界面
            all_bmp("./img/check.bmp", 632, 120, 64, 64); // 检查按钮
            all_bmp("./img/back.bmp", 632, 288, 64, 64);  // 删除按钮
            show_color(0, 0, 386, 480, 0x00FFFF00);       // 密码层黄底
            show_color(145, 0, 95, 480, 0x00FFFFFF);      // 输入框白底
        }
    }
    // 主进程触摸屏线程已回收
    printf("主进程触摸屏线程已回收\n");
    // 不回收格外流畅,放main函数比较好
    // pthread_join(ts_pt, NULL); // 回收触摸屏检测线程

    printf("退出密码验证界面！\n");
}

void goods_info_page(void)
{
    is_ts_th_good_alive = true; //允许启动商品触摸屏检测线程
    printf("登录系统\n");

    // // 密码正确，登陆系统
    // // 同时通知ubuntu
    notify_ubuntu(Login);

    // //1、打开摄像头
    Camera_Open("/dev/video7");

    pthread_t ts_good_pt;
    int ret_p = pthread_create(&ts_good_pt, NULL, touch_screen_for_goods, NULL);
    if (ret_p != 0)
    {
        perror("GOODS:touch screen:Pthread create error");
        exit(1);
    }
    else
    {
        printf("商品按键线程创建成功！");
    }

    is_camera_open = true;  //解除摄像头死循环,因为退出商品RFID检测线程使用了该标志位，必须在启动RFID检测线程之前

    pthread_t rf_good_pt;
    ret_p = pthread_create(&rf_good_pt, NULL, rfid_goods_check, NULL);
    if (ret_p != 0)
    {
        perror("GOODS:RFID:Pthread create error");
        exit(1);
    }
    else
    {
        printf("商品RFID检测线程创建成功!");
    }


    all_bmp("./img/background.bmp", 0, 0, 800, 480);
    all_bmp("./img/snap.bmp", 363, 350, 75, 75);
    all_bmp("./img/back_to_pad.bmp", 0, 0, 75, 75); //返回主界面
    // //2、运行摄像头
    Camera_Show(225, 75);
    is_ts_th_good_alive = false;    //关闭商品界面触摸屏线程

    // 3、关闭摄像头
    Camera_Close();

    printf("退出商品信息页面！\n");
}

// 密码正确时，登陆系统
void login()
{
}

// 登出系统
void logout()
{
    printf("登出系统\n");
}

// 密码错误三次时，锁死系统
void lock()
{
    islocked = true;
    printf("锁死系统\n");
}

// 解锁
void unlock(void)
{
    islocked = false;
    printf("解锁系统\n");
}

// 通知ubuntu
void notify_ubuntu(int type)
{
    switch (type)
    {
    case Login:
        Send_To_Recv("0");
        break;

    case Lock:
        Send_To_Recv("1");
        break;

    default:
        break;
    }
}
