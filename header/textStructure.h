#ifndef __TEXT_STRUCTURE_INCLUDED__
#define __TEXT_STRUCTURE_INCLUDED__
#include <stdio.h>

/*
*
* ����ṹ��StyleChar��StyleString�������ļ��������༭�߼�����ʾ�߼���
*
*/

typedef struct{
	char content;
	char style; //0 for nothing, 1 for Bold, 2 for Italic, 3 for Bold+Italic
} StyleChar;

typedef struct{
	int pointSize;
	int indent;
	int contentLen;
	int contentSpace;
	StyleChar* content;
	const char* color;
	const char* font;
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
* �����һ�е�λ����Ӣ�磬�߶�Ϊ����ֵ
*
*/
double GetStyleStringHeight(StyleString* str, double width);

/*
*
* Draw����ͨ��libgraphics�⣬ֱ����(cx, cy)Ϊ**���Ͻ�**�������л�����Ӧ����
* �������ݿ��Ϊ width����Ҫ���Ƹ߶Ƚ��� beginHeight �� endHeight ֮��Ĳ��� 
* Ҳ����˵�������������½�Ϊ(dx+width, dy-beginHeight+endHeight) 
* �����һ�е�λ����Ӣ�磬�߶�Ϊ����ֵ
*
*/
void DrawStyleString(StyleString* str, double cx, double cy, double width, double beginHeight, double endHeight);

/*
*
* position��һ����ʾ����λ�õ����ͱ���������textStringΪ�������һ���ַ�֮ǰ
* ���ص�RelativeXYΪ��Ӧ�ַ����Ͻǵ�λ��
*
*/

int GetPositionFromRelativeXY_StyleString(StyleString* str, double width, double rx, double ry);
double GetRelativeXFromPosition_StyleString(StyleString* str, double width, int position);
double GetRelativeYFromPosition_StyleString(StyleString* str, double width, int position);

double GetStyleStringElementHeight(StyleString* str);

#endif
