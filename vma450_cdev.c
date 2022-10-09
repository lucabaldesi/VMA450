#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "vma450_cdev.h"
#include "vma450_i2c.h"


static int major;

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

static struct class *cls;

struct file_operations chardev_fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
};

int vma450_cdev_init(void)
{
	major = register_chrdev(0, DEVICE_NAME, &chardev_fops);

	if (major < 0) {
		pr_alert("[VMA450] Registering char device failed with %d\n", major);
		return major;
	}
	
	cls = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	pr_info("[VMA450] Device created on /dev/%s\n", DEVICE_NAME);

	return 0;
}

void vma450_cdev_cleanup(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, DEVICE_NAME);
}

int device_open(struct inode *inode, struct file *file)
{
	if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
		return -EBUSY;
	try_module_get(THIS_MODULE);
	return 0;
}

int device_release(struct inode *inode, struct file *file)
{
	atomic_set(&already_open, CDEV_NOT_USED);
	module_put(THIS_MODULE);
	return 0;
}

ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */
                    char __user *buffer, /* buffer to fill with data */
                    size_t length, /* length of the buffer     */
                    loff_t *offset)
{
	pr_alert("Sorry, this operation is not supported.\n");
	return -EINVAL;
}

ssize_t device_write(struct file *filp, const char __user *buff,
                     size_t len, loff_t *off)
{
	vma450_i2c_send(buff, len-1);
	return 80;
}
