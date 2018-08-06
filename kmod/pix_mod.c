#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>
#include <linux/miscdevice.h>
#include "rpi-gpio.h"
#include "rpmpv3.h"

MODULE_LICENSE("GPL");
static int    param_cpu_id;
module_param(param_cpu_id    , int, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(param_cpu_id, "CPU ID that operations run on");

#include <linux/gpio.h>

void pix_thread_init(void);
void pix_thread_exit(void);


static int __init pix_MSX_init(void){
	int cpu_id;
  printk(KERN_INFO "PIX: starting MSX driver...");
	if(param_cpu_id < 0 || param_cpu_id > 4) {
		printk(KERN_INFO "toy: unable to load module without cpu parameter\n");
		return -1;
	}
	printk(KERN_INFO "toy: loading to device driver, param_cpu_id: %d\n", param_cpu_id);
	//preempt_disable(); // See notes below
	cpu_id = get_cpu();
	printk(KERN_INFO "toy init called and running on CPU: %d\n", cpu_id);
	//preempt_enable(); // See notes below
	pix_thread_init();
	put_cpu();	
  printk(KERN_INFO "PIX: starting MSX driver done.");
  return 0;
}
static void __exit pix_MSX_exit(void){
  printk(KERN_INFO "PIX: stopping MSX...");
  pix_thread_exit();
  printk(KERN_INFO "PIX: stopping MSX done.");
}

module_init(pix_MSX_init);
module_exit(pix_MSX_exit);

#define THREAD_NAME "pix"

struct task_struct *task;

struct sched_param {
	int sched_priority;
};

static unsigned char ROM[] = {
#include "Antarctic.data"
//#include "Gradius.data"
//#include "Zemix30.data"
//#include "game126.data"
};	

int pix_thread(void *data){
	volatile int signal, addr;
	volatile unsigned char a = 0, byte;
	struct task_struct *TSK;
	static const struct sched_param PARAM = { .sched_priority = MAX_RT_PRIO };
	TSK = current;

	//PARAM.sched_priority = THREAD_PRIORITY;
	sched_setscheduler(TSK, SCHED_FIFO, &PARAM);
	gpio = (unsigned int *)ioremap(GPIO_BASE, GPIO_GPPUDCLK1*4);
	gpio[GPIO_GPFSEL0] = 0x249249;
	gpio[GPIO_GPFSEL1] = 0x249249;
	gpio[GPIO_GPFSEL2] = 0x9240;
#if 1

	GPIO_CLR(0xffffffff);
	GPIO_SET(INT | WAIT | HIADDR);

	while(1) 
	{
        signal = gpio[GPIO_GPLEV0];
        if ((signal & RESET))
        {
			GPIO_SET(HIADDR);
            addr = signal & 0xff00;
			GPIO_CLR(HIADDR | 0xff);
			addr |= ((signal >> 8) & 0xff);
            if (!(signal & (RD | SLTSL)))
            {
                addr = addr - 0x4000;
                if (addr >= 0 && addr < sizeof(ROM))
                {
                    GPIO_SET(ROM[addr]);
                    while(!(gpio[GPIO_GPLEV0] & RD));
                    continue;
                }
            }
            if (!(signal & (RD | MREQ)))
            {
                if (addr == 0xc000)
                {
					GPIO_SET(DAT_EN);
                    GPIO_SET(a++);                    
                    while(!(gpio[GPIO_GPLEV0] & RD));
					GPIO_CLR(DAT_EN);
                    continue;
                }
                if (addr == 0xc001)
                {
                    GPIO_SET(0);
                    GPIO_SET(0);
                    GPIO_SET(0);
                    byte =  gpio[GPIO_GPLEV0];
//                    printf("%04x(%02x),%c%c%c%c%c\n", (signal & IORQ) ? signal & 0xffff : signal & 0xff, byte, (signal & ATN) ? '!' : 'x', (signal & IORQ) > 0 ? 'M' : 'I', (signal & SLTSL) > 0 ? 'A' : 'S', (signal & RD) > 0 ? 'W':'R', (signal & RESET) > 0 ? 'R' : 'r');            }
                }
            }
            if (!(signal & IORQ) && !(signal & RD))
            {
                if ((signal & 0xff) == 100)
                {
                    GPIO_SET(a++);                    
                    while(!(gpio[GPIO_GPLEV0] & IORQ));
                }
                continue;
            }  
		}
		if (kthread_should_stop()) break;
	}
#else
	while(1)
	{
		if (kthread_should_stop()) break;
	}
#endif	
	return 0;
}

void pix_thread_init(void){
	printk(KERN_INFO "PIX: starting thread...");
	task = kthread_create(pix_thread, NULL, THREAD_NAME);
	if (task)
	{
		wake_up_process(task);
	}
	printk(KERN_INFO "PIX: starting thread done.");
}

void pix_thread_exit(void){
	printk(KERN_INFO "PIX: stopping thread...");
	gpio[GPIO_GPFSEL0] = 0;
	gpio[GPIO_GPFSEL1] = 0;
	gpio[GPIO_GPFSEL2] = 0;  
	kthread_stop(task);
	printk(KERN_INFO "PIX: stopping thread done.");
}