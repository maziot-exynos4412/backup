#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "bsp_lcd.h"

int main()
{
        int i, j;

        lcd_init();

        for(i = 100; i < 200; i++)
        {
                for(j = 100; j < 200; j++)
                {
                        lcd_draw_point(i, j, 0xdd);
                }

        }


        while (1)
        {
        }
}
