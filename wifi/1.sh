make
sudo insmod mylib.ko   # 匯出符號（但不執行初始化）
sudo insmod main.ko    # 使用 add()
dmesg | tail
