#include "bsp_led.h"
/*初始化外设 led */
void led_init(void)
{
    gpio_pin_config_t led_config;
    led_config.direction = kGPIO_DigitalOutput;
    led_config.outputLogic = 0;

    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0);         // 复用为gpio1_io03
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0); // 设置 gpio1_io03 电气属性

    /*GPIO 初始化*/
    gpio_init(GPIO1,3,&led_config);
}
/*打开 led 灯*/
void led_on(void)
{
    gpio_pinwrite(GPIO1,3,0); // 将 bit 3 清零
}
/*关闭 led 灯*/
void led_off(void)
{
    gpio_pinwrite(GPIO1,3,1); // 将 bit3 置1
}
