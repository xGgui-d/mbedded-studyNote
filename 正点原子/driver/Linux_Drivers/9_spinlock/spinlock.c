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
#define GPIOLED_NAME "spinlock"
#define GPIOLED_CNT 1

#define OFF 0
#define ON 1

// gpioled设备结构体
struct gpioled_dev
{
    dev_t devid;
    int major;
    int minor;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int led_gpio;    // io 对应的编号
    int dev_status;  // 设备状态 0 表示设备可以使用，1表示不可以使用
    spinlock_t lock; // 自旋锁操作
};
struct gpioled_dev gpioled;

static ssize_t gpioled_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
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
        gpio_set_value(gpioled.led_gpio, 0);
    }
    else if (databuf[0] == OFF)
        gpio_set_value(gpioled.led_gpio, 1);

    return 0;
}
static ssize_t gpioled_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    // struct dtsled_dev *dev = (struct dtsled_dev *)filp->private_data;
    return 0;
}

static int gpioled_open(struct inode *inode, struct file *filp)
{
    // 加锁
    spin_lock(&gpioled.lock);
    if (gpioled.dev_status)
    {
        // 驱动不可使用
        spin_unlock(&gpioled.lock);
        return -EBUSY;
    }
    // 驱动可以使用
    gpioled.dev_status++;
    // 解锁
    spin_unlock(&gpioled.lock);
    filp->private_data = &gpioled;
    printk("open success!!!");

    return 0;
}
static int gpioled_release(struct inode *indoe, struct file *filp)
{
    // struct gpioled_dev *dev = filp->private_data;
    spin_lock(&gpioled.lock);
    if (gpioled.dev_status)
        gpioled.dev_status--;
    spin_unlock(&gpioled.lock);

    return 0;
}
// 操作集
static const struct file_operations gpioled_fops = {

    .owner = THIS_MODULE,
    .read = gpioled_read,
    .write = gpioled_write,
    .open = gpioled_open,
    .release = gpioled_release

};

static int __init gpioled_init(void)
{

    int ret = 0;

    // 初始化自旋锁
    spin_lock_init(&gpioled.lock);
    gpioled.dev_status = 0;

    // 注册字符设备号
    gpioled.major = 0;
    if (gpioled.major)
    {
        // 给定设备号
        gpioled.devid = MAJOR(gpioled.major);
        ret = register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }
    printk("gpioled major = %d, minor = %d \r\n", gpioled.major, gpioled.minor);
    // 初始化 cdev
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    // 创建类
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if (IS_ERR(gpioled.class))
    {
        return PTR_ERR(gpioled.class);
    }
    // 创建设备
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
    if (IS_ERR(gpioled.device))
    {
        return PTR_ERR(gpioled.device);
    }

    // 获取设备节点
    gpioled.nd = of_find_node_by_path("/gpioled");
    if (gpioled.nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnode;
    }
    // 获取 led 所对应的gpio(将设备树节点的gpio信息转化会gpio号)
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpios", 0);
    if (gpioled.led_gpio < 0)
    {
        printk("can't find led_gpio\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }
    else
    {
        printk("led gpio num = %d\r\n", gpioled.led_gpio);
    }

    // // 申请 IO
    // ret = gpio_request(gpioled.led_gpio, "led-gpio");
    // if (ret<0)
    // {
    //     printk("led gpio request fail\r\n");
    //     ret = -EINVAL;
    //     goto fail_findnode;
    // }

    // 使用 io，设置 io 为输出
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if (ret < 0)
    {
        // printk("led gpio request fail");
        ret = -EINVAL;
        goto fail_setoutput;
    }
    // 默认输出低电平，点亮 led 灯
    gpio_set_value(gpioled.led_gpio, 0);

    return 0;
fail_setoutput:
    gpio_free(gpioled.led_gpio);
fail_findnode:
    return ret;
}

static void __exit gpioled_exit(void)
{
    // 关灯
    gpio_set_value(gpioled.led_gpio, 1);
    // 注销字符设备驱动
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);
    // 释放 io
    gpio_free(gpioled.led_gpio);
}

// 模块入口和出口
module_init(gpioled_init);
module_exit(gpioled_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");