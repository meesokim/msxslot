#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define CMD_MEM_READ    0x00
#define CMD_MEM_WRITE   0x01
#define CMD_IO_READ     0x02
#define CMD_IO_WRITE    0x03
#define CMD_RESET       0x05
#define CMD_STATUS      0x08

void print_memory_dump(uint16_t addr, uint8_t *data, int len) {
    printf("%04X: ", addr);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

static int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, int len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = 0,
        .speed_hz = 20000000,  // Increased to 20MHz
        .bits_per_word = 8,
    };

    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

static uint8_t msx_mem_read(int fd, uint16_t addr) {
    uint8_t tx[10] = {CMD_MEM_READ, addr & 0xFF, (addr >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0};
    uint8_t rx[10] = {0};
    int i;
    
    if (spi_transfer(fd, tx, rx, 10) < 0) {
        perror("SPI transfer failed");
        return 0xFF;
    }
    
    // Find ACK (0xFF) and return the next byte
    for (i = 3; i < 9; i++) {
        if (rx[i] == 0xFF) {
            return rx[i + 1];
        }
    }
    
    return 0xFF;  // Return 0xFF if no valid data found
}

static uint8_t msx_get_status(int fd) {
    uint8_t tx[10] = {CMD_STATUS, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t rx[10] = {0};
    
    if (spi_transfer(fd, tx, rx, 10) < 0) {
        perror("SPI transfer failed");
        return 0xFF;
    }
    
    return rx[1];  // Status command returns data immediately
}

int main() {
    int fd;
    uint8_t buffer[16];
    uint8_t mode = 0;
    uint8_t bits = 8;
    uint32_t speed = 20000000;  // Set to 20MHz

    fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }

    // SPI mode
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set SPI mode");
        return 1;
    }

    // Bits per word
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("Failed to set bits per word");
        return 1;
    }

    // Max speed Hz
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Failed to set max speed hz");
        return 1;
    }

    // Read memory from 0x4000 to 0xBFFF
    for (uint16_t addr = 0x4000; addr < 0xC000; addr += 16) {
        // Read 16 bytes
        for (int i = 0; i < 16; i++) {
            buffer[i] = msx_mem_read(fd, addr + i);
        }
        print_memory_dump(addr, buffer, 16);
    }

    // Get status
    uint8_t status = msx_get_status(fd);
    printf("\nStatus: 0x%02X\n", status);

    close(fd);
    return 0;
}