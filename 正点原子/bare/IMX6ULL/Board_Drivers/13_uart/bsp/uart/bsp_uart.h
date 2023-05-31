#ifndef __BSP_UART_H
#define __BSP_UART_H
#include "imx6u.h"
// uart1 的 io 初始化
void uart_io_init();
// 串口的复位
void uart_softreset(UART_Type *base);
// 打开串口
void uart_enable(UART_Type *base);
// 关闭串口
void uart_disable(UART_Type *base);

// 串口的发送字符
void putc(unsigned char c);
// 串口接收数据
unsigned char getc();
// 通过串口发送一串字符
void puts(char* str);
// 初始化 uart 波特率固定为 115200
void uart_init();
void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz);
#endif
