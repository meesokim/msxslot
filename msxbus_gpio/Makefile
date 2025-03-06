obj-m := msxbus.o
KDIR := /lib/modules/$(shell uname -r)/build

# Specify kernel source directory for cross compilation
KERNEL_SRC := /usr/src/linux
ARCH := arm
CROSS_COMPILE := arm-linux-gnueabihf-

# Add required kernel configurations
ccflags-y := -DCONFIG_GPIO_SYSFS -DCONFIG_GPIOLIB

DOCKER := sudo docker

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

rpi:
	# Build the Docker image
	$(DOCKER) build -t rpi-kernel-module .
	# Run the Docker container and compile the module
	$(DOCKER) run --rm -v $(PWD):/workdir rpi-kernel-module bash -c "make -C /usr/src/linux M=/workdir modules"

clean:
	rm -f *.ko *.mod *.o *.mod.c .*.cmd Module.symvers modules.order