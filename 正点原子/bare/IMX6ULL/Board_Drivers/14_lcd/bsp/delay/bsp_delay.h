#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H
#include "fsl_iomuxc.h"
#include "fsl_common.h"
#include "MCIMX6Y2.h"
/*延时初始化函数*/
void delay_init();
void delay_short(volatile unsigned int n);
void delay(volatile unsigned int n);
//GPT1 中断服务函数
void gpt1_irqhandler(unsigned int gicciar, void*param);
//毫秒延时
void delay_ms(unsigned int msdelay);
// 不使用中断，而是借助GPT的CNT 寄存器的值进行精确判断
void delay_us(unsigned int usdelay);
#endif