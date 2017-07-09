#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <asm/io.h>

#include "led_driver.h"

static volatile unsigned int *GPM4CON = NULL;
static volatile unsigned int *GPM4DAT = NULL;

static int tiny4412_open(struct inode *my_indoe, struct file *my_file)
{
        *GPM4CON &= ~(0xf << 0 * 4);
        *GPM4CON |= (1 << 0 * 4);
        printk(KERN_INFO "open set the gpio to output mode\n");
        return 0;
}

static int tiny4412_release(struct inode *my_indoe, struct file *my_file)
{
        *GPM4DAT |= (1 << 0);
        printk(KERN_INFO "release close the led\n");
        return 0;
}

static ssize_t tiny4412_read(struct file *my_file, char __user *buff, size_t cnt, loff_t *loff)
{
        printk(KERN_INFO "[%s][%d] read do nothing\n", __func__, __LINE__);
        return 0;
}

static ssize_t tiny4412_write(struct file *my_file, const char __user *buff, size_t cnt, loff_t *loff)
{
        printk(KERN_INFO "[%s][%d] write do nothing\n", __func__, __LINE__);
        return 0;
}

static int tiny4412_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
        switch(cmd)
        {
        case LED_ON:
                *GPM4DAT &= ~(1 << 0);
                printk(KERN_INFO "[%s][%d] ioctl open the led\n", __func__, __LINE__);
                break;

        case LED_OFF:
                *GPM4DAT |= (1 << 0);
                printk(KERN_INFO "[%s][%d] ioctl close the led\n", __func__, __LINE__);
                break;

        default:
                *GPM4DAT |= (1 << 0);
                break;
        }

        return 0;
}

static struct file_operations tiny4412_fops =
{
        .open           = tiny4412_open,
        .release        = tiny4412_release,
        .read           = tiny4412_read,
        .write          = tiny4412_write,
        .unlocked_ioctl = tiny4412_ioctl,
};

static struct miscdevice led_misc_driver =
{
        .minor  = 255,
        .name   = "led_device",
        .fops   = &tiny4412_fops
};

static int tiny4412_led_probe(struct platform_device * tiny4412_led_device)
{
        struct resource* res = tiny4412_led_device->resource;
        GPM4CON = ioremap(res->start, 4);
        GPM4DAT = ioremap(res->start + 4, 4);
        printk("[%s][%d] running\n", __func__, __LINE__);
        misc_register(&led_misc_driver);
        return 0;
}

static int tiny4412_led_remove(struct platform_device *plat_dev)
{
        misc_deregister(&led_misc_driver);
        iounmap(GPM4CON);
        iounmap(GPM4DAT);
        printk("[%s][%d] running\n", __func__, __LINE__);
        return 0;
}

static struct platform_driver tiny4412_led_driver =
{
        .probe  = tiny4412_led_probe,
        .remove = tiny4412_led_remove,
        .driver =
        {
                .name = "tiny4412_led",
        }
};

static int __init led_driver_init(void)
{
        platform_driver_register(&tiny4412_led_driver);
        printk("[%s][%d] init ok!\n", __func__, __LINE__);
        return 0;
}

static void __exit led_driver_exit(void)
{
        platform_driver_unregister(&tiny4412_led_driver);
        printk("[%s][%d] exit ok!\n", __func__, __LINE__);
}

module_init(led_driver_init);
module_exit(led_driver_exit);

MODULE_LICENSE("GPL");
