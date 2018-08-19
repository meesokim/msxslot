#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include "msxbus.h"

struct MISS {
	int addr;
	unsigned char b;
} m[1024];

static int verbose_flag;
static int binary;
static unsigned char ROM[] = {
#include "Antarctic.data"	
};
int main(int argc, char **argv)
{
  int g,rep,i,j,addr, page=4;
  char byte;
  int offset = 0x4000;
  int size = 0x4000;
  int pagetest = 0;
  int iotest = 0, fdctest = 0, fmpactest = 0;
  int slot = 1;
  int b;
  int cnt, test = 0;
  int port, hi, lo;
  int miscount = 0;
  int sz = 0;
  int compare = 0;
  FILE *fp = 0;
  FILE *fc = 0;
  struct timespec t1, t2;
  double elapsedTime = 0;
  char comp[256*256];
  int c;
      int option_index = 0;  
	static struct option long_options[] =
		{
		  /* These options set a flag. */
		  {"verbose", no_argument,       &verbose_flag, 1},
		  {"brief",   no_argument,       &verbose_flag, 0},
		  {"binary",  no_argument,       &binary, 1},
		  {"ascii",   no_argument,       &binary, 0},
		  /* These options don’t set a flag.
			 We distinguish them by their indices. */
		  {"io",      no_argument,       0, 'i'},
		  {"test",    no_argument,       0, 't'},
		  {"fdc",     no_argument,       0, 'f'},
		  {"page",    required_argument, 0, 'p'},
		  {"1",       no_argument,       0, '1'},
		  {"0",       no_argument,       0, '0'},
		  {"compare", required_argument, 0, 'c'},
		  {"out", 	  required_argument, 0, 'o'},
		  {"fmpac",   no_argument,       0, 'm'},
		  {0, 0, 0, 0}
		};  
    while (1)
	{
      c = getopt_long (argc, argv, "vbitfp:10c:o:mI",
                       long_options, &option_index);
      if (c == -1)
        break;
		switch(c)
		{
			case 0:
			  if (long_options[option_index].flag != 0)
				break;
			  printf ("option %s", long_options[option_index].name);
			  if (optarg)
				printf (" with arg %s", optarg);
			  printf ("\n");
			  break;
			case 'i':
				iotest = 1;
				printf("iotest enabled\n");
				break;
			case 'I':
				msxinit();	
				for(int i = 0; i < 10; i++)
				{
					msxwriteio(0xd4, 0xd4);
					printf("I/O address: 0xd440 ==> 0x%02x\n", msxreadio(0x40d4));
				}
				exit(0);
				break;
			case 't':
				test = 1;
				printf("test enabled\n");
				break;
			case 'f':
				fdctest = 1;
				printf("fdctest enabled\n");
				break;
			case '1':
				slot = 2;
				printf("slot = 2\n");
				break;
			case '0':
				slot = 1;
				printf("slot = 1\n");
				break;
			case 'p':
				if (optarg)
					pagetest = atoi(optarg);
				else
					pagetest = 1;
				break;
			case 'm':
				fmpactest = 1;
				break;
			case 'c':
//				printf("compare %s\n", optarg);
				fc = fopen(optarg, "r");
//				printf("%x\n", fc);
				if (fc != 0)
				{
					fseek(fc, 0L, SEEK_END);
					sz = ftell(fc);
	//				printf("size = %d\n", sz);
					rewind(fc);
					fread((void *)comp, 1, sz, fc); 
					compare = 1;
					printf("compare (%s) enabled\n", optarg);
				}
				break;
			case 'o':
				printf("outfile %s\n", optarg);
				break;
			default:
				break;
		}	
		
	}
  msxinit();
  if (test)
  {
	  while(1)
	  {
		  for(addr=offset; addr < offset + size; addr ++)
		  {
			  c = ' ';
			  cnt = 10;
			  byte = msxread(slot, addr);
			  while(cnt--)
			  {
				  if (byte != msxread(slot, addr))
					c = '*';
			  }
			  if (addr % 16 == 0)
					printf ("\n0x%04x: ", addr);
			  printf("%02x%c", byte, c);
		  }
		  printf("\n");
	  }
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
#define KONAMI
  if (pagetest > 0)
  {
	  slot = 1;
	  int page = pagetest > 1 ?  16: 32;
	  while(1)
	  {
		  printf("\033[0;0H");
		  for(i = 0; i < page; i++)
		  {
			if (pagetest > 1)
			msxwrite(slot, 0x6000, i);
			else 
			{				
			msxwrite(slot, 0x7fff, i);
			msxwrite(slot, 0x5000, 0);
			msxwrite(slot, 0x7000, 1);
			msxwrite(slot, 0x9000, 2);
			msxwrite(slot, 0xb000, 3);
			msxwrite(slot, 0x7ffe, 1);
			}
//			for(int i=0; i < 1000000; i++);
			 printf("slot%d/page%02d:", slot, i);
			 for(j = 0; j < 16; j++)
			 {
				if (pagetest > 1)
				printf("%02x ", msxread(slot, 0x6000+j));
				else				
				printf("%02x ", msxread(slot, 0x4000+j));
			 }
			 printf("\n");
		  }
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
  if (fmpactest) 
  {
	  for(int i = 0; i < 4; i++)
	  {
		  msxwrite(slot, 0x3ff7, i);
		  for(addr=0;addr < 0x4000; addr++)
		  {
			  c = ' ';
			  cnt = 10;
			  while(cnt--)
			  {
				  byte = msxread(slot, addr);
				  if (byte != (b = msxread(slot, addr)))
				  {
					  printf ("\ndata read error: %02x <> %02x\n", byte, b);
					  exit(0);
				  }
			  }
			  if (addr % 16 == 0)
					printf ("\n0x%04x: ", addr+i*0x4000);
			  printf("%02x%c", byte, c);
		  }
	  }	
  }
  msxwriteio(0xffff, 0);
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
				b = byte;
#if 1				
				if (compare)
				{
					if(byte == comp[addr-0x4000])
						b = byte;
					else
						b = comp[addr];					
				}
				else
					b = msxread(slot, addr);
#endif				
		  }
	#if 1	  
		  if (fp)
			  fwrite(&byte, 1, 1, fp);
		  else
		  {
			  c = ' ';
			  if (byte != b) 
	#if 1
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
				  else
					  break;
				  
			  }
	#else
				c = '*';
	#endif		  
			  if (c=='*') miscount++;
			  if (verbose_flag)
			  {
				  if (binary) {
					  printf("%c", byte);
				  } else {
					  if (addr % 32 == 0)
							printf ("\n0x%04x: ", addr);
					  if (c=='*')
						printf("\e[31m%02x\e[m", byte);
					  else
						printf("%02x", byte, c);
				  }
				  
			  }
		  }
	#endif
	  }
	  if (verbose_flag)
		printf("\n");
	  addr = 0;
  };

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000000000.0;      // sec to ns
  elapsedTime += (t2.tv_nsec - t1.tv_nsec) ;   // us to ns
  if (!binary) {
	printf("elapsed time: %10.2fs, %10.2fns/i ", elapsedTime/100000000, elapsedTime / size);
	printf("total misreading: %d\n", miscount);
  }
  goto restart;
  msxclose();
  return 0;
 
} // main
