obj-m += interrupt_module.o

KERNEL = /home/user/Workspace/exynos/kernel
SHARE = /home/user/Share

all:
	make -C $(KERNEL) M=`pwd` modules
	rm -rf $(SHARE)/*
	cp -f *.ko $(SHARE)/
	make clean
	@echo "\033[31m >>>>>> make all successful <<<<<< \033[0m"
clean:
	make -C $(KERNEL) M=`pwd` modules clean
