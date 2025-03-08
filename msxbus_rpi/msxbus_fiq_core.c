#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <asm/fiq.h>
#include <asm/io.h>

#define BCM2837_PERI_BASE    0x3F000000
#define GPIO_BASE            (BCM2837_PERI_BASE + 0x200000)
#define GPIO_SIZE            0xB4

#define GPIO_CLK     8
#define GPIO_CS      9
#define GPIO_DATA_START  0
#define GPIO_DATA_END   7
#define GPIO_FIQ_PIN    25  // FIQ 인터럽트용 GPIO 핀

static void __iomem *gpio_base;
static struct fiq_handler fh = {
    .name = "msxbus_fiq"
};
// FIQ 처리를 위한 상태 변수들 - 전역 심볼로 선언
int fiq_state __attribute__((aligned(4)));
uint8_t cmd_byte __attribute__((aligned(4)));
uint16_t addr __attribute__((aligned(4)));
uint8_t data_byte __attribute__((aligned(4)));
// 메모리 배열도 전역 심볼로 선언
char mem[1024*1024*2] __attribute__((aligned(4)));
// 심볼들을 어셈블리에서 접근 가능하도록 export
EXPORT_SYMBOL(fiq_state);
EXPORT_SYMBOL(cmd_byte);
EXPORT_SYMBOL(addr);
EXPORT_SYMBOL(data_byte);
EXPORT_SYMBOL(mem);
extern uint8_t msxbus_read(void);
extern void msxbus_write(void);

// 함수 구현
uint8_t msxbus_read(void) {
    uint16_t address = addr;
    return mem[address];
}

void msxbus_write(void) {
    uint16_t address = addr;
    mem[address] = data_byte;
}

// FIQ 핸들러 (어셈블리로 구현)
extern void msxbus_fiq_handler(void);

// GPIO 제어 함수들
void gpio_set_value8(uint8_t value) {
    iowrite32((ioread32(gpio_base + 0x1C) & ~0xFF) | value, gpio_base + 0x1C);
}

uint8_t gpio_get_value8(void) {
    return (uint8_t)(ioread32(gpio_base + 0x34) & 0xFF);
}

// FIQ 초기화
static int init_fiq(void) {
    int ret;
    // struct pt_regs regs;
    cpumask_t mask;

    // FIQ 핸들러 등록
    ret = claim_fiq(&fh);
    if (ret) {
        pr_err("Failed to claim FIQ\n");
        return ret;
    }

    // FIQ 핸들러 설정
    set_fiq_handler(msxbus_fiq_handler, 0x100);

    // FIQ를 특정 CPU 코어(0번)에 할당
    cpumask_clear(&mask);
    cpumask_set_cpu(0, &mask);
    set_cpus_allowed_ptr(current, &mask);

    // GPIO FIQ 설정
    gpio_direction_input(GPIO_CS);
    enable_fiq(gpio_to_irq(GPIO_CS));

    return 0;
}

static int __init msxbus_fiq_init(void) {
    int ret;
    int i;

    // GPIO 메모리 매핑
    gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!gpio_base) {
        pr_err("Failed to map GPIO\n");
        return -ENOMEM;
    }

    // GPIO 핀 설정
    gpio_request(GPIO_CLK, "msxbus_clk");
    gpio_request(GPIO_CS, "msxbus_cs");
    for (i = GPIO_DATA_START; i <= GPIO_DATA_END; i++) {
        gpio_request(i, "msxbus_data");
    }

    gpio_direction_input(GPIO_CLK);
    gpio_direction_input(GPIO_CS);
    for (i = GPIO_DATA_START; i <= GPIO_DATA_END; i++) {
        gpio_direction_output(i, 0);
    }

    // FIQ 초기화
    ret = init_fiq();
    if (ret) {
        iounmap(gpio_base);
        return ret;
    }

    pr_info("MSXBus FIQ module initialized\n");
    return 0;
}

static void __exit msxbus_fiq_exit(void) {
    int i;

    release_fiq(&fh);
    iounmap(gpio_base);

    gpio_free(GPIO_CLK);
    gpio_free(GPIO_CS);
    for (i = GPIO_DATA_START; i <= GPIO_DATA_END; i++) {
        gpio_free(i);
    }

    pr_info("MSXBus FIQ module removed\n");
}

module_init(msxbus_fiq_init);
module_exit(msxbus_fiq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("MSX Bus FIQ Handler");