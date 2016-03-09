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
volatile unsigned *clock_base;
#define CLOCK_BASE               (BCM2708_PERI_BASE + 0x101000) /* Clocks */
 
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
#if 0
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
#define MSX_SLTSL 	    (1<<MSX_SLTSL_PIN)
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

#else
#define MSX_D0_PIN      2
#define MSX_D1_PIN      (MSX_D0_PIN+1)
#define MSX_D2_PIN      (MSX_D0_PIN+2)
#define MSX_D3_PIN      (MSX_D0_PIN+3)
#define MSX_D4_PIN      (MSX_D0_PIN+4)
#define MSX_D5_PIN      (MSX_D0_PIN+5)
#define MSX_D6_PIN      (MSX_D0_PIN+6)
#define MSX_D7_PIN      (MSX_D0_PIN+7)	
#define MSX_W_PIN       26
#define MSX_CLK_PIN     13
#define MSX_CS_PIN      19
#define MSX_A0_PIN      10
#define MSX_A1_PIN      11

#define MSX_GET_DATA    ((*(gpio+13)&(0xff<<MSX_D0_PIN))>>MSX_D0_PIN)
#define MSX_SET_DATA(x) GPIO_SET = (0xff & (x)) << MSX_D0_PIN; GPIO_CLR = (0xff & ~(x)) << MSX_D0_PIN;
#define MSX_SET_CMD(x) GPIO_SET = (3 & (x)) << MSX_A0_PIN; GPIO_CLR = (3 & ~(x)) << MSX_A0_PIN; 

#define  MSX_W          (1<<MSX_W_PIN)
#define  MSX_CLK        (1<<MSX_CLK_PIN)
#define  MSX_CS         (1<<MSX_CS_PIN)
#define  MSX_A0         (1<<MSX_A0_PIN)
#define  MSX_A1         (1<<MSX_A1_PIN)
#define  MSX_INPUT_MODE *gpio = 0
#define  MSX_OUTPUT_MODE *gpio = 0x09249240

inline void msxset(int x, int y)   
{
	int i=0;
	MSX_OUTPUT_MODE;
	GPIO_SET = MSX_W;
	GPIO_SET = (0xff &  (y)) << MSX_D0_PIN | MSX_CS | MSX_CLK |  ((3 & (x)) << MSX_A0_PIN);
	GPIO_CLR = (0xff & ~(y)) << MSX_D0_PIN | MSX_CS | MSX_W | ((3 & ~(x)) << MSX_A0_PIN);
	GPIO_CLR = MSX_CLK;
	for(i=0;i<100;i++);
    GPIO_SET = MSX_CS | MSX_CLK;
	MSX_INPUT_MODE;
    return;
}

inline int msxget(int x)   
{
	int i=0;
	int byte;
	MSX_INPUT_MODE;
	GPIO_SET = MSX_W;
    GPIO_SET = MSX_CS | MSX_CLK | MSX_W | ((3 & (x)) << MSX_A0_PIN); \
	GPIO_CLR = MSX_CS | ((3 & ~(x)) << MSX_A0_PIN);
	GPIO_CLR = MSX_CLK;
//	for(i=0;i<100;i++);
	byte = MSX_GET_DATA;
    GPIO_SET = MSX_CS | MSX_CLK;	
	return byte;
}

#endif

#define MSX_SET_OUTPUT(g) INP_GPIO(g); OUT_GPIO(g)
#define MSX_SET_INPUT(g)  INP_GPIO(g)


#define GZ_CLK_5MHz 0
#define GZ_CLK_125MHz 1

#define GZ_CLK_BUSY    (1 << 7)
#define GP_CLK0_CTL *(get_clock_base() + 0x1C)
#define GP_CLK0_DIV *(get_clock_base() + 0x1D)

__off_t get_peripheral_base();
volatile unsigned * get_gpio_base();
volatile unsigned * get_clock_base();



int clock_ena(int speed, int divider);
int clock_dis();

void setup_io();
int readmsx(int slot, int addr);
void writemsx(int slot, int addr, int byte);
void clear_io();

#define MEM 0 
#define IO 1
#define READ 0
#define WRITE 1

void msxwrite(int addr, int byte);
void msxout(int add, int byte);
int msxread(int addr);
int msxinp(int addr);

int getValue(char *str)
{
	int val = 0;
	char *s;
	if (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X'))
	{
		s = str+2;
		while(*s != 0)
		{
			val = val * 16 + (*s > '0' && *s <= '9' ? *s - '0' : *s >= 'A' && *s <= 'F' ?  *s - 'A' + 10 : *s >= 'a' && *s <= 'f' ? *s - 'a' + 10 : 0); 
			s++;
		}
	}
	else if (*str == 'b')
	{
		s = str+1;
		while(*s != 0)
		{
			val = val * 2 + (*s == '1' ? 1 : 0);
			s++;
		}
	} 
	else 
	{
		s = str;
		while(*s != 0)
		{
			val = val * 10 + (*s > '0' && *s <= '9' ? *s - '0' : 0);
			s++;
		}
	}
	//printf("%s:%d\n", str, val);
	return val;
}
 
int main(int argc, char **argv)
{
  int g,rep,i,addr=-1, page=4, type = -1;
  unsigned char bytes[0x10000];
  char byte;
  char *arg;
  int offset = 0x4000;
  int size = 0x4000;
  FILE *fp = 0;
  int idx = 0, rw = -1, length = -1;
  i = 0;
  char *v;

  // Set up gpi pointer for direct register access
  setup_io();  
  
  MSX_SET_OUTPUT(MSX_A0_PIN);
  MSX_SET_OUTPUT(MSX_A1_PIN);
  MSX_SET_OUTPUT(MSX_CS_PIN);
  MSX_SET_OUTPUT(MSX_CLK_PIN);
  MSX_SET_OUTPUT(MSX_W_PIN);
  GPIO_SET = MSX_CS | MSX_CLK | MSX_W;
  
  while (argc > ++idx)
  {
	  arg = argv[idx];
	  if (type == -1)
	  {
		  switch (*arg)
		  {
			  case 'i':
				type = IO;
				if (*(arg+1) == 'w')
					rw = WRITE;
				else
					rw = READ;
				continue;
			  case 'm':
				type = MEM;
				if (*(arg+1) == 'w')
					rw = WRITE;
				else
					rw = READ;
				continue;
			  case 'w':
				type = MEM;
				rw = WRITE;
				continue;
			  default:
			    type = MEM;
				rw = READ;
		  }
	  }
	  if (addr == -1)
	  {
		  addr = getValue(arg);
		  continue;
	  }
	  if (rw == WRITE && bytes[i] == -1)
	  {
		  bytes[i++] = getValue(arg);
		  continue;
	  }
	  if (rw == READ && length == -1)
	  {
		  length = getValue(arg);
		  continue;
	  }
  }
  if (i > 0)
	  length = i;
  if (addr < 0)
	  addr = 0;
  if (length < 0)
	  length = 1;
  
  //printf("Type:%s,RW:%s, addr=0x%04x, size=%d\n", type ? "IO" : "MEM", rw ? "WRITE" : "READ", addr, length);
  

  //clock_ena(1, 2*7);
  
  if (type == MEM)
  {
	  if (rw == READ) 
	  {
		  for(i = 0; i < length; i++)
		  {
			  byte = msxread(addr+i);
			  printf("0x%04x > 0x%02x\n", addr+i, byte);
		  }
	  }
	  else
	  {
		  for(i = 0; i < length; i++) 
		  {
			msxwrite(addr, bytes[i]);
			printf("0x%04x < 0x%02x\n", addr+i, bytes[i]);
			  
		  }
	  }
  } 
  else
  {
	  addr = addr & 0xff;
	  if (rw == READ) 
	  {
		  for(i = 0; i < length; i++)
		  {
			  byte = msxinp(addr+i);
			  printf("IO0x%02x > 0x%02x\n", addr+i, byte);
		  }
	  }
	  else
	  {
		  for(i = 0; i < length; i++) 
		  {
			msxout(addr, bytes[i]);
			printf("IO0x%02x < 0x%02x\n", addr+i, bytes[i]);
			  
		  }
	  }	  
  }
  clear_io();
  return 0;
 
} // main
 
 #define RD    (1<<7)
 #define WR    (1<<6)
 #define IORQ  (1<<5)
 #define MERQ  (1<<4)
 #define SLTSL (1<<3)
 #define RESET (1<<2)
 
 #define GETDATA     0x00
 #define READMEMORY  RD | MERQ | SLTSL
 #define WRITEMEMORY WR | MERQ | SLTSL
 #define READIO      RD | IORQ
 #define WRITEIO     WR | IORQ
 
 int msxread(int addr)
 {
	 unsigned char byte;
	 msxset(0, 0);
	 msxset(0, READMEMORY);
	 msxset(1, (addr >> 8) & 0xff);
	 msxset(2, (addr & 0xff));
	 byte = msxget(1);
//	 printf("status=0x%02x 0x%02x\n", msxget(0), msxget(2));
	 return byte;	 
 }
 
 void msxwrite(int addr, int byte)
 {
	 msxset(0, 0);
	 msxset(0, WRITEMEMORY);
	 msxset(1, (addr >> 8) & 0xff);
	 msxset(2, (addr & 0xff));
	 msxset(3, byte);
//	 printf("status=0x%02x 0x%02x\n", msxget(0), msxget(2));
	 return;
 }
 
 int msxinp(int addr)
 {
	 unsigned char byte;
	 msxset(0, READIO);
	 msxset(1, (addr >> 8) & 0xff);
	 msxset(2, (addr & 0xff));
	 byte = msxget(1);
	 return byte;	 
 }
 
 void msxout(int addr, int byte)
 {
	 msxset(0, WRITEIO);
	 msxset(1, (addr >> 8) & 0xff);
	 msxset(2, (addr & 0xff));
	 msxset(3, byte);	 
	 return;
 }
 

//
// Set up a memory regions to access GPIO
//
void setup_io()
{
	void *clock_map;	
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
   
    clock_map = mmap(
      NULL,             //Any address in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      CLOCK_BASE         //Offset to hardware clock peripheral
    );   
 
   close(mem_fd); //No need to keep mem_fd open after mmap
 
   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }
 
   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
   clock_base = (volatile unsigned *)clock_map;

   if (!bcm2835_init())
   {
		printf("bcm2835_init error\n");
		exit(-1);   
   }
    // bcm2835_spi_begin();
    // bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    // bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    // bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32); 
    // bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    // bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default   
    // bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);      // the default   
} // setup_io

void clear_io()
{
 //   bcm2835_spi_end();
    bcm2835_close();
}


volatile unsigned * get_clock_base() {
  return clock_base;
}

int clock_ena(int speed, int divisor) {
  int speed_id = 6;
  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
     printf("\rError initializing IO. Consider using sudo.\n");
     exit(-1);
  }
  if (speed < GZ_CLK_5MHz || speed > GZ_CLK_125MHz) {
    printf("clock_ena: Unsupported clock speed selected.\n");
    printf("Supported speeds: GZ_CLK_5MHz (0) and GZ_CLK_125MHz (1).\n");
    exit(-1);
  }
  if (speed == 0) {
    speed_id = 1;
  }
  if (divisor < 2) {
    printf("clock_ena: Minimum divisor value is 2.\n");
    exit(-1);
  }
  if (divisor > 0xfff) {
    printf("clock_ena: Maximum divisor value is %d.\n", 0xfff);
    exit(-1);
  }
  close(mem_fd); //No need to keep mem_fd open after mmap
  usleep(1000);
  INP_GPIO(4);
  SET_GPIO_ALT(4,0);
  GP_CLK0_CTL = 0x5A000000 | speed_id;    // GPCLK0 off
  while (GP_CLK0_CTL & GZ_CLK_BUSY) {}    // Wait for BUSY low
  GP_CLK0_DIV = 0x5A002000 | (divisor << 12); // set DIVI
  GP_CLK0_CTL = 0x5A000010 | speed_id;    // GPCLK0 on
  GPIO_SET = 1 << 4;
  return 0;
  
}

int clock_dis() {
  INP_GPIO(4);
  return 0;
}