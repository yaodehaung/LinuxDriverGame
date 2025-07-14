#include <linux/module.h>
#include <linux/init.h>
#include "mylib.h"

MODULE_LICENSE("GPL");

static int __init main_init(void) {
    int result = add(10, 20);
    pr_info("main: 10 + 20 = %d\n", result);
    return 0;
}

static void __exit main_exit(void) {
    pr_info("main module exiting\n");
}

module_init(main_init);
module_exit(main_exit);
