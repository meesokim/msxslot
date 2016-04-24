#include <stdio.h>
#include "msxbus.h"

int main(int argc, char **argv)
{
  int g,rep,i,addr, page=4;
  char byte;
  int offset = 0x4000;
  int size = 0x8000;
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
  
  msxinit();
  //writemsx(1, 0x6000, 3);
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
	  {
		  if (byte == readmsx(1, addr))
			printf ("0x%04x: 0x%02x\n", addr, byte);
		 else
			exit(0);
	  }
  }
  
  msxclose();
  return 0;
 
} // main