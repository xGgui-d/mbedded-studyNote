#include "bsp_gpio.h"
/*初始化 gpio*/
void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config)
{
    base->IMR &= ~(1U << pin);
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
    gpio_intconfig(base, pin, config->interruptMode);
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

/*使能指定GPIO中断*/
void gpio_enable(GPIO_Type *base, int pin)
{
    base->IMR |= (1 << pin);
}

void gpio_disable(GPIO_Type *base, int pin)
{
    base->IMR &= ~(1 << pin);
}

/*清除中断标志位*/
void gpio_clearintflags(GPIO_Type *base, int pin)
{
    base->ISR |= (1 << pin);
}

/*GPIO 中断初始化函数*/
void gpio_intconfig(GPIO_Type *base, int pin, gpio_interrupt_mode_t pin_int_mode)
{
    volatile uint32_t *icr;
    uint32_t icrShift;
    icrShift = pin;

    base->EDGE_SEL &= ~(1 << pin);
    if (pin < 16)
    {
        icr = &(base->ICR1);
    }
    else
    {
        icr = &(base->ICR2);
        icrShift -= 16;
    }

	switch(pin_int_mode)
	{
		case(kGPIO_IntLowLevel):
			*icr &= ~(3U << (2 * icrShift));
			break;
		case(kGPIO_IntHighLevel):
			*icr = (*icr & (~(3U << (2 * icrShift)))) | (1U << (2 * icrShift));
			break;
		case(kGPIO_IntRisingEdge):
			*icr = (*icr & (~(3U << (2 * icrShift)))) | (2U << (2 * icrShift));
			break;
		case(kGPIO_IntFallingEdge):
			*icr |= (3U << (2 * icrShift));
			break;
		case(kGPIO_IntRisingOrFallingEdge):
			base->EDGE_SEL |= (1U << pin);
			break;
		default:
			break;
	}
}