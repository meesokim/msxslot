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
#define MCS_PIN     0 
#define MA0_PIN     1
#define MODE_PIN    2
#define MCLK_PIN    4
#define MD00_PIN    5
#define MD01_PIN    6
#define MD02_PIN    7
#define MD03_PIN    8
#define MD04_PIN    9
#define MD05_PIN    10
#define MD06_PIN    11
#define MD07_PIN    12
#define MD08_PIN    13
#define MD09_PIN    14
#define MD10_PIN    15
#define MD11_PIN    16
#define MD12_PIN    17
#define MD13_PIN    18
#define MD14_PIN    19
#define MD15_PIN    20
#define SW1_PIN     21
#define SW0_PIN     22
#define WAIT_PIN    23
#define INT_PIN     24
#define BUSDIR_PIN  25

#define MSX_CS       1
#define MSX_MA0     (1<<MA0_PIN)
#define MSX_MODE    (1<<MODE_PIN)
#define MSX_SW0     (1<<SW0_PIN)
#define MSX_SW1     (1<<SW1_PIN)
#define MSX_INT     (1<<INT_PIN)
#define MSX_BUSDIR  (1<<BUSDIR_PIN)
#define MSX_WAIT    (1<<WAIT_PIN)

#define SET_ADDR(x) GPIO_CLR = 0xffff << MD00_PIN; GPIO_SET = (x & 0xffff) << MD00_PIN
#define GET_DATA(x) x = GET_GPIO(0xff << MD00_PIN) >> MD00_PIN
#define SET_DATA(x) GPIO_CLR = 0xff << MD00_PIN; GPIO_SET = (x & 0xff) << MD00_PIN

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
 
  MSX_SET_OUTPUT(MCS_PIN);
  MSX_SET_OUTPUT(MA0_PIN);  
  MSX_SET_OUTPUT(MODE_PIN);  
  MSX_SET_INPUT(BUSDIR_PIN);  
  MSX_SET_INPUT(INT_PIN);  
  MSX_SET_INPUT(WAIT_PIN);  
  MSX_SET_INPUT(SW0_PIN);  
  MSX_SET_INPUT(SW1_PIN);  
  
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
	 unsigned char byte;
	 GPIO_SET = MSX_CS;
	 GPIO_CLR = MSX_MA0 | MSX_MODE;
	 SET_ADDR(addr);
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 GPIO_SET = MSX_CS;
	 GPIO_SET = MSX_MA0;
	 GPIO_CLR = MSX_RD | MSX_MERQ | (slot == 0 ? MSX_SLTSL0 : MSX_SLTSL1);
	 GPIO_SET = MSX_WR | MSX_IORQ | (slot == 1 ? MSX_SLTSL1 : MSX_SLTSL1) | MSX_RESET | MSX_M1;
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 GPIO_SET = MSX_CS;
	 GET_DATA(byte);	 
	 return byte;	 
 }
 
 void writemsx(int slot, int addr, int byte)
 {
	 unsigned char byte;
	 GPIO_SET = MSX_CS;
	 GPIO_CLR = MSX_MA0 | MSX_MODE;
	 SET_ADDR(addr);
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 GPIO_SET = MSX_CS;
	 GPIO_SET = MSX_MA0;
	 GPIO_CLR = MSX_WR | MSX_MERQ | (slot == 0 ? MSX_SLTSL0 : MSX_SLTSL1);
	 GPIO_SET = MSX_RD | MSX_IORQ | (slot == 1 ? MSX_SLTSL1 : MSX_SLTSL1) | MSX_RESET | MSX_M1;
	 SET_DATA(byte);	 
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 return;
 }

 int inpmsx(int addr)
 {
	 unsigned char byte;
	 GPIO_SET = MSX_CS;
	 GPIO_CLR = MSX_MA0 | MSX_MODE;
	 SET_ADDR(addr);
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 GPIO_SET = MSX_CS;
	 GPIO_SET = MSX_MA0;
	 GPIO_CLR = MSX_RD | MSX_IORQ;
	 GPIO_SET = MSX_WR | MSX_MERQ | MSX_RESET | MSX_M1;
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 GPIO_SET = MSX_CS;
	 GET_DATA(byte);	 
	 return byte;	 
 }
 
 void outmsx(int addr, int byte)
 {
	 unsigned char address[2];
	 unsigned char byte;
	 GPIO_SET = MSX_CS;
	 GPIO_CLR = MSX_MA0 | MSX_MODE;
	 SET_ADDR(addr);
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
	 GPIO_SET = MSX_CS;
	 GPIO_SET = MSX_MA0;
	 GPIO_CLR = MSX_WR | MSX_IORQ ;
	 GPIO_SET = MSX_RD | MSX_MERQ | MSX_RESET | MSX_M1;
	 SET_DATA(byte);	 
	 GPIO_CLR = MSX_CS;
	 while(GET_GPIO(MSX_ACC));
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
#if 0   
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4); 
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default   
#endif
} // setup_io

void clear_io()
{
#if 0
    bcm2835_spi_end();
#endif
    bcm2835_close();
}