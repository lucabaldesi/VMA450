#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#include "vma450_sysfs.h"
#include "vma450_i2c.h"

static struct kobject *vma450_sysfs;
static int display_on = 1;

static ssize_t display_on_show(struct kobject *kobj,
                               struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", display_on);
}


static ssize_t display_on_store(struct kobject *kobj,
                                struct kobj_attribute *attr, char *buf,
                                size_t count)
{
	sscanf(buf, "%du", &display_on);
	if (display_on)
		vma450_i2c_display_on();
	else
		vma450_i2c_display_off();
	return count;
}

static struct kobj_attribute display_on_attribute =
	__ATTR(display_on, 0660, display_on_show, (void *)display_on_store);

int vma450_sysfs_init(void)
{
	int error;

	vma450_sysfs = kobject_create_and_add("vma450", kernel_kobj);

	if (!vma450_sysfs)
		return -ENOMEM;

	error = sysfs_create_file(vma450_sysfs, &display_on_attribute.attr); 

	if (error) {
		pr_info("failed to create the myvariable file "
		        "in /sys/kernel/vma450\n");
	}

	return error;
}

void vma450_sysfs_cleanup(void)
{
	kobject_put(vma450_sysfs);
}
