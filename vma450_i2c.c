#include <linux/i2c.h>
#include <linux/list.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>  /* needed for kzalloc() */
#include <linux/property.h>  /* needed for kzalloc() */

#include "vma450_i2c.h"

/*
 * Interface with I2C mapped to LCD I/O:
 * DB7 DB6 DB5 DB4 P3 E/CS RW RS
 * Data can be transfered with two rounds of 4-bit each. First DB7-DB4, then DB3-DB0.
 *
 * P3 controls the backlight and we always set it on (VMA450_CMD, VMA450_DATA).
 */

#define ENABLE_BIT 0x04
#define VMA450_CMD 0x08
#define VMA450_DATA 0x09

#define VMA450_CONF_2LINES 0x08
#define VMA450_CONF_1LINE 0x00
#define VMA450_CONF_5x8 0x00
#define VMA450_CONF_5x11 0x04

#define MIN(a, b) ((a<b) ? (a) : (b))

static struct i2c_client *vma450_dev = NULL;

int vma450_i2c_read(struct i2c_client *client, char *buf)
{
	int res = 0;
	if (buf) {
		res = i2c_master_recv(client, buf, 1);
		usleep_range (5*1000, 10*1000);
	}
	return res;
}

int vma450_remove(struct i2c_client *client)
{
	return 0;
}

int vma450_probe(struct i2c_client *client)
{
	int res;
	__u8 buf;

	res = vma450_i2c_read(client, &buf);
	return (res > 0) ? 1 : 0;
}

int vma450_i2c_write_4bit(struct i2c_client *client, __u8 head, __u8 data)
{
	int ret;
	__u8 buff;

	buff = head | ((data & 0x0f)<<4) | ENABLE_BIT;
	ret = i2c_master_send(client, &buff, 1);
	usleep_range (5*1000, 10*1000);
	buff = buff & (~ENABLE_BIT);
	ret = i2c_master_send(client, &buff, 1);
	usleep_range (5*1000, 10*1000);
	return ret;
}

int vma450_i2c_write(struct i2c_client *client, __u8 head, __u8 data)
{
	int res;
	res = vma450_i2c_write_4bit(client, head, data>>4);
	res += vma450_i2c_write_4bit(client, head, data & 0x0f);
	return res;
}

int vma450_i2c_probe(struct i2c_client *dev)
{
	char buf;
	int res;

	res = vma450_i2c_read(dev, &buf);
	return (res > 0) ? 1 : 0;
}

#ifndef i2c_new_client_device
struct i2c_client *
i2c_new_client_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
	struct i2c_client      *client;
	int    status = -1;

	client = kzalloc(sizeof *client, GFP_KERNEL);
	if (!client)
		return ERR_PTR(-ENOMEM);

	client->adapter = adap;

	client->dev.platform_data = info->platform_data;
	client->flags = info->flags;
	client->addr = info->addr;

	if (client->addr == 0x00 || client->addr > 0x7f) {
		dev_err(&adap->dev, "Invalid 7-bit I2C address 0x%02hx\n", client->addr);
		goto out_err_silent;
	}

	client->dev.parent = &client->adapter->dev;
	client->dev.bus = &i2c_bus_type;
	client->dev.type = &i2c_client_type;
	client->dev.of_node = of_node_get(info->of_node);
	client->dev.fwnode = info->fwnode;

	dev_set_name(&client->dev, "%s", "vma450");
	status = device_register(&client->dev);
	if (status)
		goto out_remove_swnode;

	dev_dbg(&adap->dev, "client [%s] registered with bus id %s\n",
		client->name, dev_name(&client->dev));

	return client;

	out_remove_swnode:
	of_node_put(info->of_node);
	dev_err(&adap->dev,
		"Failed to register i2c client %s at 0x%02x (%d)\n",
		client->name, client->addr, status);
	out_err_silent:
	kfree(client);
	return ERR_PTR(status);
}
#endif

void vma450_i2c_info_init(struct i2c_board_info *info, __u8 addr)
{
	memset(info, 0, sizeof(struct i2c_board_info));
	strscpy(info->type, "vma450", I2C_NAME_SIZE-1);
	info->addr = addr;
}


int vma450_i2c_scan(void)
{
	int adap_n;
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info i2c_info;

	adap_n = 0;
	while (!vma450_dev && adap_n < 2) {
		i2c_adap = i2c_get_adapter(adap_n);
		if (i2c_adap) {
			vma450_i2c_info_init(&i2c_info, 0x27);
			vma450_dev = i2c_new_client_device(i2c_adap, &i2c_info);
			if (IS_ERR(vma450_dev) || vma450_i2c_probe(vma450_dev)==0) {
				vma450_dev = NULL;
			}
		}
		adap_n++;
	}

	return vma450_dev ? 0 : -1;
}

int pcf8574_init(struct i2c_client *client, __u8 conf)
{
	int ret;
	ret = i2c_master_send(client, &conf, 1);
	usleep_range (5*1000, 10*1000);
	return ret;
}

void vma450_i2c_display_set(struct i2c_client * client, __u8 light, __u8 cursor, __u8 cursor_blink)
{
	__u8 conf = 0x08;
	
	if (light)
		conf |= 0x04;
	if (cursor)
		conf |= 0x02;
	if (cursor_blink)
		conf |= 0x01;
	vma450_i2c_write(client, VMA450_CMD, conf);
}

void vma450_i2c_mode_set(struct i2c_client * client, int forward, int shift)
{
	__u8 conf = 0x04;
	
	if (forward)
		conf |= 0x02;
	if (shift)
		conf |= 0x01;
	vma450_i2c_write(client, VMA450_CMD, conf);
}

void vma450_i2c_clear(struct i2c_client * client)
{
	vma450_i2c_write(client, VMA450_CMD, 0x01);
}

void vma450_i2c_home(struct i2c_client * client)
{
	vma450_i2c_write(client, VMA450_CMD, 0x02);
}

void vma450_i2c_shift_cursor(struct i2c_client * client, int right)
{
	__u8 conf = 0x10;
	
	if (right)
		conf |= 0x04;
	vma450_i2c_write(client, VMA450_CMD, conf);
}

void vma450_i2c_shift_display(struct i2c_client * client, int right)
{
	__u8 conf = 0x18;

	if (right)
		conf |= 0x04;
	vma450_i2c_write(client, VMA450_CMD, conf);
}

void vma450_i2c_write_str(struct i2c_client *client, const char * str)
{
	int i = 0;
	while (str && str[i] != '\0') {
		vma450_i2c_write(client, VMA450_DATA, str[i]);
		i++;
	}
}

void vma450_i2c_write_str_2lines(struct i2c_client *client, const char * str, int len)
{
	int i = 0, j = 0;
	char buffer[80];
	
	memset(buffer, 0, 80);

	while (i<len && j<80) {
		buffer[j] = str[i];
		i++;
		j++;
		switch (i) {
			case 20:
				j = 40;
				break;
			case 40:
				j = 20;
				break;
			case 60:
				j = 60;
				break;
			case 80:
				j = 0;
				break;
		}
	}

	vma450_i2c_clear(client);
	vma450_i2c_write_str(client, buffer);
}

void vma450_i2c_send(const char *text, int len)
{
	vma450_i2c_write_str_2lines(vma450_dev, text, len);
}

int vma450_i2c_init(int intf_init)
{
	int res;
	res = vma450_i2c_scan();
	if (!res) {
		if (intf_init) {
			pcf8574_init(vma450_dev, 0x00);  // configure all outputs
			vma450_i2c_write_4bit(vma450_dev, VMA450_CMD, 0x02);  // configure 4-bit operations
		}

		/* the line below can be executed only after a Power-On-Reset */
		vma450_i2c_write(vma450_dev, VMA450_CMD, 0x20 | VMA450_CONF_2LINES | VMA450_CONF_5x11);  // set num of lines and font

		vma450_i2c_display_set(vma450_dev, 1, 1, 0);
		vma450_i2c_mode_set(vma450_dev, 1, 0); 
	}

	return res;
}

void vma450_i2c_cleanup(void)
{
	if (vma450_dev)
		i2c_unregister_device(vma450_dev);
}
