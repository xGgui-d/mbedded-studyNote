#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>

/* input event 结构体变量 */
static struct input_event inputevent;

int main(int argc, char *argv[])
{
    int cnt = 0;
    int fd, retvalue;
    char *filename;
    unsigned char data[1];
    int ret;

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];

    /* 打开驱动文件 */
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }

    while (1)
    {
        ret = read(fd, &inputevent, sizeof(inputevent));
        if (ret > 0) // 数据读取成功
        {
            switch (inputevent.type)
            {
            case EV_KEY:
                printf("EV_KEY event\r\n");
                if (inputevent.code < BTN_MISC)
                    printf("code is %d value is %s\r\n", inputevent.code, inputevent.value?"press":"release");
                break;
            case EV_SYN:
                printf("EV_SYN event\r\n");
                break;
            case EV_REP:
                printf("EV_REP event\r\n");
                break;
            }
        }
        else
        {
            printf("data read fail!!\r\n");
        }
    }

    /* 关闭驱动文件 */
    retvalue = close(fd);
    if (retvalue < 0)
    {
        printf("Can't close file %s\r\n", filename);
        return -1;
    }

    return 0;
}