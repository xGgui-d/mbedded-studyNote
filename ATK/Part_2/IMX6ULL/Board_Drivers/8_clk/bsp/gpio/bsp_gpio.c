#include "bsp_gpio.h"
/*初始化 gpio*/
void    gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config)
{
    if (config->direction == kGPIO_DigitalInput)
    {
        base->GDIR &= ~(1 << pin);
    }
    else if (config->direction == kGPIO_DigitalOutput)
    {
        base->GDIR |= (1 << pin);
        // 设置默认输出电平 
        gpio_pinwrite(base, pin, config->outputLogic);
    }
}
/*控制GPIO 高低电平*/
void gpio_pinwrite(GPIO_Type *base, int pin, int value)
{
    if (value == 0)
    {
        base->DR &= ~(1 << pin);
    }
    else if (value == 1)
    {
        base->DR |= (1 << pin);
    }
}
/*读取GPIO 的值*/
int gpio_pinread(GPIO_Type *base, int pin)
{
    return (((base->DR) >> pin) & 0x01U);
}
