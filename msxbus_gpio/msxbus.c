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

#define GPIO_DATA_MASK 0xFF

#define GPIO_CS 9
#define GPIO_CLK 8
#define GPIO_DATA_START 0
#define GPIO_DATA_END 7

// Command definitions
#define CMD_MEM_READ    0x01
#define CMD_MEM_WRITE   0x02
#define CMD_IO_READ     0x03
#define CMD_IO_WRITE    0x04
#define CMD_RESET       0x05
#define CMD_STATUS      0x08

// GPIO Registers
#define GPSET0 0x1c
#define GPCLR0 0x28
#define GPLEV0 0x34

static void __iomem *gpio_base;

static inline void gpio_set_value8(uint8_t value) {
    iowrite32(GPIO_DATA_MASK, gpio_base + GPCLR0);  
    iowrite32(value, gpio_base + GPSET0);
}

static inline uint8_t gpio_get_value8(void) {
    return (uint8_t)(ioread32(gpio_base + GPLEV0) & GPIO_DATA_MASK);
}

static void gpio_bit_bang(uint8_t cmd, uint16_t addr, uint8_t data, uint8_t *read_data) {
    uint8_t addr_low = addr & 0xFF;
    uint8_t addr_high = (addr >> 8) & 0xFF;
    uint8_t status;
    uint8_t retry = 0xff;

    // Assert CS
    gpio_set_value(GPIO_CLK, 1);
    gpio_set_value(GPIO_CS, 0);

    // Send command
    gpio_set_value8(cmd);
    gpio_set_value(GPIO_CLK, 0);
    udelay(1);
    gpio_set_value(GPIO_CLK, 1);
    udelay(1);

    // For non-RESET and non-STATUS commands, send address
    switch (cmd) {
        case CMD_MEM_READ:
        case CMD_IO_READ:
        case CMD_MEM_WRITE:
        case CMD_IO_WRITE:
            // Send low address byte
            gpio_set_value8(addr_low);
            gpio_set_value(GPIO_CLK, 0);
            udelay(1);
            gpio_set_value(GPIO_CLK, 1);
            udelay(1);

            // Send high address byte
            gpio_set_value8(addr_high);
            gpio_set_value(GPIO_CLK, 0);
            udelay(1);
            gpio_set_value(GPIO_CLK, 1);
            udelay(1);
            if (cmd & 0x01) {
                gpio_set_value8(data);
                gpio_set_value(GPIO_CLK, 0);
                udelay(1);
                gpio_set_value(GPIO_CLK, 1);                
            }
            udelay(20);
            // Wait for acknowledgment (0xFF)
            do {
                gpio_set_value(GPIO_CLK, 0);
                udelay(1);
                status = gpio_get_value8();
                gpio_set_value(GPIO_CLK, 1);
                if (status == 0xFF) {
                    if (!(cmd & 0x01)) {
                        gpio_set_value(GPIO_CLK, 0);
                        udelay(1);
                        *read_data = gpio_get_value8();
                        gpio_set_value(GPIO_CLK, 1);
                        udelay(1);
                    }
                }
                udelay(1);
            } while (retry--);
            break;
        case CMD_STATUS:
            gpio_set_value(GPIO_CLK, 0);
            udelay(1);
            *read_data = gpio_get_value8();
            gpio_set_value(GPIO_CLK, 1);
            udelay(1);        
            break;
        default:
            break;
    }

    // Deassert CS
    gpio_set_value(GPIO_CLK, 1);
    gpio_set_value(GPIO_CS, 1);
    udelay(1);
}

static ssize_t msxbus_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    uint16_t addr;
    uint8_t data;
    uint8_t cmd;

    if (len < 1) {
        return -EINVAL;
    }

    // Get command type from user
    if (copy_from_user(&cmd, buf, 1)) {
        return -EFAULT;
    }

    switch (cmd) {
        case CMD_MEM_READ:
        case CMD_IO_READ:
            if (len != 3) return -EINVAL;
            if (copy_from_user(&addr, buf + 1, 2)) return -EFAULT;
            gpio_bit_bang(cmd, addr, 0, &data);
            if (copy_to_user(buf, &data, 1)) return -EFAULT;
            return data;

        case CMD_STATUS:
            gpio_bit_bang(CMD_STATUS, 0, 0, &data);
            if (copy_to_user(buf, &data, 1)) return -EFAULT;
            return 1;

        case CMD_RESET:
            gpio_bit_bang(CMD_RESET, 0, 0, NULL);
            return 0;

        default:
            return -EINVAL;
    }
}

static ssize_t msxbus_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    struct {
        uint8_t cmd;
        uint16_t addr;
        uint8_t data;
    } val;

    if (len != 4) {
        return -EINVAL;
    }

    if (copy_from_user((char *)&val, buf, 4)) {
        return -EFAULT;
    }

    // if (cmd != CMD_MEM_WRITE && cmd != CMD_IO_WRITE) {
    //     return -EINVAL;
    // }

    // if (copy_from_user(&addr, buf + 1, 2)) {
    //     return -EFAULT;
    // }
    // if (copy_from_user(&data, buf + 3, 1)) {
    //     return -EFAULT;
    // }

    gpio_bit_bang(val.cmd, val.addr, val.data, NULL);
    return 4;
}

// Add these includes and definitions at the top
#define MSXBUS_IOC_MAGIC 'M'
#define MSXBUS_IOCMEMREAD    _IOWR(MSXBUS_IOC_MAGIC, 0, struct msxbus_transfer)
#define MSXBUS_IOCMEMWRITE   _IOW(MSXBUS_IOC_MAGIC, 1, struct msxbus_transfer)
#define MSXBUS_IOCIOREAD     _IOWR(MSXBUS_IOC_MAGIC, 2, struct msxbus_transfer)
#define MSXBUS_IOCIOWRITE    _IOW(MSXBUS_IOC_MAGIC, 3, struct msxbus_transfer)
#define MSXBUS_IOCRESET      _IO(MSXBUS_IOC_MAGIC, 5)
#define MSXBUS_IOCSTATUS     _IOR(MSXBUS_IOC_MAGIC, 8, uint8_t)

// Add structure definition
struct msxbus_transfer {
    uint16_t addr;
    uint8_t data;
};

// Add ioctl handler
static long msxbus_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct msxbus_transfer xfer;
    uint8_t status;
    int ret = 0;

    switch (cmd) {
        case MSXBUS_IOCMEMREAD:
        case MSXBUS_IOCIOREAD:
            if (copy_from_user(&xfer, (void __user *)arg, sizeof(xfer)))
                return -EFAULT;
            gpio_bit_bang(cmd == MSXBUS_IOCMEMREAD ? CMD_MEM_READ : CMD_IO_READ,
                         xfer.addr, 0, &xfer.data);
            ret = xfer.data;
            if (copy_to_user((void __user *)arg, &xfer, sizeof(xfer)))
                return -EFAULT;
            break;

        case MSXBUS_IOCMEMWRITE:
        case MSXBUS_IOCIOWRITE:
            if (copy_from_user(&xfer, (void __user *)arg, sizeof(xfer)))
                return -EFAULT;
            gpio_bit_bang(cmd == MSXBUS_IOCMEMWRITE ? CMD_MEM_WRITE : CMD_IO_WRITE,
                         xfer.addr, xfer.data, NULL);
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
    .read = msxbus_read,
    .write = msxbus_write,
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
    gpio_direction_output(GPIO_CLK, 0);
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
