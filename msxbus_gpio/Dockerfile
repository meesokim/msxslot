FROM ubuntu:20.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive


# Configure apt to use proxy
RUN echo 'Acquire::http::Proxy "http://168.219.61.252:8080";' > /etc/apt/apt.conf.d/proxy.conf
RUN echo 'Acquire::https::Proxy "http://168.219.61.252:8080";' >> /etc/apt/apt.conf.d/proxy.conf

# Install required packages
RUN apt-get update && apt-get install -y \
    git \
    bc \
    bison \
    flex \
    libssl-dev \
    make \
    gcc \
    gcc-arm-linux-gnueabihf \
    binutils-arm-linux-gnueabihf \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN git config --global http.proxy "http://168.219.61.252:8080"
RUN git config --global http.sslVerify false

# Set up build environment
ENV ARCH=arm
ENV CROSS_COMPILE=arm-linux-gnueabihf-
ENV KERNEL_VERSION=1.20230405-1

# Get Raspberry Pi kernel source
WORKDIR /usr/src
RUN git clone --depth=1 -b rpi-5.15.y https://github.com/raspberrypi/linux.git
WORKDIR /usr/src/linux

# Get default config for Raspberry Pi
RUN make mrproper
RUN make bcm2711_defconfig
RUN make modules_prepare
RUN make scripts
RUN make prepare
RUN make modules -j 12  

# Create working directory for module compilation
WORKDIR /workdir
