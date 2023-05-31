#include "bsp_lcd.h"
#include "bsp_gpio.h"
#include "printf.h"
#include "bsp_delay.h"


// LCD 的初始化
void lcd_init()
{
    unsigned int lcdid = 0;
    lcdid = lcd_read_panelid();
    printf("lcdid = %#x \r\n", lcdid);
    lcdgpio_init();
    lcd_reset();
    printf("reset finish\r\n");
    delay_ms(10);

    lcd_noreset();
    printf("no reset finish\r\n");
    /* TFTLCD参数结构体初始化 */
    if (lcdid == ATK4342)
    {
        tftlcd_dev.height = 272;
        tftlcd_dev.width = 480;
        tftlcd_dev.vspw = 1;
        tftlcd_dev.vbpd = 8;
        tftlcd_dev.vfpd = 8;
        tftlcd_dev.hspw = 1;
        tftlcd_dev.hbpd = 40;
        tftlcd_dev.hfpd = 5;
        lcdclk_init(27, 8, 8); /* 初始化LCD时钟 10.1MHz */
    }
    else if (lcdid == ATK4384)
    {
        tftlcd_dev.height = 480;
        tftlcd_dev.width = 800;
        tftlcd_dev.vspw = 3;
        tftlcd_dev.vbpd = 32;
        tftlcd_dev.vfpd = 13;
        tftlcd_dev.hspw = 48;
        tftlcd_dev.hbpd = 88;
        tftlcd_dev.hfpd = 40;
        lcdclk_init(42, 4, 8); /* 初始化LCD时钟 31.5MHz */
        printf("select 4384\r\n");
    }
    else if (lcdid == ATK7084)
    {
        tftlcd_dev.height = 480;
        tftlcd_dev.width = 800;
        tftlcd_dev.vspw = 1;
        tftlcd_dev.vbpd = 23;
        tftlcd_dev.vfpd = 22;
        tftlcd_dev.hspw = 1;
        tftlcd_dev.hbpd = 46;
        tftlcd_dev.hfpd = 210;
        lcdclk_init(30, 3, 7); /* 初始化LCD时钟 34.2MHz */
    }
    else if (lcdid == ATK7016)
    {
        tftlcd_dev.height = 600;
        tftlcd_dev.width = 1024;
        tftlcd_dev.vspw = 3;
        tftlcd_dev.vbpd = 20;
        tftlcd_dev.vfpd = 12;
        tftlcd_dev.hspw = 20;
        tftlcd_dev.hbpd = 140;
        tftlcd_dev.hfpd = 160;
        lcdclk_init(32, 3, 5); /* 初始化LCD时钟 51.2MHz */
    }
    else if (lcdid == ATK1018)
    {
        tftlcd_dev.height = 800;
        tftlcd_dev.width = 1280;
        tftlcd_dev.vspw = 3;
        tftlcd_dev.vbpd = 10;
        tftlcd_dev.vfpd = 10;
        tftlcd_dev.hspw = 10;
        tftlcd_dev.hbpd = 80;
        tftlcd_dev.hfpd = 70;
        lcdclk_init(35, 3, 5); /* 初始化LCD时钟 56MHz */
    }
    tftlcd_dev.pixsize = 4; /* ARGB8888模式，每个像素4字节 */
    tftlcd_dev.framebuffer = LCD_FRAMEBUF_ADDR;
    tftlcd_dev.backcolor = LCD_WHITE; /* 背景色为白色 */
    tftlcd_dev.forecolor = LCD_BLACK; /* 前景色为黑色 */

    LCDIF->CTRL = 0;
    LCDIF->CTRL |= (1 << 19) | (1 << 17) | (0 << 14) | (0 << 12) |
                   (3 << 10) | (3 << 8) | (1 << 5) | (0 << 1);
    LCDIF->CTRL1 = 0;                   
    LCDIF->CTRL1 |= 7 << 16;
    LCDIF->TRANSFER_COUNT = (tftlcd_dev.height << 16) | (tftlcd_dev.width << 0);
    LCDIF->VDCTRL0 = 0; // 先清零
    LCDIF->VDCTRL0 = (0 << 29) | (1 << 28) | (0 << 27) |
                     (0 << 26) | (0 << 25) | (1 << 24) |
                     (1 << 21) | (1 << 20) | (tftlcd_dev.vspw << 0);

    /*
     * 初始化ELCDIF的VDCTRL1寄存器
     * 设置VSYNC总周期
     */
    LCDIF->VDCTRL1 = tftlcd_dev.height + tftlcd_dev.vspw + tftlcd_dev.vfpd + tftlcd_dev.vbpd; // VSYNC周期

    /*
     * 初始化ELCDIF的VDCTRL2寄存器
     * 设置HSYNC周期
     * bit[31:18] ：hsw
     * bit[17:0]  : HSYNC总周期
     */
    LCDIF->VDCTRL2 = (tftlcd_dev.hspw << 18) | (tftlcd_dev.width + tftlcd_dev.hspw + tftlcd_dev.hfpd + tftlcd_dev.hbpd);

    /*
     * 初始化ELCDIF的VDCTRL3寄存器
     * 设置HSYNC周期
     * bit[27:16] ：水平等待时钟数
     * bit[15:0]  : 垂直等待时钟数
     */
    LCDIF->VDCTRL3 = ((tftlcd_dev.hbpd + tftlcd_dev.hspw) << 16) | (tftlcd_dev.vbpd + tftlcd_dev.vspw);

    /*
     * 初始化ELCDIF的VDCTRL4寄存器
     * 设置HSYNC周期
     * bit[18] 1 : 当使用VSHYNC、HSYNC、DOTCLK的话此为置1
     * bit[17:0]  : 宽度
     */

    LCDIF->VDCTRL4 = (1 << 18) | (tftlcd_dev.width);

    /*
     * 初始化ELCDIF的CUR_BUF和NEXT_BUF寄存器
     * 设置当前显存地址和下一帧的显存地址
     */
    LCDIF->CUR_BUF = (unsigned int)tftlcd_dev.framebuffer;
    LCDIF->NEXT_BUF = (unsigned int)tftlcd_dev.framebuffer;

    lcd_enable(); /* 使能LCD 	*/
    delay_ms(10);
    lcd_clear(LCD_WHITE); /* 清屏 		*/
    printf("清屏完毕\r\n");



}

/*
 * 读取屏幕ID，
 * 描述：LCD_DATA23=R7(M0);LCD_DATA15=G7(M1);LCD_DATA07=B7(M2);
 * 		M2:M1:M0
 *		0 :0 :0	//4.3寸480*272 RGB屏,ID=0X4342
 *		0 :0 :1	//7寸800*480 RGB屏,ID=0X7084
 *	 	0 :1 :0	//7寸1024*600 RGB屏,ID=0X7016
 *  	1 :0 :1	//10.1寸1280*800,RGB屏,ID=0X1018
 *		1 :0 :0	//4.3寸800*480 RGB屏,ID=0X4384
 * @param 		: 无
 * @return 		: 屏幕ID
 */
// 读取屏幕 id （使用正点原子才需要）
unsigned short lcd_read_panelid()
{
    unsigned char idx = 0;
    // 打开模拟开关
    //  配置io结构体
    gpio_pin_config_t lcdio_config;
    lcdio_config.direction = kGPIO_DigitalOutput;
    lcdio_config.outputLogic = 1;

    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0x10b0);

    // GPIO 的初始化
    gpio_init(GPIO3, 3, &lcdio_config);
    // 读取 id
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12, 0);         // B7
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20, 0);         // G7
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28, 0);         // R7
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12, 0xf080); // B7
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20, 0xf080); // G7
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28, 0xf080); // R7
    // GPIO 的初始化
    lcdio_config.direction = kGPIO_DigitalInput;
    gpio_init(GPIO3, 12, &lcdio_config);
    gpio_init(GPIO3, 20, &lcdio_config);
    gpio_init(GPIO3, 28, &lcdio_config);

    idx = (unsigned char)gpio_pinread(GPIO3, 28);
    idx |= (unsigned char)gpio_pinread(GPIO3, 20) << 1;
    idx |= (unsigned char)gpio_pinread(GPIO3, 12) << 2;

    switch (idx)
    {
    case 0:
        return ATK4342;
    case 1:
        return ATK7084;
    case 2:
        return ATK7016;
    case 3:
        return ATK1018;
    case 4:
        return ATK4384;
    }
    return 0;
}
// lcd io的初始化
void lcdgpio_init()
{
    // io 复用
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA00_LCDIF_DATA00, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA01_LCDIF_DATA01, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA02_LCDIF_DATA02, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA03_LCDIF_DATA03, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA04_LCDIF_DATA04, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA05_LCDIF_DATA05, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA06_LCDIF_DATA06, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_LCDIF_DATA07, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA08_LCDIF_DATA08, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA09_LCDIF_DATA09, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA10_LCDIF_DATA10, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA11_LCDIF_DATA11, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA12_LCDIF_DATA12, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA13_LCDIF_DATA13, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA14_LCDIF_DATA14, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_LCDIF_DATA15, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA16_LCDIF_DATA16, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA17_LCDIF_DATA17, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA18_LCDIF_DATA18, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA19_LCDIF_DATA19, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA20_LCDIF_DATA20, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA21_LCDIF_DATA21, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA22_LCDIF_DATA22, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_LCDIF_DATA23, 0);

    IOMUXC_SetPinMux(IOMUXC_LCD_CLK_LCDIF_CLK, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_ENABLE_LCDIF_ENABLE, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_HSYNC_LCDIF_HSYNC, 0);
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_LCDIF_VSYNC, 0);

    // io电气属性
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA00_LCDIF_DATA00, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA01_LCDIF_DATA01, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA02_LCDIF_DATA02, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA03_LCDIF_DATA03, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA04_LCDIF_DATA04, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA05_LCDIF_DATA05, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA06_LCDIF_DATA06, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_LCDIF_DATA07, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA08_LCDIF_DATA08, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA09_LCDIF_DATA09, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA10_LCDIF_DATA10, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA11_LCDIF_DATA11, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA12_LCDIF_DATA12, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA13_LCDIF_DATA13, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA14_LCDIF_DATA14, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_LCDIF_DATA15, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA16_LCDIF_DATA16, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA17_LCDIF_DATA17, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA18_LCDIF_DATA18, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA19_LCDIF_DATA19, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA20_LCDIF_DATA20, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA21_LCDIF_DATA21, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA22_LCDIF_DATA22, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_LCDIF_DATA23, 0xB9);

    IOMUXC_SetPinConfig(IOMUXC_LCD_CLK_LCDIF_CLK, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_ENABLE_LCDIF_ENABLE, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_HSYNC_LCDIF_HSYNC, 0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_LCDIF_VSYNC, 0xB9);

    // 设置背光
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO08_GPIO1_IO08, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO08_GPIO1_IO08, 0x10b0);

    gpio_pin_config_t bl_config;
    bl_config.direction = kGPIO_DigitalOutput;
    bl_config.outputLogic = 1;
    // GPIO 的初始化
    gpio_init(GPIO1, 8, &bl_config);
    gpio_pinwrite(GPIO1, 8, 1);
}

// 复位lcd 控制器
void lcd_reset()
{
    LCDIF->CTRL |= 1 << 31;
}
// 停止复位
void lcd_noreset()
{
    LCDIF->CTRL &= ~(1 << 31);
}
// 使能 lcd 控制器
void lcd_enable()
{
    LCDIF->CTRL |= 1 << 0;
}

// 像素时钟初始化
void lcdclk_init(unsigned char loopDiv, unsigned char prediv, unsigned char div)
{
    /* 先初始化video pll
     * VIDEO PLL = OSC24M * (loopDivider + (denominator / numerator)) / postDivider
     *不使用小数分频器，因此denominator和numerator设置为0
     */
    CCM_ANALOG->PLL_VIDEO_NUM = 0; /* 不使用小数分频器 */
    CCM_ANALOG->PLL_VIDEO_DENOM = 0;

    /*
     * PLL_VIDEO寄存器设置
     * bit[13]:    1   使能VIDEO PLL时钟
     * bit[20:19]  2  设置postDivider为1分频
     * bit[6:0] : 32  设置loopDivider寄存器
     */
    CCM_ANALOG->PLL_VIDEO = (2 << 19) | (1 << 13) | (loopDiv << 0);

    /*
     * MISC2寄存器设置
     * bit[31:30]: 0  VIDEO的post-div设置，时钟源来源于postDivider，1分频
     */
    CCM_ANALOG->MISC2 &= ~(3 << 30);
    CCM_ANALOG->MISC2 = 0 << 30;

    /* LCD时钟源来源与PLL5，也就是VIDEO           PLL  */
    CCM->CSCDR2 &= ~(7 << 15);
    CCM->CSCDR2 |= (2 << 15); /* 设置LCDIF_PRE_CLK使用PLL5 */

    /* 设置LCDIF_PRE分频 */
    CCM->CSCDR2 &= ~(7 << 12);
    CCM->CSCDR2 |= (prediv - 1) << 12; /* 设置分频  */

    /* 设置LCDIF分频 */
    CCM->CBCMR &= ~(7 << 23);
    CCM->CBCMR |= (div - 1) << 23;

    /* 设置LCD时钟源为LCDIF_PRE时钟 */
    CCM->CSCDR2 &= ~(7 << 9); /* 清除原来的设置		 	*/
    CCM->CSCDR2 |= (0 << 9);  /* LCDIF_PRE时钟源选择LCDIF_PRE时钟 */
}
// 清屏
void lcd_clear(unsigned int color)
{
    unsigned int num;
    unsigned int i = 0;

    unsigned int *startaddr = (unsigned int *)tftlcd_dev.framebuffer; // 指向帧缓存首地址
    num = (unsigned int)tftlcd_dev.width * tftlcd_dev.height;         // 缓冲区总长度
    for (i = 0; i < num; i++)
    {
        startaddr[i] = color;
    }
}

inline void lcd_drawpoint(unsigned short x, unsigned short y, unsigned int color)
{
    *(unsigned int *)((unsigned int)tftlcd_dev.framebuffer +
                      tftlcd_dev.pixsize * (tftlcd_dev.width * y + x)) = color;
}

inline unsigned int lcd_readpoint(unsigned short x, unsigned short y)
{
    return *(unsigned int *)((unsigned int)tftlcd_dev.framebuffer +
                             tftlcd_dev.pixsize * (tftlcd_dev.width * y + x));
}

void lcd_fill(unsigned short x0, unsigned short y0,
              unsigned short x1, unsigned short y1, unsigned int color)
{
    unsigned short x, y;

    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 >= tftlcd_dev.width)
        x1 = tftlcd_dev.width - 1;
    if (y1 >= tftlcd_dev.height)
        y1 = tftlcd_dev.height - 1;

    for (y = y0; y <= y1; y++)
    {
        for (x = x0; x <= x1; x++)
            lcd_drawpoint(x, y, color);
    }
}
