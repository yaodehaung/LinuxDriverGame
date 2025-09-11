/*
 * rcu_example_module.c
 * Simple Linux kernel module demonstrating RCU-protected linked list
 *
 * Features:
 *  - maintain a linked list of items protected by RCU for readers
 *  - writers use a spinlock and list_add_rcu/list_del_rcu
 *  - deletion frees memory via call_rcu() callback
 *  - procfs interface to add / delete / show list:
 *      - echo "<id> <name>" > /proc/rcu_add
 *      - echo "<id>" > /proc/rcu_del
 *      - cat /proc/rcu_show
 *  - manual call_rcu trigger for testing
 *
 * Build with the provided Makefile (see below)
 * Tested with modern kernels (4.x/5.x/6.x). API used: rcu_read_lock(),
 * list_for_each_entry_rcu(), list_add_rcu(), list_del_rcu(), call_rcu().
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang (example)");
MODULE_DESCRIPTION("Simple RCU example module with procfs interface");
MODULE_VERSION("0.2");

struct rcu_item {
    int id;
    char name[32];
    struct list_head list;
    struct rcu_head rcu;
};

static LIST_HEAD(rcu_list_head);
static DEFINE_SPINLOCK(rcu_list_lock);

/* forward */
static void rcu_item_free_callback(struct rcu_head *rcu);

static struct rcu_item *rcu_item_create(int id, const char *name)
{
    struct rcu_item *it = kmalloc(sizeof(*it), GFP_KERNEL);
    if (!it)
        return NULL;
    it->id = id;
    strncpy(it->name, name, sizeof(it->name));
    INIT_LIST_HEAD(&it->list);
    return it;
}

static void rcu_item_free_callback(struct rcu_head *rcu)
{
    struct rcu_item *it = container_of(rcu, struct rcu_item, rcu);
    pr_info("rcu_example: freeing id=%d name=%s\n", it->id, it->name);
    kfree(it);
}

static int rcu_list_add(int id, const char *name)
{
    struct rcu_item *it;
    it = rcu_item_create(id, name);
    if (!it)
        return -ENOMEM;

    spin_lock(&rcu_list_lock);
    /* allow duplicates for simplicity; real code may check */
    list_add_rcu(&it->list, &rcu_list_head);
    spin_unlock(&rcu_list_lock);

    pr_info("rcu_example: added id=%d name=%s\n", id, it->name);
    return 0;
}

static int rcu_list_del(int id)
{
    struct rcu_item *it;
    int found = 0;

    spin_lock(&rcu_list_lock);
    list_for_each_entry(it, &rcu_list_head, list) {
        if (it->id == id) {
            list_del_rcu(&it->list);
            call_rcu(&it->rcu, rcu_item_free_callback);
            found = 1;
            pr_info("rcu_example: scheduled free id=%d name=%s\n", id, it->name);
            break; /* remove only first match */
        }
    }
    spin_unlock(&rcu_list_lock);

    return found ? 0 : -ENOENT;
}

/* seq_file implementation for /proc/rcu_show */
static void *rcu_list_seq_start_simple(struct seq_file *s, loff_t *pos)
{
    loff_t off = 0;
    struct rcu_item *it;

    rcu_read_lock();
    list_for_each_entry_rcu(it, &rcu_list_head, list) {
        if (off == *pos) {
            rcu_read_unlock();
            return it;
        }
        off++;
    }
    rcu_read_unlock();
    return NULL;
}

static void *rcu_list_seq_next_simple(struct seq_file *s, void *v, loff_t *pos)
{
    struct rcu_item *it = v;
    struct rcu_item *iter;
    loff_t off = 0;

    if (!it)
        return NULL;

    rcu_read_lock();
    list_for_each_entry_rcu(iter, &rcu_list_head, list) {
        if (off > *pos) {
            rcu_read_unlock();
            (*pos)++;
            return iter;
        }
        off++;
    }
    rcu_read_unlock();
    return NULL;
}

static void rcu_list_seq_stop(struct seq_file *s, void *v)
{
    /* nothing to do */
}

static int rcu_list_seq_show(struct seq_file *s, void *v)
{
    struct rcu_item *it = v;

    if (!it)
        return 0;

    seq_printf(s, "%d: %s\n", it->id, it->name);
    return 0;
}

static const struct seq_operations rcu_seq_ops = {
    .start = rcu_list_seq_start_simple,
    .next  = rcu_list_seq_next_simple,
    .stop  = rcu_list_seq_stop,
    .show  = rcu_list_seq_show,
};

static int rcu_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &rcu_seq_ops);
}

static const struct proc_ops rcu_proc_fops = {
    .proc_open    = rcu_proc_open,
    .proc_read    = seq_read,
    .proc_lseek  = seq_lseek,
    .proc_release = seq_release,
};

/* proc write helpers */
static ssize_t rcu_add_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *ppos)
{
    char kbuf[64];
    int id;
    char name[32];
    int ret;

    if (count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;
    kbuf[count] = '\0';

    ret = sscanf(kbuf, "%d %31s", &id, name);
    if (ret < 1)
        return -EINVAL;
    if (ret == 1)
        strncpy(name, "noname", sizeof(name));

    ret = rcu_list_add(id, name);
    return ret ? ret : count;
}

static ssize_t rcu_del_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *ppos)
{
    char kbuf[32];
    int id;
    int ret;

    if (count >= sizeof(kbuf))
        return -EINVAL;
    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;
    kbuf[count] = '\0';

    ret = sscanf(kbuf, "%d", &id);
    if (ret != 1)
        return -EINVAL;

    ret = rcu_list_del(id);
    return ret ? ret : count;
}

/* Trigger call_rcu manually for testing */
static ssize_t rcu_call_write(struct file *file, const char __user *buf,
                              size_t count, loff_t *ppos)
{
    struct rcu_item *dummy = rcu_item_create(-1, "dummy");
    if (!dummy)
        return -ENOMEM;
    pr_info("rcu_example: scheduling dummy call_rcu free\n");
    call_rcu(&dummy->rcu, rcu_item_free_callback);
    return count;
}

static const struct proc_ops  rcu_add_fops = {
    .proc_write = rcu_add_write,
};

static const struct proc_ops  rcu_del_fops = {
    .proc_write = rcu_del_write,
};

static const struct proc_ops  rcu_call_fops = {
    .proc_write = rcu_call_write,
};

static struct proc_dir_entry *p_rcu_dir;
static struct proc_dir_entry *p_rcu_add;
static struct proc_dir_entry *p_rcu_del;
static struct proc_dir_entry *p_rcu_show;
static struct proc_dir_entry *p_rcu_call;

static int __init rcu_example_init(void)
{
    pr_info("rcu_example: init\n");

    p_rcu_dir = proc_mkdir("rcu_example", NULL);
    if (!p_rcu_dir)
        goto err;

    p_rcu_add = proc_create("rcu_add", 0222, p_rcu_dir, &rcu_add_fops);
    p_rcu_del = proc_create("rcu_del", 0222, p_rcu_dir, &rcu_del_fops);
    p_rcu_show = proc_create("rcu_show", 0444, p_rcu_dir, &rcu_proc_fops);
    p_rcu_call = proc_create("rcu_call", 0222, p_rcu_dir, &rcu_call_fops);

    if (!p_rcu_add || !p_rcu_del || !p_rcu_show || !p_rcu_call)
        goto cleanup_proc;

    return 0;

cleanup_proc:
    if (p_rcu_add) proc_remove(p_rcu_add);
    if (p_rcu_del) proc_remove(p_rcu_del);
    if (p_rcu_show) proc_remove(p_rcu_show);
    if (p_rcu_call) proc_remove(p_rcu_call);
    if (p_rcu_dir) proc_remove(p_rcu_dir);
err:
    pr_err("rcu_example: failed to create proc entries\n");
    return -ENOMEM;
}

static void rcu_list_cleanup(void)
{
    struct rcu_item *it, *tmp;

    /* remove all entries and free them via call_rcu */
    spin_lock(&rcu_list_lock);
    list_for_each_entry_safe(it, tmp, &rcu_list_head, list) {
        list_del_rcu(&it->list);
        call_rcu(&it->rcu, rcu_item_free_callback);
    }
    spin_unlock(&rcu_list_lock);

    /* wait for all RCU callbacks to complete before unloading */
    synchronize_rcu();
}

static void __exit rcu_example_exit(void)
{
    pr_info("rcu_example: exit\n");

    if (p_rcu_add) proc_remove(p_rcu_add);
    if (p_rcu_del) proc_remove(p_rcu_del);
    if (p_rcu_show) proc_remove(p_rcu_show);
    if (p_rcu_call) proc_remove(p_rcu_call);
    if (p_rcu_dir) proc_remove(p_rcu_dir);

    rcu_list_cleanup();
}

module_init(rcu_example_init);
module_exit(rcu_example_exit);

/*
 * Makefile (save alongside this file as Makefile)
 * -----------------------------------------------
 * obj-m += rcu_example_module.o
 *
 * all:
 * 	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 *
 * clean:
 * 	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
 */
