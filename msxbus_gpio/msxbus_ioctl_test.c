#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

// Same definitions as in kernel module
#define MSXBUS_IOC_MAGIC 'M'
#define MSXBUS_IOCMEMREAD    _IOWR(MSXBUS_IOC_MAGIC, 0, struct msxbus_transfer)
#define MSXBUS_IOCMEMWRITE   _IOW(MSXBUS_IOC_MAGIC, 1, struct msxbus_transfer)
#define MSXBUS_IOCIOREAD     _IOWR(MSXBUS_IOC_MAGIC, 2, struct msxbus_transfer)
#define MSXBUS_IOCIOWRITE    _IOW(MSXBUS_IOC_MAGIC, 3, struct msxbus_transfer)
#define MSXBUS_IOCRESET      _IO(MSXBUS_IOC_MAGIC, 5)
#define MSXBUS_IOCSTATUS     _IOR(MSXBUS_IOC_MAGIC, 8, uint8_t)

struct msxbus_transfer {
    uint16_t addr;
    uint8_t data;
};

void print_memory_dump(uint16_t addr, uint8_t *data, int len) {
    printf("%04X: ", addr);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    int fd = open("/dev/msxbus", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    struct msxbus_transfer xfer;
    uint8_t buffer[16];

    // Read memory from 0x4000 to 0xBFFF
    for (uint16_t addr = 0x4000; addr < 0xC000; addr += 16) {
        // Read 16 bytes
        for (int i = 0; i < 16; i++) {
            xfer.addr = addr + i;
            if (ioctl(fd, MSXBUS_IOCMEMREAD, &xfer) < 0) {
                perror("ioctl failed");
                close(fd);
                return 1;
            }
            buffer[i] = xfer.data;
        }
        print_memory_dump(addr, buffer, 16);
    }

    // Get status
    uint8_t status;
    if (ioctl(fd, MSXBUS_IOCSTATUS, &status) < 0) {
        perror("ioctl failed");
        return 1;
    }
    printf("\nStatus: 0x%02X\n", status);

    close(fd);
    return 0;
}