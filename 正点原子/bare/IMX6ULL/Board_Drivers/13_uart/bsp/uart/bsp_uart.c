#include "bsp_uart.h"
#include "bsp_gpio.h"
// 初始化 uart 波特率固定为 115200
void uart_init()
{
    // 初始化 UART1 的 IO
    uart_io_init();
    // 初始化串口
    // 关闭串口
    uart_disable(UART1);
    // 复位串口
    uart_softreset(UART1);
    // 配置 UART 1
    UART1->UCR1 = 0;
    //关闭自动波特率检测
    UART1->UCR1 &= ~(1<<14);

    // 配置 UART 2
    UART1->UCR2 = 0;    
    // 发送和接收使能，数据位是 8 位，忽略 RTS 引脚
    UART1->UCR2 |= (1 << 1) | (1 << 2) | (1 << 5) | (1 << 14);
    // utar3的 bit2 必须为1
    UART1->UCR3 |= (1 << 2);
    // 设置波特率 115200
    UART1->UFCR &= ~(7<<7);
    UART1->UFCR = 5 << 7;
 #if 0   
    //要先配置 UBIR 否则乱码
    UART1->UBIR = 71;    
    UART1->UBMR = 3124; //
#endif


    uart_setbaudrate(UART1, 115200, 80000000);
    // 使能串口
    uart_enable(UART1);
}

// 关闭串口
void uart_disable(UART_Type *base)
{
    base->UCR1 &= ~(1 << 0);
}
// 打开串口
void uart_enable(UART_Type *base)
{
    base->UCR1 |= (1 << 0);
}
// 串口的复位
void uart_softreset(UART_Type *base)
{
    base->UCR2 &= ~(1 << 0);
    while ((base->UCR2 & 0x1) == 0)
        ;
}

// uart1 的 io 初始化
void uart_io_init()
{
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0); // 复用为 IOMUXC_UART1_TX_DATA_UART1_TX
    IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX, 0x10b0);

    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0); // IOMUXC_UART1_RX_DATA_UART1_RX
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX, 0x10b0);
}

// 串口的发送字符
void putc(unsigned char c)
{
    // 判断上次数据发送完成
    while (((UART1->USR2 >> 3) & 0x01) == 0)
        ;
    // 开始发送数据
    UART1->UTXD = c;
}

// 串口接收数据
unsigned char getc()
{
    // 判断是否有数据可以接收
    while ((UART1->USR2 & 0x01) == 0)
        ;
    return UART1->URXD;
}

// 通过串口发送一串字符
void puts(char* str)
{
    char *p = str;
    while(*p)
        putc(*p++);
}

void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz)
{
    uint32_t numerator = 0u;		//分子
    uint32_t denominator = 0U;		//分母
    uint32_t divisor = 0U;
    uint32_t refFreqDiv = 0U;
    uint32_t divider = 1U;
    uint64_t baudDiff = 0U;
    uint64_t tempNumerator = 0U;
    uint32_t tempDenominator = 0u;

    /* get the approximately maximum divisor */
    numerator = srcclock_hz;
    denominator = baudrate << 4;
    divisor = 1;

    while (denominator != 0)
    {
        divisor = denominator;
        denominator = numerator % denominator;
        numerator = divisor;
    }

    numerator = srcclock_hz / divisor;
    denominator = (baudrate << 4) / divisor;

    /* numerator ranges from 1 ~ 7 * 64k */
    /* denominator ranges from 1 ~ 64k */
    if ((numerator > (UART_UBIR_INC_MASK * 7)) || (denominator > UART_UBIR_INC_MASK))
    {
        uint32_t m = (numerator - 1) / (UART_UBIR_INC_MASK * 7) + 1;
        uint32_t n = (denominator - 1) / UART_UBIR_INC_MASK + 1;
        uint32_t max = m > n ? m : n;
        numerator /= max;
        denominator /= max;
        if (0 == numerator)
        {
            numerator = 1;
        }
        if (0 == denominator)
        {
            denominator = 1;
        }
    }
    divider = (numerator - 1) / UART_UBIR_INC_MASK + 1;

    switch (divider)
    {
        case 1:
            refFreqDiv = 0x05;
            break;
        case 2:
            refFreqDiv = 0x04;
            break;
        case 3:
            refFreqDiv = 0x03;
            break;
        case 4:
            refFreqDiv = 0x02;
            break;
        case 5:
            refFreqDiv = 0x01;
            break;
        case 6:
            refFreqDiv = 0x00;
            break;
        case 7:
            refFreqDiv = 0x06;
            break;
        default:
            refFreqDiv = 0x05;
            break;
    }
    /* Compare the difference between baudRate_Bps and calculated baud rate.
     * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
     * baudDiff = (srcClock_Hz/divider)/( 16 * ((numerator / divider)/ denominator).
     */
    tempNumerator = srcclock_hz;
    tempDenominator = (numerator << 4);
    divisor = 1;
    /* get the approximately maximum divisor */
    while (tempDenominator != 0)
    {
        divisor = tempDenominator;
        tempDenominator = tempNumerator % tempDenominator;
        tempNumerator = divisor;
    }
    tempNumerator = srcclock_hz / divisor;
    tempDenominator = (numerator << 4) / divisor;
    baudDiff = (tempNumerator * denominator) / tempDenominator;
    baudDiff = (baudDiff >= baudrate) ? (baudDiff - baudrate) : (baudrate - baudDiff);

    if (baudDiff < (baudrate / 100) * 3)
    {
        base->UFCR &= ~UART_UFCR_RFDIV_MASK;
        base->UFCR |= UART_UFCR_RFDIV(refFreqDiv);
        base->UBIR = UART_UBIR_INC(denominator - 1); //要先写UBIR寄存器，然后在写UBMR寄存器，3592页 
        base->UBMR = UART_UBMR_MOD(numerator / divider - 1);
        //base->ONEMS = UART_ONEMS_ONEMS(srcclock_hz / (1000 * divider));
    }

}

void raise ()
{

}