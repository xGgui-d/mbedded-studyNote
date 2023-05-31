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

#define PLATFORM_CNT 1           /* 设备号个数 */
#define PLATFORM_NAME "platform" /* 名字 */
#define LEDOFF 0                 /* 关灯 */
#define LEDON 1                  /* 开灯 */

/* 资源的数量 */
#define RESOURCE_NUM 5

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

/* 开关灯函数 */
void led_switch(u8 sta)
{
    u32 val = 0;
    if (sta == LEDON)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    }
    else if (sta == LEDOFF)
    {
        val = readl(GPIO1_DR);
        val |= (1 << 3);
        writel(val, GPIO1_DR);
    }
}

/* 字符设备结构体（注意是字符设备，还是要放到驱动模块里面的） */
struct platform_dev_t
{
    dev_t devid;           /* 设备号 	 */
    struct cdev cdev;      /* cdev 	*/
    struct class *class;   /* 类 		*/
    struct device *device; /* 设备 	 */
    int major;             /* 主设备号	  */
    int minor;             /* 次设备号   */
};

struct platform_dev_t platform_dev; /* led设备 */

static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &platform_dev; /* 设置私有数据 */
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
    unsigned char ledstate;

    retvalue = copy_from_user(databuf, buf, cnt);
    if (retvalue < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    ledstate = databuf[0]; /* 获取状态值 */

    if (ledstate == LEDON)
    {
        led_switch(LEDON); /* 打开LED灯 */
    }
    else if (ledstate == LEDOFF)
    {
        led_switch(LEDOFF); /* 关闭LED灯 */
    }
    return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
    
    return 0;
}

/* 字符设备的操作函数 */
static struct file_operations platform_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

/* struct platform_driver {
    int (*probe)(struct platform_device *);
    int (*remove)(struct platform_device *);
    void (*shutdown)(struct platform_device *);
    int (*suspend)(struct platform_device *, pm_message_t state);
    int (*resume)(struct platform_device *);
    struct device_driver driver;
    const struct platform_device_id *id_table;
    bool prevent_deferred_probe;
}; */

/* probe 函数，当驱动和设备成功匹配就会执行此函数*/
static int led_probe(struct platform_device *dev)
{
    u32 val = 0;
    int i = 0;
    struct resource *ledresource[RESOURCE_NUM];
    printk("led_probe run\r\n");
    /* 初始化 LED 字符设备驱动 */

    /* 获取设备资源（也就是寄存器的物理地址） */
    for (i = 0; i < RESOURCE_NUM; i++)
    {
        ledresource[i] = platform_get_resource(dev, IORESOURCE_MEM, i);
        if (ledresource[i] == NULL)
            return -EINVAL;
    }
    /* 1、内存映射 */
    /* static inline resource_size_t resource_size(const struct resource *res)
    {
        return res->end - res->start + 1;
    }     */
    IMX6U_CCM_CCGR1 = ioremap(ledresource[0]->start, resource_size(ledresource[0]));
    SW_MUX_GPIO1_IO03 = ioremap(ledresource[1]->start, resource_size(ledresource[1]));
    SW_PAD_GPIO1_IO03 = ioremap(ledresource[2]->start, resource_size(ledresource[2]));
    GPIO1_DR = ioremap(ledresource[3]->start, resource_size(ledresource[3]));
    GPIO1_GDIR = ioremap(ledresource[4]->start, resource_size(ledresource[4]));

    /* 2、使能GPIO1时钟 */
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26); /* 清楚以前的设置 */
    val |= (3 << 26);  /* 设置新值 */
    writel(val, IMX6U_CCM_CCGR1);

    /* 3、设置GPIO1_IO03的复用功能，将其复用为
     *    GPIO1_IO03，最后设置IO属性。
     */
    writel(5, SW_MUX_GPIO1_IO03);

    /*寄存器SW_PAD_GPIO1_IO03设置IO属性
     *bit 16:0 HYS关闭
     *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
     */
    writel(0x10B0, SW_PAD_GPIO1_IO03);

    /* 4、设置GPIO1_IO03为输出功能 */
    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3); /* 清除以前的设置 */
    val |= (1 << 3);  /* 设置为输出 */
    writel(val, GPIO1_GDIR);

    /* 5、默认关闭LED */
    val = readl(GPIO1_DR);
    val |= (1 << 3);
    writel(val, GPIO1_DR);

    /* 注册字符设备驱动 */
    /* 1、创建设备号 */
    if (platform_dev.major)
    { /*  定义了设备号 */
        platform_dev.devid = MKDEV(platform_dev.major, 0);
        register_chrdev_region(platform_dev.devid, PLATFORM_CNT, PLATFORM_NAME);
    }
    else
    {                                                                             /* 没有定义设备号 */
        alloc_chrdev_region(&platform_dev.devid, 0, PLATFORM_CNT, PLATFORM_NAME); /* 申请设备号 */
        platform_dev.major = MAJOR(platform_dev.devid);                           /* 获取分配号的主设备号 */
        platform_dev.minor = MINOR(platform_dev.devid);                           /* 获取分配号的次设备号 */
    }
    printk("newcheled major=%d,minor=%d\r\n", platform_dev.major, platform_dev.minor);

    /* 2、初始化cdev */
    platform_dev.cdev.owner = THIS_MODULE;
    cdev_init(&platform_dev.cdev, &platform_fops);

    /* 3、添加一个cdev */
    cdev_add(&platform_dev.cdev, platform_dev.devid, PLATFORM_CNT);

    /* 4、创建类 */
    platform_dev.class = class_create(THIS_MODULE, PLATFORM_NAME);
    if (IS_ERR(platform_dev.class))
    {
        return PTR_ERR(platform_dev.class);
    }

    /* 5、创建设备 */
    platform_dev.device = device_create(platform_dev.class, NULL, platform_dev.devid, NULL, PLATFORM_NAME);
    if (IS_ERR(platform_dev.device))
    {
        return PTR_ERR(platform_dev.device);
    }

    return 0;
}

/* remove 函数，当关闭驱动或者设备时（只要其中一个被关闭），会执行此函数*/
static int led_remove(struct platform_device *dev)
{
    printk("led_remove run\r\n");
	/* 取消映射 */
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	/* 注销字符设备驱动 */
	cdev_del(&platform_dev.cdev);/*  删除cdev */
	unregister_chrdev_region(platform_dev.devid, PLATFORM_CNT); /* 注销设备号 */

	device_destroy(platform_dev.class, platform_dev.devid);
	class_destroy(platform_dev.class);

    return 0;
}

/* platform 驱动结构体 */
static struct platform_driver led_driver = {
    .probe = led_probe,
    .remove = led_remove,

    .driver = {
        .name = "imx6ull-led", // 驱动名字要与设备名字匹配
    },
};

/* 驱动加载 */
static int __init leddriver_init(void)
{
    platform_driver_register(&led_driver);
    return 0;
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