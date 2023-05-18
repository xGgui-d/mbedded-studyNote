#include "main.h"

int main(void)
{
    static unsigned int state = 0;
    /*中断初始化函数*/
    int_init();
    imx6u_clkinit();
    clk_enable();
    led_init();
    beep_init();
    keyfilter_init();

    while(1)
    {
        delay(500);
        led_switch(state);
        state = !state;
    }

    return 0;
}