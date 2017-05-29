#include "bsp_lcd.h"
#include <linux/fb.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

int lcd_fd = -1;                                        // LCD 帧缓冲文件描述符
static struct fb_var_screeninfo var;                    // LCD 固定参数信息
static struct fb_fix_screeninfo fix;                    // LCD 可变参数信息
unsigned char* fbmem = NULL;                            // LCD 映射到 DDR 中的首地址

/**
 * @brief lcd 显示相关的初始化操作
 * @return 0 success -1 fail
 */
int lcd_init(void)
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
        lcd_clear_screen(0xffffff);
        return 0;
}

/**
 * @brief 根据指定的坐标和颜色值画点
 * @param x 指定的x坐标
 * @param y 指定的y坐标
 * @param color 指定的颜色值
 * @rerutrn null
 */
void lcd_draw_point(int x, int y, unsigned int color)
{
        unsigned int* point = NULL;

        /* 定位到 LCD 屏上的位置 */
        point = (unsigned int*) (fbmem + y * var.xres * var.bits_per_pixel / 8 + x * var.bits_per_pixel / 8);

        /* 向指向的 LCD 地址赋数据 */
        *point = color;
}

/**
 * @brief 将屏幕中的一行绘制成单一的颜色
 * @param color 指定的单一的颜色值
 * @return null
 */
void lcd_draw_line(int y, unsigned int color)
{
        int i;
        for(i = 0; i < var.xres; i++)
        {
                lcd_draw_point(i, y, color);
        }
}

/**
 * @brief 将屏幕中的一行依次绘制成指定的颜色
 * @param color 指定的颜色数据
 * @return null
 */
void lcd_draw_line_colorful(unsigned char *src, unsigned char* dst, unsigned int width)
{
        unsigned char r, g, b;
        unsigned int color;
        int i;
        unsigned int* dst32 = (unsigned int *)dst;
        for(i = 0; i < width; i++)
        {
                b = src[0];
                g = src[1];
                r = src[2];
                color = r << 16 | g << 8 | b;
                *dst32++ = color;
                src += 3;
        }
}

/**
 * @brief 清屏函数，将屏幕刷新成指定的颜色
 * @return null
 */
void lcd_clear_screen(unsigned int color)
{
        int i, j;
        for(i = 0; i < var.xres; i++)
        {
                for(j = 0; j < var.yres; j++)
                {
                        lcd_draw_point(i, j, color);
                }
        }
}
