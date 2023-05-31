
#include "bsp_key.h"

/*初始化按键*/
void key_init()
{
    // 配置io结构体
    gpio_pin_config_t key_config;
    key_config.direction = kGPIO_DigitalInput;
    key_config.outputLogic = 1;

    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0); // 复用为 GPIO1_IO18
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xf080);

    // GPIO 的初始化
    gpio_init(GPIO1, 18, &key_config);
}
/*读取按键值*/
int read_key(void)
{
    return gpio_pinread(GPIO1, 18); // 读取DR寄存器的第18位的值
}

int key_getvalue(void)
{

    if (read_key() == 0) // 按键按下
    {
        delay(10);
        if (read_key() == 0) // 如果延时10ms后还是0 则按键有效
        {
            return 1;
        }
    }
    else if (read_key() == 1) // 没有按下
    {
        return 0;
    }
    return 0;
}