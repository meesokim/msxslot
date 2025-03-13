#!/bin/bash

# Set CPU3 to performance mode
echo performance > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor

# Set real-time priority and CPU affinity for your application
# Replace /path/to/your/app with your actual application path
chrt -f 99 taskset -c 3 /path/to/your/app &