#include "main.h"

int main(void)
{
    clk_enable();
    /*初始化 led*/
    led_init();
    /*设置 led 闪烁*/
    while (1)
    {
        led_on();
        delay(1000);
        led_off();
        delay(1000);
        led_on();
        delay(100);
        led_off();
        delay(100);
    }
    return 0;
}