#ifndef __MSXBUS__
#define __MSXBUS__
int readmsx(int slot, int addr);
void writemsx(int slot, int addr, int byte);
int inpmsx(int addr);
void outmsx(int addr, int byte);
int msxinit();
int msxclose();
#endif