KERNELDIR=/opt/EmbedSky/linux-2.6.30.4
PWD:=$(shell pwd)
INSTALLDIR=/opt/EmbedSky/lcd
CC=arm-linux-gcc
obj-m :=gsm.o 
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
modules_install:
	cp gsm.ko $(INSTALLDIR)
clean:
	rm -rf *.o *.ko *.mod.c *.markers *.order *.symvers
.PHONY:modules modules_install clean

