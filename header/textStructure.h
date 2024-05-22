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

StyleString* ReadStyleString(FILE* f);
void         WriteStyleString(StyleString* str, FILE* f);

#endif
