#include "bsp_beep.h"
/*beep 初始化*/
void beep_init(void)
{

    gpio_pin_config_t beep_config;
    beep_config.outputLogic = 1;
    beep_config.direction = kGPIO_DigitalOutput;

    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0); // 复用位 GPIO5_IO1
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0x10B0);
    // GPIO 的初始化
    gpio_init(GPIO5,1,&beep_config);
}


/*蜂鸣器控制函数*/
void beep_switch(int status)
{
    if (status == ON)
        gpio_pinwrite(GPIO5,1,0); // 将 beep 打开
    else if (status == OFF)
        gpio_pinwrite(GPIO5,1,1);
}