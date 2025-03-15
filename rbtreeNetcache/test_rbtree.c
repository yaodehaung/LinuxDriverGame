#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "rbtree"

// 定義 Red-Black Tree 的節點
struct my_node {
    struct rb_node node;
    int key;
};

// Red-Black Tree
static struct rb_root my_tree = RB_ROOT;

// 插入節點
static struct my_node *rbtree_insert(int key)
{
    struct rb_node **new = &(my_tree.rb_node), *parent = NULL;
    struct my_node *data;

    while (*new) {
        struct my_node *this = container_of(*new, struct my_node, node);
        parent = *new;
        
        if (key < this->key)
            new = &((*new)->rb_left);
        else if (key > this->key)
            new = &((*new)->rb_right);
        else
            return NULL;  // Key 已存在
    }

    data = kmalloc(sizeof(struct my_node), GFP_KERNEL);
    if (!data)
        return NULL;

    data->key = key;
    rb_link_node(&data->node, parent, new);
    rb_insert_color(&data->node, &my_tree);

    return data;
}

// 刪除節點
static int rbtree_delete(int key)
{
    struct rb_node *node = my_tree.rb_node;

    while (node) {
        struct my_node *data = container_of(node, struct my_node, node);

        if (key < data->key)
            node = node->rb_left;
        else if (key > data->key)
            node = node->rb_right;
        else {
            rb_erase(&data->node, &my_tree);
            kfree(data);
            return 1;  // 成功刪除
        }
    }
    return 0;  // 找不到 key
}

// 釋放所有節點
static void rbtree_free(void)
{
    struct rb_node *node;
    struct my_node *data;

    while ((node = rb_first(&my_tree))) {
        data = container_of(node, struct my_node, node);
        rb_erase(node, &my_tree);
        kfree(data);
    }
}

// `cat /proc/rbtree` 會列出所有節點
static int rbtree_show(struct seq_file *m, void *v)
{
    struct rb_node *node;
    struct my_node *data;

    seq_printf(m, "Red-Black Tree Contents:\n");
    for (node = rb_first(&my_tree); node; node = rb_next(node)) {
        data = container_of(node, struct my_node, node);
        seq_printf(m, "%d\n", data->key);
    }
    return 0;
}

// `seq_file` 介面
static int rbtree_open(struct inode *inode, struct file *file)
{
    return single_open(file, rbtree_show, NULL);
}

// `/proc/rbtree` 寫入處理 (用於插入/刪除節點)
static ssize_t rbtree_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char input[32];
    int key;
    
    if (count > sizeof(input) - 1)
        return -EINVAL;

    if (copy_from_user(input, buffer, count))
        return -EFAULT;

    input[count] = '\0';

    if (sscanf(input, "add %d", &key) == 1) {
        rbtree_insert(key);
    } else if (sscanf(input, "del %d", &key) == 1) {
        rbtree_delete(key);
    }

    return count;
}

// `/proc/rbtree` 檔案的 file_operations
static const struct proc_ops rbtree_fops = {
    .proc_open    = rbtree_open,
    .proc_read    = seq_read,
    .proc_write   = rbtree_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

// 模組初始化
static int __init rbtree_init(void)
{
    proc_create(PROC_NAME, 0666, NULL, &rbtree_fops);

    // 測試插入初始數據
    rbtree_insert(10);
    rbtree_insert(20);
    rbtree_insert(15);
    rbtree_insert(30);
    
    printk(KERN_INFO "rbtree_driver loaded.\n");
    return 0;
}

// 模組卸載
static void __exit rbtree_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    rbtree_free();
    printk(KERN_INFO "rbtree_driver unloaded.\n");
}

module_init(rbtree_init);
module_exit(rbtree_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("A simple Linux driver using Red-Black Tree with delete function");
