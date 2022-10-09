#ifndef __VMA450_I2C_H__
#define __VMA450_I2C_H__

int vma450_i2c_init(int intf_init);
void vma450_i2c_cleanup(void);
void vma450_i2c_send(const char *text, int len);

#endif
