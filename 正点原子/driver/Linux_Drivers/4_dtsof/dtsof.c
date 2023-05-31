#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ide.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/of.h>

#if 0
	backlight {
		compatible = "pwm-backlight";
		pwms = <&pwm1 0 5000000>;
		brightness-levels = <0 4 8 16 32 64 128 255>;
		default-brightness-level = <6>;
		status = "okay";
	};
#endif

static int __init dtsof_init(void)
{

    int ret = 0;
    struct device_node *nd;
    struct property *comppro;
    const char *str;
    u32 def_value = 0;
    u32 *brival;
    int elemsize = 0;

    // 找到设备树的某个节点 backlight
    // 路径是 /backlight
    nd = of_find_node_by_path("/backlight");
    if (nd == NULL)
    {
        ret = -EINVAL;
        goto fail_findnd;
    }
    // 获取属性 compatible
    comppro = of_find_property(nd, "compatible", NULL);
    if (comppro == NULL)
    {
        ret = -EINVAL;
        goto fail_findpro;
    }
    // 获取 status
    ret = of_property_read_string(nd, "status", &str);
    if (ret < 0)
    {
        goto fail_findrs;
    }
    // 获取 数字属性值
    ret = of_property_read_u32(nd, "default-brightness-level", &def_value);
    if (ret < 0)
    {
        goto fail_read32;
    }
    // 获取u32数组属性值
    // 先获取数组大小
    elemsize = of_property_count_elems_of_size(nd, "brightness-levels", sizeof(u32));
    if (elemsize < 0)
    {
        goto fail_readelecount;
    }
    else
    {
        printk("brightness-levels-count = %d\r\n", elemsize);
        // 申请内存保存数组值
        brival = kmalloc(elemsize * sizeof(u32), GFP_KERNEL);      
        if (!brival)
            goto fail_mem;
        // 数组中各个元素
        ret = of_property_read_u32_array(nd, "brightness-levels", brival, elemsize);
        if (ret < 0)
            goto fail_readu32array;
        else
        {
            int m = 0;
            printk("brightness-levels: ");
            for (; m < elemsize; m++)
                printk("%d ", *(brival + m));
            printk("\r\n");
            kfree(brival);
        }
    }

    // 打印值
    printk("compatible = %s\r\n", (char *)comppro->value);
    printk("status = %s\r\n", str);
    printk("default-brightness-level = %d\r\n", def_value);
    return 0;

fail_readu32array:
fail_mem:
fail_readelecount:
fail_read32:
fail_findrs:
fail_findpro:
fail_findnd:
    return ret;
}

static void __exit dtsof_exit(void)
{
}

// 模块入口和出口
module_init(dtsof_init);
module_exit(dtsof_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xggui");