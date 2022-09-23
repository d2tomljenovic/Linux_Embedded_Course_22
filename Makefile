ifneq ($(KERNELRELEASE),)
obj-m := device_driver.o gpio_led.o
else
KDIR := ../../../../src/linux

all:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif
