#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#define GPIO_FIQ_PIN1 19
#define GPIO_FIQ_PIN2 17
#define GPIO_OUT_PIN  24

static irqreturn_t gpio_fiq_handler(int irq, void *dev_id);

static int __init msxbus_fiq_init(void)
{
    int ret;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING;

    // Try to use FIQ if available, otherwise fall back to regular IRQ
    if (irq_set_irq_type(gpio_to_irq(GPIO_FIQ_PIN1), IRQ_TYPE_LEVEL_LOW) == 0 &&
        irq_set_irq_type(gpio_to_irq(GPIO_FIQ_PIN2), IRQ_TYPE_LEVEL_LOW) == 0) {
        irq_flags |= IRQF_NO_THREAD;
        printk(KERN_INFO "Using FIQ mode\n");
    } else {
        printk(KERN_INFO "FIQ not available, using regular IRQ mode\n");
    }

    // Configure GPIO pins
    ret = gpio_request(GPIO_FIQ_PIN1, "msxbus_fiq1");
    if (ret) {
        printk(KERN_ERR "Failed to request GPIO %d\n", GPIO_FIQ_PIN1);
        return ret;
    }

    ret = gpio_request(GPIO_FIQ_PIN2, "msxbus_fiq2");
    if (ret) {
        gpio_free(GPIO_FIQ_PIN1);
        printk(KERN_ERR "Failed to request GPIO %d\n", GPIO_FIQ_PIN2);
        return ret;
    }

    ret = gpio_request(GPIO_OUT_PIN, "msxbus_out");
    if (ret) {
        gpio_free(GPIO_FIQ_PIN1);
        gpio_free(GPIO_FIQ_PIN2);
        printk(KERN_ERR "Failed to request GPIO %d\n", GPIO_OUT_PIN);
        return ret;
    }

    // Set GPIO directions
    gpio_direction_input(GPIO_FIQ_PIN1);
    gpio_direction_input(GPIO_FIQ_PIN2);
    gpio_direction_output(GPIO_OUT_PIN, 1);

    // Request interrupts with appropriate flags
    ret = request_irq(gpio_to_irq(GPIO_FIQ_PIN1), 
                     (irq_handler_t)gpio_fiq_handler,
                     irq_flags,
                     "msxbus_fiq1",
                     NULL);
    if (ret) {
        printk(KERN_ERR "Failed to request IRQ for GPIO %d\n", GPIO_FIQ_PIN1);
        goto cleanup;
    }

    ret = request_irq(gpio_to_irq(GPIO_FIQ_PIN2),
                     (irq_handler_t)gpio_fiq_handler,
                     irq_flags,
                     "msxbus_fiq2",
                     NULL);
    if (ret) {
        printk(KERN_ERR "Failed to request IRQ for GPIO %d\n", GPIO_FIQ_PIN2);
        free_irq(gpio_to_irq(GPIO_FIQ_PIN1), NULL);
        goto cleanup;
    }

    printk(KERN_INFO "MSXBUS FIQ module initialized\n");
    return 0;

cleanup:
    gpio_free(GPIO_FIQ_PIN1);
    gpio_free(GPIO_FIQ_PIN2);
    gpio_free(GPIO_OUT_PIN);
    return ret;
}

asmlinkage void __naked gpio_fiq_handler_asm(void);
static irqreturn_t gpio_fiq_handler(int irq, void *dev_id)
{
    gpio_fiq_handler_asm();
    return IRQ_HANDLED;
}

// ARM assembly implementation of the FIQ handler
__asm__(
    ".text\n"
    ".global gpio_fiq_handler_asm\n"
    "gpio_fiq_handler_asm:\n"
    "    push    {r0-r3, lr}\n"
    "    mov     r0, #24\n"          // GPIO 24
    "    mov     r1, #0\n"           // Value = 0
    "    ldr     r2, =0x3F200000\n"  // GPIO base address for RPi
    "    mov     r3, r0, lsr #5\n"   // Get GPIO register offset (GPIO/32)
    "    mov     r0, #1\n"
    "    lsl     r0, r0, r1\n"       // Create bit mask
    "    str     r0, [r2, #40]\n"    // Clear GPIO (GPCLR0 offset = 40)
    "    pop     {r0-r3, pc}\n"
);

static void __exit msxbus_fiq_exit(void)
{
    free_irq(gpio_to_irq(GPIO_FIQ_PIN1), NULL);
    free_irq(gpio_to_irq(GPIO_FIQ_PIN2), NULL);
    gpio_free(GPIO_FIQ_PIN1);
    gpio_free(GPIO_FIQ_PIN2);
    gpio_free(GPIO_OUT_PIN);
    printk(KERN_INFO "MSXBUS FIQ module removed\n");
}

module_init(msxbus_fiq_init);
module_exit(msxbus_fiq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("MSX Bus FIQ Handler for GPIO");
