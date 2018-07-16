#ifndef RPMP

#define RPMP
#define RPMPV3

#define RA08	(1 << 8)
#define RA09	(1 << 9)
#define RA10	(1 << 10)
#define RA11	(1 << 11)
#define RA12	(1 << 12)
#define RA13	(1 << 13)
#define RA14	(1 << 14)
#define RA15	(1 << 15)
#define RC16	(1 << 16)
#define RC17	(1 << 17)
#define RC18	(1 << 18)
#define RC19	(1 << 19)
#define RC20	(1 << 20)
#define RC21	(1 << 21)
#define RC22	(1 << 22)
#define RC23	(1 << 23)
#define RC24	(1 << 24)
#define RC25	(1 << 25)
#define RC26	(1 << 26)
#define RC27	(1 << 27)

#define RESET   RC16
#define SLTSL   RC17
#define SNDOUT  RC18
#define IORQ    RC19
#define RD      RC20
#define ATN     RC21
#define RATN    RC22
#define INT     RC23
#define WAIT    RC24
#define DAT_DIR RC25
#define MREQ    RC26

volatile unsigned int* gpio;

void GPIO_SET(unsigned int b)
{
	gpio[GPIO_GPSET0] = b;
}

void GPIO_CLR(unsigned int b)
{
	gpio[GPIO_GPCLR0] = b;
}

void GPIO_PUT(unsigned int a, unsigned int b)
{
	gpio[GPIO_GPSET0] = a;
	gpio[GPIO_GPCLR0] = b;
}

#endif