#include <linux/module.h>
#include <linux/init.h>
#include "mylib.h"

MODULE_LICENSE("GPL");

static int __init main_init(void) {
    int result = add(10, 20);
    pr_info("main: 10 + 20 = %d\n", result);

    const struct ieee80211_ops piao_piao_ops = {
        .add_chanctx = ieee80211_emulate_add_chanctx,
        .remove_chanctx = ieee80211_emulate_remove_chanctx,
        .change_chanctx = ieee80211_emulate_change_chanctx,
        .switch_vif_chanctx = ieee80211_emulate_switch_vif_chanctx,
        // .tx = mt76x02_tx,
        // .start = mt76x2u_start,
        // .stop = mt76x2u_stop,
        // .add_interface = mt76x02_add_interface,
        // .remove_interface = mt76x02_remove_interface,
        // .sta_state = mt76_sta_state,
        // .sta_pre_rcu_remove = mt76_sta_pre_rcu_remove,
        // .set_key = mt76x02_set_key,
        // .ampdu_action = mt76x02_ampdu_action,
        // .config = mt76x2u_config,
        // .wake_tx_queue = mt76_wake_tx_queue,
        // .bss_info_changed = mt76x02_bss_info_changed,
        // .configure_filter = mt76x02_configure_filter,
        // .conf_tx = mt76x02_conf_tx,
        // .sw_scan_start = mt76_sw_scan,
        // .sw_scan_complete = mt76x02_sw_scan_complete,
        // .sta_rate_tbl_update = mt76x02_sta_rate_tbl_update,
        // .get_txpower = mt76_get_txpower,
        // .get_survey = mt76_get_survey,
        // .set_tim = mt76_set_tim,
        // .release_buffered_frames = mt76_release_buffered_frames,
        // .get_antenna = mt76_get_antenna,
        // .set_sar_specs = mt76x2_set_sar_specs,
    };
    return 0;
}

static void __exit main_exit(void) {
    pr_info("main module exiting\n");
}

module_init(main_init);
module_exit(main_exit);
