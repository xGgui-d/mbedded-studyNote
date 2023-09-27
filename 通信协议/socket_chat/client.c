#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_PORT 11451 // 服务器端口
#define BUFSIZE 4096 // 缓冲区字节数

int back = 0;
int cfd;
char read_buf[BUFSIZE];

/* 出错函数 */
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

/* 接受sock信息 */
void *recv_info(void *arg)
{
    int ret;
    while(1) {
        ret = read(cfd, read_buf, sizeof(read_buf));
        if(ret != 0) {
            write(STDOUT_FILENO, read_buf, ret);
        }
    }
}

int main()
{
    pthread_t pthid; // 
    /* 初始化服务器地址结构 */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret;
    int i;
    char ctrl_buf[BUFSIZE];

    /* sock 创建客户端文件描述符 */
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1) {
        sys_err("sock error");
    }
    /* connect 建立连接 */
    ret = connect(cfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret == -1) {
        sys_err("connect error");
    }
    printf("服务器连接成功!!!\r\n");

    /* 建立子线程用于随时接受信息 */
    ret = pthread_create(&pthid, NULL, recv_info, NULL);
    if(ret != 0) {
        sys_err("thread create error");
    }    

    while(1) {

        printf("欢迎使用聊天室，功能如下: \r\n");
        printf("'l' 查看当前聊天室信息\r\n");
        printf("'s' 切换成群发模式\r\n");
        printf("'n[客户端编号]' 切换成指定发送模式\r\n");
        printf(":q 返回上一级\r\n");
        read(STDIN_FILENO, ctrl_buf, sizeof(ctrl_buf));
        switch(ctrl_buf[0]) {
        int save_fd;  
        char write_buf[BUFSIZE];          
        case 'l':
            printf("打印聊天室信息\r\n");
            write(cfd, ctrl_buf, sizeof(ctrl_buf)); 
            break;
        case 's': 
            while(1) {
                printf("请输入群发信息(:q退出):\r\n");
                ret = read(STDIN_FILENO, write_buf, sizeof(write_buf));
                if(write_buf[0] == ':' && write_buf[1] == 'q') {
                    break;
                }
                // 将缓存向后移动一位
                for(i = ret-1; i >= 0; i--) {
                    write_buf[i+1] = write_buf[i];
                }
                write_buf[0] = 's';
                write(cfd, write_buf, ret+1); 
            }
            break;
        case 'n': 
            while(1) {
                printf("请输入发送给客户端[%c]的信息(:q退出)\r\n", ctrl_buf[1]);
                ret = read(STDIN_FILENO, write_buf, sizeof(write_buf));
                if(write_buf[0] == ':' && write_buf[1] == 'q') {
                    break;
                }
                // 将缓存向后移动两位
                for(i = ret-1; i >= 0; i--) {
                    write_buf[i+2] = write_buf[i];
                }
                write_buf[0] = 'n';
                write_buf[1] = ctrl_buf[1];
                write(cfd, write_buf, ret+2); 
            }        
            break;            
        }

    }

    pthread_join(pthid, NULL);
    if(ret != 0) {
        sys_err("thread join error");
    }
    exit(0);

}