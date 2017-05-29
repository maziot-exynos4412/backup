#ifdef __BSP_CAMERA_H
#define __BSP_CAMERA_H

int camera_init(void);
void *camera_pthread(void * arg);

#pragma pack(push)//入栈保护
#pragma pack(1)//以下结构体以一个字节对齐
typedef struct
{
	short bfType;//表明位图文件的类型，必须为BM
	long bfSize;//表明位图文件的大小，以字节为单位
	short bfReserved1;//属于保留字，必须为本0
	short bfReserved2;//也是保留字，必须为本0
	long bfOffBits;//位图阵列的起始位置，以字节为单位
} T_BITMAPFILEHEADER,*PT_BITMAPFILEHEADER;

typedef struct {
	long biSize; //指出本数据结构所需要的字节数
	long biWidth;//以象素为单位，给出BMP图象的宽度
	long biHeight;//以象素为单位，给出BMP图象的高度
	short biPlanes;//输出设备的位平面数，必须置为1
	short biBitCount;//给出每个象素的位数
	long biCompress;//给出位图的压缩类型
	long biSizeImage;//给出图象字节数的多少
	long biXPelsPerMeter;//图像的水平分辨率
	long biYPelsPerMeter;//图象的垂直分辨率
	long biClrUsed;//调色板中图象实际使用的颜色素数
	long biClrImportant;//给出重要颜色的索引值
} T_BITMAPINFOHEADER,*PT_BITMAPINFOHEADER;
#pragma pack(pop)


#endif /* __BSP_CAMERA_H */
