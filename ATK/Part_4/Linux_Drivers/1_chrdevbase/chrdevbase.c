#include <linux/module.h>

static int __init chrdevbase_init(void)
{
	return 0;
}

static void __exit chrdevbase_exit(void)
{

}

/*模块入口*/
module_init(chrdevbase_init);
/*模块出口*/
module_exit(chrdevbase_exit);
