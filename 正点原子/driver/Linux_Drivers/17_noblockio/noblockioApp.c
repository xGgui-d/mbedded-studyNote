#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"
#include "linux/ioctl.h"

#define KEY0VALUE 0xF0
#define INVAKEY 0x00

int main(int argc, char *argv[])
{
    int cnt = 0;
    int fd, retvalue;
    char *filename;
    unsigned char data[1];
    int ret = 0;
    struct pollfd fds;
    fd_set readfds;         // 读操作文件描述集
    struct timeval timeout; // 超时结构体

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];
    // 打开驱动文件
    fd = open(filename, O_RDWR | O_NONBLOCK); // 以非阻塞方式访问
    if (fd < 0)
    {
        printf("Can't open file %s \r\n", filename);
        return -1;
    }
    // 循环读取
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // 构造超时时间
        timeout.tv_sec = 1;
        timeout.tv_usec = 0; // 1s

        ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
        switch (ret)
        {
        case 0:
            // 超时
            printf("is timeout!!!");
            break;
        case -1:
            // 错误
            printf("is error!!!");
            break;
        default:
            // 可以读取数据
            if (FD_ISSET(fd, &readfds))
            {
                ret = read(fd, &data, sizeof(data));
                if (ret < 0)
                {
                    /* 读取错误 */
                    //printf("read error\r\n");
                }
                else
                {
                    if (data)
                        printf("key value=%d\r\n", data);
                }
            }
            break;
        }
    }

    // 关闭设备
    retvalue = close(fd);
    if (retvalue < 0)
    {
        printf("Can't close file %s\r\n", filename);
        return -1;
    }

    return 0;
}