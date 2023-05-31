#include <stdio.h>
#include "fsl_iomuxc.h"
#include "fsl_common.h"
#include "MCIMX6Y2.h"

/*使能外设时钟*/
void clk_enable(void)
{
    CCM->CCGR0 = 0xFFFFFFFF;
    CCM->CCGR1 = 0xFFFFFFFF;
    CCM->CCGR2 = 0xFFFFFFFF;
    CCM->CCGR3 = 0xFFFFFFFF;
    CCM->CCGR4 = 0xFFFFFFFF;
    CCM->CCGR5 = 0xFFFFFFFF;
    CCM->CCGR6 = 0xFFFFFFFF;
}

/*初始化外设 led */
void led_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0);         // 复用为gpio1_io03
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0); // 设置 gpio1_io03 电气属性
    /*GPIO 初始化*/
    GPIO1->GDIR = 0x08; // 设置为输出
    GPIO1->DR = 0x0;    // 打开led灯
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
/*打开 led 灯*/
void led_on(void)
{
    GPIO1->DR &= ~(1 << 3); // 将 bit 3 清零
}
/*关闭 led 灯*/
void led_off(void)
{
    GPIO1->DR |= (1 << 3); // 将 bit3 置1
}
int main(void)
{
    clk_enable();
    /*初始化 led*/
    led_init();
    /*设置 led 闪烁*/
    while (1)
    {
        led_on();
        delay(500);
        led_off();
        delay(500);
    }
    return 0;
}