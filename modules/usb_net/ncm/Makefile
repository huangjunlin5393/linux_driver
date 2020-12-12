obj-m += xin_ncm.o

KERNELDIR := /lib/modules/`uname -r`/build

all::
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules

clean:
	rm -rf *.o *.ko *.order *.symvers *.mod.c
