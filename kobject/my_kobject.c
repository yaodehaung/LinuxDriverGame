#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/slab.h>

struct my_kobject {
    struct kobject kobj;
    int value;
};

// 釋放函數
static void my_object_release(struct kobject *kobj) {
    struct my_kobject *my_obj = container_of(kobj, struct my_kobject, kobj);
    printk(KERN_INFO "my_object_release: Freeing my_kobject\n");
    kfree(my_obj);
}

// sysfs 屬性讀取 (手動定義)
static ssize_t my_object_show(struct kobject *kobj, struct attribute *attr, char *buf) {
    struct my_kobject *my_obj = container_of(kobj, struct my_kobject, kobj);
    return sprintf(buf, "%d\n", my_obj->value);
}

// sysfs 屬性寫入 (手動定義)
static ssize_t my_object_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count) {
    struct my_kobject *my_obj = container_of(kobj, struct my_kobject, kobj);
    sscanf(buf, "%d", &my_obj->value);
    return count;
}

// 定義 sysfs_ops
static struct sysfs_ops my_sysfs_ops = {
    .show = my_object_show,
    .store = my_object_store,
};

// 定義 sysfs 屬性
static struct attribute my_attr = {
    .name = "my_value",
    .mode = 0666,
};

// 建立 sysfs 屬性群組
static struct attribute *my_attrs[] = {
    &my_attr,
    NULL, // 結束標記
};

// 定義 attribute_group
static struct attribute_group my_attr_group = {
    .attrs = my_attrs,
};

// kobject_type
static struct kobj_type my_kobj_type = {
    .release = my_object_release,
    .sysfs_ops = &my_sysfs_ops,
    .default_groups = (const struct attribute_group *[]){ &my_attr_group, NULL },
};

// 根 kobject
static struct kobject *root_kobj;

static int __init my_kobject_init(void) {
    struct my_kobject *my_obj;

    // 配置記憶體
    my_obj = kzalloc(sizeof(*my_obj), GFP_KERNEL);
    if (!my_obj)
        return -ENOMEM;

    // 初始化 kobject
    kobject_init(&my_obj->kobj, &my_kobj_type);

    // 建立 `/sys/kernel/my_kobject/`
    if (kobject_add(&my_obj->kobj, kernel_kobj, "my_kobject")) {
        kobject_put(&my_obj->kobj);
        return -ENOMEM;
    }

    root_kobj = &my_obj->kobj;
    printk(KERN_INFO "my_kobject created successfully\n");
    return 0;
}

static void __exit my_kobject_exit(void) {
    if (root_kobj) {
        kobject_put(root_kobj); // 釋放 kobject，會觸發 `my_object_release`
        root_kobj = NULL;
    }
    printk(KERN_INFO "my_kobject removed\n");
}

module_init(my_kobject_init);
module_exit(my_kobject_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("A kobject example with sysfs_ops");
