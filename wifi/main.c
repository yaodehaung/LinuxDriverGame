#include <linux/module.h>
#include <linux/netdevice.h>

static struct net_device *dummy_dev;

static int dummy_open(struct net_device *dev)
{
    netif_start_queue(dev);
    pr_info("dummy: opened\n");
    return 0;
}

static int dummy_stop(struct net_device *dev)
{
    netif_stop_queue(dev);
    pr_info("dummy: stopped\n");
    return 0;
}

static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *dev)
{
    pr_info("dummy: xmit\n");
    dev_kfree_skb(skb);
    return NETDEV_TX_OK;
}

static const struct net_device_ops dummy_netdev_ops = {
    .ndo_open = dummy_open,
    .ndo_stop = dummy_stop,
    .ndo_start_xmit = dummy_xmit,
};

static void dummy_setup(struct net_device *dev)
{
    ether_setup(dev);
    dev->netdev_ops = &dummy_netdev_ops;
    dev->flags |= IFF_NOARP;
    dev->mtu = 1500;
    strcpy(dev->name, "dummy%d");
}

static int __init dummy_init(void)
{
    int err;
    dummy_dev = alloc_netdev(0, "dummy%d", NET_NAME_UNKNOWN, dummy_setup);
    if (!dummy_dev)
        return -ENOMEM;
    err = register_netdev(dummy_dev);
    if (err) {
        free_netdev(dummy_dev);
        return err;
    }
    pr_info("dummy net device registered\n");
    return 0;
}

static void __exit dummy_exit(void)
{
    unregister_netdev(dummy_dev);
    free_netdev(dummy_dev);
    pr_info("dummy net device unregistered\n");
}

module_init(dummy_init);
module_exit(dummy_exit);
MODULE_LICENSE("GPL");
