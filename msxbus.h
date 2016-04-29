#ifndef __MSXBUS__
#define __MSXBUS__
int msxread(int slot, unsigned short addr);
void msxwrite(int slot, unsigned short addr, unsigned char byte);
int msxioread(unsigned short addr);
void msxiowrite(int addr, int byte);
int msxinit();
int msxclose();
#endif