#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"

/*枚举类型和GPIO结构体*/
/*设置一个io的输入输出状态*/
typedef enum _gpio_pin_direction
{
    //U表示 unsigned 在嵌入式中很常见
    kGPIO_DigitalInput = 0U,
    kGPIO_DigitalOutput = 1U
}gpio_pin_direction_t;

typedef struct _gpio_pin_config
{
    gpio_pin_direction_t direction; //io方向
    uint8_t outputLogic; //输出电平
}gpio_pin_config_t;

/*初始化gpio*/
void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config);
/*控制GPIO高低电平*/
void gpio_pinwrite(GPIO_Type *base, int pin, int value);
/*读取GPIO的值*/
int gpio_pinread(GPIO_Type *base, int pin);



#endif