#include "bsp_clk.h"
/*使能外设时钟*/
void clk_enable(void)
{
    CCM->CCGR0 = 0xFFFFFFFF;
    CCM->CCGR1 = 0xFFFFFFFF;
    CCM->CCGR2 = 0xFFFFFFFF;
    CCM->CCGR3 = 0xFFFFFFFF;
    CCM->CCGR4 = 0xFFFFFFFF;
    CCM->CCGR5 = 0xFFFFFFFF;
    CCM->CCGR6 = 0xFFFFFFFF;
}

/*初始化时钟*/
void imx6u_clkinit()
{
    unsigned int reg = 0;
    /*初始化6U的主频为528MHz*/

    /*先切换系统时钟为OSC晶振*/
    if (((CCM->CCSR >> 2) & 0x01) == 0) // 当前时钟使用 pll1_main_clk，也就是 PLL1
    {
        CCM->CCSR &= ~(1 << 8); //(第8位置0)设置 step_clk = osc_clk = 24M
        CCM->CCSR |= (1 << 2);  // 将 pll1_main_clk 转为 step_clk
    }

    /*设置PLL1=1056MHZ*/
    CCM_ANALOG->PLL_ARM = ((88 << 0) & 0x7f) | (1 << 13); // 设置DIEV_SEL = 88,使能 enable
    CCM->CACRR = 1;                                       // 设置2分频

    /*切换回时钟*/
    CCM->CCSR &= ~(1 << 2);

    /*设置PLL2的4路PFD*/
    reg = CCM_ANALOG->PFD_528;
    reg &= ~(0x3F3F3F3F); // 清空所有的PFDn_FRAC位
    reg |= (32 << 24);    // PFD3 = 297  528*18/297 = 32
    reg |= (24 << 16);    // PFD2 = 396  24
    reg |= (16 << 8);     // PFD1 = 594  16
    reg |= (27 << 0);     // PFD0 = 352 27
    CCM_ANALOG->PFD_528 = reg;

    /*设置PLL3的4路PFD*/
    reg = CCM_ANALOG->PFD_480;
    reg &= ~(0x3F3F3F3F); // 清空所有的PFDn_FRAC位
    reg |= (19 << 24);    // PFD3 = 454.7  480*18/454.7 = 19
    reg |= (17 << 16);    // PFD2 = 508.2  17
    reg |= (16 << 8);     // PFD1 = 540  16
    reg |= (12 << 0);     // PFD0 = 720 12
    CCM_ANALOG->PFD_480 = reg;

    /*设置 AHB_CLK_ROOT = 132MHz*/
    CCM->CBCMR &= ~(3 << 18);      // 将PRE_PERIPH_CLK_SEL 2位清零
    CCM->CBCMR |= 1 << 18;         // 将PRE_PERIPH_CLK_SEL 2位 设置为 0x01
    CCM->CBCDR &= ~(1 << 25);      // 将PERIPH_CLK_SEL 位清零
    while (CCM->CDHIPR & (1 << 5)) // 等待这个引脚不繁忙再退出函数
        ;
#if 0  
    //这里设置3分频回出问题，但是uboot默认设置了3分频       
    CCM->CBCDR &= ~(7 << 10);      // 将AHB_PODF清零 并置0
    CCM->CBCDR |= (2 << 10);       // 将AHB_PODF置为 010 3分频
    while (CCM->CDHIPR & (1 << 1)) // 等待这个引脚不繁忙再退出函数
        ;
#endif
    /*设置IPG_CLK_ROOT = 66MHz*/
    CCM->CBCDR &= ~(3 << 8); // 清零
    CCM->CBCDR |= (1 << 8);  // 设置 2分频

    /*设置 PERCLK_CLK_ROOT=66MHz*/
    CCM->CSCMR1 &= ~(1 << 6);
    CCM->CSCMR1 &= ~(0x3f << 0); //1分频
    
}