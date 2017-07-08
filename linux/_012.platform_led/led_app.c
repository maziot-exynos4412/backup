#include <stdio.h>

int main(int argc, char**argv)
{
        int fd;
        fd = open("/dev/led_device", 2);
        if(fd < 0)
        {
                printf("driver open fail\n");
                return -1;
        }

        return 0;
}
