#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/io.h>

#include "rpi-gpio.h"
#include "rpmcv8.h"

enum {
   SLOT1,
   SLOT3,
   IO
};

#define MSX_READ 0x10
#define MSX_WRITE 0

volatile unsigned *gpio;

#define	 CLASS_NAME "msx"
#define  DEVICE_NAME "msxbus"    ///< The device will appear at /dev/MSX Bus using this value
#define FIRST_MINOR 0
#define MINOR_CNT 1

int msxread(char slot, unsigned short addr);
void msxwrite(char slot, unsigned short addr, unsigned char byte);
int msxreadio(unsigned short addr);
void msxwriteio(unsigned short addr, unsigned char byte);

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long    dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .unlocked_ioctl = dev_ioctl,
   .write = dev_write,
   .release = dev_release,
};

// static int    majorNumber;                  ///< Stores the device number -- determined automatically
// static char   msxdata[256*256] = {0};           ///< Memory for the string that is passed from userspace
// static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  msxClass  = NULL; ///< The device-driver class struct pointer
static dev_t dev;
static struct cdev c_dev;
static unsigned bk_gpio[0xB0/4];

static int dev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}
 
static int __init msxdrv_init(void)
{
    int ret;    
    struct device* msxDevice = NULL; ///< The device-driver device struct pointer
    printk(KERN_INFO "MSX Bus: Initializing the MSX Bus LKM\n");

#if 1
    if((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, DEVICE_NAME)) < 0)
    {
        printk("MSX Bus: alloc_chrdev_region Error %d \n", ret);
        return ret;
    }    
    cdev_init(&c_dev, &fops);    

    if((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        printk("alloc_chrdev_region error\n");
        unregister_chrdev_region(dev, MINOR_CNT);
        return ret;
    }    

    printk(KERN_INFO "MSX Bus: alloc_chrdev_region Okay \n");
    if(IS_ERR(msxClass = class_create(THIS_MODULE, DEVICE_NAME)))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        printk("class_create ERROR \n");
        return PTR_ERR(msxClass);
    }
    msxClass->dev_uevent = dev_uevent;
    printk(KERN_INFO "MSX Bus: class_create okay \n");

    if(IS_ERR(msxDevice = device_create(msxClass, NULL, dev, "%s", DEVICE_NAME)))
    {
        class_destroy(msxClass);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        printk("device_create ERROR \n");
        return PTR_ERR(msxDevice);
    }

#else    
    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "MSX Bus failed to register a major number\n");
        return majorNumber;
    }

    printk(KERN_INFO "MSX Bus: registered correctly with major number %d\n", majorNumber);

    // Register the device class
    msxClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(msxClass)){                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(msxClass);          // Correct way to return an error on a pointer
    }
    msxClass->dev_uevent = dev_uevent;
    printk(KERN_INFO "MSX Bus: device class registered correctly\n");

    // Register the device driver
    msxDevice = device_create(msxClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(msxDevice)){               // Clean up if there is an error
        class_destroy(msxClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(msxDevice);
    }

#endif
    printk(KERN_INFO "MSX Bus: device registered correctly\n");    

    if ((gpio = ioremap(GPIO_BASE, 0xB0)) == NULL) {
        printk(KERN_INFO "io remap failed\n");
        return -EBUSY;
    }
    // gpio = (unsigned int *)ioremap(GPIO_BASE, GPIO_GPPUDCLK1*4);
    bk_gpio[GPIO_GPFSEL0] = gpio[GPIO_GPFSEL0];
    bk_gpio[GPIO_GPFSEL1] = gpio[GPIO_GPFSEL1];
    bk_gpio[GPIO_GPFSEL2] = gpio[GPIO_GPFSEL2];
    bk_gpio[GPIO_GPPUD] = gpio[GPIO_GPPUD];
    gpio[GPIO_GPFSEL0] = GPIO_FSEL0_OUT | GPIO_FSEL1_OUT | GPIO_FSEL2_OUT | GPIO_FSEL3_OUT | GPIO_FSEL4_OUT | GPIO_FSEL5_OUT | GPIO_FSEL6_OUT | GPIO_FSEL7_OUT | GPIO_FSEL8_OUT | GPIO_FSEL9_OUT;
    gpio[GPIO_GPFSEL1] = GPIO_FSEL0_OUT | GPIO_FSEL1_OUT | GPIO_FSEL2_OUT | GPIO_FSEL3_OUT | GPIO_FSEL4_OUT | GPIO_FSEL5_OUT | GPIO_FSEL6_OUT | GPIO_FSEL7_OUT | GPIO_FSEL8_OUT | GPIO_FSEL9_OUT;
    gpio[GPIO_GPFSEL2] = GPIO_FSEL0_OUT | GPIO_FSEL1_OUT | GPIO_FSEL2_OUT | GPIO_FSEL3_OUT | GPIO_FSEL4_OUT | GPIO_FSEL5_OUT | GPIO_FSEL6_OUT | GPIO_FSEL7_OUT;   
    gpio[GPIO_GPPUD] = 2;
    ndelay(1500);
    gpio[GPIO_GPPUDCLK0] = MSX_WAIT;

	GPIO_SET(LE_C | MSX_CONTROLS | MSX_WAIT | MSX_INT);
	GPIO_SET(LE_A | LE_D);
	GPIO_CLR(LE_C | 0xffff);
	GPIO_CLR(LE_C);
	GPIO_CLR(MSX_RESET);
	for(int i=0;i<2000000;i++);
	GPIO_SET(MSX_RESET);
	// for(int i=0;i<1000000;i++);
    return 0;
}

static void __exit msxdrv_exit(void)
{
    gpio[GPIO_GPFSEL0] = bk_gpio[GPIO_GPFSEL0];
    gpio[GPIO_GPFSEL1] = bk_gpio[GPIO_GPFSEL1];
    gpio[GPIO_GPFSEL2] = bk_gpio[GPIO_GPFSEL2];
    gpio[GPIO_GPPUD] = bk_gpio[GPIO_GPPUD];
    iounmap(gpio);
    printk(KERN_INFO "MSX Bus: io remap released\n");
    device_destroy(msxClass, dev);
    class_destroy(msxClass);           // Repeated code but the alternative is goto statements
    printk(KERN_INFO "MSX Bus: device class unregistered\n");
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
    printk(KERN_INFO "MSX Bus: /dev/msxbus unregistered\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "MSX Bus: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "MSX Bus: Device successfully closed by %d\n", numberOpens--);
    return 0;
}

 
/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
//    int error_count = 0;
//    int count = 0;
//    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
//    // printk(KERN_INFO "f_pos: %ll\n", *offset);
//    while(len--)
//    {
// 	   msxdata[count++] = msxread(0, *offset++);
//    }
//    error_count = copy_to_user(buffer, msxdata, count);
//    if (error_count==0){            // if true then have success
//       printk(KERN_INFO "MSX Bus: Sent %d characters to the user\n", size_of_message);
//       return (size_of_message=0);  // clear the position to the start and return 0
//    }
//    else {
//       printk(KERN_INFO "MSX Bus: Failed to send %d characters to the user\n", error_count);
//       return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
//    }
    return 0;
}
 
/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
//   sprintf(msxdata, "%s(%zu letters)", buffer, len);   // appending received string with its length
//   size_of_message = strlen(message);                 // store the length of the stored message
	// int count = 0;
	// while(len--)
	// {
	// 	msxwrite(0, filep->f_pos++, buffer[count++]);
	// }
	// len = count;
	// printk(KERN_INFO "MSX Bus: Received %zu characters from the user\n", len);
   return len;
}
 

static long dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

    long byte = 0;
    // local_irq_disable();
    if (cmd & MSX_READ) {
        byte =  msxread(cmd & 0xf, arg & 0xffff);   
    } else {
        msxwrite(cmd, arg & 0xffff, (arg >> 16) & 0xff);
    }
    // local_irq_enable();
    return byte;
}

void SetAddress(unsigned short addr)
{

    GPIO_CLR(LE_C | 0xffff | DAT_DIR);
    GPIO_SET(LE_A | LE_D | addr);
    GPIO_CLR(LE_A);
    GPIO_SET(LE_C | MSX_CONTROLS);
    GPIO_CLR(LE_D | 0xff);
}	

void SetData(int ioflag, int flag, int delay, unsigned char byte)
{
    GPIO_SET(MSX_MREQ | MSX_WR);
    GPIO_CLR(DAT_DIR | 0xff);
    GPIO_SET(byte);
    GPIO_CLR(ioflag | flag);
    ndelay(delay);
    while(!(GPIO_GET() & MSX_WAIT));
    GPIO_SET(LE_D | MSX_CONTROLS);
    GPIO_CLR(LE_C);
}   

unsigned char GetData(int flag,  int rflag, int delay)
{
    unsigned char byte;
    GPIO_SET(DAT_DIR);
    GPIO_CLR(flag | rflag);
    while(!(GPIO_GET() & MSX_WAIT));
    ndelay(delay);
    byte = GPIO_GET();
    GPIO_SET(LE_D | MSX_CONTROLS);
    GPIO_CLR(LE_C);
    return byte;
}

int msxread(char slot_io, unsigned short addr)
{
    unsigned char byte;
    int sio, cs1, cs2;
    cs1 = (addr & 0xc000) == 0x4000 ? MSX_CS1: 0;
    cs2 = (addr & 0xc000) == 0x8000 ? MSX_CS2: 0;
    SetAddress(addr);
    switch (slot_io) {
        case SLOT1:
            sio = MSX_SLTSL1 | MSX_MREQ | cs1 | cs2;
            break;
        case SLOT3:
            sio = MSX_SLTSL3 | MSX_MREQ | cs2 | cs2;
            break;
        case IO:
            sio = MSX_IORQ;
            break;
        default:
            return 0;      
    }
    byte = GetData(sio,  MSX_RD, 1);
    return byte;	 
}

void msxwrite(char slot_io, unsigned short addr, unsigned char byte)
{
	int sio;
   switch (slot_io) {
      case SLOT1:
         sio = MSX_SLTSL1 | MSX_MREQ;
         break;
      case SLOT3:
         sio = MSX_SLTSL3 | MSX_MREQ;
         break;
      case IO:
         sio = MSX_IORQ;
      default:
         return;      
   }
	SetAddress(addr);
	SetData(sio, MSX_WR, 90, byte);   
	return;
}

module_init(msxdrv_init);
module_exit(msxdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miso Kim");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("1MSX Bus driver for the RPMC board");  ///< The description -- see modinfo
MODULE_VERSION("1.0");            ///< A version number to inform users