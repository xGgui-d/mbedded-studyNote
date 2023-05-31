#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/types.h>
#include <linux/delay.h>

#define CHRDEVBASE_MAJOR 200		 // 主设备号
#define CHRDEVBASE_NAME "chrdevbase" // 设备名

static char readbuf[100];  // 读缓冲区
static char writebuf[100]; // 写缓冲区
static char kerneldata[] = {"kernel data !"};

// 打开设备
static int chrdevbase_open(struct inode *inode, struct file *filp)
{
	/* 用户实现具体功能 */
	return 0;
}
// 从设备中读取
static ssize_t chrdevbase_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue = 0;
	// 向用户空间发送数据
	memcpy(readbuf, kerneldata, sizeof(kerneldata));
	retvalue = copy_to_user(buf, readbuf, cnt);
	if (retvalue == 0)
	{
		printk("kernel senddata ok!\r\n");
	}
	else
	{
		printk("kernel senddata failed!\r\n");
	}
	return 0;
}

// 向设备中写
static ssize_t chrdevbase_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	int retvalue = 0;
	retvalue = copy_from_user(writebuf, buf, cnt);
	if (retvalue == 0)
	{
		printk("kernel recevdata:%s\r\n", writebuf);
	}
	else
	{
		printk("kernel recevdata failed!\r\n");
	}

	
	return 0;
}

/* 关闭/释放设备 */
static int chrdevbase_release(struct inode *inode, struct file *filp)
{
	/* 用户实现具体功能 */
	return 0;
}

static struct file_operations test_fops = {
	.owner = THIS_MODULE,
	.open = chrdevbase_open,
	.read = chrdevbase_read,
	.write = chrdevbase_write,
	.release = chrdevbase_release,
};
// 设备初始化入口
static int __init chrdevbase_init(void)
{

	int retvalue = 0;
	printk("init: chrdevbase\r\n");
	// 注册字符设备
	retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &test_fops);
	if (retvalue < 0)
		printk("register fail\r\n");

	return 0;
}
// 设备注销出口
static void __exit chrdevbase_exit(void)
{
	printk("exit: chrdevbase\r\n");
	// 注销字符设备
	unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
}

/*模块入口*/
module_init(chrdevbase_init);
/*模块出口*/
module_exit(chrdevbase_exit);

// 添加 license
MODULE_LICENSE("GPL");
// 作者
MODULE_AUTHOR("xggui");
