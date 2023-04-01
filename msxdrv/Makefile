obj-m += msxdrv.o 
 
PWD := $(CURDIR) 

HOSTOS=$(shell uname -a | awk '{ print $2 }')

ifeq ($(findstring raspberrypi, $(HOSTOS)), raspberrypi)
	KERNEL=/lib/modules/$(shell uname -r)/build
	COMPILE=
else
	KERNEL=~/linux
	COMPILE=CROSS_COMPILE=arm-linux-gnueabihf-
endif
 
all: test
	make ARCH=arm $(COMPILE) -C $(KERNEL) M=$(PWD) modules
 
clean: 
	rm -f test
	make -C $(KERNEL) M=$(PWD) clean

test: msxbus_test.c
	$(CC) -o $@ $<

install: 
	sudo rmmod msxdrv
	sudo insmod msxdrv.ko

remove:
	sudo rmmod msxdrv