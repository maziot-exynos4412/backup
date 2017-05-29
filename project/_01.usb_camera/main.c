#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "./bsp_lcd.h"

int main()
{
        lcd_init();
        printf("lcd int successful \n");
        camera_init();
        printf("camera init successful \n");
        camera_start();
        printf("camera_start successful \n");
        while(1);
        return 0;
}
