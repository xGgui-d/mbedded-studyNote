#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define LEDOFF 0
#define LEDON 1

int main(int argc, char *argv[])
{
    int cnt = 0;
    int fd, retvalue;
    char *filename;
    unsigned char databuf[1];

    if (argc != 3)
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

    databuf[0] = atoi(argv[2]); // 要执行的操作：打开或关闭

    // 向/dev/led 文件写入数据
    retvalue = write(fd, databuf, sizeof(databuf));
    if (retvalue < 0)
    {
        printf("LED Control Failed!\r\n");
        close(fd);
        return -1;
    }

    // 模拟应用占用驱动25s
    while(1)
    {
        sleep(5);
        cnt++;
        printf("app runing times %d\r\n",cnt);
        if(cnt>=5)
        break;

    }
    printf("app runing finish");
    
    // 关闭设备
    retvalue = close(fd);
    if (retvalue < 0)
    {
        printf("Can't close file %s\r\n", filename);
        return -1;
    }

    
    return 0;
}