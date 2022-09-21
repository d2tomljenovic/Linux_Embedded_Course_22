#
# By default, the build is done against the running linux kernel source.
# To build against a different kernel source tree, set KDIR:
#
#    make KERNEL_SOURCES=/path/to/kernel/source

KERNEL_SOURCES	 = ~/linux-kernel-labs/src/linux
obj-m := hello_version.o

default: modules
.PHONY: default

modules:
	$(MAKE) -C $(KERNEL_SOURCES) M=$$PWD $@

clean:
	$(MAKE) -C $(KERNEL_SOURCES) M=$$PWD $@

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y
git remote set-url origin https://d2tomljenovic@github.com/d2tomljenovic/Linux_Embedded_Course_22.git