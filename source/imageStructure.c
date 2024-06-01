#include "imageStructure.h"
#include <stdio.h>
#include "fileSystemCore.h"
#include <assert.h>
#include <stdlib.h>
/*call the functions related to colors of fileSystemCore.h 
 *to satisfy the color need of images. For the huge amount of colors needed,
 *the color table needs to be changed.！！需要改一下colorTable的上限*/

int ReadBMP(int count,FILE* f)
{
    int temp[5],target=0,i;
    fread(temp,1,count,f);//size其实默认是1，因为每个字节数据需要单独处理，一个字节的二进制码可以正确转成int吗？
    for(i=0;i<count;i++){
        target=target+temp[i]<<(8*i);//低地址存低位数据，高地址存高位数据
    }
    return target;
}

const char* GetColorName(int r,int g,int b)//将RGB值变成字符串作为唯一的名字标识
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
    if(strcmp(tempchar,"MB")!=0){//判定图片是否是BMP
        assert("imagType is false");//由于存储的特性，所以字母反了一下
    }
    fseek(f,28,SEEK_SET);

    biCompression=ReadBMP(2,f);//获取BMP压缩方式
    if(biCompression!=0){
        assert("imageType is false");
    }

    fseek(f,10,SEEK_SET);
    bfOffBits=ReadBMP(4,f);//获取从文件开头到bitmap information的字节偏移量，后面要用

    ptr=(ImageInfo*)malloc(sizeof(ImageInfo));
    fseek(f,18,SEEK_SET);
    ptr->biWidth=ReadBMP(4,f);
    ptr->biHeight=ReadBMP(4,f);
    ptr->biBitCount=ReadBMP(2,f);
    if(ptr->biBitCount<24){//根据每个像素所需比特位的不同采用不同的编码方式
        fseek(f,46,SEEK_SET);
        ptr->biClrUsed=ReadBMP(4,f);//获取所用到的颜色索引数
        ptr->colorIdList=(int*)malloc(sizeof(int)*ptr->biClrUsed);
        int i;
        int tempColorCode[4];
        const char* colorName;

        for(i=0;i<ptr->biClrUsed;i++){
            fread(tempColorCode,1,4,f);
            colorName=GetColorName(tempColorCode[2],tempColorCode[1],tempColorCode[0]);//根据颜色RGB值生成唯一名字
            ptr->colorIdList[i]=RegisterColorTable(colorName);
            RegisterColorName(colorName,tempColorCode[2]/255.0,tempColorCode[1]/255.0,tempColorCode[0]/255.0);//将RGB转化为simpleGUI可用
        }

        ptr->pixelList=(int*)malloc(sizeof(int)*ptr->biHeight*ptr->biWidth);
        fseek(f,bfOffBits,SEEK_SET);
        for(i=0;i<abs(ptr->biHeight);i++){
            fread(ptr->pixelList+ptr->biWidth*i,1,ptr->biWidth,f);//读入整行数据
            fseek(f,4-((ptr->biWidth*ptr->biBitCount)>>3)&3,SEEK_CUR);//跳过无意义的填充
        }
    }
    else{//此时没有调色板
        int i,j;
        int tempColorCode[4];
        const char* colorName;

        ptr->pixelList=(int*)malloc(sizeof(int)*ptr->biHeight*ptr->biWidth);
        fseek(f,bfOffBits,SEEK_SET);
        for(i=0;i<abs(ptr->biHeight);i++){
            for(j=0;j<ptr->biWidth;j++){
                fread(tempColorCode,1,ptr->biBitCount/8,f);
                colorName=GetColorName(tempColorCode[2],tempColorCode[1],tempColorCode[0]);
                ptr->pixelList[i*ptr->biWidth+j]=RegisterColorTable(colorName);//每个像素点对应颜色id
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

