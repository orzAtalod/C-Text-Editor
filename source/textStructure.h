#ifndef __TEXT_STRUCTURE_INCLUDED__
#define __TEXT_STRUCTURE_INCLUDED__
#include <stdio.h>

/*
*
* document: 24/5/21 林景行 
* program: 24/5/21 林景行 
* last change: 24/5/21 林景行 
* details in github history
*
* 定义结构体StyleChar和StyleString，用于文件操作、编辑逻辑和显示逻辑。
* 现支持读写 
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
