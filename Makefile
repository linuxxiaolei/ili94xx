KERNELDIR=/opt/EmbedSky/linux-2.6.30.4
PWD:=$(shell pwd)
INSTALLDIR=/opt/EmbedSky/lcd
CC=arm-linux-gcc
obj-m :=ili9418.o 
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
modules_install:
	cp ili9418.ko $(INSTALLDIR)
clean:
	rm -rf *.o *.ko *.mod.c *.markers *.order *.symvers
.PHONY:modules modules_install clean

