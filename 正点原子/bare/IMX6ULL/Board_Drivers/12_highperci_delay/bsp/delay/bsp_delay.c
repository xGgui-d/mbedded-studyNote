#include "bsp_delay.h"
#include "bsp_int.h"
#include "bsp_led.h"
/*延时初始化函数*/
void delay_init()
{
    // 清零cr
    GPT1->CR = 0;
    // 将SWR位置一，然后该位会自动清0
    GPT1->CR = 1 << 15;
    // 判断直到自动清零完毕跳出循环
    while ((GPT1->CR >> 15) & 0x01)
        ;

    // 开始配置 GPT ipg_clk = 66M  restart 模式
    // 计数器寄存器从0 开始计时
    GPT1->CR |= (1 << 1) | (1 << 6) | (0 << 9);
    // 设置分频
    GPT1->PR = 65; // 66分频 频率 = 66/66 = 1M

#if 0
    // 配置比较通道1
    GPT1->OCR[0] = 1000000 / 2; // 设置中断周期为500 ms
    // 打开 GPT1 的输出比较通道中断
    GPT1->IR = 1 << 0;
    // 使能 GIC
    GIC_EnableIRQ(GPT1_IRQn);
    // 注册中断服务函数
    system_register_irqhandler(GPT1_IRQn, gpt1_irqhandler, NULL);
#endif

    GPT1->CR |= 1 << 0; // 打开gpt1

}
// GPT1 中断服务函数
void gpt1_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned char state = 0;
    // 先判断一下是不是比较中断通道1的中断
    if (GPT1->SR & (1 << 0))
    {
        state = !state;
        led_switch(state);
    }

    // 清除中断标志位
    GPT1->SR |= 1 << 0;
}

/*短延时*/
void delay_short(volatile unsigned int n)
{
    while (n--)
    {
    }
}
/*延时，一次循环大概是1ms n 是延时的毫秒数，在主频396MHZ*/
void delay(volatile unsigned int n)
{
    while (n--)
    {
        delay_short(0x7ff);
    }
}

// 不使用中断，而是借助GPT的CNT 寄存器的值进行精确判断
void delay_us(unsigned int usdelay)
{
    unsigned long oldcnt, newcnt;
    unsigned long tcntvalue = 0;
    oldcnt = GPT1->CNT;
    while (1)
    {
        newcnt = GPT1->CNT;
        if (newcnt != oldcnt)
        {
            if (newcnt > oldcnt)
                tcntvalue += newcnt - oldcnt;
            else
                tcntvalue += 0xffffffff + newcnt - oldcnt;
            oldcnt = newcnt;
            if (tcntvalue >= usdelay)
                break;
        }
    }
}

//毫秒延时
void delay_ms(unsigned int msdelay)
{
    int i = 0;
    for(;i<msdelay;i++)
    {
        delay_us(1000);
    }

}