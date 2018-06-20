#Makefile for a basic kernel module

obj-m   := dev_driver.o
#mymain-objs += fpga_dot_driver.o fpga_fnd_driver.o fpga_led_driver.o main.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:
	cp -a dev_driver.ko /nfsroot
	
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .tmp*
	rm -rf .dev_driver*
