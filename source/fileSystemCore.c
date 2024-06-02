#include<stdio.h>
#include"fileSystemCore.h"
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#define NAME_STRING_LIMIT 20 

//��дcolor����дfont���ȶ�color���ٶ�font 
typedef struct{
	char name[NAME_STRING_LIMIT];
	double code[3];//��ԭɫֵ 
}colorInfo;
struct colorTable{
	int flength;//first�ж��������� 
	int slength;//second�ж��������� 
	char *second[255];//�ڶ����� 
	colorInfo first[255];//��һ���� 
};
static struct colorTable* colorPage[255]; 
static int curColorPage=0;//��ǰ����һҳ 
static unsigned long long colorPageExsists[4];

void ChangePageOfColorTable(int p)//������ת��ҳ��Ҳ�Ǵ���һ��ҳ�����Ե�һ�ε���color���ݽṹʱ����Ҫ���������������һ��ҳ 
{
	if(colorPageExsists[p%64] | (1LLU<<(p/64))){
		curColorPage=p;//��һҳ�Ѿ����ڣ�ֱ��ת�� 
	}else{//����ҳ 
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
	char *tempname;
	tempname=(char *)malloc(sizeof(char)*NAME_STRING_LIMIT);
	double tempcode[3];
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	fread(tempcode,sizeof(double),3,f);
	while(strcmp(tempname,"_firstend")!=0){//firstend�ǽ�����־  
		RegisterColorName(tempname,tempcode[0],tempcode[1],tempcode[2]);
		fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
		fread(tempcode,sizeof(double),3,f);
	}
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	while(strcmp(tempname,"_secondend")!=0){//secondend�ǽ�����־  
		(void)RegisterColorTable(tempname); //make complier happy
		fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	}
}

void WriteColorTable(FILE* f)
{
	struct colorTable* p=colorPage[curColorPage];//ָ��ǰҳ 
	int firstl=p->flength;
	int secondl=p->slength;
	int i;
	for(i=1;i<=firstl;i++){
		fwrite(p->first[i].name,sizeof(char),NAME_STRING_LIMIT,f);
		fwrite(p->first[i].code,sizeof(double),3,f);
	}
	char endchar[NAME_STRING_LIMIT]="_firstend";//���д�������־ 
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
	if(l!=0){//colorTable���Ƿ���Ԫ�� 
		for(i=1;i<=l;i++){
			if(strcmp(colorName,p->second[i])==0){
				return i;//����Ѿ�������ͷ���id 
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
	strcpy(p->first[l+1].name,colorName);//������ɫ��
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
			return i;//����Ѿ�������ͷ���id 
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


//font����


struct fontTable{
	int length;//�ж��������� 
	char *second[255];
};
static struct fontTable* fontPage[255];
static int curFontPage=0;//��ǰ����һҳ 
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
	char *tempname;
	tempname=(char *)malloc(sizeof(char)*NAME_STRING_LIMIT);
	fread(tempname,sizeof(char),NAME_STRING_LIMIT,f);
	while(strcmp(tempname,"_fontend")!=0){//fontend�ǽ�����־ 
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
	char endchar[NAME_STRING_LIMIT]="_fontend";//���д�������־ 
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
				return i;//����Ѿ�������ͷ���id 
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
			return i;//����Ѿ�������ͷ���id 
		}
	}
	return 0;
}

