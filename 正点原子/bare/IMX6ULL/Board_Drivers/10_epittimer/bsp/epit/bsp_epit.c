#include "bsp_epit.h"
#include "bsp_int.h"
#include "bsp_led.h"

/*初始化 epit 参数，分频值，比较值*/
void epit_init(unsigned int frac, unsigned int value)
{
    // 判断分频值是否越界
    if (frac > 4095)
        frac = 4095;
    // 配置 EPIT1_CR 寄存器
    // 先不使能
    EPIT1->CR = 0;
    // 开始配置
    EPIT1->CR = (1 << 1) | (1 << 2) | (1 << 3) | (frac << 4) | (1 << 24);
    // 配置 EPIT1_LR 寄存器，相当于倒计数值
    EPIT1->LR = value;
    // 配置比较寄存器 EPIT_CMP
    EPIT1->CMPR = 0; // 计数到0触发中断

    // 初始化中断
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, epit1_irqhandler, NULL);

    // 使能 EPIT1
    EPIT1->CR |= (1 << 0);
}

/*EPIT中断服务函数*/
void epit1_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned char state = 0;
    state = !state;
    if (EPIT1->SR & (1 << 0)) /*中断确实发生了*/
    {
        led_switch(state);
    }
    // 清除中断位(写1清除)
    EPIT1->SR |= (1 << 0);
}
