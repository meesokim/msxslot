extern "C" {
#include "zmxbus.h"
#include <bcm2835.h>
#include <stdio.h>
#include <time.h>

#define GPIO_GPFSEL0    0
#define GPIO_GPFSEL1    1
#define GPIO_GPFSEL2    2
#define GPIO_GPFSEL3    3
#define GPIO_GPFSEL4    4
#define GPIO_GPFSEL5    5

#define GPIO_GPSET0     7
#define GPIO_GPSET1     8

#define GPIO_GPCLR0     10
#define GPIO_GPCLR1     11

#define GPIO_GPLEV0     13
#define GPIO_GPLEV1     14

#define GPIO_GPEDS0     16
#define GPIO_GPEDS1     17

#define GPIO_GPREN0     19
#define GPIO_GPREN1     20

#define GPIO_GPFEN0     22
#define GPIO_GPFEN1     23

#define GPIO_GPHEN0     25
#define GPIO_GPHEN1     26

#define GPIO_GPLEN0     28
#define GPIO_GPLEN1     29

#define GPIO_GPAREN0    31
#define GPIO_GPAREN1    32

#define GPIO_GPAFEN0    34
#define GPIO_GPAFEN1    35

#define GPIO_GPPUD      37
#define GPIO_GPPUDCLK0  38
#define GPIO_GPPUDCLK1  39

#define RD0		0
#define RD1		1
#define RD2		2
#define RD3		3
#define RD4		4
#define RD5		5
#define RD6		6
#define RD7		7
#define RA8		8
#define RA9		9
#define RA10	10
#define RA11	11
#define RA12	12
#define RA13	13
#define RA14	14
#define RA15	15
#define RC16	16
#define RC17	17
#define RC18	18
#define RC19	19
#define RC20	20
#define RC21	21
#define RC22	22
#define RC23	23
#define RC24	24
#define RC25	25
#define RC26	26
#define RC27	27

#define DATA_PIN 	0
#define MREQ        (1 << RA15)
#define IORQ        (1 << RA14)
#define WR          (1 << RA13)
#define RD          (1 << RA12)
#define CS1         (1 << RA11)
#define CS2         (1 << RA10)
#define SLTSL2      (1 << RA9)
#define SLTSL1      (1 << RA8)
#define LE_A		(1 << RC16)
#define LE_C 		(1 << RC17)
#define LE_D		(1 << RC18)
#define RESET   	(1 << RC19)
#define CLK			(1 << RC20) 
#define DAT_DIR		(1 << RC21)
#define INT			(1 << RC24)
#define WAIT		(1 << RC25)
#define SW1			(1 << RC27)

#define GPSEL0 0
#define GPSEL1 1
#define GPSET0 7
#define GPCLR0 10
#define GPLEV0 13

#define CLR0(a) gpio[GPCLR0] = a
#define SET0(a) gpio[GPSET0] = a
#define SEL0(a) gpio[GPSEL0] = a 
#define SEL1(a) gpio[GPSEL1] = a 
#define PULSE0(a) gpio[GPCLR0] = a; gpio[GPSET0] = a
#define LEV0() gpio[GPLEV0]

static volatile unsigned *gpio;

#include <pthread.h>

/* This is the critical section object (statically allocated). */
static pthread_mutex_t cs_mutex =  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

#define TRIES 6

void reset(int );
int dir[28] = { 1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1, 5,1,1,1,0,0,0,0 };
void init(char *path) 
{
    //printf("init(%s)\n", path);
	if (!bcm2835_init()) return;
	gpio = bcm2835_regbase(BCM2835_REGBASE_GPIO);
    // printf("gpio_base:%x(%d)\n", gpio,BCM2835_REGBASE_GPIO);    
	for(int i=0; i < 27; i++)
	{
		bcm2835_gpio_fsel(i, dir[i]);
        if (dir[i] < 2)
            bcm2835_gpio_set_pud(i, dir[i] ? BCM2835_GPIO_PUD_OFF : BCM2835_GPIO_PUD_UP);
	} 
    reset(10);
    SET0(0xff00 | LE_C);
    CLR0(LE_C);
    // req.tv_sec = 0;
    // req.tv_nsec = 1000;
    // // printf("%x\n", GPIO_GET());
}

inline void SetAddress(unsigned short addr) 
{
    // GPIO_SEL(0, 0x49249249);
    CLR0(0xffff);
    SET0(addr & 0xffff | LE_A);
    CLR0(0x00);
    SET0(00);
    CLR0(LE_A); 
    SET0(0xff00);
    //CLR0(0xff);
    SET0(LE_C);
    CLR0(0xff);
}

unsigned char msxread(int cmd, unsigned short addr) 
{
    if (addr > 0xc000) return 0xff;
    //pthread_mutex_lock( &cs_mutex );    
    unsigned char b = 0xff;
    SetAddress(addr);
    CLR0(RD | LE_D);
    switch(cmd) {
        case RD_SLTSL1:
            if (addr > 0xc000) return 0xff;
            CLR0(MREQ | (addr & 0x8000 ? CS2 : 0) | (addr & 0x4000 ? CS1 : 0) | SLTSL1);
            break;
        case RD_SLTSL2:
            if (addr > 0xc000) return 0xff;
            CLR0(MREQ | (addr & 0x8000 ? CS2 : 0) | (addr & 0x4000 ? CS1 : 0) | SLTSL2);
            break;
        case RD_MEM:
            CLR0(MREQ);
            break;
        case RD_IO:
            CLR0(IORQ);
        default:
            break;
    }           
    SET0(DAT_DIR);
    int tries = TRIES;
    while(!(LEV0() & WAIT) || tries-->0);
    b = LEV0();
    SET0(0xff00 | LE_D);
    CLR0(0); SET0(0); 
    CLR0(LE_C); 
    //pthread_mutex_unlock( &cs_mutex );
    return b;
}

void msxwrite(int cmd, unsigned short addr, unsigned char value)
{
    if (addr > 0xc000) return;
    __sync_synchronize();
    //pthread_mutex_lock( &cs_mutex );    
    SetAddress(addr);
    SET0(0xff00 | value);
    CLR0(DAT_DIR | LE_D);
    //CLR0(LE_D); SET0(value);
    //CLR0(0); SET0(0);
    //__sync_synchronize();
    switch(cmd) {
        case WR_SLTSL1:
            CLR0(MREQ | WR | (addr & 0x8000? CS2 : 0) | (addr & 0x4000? CS1 : 0) | SLTSL1);
            break;
        case WR_SLTSL2:
            CLR0(MREQ | WR | (addr & 0x8000? CS2 : 0) | (addr & 0x4000? CS1 : 0) | SLTSL2);
            break;
        case WR_MEM:
            CLR0(MREQ | WR) ;
            break;
        case WR_IO:
            CLR0(IORQ | WR);
            break;
        default:
            break;
    }    
    //SET0(0); CLR0(0); SET0(0); CLR0(0);
    int tries = TRIES;
    char b;
    while(!(LEV0() & WAIT) || tries-->0);
    SET0(0xff00);
    CLR0(0);
    CLR0(LE_C);
    //SET0(LE_D);
    // printf("WI:%02x,%02x\n", addr, value);
    //pthread_mutex_unlock( &cs_mutex );
}

void reset(int ms) 
{
    // SET0(LE_A | LE_C | LE_D | 0xffff);
    // CLR0(LE_A | LE_C);
    // SET0(RESET);
    // CLR0(RESET);
    // for (int i = 0; i < 1000 * ms; i++) SET0(0);
    // SET0(RESET);
    // SET0(LE_C | 0xff00);
    // CLR0(LE_C);     
    msxwrite(WR_IO, 0, 0);
    msxread(RD_SLTSL1, 0x4000);
}
}
