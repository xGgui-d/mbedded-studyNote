#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#define KEYINPUT_NAME "keyinput"
#define KEYINPUT_CNT 1

/* key 结构体 */
struct irq_keydesc
{
    int gpio;                                      // io 编号
    int irqnum;                                    // 终端号
    char name[10];                                 // 名字
    irqreturn_t (*handler)(int irq, void *dev_id); // 中断处理函数
    struct tasklet_struct tasklet;
};

/* keyinput 设备结构体 */
struct keyinput_dev_t
{
    struct device_node *nd;
    int dev_status;            // 设备状态 0 表示设备可以使用，1表示不可以使用
    struct mutex lock;         // 互斥锁操作
    struct irq_keydesc irqkey; // key 结构体
    struct timer_list timer;   // 定时器结构体

    struct input_dev *inputdev; // 输入设备结构体
};
struct keyinput_dev_t keyinput_dev;

/* 中断处理函数 */
static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct keyinput_dev_t *dev = dev_id;

// 按下按键，触发中断函数，开启定时 10 ms
#if 0 
    dev->timer.data = (unsigned long)dev;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
#endif
    // 调度 tasklet
    tasklet_schedule(&dev->irqkey.tasklet);
    // 退出中断
    return IRQ_HANDLED;
}

/* 执行 takslet */
static void key_tasklet(unsigned long data)
{
    struct keyinput_dev_t *dev = (struct keyinput_dev_t *)data;

    // printk("tasklet use\r\n");
    // 开启定时器
    dev->timer.data = (unsigned long)dev;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
}

/* 定时器结束，执行定时器处理函数 */
static void timer_func(unsigned long arg)
{
    struct keyinput_dev_t *dev = (struct keyinput_dev_t *)arg;
    // 定时器10ms 过后执行如下操作
    int value = 0;

    value = gpio_get_value(dev->irqkey.gpio);
    if (value == 0)
    {
        // 按下 上报按键值
        input_event(keyinput_dev.inputdev, EV_KEY, KEY_0, 1);
        input_sync(keyinput_dev.inputdev);
    }
    else if (value == 1)
    {
        // 释放 上报按键值
        input_event(keyinput_dev.inputdev, EV_KEY, KEY_0, 0);
        input_sync(keyinput_dev.inputdev);
    }
}

/* 初始化按键io */
static int keyio_init(struct keyinput_dev_t *dev)
{

    int ret = 0;
    int i = 0;

    /* 按键初始化 */
    /* 获取设备节点 */
    dev->nd = of_find_node_by_path("/key");
    if (dev->nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnode;
    }
    /* 获取 key 所对应的gpio(将设备树节点的gpio信息转化会gpio号)*/
    // 初始化字符数组
    dev->irqkey.gpio = of_get_named_gpio(dev->nd, "key-gpios", i);
    if (dev->irqkey.gpio < 0)
    {
        printk("can't find key_gpio\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }
    else
    {
        printk("key gpio num = %d\r\n", keyinput_dev.irqkey.gpio);
    }

    /* 申请 IO */
    memset(dev->irqkey.name, 0, sizeof dev->irqkey.name);
    sprintf(dev->irqkey.name, "KEY%d", i);
    ret = gpio_request(dev->irqkey.gpio, dev->irqkey.name);
    if (ret < 0)
    {
        printk("key gpio request fail\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }

    // 使用 io，设置 io 为输入 并且设置成中断模式
    ret = gpio_direction_input(dev->irqkey.gpio);
    if (ret < 0)
    {
        // printk("led gpio request fail");
        ret = -EINVAL;
        goto fail_setinput;
    }
    // 获取 io 的中断号
    dev->irqkey.irqnum = gpio_to_irq(dev->irqkey.gpio);
    // dev->irqkey[i].irqnum = irq_of_parse_and_map(dev->nd, i);

    dev->irqkey.handler = key0_handler;

    /* 按键中断初始化 */
    ret = request_irq(dev->irqkey.irqnum,
                      dev->irqkey.handler,
                      IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                      dev->irqkey.name, &keyinput_dev);
    if (ret)
    {
        printk("irq %d request fail", dev->irqkey.irqnum);
        goto fail_irq;
    }

    tasklet_init(&dev->irqkey.tasklet, key_tasklet, (unsigned long)dev);

    return 0;

fail_irq:
    gpio_free(dev->irqkey.gpio);
fail_setinput:
fail_findnode:
    return ret;
}

static int __init keyinput_dev_init(void)
{
    int ret = 0;

    /* 初始化互斥锁 */
    mutex_init(&keyinput_dev.lock);

    /* 初始化 io */
    ret = keyio_init(&keyinput_dev);
    if (ret < 0)
    {
        goto fail_keyio_init;
    }

    /* 初始化定时器 */
    init_timer(&keyinput_dev.timer);
    keyinput_dev.timer.function = timer_func;

    /* 注册 input 设备结构体 */
    keyinput_dev.inputdev = input_allocate_device(); // 申请 device
    if (keyinput_dev.inputdev == NULL)
    {
        ret = -EINVAL;
        goto fail_allo_input;
    }
    /* 配置 input 设备结构体 */
    keyinput_dev.inputdev->name = KEYINPUT_NAME;     // 设备节点文件名字
    __set_bit(EV_KEY, keyinput_dev.inputdev->evbit); // 按键事件
    __set_bit(EV_REP, keyinput_dev.inputdev->evbit); // 重复事件
    __set_bit(KEY_0, keyinput_dev.inputdev->keybit); // 按键值

    /* 注册 input 设备 */
    ret = input_register_device(keyinput_dev.inputdev);
    if (ret)
    {
        goto fail_input_register;
    }
    return 0;

fail_input_register:
    input_free_device(keyinput_dev.inputdev);
fail_allo_input:
fail_keyio_init:
    return ret;
}

static void __exit keyinput_dev_exit(void)
{
    /* 释放 io 和中断 */
    free_irq(keyinput_dev.irqkey.irqnum, &keyinput_dev);
    gpio_free(keyinput_dev.irqkey.gpio);
    /* 删除定时器 */
    del_timer(&keyinput_dev.timer);
    /* 注销input_dev */
    input_unregister_device(keyinput_dev.inputdev);
    input_free_device(keyinput_dev.inputdev);
}

/* 模块入口和出口 */
module_init(keyinput_dev_init);
module_exit(keyinput_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");