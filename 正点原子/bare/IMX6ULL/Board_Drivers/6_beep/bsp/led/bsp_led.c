#include "bsp_led.h"
/*初始化外设 led */
void led_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0);         // 复用为gpio1_io03
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0); // 设置 gpio1_io03 电气属性
    /*GPIO 初始化*/
    GPIO1->GDIR = 0x08; // 设置为输出
    GPIO1->DR = 0x0;    // 打开led灯
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
