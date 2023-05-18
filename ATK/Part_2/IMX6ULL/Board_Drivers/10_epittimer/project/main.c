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
    //exit_init();
    epit_init(0,66000000/2);//分频 66MHz 得到 周期为500ms

    while(1)
    {
    }

    return 0;
}