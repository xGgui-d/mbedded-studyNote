#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define NEWCHRLED_CNT 1			   /* 设备号个数 表示设备数量，在申请设备号或者向 Linux 内核添加字 \
符设备的时候需要设置设备数量，一般我们一个驱动一个设备，所以这个宏为 1*/
#define NEWCHRLED_NAME "newchrled" /* 设备名字 */

#define LEDOFF 0 // 关灯
#define LEDON 1	 // 开灯

// 需要用到的寄存器地址（物理地址）
#define CCM_CCGR1_BASE (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4)
#define GPIO1_DR_BASE (0X0209C000)
#define GPIO1_GDIR_BASE (0X0209C004)

// 映射后的寄存器虚拟地址指针
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

// 设备结构体
struct newchrled_dev
{
	dev_t devid;		   /* 设备号 */
	int major;			   /* 主设备号 */
	int minor;			   /* 次设备号 */

	struct cdev cdev;	   /* cdev 结构体，里面有 ops owner*/
	struct class *class;   /* 类 */
	struct device *device; /* 设备 */
};

struct newchrled_dev newchrled;

// 开关函数，给 GPIO1_DR 写值
void led_switch(u8 sta)
{
	u32 val = 0;
	if (sta == LEDON)
	{
		// 读取寄存器的值
		val = readl(GPIO1_DR);
		val &= ~(1 << 3);
		// 给寄存器写入值
		writel(val, GPIO1_DR);
	}
	else if (sta == LEDOFF)
	{
		val = readl(GPIO1_DR);
		val |= (1 << 3);
		writel(val, GPIO1_DR);
	}
}

// 打开设备函数
/*设备文件， file 结构体有个叫做 private_data 的成员变量
一般在 open 的时候将 private_data 指向设备结构体。*/
static int led_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &newchrled; // 设置私有数据
	return 0;
}

// 从设备中读取数据
static ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

// 向设备中写入数据(这里通过写入数据控制灯的开关)
static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue;
	unsigned char databuf[1]; // 缓存
	unsigned char ledstat;	  // 灯的状态

	retvalue = copy_from_user(databuf, buf, cnt);
	// 读取数据失败
	if (retvalue < 0)
	{
		printk("kernel write failed!\r\n");
		return -EFAULT;
	}
	ledstat = databuf[0]; // 暂存读取到的灯的状态
	// 根据控制字进行操作
	if (ledstat == LEDON)
	{
		led_switch(LEDON); /* 打开 LED 灯 */
	}
	else if (ledstat == LEDOFF)
	{
		led_switch(LEDOFF); /* 关闭 LED 灯 */
	}

	return 0;
}

/* 关闭/释放设备 */
static int led_release(struct inode *inode, struct file *filp)
{
	/* 用户实现具体功能 */
	return 0;
}

static struct file_operations newchrled_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = led_release,
};
// 设备初始化入口
static int __init led_init(void)
{

	int retvalue = 0;
	u32 val = 0;

	/* 1、寄存器地址映射 */
	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
	SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

	/* 2、使能 GPIO1 时钟 */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26); /* 清除以前的设置 */
	val |= (3 << 26);  /* 设置新值 */
	writel(val, IMX6U_CCM_CCGR1);

	/* 3、设置 GPIO1_IO03 的复用功能，将其复用为
	 * GPIO1_IO03，最后设置 IO 属性。
	 */
	writel(0x05, SW_MUX_GPIO1_IO03);
	/* 寄存器 SW_PAD_GPIO1_IO03 设置 IO 属性 */
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	/* 4、设置 GPIO1_IO03 为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3); /* 清除以前的设置 */
	val |= (1 << 3);  /* 设置为输出 */
	writel(val, GPIO1_GDIR);

	/* 5、默认关闭 LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);
	writel(val, GPIO1_DR);

	/* 6、注册字符设备驱动 */
	if (newchrled.major)
	{
		// 如果定义了设备号
		newchrled.devid = MKDEV(newchrled.major, 0);
		register_chrdev_region(newchrled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
	}
	else
	{
		// 如果没有定义设备号
		// 获取设备号
		alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);
		newchrled.major = MAJOR(newchrled.devid);
		newchrled.minor = MINOR(newchrled.devid);
	}
	printk("newchrled major = %d, minor = %d\r\n", newchrled.major, newchrled.minor);
	/*7、初始化 cdev*/
	newchrled.cdev.owner = THIS_MODULE;
	cdev_init(&newchrled.cdev, &newchrled_fops);
	/*8、添加一个 cdev*/
	cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_CNT);
	/*9、创建类*/
	newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
	if (IS_ERR(newchrled.class))
	{
		return PTR_ERR(newchrled.class);
	}
	/*10、创建设备节点*/
	newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
	if (IS_ERR(newchrled.device))
	{
		return PTR_ERR(newchrled.device);
	}

	return 0;
}
// 设备注销出口
static void __exit led_exit(void)
{
	/* 取消映射 */
	iounmap(IMX6U_CCM_CCGR1);
	iounmap(SW_MUX_GPIO1_IO03);
	iounmap(SW_PAD_GPIO1_IO03);
	iounmap(GPIO1_DR);
	iounmap(GPIO1_GDIR);

	/* 注销字符设备驱动 */
	cdev_del(&newchrled.cdev);
	unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT);
	device_destroy(newchrled.class, newchrled.devid);
	class_destroy(newchrled.class);
}

/*模块入口*/
module_init(led_init);
/*模块出口*/
module_exit(led_exit);

// 添加 license
MODULE_LICENSE("GPL");
// 作者
MODULE_AUTHOR("xggui");