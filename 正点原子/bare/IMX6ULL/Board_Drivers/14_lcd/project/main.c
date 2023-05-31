#include "main.h"

int main(void)
{
    static unsigned int state = 0;
    /*中断初始化函数*/
    int_init();
    imx6u_clkinit();
    clk_enable();
    led_init();
    uart_init();
    delay_init();
    lcd_init();

    while (1)
    {
        state = !state;
        led_switch(state);
        delay(300);
        lcd_fill(0, 0, 800, 480, LCD_ORANGE);
        tftlcd_dev.backcolor = LCD_ORANGE;
        tftlcd_dev.forecolor = LCD_WHITE;
        lcd_show_string(350, 230, 200, 32, 32, (char *)"PLEASE");
        delay(100);
        lcd_fill(0, 0, 800, 480, LCD_GREEN);
        tftlcd_dev.backcolor = LCD_GREEN;
        tftlcd_dev.forecolor = LCD_WHITE;        
        lcd_show_string(350, 230, 200, 32, 32, (char *)"READY");
        delay(100);
        lcd_fill(0, 0, 800, 480, LCD_RED);
        tftlcd_dev.backcolor = LCD_RED;
        tftlcd_dev.forecolor = LCD_WHITE;        
        lcd_show_string(350, 230, 200, 32, 32, (char *)"LET");
        delay(100);
        lcd_fill(0, 0, 800, 480, LCD_BLUE);
        tftlcd_dev.backcolor = LCD_BLUE;
        tftlcd_dev.forecolor = LCD_WHITE;        
        lcd_show_string(350, 230, 200, 32, 32, (char *)"GO!!!");
        delay(100);
    }
    return 0;
}