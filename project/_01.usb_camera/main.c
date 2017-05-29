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
        camera_init();



        return 0;
}
