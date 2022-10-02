KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

vma450cli:
	$(CC) -o vma450cli userspace/VMA450_userspace.c

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm -f vma450cli
