#include "bsp_int.h"

static unsigned int irqNesting;

/*中断处理函数表*/
static sys_irq_handle_t irqTable[NUMBER_OF_INT_VECTORS]; // 160个中断定义
/*初始化中断处理函数表*/
void system_irqtable_init(void)
{
    irqNesting = 0;
    unsigned int i = 0;
    for (i = 0; i < NUMBER_OF_INT_VECTORS; i++)
        system_register_irqhandler((IRQn_Type)i, default_irqhandler, NULL);
}

/*中断初始化函数*/
void int_init(void)
{
    GIC_Init();
    system_irqtable_init();
    /*中断向量偏移设置*/
    __set_VBAR(0x87800000);
}

/*具体的中断处理函数，如果触发了IRQ中断则会调用此函数，此函数的参数为中断id 也就是寄存器 r0 的值*/
void system_irqhandler(unsigned gicciar)
{

    uint32_t intNum = gicciar & 0x3ff; // 取 GICC_IAR 的前10 位
    if (intNum >= NUMBER_OF_INT_VECTORS)
        return;
    // 当前处理的中断个数+1
    irqNesting++;

    irqTable[intNum].irqHandler(intNum, irqTable[intNum].userParam);

    irqNesting--;
}

/*默认中断处理函数*/
void default_irqhandler(unsigned int gicciar, void *userParam)
{
    while (1)
    {
    }
}

/*注册中断处理函数*/
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam)
{
    irqTable[irq].irqHandler = handler;
    irqTable[irq].userParam = userParam;
}
