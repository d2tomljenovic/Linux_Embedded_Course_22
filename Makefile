KDIR := /home/student/linux-kernel-labs/src/linux


obj-m := morse_encrypter.o 

morse_encrypter-y += device_driver.o
morse_encrypter-y += gpio_led.o

all:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

