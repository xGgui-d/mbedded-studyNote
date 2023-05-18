#include "main.h"

int main(void)
{
    imx6u_clkinit();
    clk_enable();
    led_init();
    while(1)
    {
        delay(1000);
        led_off();
        delay(1000);
        led_on();
    }

    return 0;
}