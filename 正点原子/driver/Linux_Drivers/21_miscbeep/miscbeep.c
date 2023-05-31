#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#define MISCBEEP_NAME "miscbeep"
#define MISCBEEP_MINOR 144

#define BEEP_OFF 0
#define BEEP_ON 1

/* miscbeep 蜂鸣器设备结构体 */
struct beep_dev_t
{
    int beep_gpio;          // gpio 号
    struct device_node *nd; // 设备节点
};
static struct beep_dev_t beep_dev;

ssize_t beep_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    return 0;
}

ssize_t beep_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret;
    struct beep_dev_t *dev = (struct beep_dev_t *)filp->private_data;
    unsigned char databuf;
    ret = copy_from_user(&databuf, buf, count);
    if (ret < 0)
    {
        return -EINVAL;
    }
    if (databuf == BEEP_ON)
    {
        gpio_set_value(dev->beep_gpio, 0);
    }
    else if (databuf == BEEP_OFF)
    {
        gpio_set_value(dev->beep_gpio, 1);
    }

    return 0;
}

int beep_open(struct inode *node, struct file *filp)
{
    filp->private_data = &beep_dev; /* 设置私有数据 */
    return 0;
}

int beep_release(struct inode *mode, struct file *filp)
{
    return 0;
}

/* 字符设备的操作函数集 */
static struct file_operations miscbeep_fops = {

    .owner = THIS_MODULE,
    .open = beep_open,
    .write = beep_write,
    .read = beep_read,
    .release = beep_release,

};

/* miscdevice 设备结构体*/
static struct miscdevice misc_dev = {

    .minor = MISCBEEP_MINOR,
    .name = MISCBEEP_NAME,
    .fops = &miscbeep_fops

};

/* probe 函数 */
static int beep_probe(struct platform_device *dev)
{
    int ret = 0;
    /* 初始化 beep io */
    beep_dev.nd = dev->dev.of_node;                                       // 获取节点（不需要find by name）
    beep_dev.beep_gpio = of_get_named_gpio(beep_dev.nd, "beep-gpios", 0); // 获取 gpio 号
    if (beep_dev.beep_gpio < 0)
    {
        ret = -EINVAL;
        goto fail_findgpio;
    }
    ret = gpio_request(beep_dev.beep_gpio, "beep-gpio01"); // 申请 gpio
    if (ret < 0)
    {
        printk("can't request gpio\r\n");
        ret = -EINVAL;
        goto fail_requestgpio;
    }
    ret = gpio_direction_output(beep_dev.beep_gpio, 1); // 设置 io 为输出，默认高电平
    if (ret < 0)
    {
        printk("can't setout gpio\r\n");
        ret = -EINVAL;
        goto fail_setoutgpio;
    }

    /* misc 驱动注册 */
    ret = misc_register(&misc_dev);
    if (ret < 0)
    {
        goto fail_misc_register;
    }

    return 0;
fail_misc_register:
fail_setoutgpio:
fail_requestgpio:
fail_findgpio:

    return ret;
}

/* remove 函数*/
static int beep_remove(struct platform_device *dev)
{
    int ret = 0;
    gpio_set_value(beep_dev.beep_gpio, 1);
    /* 释放 gpio */
    gpio_free(beep_dev.beep_gpio);

    /* 注销 misc */
    ret = misc_deregister(&misc_dev);
    if (ret < 0)
    {
        printk("deregister fail\r\n");
    }
    return 0;
}

/* platform 匹配表 */
static const struct of_device_id beep_of_match[] = {
    {.compatible = "alientek,beep"}, // 与设备树的beep节点进行匹配
    {.compatible = "xxx"},
    {/* sentinel*/},
};

/* platform 结构体 */
static struct platform_driver miscbeep_driver = {
    .driver = {
        .name = "imx6ull-beep",
        .of_match_table = beep_of_match, /* 设备树匹配表 */
    },
    .remove = beep_remove,
    .probe = beep_probe,
};

/* 驱动模块入口函数 */
static int __init miscbeep_init(void)
{
    return platform_driver_register(&miscbeep_driver);
}

/* 驱动模块出口函数 */
static void __exit miscbeep_exit(void)
{
    platform_driver_unregister(&miscbeep_driver);
}

module_init(miscbeep_init);
module_exit(miscbeep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");
