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
#define MSX_SLTSL_PIN		17
#define MSX_A0_PIN		16
#define MSX_RW_PIN		18
#define MSX_MIRQ_PIN	19
#define MSX_BUSDIR_PIN	20
#define MSX_INTR_PIN	21
#define MSX_WAIT_PIN	22
#define MSX_RESET_PIN	23
#define MSX_D0_PIN      0
#define MSX_D1_PIN      (MSX_D0_PIN+1)
#define MSX_D2_PIN      (MSX_D0_PIN+2)
#define MSX_D3_PIN      (MSX_D0_PIN+3)
#define MSX_D4_PIN      (MSX_D0_PIN+4)
#define MSX_D5_PIN      (MSX_D0_PIN+5)
#define MSX_D6_PIN      (MSX_D0_PIN+6)
#define MSX_D7_PIN      (MSX_D0_PIN+7)
#define MSX_ADDR_PIN	0

#define MSX_SLTSL		(1<<MSX_SLTSL_PIN)
#define MSX_A0			(1<<MSX_A0_PIN)
#define MSX_RW			(1<<MSX_RW_PIN)
#define MSX_MIRQ		(1<<MSX_MIRQ_PIN)
#define MSX_BUSDIR		(1<<MSX_BUSDIR_PIN)
#define MSX_INTR  		(1<<MSX_INTR_PIN)
#define MSX_WAIT 		(1<<MSX_WAIT_PIN)
#define MSX_RESET 		(1<<MSX_RESET_PIN)
#define MSX_DATA      	(0xff<<MSX_D0_PIN)
#define MSX_ADDR      	(0xffff<<MSX_D0_PIN)

#define MSX_GET_DATA ((*(gpio+13))&(MSX_DATA))
#define MSX_GET_ADDR ((*(gpio+13))&(MSX_ADDR))

#define MSX_SET_OUTPUT(g) INP_GPIO(g); OUT_GPIO(g)
#define MSX_SET_INPUT(g)  INP_GPIO(g)

void setup_io();
void clear_io();

int main(int argc, char **argv)
{
  int g,rep,i,addr, page=4;
  char byte;
  int offset = 0;
  int val, pval;
  int size = 0x4000;
  char mem[0xffff];
  FILE *fp = 0;
  
  if (argc > 1)
  {
		fp = fopen(argv[1], "rb");
		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);	  
		rewind(fp);
  }
  if (argc > 2)
  {
	  offset = atoi(argv[2]);
  }
  for(addr=0; addr < offset; addr++)
	  mem[offset+addr] = fgetc(fp);
  // Set up gpi pointer for direct register access
  setup_io();

  printf("MSX Mega Box activated.\n");
  MSX_SET_INPUT(MSX_SLTSL_PIN);
  MSX_SET_OUTPUT(MSX_A0_PIN);
  MSX_SET_INPUT(MSX_RW_PIN);
  MSX_SET_INPUT(MSX_MIRQ_PIN);  
  MSX_SET_OUTPUT(MSX_BUSDIR_PIN);  
  MSX_SET_OUTPUT(MSX_INTR_PIN);  
  MSX_SET_OUTPUT(MSX_WAIT_PIN);  
  MSX_SET_OUTPUT(MSX_RESET_PIN);  
  GPIO_SET = MSX_RESET | MSX_BUSDIR | MSX_INTR | MSX_WAIT; 
  GPIO_CLR = MSX_A0;
  for (i = 0; i < 16; i++)
	MSX_SET_INPUT(MSX_D0_PIN+i);

  printf("%08x\n", MSX_SLTSL  & 0xb0700040);
  while(1)
  {
	  val = (*(gpio+13));
	  if (pval != val)
	  {
		//printf("%08x\n", val);
		pval = val;
		addr = val & MSX_ADDR;
#if 1		
		  if ((val & (MSX_SLTSL | MSX_MIRQ) & !(addr >= 0x4000 && addr < 0xc000) ) == 0)
		  {
			  if ((val & MSX_RW) == 0)
			  {
				*gpio = 0x249249;
				GPIO_CLR = 0xff;
				GPIO_SET = MSX_A0 | mem[addr];
				while(*(gpio+13)&MSX_SLTSL==0);
				printf("r%04x = %02x\n", addr, mem[addr]);
				*gpio = 0;
			  }
			  else
			  {
				GPIO_SET = MSX_A0;
//				printf("w%04x = %02x\n", addr, *(gpio+13) & 0xff);
//				mem[val & MSX_ADDR] = *(gpio+13) & 0xff;
			  }
		  }
		  else
		  GPIO_CLR = MSX_A0;
#endif	  
	  }
  };
  
  clear_io();
  return 0;
 
} // main
 
 
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
	gpio = gpio_map;
} // setup_io

void clear_io()
{
}
