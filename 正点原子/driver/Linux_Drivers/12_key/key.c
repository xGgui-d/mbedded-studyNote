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
#define KEY_NAME "key"
#define KEY_CNT 1
#define KEY0VALUE 0xF0
#define INVAKEY 0x00

// gpioled设备结构体
struct key_dev
{
    dev_t devid;
    int major;
    int minor;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int key_gpio; // io 对应的编号
    atomic_t keyvalue;
    int dev_status;    // 设备状态 0 表示设备可以使用，1表示不可以使用
    struct mutex lock; // 互斥锁操作
};
struct key_dev key;

static ssize_t key_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{

    return 0;
}
static ssize_t key_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int value;
    int ret;
    struct key_dev *dev = (struct key_dev *)filp->private_data;
    if (gpio_get_value(dev->key_gpio) == 0)
    { // 按下
        while (!gpio_get_value(dev->key_gpio))
            ;
        atomic_set(&dev->keyvalue, KEY0VALUE);
    }
    else
    {
        atomic_set(&dev->keyvalue, INVAKEY);
    }
    value = atomic_read(&dev->keyvalue);
    ret = copy_to_user(buf, &value, sizeof(value));


    return ret;
}

static int key_open(struct inode *inode, struct file *filp)
{

    filp->private_data = &key;
    printk("open success!!!\r\n");
    mutex_lock(&key.lock);
    return 0;
}
static int key_release(struct inode *indoe, struct file *filp)
{
    mutex_unlock(&key.lock);

    return 0;
}
// 操作集
static const struct file_operations key_fops = {

    .owner = THIS_MODULE,
    .read = key_read,
    .write = key_write,
    .open = key_open,
    .release = key_release

};

static int __init keyio_init(void)
{

    int ret = 0;
    atomic_set(&key.keyvalue, INVAKEY);

    // 初始化互斥锁
    mutex_init(&key.lock);

    // 注册字符设备号
    key.major = 0;
    if (key.major)
    {
        // 给定设备号
        key.devid = MAJOR(key.major);
        ret = register_chrdev_region(key.devid, KEY_CNT, KEY_NAME);
    }
    else
    {
        ret = alloc_chrdev_region(&key.devid, 0, KEY_CNT, KEY_NAME);
        key.major = MAJOR(key.devid);
        key.minor = MINOR(key.devid);
    }
    printk("key major = %d, minor = %d \r\n", key.major, key.minor);
    // 初始化 cdev
    key.cdev.owner = THIS_MODULE;
    cdev_init(&key.cdev, &key_fops);
    cdev_add(&key.cdev, key.devid, KEY_CNT);

    // 创建类
    key.class = class_create(THIS_MODULE, KEY_NAME);
    if (IS_ERR(key.class))
    {
        return PTR_ERR(key.class);
    }
    // 创建设备
    key.device = device_create(key.class, NULL, key.devid, NULL, KEY_NAME);
    if (IS_ERR(key.device))
    {
        return PTR_ERR(key.device);
    }

    // 获取设备节点
    key.nd = of_find_node_by_path("/key");
    if (key.nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnode;
    }
    // 获取 key 所对应的gpio(将设备树节点的gpio信息转化会gpio号)
    key.key_gpio = of_get_named_gpio(key.nd, "key-gpios", 0);
    if (key.key_gpio < 0)
    {
        printk("can't find key_gpio\r\n");
        ret = -EINVAL;
        goto fail_findnode;
    }
    else
    {
        printk("key gpio num = %d\r\n", key.key_gpio);
    }

    // // 申请 IO
    // ret = gpio_request(gpioled.led_gpio, "led-gpio");
    // if (ret<0)
    // {
    //     printk("led gpio request fail\r\n");
    //     ret = -EINVAL;
    //     goto fail_findnode;
    // }

    // 使用 io，设置 io 为输入
    ret = gpio_direction_input(key.key_gpio);
    if (ret < 0)
    {
        // printk("led gpio request fail");
        ret = -EINVAL;
        goto fail_setinput;
    }

    return 0;

fail_setinput:
    gpio_free(key.key_gpio);
fail_findnode:
    return ret;
}

static void __exit keyio_exit(void)
{
    // 关灯
    gpio_set_value(key.key_gpio, 1);
    // 注销字符设备驱动
    cdev_del(&key.cdev);
    unregister_chrdev_region(key.devid, KEY_CNT);
    device_destroy(key.class, key.devid);
    class_destroy(key.class);
    // 释放 io
    gpio_free(key.key_gpio);
}

// 模块入口和出口
module_init(keyio_init);
module_exit(keyio_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");