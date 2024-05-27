#ifndef __TEXT_STRUCTURE_INCLUDED__
#define __TEXT_STRUCTURE_INCLUDED__
#include <stdio.h>

/*
*
* document: 24/5/21 �־��� 
* program: 24/5/21 �־��� 
* last change: 24/5/21 �־��� 
* details in github history
*
* ����ṹ��StyleChar��StyleString�������ļ��������༭�߼�����ʾ�߼���
* ��֧�ֶ�д 
*
*/

typedef struct{
	char context;
	char style; //0 for nothing, 1 for Bold, 2 for Italic, 3 for Bold+Italic
} StyleChar;

typedef struct{
	int pointSize;
	int indent;
	int contextLen;
	int contextSpace;
	const char* color;
	const char* font;
	StyleChar* context;
} StyleString;

/* 
*
* Read��Write����ֱ�����ļ�������ֱ�����ļ��н��ж�д����Ӧ���κζ�����ٶ� 
*
*/
StyleString* ReadStyleString(FILE* f);
void WriteStyleString(StyleString* str, FILE* f);

/*
*
* GetStyleStringHeight����ͨ����������ָ��Ϳ��ֱ�Ӽ�����߶�
* �����һ�е�λ����Ӣ�� 
*
*/
void GetStyleStringHeight(StyleString* str, double width);

/*
*
* Draw����ͨ��libgraphics�⣬ֱ����(cx, cy)Ϊ**���Ͻ�**�������л�����Ӧ����
* �������ݿ��Ϊ width����Ҫ���Ƹ߶Ƚ��� beginHeight �� endHeight ֮��Ĳ��� 
* Ҳ����˵�������������½�Ϊ(dx+width, dy+endHeight-beginHeight) 
* �����һ�е�λ����Ӣ�� 
*
*/
void DrawStyleString(StyleString* str, double cx, double cy, double width, double beginHeight, double endHeight);

#endif
