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
#define BEEP_NAME "beep"
#define BEEP_CNT 1

#define OFF 0
#define ON 1

// gpioled设备结构体
struct beep_dev
{
    dev_t devid;
    int major;
    int minor;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int beep_gpio; // io 对应的编号
};
struct beep_dev beep;

static ssize_t beep_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret;
    unsigned char databuf[1];
    ret = copy_from_user(databuf, buf, cnt);
    if (ret < 0)
    {
        return -EINVAL;
    }
    if (databuf[0] == ON)
    {
        gpio_set_value(beep.beep_gpio, 0);
    }
    else if (databuf[0] == OFF)
        gpio_set_value(beep.beep_gpio, 1);

    return 0;
}
static ssize_t beep_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    // struct dtsled_dev *dev = (struct dtsled_dev *)filp->private_data;
    return 0;
}

static int beep_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &beep;
    return 0;
}
static int beep_release(struct inode *indoe, struct file *filp)
{
    // struct dtsled_dev *dev = (struct dtsled_dev *)filp->private_data;
    return 0;
}
// 操作集
static const struct file_operations beep_fops = {

    .owner = THIS_MODULE,
    .read = beep_read,
    .write = beep_write,
    .open = beep_open,
    .release = beep_release

};

static int __init beep_init(void)
{

    int ret = 0;
    // 注册字符设备号
    beep.major = 0;
    if (beep.major)
    {
        // 给定设备号
        beep.devid = MAJOR(beep.major);
        ret = register_chrdev_region(beep.devid, BEEP_CNT, BEEP_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&beep.devid, 0, BEEP_CNT, BEEP_NAME);
        beep.major = MAJOR(beep.devid);
        beep.minor = MINOR(beep.devid);
    }
    printk("beep major = %d, minor = %d \r\n", beep.major, beep.minor);
    // 初始化 cdev
    beep.cdev.owner = THIS_MODULE;
    cdev_init(&beep.cdev, &beep_fops);
    cdev_add(&beep.cdev, beep.devid, BEEP_CNT);

    // 创建类
    beep.class = class_create(THIS_MODULE, BEEP_NAME);
    if (IS_ERR(beep.class))
    {
        return PTR_ERR(beep.class);
    }
    // 创建设备
    beep.device = device_create(beep.class, NULL, beep.devid, NULL, BEEP_NAME);
    if (IS_ERR(beep.device))
    {
        return PTR_ERR(beep.device);
    }

    // 获取设备节点
    beep.nd = of_find_node_by_path("/beep");
    if (beep.nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnode;
    }
    // 获取 beep 所对应的gpio(将设备树节点的gpio信息转化会gpio号)
    beep.beep_gpio = of_get_named_gpio(beep.nd, "beep-gpios", 0);
    if (beep.beep_gpio < 0)
    {
        printk("can't find beep_gpio\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }
    else
    {
        printk("beep gpio num = %d\r\n", beep.beep_gpio);
    }

    // 申请 IO
    ret = gpio_request(beep.beep_gpio, "beep-gpio");
    if (ret<0)
    {
        printk("beep gpio request fail\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }

    // 使用 io，设置 io 为输出
    ret = gpio_direction_output(beep.beep_gpio, 1);
    if (ret < 0)
    {
        // printk("led gpio request fail");
        ret = -EINVAL;
        goto fail_setoutput;
    }
    // 默认输出高电平，关闭 beep
    gpio_set_value(beep.beep_gpio, 1);

    return 0;
fail_setoutput:
    gpio_free(beep.beep_gpio);
fail_findnode:
    return ret;
}

static void __exit beep_exit(void)
{
    // 关闭蜂鸣器
    gpio_set_value(beep.beep_gpio, 1);
    // 注销字符设备驱动
    cdev_del(&beep.cdev);
    unregister_chrdev_region(beep.devid, BEEP_CNT);
    device_destroy(beep.class, beep.devid);
    class_destroy(beep.class);
    // 释放 io
    gpio_free(beep.beep_gpio);
}

// 模块入口和出口
module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");