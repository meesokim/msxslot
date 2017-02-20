#include <stdio.h>
#include <time.h>
#include "msxbus.h"

struct MISS {
	int addr;
	unsigned char b;
} m[1024];


int main(int argc, char **argv)
{
  int g,rep,i,j,addr, page=4;
  char byte;
  int offset = 0x4000;
  int size = 0x8000;
  int pagetest = 0;
  int iotest = 0, fdctest = 0;
  int slot = 1;
  int b;
  int cnt, test = 0;
  int port, hi, lo;
  int miscount = 0;
  FILE *fp = 0;
  struct timespec t1, t2;
  double elapsedTime = 0;
  char c;
  if (argc > 1)
  {
	  if (argv[1][0] == '2')
		  slot = 2;
	  else if (argv[1][0] == '1')
		  slot = 1;
	  else if (argv[1][0] == 'i')
		iotest = 1;
	  else if (argv[1][0] == 't')
		test = 1;
	  else if (argv[1][0] == 'f')
		fdctest = 1;
	  else if (argv[1][0] != 'p')
		fp = fopen(argv[1], "wb");
	  else 
		pagetest = 1;
  }
  if (argc > 2)
  {
	  offset = atoi(argv[2]);
	  if (offset == 0)
		  offset = strtol(argv[2], NULL, 16);
  }
  if (argc > 3)
  {
	  size = atoi(argv[3]);
  }
  
  msxinit();
  if (test)
  {
	  for(addr=offset; addr < offset + size; addr ++)
	  {
		  c = ' ';
		  cnt = 2;
		  while(cnt--)
		  {
			  byte = msxread(slot, addr);
			  if (byte != (b = msxread(slot, addr)))
			  {
				  cnt = 2;
				  byte = b;
			  }				  
			  
		  }
		  if (addr % 16 == 0)
				printf ("\n0x%04x: ", addr);
		  printf("%02x%c", byte, c);
	  }
	  printf("\n");
	  exit(0);
  }
  if (iotest)
  {
	  port = 0x50;
	  hi = 0x81;
	  lo = 0x42;
	  msxwriteio(port, 0x0);
	  msxwriteio(port+2, hi);
	  msxwriteio(port+3, lo);
	  while(1)
	  {
		  for (i=0; i<255; i++)
		  {
			  msxwriteio(i, i);
			  msxreadio(i);
			  //printf("%02x ", msxreadio(i));
			  if (i == 15)
				  printf("\n");
		  }
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
	
	  pagetest:
	  slot = 1;
	  for(i = 0; i < 16; i++)
	  {
		 msxwrite(slot, 0x7000, i);
		 printf("slot%d/page%02d:", slot, i);
	     for(j = 0; j < 16; j++)
		 {
			 printf("%02x ", msxread(slot, 0x8000+j));
		 }
		 printf("\n");
	  }
//	 goto pagetest;
	  exit(0);
  }
  if (fdctest)
  {
	  int c0;
	  msxwrite(slot, 0x1000, 00);
	  msxwrite(slot, 0x1000, 0xc);
	  c = msxread(slot, 0x0);
	  printf("!0x%02x\n", c);
	  msxwrite(slot, 0x1, 0x3);
	  msxwrite(slot, 0x1, 0x9f);
	  msxwrite(slot, 0x1, 0x3);
	  msxwrite(slot, 0x1000, 0x1c);
	  msxwrite(slot, 0x1, 0x7);
	  msxwrite(slot, 0x1, 0x0);
	  msxwrite(slot, 0x1000, 0xc);
	  msxwrite(slot, 0x1000, 0x2d);
	  msxwrite(slot, 0x1, 0x08);
	  while(1) {
		  c = msxread(slot, 0x0);
		  if (c != c0)
		  {
			printf("0x%02x,", c);
		    c0 = c;
		  }
	  };
	  
  }
  restart:
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
  {
	  miscount = 0;
	  for(addr=offset; addr < offset + size; addr ++)
	  {
		  //usleep(0);
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
			//b = msxread(slot, addr);
		  }
	#if 1	  
		  if (fp)
			  fwrite(&byte, 1, 1, fp);
		  else
		  {
			  c = ' ';
	#if 0
			  if (byte != b) 
			  {
				  cnt = 10;
				  while(cnt--)
				  if (byte != (b = msxread(slot, addr)))
				  {
					 int cc = 10;
//					 printf("addr=0x%04x, value=", addr);
//					 printf("0x%02x!=0x%02x\n", byte, b);
					 byte = b;
					 #if 0
					 while(cnt--) {
						 b = msxread(slot, addr);
						 printf("0x%02x ", b);
					 }
					 printf("\n\n\n\n\n\n\n");
			//		 goto restart;
					 #endif
					 c='*';
				  }
				  
			  }
	#endif		  
			  if (c=='*') miscount++;
			  #if 1
			  if (addr % 16 == 0)
					printf ("\n0x%04x: ", addr);
			  printf("%02x%c", byte, c);
			  #endif
		  }
	#endif
	  }
	  addr = 0;
  };

  printf("\n");
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000000000.0;      // sec to ns
  elapsedTime += (t2.tv_nsec - t1.tv_nsec) ;   // us to ns
  printf("elapsed time: %10.2fns, %10.2fns/i ", elapsedTime, elapsedTime / size);
  printf("total misreading: %d\n", miscount);
//  goto restart;
  msxclose();
  printf("\n");
  return 0;
 
} // main
