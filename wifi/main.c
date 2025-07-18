#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/cfg80211.h>

static struct net_device *dummy_dev;
static struct wireless_dev *dummy_wdev;
static struct wiphy *dummy_wiphy;

static struct ieee80211_channel dummy_channels[] = {
    {
        .center_freq = 2412,
        .hw_value = 1,
        .flags = 0,
    },
};

static struct ieee80211_rate dummy_rates[] = {
    {
        .bitrate = 10,
        .hw_value = 0,
    },
};

static struct ieee80211_supported_band dummy_band_2ghz = {
    .band = NL80211_BAND_2GHZ,
    .channels = dummy_channels,
    .n_channels = ARRAY_SIZE(dummy_channels),
    .bitrates = dummy_rates,
    .n_bitrates = ARRAY_SIZE(dummy_rates),
};


//
// net_device operations
//
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
    .ndo_open       = dummy_open,
    .ndo_stop       = dummy_stop,
    .ndo_start_xmit = dummy_xmit,
};

//
// cfg80211 operations (minimal)
//
static int dummy_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
                                      const u8 *mac, struct station_info *sinfo)
{
    return -ENOTSUPP;
}

static const struct cfg80211_ops dummy_cfg80211_ops = {
    .get_station = dummy_cfg80211_get_station,
};

//
// Device setup (called by alloc_netdev)
//
static void dummy_setup(struct net_device *dev)
{
    ether_setup(dev);
    dev->netdev_ops = &dummy_netdev_ops;
    dev->flags |= IFF_NOARP;
    dev->mtu = 1500;
    eth_hw_addr_random(dev);
}

//
// Module Init
//
static int __init dummy_init(void)
{
    int err;

    // 1. ���� wiphy
    struct wiphy *wiphy;
    struct wireless_dev *wdev;

    wiphy = wiphy_new(&dummy_cfg80211_ops, 0);
    if (!wiphy)
        return -ENOMEM;

    wiphy->max_scan_ssids = 1;
    wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
    wiphy->bands[NL80211_BAND_2GHZ] = NULL;

    wiphy->bands[NL80211_BAND_2GHZ] = &dummy_band_2ghz;

    err = wiphy_register(wiphy);
    if (err) {
        wiphy_free(wiphy);
        return err;
    }

    dummy_wiphy = wiphy;

    // 2. ���� wireless_dev
    wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
    if (!wdev) {
        wiphy_unregister(wiphy);
        wiphy_free(wiphy);
        return -ENOMEM;
    }

    wdev->wiphy = wiphy;
    wdev->iftype = NL80211_IFTYPE_STATION;

    // 3. ���� net_device
    dummy_dev = alloc_netdev(0, "dummy%d", NET_NAME_UNKNOWN, dummy_setup);
    if (!dummy_dev) {
        kfree(wdev);
        wiphy_unregister(wiphy);
        wiphy_free(wiphy);
        return -ENOMEM;
    }

    dummy_dev->ieee80211_ptr = wdev;
    SET_NETDEV_DEV(dummy_dev, wiphy_dev(wiphy));
    wdev->netdev = dummy_dev;

    err = register_netdev(dummy_dev);
    if (err) {
        free_netdev(dummy_dev);
        kfree(wdev);
        wiphy_unregister(wiphy);
        wiphy_free(wiphy);
        return err;
    }

    dummy_wdev = wdev;

    pr_info("dummy net device with cfg80211 registered\n");
    return 0;
}

//
// Module Exit
//
static void __exit dummy_exit(void)
{
    unregister_netdev(dummy_dev);
    free_netdev(dummy_dev);

    if (dummy_wdev)
        kfree(dummy_wdev);

    if (dummy_wiphy) {
        wiphy_unregister(dummy_wiphy);
        wiphy_free(dummy_wiphy);
    }

    pr_info("dummy net device with cfg80211 unregistered\n");
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Huang");
MODULE_DESCRIPTION("Minimal dummy netdev + cfg80211 example");
