#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define GPIO_BASE 0x3F200000
#define GPIO_SIZE 0xB4

#define GPIO_DATA_MASK 0xFFFF
#define GPIO_CS 17
#define GPIO_CLK 16
#define GPIO_DATA_START 0
#define GPIO_DATA_END 15

// Command definitions
#define CMD_MEM_READ    0x00
#define CMD_MEM_WRITE   0x01
#define CMD_IO_READ     0x02
#define CMD_IO_WRITE    0x03
#define CMD_RESET       0x05
#define CMD_STATUS      0x08

// GPIO Registers
#define GPSEL0 0
#define GPSET0 0x1c
#define GPCLR0 0x28
#define GPLEV0 0x34

#define OUTPUT_DIR0 0x09249249
#define OUTPUT_DIR1 0x04049249
#define INPUT_DIR0  0x09200000
#define INPUT_DIR1  0x00048000

static void __iomem *gpio_base;

#define GPIO_CLK_0 iowrite32(1 << GPIO_CLK, gpio_base + GPCLR0)
#define GPIO_CLK_1 iowrite32(1 << GPIO_CLK, gpio_base + GPSET0)
#define GPIO_CS_0 iowrite32(1 << GPIO_CS, gpio_base + GPCLR0)
#define GPIO_CS_1 iowrite32(1 << GPIO_CS, gpio_base + GPSET0)

static inline void gpio_set_value8(uint8_t value) {
    iowrite32(OUTPUT_DIR0, gpio_base + GPSEL0);
    iowrite32(GPIO_DATA_MASK << GPIO_DATA_START, gpio_base + GPCLR0);  
    iowrite32(value << GPIO_DATA_START, gpio_base + GPSET0);
    GPIO_CLK_0;
    GPIO_CLK_1;
}

static inline void gpio_set_value16(uint16_t value) {
    iowrite32(OUTPUT_DIR0, gpio_base + GPSEL0);
    iowrite32(OUTPUT_DIR1, gpio_base + GPSEL1);
    iowrite32(GPIO_DATA_MASK << GPIO_DATA_START, gpio_base + GPCLR0);  
    iowrite32(value << GPIO_DATA_START, gpio_base + GPSET0);
    GPIO_CLK_0;
    GPIO_CLK_1;
}


static inline uint8_t gpio_get_value8(void) {
    //iowrite32(GPIO_DATA_MASK << GPIO_DATA_START, gpio_base + GPSET0);
    uint8_t ret;
    GPIO_CLK_0;
    iowrite32(INPUT_DIR0, gpio_base + GPSEL0);
    ret = (uint8_t)((ioread32(gpio_base + GPLEV0) >> GPIO_DATA_START));
    GPIO_CLK_1;
    return ret;
}

static inline uint8_t gpio_get_value16(void) {
    uint8_t ret;
    GPIO_CLK_0;
    iowrite32(INPUT_DIR0, gpio_base + GPSEL0);
    iowrite32(INPUT_DIR1, gpio_base + GPSEL1);
    ret = (uint16_t)((ioread32(gpio_base + GPLEV0) >> GPIO_DATA_START));
    GPIO_CLK_1;
    return ret;
}


static void gpio_bit_bang(uint8_t cmd, uint16_t addr, uint8_t data, uint8_t *read_data) {
    uint16_t value;
    uint8_t retry = 0x5;

    // Assert CS
    GPIO_CS_0;
    gpio_set_value16(addr);
    gpio_set_value16(cmd << 8 | data);
    if (cmd < 4) {
        do {
            value = gpio_get_value16();
            if (value & WAIT)
                break;
        } while(retry--);
    } else if (cmd == CMD_STATUS) {
        value = gpio_get_value16();
    }
    GPIO_CS_1;
    if (read_data)
        *read_data = value;
}

// Add these includes and definitions at the top
#define MSXBUS_IOC_MAGIC 'M'
#define MSXBUS_IOCREAD    _IOWR(MSXBUS_IOC_MAGIC, 0, struct msxbus_transfer)
#define MSXBUS_IOCWRITE   _IOW(MSXBUS_IOC_MAGIC, 1, struct msxbus_transfer)
#define MSXBUS_IOCRESET   _IO(MSXBUS_IOC_MAGIC, 5)
#define MSXBUS_IOCSTATUS  _IOR(MSXBUS_IOC_MAGIC, 8, uint8_t)

// Add structure definition
struct msxbus_transfer {
    uint8_t cmd;
    uint16_t addr;
    uint8_t data;
};

// Add ioctl handler
static long msxbus_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct msxbus_transfer xfer;
    uint8_t status;
    int ret = sizeof(xfer);

    switch (cmd) {
        case MSXBUS_IOCREAD:
            if (copy_from_user(&xfer, (void __user *)arg, sizeof(xfer)))
                return -EFAULT;
            gpio_bit_bang(xfer.cmd, xfer.addr, 0, &xfer.data);
            if (copy_to_user((void __user *)arg, &xfer, sizeof(xfer)))
                return -EFAULT;
            break;

        case MSXBUS_IOCWRITE:
            if (copy_from_user(&xfer, (void __user *)arg, sizeof(xfer)))
                return -EFAULT;
            gpio_bit_bang(xfer.cmd, xfer.addr, xfer.data, NULL);
            break;

        case MSXBUS_IOCRESET:
            gpio_bit_bang(CMD_RESET, 0, 0, NULL);
            break;

        case MSXBUS_IOCSTATUS:
            gpio_bit_bang(CMD_STATUS, 0, 0, &status);
            if (copy_to_user((void __user *)arg, &status, sizeof(status)))
                return -EFAULT;
            break;

        default:
            ret = -ENOTTY;
            break;
    }

    return ret;
}

// Modify file operations structure
static const struct file_operations msxbus_fops = {
    .owner = THIS_MODULE,
    // .read = msxbus_read,
    // .write = msxbus_write,
    .unlocked_ioctl = msxbus_ioctl,    // Add ioctl handler
};

// Add these at the top with other includes
#include <linux/cdev.h>
#include <linux/device.h>

// Add these as global variables
static dev_t msxbus_dev;
static struct cdev msxbus_cdev;
static struct class *msxbus_class;

// Add near the top of the file
#define DEV_CLASS_MODE ((umode_t)(S_IRUGO|S_IWUGO))

// Add before msxbus_init
static char *msxbus_devnode(struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = DEV_CLASS_MODE;
    return NULL;
}

// Modify msxbus_init function
static int __init msxbus_init(void) {
    int ret;
    int i;
    struct device *device;

    gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!gpio_base) {
        pr_err("msxbus: failed to ioremap GPIO base\n");
        return -ENOMEM;
    }

    // Allocate character device region
    ret = alloc_chrdev_region(&msxbus_dev, 0, 1, "msxbus");
    if (ret < 0) {
        pr_err("msxbus: failed to allocate character device region\n");
        iounmap(gpio_base);
        return ret;
    }

    // Initialize character device
    cdev_init(&msxbus_cdev, &msxbus_fops);
    msxbus_cdev.owner = THIS_MODULE;

    // Add character device to system
    ret = cdev_add(&msxbus_cdev, msxbus_dev, 1);
    if (ret < 0) {
        pr_err("msxbus: failed to add character device\n");
        unregister_chrdev_region(msxbus_dev, 1);
        iounmap(gpio_base);
        return ret;
    }

    // Create device class
    msxbus_class = class_create(THIS_MODULE, "msxbus");
    if (IS_ERR(msxbus_class)) {
        pr_err("msxbus: failed to create device class\n");
        cdev_del(&msxbus_cdev);
        unregister_chrdev_region(msxbus_dev, 1);
        iounmap(gpio_base);
        return PTR_ERR(msxbus_class);
    }
    msxbus_class->devnode = msxbus_devnode;

    // Create device node
    device = device_create(msxbus_class, NULL, msxbus_dev, NULL, "msxbus");
    if (IS_ERR(device)) {
        pr_err("msxbus: failed to create device\n");
        class_destroy(msxbus_class);
        cdev_del(&msxbus_cdev);
        unregister_chrdev_region(msxbus_dev, 1);
        iounmap(gpio_base);
        return PTR_ERR(msxbus_class);
    }

    gpio_request(GPIO_CS, "msxbus_cs");
    gpio_request(GPIO_CLK, "msxbus_clk");
    for ( i = GPIO_DATA_START; i <= GPIO_DATA_END; ++i) {
        gpio_request(i, "msxbus_data");
    }

    gpio_direction_output(GPIO_CS, 1);
    gpio_direction_output(GPIO_CLK, 1);
    for ( i = GPIO_DATA_START; i <= GPIO_DATA_END; ++i) {
        gpio_direction_output(i, 0);
    }

    pr_info("msxbus: initialized\n");
    return 0;
}

// Modify msxbus_exit function
static void __exit msxbus_exit(void) {
    int i;
    
    device_destroy(msxbus_class, msxbus_dev);
    class_destroy(msxbus_class);
    cdev_del(&msxbus_cdev);
    unregister_chrdev_region(msxbus_dev, 1);
    iounmap(gpio_base);

    gpio_free(GPIO_CS);
    gpio_free(GPIO_CLK);
    for (i = GPIO_DATA_START; i <= GPIO_DATA_END; ++i) {
        gpio_free(i);
    }

    pr_info("msxbus: exited\n");
}

module_init(msxbus_init);
module_exit(msxbus_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MSXBUS GPIO bit-banging driver");
MODULE_AUTHOR("meesokim");
