#include <linux/module.h>
#include <l4/log/log.h>

static int __init l4x_module_init(void)
{
	L4XV_V(f);
	printk("Hi from the sample module\n");

	L4XV_L(f);
	LOG_printf("sample module: Also a warm welcome to the console\n");
	L4XV_U(f);

	return 0;
}

static void __exit l4x_module_exit(void)
{
	L4XV_V(f);
	L4XV_L(f);
	LOG_printf("Bye from sample module\n");
	L4XV_U(f);
}

module_init(l4x_module_init);
module_exit(l4x_module_exit);

MODULE_AUTHOR("Adam Lackorzynski <adam@os.inf.tu-dresden.de>");
MODULE_DESCRIPTION("L4Linux sample module");
MODULE_LICENSE("GPL");
