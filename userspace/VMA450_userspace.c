#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

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


int vma450_read(int dev_fd, __u8 *buffer)
{
    int ret;
    ret = read(dev_fd, buffer, 1);
    usleep (5*1000);
    return ret;
}

int vma450_probe(__u8 addr)
{
    int dev_fd, adapter = 2;
    int found = 0;
    char filename[20];
    __u8 buffer;

    while (!found && adapter>=0) {
        snprintf(filename, 19, "/dev/i2c-%d", adapter);
        dev_fd = open(filename, O_RDWR);

        if (dev_fd >= 0) {
            ioctl(dev_fd, I2C_SLAVE, addr);
            if (vma450_read(dev_fd, &buffer) >=1) {
                found = 1;
            } else {
                close(dev_fd);
            }
        }
        adapter--;
    }

    return (found ? dev_fd : -1);
}

int pcf8574_init(int dev_fd, __u8 conf)
{
    int ret;
    ret = write(dev_fd, &conf, 1);
    usleep (5*1000);
    return ret;
}

int vma450_send_4bit(int dev_fd, __u8 head, __u8 data)
{
    int ret;
    __u8 buff;

    buff = head | ((data & 0x0f)<<4) | ENABLE_BIT;
    ret = write(dev_fd, &buff, 1);
    usleep (5*1000);
    buff = buff & (~ENABLE_BIT);
    ret = write(dev_fd, &buff, 1);
    usleep (5*1000);
    return ret;
}

int vma450_send(int dev_fd, __u8 head, __u8 data)
{
    vma450_send_4bit(dev_fd, head, data>>4);
    vma450_send_4bit(dev_fd, head, data & 0x0f);
}

void vma450_write_str(int dev_fd, const char * str)
{
    int i = 0;
    while (str && str[i] != '\0') {
        vma450_send(dev_fd, VMA450_DATA, str[i]);
        i++;
    }
}

void vma450_display_set(int dev_fd, __u8 light, __u8 cursor, __u8 cursor_blink)
{
    __u8 conf = 0x08;

    if (light)
        conf |= 0x04;
    if (cursor)
        conf |= 0x02;
    if (cursor_blink)
        conf |= 0x01;
    vma450_send(dev_fd, VMA450_CMD, conf);
}

void vma450_mode_set(int dev_fd, int forward, int shift)
{
    __u8 conf = 0x04;

    if (forward)
        conf |= 0x02;
    if (shift)
        conf |= 0x01;
    vma450_send(dev_fd, VMA450_CMD, conf);
}

void vma450_clear(int dev_fd)
{
    vma450_send(dev_fd, VMA450_CMD, 0x01);
}

void vma450_home(int dev_fd)
{
    vma450_send(dev_fd, VMA450_CMD, 0x02);
}

void vma450_shift_cursor(int dev_fd, int right)
{
    __u8 conf = 0x10;

    if (right)
        conf |= 0x04;
    vma450_send(dev_fd, VMA450_CMD, conf);
}

void vma450_shift_display(int dev_fd, int right)
{
    __u8 conf = 0x18;

    if (right)
        conf |= 0x04;
    vma450_send(dev_fd, VMA450_CMD, conf);
}

int vma450_init(int dev_fd)
{
    pcf8574_init(dev_fd, 0x00);  // configure all outputs

    vma450_send_4bit(dev_fd, VMA450_CMD, 0x02);  // configure 4-bit operations

    /* the line below can be executed only after a Power-On-Reset */
    vma450_send(dev_fd, VMA450_CMD, 0x20 | VMA450_CONF_2LINES | VMA450_CONF_5x11);  // set num of lines and font

    vma450_display_set(dev_fd, 1, 1, 0);

    vma450_mode_set(dev_fd, 1, 0); 


    return 0;
}

void vma450_write_str_2lines(int dev_fd, const char * str)
{
    int len, i = 0, j = 0;
    char buffer[80];

    memset(buffer, 0, 80);
    len = strlen(str);
    
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

    vma450_clear(dev_fd);
    vma450_write_str(dev_fd, buffer);
}

void usage(const char *cmd)
{
    printf("Usage:\n");
    printf("\t%s [-h] [-i] [-w <str>] [-c]\n", cmd);
    printf("\t-h:\t\tshows the help and exit\n");
    printf("\t-i:\t\tinitializes the chip. Works only once per reset.\n");
    printf("\t-w <str>:\twrites <str> on the display.\n");
    printf("\t-c:\t\tclears the display.\n");
}

int main(int argc, char ** argv)
{
    int dev_fd, opt;

    dev_fd = vma450_probe(0x27);
    if (dev_fd < 0)
        printf("[ERROR] vma450 not found.\n");
    else {
        while ((opt = getopt(argc, argv, "hiw:c")) != -1) {
            switch (opt) {
                case 'h':
                    usage(argv[0]);
                    return 0;
                    break;
                case 'i':
                    vma450_init(dev_fd);
                    break;
                case 'w':
                    vma450_display_set(dev_fd, 0, 1, 0);
                    vma450_write_str_2lines(dev_fd, optarg);
                    vma450_display_set(dev_fd, 1, 1, 0);
                    break;
                case 'c':
                    vma450_clear(dev_fd);
                    break;
                default:
                    usage(argv[0]);
             }
        }
        close(dev_fd);
    }
    return 0;
}
