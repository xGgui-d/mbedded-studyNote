# 编译后的内核的路径
KERNELDIR := /home/xggui/linux/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/
# 当前路径
CURRENT_PATH := $(shell pwd)

obj-m := leddevice.o leddriver.o
build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH)
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean	
