// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bitmap.h>

#define N_BITS 16   // 我們測試 16 個 bits

static int __init bitmap_test_init(void)
{
    DECLARE_BITMAP(my_bitmap, N_BITS);

    pr_info("=== Bitmap 測試開始 ===\n");

    /* 1. 先清空 bitmap */
    bitmap_zero(my_bitmap, N_BITS);
    pr_info("Step1: bitmap_zero -> bitmap_empty = %d\n",
            bitmap_empty(my_bitmap, N_BITS));

    /* 2. 設定第 3 個 bit (從 0 開始算) */
    set_bit(3, my_bitmap);
    pr_info("Step2: set_bit(3) -> test_bit(3) = %d\n",
            test_bit(3, my_bitmap));

    /* 3. 全部設成 1 */
    bitmap_fill(my_bitmap, N_BITS);
    pr_info("Step3: bitmap_fill -> bitmap_full = %d\n",
            bitmap_full(my_bitmap, N_BITS));

    /* 4. 清掉第 5 個 bit */
    clear_bit(5, my_bitmap);
    pr_info("Step4: clear_bit(5) -> bitmap_full = %d, test_bit(5) = %d\n",
            bitmap_full(my_bitmap, N_BITS),
            test_bit(5, my_bitmap));

    /* 5. 再次清空 */
    bitmap_clear(my_bitmap, 0, N_BITS);
    pr_info("Step5: bitmap_clear -> bitmap_empty = %d\n",
            bitmap_empty(my_bitmap, N_BITS));

    pr_info("=== Bitmap 測試結束 ===\n");
    return 0;
}

static void __exit bitmap_test_exit(void)
{
    pr_info("bitmap_test module exit\n");
}

module_init(bitmap_test_init);
module_exit(bitmap_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("Bitmap API 測試範例");

