extern "C" {
#include "zmxbus.h"
}

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include "bcm2835.h"

#define GPIO_DATA_MASK 0xFFFF
#define GPIO_RELEASE 19
#define GPIO_WAIT 18
#define GPIO_CS 17
#define GPIO_CLK 16
#define GPIO_DATA_START 0
#define GPIO_DATA_END 7

#define GPSEL0 0
#define GPSEL1 1
#define GPSET0 7
#define GPCLR0 10
#define GPLEV0 13

// Command definitions
// #define CMD_MEM_READ    0x00
// #define CMD_MEM_WRITE   0x01
// #define CMD_IO_READ     0x02
// #define CMD_IO_WRITE    0x03
#define CMD_RESET       0x05
#define CMD_STATUS      0x08

static volatile uint32_t *gpio;

#define PCLK (1 << GPIO_CLK)
#define CS (1 << GPIO_CS)
#define RELEASE (1 << GPIO_RELEASE)
#define CLR0(a) gpio[GPCLR0] = a
#define SET0(a) gpio[GPSET0] = a
#define SEL0(a) gpio[GPSEL0] = a 
#define SEL1(a) gpio[GPSEL1] = a 
#define PULSE0(a) gpio[GPCLR0] = a; gpio[GPSET0] = a
#define LEV0() gpio[GPLEV0]

void gpio_set_value8(uint8_t value) {
    CLR0(GPIO_DATA_MASK);
    SET0(value | RELEASE | PCLK);
    PULSE0(PCLK);
}

void gpio_set_value8_delay(uint8_t value, int ms) {
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
    ret = LEV0();
    SET0(PCLK);
    return ret;
}

uint8_t gpio_get_data8(void) {
    uint32_t ret;
    int tries = 2;
    // CLR0(RELEASE | GPIO_DATA_MASK);
    do { PULSE0(PCLK); ret = LEV0(); } while(!(ret & 1 << GPIO_WAIT));
    PULSE0(PCLK);
    ret = LEV0();
    return ret;
}

void gpio_set_data8(uint8_t value) {
    int tries = 3;
    CLR0(GPIO_DATA_MASK | RELEASE);
    SET0(value);
    // do { PULSE0(PCLK); } while(!(LEV0() & 1 << GPIO_WAIT));
    PULSE0(PCLK);
    // while(!(LEV0() & 1 << GPIO_WAIT) || tries--);
    return;
}

extern "C" {
    EXPORT unsigned char msxread(int cmd, unsigned short addr);
    EXPORT void reset(int ms) 
    {
        CLR0(CS);
        gpio_set_value8_delay(CMD_RESET, ms);
        SET0(CS);
        msxread(0,0);
    }
    
    // Add at the top with other defines
    #define GPFSEL2 2
    #define CM_GP0CTL 0x70
    #define CM_GP0DIV 0x74
    #define GPCLK0 20
    // Modify init function
    EXPORT int init(char *path) 
    {
        if (!bcm2835_init()) return -1;
        gpio = bcm2835_regbase(BCM2835_REGBASE_GPIO);
        volatile uint32_t *cm = bcm2835_regbase(BCM2835_REGBASE_CLK);
        
        // Rest of the initialization
        SEL0(0x09249249);
        SEL1(0x09249249);
        // Set GPIO20 to ALT5 (GPCLK0)
        bcm2835_gpio_fsel(GPCLK0, BCM2835_GPIO_FSEL_ALT5);
        // uint32_t fsel = gpio[GPFSEL2];
        // fsel &= ~(7 << 0);  // Clear bits for GPIO20
        // fsel |= (2 << 0);   // Set ALT5 function
        // gpio[GPFSEL2] = fsel;
        printf("gpio:%x\n", gpio);
        if (cm != MAP_FAILED)
        {
            printf("cm:%x\n", cm);
            // Configure GPCLK0 for 3.56MHz (500MHz / 140 ? 3.57MHz)
            cm[CM_GP0CTL/4] = 0x5A000000;  // Stop GPCLK0
            usleep(10);
            cm[CM_GP0DIV/4] = 0x5A000000 | (140 << 12);  // Set divisor
            cm[CM_GP0CTL/4] = 0x5A000005;  // Start GPCLK0 with PLLD source
        }
        
        CLR0(CS | PCLK | RELEASE | GPIO_DATA_MASK);
        SET0(CS | PCLK);
        return 0;
    }
    
    EXPORT unsigned char msxread(int cmd, unsigned short addr) 
    {
        uint8_t data = 0;
        CLR0(CS | 1 << GPIO_RELEASE);
        // Send command
        gpio_set_value8(cmd);
        // Send low address byte
        gpio_set_value8(addr);
        // Send high address byte
        gpio_set_value8(addr >> 8);
        // Wait for acknowledgment (0xFF)
        data = gpio_get_data8();
        // Deassert CS
        SET0(CS | PCLK);
        return data;
    }
    
    EXPORT void msxwrite(int cmd, unsigned short addr, unsigned char value)
    {
        // Assert CS
        CLR0(CS | 1 << GPIO_RELEASE);
        // Send command
        gpio_set_value8(cmd);
        // // // Send data
        gpio_set_value8(value);
        // // // Send low address byte
        gpio_set_value8(addr);
        // // Send high address byte
        gpio_set_value8(addr >> 8);
        do { PULSE0(PCLK); } while(!(LEV0() & 1 << GPIO_WAIT));
        // PULSE0(PCLK);
        // Send data
        SET0(CS | PCLK);
        return;
    }
    
    EXPORT unsigned char msxstatus()
    {
        SET0(CS | PCLK);
        CLR0(CS);
        gpio_set_value8(CMD_STATUS);
        uint8_t status = gpio_get_value8();
        SET0(CS);
        return status;    
    }
}

