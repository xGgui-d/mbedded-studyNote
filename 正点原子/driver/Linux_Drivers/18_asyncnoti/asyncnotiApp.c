#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <fcntl.h>

#define KEY0VALUE 0xF0
#define INVAKEY 0x00

int fd, retvalue;
int cnt = 0;
char *filename;
unsigned char data[1];
int ret;

void sigio_handler(int num)
{
    int ret;
    
    ret = read(fd, &data, sizeof(data));
    if (ret < 0)
    { /* 数据读取错误或者无效 */
    }
    else
    {             /* 数据读取正确 */
        if (data) /* 读取到数据 */
            printf("sigio signal happend !!key value = %#X\r\n", data);
    }

}

int main(int argc, char *argv[])
{

    int flags;
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

    // 设置信号处理函数
    signal(SIGIO, sigio_handler);

    // 设置当前的线程接收 SIGIO 信号
    fcntl(fd, F_SETOWN,getpid());
    
    // 获取当前的进程状态
    flags = fcntl(fd, F_GETFL);

    // 开启异步通知
    fcntl(fd, F_SETFL, flags | FASYNC);

    while(1)
    {

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