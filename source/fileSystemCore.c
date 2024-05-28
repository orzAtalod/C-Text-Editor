#include<stdio.h>
#include"fileSystemCore.h"
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#define NAME_STRING_LIMIT 10 

//先写color，再写font；先读color，再读font 
typedef struct{
	char name[NAME_STRING_LIMIT];
	double code[3];//三原色值 
}colorInfo;
struct colorTable{
	colorInfo first[20];//第一部分 
	char *second[20];//第二部分 
	int flength;//first有多少数据项 
	int slength;//second有多少数据项 
};
static struct colorTable* colorPage[255]; 
static int curColorPage=0;//当前在哪一页 
static int colorPageNum=0;//当前有多少页 


void ChangePageOfColorTable(int p)//不仅是转换页，也是创建一个页，所以第一次调用color数据结构时，需要先用这个函数创建一个页 
{
	if(p<=colorPageNum){
		curColorPage=p;//这一页已经存在，直接转换 
	}else{//创建页 
		curColorPage=p;
		colorPage[p]=(struct colorTable*)malloc(sizeof(struct colorTable));
		colorPage[p]->flength=0;
		colorPage[p]->slength=0;
	}

}

void ReadColorTable(FILE* f, ColorDefinitionFunction func)
{
	char *tempname;
	tempname=(char *)malloc(sizeof(char)*NAME_STRING_LIMIT);
	double tempcode[3];
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	fread(tempcode,sizeof(double),3,f);
	while(strcmp(tempname,"firstend")!=0){//firstend是结束标志  
		RegisterColorName(tempname,tempcode[0],tempcode[1],tempcode[2]);
		fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
		fread(tempcode,sizeof(double),3,f);
	}
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	while(strcmp(tempname,"secondend")!=0){//secondend是结束标志  
		RegisterColorTable(tempname);
		fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	}
}

void WriteColorTable(FILE* f)
{
	struct colorTable* p=colorPage[curColorPage];//指向当前页 
	int firstl=p->flength;
	int secondl=p->slength;
	int i;
	for(i=1;i<=firstl;i++){
		fwrite(p->first[i].name,sizeof(char),NAME_STRING_LIMIT,f);
		fwrite(p->first[i].code,sizeof(double),3,f);
	}
	char endchar[NAME_STRING_LIMIT]="firstend";//最后写入结束标志 
	double enddouble[3]={0,0,0};
	fwrite(endchar,sizeof(char),NAME_STRING_LIMIT,f);
	fwrite(enddouble,sizeof(double),3,f);
	
	for(i=1;i<=secondl;i++){
		fwrite(p->second[i],sizeof(char),NAME_STRING_LIMIT,f);
	}
	strcpy(endchar,"secondend");
	fwrite(endchar,sizeof(char),NAME_STRING_LIMIT,f);
}

int RegisterColorTable(const char* colorName)
{
	struct colorTable* p=colorPage[curColorPage];
	int l=p->slength;
	int i;
	if(l!=0){//colorTable里是否有元素 
		for(i=1;i<=l;i++){
			if(strcmp(colorName,p->second[i])==0){
				return i;//如果已经在里面就返回id 
			}
		}
	}
	p->second[l+1]=malloc(sizeof(char)*10);
	strcpy(p->second[l+1],colorName);
	p->slength++;
	return l+1;
 } 
 
void RegisterColorName(const char* colorName,double r,double g,double b)
{
	struct colorTable* p=colorPage[curColorPage];
	int l=p->flength;
	int i,flag;
	for(i=1;i<=l;i++){
		if(strcmp(colorName,p->first[i].name)==0){
			flag=1;
		}
	}
	assert(flag==1);//找到，报错 
	strcpy(p->first[l+1].name,colorName);//存入颜色名
	p->first[l+1].code[0]=r;
	p->first[l+1].code[0]=g;
	p->first[l+1].code[0]=b;
	p->flength++;
}

const char* LookupIDInColorTable(int id)
{
	struct colorTable* p=colorPage[curColorPage];
	const char* temp=p->second[id];
	return temp;
}

int LookupColorNameInColorTable(const char* colorName)
{
	struct colorTable* p=colorPage[curColorPage];
	int l=p->slength;
	int i;
	for(i=1;i<=l;i++){
		if(strcmp(colorName,p->second[i])==0){
			return i;//如果已经在里面就返回id 
		}
	}
	return 0;
}

//font部分


struct fontTable{
	char *second[5];
	int length;//有多少数据项 
};
static struct fontTable* fontPage[255];
static int curFontPage=0;//当前在哪一页 
static int fontPageNum=0;//当前有多少页 


void ChangePageOfFontTable(int p)
{
	if(p<=fontPageNum){
		curFontPage=p;
	}else{
		curFontPage=p;
		fontPage[p]=(struct fontTable*)malloc(sizeof(struct fontTable));
		fontPage[p]->length=0;
	}

}

void ReadFontTable(FILE* f)
{
	char *tempname;
	tempname=(char *)malloc(sizeof(char)*10);
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	while(strcmp(tempname,"fontend")!=0){//fontend是结束标志 
		RegisterFontTable(tempname);
		fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	}
}

void WriteFontTable(FILE* f)
{
	struct fontTable* p=fontPage[curFontPage];
	int l=p->length;
	int i;
	for(i=1;i<=l;i++){
		fwrite(p->second[i],sizeof(char),NAME_STRING_LIMIT,f);
	}
	char endchar[NAME_STRING_LIMIT]="fontend";//最后写入结束标志 
	fwrite(endchar,sizeof(char),NAME_STRING_LIMIT,f);
}

int RegisterFontTable(const char* fontName)
{
	struct fontTable* p=fontPage[curFontPage];
	int l=p->length;
	int i;
	if(l!=0){
		for(i=1;i<=l;i++){
			if(strcmp(fontName,p->second[i])==0){
				return i;//如果已经在里面就返回id 
			}
		}
	} 
	p->second[l+1]=malloc(sizeof(char)*NAME_STRING_LIMIT);
	strcpy(p->second[l+1],fontName);
	p->length++;
	return l+1;
 } 
 

const char* LookupIDInFontTable(int id)
{
	struct fontTable* p=fontPage[curFontPage];
	const char* temp=p->second[id];
	return temp;
}

int LookupFontNameInFontTable(const char* fontName)
{
	struct fontTable* p=fontPage[curFontPage];
	int l=p->length;
	int i;
	for(i=1;i<=l;i++){
		if(strcmp(fontName,p->second[i])==0){
			return i;//如果已经在里面就返回id 
		}
	}
	return 0;
}

