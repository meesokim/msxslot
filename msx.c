#include <stdio.h>
#include "msxbus.h"

int main(int argc, char **argv)
{
  int g,rep,i,j,addr, page=4;
  char byte;
  int offset = 0x4000;
  int size = 0x8000;
  int pagetest = 0;
  int iotest = 0;
  int slot = 1;
  int b;
  int cnt;
  int port, hi, lo;
  FILE *fp = 0;
  
  if (argc > 1)
  {
	  if (argv[1][0] == '2')
		  slot = 2;
	  else if (argv[1][0] == '1')
		  slot = 1;
	  else if (argv[1][0] == 'i')
		iotest = 1;
	  else if (argv[1][0] != 'p')
		fp = fopen(argv[1], "wb");
	  else 
		pagetest = 1;
  }
  if (argc > 2)
  {
	  offset = atoi(argv[2]);
  }
  if (argc > 3)
  {
	  size = atoi(argv[3]);
  }
  
  msxinit();
  if (iotest)
  {
	  port = 0x50;
	  hi = 0x81;
	  lo = 0x42;
	  msxwriteio(port, 0x0);
	  msxwriteio(port+2, hi);
	  msxwriteio(port+3, lo);
	  for (i=0; i<32; i++)
	  {
		  printf("%02x ", msxreadio(port+3));
		  if (i == 15)
			  printf("\n");
	  }
/*	  
	  printf("Port%02x:",port);
	  for(i=0;i<16;i++)
	  {
		  printf("%02x ", msxreadio(port));
	  }
*/	  
	  printf("\n");
	  exit(0);
  }
  if (pagetest)
  {
	  for(i = 0; i < 16; i++)
	  {
	  	 msxwrite(slot, 0x7000, i);
		 printf("slot%d/page%02d:", slot, i);
	     for(j = 0; j < 16; j++)
		 {
			 printf("%02x ", msxread(slot, 0x6000+j));
		 }
		 printf("\n");
	  }
	  exit(0);
  }
  for(addr=offset; addr < offset + size; addr ++)
  {
	  if (addr > 0xbfff)
	  {
		 if (!(addr & 0x1fff)) {
			msxwrite(slot, 0x8000, page++);
			printf("slot%d/page%02d, address=0x%04x\n", slot, page-1, addr );
		 }
		 byte = msxread(slot, 0x8000 + (addr & 0x1fff));
	  }
	  else
	  {
		  byte = msxread(slot, addr);
	  }
	  if (fp)
		  fwrite(&byte, 1, 1, fp);
	  else
	  {
		  cnt = 10;
		  while(cnt--)
		  if (byte != (b = msxread(slot, addr)))
		  {
			 printf("\naddr=0x%04x, value=0x%02x, 0x%02x\n", addr, byte, b);
			 exit(0);
		  }
		  if (addr % 16 == 0)
				printf ("\n0x%04x: ", addr);
		  printf("%02x ", byte);
		
	  }
  }
  
  msxclose();
  printf("\n");
  return 0;
 
} // main