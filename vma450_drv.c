/* 
 *
 *  vma450.c - short description
 *
 *
 */

#include <linux/init.h> /* Needed for the macros */
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h> /* Needed by all modules */

#include "vma450_cdev.h" /* Needed by all modules */

MODULE_AUTHOR("Luca Baldesi <luca@baldesi.ovh>");
MODULE_DESCRIPTION("Driver for I2C device Whadda VMA450");


static int __init vma450_init(void)
{
	pr_info("Hello, world 2\n");
	return vma450_cdev_init();
}


static void __exit vma450_exit(void)
{
	pr_info("Goodbye, world 2\n");
	vma450_cdev_cleanup();
}

module_init(vma450_init);
module_exit(vma450_exit);


MODULE_LICENSE("GPL");
