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

#define GPIO_DATA_MASK16 0xFFFF
#define GPIO_DATA_MASK8 0xFF
#define GPIO_CS 17
#define GPIO_CLK 16
#define GPIO_DATA_START 0
#define GPIO_DATA_END 15

#define GPSEL0 0
#define GPSEL1 1
#define GPSET0 7
#define GPCLR0 10
#define GPLEV0 13

// Command definitions
#define CMD_MEM_READ    0x00
#define CMD_MEM_WRITE   0x01
#define CMD_IO_READ     0x02
#define CMD_IO_WRITE    0x03
#define CMD_RESET       0x05
#define CMD_STATUS      0x08
#define WAIT            (1 << 15)

#define OUTPUT_DIR0 0x09249249
#define OUTPUT_DIR1 0x09249249
#define INPUT_DIR0  0x00000000
#define INPUT_DIR1  0x00048000

static volatile uint32_t *gpio;

#define GPIO_CLK_0 *(gpio + GPCLR0) = 1 << GPIO_CLK
#define GPIO_CLK_1 *(gpio + GPSET0) = 1 << GPIO_CLK
#define GPIO_CS_0  *(gpio + GPCLR0) = 1 << GPIO_CS
#define GPIO_CS_1  *(gpio + GPSET0) = 1 << GPIO_CS

#define udelay usleep

void gpio_set_value8(uint8_t value) {
    // Set GPIO 0-7 to output
    *(gpio + GPSEL0) = OUTPUT_DIR0;
    *(gpio + GPCLR0) = GPIO_DATA_MASK8;
    *(gpio + GPSET0) = value;
    *(gpio + GPCLR0) = 1 << GPIO_CLK;
    *(gpio + GPSET0) = 1 << GPIO_CLK;
}

void gpio_set_value16(uint16_t value) {
    // Set GPIO 0-15 to output
    // *(gpio + GPSEL0) = OUTPUT_DIR0;
    // *(gpio + GPSEL1) = OUTPUT_DIR1;
    *(gpio + GPCLR0) = GPIO_DATA_MASK16;
    *(gpio + GPSET0) = value;
    *(gpio + GPCLR0) = 1 << GPIO_CLK;
    *(gpio + GPSET0) = 1 << GPIO_CLK;
    printf("w0x%04x\n", value);
}

uint8_t gpio_get_value8(void) {
    // Set GPIO 0-7 to input
    uint8_t ret;
    *(gpio + GPSEL0) = INPUT_DIR0;
    *(gpio + GPCLR0) = 1 << GPIO_CLK;
    *(gpio + GPSEL0) = 0x09200000;
    ret = (uint8_t)(*(gpio + GPLEV0) & GPIO_DATA_MASK8);
    *(gpio + GPSET0) = 1 << GPIO_CLK;
    return ret;
}

uint16_t gpio_get_value16(void) {
    // Set GPIO 0-15 to input
    uint16_t ret;
    *(gpio + GPSET0) = 1 << GPIO_CLK;
    *(gpio + GPCLR0) = 1 << GPIO_CLK;
    *(gpio + GPSEL0) = INPUT_DIR0;
    *(gpio + GPSEL1) = INPUT_DIR1;
    ret = (uint16_t)(*(gpio + GPLEV0));
    *(gpio + GPSET0) = 1 << GPIO_CLK;
    printf("r0x%04x\n", ret);
    return ret;
}


static void gpio_bit_bang(uint8_t cmd, uint16_t addr, uint8_t data, uint8_t *read_data) {
    uint8_t value;
    uint8_t retry = 0xff;

    GPIO_CS_0;
    GPIO_CLK_1;

    // Send address
    gpio_set_value16(addr);
    // Send command and data
    gpio_set_value16(cmd << 8 | data);
    if (cmd <= CMD_IO_WRITE) {
        do {
            value = gpio_get_value16();
            if (value & WAIT)
                break;
        } while(retry--);
    } else if (cmd == CMD_STATUS) {
        value = gpio_get_value16();
    }
    GPIO_CS_1;
    if (read_data)
        *read_data = value;
}

void msxbus_reset(int value) {
    // Assert CS
    GPIO_PCLK_1;
    GPIO_CS_0;
    // Send command
    gpio_set_value16(0);
    // Send command and data
    gpio_set_value16((CMD_RESET | (value ? 1 : 0) << 4 ) << 8);

    GPIO_CS_1;
}

uint8_t msxbus_mem_read(uint16_t addr) {
    uint8_t data = 0;
    uint16_t value = 0;
    int retry = 5;
    uint8_t cmd = CMD_MEM_READ;
    // Assert CS
    GPIO_PCLK_1;
    GPIO_CS_0;

    // Send command
    gpio_set_value16(addr);
    // Send command and data
    gpio_set_value16(cmd << 8);
    do {
        value = gpio_get_value16();
        printf("r0x%04x\n", value);
        // //printf("%04x,", value);
        // if (value & WAIT)
        //     break;
    } while(retry--);   
    GPIO_CS_1; 
    return (uint8_t) value;
}

void msxbus_mem_write(uint16_t addr, uint8_t data) {
    uint16_t value = 0;
    int retry = 5;
    uint8_t cmd = CMD_MEM_READ;
    // Assert CS
    GPIO_CS_0;

    // Send command
    gpio_set_value16(addr);
    // Send command and data
    gpio_set_value16(cmd << 8 | data);
    do {
        value = gpio_get_value16();
        if (value & WAIT)
            break;
    } while(retry--); 
    GPIO_CS_1; 
    return;
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
    *(gpio + GPSEL0) = OUTPUT_DIR0;  // Set GPIO 0-9 to output
    *(gpio + GPSEL1) = OUTPUT_DIR1;  // Set GPIO 10-17 to output
    *(gpio + GPSET0) = 1 << GPIO_CLK | 1 << GPIO_CS;

    // msxbus_reset(0);
    // msxbus_reset(1);

    // Read memory from 0x4000 to 0xBFFF
    for (uint16_t addr = 0x4000; addr < 0x4100; addr += 16) {
        for (int i = 0; i < 16; i++) {
            buffer[i] = msxbus_mem_read(addr + i);
        }
        print_memory_dump(addr, buffer, 16);
    }

    munmap((void*)gpio, BLOCK_SIZE);
    close(mem_fd);
    return 0;
}
