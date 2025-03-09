#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

// Change peripheral base address for Raspberry Pi 3 (BCM2837)
#define BCM2837_PERI_BASE 0x3F000000
#define GPIO_BASE (BCM2837_PERI_BASE + 0x200000)
#define PAGE_SIZE 4096
#define BLOCK_SIZE 4096

#define GPIO_DATA_MASK 0xFF
#define GPIO_CS 9
#define GPIO_CLK 8
#define GPIO_DATA_START 0
#define GPIO_DATA_END 7

#define GPSEL0 0x0
#define GPFSEL0 0x00
#define GPFSEL1 0x01
#define GPSET0 0x1c/4
#define GPCLR0 0x28/4
#define GPLEV0 0x34/4

// Command definitions
#define CMD_MEM_READ    0x00
#define CMD_MEM_WRITE   0x01
#define CMD_IO_READ     0x02
#define CMD_IO_WRITE    0x03
#define CMD_RESET       0x05
#define CMD_STATUS      0x08

static volatile uint32_t *gpio;

#define udelay usleep

// void gpio_set_value(int pin, int value) {
//     if (value)
//         *(gpio + GPSET0) = 1 << pin;
//     else
//         *(gpio + GPCLR0) = 1 << pin;
// }

void gpio_set_value8(uint8_t value) {
    // Set GPIO 0-7 to output
    *(gpio + GPSEL0) = 0x09249249;
    *(gpio + GPCLR0) = GPIO_DATA_MASK;
    *(gpio + GPSET0) = value;
    *(gpio + GPCLR0) = 1 << GPIO_CLK;
    *(gpio + GPSET0) = 1 << GPIO_CLK;
}

uint8_t gpio_get_value8(void) {
    // Set GPIO 0-7 to input
    uint8_t ret;
    *(gpio + GPSEL0) = 0x09200000;
    *(gpio + GPCLR0) = 1 << GPIO_CLK;
    ret = (uint8_t)(*(gpio + GPLEV0) & GPIO_DATA_MASK);
    *(gpio + GPSET0) = 1 << GPIO_CLK;
    return ret;
}

static void gpio_bit_bang(uint8_t cmd, uint16_t addr, uint8_t data, uint8_t *read_data) {
    uint8_t addr_low = addr & 0xFF;
    uint8_t addr_high = (addr >> 8) & 0xFF;
    uint8_t status;
    uint8_t retry = 0xff;

    // Assert CS
    *(gpio + GPCLR0) = 1 << GPIO_CS;

    // Send command
    gpio_set_value8(cmd);

    // For non-RESET and non-STATUS commands, send address
    switch (cmd & 0xf) {
        case CMD_MEM_READ:
        case CMD_IO_READ:
        case CMD_MEM_WRITE:
        case CMD_IO_WRITE:
            // Send low address byte
            gpio_set_value8(addr_low);
            // Send high address byte
            gpio_set_value8(addr_high);
            if (cmd & 0x01) {
                gpio_set_value8(data);
            } else {
                gpio_get_value8();                
            }
            // Wait for acknowledgment (0xFF)
            do {
                status = gpio_get_value8();
                if (status == 0xFF) {
                    if (!(cmd & 0x01)) {
                        *read_data = gpio_get_value8();
                    }
                }
            } while (retry--);
            break;
        case CMD_STATUS:
            *read_data = gpio_get_value8();
            break;
        default:
            break;
    }

    // Deassert CS
    *(gpio + GPSET0) = 1 << GPIO_CS | 1 << GPIO_CLK;
}


uint8_t msxbus_mem_read(uint16_t addr) {
    uint8_t data = 0, status;
    int retry = 255;
    uint8_t cmd = CMD_MEM_READ;
    // Assert CS
    *(gpio + GPCLR0) = 1 << GPIO_CS;

    // Send command
    gpio_set_value8(cmd);

    // For non-RESET and non-STATUS commands, send address
    switch (cmd) {
        case CMD_MEM_READ:
        case CMD_IO_READ:
            // Send low address byte
            gpio_set_value8(addr);
            // Send high address byte
            gpio_set_value8(addr >> 8);
            gpio_get_value8();                
            // Wait for acknowledgment (0xFF)
            do {
                status = gpio_get_value8();
                if (status == 0xFF) {
                    if (!(cmd & 0x01)) {
                        data = gpio_get_value8();
                    }
                }
            } while (retry--);
            break;
        case CMD_STATUS:
            data = gpio_get_value8();
            break;
        default:
            break;
    }

    // Deassert CS
    *(gpio + GPSET0) = 1 << GPIO_CS | 1 << GPIO_CLK;

    return data;
}

void print_memory_dump(uint16_t addr, uint8_t *data, int len) {
    printf("%04X: ", addr);
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

int main() {
    int mem_fd;
    uint8_t buffer[16];

    // Open /dev/mem
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
        printf("Failed to open /dev/mem\n");
        return -1;
    }

    // Map GPIO registers
    gpio = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE);
    if (gpio == MAP_FAILED) {
        printf("mmap failed\n");
        close(mem_fd);
        return -1;
    }
	             
    // Set GPIO 0-9 (data pins, CLK, CS) to output
    *(gpio + GPFSEL0) = 0x09249249;  // Set GPIO 0-7 to output
    *(gpio + GPSET0) = 1 << GPIO_CS | 1 << GPIO_CLK;

    // Read memory from 0x4000 to 0xBFFF
    for (uint16_t addr = 0x4000; addr < 0xC000; addr += 16) {
        for (int i = 0; i < 16; i++) {
            buffer[i] = msxbus_mem_read(addr + i);
        }
        print_memory_dump(addr, buffer, 16);
    }

    munmap((void*)gpio, BLOCK_SIZE);
    close(mem_fd);
    return 0;
}
