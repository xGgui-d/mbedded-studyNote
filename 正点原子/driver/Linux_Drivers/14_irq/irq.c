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

#define IRQ_NAME "irq"
#define IRQ_CNT 1

#define KEY0VALUE 0xF0
#define INVAKEY 0x00

#define KEY_NUM 1

// key 结构体
struct irq_keydesc
{
    int gpio;                                      // io 编号
    int irqnum;                                    // 终端号
    unsigned char value;                           // 键值
    char name[10];                                 // 名字
    irqreturn_t (*handler)(int irq, void *dev_id); // 中断处理函数
    struct tasklet_struct tasklet;
};

// irq设备结构体
struct irq_dev
{
    dev_t devid;
    int major;
    int minor;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int dev_status;    // 设备状态 0 表示设备可以使用，1表示不可以使用
    struct mutex lock; // 互斥锁操作
    struct irq_keydesc irqkey[KEY_NUM];
    struct timer_list timer; // 定时器结构体

    atomic_t keyvalue;
    atomic_t releasekey;
};
struct irq_dev irq;

static ssize_t irq_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{

    return 0;
}
// 从设备读取数据
static ssize_t irq_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    struct irq_dev *dev = (struct irq_dev *)filp->private_data;
    unsigned char keyvalue = atomic_read(&dev->keyvalue);
    unsigned char releasekey = atomic_read(&dev->releasekey);

    if (releasekey == 1)
    {
        // 将值上报给应用程序
        ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
        atomic_set(&dev->releasekey, 0);
    }
    else
        return -1;

    // 这个返回值在 app 中通过read 函数返回出去
    return 0;
}

static int irq_open(struct inode *inode, struct file *filp)
{

    filp->private_data = &irq;
    printk("open success!!!\r\n");
    mutex_lock(&irq.lock);
    return 0;
}
static int irq_release(struct inode *indoe, struct file *filp)
{
    mutex_unlock(&irq.lock);

    return 0;
}
// 操作集
static const struct file_operations irq_fops = {

    .owner = THIS_MODULE,
    .read = irq_read,
    .write = irq_write,
    .open = irq_open,
    .release = irq_release

};

// 中断处理函数
static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct irq_dev *dev = dev_id;

// 按下按键，触发中断函数，开启定时 10 ms
#if 0 
    dev->timer.data = (unsigned long)dev;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
#endif
    // 调度tasklet
    tasklet_schedule(&dev->irqkey[0].tasklet);

    return IRQ_HANDLED;
}
// takslet 
static void key_tasklet(unsigned long data)
{
    struct irq_dev* dev = (struct irq_dev*) data;

    printk("tasklet use\r\n");
    dev->timer.data = (unsigned long)dev;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));
}

// 定时器处理函数
static void timer_func(unsigned long arg)
{
    struct irq_dev *dev = (struct irq_dev *)arg;
    // 定时器10ms 过后执行如下操作
    int value = 0;

    value = gpio_get_value(dev->irqkey[0].gpio);
    if (value == 0)
    {
        // 按下
        printk("KEY0 press\r\n");
        // 把当前按键的值赋值给设备
        atomic_set(&dev->keyvalue, dev->irqkey[0].value);
    }
    else if (value == 1)
    {
        // 释放
        printk("KEY0 release\r\n");
        // 完成一次按键释放
        atomic_set(&dev->releasekey, 1);
    }
}

static int keyio_init(struct irq_dev *dev)
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
    for (i = 0; i < KEY_NUM; i++)
    {
        // 初始化字符数组
        dev->irqkey[i].gpio = of_get_named_gpio(dev->nd, "key-gpios", i);
        if (dev->irqkey[i].gpio < 0)
        {
            printk("can't find key_gpio\r\n");
            ret = -EINVAL;
            goto fail_findnode;
        }
        else
        {
            printk("key gpio num = %d\r\n", irq.irqkey[i].gpio);
        }
    }

    /* 申请 IO */
    for (i = 0; i < KEY_NUM; i++)
    {
        memset(dev->irqkey[i].name, 0, sizeof dev->irqkey[i].name);
        sprintf(dev->irqkey[i].name, "KEY%d", i);
        ret = gpio_request(dev->irqkey[i].gpio, dev->irqkey[i].name);
        if (ret < 0)
        {
            printk("key gpio request fail\r\n");
            ret = -EINVAL;
            goto fail_findnode;
        }

        // 使用 io，设置 io 为输入 并且设置成中断模式
        ret = gpio_direction_input(dev->irqkey[i].gpio);
        if (ret < 0)
        {
            // printk("led gpio request fail");
            ret = -EINVAL;
            goto fail_setinput;
        }
        // 获取 io 的中断号
        dev->irqkey[i].irqnum = gpio_to_irq(dev->irqkey[i].gpio);
        // dev->irqkey[i].irqnum = irq_of_parse_and_map(dev->nd, i);
    }

    dev->irqkey[0].handler = key0_handler;
    dev->irqkey[0].value = KEY0VALUE;
    /* 按键中断初始化 */
    for (i = 0; i < KEY_NUM; i++)
    {
        ret = request_irq(dev->irqkey[i].irqnum,
                          dev->irqkey[i].handler,
                          IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                          dev->irqkey[i].name, &irq);

        if (ret)
        {
            printk("irq %d request fail", dev->irqkey[i].irqnum);
            goto fail_irq;
        }

        tasklet_init(&dev->irqkey[i].tasklet, key_tasklet, (unsigned long)dev);
    }

    // 初始化原子变量
    atomic_set(&irq.keyvalue, INVAKEY);
    atomic_set(&irq.releasekey, 0);

    return 0;

fail_irq:
    for (i = 0; i < KEY_NUM; i++)
        gpio_free(dev->irqkey[i].gpio);
fail_setinput:
fail_findnode:
    return ret;
}

static int __init irq_dev_init(void)
{

    int ret = 0;

    // 初始化互斥锁
    mutex_init(&irq.lock);

    // 注册字符设备号
    irq.major = 0;
    if (irq.major)
    {
        // 给定设备号
        irq.devid = MAJOR(irq.major);
        ret = register_chrdev_region(irq.devid, IRQ_CNT, IRQ_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&irq.devid, 0, IRQ_CNT, IRQ_NAME);
        irq.major = MAJOR(irq.devid);
        irq.minor = MINOR(irq.devid);
    }
    printk("key major = %d, minor = %d \r\n", irq.major, irq.minor);

    // 初始化 cdev
    irq.cdev.owner = THIS_MODULE;
    cdev_init(&irq.cdev, &irq_fops);
    cdev_add(&irq.cdev, irq.devid, IRQ_CNT);

    // 创建类
    irq.class = class_create(THIS_MODULE, IRQ_NAME);
    if (IS_ERR(irq.class))
    {
        return PTR_ERR(irq.class);
    }
    // 创建设备
    irq.device = device_create(irq.class, NULL, irq.devid, NULL, IRQ_NAME);
    if (IS_ERR(irq.device))
    {
        return PTR_ERR(irq.device);
    }
    // 初始化 io
    ret = keyio_init(&irq);
    if (ret < 0)
    {
        goto fail_keyinit;
    }
    // 初始化定时器
    init_timer(&irq.timer);
    irq.timer.function = timer_func;

    return 0;

fail_keyinit:
    return ret;
}

static void __exit irq_dev_exit(void)
{
    int i = 0;

    // 释放 io 和中断
    for (i = 0; i < KEY_NUM; i++)
        free_irq(irq.irqkey[i].irqnum, &irq);
    for (i = 0; i < KEY_NUM; i++)
        gpio_free(irq.irqkey[i].gpio);
    // 删除定时器
    del_timer(&irq.timer);
    // 注销字符设备驱动
    cdev_del(&irq.cdev);
    unregister_chrdev_region(irq.devid, IRQ_CNT);
    device_destroy(irq.class, irq.devid);
    class_destroy(irq.class);
}

// 模块入口和出口
module_init(irq_dev_init);
module_exit(irq_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");