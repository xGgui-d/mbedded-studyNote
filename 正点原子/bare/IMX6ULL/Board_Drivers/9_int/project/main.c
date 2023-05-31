#include "main.h"

int main(void)
{
    
    /*中断初始化函数*/
    int_init();
    imx6u_clkinit();
    clk_enable();
    led_init();
    //key_init();
    beep_init();
    exit_init();

    while(1)
    {
        delay(300);
        led_off();
        delay(300);
        led_on();
    }

    return 0;
}