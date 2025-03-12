obj-m := msxbus.o
KDIR := /lib/modules/$(shell uname -r)/build
DOCKER := sudo docker

all: compile insmod

compile:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

insmod:
	sudo rmmod msxbus
	sudo insmod msxbus.ko

ioctl:
	cc -o test_ioctl msxbus_ioctl_test.c

gpio:
	cc -o test_gpio msxbus_gpio_test.c

gpio8:
	cc -o test_gpio8 msxbus_gpio8_test.c ../lib/libbcm2835.a

spi:
	cc -o test_spi msxbus_spi_test.c

rpi:
	# Build the Docker image
	$(DOCKER) build -t rpi-kernel-module .
	# Run the Docker container and compile the module
	$(DOCKER) run --rm -v $(PWD):/workdir rpi-kernel-module bash -c "make -C /usr/src/linux M=/workdir modules"

clean:
	rm -f *.ko *.mod *.o *.mod.c .*.cmd Module.symvers modules.order test_*

