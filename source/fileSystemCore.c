#include<stdio.h>
#include"fileSystemCore.h"
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#define NAME_STRING_LIMIT 30 

//先写color，再写font；先读color，再读font

typedef struct{
	char name[NAME_STRING_LIMIT];
	double code[3];//三原色值 
}colorInfo;
struct colorTable{
	int flength;//first有多少数据项 
	int slength;//second有多少数据项 
	char *second[255];//第二部分 
	colorInfo first[255];//第一部分 
};

static struct colorTable* colorPage[255]; 
static int curColorPage=0;//当前在哪一页 
static unsigned long long colorPageExsists[4];

////////////////////////////////////////////////////////color 部分//////////////////////////////////////////////////

void ChangePageOfColorTable(int p)//不仅是转换页，也是创建一个页，所以第一次调用color数据结构时，需要先用这个函数创建一个页 
{
	if(colorPageExsists[p%64] | (1LLU<<(p/64))){
		curColorPage=p;//这一页已经存在，直接转换 
	}else{//创建页 
		colorPageExsists[p%64] |= 1LLU<<(p/64);
		curColorPage=p;
		colorPage[p]=(struct colorTable*)malloc(sizeof(struct colorTable));
		colorPage[p]->flength=0;
		colorPage[p]->slength=0;
	}
}

void ClearColorTable()
{
	if(colorPage[p]) free(colorPage[p]);
	colorPage[p]=(struct colorTable*)malloc(sizeof(struct colorTable));
	colorPage[p]->flength=0;
	colorPage[p]->slength=0;	
}

void ReadColorTable(FILE* f)
{
	char* tempname;
	tempname=(char *)malloc(sizeof(char)*NAME_STRING_LIMIT);
	double tempcode[3];
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	fread(tempcode,sizeof(double),3,f);
	while(strcmp(tempname,"_firstend")!=0){//firstend是结束标志  
		RegisterColorName(tempname,tempcode[0],tempcode[1],tempcode[2]);
		fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
		fread(tempcode,sizeof(double),3,f);
	}
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	while(strcmp(tempname,"_secondend")!=0){//secondend是结束标志  
		(void)RegisterColorTable(tempname); //make complier happy
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
	char endchar[NAME_STRING_LIMIT]="_firstend";//最后写入结束标志 
	double enddouble[3]={0,0,0};
	fwrite(endchar,sizeof(char),NAME_STRING_LIMIT,f);
	fwrite(enddouble,sizeof(double),3,f);
	
	for(i=1;i<=secondl;i++){
		fwrite(p->second[i],sizeof(char),NAME_STRING_LIMIT,f);
	}
	strcpy(endchar,"_secondend");
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
	p->second[l+1]=malloc(sizeof(char)*NAME_STRING_LIMIT);
	strcpy(p->second[l+1],colorName);
	p->slength++;
	return l+1;
 } 
 
int RegisterColorName(const char* colorName,double r,double g,double b)
{
	struct colorTable* p=colorPage[curColorPage];
	int l=p->flength;
	int i;
	for(i=1;i<=l;i++){
		if(strcmp(colorName,p->first[i].name)==0){
			return 1;
		}
	}
	strcpy(p->first[l+1].name,colorName);//存入颜色名
	p->first[l+1].code[0]=r;
	p->first[l+1].code[0]=g;
	p->first[l+1].code[0]=b;
	p->flength++;
	return 0; 
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

void TraverseColorDifinitions(ColorDefinitionTraverseFunction func)
{
	struct colorTable* p = colorPage[curColorPage];
	for(int i=1; i<=p->flength; ++i)
	{
		func(p->first[i].name, p->first[i].code[0], p->first[i].code[1], p->first[i].code[2]);
	}
}


/////////////////////////////////////////////////////font部分///////////////////////////////////


struct fontTable{
	int length;//有多少数据项 
	char* second[255];
};
static struct fontTable* fontPage[255];
static int curFontPage=0;//当前在哪一页 
static unsigned long long fontPageExsists[4];


void ChangePageOfFontTable(int p)
{
	if(fontPageExsists[p/64] | (1LLU<<(p%64))){
		curFontPage=p;
	}else{
		fontPageExsists[p/64] |= 1LLU<<(p%64);
		curFontPage=p;
		fontPage[p]=(struct fontTable*)malloc(sizeof(struct fontTable));
		fontPage[p]->length=0;
	}
}

void ClearFontTable()
{
	if(fontPage[p]) free(fontPage[p]);
	fontPage[p]=(struct fontTable*)malloc(sizeof(struct fontTable));
	fontPage[p]->length=0;	
}

void ReadFontTable(FILE* f)
{
	char* tempname;
	tempname=(char *)malloc(sizeof(char)*NAME_STRING_LIMIT);
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	while(strcmp(tempname,"_fontend")!=0){//fontend是结束标志 
		(void)RegisterFontTable(tempname); //make complier happy
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
	char endchar[NAME_STRING_LIMIT]="_fontend";//最后写入结束标志 
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

