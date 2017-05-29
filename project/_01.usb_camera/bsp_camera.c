#include "bsp_camera.h"
#include "bsp_color.h"
#include <linux/fb.h>
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

int camera_fd = -1;
int iFd = -1;
struct v4l2_buffer tV4l2Buf;
unsigned char* pucVideBuf[4];
unsigned char* videobuff;
PT_PixelDatas video_buff;

extern struct fb_var_screeninfo var;                  // LCD 固定参数信息
extern struct fb_fix_screeninfo fix;                  // LCD 可变参数信息
extern unsigned char* fbmem;                                 // LCD 映射到 DDR 中的首地址

int camera_init1(void)
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

                /* 第八步: 确定每一个 buffer 的信息, 并且 mmap */
                ioctl(camera_fd, VIDIOC_QUERYBUF, &tV4l2Buf);
                pucVideBuf[i] = mmap(0 /* start anywhere */, tV4l2Buf.length, PROT_READ, MAP_SHARED, camera_fd,
                                tV4l2Buf.m.offset);

                /* 第九步: 放入采集队列中 */
                ioctl(camera_fd, VIDIOC_QBUF, &tV4l2Buf);
        }

        /* 第十步: 启动摄像头开始读数据 */
        ioctl(camera_fd, VIDIOC_STREAMON, V4L2_BUF_TYPE_VIDEO_CAPTURE);

        return 0;
}

int camera_init(void)
{

	int i=0;
	int cnt=0;
	int error;

	int iType = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct v4l2_capability tV4L2Cap;
	struct v4l2_fmtdesc tV4L2FmtDesc;

	struct v4l2_format  tV4l2Fmt;

	struct v4l2_requestbuffers tV4l2ReqBuffs;

	/* 1、打开视频设备 */
	iFd = open("/dev/video15",O_RDWR);
	if(iFd < 0)
	{
		printf("can't open /dev/video\n");
		return 0;
	}

	/* 2、VIDIOC_QUERYCAP 确定它是否视频捕捉设备,支持哪种接口(streaming/read,write) */
	error = ioctl(iFd,VIDIOC_QUERYCAP,&tV4L2Cap);
	if(error)
	{
		printf("no this video device\n");
		return -1;
	}
	/* 2.1、检测是否视频CAPTURE设备 */
	if (!(tV4L2Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
    	printf("not a video capture device\n");
       return -1;
    }
	/* 2.2、支持哪种接口:mmap read/write */
	if (tV4L2Cap.capabilities & V4L2_CAP_STREAMING)
	{
	    printf("supports streaming i/o\n");
	}

	if (tV4L2Cap.capabilities & V4L2_CAP_READWRITE)
	{
	    printf("supports read i/o\n");
	}
	/* 3、VIDIOC_ENUM_FMT 查询支持哪种格式 */
	memset(&tV4L2FmtDesc, 0, sizeof(tV4L2FmtDesc));
	tV4L2FmtDesc.index = 0;
	tV4L2FmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(iFd, VIDIOC_ENUM_FMT, &tV4L2FmtDesc);

	/* 4、 VIDIOC_S_FMT 设置摄像头使用哪种格式 */
	memset(&tV4l2Fmt, 0, sizeof(struct v4l2_format));
	tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tV4l2Fmt.fmt.pix.pixelformat = tV4L2FmtDesc.pixelformat;
	printf("Support :%d\n",tV4l2Fmt.fmt.pix.pixelformat);

	/*修改显示的尺寸---在LCD上显示的位置*/
	tV4l2Fmt.fmt.pix.width       = 640;
	tV4l2Fmt.fmt.pix.height      = 480;
	tV4l2Fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    /* 如果驱动程序发现无法某些参数(比如分辨率),
     * 它会调整这些参数, 并且返回给应用程序
     */
    error = ioctl(iFd, VIDIOC_S_FMT, &tV4l2Fmt);
    if (error)
    {
    	printf("Unable to set format\n");
       return -1;
    }

	printf("Support Format:%d\n",tV4l2Fmt.fmt.pix.pixelformat);
	printf("Support width:%d\n",tV4l2Fmt.fmt.pix.width);
	printf("Support height:%d\n",tV4l2Fmt.fmt.pix.height);


	/* 初始化ptVideoBufIn结构体，为转化做准备 */
	ptVideoBufIn.iBpp = (tV4l2Fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) ? 24 : \
                                        (tV4l2Fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) ? 0 :  \
                                        (tV4l2Fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565) ? 16 :  \
                                        0;
	ptVideoBufIn.iHeight = tV4l2Fmt.fmt.pix.height;
	ptVideoBufIn.iWidth = tV4l2Fmt.fmt.pix.width;
	ptVideoBufIn.iLineBytes = ptVideoBufIn.iWidth*ptVideoBufIn.iBpp/8;
	ptVideoBufIn.iTotalBytes = ptVideoBufIn.iLineBytes * ptVideoBufIn.iHeight;
	printf("ptVideoBufIn.iBpp = %d\n",ptVideoBufIn.iBpp);
	/* 5、VIDIOC_REQBUFS  申请buffer */
	memset(&tV4l2ReqBuffs, 0, sizeof(struct v4l2_requestbuffers));
	/* 人为的想要分配4个buffer:实际上由VIDIOC_REQBUFS获取到的信息来决定 */
	tV4l2ReqBuffs.count   = 4;
	tV4l2ReqBuffs.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/* 表示申请的缓冲是支持MMAP */
	tV4l2ReqBuffs.memory  = V4L2_MEMORY_MMAP;
	/* 为分配buffer做准备 */
	error = ioctl(iFd, VIDIOC_REQBUFS, &tV4l2ReqBuffs);
	if (error)
	{
		printf("Unable to allocate buffers.\n");
	    return -1;
	}
	/* 判断是否支持mmap */
	if (tV4L2Cap.capabilities & V4L2_CAP_STREAMING)
	{
		 /* map the buffers */
        for (i = 0; i < tV4l2ReqBuffs.count; i++)
        {
        	memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
        	tV4l2Buf.index = i;
        	tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
			/* 6、VIDIOC_QUERYBUF 确定每一个buffer的信息 并且 mmap */
        	error = ioctl(iFd, VIDIOC_QUERYBUF, &tV4l2Buf);
    if (error)
		{
			    printf("Unable to query buffer.\n");
			   return -1;
		}

         printf("length = %d\n",tV4l2Buf.length);

        	pucVideBuf[i] = mmap(0 /* start anywhere */ ,
        			  tV4l2Buf.length, PROT_READ, MAP_SHARED, iFd,
        			  tV4l2Buf.m.offset);
        	if (pucVideBuf[i] == MAP_FAILED)
            {
        	    printf("Unable to map buffer\n");
        	   return -1;
        	}
			printf("mmap %d addr:%p\n",i,pucVideBuf[i]);
		}
	}

	/* 7、VIDIOC_QBUF  放入队列 */
    for (i = 0; i <tV4l2ReqBuffs.count; i++)
    {
    	memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
    	tV4l2Buf.index = i;
    	tV4l2Buf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	tV4l2Buf.memory = V4L2_MEMORY_MMAP;
    	error = ioctl(iFd, VIDIOC_QBUF, &tV4l2Buf);
    	if (error)
        {
    	    printf("Unable to queue buffer.\n");
    	  	 return -1;
    	}
    }
	printf("ready to read data\n");

	/* 8、启动摄像头开始读数据 */
    error = ioctl(iFd, VIDIOC_STREAMON, &iType);
    if (error)
    {
    	printf("Unable to start capture.\n");
    	return -1;
    }
    printf("ready to read data\n");
	return 0;
}

static unsigned int Pyuv422torgb32(unsigned char x, unsigned y, unsigned char * input_ptr, unsigned char * output_ptr,
                unsigned int image_width, unsigned int image_height)
{
        unsigned int i, size, j;
        unsigned char Y, Y1, U, V;
        unsigned char *buff = input_ptr;
        unsigned char *output_pt = output_ptr;
        unsigned char *src = NULL;
        unsigned char * dst = NULL;
        unsigned char r, g, b;
        unsigned int color;

        videobuff = buff;

        size = image_width * image_height / 2;
        for(i = size; i > 0; i--)
        {
                /* bgr instead rgb ?? */
                Y = buff[0];
                U = buff[1];
                Y1 = buff[2];
                V = buff[3];
                buff += 4;
                r = R_FROMYV(Y, V);
                g = G_FROMYUV(Y, U, V); //b
                b = B_FROMYU(Y, U); //v
                *output_pt++ = b;
                *output_pt++ = g;
                *output_pt++ = r;
                r = R_FROMYV(Y1, V);
                g = G_FROMYUV(Y1, U, V); //b
                b = B_FROMYU(Y1, U); //v
                *output_pt++ = b;
                *output_pt++ = g;
                *output_pt++ = r;
        }
        src = output_ptr;

        int line_width = var.xres * var.bits_per_pixel / 8;
        dst = fbmem + var.xres * var.bits_per_pixel / 8 * y + x * var.bits_per_pixel / 8;
        for(j = 0; j < image_height; j++)
        {
                lcd_draw_line_colorful(src, dst, image_width);
                src += (image_width * 3);
                dst += line_width;
        }
        return 0;

}

static int Yuv2RgbConvert(unsigned char x, unsigned char y, PT_PixelDatas ptVideoBufIn, PT_PixelDatas ptVideoBufOut)
{
        PT_PixelDatas ptPixelDatasIn = ptVideoBufIn;
        PT_PixelDatas ptPixelDatasOut = ptVideoBufOut;

        video_buff = ptVideoBufIn;
        ptPixelDatasOut->iWidth = ptPixelDatasIn->iWidth;
        ptPixelDatasOut->iHeight = ptPixelDatasIn->iHeight;

        ptPixelDatasOut->iBpp = 32;
        ptPixelDatasOut->iLineBytes = ptPixelDatasOut->iWidth * ptPixelDatasOut->iBpp / 8;
        ptPixelDatasOut->iTotalBytes = ptPixelDatasOut->iLineBytes * ptPixelDatasOut->iHeight;

        if(!ptPixelDatasOut->aucPixelDatas)
        {
                ptPixelDatasOut->aucPixelDatas = malloc(ptPixelDatasOut->iTotalBytes);
        }

        Pyuv422torgb32(x, y, ptPixelDatasIn->aucPixelDatas, ptPixelDatasOut->aucPixelDatas, ptPixelDatasOut->iWidth,
                        ptPixelDatasOut->iHeight);

        return 0;

}

void *camera_pthread(void * arg)
{
        int cnt = 0;
        int i = 0;
        int ListNum;
        struct pollfd fds[1];

        /* 使用poll来等待是否有数据 */
        fds[0].fd = camera_fd;
        fds[0].events = POLLIN;

        /* 将 YUV 格式的数据，转化为 RGB 888 ,方便显示在 LCD 上 */
        initLut();
        ptVideoBufOut.aucPixelDatas = NULL;

        while(1)
        {
                poll(fds, 1, -1);
                memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
                tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                tV4l2Buf.memory = V4L2_MEMORY_MMAP;

                /* 从队列中取出一帧数据 */
                ioctl(camera_fd, VIDIOC_DQBUF, &tV4l2Buf);
                ListNum = tV4l2Buf.index;
                ptVideoBufIn.aucPixelDatas = pucVideBuf[ListNum];

                /* 将 YUV 格式的数据转化为 RGB 格式 */
                Yuv2RgbConvert(5, 90, &ptVideoBufIn, &ptVideoBufOut);
                /* 取出了一帧数据之后，不要忘了缓冲区的 buffer 加回采集队列中 */
                memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
                tV4l2Buf.index = ListNum;
                tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                tV4l2Buf.memory = V4L2_MEMORY_MMAP;
                ioctl(camera_fd, VIDIOC_QBUF, &tV4l2Buf);
        }

}

void camera_start(void)
{
        pthread_t camerathread;
        pthread_create(&camerathread, NULL, camera_pthread, NULL); //摄像头线程
}
