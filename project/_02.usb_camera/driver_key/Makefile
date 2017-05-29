obj-m += key_module.o

KERNEL = /home/user/exynos/source/linux-3.5

all:
	@make -C $(KERNEL) M=`pwd` modules
	@arm-linux-gcc key_app.c -o app
	cp app ~/board
	cp *.ko ~/board
	@echo "\033[31m >>>>>> make all successful <<<<<< \033[0m"



