#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define DEVICE "/dev/msxbus"

int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    for (uint16_t addr = 0; addr < 0xFFFF; addr += 16) {
        uint8_t buffer[2];
        uint8_t data;

        buffer[0] = addr & 0xFF;
        buffer[1] = (addr >> 8) & 0xFF;

        if (write(fd, buffer, 2) != 2) {
            perror("Failed to write address");
            close(fd);
            return 1;
        }

        if (read(fd, &data, 1) != 1) {
            perror("Failed to read data");
            close(fd);
            return 1;
        }

        printf("Address: 0x%04X, Data: 0x%02X\n", addr, data);
    }

    close(fd);
    return 0;
}