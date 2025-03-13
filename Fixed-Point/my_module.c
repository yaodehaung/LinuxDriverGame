#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

typedef int32_t fixed_t;  // Q16.16 格式
#define FIXED_SCALE (1 << 16) // 2^16 = 65536

// 浮點數轉 Q16.16
static inline fixed_t float_to_fixed(float x) {
    return (fixed_t)(x * FIXED_SCALE);
}

// Q16.16 轉回浮點數
static inline float fixed_to_float(fixed_t x) {
    return (float)x / FIXED_SCALE;
}

// Q16.16 乘法
static inline fixed_t fixed_mul(fixed_t a, fixed_t b) {
    return ((int64_t)a * b) >> 16;
}

// Q16.16 除法
static inline fixed_t fixed_div(fixed_t a, fixed_t b) {
    return ((int64_t)a << 16) / b;
}

static int __init fixed_point_init(void) {
    fixed_t a = float_to_fixed(3.5);
    fixed_t b = float_to_fixed(2.0);

    fixed_t result_mul = fixed_mul(a, b);
    fixed_t result_div = fixed_div(a, b);

    printk(KERN_INFO "Fixed-Point Math in Kernel:\n");
    printk(KERN_INFO "3.5 * 2.0 = %f\n", fixed_to_float(result_mul));
    printk(KERN_INFO "3.5 / 2.0 = %f\n", fixed_to_float(result_div));

    return 0;
}

static void __exit fixed_point_exit(void) {
    printk(KERN_INFO "Fixed-Point Kernel Module Removed\n");
}

module_init(fixed_point_init);
module_exit(fixed_point_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("Fixed-Point Math in Linux Kernel");
