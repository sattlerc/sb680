obj-m+=sb680.o sb885.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

load-sb680: all
	bash -c 'sudo rmmod hid_generic; sudo rmmod sb680; sudo insmod sb680.ko; sudo modprobe hid_generic; dmesg | tail'
	make -C ../adhoc setup

load-sb885: all
	bash -c 'sudo rmmod hid_multitouch; sudo rmmod hid_generic; sudo rmmod sb885; sudo insmod sb885.ko; sudo modprobe hid_multitouch; sudo modprobe hid_generic; dmesg | tail'
	make -C ../adhoc setup

unload-sb680:
	sudo rmmod sb680; sudo rmmod hid_generic; sudo modprobe hid_generic

unload-sb885:
	sudo rmmod sb885; sudo rmmod hid_multitouch; sudo modprobe hid_multitouch; sudo rmmod hid_generic; sudo modprobe hid_generic
