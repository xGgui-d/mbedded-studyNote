#ifndef __BSP_KEY_H
#define __BSP_KEY_H
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"

#include "bsp_delay.h"
#include "bsp_gpio.h"

void key_init();
int read_key(void);
int key_getvalue(void);

#endif