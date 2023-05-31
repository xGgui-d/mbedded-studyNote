#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#define CLOSE_CMD _IO(0XEF, 1)           // 关闭定时器
#define OPEN_CMD _IO(0xEF, 2)            // 打开定时器
#define SETPERIOD_CMD _IOW(0xED, 3, int) // 设置周期

int main(int argc, char *argv[])
{
    int cnt = 0;
    int fd, retvalue;
    char *filename;
    unsigned char databuf[1];

    unsigned int cmd;
    unsigned int arg;
    unsigned char str[100];

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];
    // 打开驱动文件
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s \r\n", filename);
        return -1;
    }

    while (1)
    {
        printf("Input CMD: ");
        retvalue = scanf("%d", &cmd);
        if (retvalue != 1)
        {
            gets(str);
        }
        if (cmd == 1)
            ioctl(fd, CLOSE_CMD);
        else if (cmd == 2)
            ioctl(fd, OPEN_CMD);
        else if (cmd == 3)
        {

            printf("input timer period: ");
            retvalue = scanf("%d", &arg);
            if (retvalue != 1)
            {
                gets(str);
            }
            ioctl(fd, SETPERIOD_CMD, arg);
        }
        else
            printf("is not a cmd\r\n");
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