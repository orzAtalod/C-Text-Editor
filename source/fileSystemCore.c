#include<stdio.h>
#include"fileSystemCore.h"
#include<stdlib.h>
#include<string.h>
#include<assert.h>
//��дcolor����дfont���ȶ�color���ٶ�font 
typedef struct{
	char name[10];
	double code[3];//��ԭɫֵ 
}colorInfo;
struct colorTable{
	colorInfo first[20];//��һ���� 
	char *second[20];//�ڶ����� 
	int flength;//first�ж��������� 
	int slength;//second�ж��������� 
};
static struct colorTable* colorPage[255]; 
static int curColorPage=0;//��ǰ����һҳ 
static int colorPageNum=0;//��ǰ�ж���ҳ 


void ChangePageOfColorTable(int p)//������ת��ҳ��Ҳ�Ǵ���һ��ҳ�����Ե�һ�ε���color���ݽṹʱ����Ҫ���������������һ��ҳ 
{
	if(p<=colorPageNum){
		curColorPage=p;//��һҳ�Ѿ����ڣ�ֱ��ת�� 
	}else{//����ҳ 
		curColorPage=p;
		colorPage[p]=(struct colorTable*)malloc(sizeof(struct colorTable));
		colorPage[p]->flength=0;
		colorPage[p]->slength=0;
	}

}

void ReadColorTable(FILE* f, ColorDefinitionFunction func)
{
	char *tempname;
	tempname=(char *)malloc(sizeof(char)*10);
	double tempcode[3];
	fread(tempname,sizeof(char),10,f);
	fread(tempcode,sizeof(double),3,f);
	while(strcmp(tempname,"firstend")!=0){//firstend�ǽ�����־  
		RegisterColorName(tempname,tempcode[0],tempcode[1],tempcode[2]);
		fread(tempname,sizeof(char),10,f);
		fread(tempcode,sizeof(double),3,f);
	}
	fread(tempname,sizeof(char),10,f);
	while(strcmp(tempname,"secondend")!=0){//secondend�ǽ�����־  
		RegisterColorTable(tempname);
		fread(tempname,sizeof(char),10,f);
	}
}

void WriteColorTable(FILE* f)
{
	struct colorTable* p=colorPage[curColorPage];//ָ��ǰҳ 
	int firstl=p->flength;
	int secondl=p->slength;
	int i;
	for(i=1;i<=firstl;i++){
		fwrite(p->first[i].name,sizeof(char),10,f);
		fwrite(p->first[i].code,sizeof(double),3,f);
	}
	char endchar[10]="firstend";//���д�������־ 
	double enddouble[3]={0,0,0};
	fwrite(endchar,sizeof(char),10,f);
	fwrite(enddouble,sizeof(double),3,f);
	
	for(i=1;i<=secondl;i++){
		fwrite(p->second[i],sizeof(char),10,f);
	}
	strcpy(endchar,"secondend");
	fwrite(endchar,sizeof(char),10,f);
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
	assert(flag==1);//�ҵ������� 
	strcpy(p->first[l+1].name,colorName);//������ɫ��
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
			return i;//����Ѿ�������ͷ���id 
		}
	}
	return 0;
}

//font����


struct fontTable{
	char *second[5];
	int length;//�ж��������� 
};
static struct fontTable* fontPage[255];
static int curFontPage=0;//��ǰ����һҳ 
static int fontPageNum=0;//��ǰ�ж���ҳ 


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
	fread(tempname,sizeof(char),10,f);
	while(strcmp(tempname,"fontend")!=0){//fontend�ǽ�����־ 
		RegisterFontTable(tempname);
		fread(tempname,sizeof(char),10,f);
	}
}

void WriteFontTable(FILE* f)
{
	struct fontTable* p=fontPage[curFontPage];
	int l=p->length;
	int i;
	for(i=1;i<=l;i++){
		fwrite(p->second[i],sizeof(char),10,f);
	}
	char endchar[10]="fontend";//���д�������־ 
	fwrite(endchar,sizeof(char),10,f);
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
	p->second[l+1]=malloc(sizeof(char)*10);
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

