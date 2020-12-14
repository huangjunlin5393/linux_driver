KERNELDIR := /root/openwrt/ers-micro/build_dir/target-mips_34kc_uClibc-0.9.33.2/linux-ar71xx_generic/linux-3.18.19/
TOOLCHAIN ?= /root/openwrt/ers-micro/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-
obj-m := hoc_k.o

all:
	$(MAKE)  -C $(KERNELDIR) ARCH=mips  CROSS_COMPILE=$(TOOLCHAIN)  M=$(shell pwd) modules

clean:
	rm -rf *.ko *.o Module* module* *.mod* .*.cmd .*versions
