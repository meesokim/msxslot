#include <bcm2835.h>
#include <stdio.h>
#include <sys/mman.h>

#define GP_CLK0_CTL *(gclk_base + 0x1C)
#define GP_CLK0_DIV *(gclk_base + 0x1D)

static volatile unsigned *gpio;

int main(int argc, char **argv)
{
    if (!bcm2835_init()) return -1;
    gpio = bcm2835_regbase(BCM2835_REGBASE_GPIO);
	unsigned *gclk_base = bcm2835_regbase(BCM2835_REGBASE_CLK);
	printf("gclk_base:%x\n", gclk_base);
	int speed_hz[] = { 0, 19200000, 0, 0, 0, 100000000, 500000000, 216000000, 0};
	if (gclk_base != MAP_FAILED)
	{
		int divi, divr, divf, freq, src_freq, divisor;
		bcm2835_gpio_fsel(20, BCM2835_GPIO_FSEL_ALT5); // GPIO_20
		// 0     0 Hz     Ground
		// 1     19.2 MHz oscillator
		// 2     0 Hz     testdebug0
		// 3     0 Hz     testdebug1
		// 4     0 Hz     PLLA
		// 5     1000 MHz PLLC (changes with overclock settings)
		// 6     500 MHz  PLLD
		// 7     216 MHz  HDMI auxiliary
		// 8-15  0 Hz     Ground
		int speed_id = 6;
		src_freq = speed_hz[speed_id];
		freq = 36500000;
		divi = src_freq / freq ;
		divr = src_freq % freq ;
		divf = (int)((double)divr * 4096.0 / src_freq) ;
		if (divi > 4095)
			divi = 4095 ;		
		divisor = 1 < 12;// | (int)(6648/1024);
		GP_CLK0_CTL = 0x5A000000 | speed_id;    // GPCLK0 off
		while (GP_CLK0_CTL & 0x80);    // Wait for BUSY low
		GP_CLK0_DIV = 0x5A000000 | (divi << 12) | divf; // set DIVI
		GP_CLK0_CTL = 0x5A000010 | speed_id;    // GPCLK0 on
		printf("clock enabled: 0x%08x\n", GP_CLK0_CTL );
	}
    return 0;
}