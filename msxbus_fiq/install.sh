#!/bin/bash
make
sudo cp msxbus_fiq.ko /lib/modules/$(uname -r)/kernel/drivers/gpio/
sudo depmod -a