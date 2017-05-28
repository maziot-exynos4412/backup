#include "bsp_lcd.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int lcd_fd = -1;                                        // LCD 帧缓冲文件描述符
static struct fb_var_screeninfo var;                    // LCD 固定参数信息
static struct fb_fix_screeninfo fix;                    // LCD 可变参数信息
unsigned char* fbmem = NULL;                             // LCD 映射到 DDR 中的首地址

int lcd_init()
{
        /* 打开帧缓冲文件 */
        lcd_fd = open("/dev/fb0", 2);
        if(lcd_fd < 0)
        {
                /* 倘若失败，答应错误信息 */
                printf("[%s][%d]open lcd frame buffer is error!\n", __func__, __LINE__);
                return -1;
        }

        /* 获取保存 LCD 参数信息的结构体 */
        ioctl(lcd_fd, FBIOGET_VSCREENINFO, &var);       // 获取固定参数
        ioctl(lcd_fd, FBIOGET_FSCREENINFO, &fix);       // 获取可变参数

        /* 计算出 */
        // line_width  = var.xres * var.bits_per_pixel / 8;

        /* 映射帧缓冲到 DDR 中，以后操作这片内存就是直接操作 LCD */
        fbmem = (unsigned char*)mmap(NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd ,0);
        if(fbmem == (unsigned char *)-1)
        {
                printf("fbmmap is error!!!\n");
                munmap(fbmem,fix.smem_len);
                return -1;
        }

        /* 清屏，将屏幕刷成白色 */
        memset(fbmem, 0xff, fix.smem_len);
        return 0;
}

/**
 * @description 根据指定的坐标和颜色值画点
 * @param x 指定的x坐标
 * @param y 指定的y坐标
 * @param color 指定的颜色值
 */
int lcd_draw_point(int x, int y, unsigned int color)
{
        unsigned int* point = NULL;

        /* 定位到 LCD 屏上的位置 */
        point = (unsigned int*) (fbmem + y * var.xres * var.bits_per_pixel / 8 + x * var.bits_per_pixel / 8);

        /* 向指向的 LCD 地址赋数据 */
        *point = color;

        return 0;
}


//
