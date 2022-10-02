#ifndef __VMA450_CDEV_H__
#define __VMA450_CDEV_H__

#define DEVICE_NAME "vma450lcd" /* Dev name as it appears in /proc/devices   */ 

enum {
	CDEV_NOT_USED = 0,
	CDEV_EXCLUSIVE_OPEN = 1,
};

int vma450_cdev_init(void);

void vma450_cdev_cleanup(void);

int device_open(struct inode *, struct file *); 

int device_release(struct inode *, struct file *); 

ssize_t device_read(struct file *, char __user *, size_t, loff_t *); 

ssize_t device_write(struct file *, const char __user *, size_t, 
loff_t *); 

#endif
