# WPI450/VMA450 LCD Linux driver

This repo contains a Linux driver for the Whadda [20x4 LCD screen](https://whadda.com/product/i%c2%b2c-20x4-blue-lcd-module-wpi450/).
This LCD has 4 lines for a total of 80 characters.

In the _userspace_ folder, there is the source for a separate userspace driver.

## Loading

```
$> make
$> sudo insmode vma450.ko [intf_init=1]
```

The optional parameter _intf_init_ tells the driver whether to reinitialize the device.
The initialization must happen only *once* after reboot (or Power-on Reset).

## Use

The driver creates a character device called _/dev/vma450lcd_.
This file cannot be read, but you can write to it and the text will appear on the screen.

```
$> sudo chmod 777 /dev/vma450lcd
$> echo hello world > /dev/vma450lcd
```

The driver exposes also a sysfs interface, in _/sys/kernel/vma450/_
As of now, the only implemented control is on the display of the text:

```
$> sudo chmod 777 /sys/kernel/vma450/display_on
$> echo 0 > /sys/kernel/vma450/display_on  # turns it off
$> echo 1 > /sys/kernel/vma450/display_on  # turns it on
```
