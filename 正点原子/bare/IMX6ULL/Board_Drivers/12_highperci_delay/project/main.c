#include "main.h"

int main(void)
{
    static unsigned int state = 0;
    /*中断初始化函数*/
    int_init();
    imx6u_clkinit();
    clk_enable();
    led_init();
    delay_init();    

    while(1)
    {
        delay_ms(200);
        led_switch(state);
        state =!state;
    }

    return 0;
}