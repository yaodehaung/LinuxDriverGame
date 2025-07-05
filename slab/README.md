```sh
sudo insmod test_slab.ko

dmesg | tail -n 20

sudo rmmod my_module


cat /proc/rbtree

echo "add 25" > /proc/rbtree
echo "add 5" > /proc/rbtree
cat /proc/rbtree
```