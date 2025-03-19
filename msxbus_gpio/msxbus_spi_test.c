#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bcm2835.h"

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

static inline uint8_t spi_transfer_byte(uint8_t data) {
    return bcm2835_spi_transfer(data);
}

static uint8_t msx_mem_read(uint16_t addr) {
    uint8_t rx;
    int i;
    
    // Send command and address
    spi_transfer_byte(CMD_MEM_READ);
    spi_transfer_byte(addr & 0xFF);
    spi_transfer_byte((addr >> 8) & 0xFF);
    
    // Send dummy bytes until we get 0xFF (ACK)
    for (i = 0; i < 6; i++) {
        rx = spi_transfer_byte(0x00);
        if (rx == 0xFF) {
            // Next byte is our data
            return spi_transfer_byte(0x00);
        }
    }
    
    return 0xFF;  // Return 0xFF if no valid data found
}

static void msx_mem_write(uint16_t addr, uint8_t data) {
    uint8_t rx;
    int i;
    
    // Send command, address and data
    spi_transfer_byte(CMD_MEM_WRITE);
    spi_transfer_byte(addr & 0xFF);
    spi_transfer_byte((addr >> 8) & 0xFF);
    spi_transfer_byte(data);
    
    // Send dummy bytes until we get 0xFF (ACK)
    for (i = 0; i < 6; i++) {
        rx = spi_transfer_byte(0x00);
        if (rx == 0xFF) {
            return;  // Write complete
        }
    }
}

static uint8_t msx_get_status(void) {
    uint8_t status;
    
    spi_transfer_byte(CMD_STATUS);
    status = spi_transfer_byte(0x00);  // Status returns immediately
    
    return status;
}

int main() {
    uint8_t buffer[16];

    // SPI settings
    uint32_t core_freq = 250000000;
    uint32_t target_freq = 62500000;  // Target 62.5MHz
    uint32_t divider = (core_freq + target_freq - 1) / target_freq;
    
    // Ensure divider is power of 2 and at least 2
    divider = divider < 2 ? 2 : divider;
    divider = 1 << (32 - __builtin_clz(divider - 1));
    
    printf("Core frequency (from register): %u Hz\n", core_freq);
    printf("SPI frequency: %u Hz (div=%d)\n", core_freq / divider, divider);

    if (!bcm2835_init()) {
        printf("bcm2835_init failed\n");
        return 1;
    }

    if (!bcm2835_spi_begin()) {
        printf("bcm2835_spi_begin failed\n");
        return 1;
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(divider);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);

    // Read memory from 0x4000 to 0xBFFF
    for (uint16_t addr = 0x4000; addr < 0xC000; addr += 16) {
        // Read 16 bytes
        for (int i = 0; i < 16; i++) {
            buffer[i] = msx_mem_read(addr + i);
        }
        print_memory_dump(addr, buffer, 16);
    }

    // Get status
    uint8_t status = msx_get_status();
    printf("\nStatus: 0x%02X\n", status);

    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}