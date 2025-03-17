```sh
sudo insmod my_kobject.ko

dmesg | tail -n 20

sudo rmmod my_kobject


$ modprobe my_kobject
$ cat /sys/kernel/my_kobject/my_value
0
$ echo 42 > /sys/kernel/my_kobject/my_value
$ cat /sys/kernel/my_kobject/my_value
42
$ rmmod my_kobject
```