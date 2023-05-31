#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/types.h>
#include <linux/delay.h>

#define LED_MAJOR 200  // 主设备号
#define LED_NAME "led" // 设备名

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
static int led_open(struct inode *inode, struct file *filp)
{
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

static struct file_operations led_fops = {
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
	retvalue = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
	if (retvalue < 0)
	{
		printk("register chrdev failed!\r\n");
		return -EIO;
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
	unregister_chrdev(LED_MAJOR, LED_NAME);
}

/*模块入口*/
module_init(led_init);
/*模块出口*/
module_exit(led_exit);

// 添加 license
MODULE_LICENSE("GPL");
// 作者
MODULE_AUTHOR("xggui");