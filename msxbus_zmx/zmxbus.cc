extern "C" {
#include "zmxbus.h"
}

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "bcm2835.h"

#define GPIO_DATA_MASK 0xFF
#define GPIO_CS 9
#define GPIO_CLK 8
#define GPIO_DATA_START 0
#define GPIO_DATA_END 7

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

static volatile uint32_t *gpio;

#define PCLK (1 << GPIO_CLK)
#define CS (1 << GPIO_CS)
#define CLR0(a) gpio[GPCLR0] = a
#define SET0(a) gpio[GPSET0] = a
#define SEL0(a) gpio[GPSEL0] = a 
#define PULSE0(a) gpio[GPCLR0] = a; gpio[GPSET0] = a
#define LEV0() gpio[GPLEV0]

void gpio_set_value8(uint8_t value) {
    // Set GPIO 0-7 to output
    SEL0(0x09249249);
    CLR0(GPIO_DATA_MASK);
    SET0(value);
    PULSE0(PCLK);
}

void gpio_set_value8_delay(uint8_t value, int ms) {
    // Set GPIO 0-7 to output
    SEL0(0x09249249);
    CLR0(GPIO_DATA_MASK);
    SET0(value);
    CLR0(PCLK);
    usleep(ms);
    SET0(PCLK);
}

uint8_t gpio_get_value8(void) {
    // Set GPIO 0-7 to input
    uint8_t ret;
    CLR0(PCLK);
    SEL0(0x09200000);
    ret = LEV0();
    SET0(PCLK);
    return ret;
}

uint8_t gpio_get_data(void) {
    uint8_t ret;
    do {
        PULSE0(PCLK);
        ret = LEV0();
    }
    while(ret != 0xff);
    PULSE0(PCLK);
    return LEV0();
}

extern "C" {
    EXPORT void reset(int ms) 
    {
        SET0(CS | PCLK);
        CLR0(CS);
        gpio_set_value8_delay(CMD_RESET, ms);
        SET0(CS);
    }
    
    EXPORT int init(char *path) 
    {
        if (!bcm2835_init()) return -1;
        gpio = bcm2835_regbase(BCM2835_REGBASE_GPIO);
        reset(0);

	return 0;
    }
    
    EXPORT unsigned char msxread(int cmd, unsigned short addr) 
    {
        uint8_t data = 0, status;
        int retry = 255;
        SET0(CS | PCLK);
        // Assert CS
        CLR0(CS);
        // Send command
        gpio_set_value8(cmd);
        // Send low address byte
        gpio_set_value8(addr);
        // Send high address byte
        gpio_set_value8(addr >> 8);
        // Wait for acknowledgment (0xFF)
        data = gpio_get_data();
        // Deassert CS
        SET0(CS);
    
        return data;
    }
    
    EXPORT void msxwrite(int cmd, unsigned short addr, unsigned char value)
    {
        uint8_t status;
        int retry = 255;
        SET0(CS | PCLK);
        // Assert CS
        CLR0(CS);
        // Send command
        gpio_set_value8(cmd);
        // Send data
        gpio_set_value8(value);
        // Send low address byte
        gpio_set_value8(addr);
        // Send high address byte
        gpio_set_value8(addr >> 8);
        PULSE0(PCLK);
        SET0(CS);
    
        return;
    }
    
    unsigned char msxstatus()
    {
        SET0(CS | PCLK);
        CLR0(CS);
        gpio_set_value8(CMD_STATUS);
        uint8_t status = gpio_get_value8();
        SET0(CS);
        return status;    
    }
}

