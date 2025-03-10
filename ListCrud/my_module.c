#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>

struct my_device {
    int id;
    struct list_head list;
};

static LIST_HEAD(device_list);  // 初始化鏈結串列

// **Create：新增設備**
void add_device(int id) {
    struct my_device *dev = kmalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev) {
        printk(KERN_ERR "Failed to allocate memory for device\n");
        return;
    }

    dev->id = id;
    list_add_tail(&dev->list, &device_list);
    printk(KERN_INFO "Added Device ID: %d\n", id);
}

// **Read：顯示設備列表**
void show_devices(void) {
    struct my_device *dev;
    printk(KERN_INFO "Current Devices:\n");
    list_for_each_entry(dev, &device_list, list) {
        printk(KERN_INFO "  Device ID: %d\n", dev->id);
    }
}

// **Update：更新設備 ID**
void update_device(int old_id, int new_id) {
    struct my_device *dev;
    list_for_each_entry(dev, &device_list, list) {
        if (dev->id == old_id) {
            dev->id = new_id;
            printk(KERN_INFO "Updated Device ID: %d -> %d\n", old_id, new_id);
            return;
        }
    }
    printk(KERN_INFO "Device ID %d not found\n", old_id);
}

// **Delete：刪除設備**
void delete_device(int id) {
    struct my_device *dev, *tmp;
    list_for_each_entry_safe(dev, tmp, &device_list, list) {
        if (dev->id == id) {
            list_del(&dev->list);
            kfree(dev);
            printk(KERN_INFO "Deleted Device ID: %d\n", id);
            return;
        }
    }
    printk(KERN_INFO "Device ID %d not found\n", id);
}

// **清除所有設備**
void cleanup_devices(void) {
    struct my_device *dev, *tmp;
    list_for_each_entry_safe(dev, tmp, &device_list, list) {
        list_del(&dev->list);
        kfree(dev);
    }
    printk(KERN_INFO "All devices removed\n");
}

// **模組載入時執行**
static int __init my_module_init(void) {
    printk(KERN_INFO "Module loaded\n");

    add_device(1);
    add_device(2);
    add_device(3);

    show_devices();

    update_device(2, 20);
    show_devices();

    delete_device(1);
    show_devices();

    return 0;
}

// **模組卸載時執行**
static void __exit my_module_exit(void) {
    cleanup_devices();
    printk(KERN_INFO "Module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("Linux list_for_each_entry_safe CRUD Example");

