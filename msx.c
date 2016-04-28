#include <stdio.h>
#include "msxbus.h"

int main(int argc, char **argv)
{
  int g,rep,i,j,addr, page=4;
  char byte;
  int offset = 0x4000;
  int size = 0x8000;
  int pagetest = 0;
  FILE *fp = 0;
  
  if (argc > 1)
  {
	  if (argv[1][0] != 'p')
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
  if (pagetest)
  {
	  for(i = 0; i < 16; i++)
	  {
	  	 writemsx(1, 0x6000, i);
		 printf("page%02d:", i);
	     for(j = 0; j < 8; j++)
		 {
			 printf("%02x ", readmsx(1, 0x6000+j));
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
			writemsx(1, 0x8000, page++);
			printf("page:%d, address=0x%04x\n", page-1, addr );
		 }
		 byte = readmsx(1, 0x8000 + (addr & 0x1fff));
	  }
	  else
	  {
		  byte = readmsx(1, addr);
	  }
	  if (fp)
		  fwrite(&byte, 1, 1, fp);
	  else
	  {
		  if (byte == readmsx(1, addr))
		  {
			  if (addr % 16 == 0)
				printf ("\n0x%04x: ", addr);
			  printf("%02x ", byte);
		  }
		 else
			exit(0);
	  }
  }
  
  msxclose();
  printf("\n");
  return 0;
 
} // main