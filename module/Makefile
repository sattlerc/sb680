obj-m+=sb680.o sb880.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

load-sb680:
	make && bash -c 'sudo rmmod hid_generic; sudo rmmod sb680; sudo insmod sb680.ko && sudo modprobe hid_generic; dmesg | tail'

load-sb880:
	make && bash -c 'sudo rmmod hid_multitouch; sudo rmmod sb880; sudo insmod sb880.ko && sudo modprobe hid_multitouch; dmesg | tail'

unload-sb680:
	sudo rmmod sb680; sudo rmmod hid_generic; sudo modprobe hid_generic

unload-sb880:
	sudo rmmod sb880; sudo rmmod hid_multitouch; sudo modprobe hid_multitouch
