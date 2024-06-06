#ifndef _IMAGE_STRUCTURE
#define _IMAGE_STRUCTURE
#include <stdio.h>

/*
 *
 * document:24/5/31 项健乐
 *
 * 本程序仅用于未经压缩的BMP格式图片
 * 
 * 接下来先介绍需要用到的BMP文件格式(参考https://blog.csdn.net/u013066730/article/details/82625158)
 * 
 * BMP四个基础结构
 *  file header：提供文件格式大小等，占14B
 *  bitmap information：提供图像尺寸，压缩方式等，占40B
 *  color palette：当颜色位数在24以下，使用调色板展示索引信息，长度不定，如果在24位及以上，没有调色板
 *  bitmap data：对每个像素点的描述
 * 存储规则
 *  如果一个数据需要多个字节表示，那么低地址存放低位数据，高地址存放高位数据
 *  由于windows读取方式为4字节一单位，所以bitmap data中会有无意义的数字填充，使得每行数据为4的倍数
 *  需要跳过skip = 4 - ((m_iImageWidth * m_iBitsPerPixel)>>3) & 3
 *  m_iImageWidth图像像素宽度
 *  m_iBitsPerPixel每个像素占用比特数
 * file header(偏移表示距离文件开头的字节数)
 *  bftype:偏移0字节，占2字节，BMP对应的值应为'BM'
 *  bfOffBits:偏移10字节，占4字节，从文件开头到bitmap information的字节数
 * bitmap information
 *  biWidth:偏移18字节，占4字节，像素单位的图像宽度
 *  biHeight:偏移22字节，占4字节，像素为单位的图像高度，默认为正，表示bitmap information中的像素排布是从左下角到右上角，为负则相反
 *  biBitCount:偏移28字节，占2字节，每个像素需要几个比特描述
 *  biCompression:偏移30字节，占4字节，BMP的压缩方式，一般BMP图像无压缩，值为0，本程序仅适用此情况
 *  biClrUsed:偏移46字节，占4字节，实际使用的颜色索引数
 * color palette
 *  二维表palette[N][4],N=biClrUsed(实际使用的颜色索引数)
 *  每个颜色用一个占4字节的4元数组描述，分别表示B,G,R不透明度
 * bitmap informaiton
 *  这里分为不同情况
 *  如果biBitCount<24，使用调色板，每个像素占一个字节，代表使用的颜色id
 *  如果biBitCount=24，没有调色板，每个像素占用3字节，代表B,G,R
 *  如果biBitCount=32，没有调色板，每个像素占用4字节，代表B,G,R,不透明度(但本程序忽略不透明度)
 *  */

typedef struct{
    int biWidth;
    int biHeight;
    int biBitCount;
    int biClrUsed;
    int (*pixelList)[3];//每个像素点在bmp文件的颜色id或者24位图在本程序获得的颜色id
}ImageInfo;

/*
 *OpenImage()从BMP图像文件f中获取所需数据
 */
ImageInfo* OpenImage(FILE* f);

/*将结构ImageInfo在笔记文件中进行读写操作*/
ImageInfo* ReadImage(FILE* F);
void WriteImage(ImageInfo* ptr,FILE* f);

/*展示图像，图像左上角在(cx,cy)，以英寸位单位*/
void DisplayImage(ImageInfo* ptr,double cx,double cy,double GivenWidth);

/*基于存储特性写的用于替代fread的函数,从文件f读取count个大小为1字节的数据块，返回对应的int值*/
int ReadBMP(int count, FILE* f);

/*根据传入的结构计算图片高度，像素到英寸的转换*/
double GetImageHeight(ImageInfo* ptr,double GivenWidth);

void FreeImageStructure(ImageInfo* ptr);

#endif
