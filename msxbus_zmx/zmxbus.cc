extern "C" {
#include "zmxbus.h"
}

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

// Same definitions as in kernel module
#define MSXBUS_IOC_MAGIC 'M'
#define MSXBUS_IOCREAD    _IOWR(MSXBUS_IOC_MAGIC, 0, struct msxbus_transfer)
#define MSXBUS_IOCWRITE   _IOW(MSXBUS_IOC_MAGIC, 1, struct msxbus_transfer)
#define MSXBUS_IOCRESET      _IO(MSXBUS_IOC_MAGIC, 5)
#define MSXBUS_IOCSTATUS     _IOR(MSXBUS_IOC_MAGIC, 8, uint8_t)

struct msxbus_transfer {
    uint8_t cmd;
    uint16_t addr;
    uint8_t data;
};

#define CMD_RESET 5
static int fd;
extern "C" {
void reset(char b) 
{
    struct msxbus_transfer xfer;
    xfer.cmd = CMD_RESET | (b > 0) << 4;
    if (ioctl(fd, MSXBUS_IOCRESET, &xfer) < 0) {
        perror("ioctl failed");
        close(fd);
    }
}

void init(char *path) 
{
    printf("init(%s)\n", path);
    fd = open("/dev/msxbus", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
    }
    reset(0);
    usleep(100);
    reset(1);
}

unsigned char msxread(int cmd, unsigned short addr) 
{
    struct msxbus_transfer xfer;
    xfer.cmd = cmd;
    xfer.addr = addr;
    if (ioctl(fd, MSXBUS_IOCREAD, &xfer) < 0) {
        perror("ioctl failed");
        close(fd);
        return 1;
    }
    return xfer.data;
}

void msxwrite(int cmd, unsigned short addr, unsigned char value)
{
    struct msxbus_transfer xfer;
    xfer.cmd = cmd;
    xfer.addr = addr;
    xfer.data = value;
    if (ioctl(fd, MSXBUS_IOCWRITE, &xfer) < 0) {
        perror("ioctl failed");
        close(fd);
    }
    return;
}

unsigned char msxstatus()
{
    struct msxbus_transfer xfer;
    unsigned char status;
    if (ioctl(fd, MSXBUS_IOCSTATUS, &status) < 0) {
        perror("ioctl failed");
        close(fd);
        return 0;
    }
    return status;    
}
}
