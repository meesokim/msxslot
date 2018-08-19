//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013
 
 
// Access from ARM Running Linux 
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <bcm2835.h>

#include "rpi-gpio.h"
#include "rpmpv3.h"
 
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
 
int  mem_fd;
void *gpio_map;
int setup_io();
// I/O access
volatile unsigned *gpio; 

static unsigned char ROM[] = {
#include "Antarctic.data"
//#include "Gradius.data"
//#include "Zemix30.data"
//#include "game126.data"
};	

int GetAddress()
{
	int addr;
	GPIO_CLR(HIADDR);
	addr = (gpio[GPIO_GPLEV0] & 0xff00);
	GPIO_SET(HIADDR);
	addr |= (gpio[GPIO_GPLEV0] >> 8) & 0xff;
	return addr;
}	
  
int main(int argc, char **argv)
{
	// Set up gpi pointer for direct register access
	
	int signal;
	unsigned short addr;
	unsigned char a;
	unsigned char b = 0;
	unsigned char byte;
	setup_io();

	gpio[GPIO_GPFSEL0] = 0x249249;
	gpio[GPIO_GPFSEL1] = 0x249249;
	gpio[GPIO_GPFSEL2] = 0x9240;
	
	GPIO_CLR(0xffffffff);
	GPIO_SET(INT | WAIT | HIADDR | DAT_EN);

	while(1) 
	{
        signal = gpio[GPIO_GPLEV0] & (SLTSL | MREQ | IORQ | RD | WR);
		if (signal != (SLTSL | MREQ | IORQ | RD | WR))
		{
            if (!(signal & (RD | SLTSL)))
            {
                addr = GetAddress() - 0x4000;
                if (addr >= 0 && addr < sizeof(ROM))
                {
                    GPIO_SET(ROM[addr]| DAT_DIR);
                    while(!(gpio[GPIO_GPLEV0] & SLTSL));
					GPIO_CLR(DAT_DIR);
					printf("%04x(%02x),%c%c%c%c\n", addr + 0x4000, gpio[GPIO_GPLEV0] & 0xff, (signal & IORQ) > 0 ? 'M' : 'I', (signal & SLTSL) > 0 ? 'A' : 'S', (signal & RD) > 0 ? 'W':'R', (signal & RESET) > 0 ? 'R' : 'r');   continue;
                }
            }
#if 0	
            if (!(signal & (RD | MREQ)))
            {
				addr = GetAddress();
                if (addr == 0xc000)
                {
					GPIO_CLR(0xff| DAT_DIR);
                    GPIO_SET(a++); 
                    while(!(gpio[GPIO_GPLEV0] & MREQ));
                    GPIO_SET(DAT_DIR);                    
					printf("%04x(%02x),%c%c%c%c\n", addr, a, (signal & IORQ) > 0 ? 'M' : 'I', (signal & SLTSL) > 0 ? 'A' : 'S', (signal & RD) > 0 ? 'W':'R', (signal & RESET) > 0 ? 'R' : 'r');            
                    continue;
                }
                if (addr == 0xc001)
                {
                    GPIO_SET(0);
                    GPIO_SET(0);
                    GPIO_SET(0);
                    byte =  gpio[GPIO_GPLEV0];
//                    printf("%04x(%02x),%c%c%c%c%c\n", (signal & IORQ) ? signal & 0xffff : signal & 0xff, byte, (signal & ATN) ? '!' : 'x', (signal & IORQ) > 0 ? 'M' : 'I', (signal & SLTSL) > 0 ? 'A' : 'S', (signal & RD) > 0 ? 'W':'R', (signal & RESET) > 0 ? 'R' : 'r');            }
                }
            }
#endif			
            if (!(signal & (IORQ | RD)))
            {
				GPIO_CLR(HIADDR | 0xff);
				addr = (gpio[GPIO_GPLEV0] >> 8) & 0xff;
                if (addr == 100)
                {
					GPIO_CLR(0xff);
					GPIO_SET(++b | DAT_DIR);                    
                    while(!(gpio[GPIO_GPLEV0] & IORQ));
					GPIO_CLR(DAT_DIR | 0xff);
					if (b > 99)
						b = 0;
                }
                continue;
            } 
		}			
	}
	return 0;
 
} // main
 
 
//
// Set up a memory regions to access GPIO
//
int setup_io()
{
	int i ;	
	if (!bcm2835_init()) return -1;
	gpio = bcm2835_regbase(BCM2835_REGBASE_GPIO);
	for(int i=0; i < 28; i++)
	{
		bcm2835_gpio_fsel(i, i < 21 ? 1 : 0);    
	}
	return 0;
} // setup_io

