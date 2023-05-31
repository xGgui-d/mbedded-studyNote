#include "bsp_keyfilter.h"
#include "bsp_int.h"


/*初始化keyfilter*/
void keyfilter_init()
{
    /* 1、设置IO复用 */
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0); /* 复用为GPIO1_IO18 */
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xF080);

    /* 2、初始化GPIO为中断模式 */
    gpio_pin_config_t key_config;
    key_config.direction = kGPIO_DigitalInput;
    key_config.interruptMode = kGPIO_IntFallingEdge;
    key_config.outputLogic = 1;
    gpio_init(GPIO1, 18, &key_config);

    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);                                          /* 使能GIC中对应的中断 */
    system_register_irqhandler(GPIO1_Combined_16_31_IRQn, (system_irq_handler_t)gpio_16_31_irqhandle, NULL); /* 注册中断服务函数 */
    gpio_enable(GPIO1, 18);


    filterTimer_init(66000000/100);
}

/*按键中断服务函数*/
void gpio_key_irqhandle(unsigned int gicciar, void *param)
{
    // 开启定时器
    filtertimer_restart(66000000/100);//10ms
    //清除中断标志位
    gpio_clearintflags(GPIO1,18);
}

/*初始化EPIT定时器*/
void filterTimer_init(unsigned int value)
{
    // 配置 EPIT1_CR 寄存器
    // 先不使能
    EPIT1->CR = 0;
    // 开始配置
    EPIT1->CR = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 24);
    // 配置 EPIT1_LR 寄存器，相当于倒计数值
    EPIT1->LR = value;
    // 配置比较寄存器 EPIT_CMP
    EPIT1->CMPR = 0; // 计数到0触发中断

    // 初始化中断
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, (system_irq_handler_t)gpio_irqhandler, NULL);

}

/*关闭中断计时器*/
void filtertimer_stop()
{
    EPIT1->CR &= ~(1 << 0);
}
/*开启中断计时器*/
void filtertimer_start()
{
    EPIT1->CR |= 1 << 0;
}
/*重启 EPIT1 计时器*/
void filtertimer_restart(unsigned int value)
{
    // 先关闭计时器
    filtertimer_stop();
    EPIT1->LR = value;
    filtertimer_start();
}

/*EPIT1 定时器中断处理函数*/
void gpio_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned int state = 0;
    if (EPIT1->SR & (1 << 0)) /*中断确实发生了*/
    {
        // 关闭中断计时器
        filtertimer_stop();
        // 再次判断是否按下
        if (gpio_pinread(GPIO1, 18) == 0)
        {
            state = !state;
            beep_switch(state);
        }
    }

    // 清除中断标志位
    EPIT1->SR |= 1 << 0;
}