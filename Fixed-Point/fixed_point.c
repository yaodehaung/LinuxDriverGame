#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

typedef int32_t fixed_t;  // Q16.16 格式
#define FIXED_SCALE (1 << 16) // 65536

// 直接使用整數初始化定點數
#define INT_TO_FIXED(x) ((x) << 16)   // 轉 Q16.16
#define FIXED_TO_INT(x) ((x) >> 16)   // 轉回整數

// Q16.16 乘法
static inline fixed_t fixed_mul(fixed_t a, fixed_t b) {
    return ((int64_t)a * b) >> 16;
}

// Q16.16 除法
static inline fixed_t fixed_div(fixed_t a, fixed_t b) {
    return ((int64_t)a << 16) / b;
}

static int __init fixed_point_init(void) {
    fixed_t a = INT_TO_FIXED(3);  // 3.0 轉定點數
    fixed_t b = INT_TO_FIXED(2);  // 2.0 轉定點數

    fixed_t result_mul = fixed_mul(a, b);
    fixed_t result_div = fixed_div(a, b);

    printk(KERN_INFO "Fixed-Point Math in Kernel:\n");
    printk(KERN_INFO "3.0 * 2.0 = %d.%04d\n", FIXED_TO_INT(result_mul), 
           (result_mul & 0xFFFF) * 10000 / 65536);
    printk(KERN_INFO "3.0 / 2.0 = %d.%04d\n", FIXED_TO_INT(result_div), 
           (result_div & 0xFFFF) * 10000 / 65536);

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
