/* 
 *
 *  vma450.c - short description
 *
 *
 */

#include <linux/init.h> /* Needed for the macros */
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h> /* Needed by all modules */

#include "vma450_cdev.h"
#include "vma450_i2c.h"
#include "vma450_sysfs.h"

MODULE_AUTHOR("Luca Baldesi <luca@baldesi.ovh>");
MODULE_DESCRIPTION("Driver for I2C device Whadda VMA450");

int intf_init = 1;

module_param(intf_init, int, 0);
MODULE_PARM_DESC(intf_init, "Whether to initialize the 4bit interface with vma450. Default is \"1\", meaning \"yes\".");

static int __init vma450_init(void)
{
	int res;

	res = vma450_i2c_init(intf_init);
	if (res)
		return res;

	res = vma450_cdev_init();
	if (res) {
		vma450_i2c_cleanup();
		return res;
	}

	res = vma450_sysfs_init();
	if (res) {
		vma450_i2c_cleanup();
		vma450_cdev_cleanup();
	}

	return res;
}


static void __exit vma450_exit(void)
{
	vma450_i2c_cleanup();
	vma450_cdev_cleanup();
	vma450_sysfs_cleanup();
}

module_init(vma450_init);
module_exit(vma450_exit);


MODULE_LICENSE("GPL");
