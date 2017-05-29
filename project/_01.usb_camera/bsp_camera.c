#include "bsp_camera.h"
#include <pthread.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <stdlib.h>
#include <tslib.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/videodev2.h>
#include <sys/time.h>



int camera_fd = -1;
struct v4l2_buffer tV4l2Buf;
unsigned char* pucVideBuf[4];



typedef struct PixelDatas
{
        int iWidth; /* 宽度: 一行有多少个象素 */
        int iHeight; /* 高度: 一列有多少个象素 */
        int iBpp; /* 一个象素用多少位来表示 */
        int iLineBytes; /* 一行数据有多少字节 */
        int iTotalBytes; /* 所有字节数 */
        unsigned char *aucPixelDatas; /* 象素数据存储的地方 */
} T_PixelDatas, *PT_PixelDatas;

T_PixelDatas ptVideoBufOut;
T_PixelDatas ptVideoBufIn;

int camera_init(void)
{
        int i = 0;
        struct v4l2_capability tV4L2Cap;
        struct v4l2_fmtdesc tV4L2FmtDesc;
        struct v4l2_format tV4l2Fmt;
        struct v4l2_requestbuffers tV4l2ReqBuffs;

        /* 第一步: 打开视频设备节点 */
        camera_fd = open("/dev/video15", O_RDWR);

        /* 第二步: 使用 VIDIOC_QUERYCAP 命令读取信息 */
        memset(&tV4L2Cap, 0, sizeof(struct v4l2_capability));
        ioctl(camera_fd, VIDIOC_QUERYCAP, &tV4L2Cap);

        /* 第三步: VIDIOC_ENUM_FMT 查询支持哪种格式 */
        memset(&tV4L2FmtDesc, 0, sizeof(tV4L2FmtDesc));
        ioctl(camera_fd, VIDIOC_ENUM_FMT, &tV4L2FmtDesc);

        /* 第四步: VIDIOC_S_FMT 设置摄像头使用哪种格式 */
        memset(&tV4l2Fmt, 0, sizeof(struct v4l2_format));
        tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tV4l2Fmt.fmt.pix.pixelformat = tV4L2FmtDesc.pixelformat;

        /* 第五步: 修改显示的尺寸在 LCD 上显示的位置 */
        tV4l2Fmt.fmt.pix.width = 640;
        tV4l2Fmt.fmt.pix.height = 480;
        tV4l2Fmt.fmt.pix.field = V4L2_FIELD_ANY;

        /* 第六步: 初始化 ptVideoBufIn 结构体，为转化做准备 */
        ptVideoBufIn.iBpp = 24;
        ptVideoBufIn.iHeight = tV4l2Fmt.fmt.pix.height;
        ptVideoBufIn.iWidth = tV4l2Fmt.fmt.pix.width;
        ptVideoBufIn.iLineBytes = ptVideoBufIn.iWidth * ptVideoBufIn.iBpp / 8;
        ptVideoBufIn.iTotalBytes = ptVideoBufIn.iLineBytes * ptVideoBufIn.iHeight;

        /* 第七步: 申请 buffer */
        memset(&tV4l2ReqBuffs, 0, sizeof(struct v4l2_requestbuffers));
        tV4l2ReqBuffs.count = 4; /* 分配4个buffer */
        tV4l2ReqBuffs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tV4l2ReqBuffs.memory = V4L2_MEMORY_MMAP; /* 表示申请的缓冲是支持MMAP */
        ioctl(camera_fd, VIDIOC_REQBUFS, &tV4l2ReqBuffs); /* 为分配buffer做准备 */

        for(i = 0; i < tV4l2ReqBuffs.count; i++)
        {
                memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
                tV4l2Buf.index = i;
                tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                tV4l2Buf.memory = V4L2_MEMORY_MMAP;

                /* 第八步: 确定每一个buffer的信息, 并且 mmap */
                ioctl(camera_fd, VIDIOC_QUERYBUF, &tV4l2Buf);
                pucVideBuf[i] = mmap(0 /* start anywhere */, tV4l2Buf.length, PROT_READ, MAP_SHARED, camera_fd,
                                tV4l2Buf.m.offset);

                /* 第九步: 放入队列 */
                ioctl(camera_fd, VIDIOC_QBUF, &tV4l2Buf);
        }

        /* 第十步: 启动摄像头开始读数据 */
        ioctl(camera_fd, VIDIOC_STREAMON, V4L2_BUF_TYPE_VIDEO_CAPTURE);
        return 0;
}



void *camera_pthread(void * arg)
{
        /*
        int error;
        int cnt = 0;
        int i = 0;
        int ListNum;
        struct pollfd fds[1];
        8.1、使用poll来等待是否有数据
        fds[0].fd = iFd;
        fds[0].events = POLLIN;

         YUV格式的数据<------>在LCD上显示:rgb888
        initLut();
        ptVideoBufOut.aucPixelDatas = NULL;

        while(1)
        {
                error = poll(fds, 1, -1);
                memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
                tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                tV4l2Buf.memory = V4L2_MEMORY_MMAP;
                9、VIDIOC_DQBUF    从队列中取出
                error = ioctl(iFd, VIDIOC_DQBUF, &tV4l2Buf);
                ListNum = tV4l2Buf.index;
                ptVideoBufIn.aucPixelDatas = pucVideBuf[ListNum];
                Yuv2RgbConvert(5, 90, &ptVideoBufIn, &ptVideoBufOut);

                 显示转化的数据:已经转化为24bpp了

                memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
                tV4l2Buf.index = ListNum;
                tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                tV4l2Buf.memory = V4L2_MEMORY_MMAP;
                error = ioctl(iFd, VIDIOC_QBUF, &tV4l2Buf);
        }*/

}

void camera_start(void)
{
        pthread_t camerathread;
        pthread_create(&camerathread,NULL,camera_pthread,NULL); //摄像头线程
}
