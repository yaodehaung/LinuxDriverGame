// mylib.c
#include <linux/module.h>
#include "mylib.h"

MODULE_LICENSE("GPL");


int add(int a, int b) {
    return a + b;
}
EXPORT_SYMBOL(add);
