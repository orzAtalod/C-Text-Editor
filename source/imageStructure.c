#include "imageStructure.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

//pixelList�е�rgbֵ������������simpleGUI��ʹ����Ҫ����255.0
static int (*colorPalette)[3];//�洢��ɫ��
int ReadBMP(int count,FILE* f)
{
    char temp[5]={0};
    int i,target=0;
    fread(temp,1,count,f);//ÿ���ֽ�������Ҫ��������
    for(i=0;i<count;i++){
        target=target+(temp[i]<<(8*i*));//�͵�ַ���λ���ݣ��ߵ�ַ���λ����
    }
    return target;
}

ImageInfo* OpenImage(FILE* f)
{
    char tempchar[3];
    int biCompression,bfOffBits;
    ImageInfo* ptr;

    fread(tempchar,sizeof(char),2,f);
    tempchar[2]='\0';
    if(strcmp(tempchar,"BM")!=0){//�ж�ͼƬ�Ƿ���BMP
        assert(!"imagType is false");//���ڴ洢�����ԣ�������ĸ����һ��
    }
    fseek(f,30,SEEK_SET);

    biCompression=ReadBMP(4,f);//��ȡBMPѹ����ʽ
    if(biCompression!=0){
        assert(!"imageType is false");
    }

    fseek(f,10,SEEK_SET);
    bfOffBits=ReadBMP(4,f);//��ȡ���ļ���ͷ��bitmap information���ֽ�ƫ����������Ҫ��

    ptr=(ImageInfo*)malloc(sizeof(ImageInfo));
    fseek(f,18,SEEK_SET);
    ptr->biWidth=ReadBMP(4,f);
    ptr->biHeight=ReadBMP(4,f);
    fseek(f,28,SEEK_SET);
    ptr->biBitCount=ReadBMP(2,f);
    if(ptr->biBitCount<24){//����ÿ�������������λ�Ĳ�ͬ���ò�ͬ�ı��뷽ʽ
        fseek(f,46,SEEK_SET);
        ptr->biClrUsed=ReadBMP(4,f);//��ȡ���õ�����ɫ������
        if(ptr->biClrUsed==0) ptr->biClrUsed=pow(2,ptr->biBitCount);//�����������0˵��ʹ�����п�����ɫ 
        colorPalette=(int(*)[3])malloc(sizeof(int)*ptr->biClrUsed*3);
        int i,j,colorId;
        int tempColorCode[4];
		int biSize;
		fseek(f,14,SEEK_SET);
		fread(&biSize,1,4,f);
		
		fseek(f,14+biSize,SEEK_SET);
        for(i=0;i<ptr->biClrUsed;i++){//��ȡ��ɫ��
            fread(tempColorCode,1,1,f);
            fread(tempColorCode+1,1,1,f);
            fread(tempColorCode+2,1,1,f);
            fseek(f,1,SEEK_CUR);
            *(colorPalette+i)[0]=tempColorCode[2];
            *(colorPalette+i)[1]=tempColorCode[1];
            *(colorPalette+i)[2]=tempColorCode[0];
        }

        ptr->pixelList=(int(*)[3])malloc(sizeof(int)*ptr->biHeight*ptr->biWidth*3);
        fseek(f,bfOffBits,SEEK_SET);
        for(i=0;i<abs(ptr->biHeight);i++){//Ϊÿ�����ص㸳��rgbֵ
            for(j=0;j<ptr->biWidth;j++){
                colorId=ReadBMP(1,f);
                *(ptr->pixelList+i*ptr->biWidth+j)[0]=*(colorPalette+colorId)[0];
                *(ptr->pixelList+i*ptr->biWidth+j)[1]=*(colorPalette+colorId)[1];
                *(ptr->pixelList+i*ptr->biWidth+j)[2]=*(colorPalette+colorId)[2];
            }
            fseek(f,4-((ptr->biWidth*ptr->biBitCount)>>3)&3,SEEK_CUR);//��������������
        }
    }
    else{//��ʱû�е�ɫ��
        int i,j;
        int tempColorCode[4]={0};

        ptr->pixelList=(int(*)[3])malloc(sizeof(int)*ptr->biHeight*ptr->biWidth*3);
        fseek(f,bfOffBits,SEEK_SET);
        for(i=0;i<abs(ptr->biHeight);i++){
            for(j=0;j<ptr->biWidth;j++){
                fread(tempColorCode,1,1,f);
                fread(tempColorCode+1,1,1,f);
                fread(tempColorCode+2,1,1,f);
                if(ptr->biBitCount==32) fseek(f,1,SEEK_CUR);
                *(ptr->pixelList+i*ptr->biWidth+j)[0]=tempColorCode[2];
                *(ptr->pixelList+i*ptr->biWidth+j)[1]=tempColorCode[1];
                *(ptr->pixelList+i*ptr->biWidth+j)[2]=tempColorCode[0];
            }
            fseek(f,4-((ptr->biWidth*ptr->biBitCount)>>3)&3,SEEK_CUR);
        }
    }
    return ptr;
}

ImageInfo* ReadImage(FILE* f)
{
    ImageInfo* ptr;
    int temp[5];
    ptr=(ImageInfo*)malloc(sizeof(ImageInfo));
    fseek(f,0,SEEK_SET);
    fread(temp,sizeof(int),4,f);
    ptr->biWidth=temp[0];
    ptr->biHeight=temp[1];
    ptr->biBitCount=temp[2];
    ptr->biClrUsed=temp[3];

    ptr->pixelList=(int(*)[3])malloc(sizeof(int)*ptr->biWidth*ptr->biHeight*3);
    fread(ptr->pixelList,sizeof(int)*3,ptr->biWidth*ptr->biHeight,f);

    return ptr;
}

void WriteImage(ImageInfo* ptr,FILE* f)
{
    int temp[5];
    temp[0]=ptr->biWidth;
    temp[1]=ptr->biHeight;
    temp[2]=ptr->biBitCount;
    temp[3]=ptr->biClrUsed;
    fwrite(temp,sizeof(int),4,f);
    fwrite(ptr->pixelList,sizeof(int)*3,ptr->biHeight*ptr->biWidth,f);
}

