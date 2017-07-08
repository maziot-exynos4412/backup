#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <asm/io.h>

static volatile unsigned int *GPM4CON = NULL;
static volatile unsigned int *GPM4DAT = NULL;

static int tiny4412_open(struct inode *my_inode, struct file *my_file)
{
        printk("[%s][%d] running\n", __func__, __LINE__);
        return 0;
}

static int tiny4412_release(struct inode *my_inode, struct file *my_file)
{
        printk("[%s][%d] running\n", __func__, __LINE__);
        return 0;
}

static struct file_operations ops =
{
        .open           = tiny4412_open,
        .release        = tiny4412_release
};

static struct miscdevice led_misc_driver =
{
        .minor  = 255,
        .name   = "led_device",
        .fops   = &ops
};

static int tiny4412_led_probe(struct platform_device * tiny4412_led_device)
{
        struct resource* res = tiny4412_led_device->resource;
        GPM4CON = ioremap(0x110002E0, 4);
        GPM4DAT = ioremap(0x110002E4, 4);
        printk("[%s][%d] running\n", __func__, __LINE__);
        misc_register(&led_misc_driver);
        return 0;
}

static int tiny4412_led_remove(struct platform_device *plat_dev)
{
        misc_deregister(&led_misc_driver);
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
