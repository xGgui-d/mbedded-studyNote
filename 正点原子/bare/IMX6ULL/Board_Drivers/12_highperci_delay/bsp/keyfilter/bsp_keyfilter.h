#ifndef __BSP_KEYFILTER_H
#define __BSP_KEYFILTER_H
#include "imx6u.h"
#include "bsp_key.h"
#include "bsp_exit.h"
#include "bsp_beep.h"
/*EPIT1 定时器中断处理函数*/
void gpio_irqhandler(unsigned int gicciar, void *param);
/*按键中断服务函数*/
void gpio_key_irqhandle(unsigned int gicciar, void *param);
/*关闭中断计时器*/
void filtertimer_stop();
/*开启中断计时器*/
void filtertimer_start();
/*重启 EPIT1 计时器*/
void filtertimer_restart(unsigned int value);
/*按键过滤程序的初始化*/
void keyfilter_init();
/*初始化EPIT定时器*/
void filterTimer_init(unsigned int value);

#endif 