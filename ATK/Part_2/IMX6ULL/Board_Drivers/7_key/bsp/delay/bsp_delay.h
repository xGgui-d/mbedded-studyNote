#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H
#include "fsl_iomuxc.h"
#include "fsl_common.h"
#include "MCIMX6Y2.h"

void delay_short(volatile unsigned int n);
void delay(volatile unsigned int n);


#endif