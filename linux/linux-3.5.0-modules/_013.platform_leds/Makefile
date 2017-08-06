obj-m += led_driver.o
obj-m += led_gpm4_0.o
obj-m += led_gpm4_1.o
obj-m += led_gpm4_2.o
obj-m += led_gpm4_3.o

KERNEL = /home/user/Workspace/exynos/kernel
SHARE = /home/user/Share

all:
	make -C $(KERNEL) M=`pwd` modules
	arm-linux-gcc led_app.c -o app
	rm -rf $(SHARE)/*
	cp -f *.ko $(SHARE)/
	cp -f app $(SHARE)/
	make clean
	@echo "\033[31m >>>>>> make all successful <<<<<< \033[0m"
clean:
	make -C $(KERNEL) M=`pwd` modules clean
	rm -rf app
