# Use an official ARMv7 cross-compilation image
FROM multiarch/crossbuild

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    libncurses5-dev \
    bc \
    bison \
    flex \
    libssl-dev \
    wget \
    git \
    && apt-get clean

# Set environment variables for cross-compilation
ENV ARCH=arm
ENV CROSS_COMPILE=arm-linux-gnueabihf-

# Download and extract the Raspberry Pi kernel source
RUN mkdir -p /usr/src/kernel
WORKDIR /usr/src/kernel
RUN wget https://github.com/raspberrypi/linux/archive/refs/tags/raspberrypi-kernel_1.20211029-1.tar.gz \
    && tar -xzf raspberrypi-kernel_1.20211029-1.tar.gz --strip-components=1 \
    && rm raspberrypi-kernel_1.20211029-1.tar.gz

# Copy the module source code into the container
COPY msxbus.c /usr/src/kernel/msxbus.c
COPY Makefile /usr/src/kernel/Makefile

# Compile the kernel module
RUN make modules_prepare
RUN make M=/usr/src/kernel

# Set the entrypoint to bash
ENTRYPOINT ["/bin/bash"]