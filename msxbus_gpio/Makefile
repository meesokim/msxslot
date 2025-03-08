obj-m := msxbus.o
KDIR := /lib/modules/$(shell uname -r)/build
DOCKER := sudo docker

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

ioctl:
	cc -o test_ioctl msxbus_ioctl_test.c

rpi:
	# Build the Docker image
	$(DOCKER) build -t rpi-kernel-module .
	# Run the Docker container and compile the module
	$(DOCKER) run --rm -v $(PWD):/workdir rpi-kernel-module bash -c "make -C /usr/src/linux M=/workdir modules"

clean:
	rm -f *.ko *.mod *.o *.mod.c .*.cmd Module.symvers modules.order

