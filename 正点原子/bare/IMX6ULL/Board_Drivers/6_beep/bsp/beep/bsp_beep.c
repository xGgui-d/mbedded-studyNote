#include "bsp_beep.h"
/*beep 初始化*/
void beep_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0); // 复用位 GPIO5_IO1
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0x10B0);
    // GPIO 的初始化
    GPIO5->GDIR |= (1 << 1); // 设置为输出
    GPIO5->DR &= ~(1 << 1);  // 将 bit1 清零(默认打开)
}
/*蜂鸣器控制函数*/
void beep_switch(int status)
{
    if (status == ON)
        GPIO5->DR &= ~(1 << 1); // 将 beep 打开
    else if (status == OFF)
        GPIO5->DR |= (1 << 1);
}