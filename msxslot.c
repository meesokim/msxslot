//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013
 
 
// Access from ARM Running Linux
 
#define BCM2708_PERI_BASE        0x3F000000 // Raspberry Pi 2
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
  
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <bcm2835.h>
#include <time.h>
 
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
 
int  mem_fd;
void *gpio_map;
 
// I/O access
volatile unsigned *gpio;
 
 
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
 
#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
 
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
 
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

// MSX slot access macro
#define MSX_RST_PIN     0
#define MSX_WAIT_PIN    1
#define MSX_CLK_PIN     2
#define MSX_WR_PIN      3
#define MSX_RD_PIN      4
#define MSX_BUSDIR_PIN  5
#define MSX_INT_PIN     6
#define MSX_SLTSL_PIN   12
#define MSX_CS1_PIN     13
#define MSX_CS12_PIN    16
#define MSX_IORQ_PIN    17
#define MSX_CS2_PIN     18
#define MSX_D0_PIN      19
#define MSX_D1_PIN      (MSX_D0_PIN+1)
#define MSX_D2_PIN      (MSX_D0_PIN+2)
#define MSX_D3_PIN      (MSX_D0_PIN+3)
#define MSX_D4_PIN      (MSX_D0_PIN+4)
#define MSX_D5_PIN      (MSX_D0_PIN+5)
#define MSX_D6_PIN      (MSX_D0_PIN+6)
#define MSX_D7_PIN      (MSX_D0_PIN+7)
#define MSX_MERQ_PIN    27

#define MSX_RST     	(1<<MSX_RST_PIN)
#define MSX_WAIT        (1<<MSX_WAIT_PIN)
#define MSX_CLK         (1<<MSX_CLK_PIN)
#define MSX_WR          (1<<MSX_WR_PIN)
#define MSX_RD          (1<<MSX_RD_PIN)
#define MSX_BUSDIR      (1<<MSX_BUSDIR)
#define MSX_INT         (1<<MSX_INT_PIN)
#define MSX_SLTSL 	(1<<MSX_SLTSL_PIN)
#define MSX_CS1         (1<<MSX_CS1_PIN)
#define MSX_CS12        (1<<MSX_CS12_PIN)
#define MSX_IORQ        (1<<MSX_IORQ_PIN)
#define MSX_CS2         (1<<MSX_CS2_PIN)
#define MSX_D0      	(1<<MSX_D0_PIN)
#define MSX_D1      	(1<<(MSX_D0_PIN+1))
#define MSX_D2      	(1<<(MSX_D0_PIN+2))
#define MSX_D3          (1<<(MSX_D0_PIN+3))
#define MSX_D4          (1<<(MSX_D0_PIN+4))
#define MSX_D5          (1<<(MSX_D0_PIN+5))
#define MSX_D6          (1<<(MSX_D0_PIN+6))
#define MSX_D7          (1<<(MSX_D0_PIN+7))
#define MSX_MERQ    	(1<<MSX_MERQ_PIN)

#define MSX_GET_DATA ((*(gpio+13)&(0xff<<19))>>19)
#define MSX_CLK_HIGH    GPIO_SET = MSX_CLK
#define MSX_CLK_LOW     GPIO_CLR = MSX_CLK
#define MSX_RD_HIGH     GPIO_SET = MSX_RD
#define MSX_RD_LOW      GPIO_CLR = MSX_RD
#define MSX_WR_HIGH     GPIO_SET = MSX_WR
#define MSX_WR_LOW      GPIO_CLR = MSX_WR
#define MSX_SLTSL_HIGH  GPIO_SET = MSX_SLTSL
#define MSX_SLTSL_LOW   GPIO_CLR = MSX_SLTSL
#define MSX_CS1_HIGH    GPIO_SET = MSX_CS1
#define MSX_CS1_LOW     GPIO_CLR = MSX_CS1
#define MSX_CS2_HIGH    GPIO_SET = MSX_CS2
#define MSX_CS2_LOW     GPIO_CLR = MSX_CS2
#define MSX_MERQ_HIGH   GPIO_SET = MSX_MERQ
#define MSX_MERQ_LOW    GPIO_CLR = MSX_MERQ
#define MSX_IORQ_HIGH   GPIO_SET = MSX_IORQ
#define MSX_IORQ_LOW    GPIO_CLR = MSX_IORQ

#define MSX_SET_OUTPUT(g) INP_GPIO(g); OUT_GPIO(g)
#define MSX_SET_INPUT(g)  INP_GPIO(g)

void setup_io();
int readmsx(int slot, int addr);
void writemsx(int slot, int addr, int byte);
void clear_io();
 
int main(int argc, char **argv)
{
  int g,rep,i,addr, page=4;
  char byte;
  int offset = 0;
  int size = 0x4000;
  FILE *fp = 0;
  
  if (argc > 1)
  {
	  fp = fopen(argv[1], "wb");
  }
  if (argc > 2)
  {
	  offset = atoi(argv[2]);
  }
  if (argc > 3)
  {
	  size = atoi(argv[3]);
  }
 
  // Set up gpi pointer for direct register access
  setup_io();
 
 
  MSX_SET_OUTPUT(MSX_RST_PIN);
  MSX_SET_OUTPUT(MSX_CLK_PIN);  
  MSX_SET_OUTPUT(MSX_RD_PIN);  
  MSX_SET_OUTPUT(MSX_WR_PIN);  
  MSX_SET_OUTPUT(MSX_SLTSL_PIN);  
  MSX_SET_OUTPUT(MSX_CS1_PIN);  
  MSX_SET_OUTPUT(MSX_CS2_PIN);  
  MSX_SET_OUTPUT(MSX_CS12_PIN);  
  MSX_SET_OUTPUT(MSX_MERQ_PIN);  
  MSX_SET_OUTPUT(MSX_IORQ_PIN);  
  GPIO_SET = MSX_RST| MSX_CLK | MSX_RD | MSX_WR | MSX_SLTSL | MSX_CS1 | MSX_CS2 | MSX_MERQ | MSX_IORQ;  
  GPIO_CLR = MSX_RST;
  usleep(1000);
  GPIO_SET = MSX_RST;
  MSX_SET_INPUT(MSX_WAIT_PIN);  
  MSX_SET_INPUT(MSX_INT_PIN);  
  MSX_SET_INPUT(MSX_BUSDIR_PIN);  
  for (i = 0; i < 8; i++)
	MSX_SET_INPUT(MSX_D0_PIN+i);
  
  writemsx(1, 0x6000, 3);
  for(addr=offset; addr < offset + size; addr ++)
  {
	  if (addr > 0xbfff)
	  {
		 if (!(addr & 0x1fff)) {
			writemsx(1, 0x6000, page++);
			printf("page:%d, address=0x%04x\n", page-1, addr );
		 }
		 byte = readmsx(1, 0x6000 + (addr & 0x1ffff));
	  }
	  else
	  {
		  byte = readmsx(1, addr);
	  }
	  if (fp)
		  fwrite(&byte, 1, 1, fp);
	  else
		  printf ("0x%02x\n", byte);
  }
  
  clear_io();
  return 0;
 
} // main
 
 int readmsx(int slot, int addr)
 {
	 unsigned char address[2];
	 unsigned char byte;
	 struct timespec tv, tr;
	 tv.tv_sec = 0;
	 tv.tv_nsec = 200;
	 address[0] = (addr >> 8) & 0xff;
	 address[1] = (addr & 0xff);
	 GPIO_SET = MSX_RD | MSX_WR | MSX_SLTSL | MSX_CS1 | MSX_CS2 | MSX_MERQ | MSX_IORQ;
	 bcm2835_spi_transfern((char *)&address,2);
	 GPIO_CLR = MSX_RD | MSX_SLTSL | (addr < 0x8000 ? MSX_CS1 : MSX_CS2) | MSX_MERQ | (addr >= 0x4000 && addr < 0xc000 ? MSX_CS12 : 0);
	 nanosleep(&tv, &tr);
	 //usleep(1);
	 byte = MSX_GET_DATA;
//	 while(GET_GPIO(MSX_WAIT));
	 GPIO_SET = MSX_CLK | MSX_RD | MSX_SLTSL | MSX_CS1 | MSX_CS2 | MSX_CS12 | MSX_MERQ | (addr >= 0x4000 && addr < 0xc000 ? MSX_CS12 : 0);
	 return byte;	 
 }
 
 void writemsx(int slot, int addr, int byte)
 {
	 unsigned char data[3];
	 data[2] = addr & 0xff;
	 data[1] = (addr >> 8) & 0xff;
	 data[0] = byte;
	 GPIO_SET = MSX_CLK | MSX_RD | MSX_WR | MSX_SLTSL | MSX_CS1 | MSX_CS2 | MSX_MERQ | MSX_IORQ;
	 bcm2835_spi_transfern(data,3);
	 GPIO_CLR = MSX_CLK | MSX_WR | (addr < 0x8000 ? MSX_CS1 : MSX_CS2) | MSX_MERQ | MSX_CS12 | MSX_SLTSL | (addr >= 0x4000 && addr < 0xc000 ? MSX_CS12 : 0);
	 GPIO_SET = MSX_CLK;
	 GPIO_CLR = MSX_CLK;
	 GPIO_SET = MSX_CLK;
	 GPIO_CLR = MSX_CLK;
	 GPIO_SET = MSX_CLK;
	 GPIO_CLR = MSX_CLK;
	 usleep(1);
	 GPIO_SET = MSX_CLK | MSX_RD | MSX_WR | MSX_SLTSL | MSX_CS1 | MSX_CS2 | MSX_MERQ | MSX_IORQ | MSX_SLTSL | (addr >= 0x4000 && addr < 0xc000 ? MSX_CS12 : 0);
	 return;
 }
//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }
 
   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );
 
   close(mem_fd); //No need to keep mem_fd open after mmap
 
   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }
 
   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;

   if (!bcm2835_init())
   {
		printf("bcm2835_init error\n");
		exit(-1);   
   }
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4); 
    bcm2835_spi_chipSelect(BCM2835_SPI_CS1);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default   
} // setup_io

void clear_io()
{
    bcm2835_spi_end();
    bcm2835_close();
}
