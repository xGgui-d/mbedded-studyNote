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

#include <linux/timer.h>
#include <linux/jiffies.h>

#define TIMER_NAME "timer"
#define TIMER_CNT 1


#define CLOSE_CMD _IO(0XEF, 1) // 关闭定时器
#define OPEN_CMD _IO(0xEF, 2) // 打开定时器
#define SETPERIOD_CMD _IOW(0xED, 3, int) // 设置周期


// timer 设备结构体
struct timer_dev
{
    dev_t devid;
    int major;
    int minor;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int led_gpio;      // io 对应的编号
    int dev_status;    // 设备状态 0 表示设备可以使用，1表示不可以使用
    struct mutex lock; // 互斥锁操作

    struct timer_list timer; // 定时器结构体
    int timeperiod; // 周期ms
};
struct timer_dev timer;

static ssize_t timer_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret = 0;
    return ret;
}
static ssize_t timer_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    return ret;
}

static int timer_open(struct inode *inode, struct file *filp)
{

    filp->private_data = &timer;
    printk("dev open success!!!\r\n");
    mutex_lock(&timer.lock);
    return 0;
}
static int timer_release(struct inode *indoe, struct file *filp)
{
    mutex_unlock(&timer.lock);

    return 0;
}

static long timer_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct timer_dev *dev =  (struct timer_dev *)filp->private_data;
	int timerperiod;
	unsigned long flags;
	
	switch (cmd) {
		case CLOSE_CMD:		/* 关闭定时器 */
			del_timer_sync(&dev->timer);
			break;
		case OPEN_CMD:		/* 打开定时器 */
			spin_lock_irqsave(&dev->lock, flags);
			timerperiod = dev->timeperiod;
			spin_unlock_irqrestore(&dev->lock, flags);
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(timerperiod));
			break;
		case SETPERIOD_CMD: /* 设置定时器周期 */
			spin_lock_irqsave(&dev->lock, flags);
			dev->timeperiod = arg;
			spin_unlock_irqrestore(&dev->lock, flags);
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(arg));
			break;
		default:
			break;
	}
	return 0;
}

// 操作集
static const struct file_operations timer_fops = {

    .owner = THIS_MODULE,
    .read = timer_read,
    .write = timer_write,
    .open = timer_open,
    .release = timer_release,
    .unlocked_ioctl = timer_unlocked_ioctl

};
// 定时器处理函数
static void timer_func(unsigned long arg)
{
    struct timer_dev *dev = (struct timer_dev *)arg;
    static int sta = 1;
    sta = !sta;
    gpio_set_value(dev->led_gpio, sta);

    // 再次开启定时器，从而实现周期循环
    mod_timer(&timer.timer,jiffies+ msecs_to_jiffies(timer.timeperiod));
}

static int __init timer_init(void)
{

    int ret = 0;

    // 初始化互斥锁
    mutex_init(&timer.lock);

    // 注册字符设备号
    timer.major = 0;
    if (timer.major)
    {
        // 给定设备号
        timer.devid = MAJOR(timer.major);
        ret = register_chrdev_region(timer.devid, TIMER_CNT, TIMER_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&timer.devid, 0, TIMER_CNT, TIMER_NAME);
        timer.major = MAJOR(timer.devid);
        timer.minor = MINOR(timer.devid);
    }
    printk("timer major = %d, minor = %d \r\n", timer.major, timer.minor);
    // 初始化 cdev
    timer.cdev.owner = THIS_MODULE;
    cdev_init(&timer.cdev, &timer_fops);
    cdev_add(&timer.cdev, timer.devid, TIMER_CNT);

    // 创建类
    timer.class = class_create(THIS_MODULE, TIMER_NAME);
    if (IS_ERR(timer.class))
    {
        return PTR_ERR(timer.class);
    }
    // 创建设备
    timer.device = device_create(timer.class, NULL, timer.devid, NULL, TIMER_NAME);
    if (IS_ERR(timer.device))
    {
        return PTR_ERR(timer.device);
    }

    // 获取设备节点
    timer.nd = of_find_node_by_path("/gpioled");
    if (timer.nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnode;
    }
    // 获取 led 所对应的gpio(将设备树节点的gpio信息转化会gpio号)
    timer.led_gpio = of_get_named_gpio(timer.nd, "led-gpios", 0);
    if (timer.led_gpio < 0)
    {
        printk("can't find led_gpio\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }
    else
    {
        printk("led gpio num = %d\r\n", timer.led_gpio);
    }

    // // 申请 IO
    // ret = gpio_request(gpioled.led_gpio, "led-gpio");
    // if (ret<0)
    // {
    //     printk("led gpio request fail\r\n");
    //     ret = -EINVAL;
    //     goto fail_findnode;
    // }

    // 使用 io 设置 io 为输出 默认值为1 关灯
    ret = gpio_direction_output(timer.led_gpio, 1);
    if (ret < 0)
    {
        // printk("led gpio request fail");
        ret = -EINVAL;
        goto fail_setoutput;
    }

    // 初始化定时器
    timer.timeperiod = 500;
    init_timer(&timer.timer);
    timer.timer.expires = jiffies + msecs_to_jiffies(timer.timeperiod); // 将500ms 转化为jiffies
    timer.timer.function = timer_func;
    timer.timer.data = (unsigned long)&timer; // 将 timer 设备结构体作为 timer_func 函数的参数
    add_timer(&timer.timer);                  // 添加定时器

    return 0;

fail_setoutput:
    gpio_free(timer.led_gpio);
fail_findnode:
    return ret;
}

static void __exit timer_exit(void)
{
    // 删除定时器
    del_timer(&timer.timer);
    // 关灯
    gpio_set_value(timer.led_gpio, 1);
    // 注销字符设备驱动
    cdev_del(&timer.cdev);
    unregister_chrdev_region(timer.devid, TIMER_CNT);
    device_destroy(timer.class, timer.devid);
    class_destroy(timer.class);
    // 释放 io
    gpio_free(timer.led_gpio);

    printk("dev release success !!!\r\n");
}

// 模块入口和出口
module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");