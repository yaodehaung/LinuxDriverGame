#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>         // 提供 file_operations 結構
#include <linux/cdev.h>       // 提供 cdev 結構
#include <linux/device.h>     // 提供 class_create, device_create
#include <linux/uaccess.h>    // copy_to_user, copy_from_user

#define DEVICE_NAME "mychardev"

static dev_t dev_num;             // 存放主次編號
static struct cdev my_cdev;       // cdev 結構
static struct class *my_class;    // 用來創建 /dev 的 class

// 開啟裝置時呼叫
static int my_open(struct inode *inode, struct file *file)
{
    pr_info("mychardev: device opened\n");
    return 0;
}

// 關閉裝置時呼叫
static int my_release(struct inode *inode, struct file *file)
{
    pr_info("mychardev: device closed\n");
    return 0;
}

// 讀取裝置時呼叫
static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    char msg[] = "Hello from kernel!\n";
    int msg_len = sizeof(msg);

    // 若 offset 超過一次長度，回傳 0 表示 EOF
    if (*offset >= msg_len)
        return 0;

    // 如果使用者請求長度大於我們有的，就只複製我們的長度
    if (len > msg_len - *offset)
        len = msg_len - *offset;

    // 從核心複製到使用者空間
    if (copy_to_user(buf, msg + *offset, len))
        return -EFAULT;

    *offset += len;
    pr_info("mychardev: read %zu bytes\n", len);
    return len;
}

// 寫入裝置時呼叫
static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    char kbuf[128] = {0};

    // 限制最多 127 bytes
    if (len > 127)
        len = 127;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';
    pr_info("mychardev: received from user: %s\n", kbuf);

    return len;
}

// 定義 file_operations 結構
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

// 模組初始化
static int __init my_init(void)
{
    int ret;

    // 分配主次編號，第一個次編號是 0，只需要 1 個裝置
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("failed to allocate chrdev region\n");
        return ret;
    }
    pr_info("mychardev: major=%d minor=%d\n", MAJOR(dev_num), MINOR(dev_num));

    // 初始化 cdev，並指定 fops
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // 註冊 cdev 到系統
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // 創建 class（會在 /sys/class 下看到）
    my_class = class_create("my_class");
    if (IS_ERR(my_class)) {
        pr_err("failed to create class\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }

    // 創建裝置節點 /dev/mychardev
    device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);

    pr_info("mychardev: module loaded\n");
    return 0;
}

// 模組卸載
static void __exit my_exit(void)
{
    // 移除 /dev 裝置節點
    device_destroy(my_class, dev_num);
    // 刪除 class
    class_destroy(my_class);
    // 刪除 cdev
    cdev_del(&my_cdev);
    // 釋放主次編號
    unregister_chrdev_region(dev_num, 1);

    pr_info("mychardev: module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("你的名字");
MODULE_DESCRIPTION("簡單字元驅動程式完整範例");
