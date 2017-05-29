#include <stdio.h>

int main(int argc, char**argv)
{
        int fd;
        fd = open("/dev/key_device", 2);
        if(fd < 0)
        {
                printf("driver open fail\n");
                return -1;
        }

        int data = -1;
        write(fd, &data, 4);

        while(1)
        {
                read(fd, &data, 4);

                if(data > 0)
                {
                        printf("[%s] the key value is = %d\n", __func__, data);
                        while (data > 0)
                        {
                                read(fd, &data, 4);
                        }
                }
        }

        close(fd);

        return 0;
}
