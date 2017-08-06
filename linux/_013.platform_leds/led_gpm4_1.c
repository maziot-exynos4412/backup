#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>

// 第一步: 定义平台资源
static struct resource led_resource[] =
{
        // LED GPIO 内存资源, LED 对应 GPIO 的控制寄存器
        [0] =
        {
                .start = 0x110002E0,
                .end   = 0x110002E8,
                .name  = "led",
                .flags = IORESOURCE_MEM,
        },
};

static void plat_dev_release(struct device *dev)
{
        printk("[%s][%d] running\n", __func__, __LINE__);
}

static struct platform_device tiny4412_led_device =
{
        .name           ="GPM4_1_LED",                  // 设备名称, 由于和平台驱动匹配
        .id             = 1,                            // 设备 id
        .num_resources  = ARRAY_SIZE(led_resource),     // 设备拥有的资源数量
        .resource       = led_resource,                 // 设备的资源结构体
        .dev            =
        {
                .release        = plat_dev_release,
                .platform_data  = 1,                  // 向 platform 传递的数据
        },
};

static int __init led_device_init(void)
{
        // 向内核注册一个平台设备
        platform_device_register(&tiny4412_led_device);
        printk("[%s][%d] init ok!\n", __func__, __LINE__);
        return 0;
}

static void __exit led_device_exit(void)
{
        // 在内核中删除指定的平台设备
        platform_device_unregister(&tiny4412_led_device);
        printk("[%s][%d] exit ok!\n", __func__, __LINE__);
}

module_init(led_device_init);
module_exit(led_device_exit);

MODULE_LICENSE("GPL");
