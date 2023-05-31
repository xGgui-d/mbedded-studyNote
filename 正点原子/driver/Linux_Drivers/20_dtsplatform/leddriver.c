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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DTSPLATFORM_CNT 1              /* 设备号个数 */
#define DTSPLATFORM_NAME "dtsplatform" /* 名字 */
#define LEDOFF 0                       /* 关灯 */
#define LEDON 1                        /* 开灯 */

/* 字符设备结构体 */
struct dtsled_dev_t
{
    dev_t devid;            /* 设备号 	 */
    struct cdev cdev;       /* cdev 	*/
    struct class *class;    /* 类 		*/
    struct device *device;  /* 设备 	 */
    int major;              /* 主设备号	  */
    int minor;              /* 次设备号   */
    struct device_node *nd; /* 设备节点 */
    unsigned int led_gpio;  /* led所使用的GPIO编号		*/
};
struct dtsled_dev_t dtsled_dev; /* led设备 */

static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &dtsled_dev; /* 设置私有数据 */
    return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char ledstat;
    struct dtsled_dev_t *dev = filp->private_data;

    retvalue = copy_from_user(databuf, buf, cnt);
    if (retvalue < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    ledstat = databuf[0]; /* 获取状态值 */

    if (ledstat == LEDON)
    {
        gpio_set_value(dev->led_gpio, 0); /* 打开LED灯 */
    }
    else if (ledstat == LEDOFF)
    {
        gpio_set_value(dev->led_gpio, 1); /* 关闭LED灯 */
    }
    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/* 字符设备操作函数 */
static struct file_operations dtsled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/* struct of_device_id {
    char	name[32];
    char	type[32];
    char	compatible[128];
    const void *data;
}; */

/* probe 函数，只要该驱动与设备树匹配起来就会执行该函数 */
static int led_probe(struct platform_device *dev)
{
    int ret = 0;

    /* 设置LED所使用的GPIO */
    /* 1、获取设备节点：gpioled */
    /* 这里由于驱动已经与设备树匹配成功了，所以就不需要再找节点*/
#if 0    
	dtsled_dev.nd = of_find_node_by_path("/gpioled");
	if(dtsled_dev.nd == NULL) {
		printk("gpioled node not find!\r\n");
		return -EINVAL;
	} else {
		printk("gpioled node find!\r\n");
	}
#endif
    // 因此可以直接通过指针寻找到节点
    dtsled_dev.nd = dev->dev.of_node;

    /* 2、 获取设备树中的gpio属性，得到LED所使用的LED编号 */
    dtsled_dev.led_gpio = of_get_named_gpio(dtsled_dev.nd, "led-gpios", 0);
    if (dtsled_dev.led_gpio < 0)
    {
        printk("can't get led-gpio");
        return -EINVAL;
    }
    printk("led-gpio num = %d\r\n", dtsled_dev.led_gpio);

    /* 3、设置GPIO1_IO03为输出，并且输出高电平，默认关闭LED灯 */
    ret = gpio_direction_output(dtsled_dev.led_gpio, 1);
    if (ret < 0)
    {
        printk("can't set gpio!\r\n");
    }

    /* 注册字符设备驱动 */
    /* 1、创建设备号 */
    if (dtsled_dev.major)
    { /*  定义了设备号 */
        dtsled_dev.devid = MKDEV(dtsled_dev.major, 0);
        register_chrdev_region(dtsled_dev.devid, DTSPLATFORM_CNT, DTSPLATFORM_NAME);
    }
    else
    {                                                                                 /* 没有定义设备号 */
        alloc_chrdev_region(&dtsled_dev.devid, 0, DTSPLATFORM_CNT, DTSPLATFORM_NAME); /* 申请设备号 */
        dtsled_dev.major = MAJOR(dtsled_dev.devid);                                   /* 获取分配号的主设备号 */
        dtsled_dev.minor = MINOR(dtsled_dev.devid);                                   /* 获取分配号的次设备号 */
    }
    printk("gpioled major=%d,minor=%d\r\n", dtsled_dev.major, dtsled_dev.minor);

    /* 2、初始化cdev */
    dtsled_dev.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled_dev.cdev, &dtsled_fops);

    /* 3、添加一个cdev */
    cdev_add(&dtsled_dev.cdev, dtsled_dev.devid, DTSPLATFORM_CNT);

    /* 4、创建类 */
    dtsled_dev.class = class_create(THIS_MODULE, DTSPLATFORM_NAME);
    if (IS_ERR(dtsled_dev.class))
    {
        return PTR_ERR(dtsled_dev.class);
    }

    /* 5、创建设备 */
    dtsled_dev.device = device_create(dtsled_dev.class, NULL, dtsled_dev.devid, NULL, DTSPLATFORM_NAME);
    if (IS_ERR(dtsled_dev.device))
    {
        return PTR_ERR(dtsled_dev.device);
    }
    return 0;
}

static int led_remove(struct platform_device *dev)
{
    printk("led remove run\r\n");

    /* 注销字符设备驱动 */
    cdev_del(&dtsled_dev.cdev);                                  /*  删除cdev */
    unregister_chrdev_region(dtsled_dev.devid, DTSPLATFORM_CNT); /* 注销设备号 */

    device_destroy(dtsled_dev.class, dtsled_dev.devid);
    class_destroy(dtsled_dev.class);

    return 0;
}

/* 设备树匹配表结构体(就是一个设备id数组) */
struct of_device_id led_of_match[] = {
    {.compatible = "alientek,gpioled"},
    {.compatible = "xxx"},
    {/* sentinel */},
};

struct platform_driver led_driver = {
    .driver = {
        .name = "imx6ull-led",          /* 无设备树的时候用来进行匹配 */
        .of_match_table = led_of_match, /* 设备树匹配表 */
    },
    .remove = led_remove,
    .probe = led_probe,
};

/* 驱动加载 */
static int __init leddriver_init(void)
{
    return platform_driver_register(&led_driver);
}

/* 驱动卸载 */
static void __exit leddriver_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(leddriver_init);
module_exit(leddriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");