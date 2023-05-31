#include "main.h"

int main(void)
{
    int a, b;
    static unsigned int state = 0;
    /*中断初始化函数*/
    int_init();
    imx6u_clkinit();
    clk_enable();
    led_init();
    uart_init();

    while (1)
    {
        printf("输入两个整数，使用空格隔开:");
        scanf("%d %d", &a, &b);
        printf("\r\n 数据%d + %d = %d\r\n\r\n", a, b, a + b); /* 输出和 */

        state = !state;
        led_switch(state);
    }
    return 0;
}