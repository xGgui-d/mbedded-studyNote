#include "main.h"

int main(void)
{
    clk_enable();
    led_init();
    key_init();
    beep_init();

    int keyvalue = 0;
    while(1)
    {
        keyvalue = key_getvalue();
        if(keyvalue == 0)
        {
            led_off();
            beep_switch(OFF);


        }else if(keyvalue == 1)
        {
            led_on();
            beep_switch(ON);
        }
        delay(10);
    }

    return 0;
}