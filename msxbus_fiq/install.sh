#!/bin/bash
make
sudo cp msxbus_fiq.ko /lib/modules/$(uname -r)/kernel/drivers/gpio/
sudo depmod -a

# Make the script executable and copy to system location
sudo chmod +x cpu_setup.sh
sudo cp cpu_setup.sh /usr/local/bin/

# Copy and enable the service
sudo cp cpu-setup.service /etc/systemd/system/
sudo systemctl enable cpu-setup.service
sudo systemctl start cpu-setup.service