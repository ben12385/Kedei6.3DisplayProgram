#include <linux/init.h>
#include <linux/module.h>
   
static int my_init(void)
{
	register_chrdev(0, "Kedi"
	printk(KERN_INFO "Hello world 1. \n")
	return  0;
}
   
static void my_exit(void)
{
	return;
}
   
module_init(my_init);
module_exit(my_exit);