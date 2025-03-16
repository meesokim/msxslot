obj-m := msxbus_fiq.o
DOCKER := sudo docker

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo cp msxbus-fiq.conf /etc/modules-load.d/
	
rpi:
	# Build the Docker image
	$(DOCKER) build --build-arg https_proxy=$(https_proxy) --build-arg http_proxy=$(http_proxy) -t rpi-kernel-module .
	# Run the Docker container and compile the module
	$(DOCKER) run --rm -v $(PWD):/workdir rpi-kernel-module bash -c "make -C /usr/src/linux M=/workdir modules"

clean:
	# make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f .*.cmd *.symvers *.mod *.mod.c *.o *.ko
