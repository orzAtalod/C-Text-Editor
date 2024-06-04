#ifndef _IMAGE_STRUCTURE
#define _IMAGE_STRUCTURE
#include <stdio.h>

/*
 *
 * document:24/5/31 ���
 *
 * �����������δ��ѹ����BMP��ʽͼƬ
 * 
 * �������Ƚ�����Ҫ�õ���BMP�ļ���ʽ(�ο�https://blog.csdn.net/u013066730/article/details/82625158)
 * 
 * BMP�ĸ������ṹ
 *  file header���ṩ�ļ���ʽ��С�ȣ�ռ14B
 *  bitmap information���ṩͼ��ߴ磬ѹ����ʽ�ȣ�ռ40B
 *  color palette������ɫλ����24���£�ʹ�õ�ɫ��չʾ������Ϣ�����Ȳ����������24λ�����ϣ�û�е�ɫ��
 *  bitmap data����ÿ�����ص������
 * �洢����
 *  ���һ��������Ҫ����ֽڱ�ʾ����ô�͵�ַ��ŵ�λ���ݣ��ߵ�ַ��Ÿ�λ����
 *  ����windows��ȡ��ʽΪ4�ֽ�һ��λ������bitmap data�л����������������䣬ʹ��ÿ������Ϊ4�ı���
 *  ��Ҫ����skip = 4 - ((m_iImageWidth * m_iBitsPerPixel)>>3) & 3
 *  m_iImageWidthͼ�����ؿ��
 *  m_iBitsPerPixelÿ������ռ�ñ�����
 * file header(ƫ�Ʊ�ʾ�����ļ���ͷ���ֽ���)
 *  bftype:ƫ��0�ֽڣ�ռ2�ֽڣ�BMP��Ӧ��ֵӦΪ'BM'
 *  bfOffBits:ƫ��10�ֽڣ�ռ4�ֽڣ����ļ���ͷ��bitmap information���ֽ���
 * bitmap information
 *  biWidth:ƫ��18�ֽڣ�ռ4�ֽڣ����ص�λ��ͼ����
 *  biHeight:ƫ��22�ֽڣ�ռ4�ֽڣ�����Ϊ��λ��ͼ��߶ȣ�Ĭ��Ϊ������ʾbitmap information�е������Ų��Ǵ����½ǵ����Ͻǣ�Ϊ�����෴
 *  biBitCount:ƫ��28�ֽڣ�ռ2�ֽڣ�ÿ��������Ҫ������������
 *  biCompression:ƫ��30�ֽڣ�ռ4�ֽڣ�BMP��ѹ����ʽ��һ��BMPͼ����ѹ����ֵΪ0������������ô����
 *  biClrUsed:ƫ��46�ֽڣ�ռ4�ֽڣ�ʵ��ʹ�õ���ɫ������
 * color palette
 *  ��ά��palette[N][4],N=biClrUsed(ʵ��ʹ�õ���ɫ������)
 *  ÿ����ɫ��һ��ռ4�ֽڵ�4Ԫ�����������ֱ��ʾB,G,R��͸����
 * bitmap informaiton
 *  �����Ϊ��ͬ���
 *  ���biBitCount<24��ʹ�õ�ɫ�壬ÿ������ռһ���ֽڣ�����ʹ�õ���ɫid
 *  ���biBitCount=24��û�е�ɫ�壬ÿ������ռ��3�ֽڣ�����B,G,R
 *  ���biBitCount=32��û�е�ɫ�壬ÿ������ռ��4�ֽڣ�����B,G,R,��͸����(����������Բ�͸����)
 *  */

typedef struct{
    int biWidth;
    int biHeight;
    int biBitCount;
    int biClrUsed;
    int (*pixelList)[3];//ÿ�����ص���bmp�ļ�����ɫid����24λͼ�ڱ������õ���ɫid
}ImageInfo;

/*
 *OpenImage()��BMPͼ���ļ�f�л�ȡ��������
 */
ImageInfo* OpenImage(FILE* f);

/*���ṹImageInfo�ڱʼ��ļ��н��ж�д����*/
ImageInfo* ReadImage(FILE* F);
void WriteImage(ImageInfo* ptr,FILE* f);

/*չʾͼ��ͼ�����Ͻ���(cx,cy)����Ӣ��λ��λ*/
void DisplayImage(ImageInfo* ptr,double cx,double cy);

/*���ڴ洢����д���������fread�ĺ���,���ļ�f��ȡcount����СΪ1�ֽڵ����ݿ飬���ض�Ӧ��intֵ*/
int ReadBMP(int count, FILE* f);

/*���ݴ���Ľṹ����ͼƬ�߶ȣ����ص�Ӣ���ת��*/
double GetImageHeight(ImageInfo* ptr);

#endif