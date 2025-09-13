

# sudo insmod hello.ko
# dmesg | tail
# sudo modprobe hello
# sudo insmod ./bitmapToy.ko
 
obj-m += bitmapToy.o 

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean 

