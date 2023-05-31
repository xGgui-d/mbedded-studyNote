#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define KEY0VALUE 0xF0
#define INVAKEY 0x00


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
    // 打开驱动文件
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s \r\n", filename);
        return -1;
    }
    // 循环读取
    while (1)
    {
		ret = read(fd, &data, sizeof(data));
		if (ret < 0) {  /* 数据读取错误或者无效 */
			
		} else {		/* 数据读取正确 */
			if (data)	/* 读取到数据 */
				printf("key value = %#X\r\n", data);
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