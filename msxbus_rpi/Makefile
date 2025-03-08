obj-m := msxbus_fiq.o 
msxbus_fiq-objs := msxbus_fiq_handler.o msxbus_fiq_core.o 
DOCKER := sudo docker
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

ioctl:
	cc -o test_ioctl msxbus_ioctl_test.c

gpio:
	cc -o test_gpio msxbus_gpio_test.c
rpi:
	# Build the Docker image
	$(DOCKER) build -t rpi-kernel-module .
	# Run the Docker container and compile the module
	$(DOCKER) run --rm -v $(PWD):/workdir rpi-kernel-module bash -c "make -C /usr/src/linux M=/workdir modules"

rpi-clean:
	# Run the Docker container and compile the module
	$(DOCKER) run --rm -v $(PWD):/workdir rpi-kernel-module bash -c "make -C /usr/src/linux M=/workdir clean"

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

