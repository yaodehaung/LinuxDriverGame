#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/circ_buf.h>
#include <linux/uaccess.h>

#define BUF_SIZE 128  // 環形緩衝區大小
#define DEVICE_NAME "circ_buf"

static struct circ_buf my_circ_buf;
static dev_t dev_num;
static struct cdev my_cdev;

// **寫入函式：支援 `echo "data" > /dev/circ_buf`**
static ssize_t circ_buf_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int free_space = CIRC_SPACE(my_circ_buf.head, my_circ_buf.tail, BUF_SIZE);
    char kbuf[BUF_SIZE];
    int i;

    if (count > free_space)
        return -ENOSPC;  // 空間不足

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    for (i = 0; i < count; i++) {
        my_circ_buf.buf[my_circ_buf.head] = kbuf[i];
        my_circ_buf.head = (my_circ_buf.head + 1) & (BUF_SIZE - 1);
    }
    return count;
}

// **讀取函式：支援 `cat /dev/circ_buf`**
static ssize_t circ_buf_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int available = CIRC_CNT(my_circ_buf.head, my_circ_buf.tail, BUF_SIZE);
    char kbuf[BUF_SIZE];
    int i;

    if (count > available)
        count = available;

    if (count == 0)
        return 0; // 沒有可讀取的數據

    for (i = 0; i < count; i++) {
        kbuf[i] = my_circ_buf.buf[my_circ_buf.tail];
        my_circ_buf.tail = (my_circ_buf.tail + 1) & (BUF_SIZE - 1);
    }

    if (copy_to_user(buf, kbuf, count))
        return -EFAULT;

    return count;
}

// **設備開啟**
static int circ_buf_open(struct inode *inode, struct file *file)
{
    return 0;
}

// **設備關閉**
static int circ_buf_release(struct inode *inode, struct file *file)
{
    return 0;
}

// **文件操作結構**
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = circ_buf_open,
    .release = circ_buf_release,
    .read = circ_buf_read,
    .write = circ_buf_write,
};

// **模組初始化**
static int __init circ_buf_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return -1;
    }

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    // 初始化環形緩衝區
    my_circ_buf.buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!my_circ_buf.buf) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }
    my_circ_buf.head = 0;
    my_circ_buf.tail = 0;

    printk(KERN_INFO "Circular buffer device initialized as /dev/%s\n", DEVICE_NAME);
    return 0;
}

// **模組卸載**
static void __exit circ_buf_exit(void)
{
    kfree(my_circ_buf.buf);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "Circular buffer device removed\n");
}

module_init(circ_buf_init);
module_exit(circ_buf_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("Circular Buffer Device Driver");

