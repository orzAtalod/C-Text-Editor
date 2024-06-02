#include "imageStructure.h"
#include <stdio.h>
#include "fileSystemCore.h"
#include <assert.h>
#include <stdlib.h>
/*call the functions related to colors of fileSystemCore.h 
 *to satisfy the color need of images. For the huge amount of colors needed,
 *the color table needs to be changed.������Ҫ��һ��colorTable������*/

int ReadBMP(int count,FILE* f)
{
    int temp[5],target=0,i;
    fread(temp,1,count,f);//size��ʵĬ����1����Ϊÿ���ֽ�������Ҫ��������һ���ֽڵĶ������������ȷת��int��
    for(i=0;i<count;i++){
        target=target+temp[i]<<(8*i);//�͵�ַ���λ���ݣ��ߵ�ַ���λ����
    }
    return target;
}

const char* GetColorName(int r,int g,int b)//��RGBֵ����ַ�����ΪΨһ�����ֱ�ʶ
{
    char temp[10];
    temp[0]=r/100+'0';
    temp[1]=(r/10-r/100*10)+'0';
    temp[2]=r%10+'0';
    temp[3]=g/100+'0';
    temp[4]=(g/10-g/100*10)+'0';
    temp[5]=g%10+'0';
    temp[6]=b/100+'0';
    temp[7]=(b/10-b/100*10)+'0';
    temp[8]=b%10+'0';
    temp[9]='\0';
    return temp;
}

ImageInfo* OpenImage(FILE* f)
{
    char tempchar[3]={'\0'};
    int biCompression,bfOffBits;
    ImageInfo* ptr;

    fread(tempchar,sizeof(char),2,f);
    if(strcmp(tempchar,"MB")!=0){//�ж�ͼƬ�Ƿ���BMP
        assert("imagType is false");//���ڴ洢�����ԣ�������ĸ����һ��
    }
    fseek(f,28,SEEK_SET);

    biCompression=ReadBMP(2,f);//��ȡBMPѹ����ʽ
    if(biCompression!=0){
        assert("imageType is false");
    }

    fseek(f,10,SEEK_SET);
    bfOffBits=ReadBMP(4,f);//��ȡ���ļ���ͷ��bitmap information���ֽ�ƫ����������Ҫ��

    ptr=(ImageInfo*)malloc(sizeof(ImageInfo));
    fseek(f,18,SEEK_SET);
    ptr->biWidth=ReadBMP(4,f);
    ptr->biHeight=ReadBMP(4,f);
    ptr->biBitCount=ReadBMP(2,f);
    if(ptr->biBitCount<24){//����ÿ�������������λ�Ĳ�ͬ���ò�ͬ�ı��뷽ʽ
        fseek(f,46,SEEK_SET);
        ptr->biClrUsed=ReadBMP(4,f);//��ȡ���õ�����ɫ������
        ptr->colorIdList=(int*)malloc(sizeof(int)*ptr->biClrUsed);
        int i;
        int tempColorCode[4];
        const char* colorName;

        for(i=0;i<ptr->biClrUsed;i++){
            fread(tempColorCode,1,4,f);
            colorName=GetColorName(tempColorCode[2],tempColorCode[1],tempColorCode[0]);//������ɫRGBֵ����Ψһ����
            ptr->colorIdList[i]=RegisterColorTable(colorName);
            RegisterColorName(colorName,tempColorCode[2]/255.0,tempColorCode[1]/255.0,tempColorCode[0]/255.0);//��RGBת��ΪsimpleGUI����
        }

        ptr->pixelList=(int*)malloc(sizeof(int)*ptr->biHeight*ptr->biWidth);
        fseek(f,bfOffBits,SEEK_SET);
        for(i=0;i<abs(ptr->biHeight);i++){
            fread(ptr->pixelList+ptr->biWidth*i,1,ptr->biWidth,f);//������������
            fseek(f,4-((ptr->biWidth*ptr->biBitCount)>>3)&3,SEEK_CUR);//��������������
        }
    }
    else{//��ʱû�е�ɫ��
        int i,j;
        int tempColorCode[4];
        const char* colorName;

        ptr->pixelList=(int*)malloc(sizeof(int)*ptr->biHeight*ptr->biWidth);
        fseek(f,bfOffBits,SEEK_SET);
        for(i=0;i<abs(ptr->biHeight);i++){
            for(j=0;j<ptr->biWidth;j++){
                fread(tempColorCode,1,ptr->biBitCount/8,f);
                colorName=GetColorName(tempColorCode[2],tempColorCode[1],tempColorCode[0]);
                ptr->pixelList[i*ptr->biWidth+j]=RegisterColorTable(colorName);//ÿ�����ص��Ӧ��ɫid
                RegisterColorName(colorName,tempColorCode[2]/255.0,tempColorCode[1]/255.0,tempColorCode[0]/255.0);
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
    fread(temp,sizeof(int),4,f);
    ptr->biWidth=temp[0];
    ptr->biHeight=temp[1];
    ptr->biBitCount=temp[2];
    ptr->biClrUsed=temp[3];
    if(ptr->biBitCount<24){
        ptr->colorIdList=(int*)malloc(sizeof(int)*ptr->biClrUsed);
        fread(ptr->colorIdList,sizeof(int),ptr->biClrUsed,f);
        ptr->pixelList=(int*)malloc(sizeof(int)*ptr->biWidth*ptr->biHeight);
        fread(ptr->pixelList,sizeof(int),ptr->biWidth*ptr->biHeight,f);
    }
    else{
        ptr->pixelList=(int*)malloc(sizeof(int)*ptr->biWidth*ptr->biHeight);
        fread(ptr->pixelList,sizeof(int),ptr->biWidth*ptr->biHeight,f);
    }
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
    if(ptr->biClrUsed<24){
        fwrite(ptr->colorIdList,sizeof(int),ptr->biClrUsed,f);
        fwrite(ptr->pixelList,sizeof(int),ptr->biHeight*ptr->biWidth,f);
    }
    else{
        fwrite(ptr->pixelList,sizeof(int),ptr->biHeight*ptr->biWidth,f);
    }
}

