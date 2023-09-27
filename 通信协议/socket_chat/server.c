#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERV_PORT 11451 // 服务器端口
#define BUFSIZE 4096 // 缓冲区字节数
#define MAX_LISTEN 128 // 最大同时监听数
#define MAX_CONNECT 9 // 最大连接客户端数量

int sfd[128]; // 用于通信的文件描述符
int lfd; // 接待文件描述符
int connect_count = 0; // 当前的连接数

struct sockaddr_in client_addr[MAX_CONNECT]; // 定义客户端的地址结构数组
socklen_t client_addr_len[MAX_CONNECT]; // 定义客户端的地址结构长度数组
char client_ip[MAX_CONNECT][32]; // 定义客户端的ip

/* 出错函数 */
static void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

/* 将数据去掉头n个字节 */
static void del_head_char(int n, char *buf, int length)
{
    int i = 0;
    for(i = 0; i < length-n; i++) {
        buf[i] = buf[i+n];
    }
    buf[length-n] = '\0';
}

/* 终止信号处理函数 */
static void sig_handler()
{
    int i;
    printf("服务器中断\r\n");
    /* 关闭文件 */
    for(i = 0; i < connect_count; i++) {
        close(sfd[i]);
    }
    close(lfd);
    exit(0);    
}

/* 转发函数 */
static void *transmit_thread_start(void *arg)
{
    char read_buf[BUFSIZE];
    int sock_index = *(int *)arg;
    int ret;
    int i;
    printf("transmit_thread start id [%lu] socket index[%d]\r\n", pthread_self(), sock_index);
    while(1) {
        // 从socket读取数据
        ret = read(sfd[sock_index], read_buf, sizeof(read_buf));
        if(ret > 0) {
            printf("read_buf的数据：%s", read_buf);
            switch(read_buf[0]) {
            int save_fd;
            case 'l':  // 打印聊天室信息
                save_fd = dup(STDOUT_FILENO);
                dup2(sfd[sock_index], STDOUT_FILENO);
                for(i = 0; i < connect_count; i++) {
                    printf("编号[%d] IP地址[%s] 端口[%d]\r\n", i
                        , inet_ntop(AF_INET, &client_addr[i].sin_addr.s_addr, (void *)&client_ip[i], sizeof(client_ip[i]))
                        , ntohs(client_addr[i].sin_port));
                }
                dup2(save_fd, STDOUT_FILENO);   
                break;
            case 's':  // 群发消息
                del_head_char(1, read_buf, ret); // 将数据去掉头1个字节   
                for(i = 0; i < connect_count; i++) {
                    // 重定向printf到socket文件描述符        
                    save_fd = dup(STDOUT_FILENO);
                    dup2(sfd[i], STDOUT_FILENO);
                    printf("群发-> %s", read_buf);
                    dup2(save_fd, STDOUT_FILENO);
                }              
                break;
            case 'n':  // 指定发送消息
                    char c_num = read_buf[1];
                    int num = atoi(&c_num); // 保持编号
                    del_head_char(2, read_buf, ret); // 将数据去掉头2个字节
                    save_fd = dup(STDOUT_FILENO);
                    dup2(sfd[num], STDOUT_FILENO);
                    printf("客户端[%d]-> %s", sock_index, read_buf);
                    dup2(save_fd, STDOUT_FILENO);                
                break;
            }
        }
        /* 清空缓存 */
        for (unsigned int i = 0; i < ret; i++) {
            read_buf[i] = '\0'; 
        }
    }
    pthread_exit((void *)0);
}

int main()
{
    int ret = 0;
    int i = 0;

    pthread_t transmit_thread[MAX_CONNECT]; // 线程id
    void *pthret[MAX_CONNECT]; // 线程返回值

    /* 处理终止信号 ctrl+c*/
    __sighandler_t sig_ret;
    sig_ret = signal(SIGINT, sig_handler);
    if(sig_ret == SIG_ERR) {
        sys_err("signal error");
    }

    /* 初始化服务器地址结构 */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* sock 创建接待文件描述符 */
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1) {
        sys_err("socket error");
    }
    /* bind 绑定服务器的地址结构 */
    ret = bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret == -1) {
        sys_err("bind error");
    }
    /* listen 设置监听 */
    ret = listen(lfd, MAX_LISTEN);
    if(ret == -1) {
        sys_err("listen error");
    }
    /* 阻塞等待连接 */
    for(i = 0; i < MAX_CONNECT; i++) {
        client_addr_len[i] = sizeof(client_addr[i]);
        sfd[i] = accept(lfd, (struct sockaddr *)&client_addr[i], &client_addr_len[i]);
        if(sfd[i] == -1) {
            sys_err("accept error");
        }
        /* 创建子线程用于转发客户端的信息 */
        ret = pthread_create(&transmit_thread[i], NULL, transmit_thread_start, &i); // 参数   
        if(ret != 0) {
            sys_err("create thread error");
        }
        printf("client[%d] connet success!\r\n", i);
        printf("client ip[%s] port[%d]\r\n"
            , inet_ntop(AF_INET, &client_addr[i].sin_addr.s_addr, (char *)&client_ip[i], sizeof(client_ip[i]))
            , ntohs(client_addr[i].sin_port));
        connect_count++;   
        sleep(1);
    }

    /* 等待子线程结束 */
    for(i = 0; i < connect_count; i++) {
        ret = pthread_join(transmit_thread[i], &pthret[i]);
        if(ret != 0) {
            sys_err("join thread error");
        }     
    }

    /* 关闭文件 */
    for(i = 0; i < connect_count; i++) {
        close(sfd[i]);
    }
    close(lfd);

}