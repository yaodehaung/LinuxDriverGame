#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>      // for kmem_cache APIs
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("Simple Slab Allocator Example");

struct my_object {
    int id;
    char name[32];
};

static struct kmem_cache *my_cache = NULL;
static struct my_object *obj1 = NULL;

static int __init slab_example_init(void)
{
    pr_info("Slab example module init\n");

    // 建立一個 cache，每個 object 大小為 struct my_object
    my_cache = kmem_cache_create("my_object_cache",
                                 sizeof(struct my_object),
                                 0,            // alignment，0 表示自動
                                 SLAB_HWCACHE_ALIGN, // flags
                                 NULL);        // optional constructor
    if (!my_cache) {
        pr_err("kmem_cache_create failed\n");
        return -ENOMEM;
    }

    // From cache  assign one object 
    obj1 = kmem_cache_alloc(my_cache, GFP_KERNEL);
    if (!obj1) {
        pr_err("kmem_cache_alloc failed\n");
        kmem_cache_destroy(my_cache);
        return -ENOMEM;
    }

    obj1->id = 1;
    strncpy(obj1->name, "test object", sizeof(obj1->name));
    pr_info("Allocated object: id=%d, name=%s\n", obj1->id, obj1->name);

    return 0;
}

static void __exit slab_example_exit(void)
{
    pr_info("Slab example module exit\n");

    if (obj1)
        kmem_cache_free(my_cache, obj1);

    if (my_cache)
        kmem_cache_destroy(my_cache);
}

module_init(slab_example_init);
module_exit(slab_example_exit);
