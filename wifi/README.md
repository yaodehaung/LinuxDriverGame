
```
lspci -n
```

REF: 
    1. mwifiex
    2. 

    wiphy
      |
      |
    cfg80211
      |
      |
    kernel

    led 
    usb

```
[ User space: iw / wpa_supplicant ]
        ↓ nl80211
[ Kernel: cfg80211 + 你的 cfg80211_ops 實作 ]
        ↓
[ 驅動：USB 傳輸 → 晶片 (FullMAC) ]
```