
CC = arm-linux-gcc

all: bsp_lcd.h bsp_camera.h bsp_color.h
	$(CC) -lpthread main.c bsp_lcd.c bsp_camera.c bsp_color.c -o app
	cp app ~/board/
	@echo "\033[31m >>>>>> make successful <<<<<< \033[0m"
	@echo "\033[31m >>>>>> make successful <<<<<< \033[0m"
	@echo "\033[31m >>>>>> make successful <<<<<< \033[0m"
