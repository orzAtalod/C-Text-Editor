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

/* 
*
* Read和Write函数直接与文件操作，直接在文件中进行读写，不应有任何多读和少读 
*
*/
StyleString* ReadStyleString(FILE* f);
void WriteStyleString(StyleString* str, FILE* f);

/*
*
* GetStyleStringHeight函数通过数据内容指针和宽度直接计算出高度
* 传入的一切单位都是英寸 
*
*/
void GetStyleStringHeight(StyleString* str, double width);

/*
*
* Draw函数通过libgraphics库，直接在(cx, cy)为**左上角**的区域中绘制相应内容
* 绘制内容宽度为 width，需要绘制高度介于 beginHeight 和 endHeight 之间的部分 
* 也就是说，绘制内容右下角为(dx+width, dy+endHeight-beginHeight) 
* 传入的一切单位都是英寸 
*
*/
void DrawStyleString(StyleString* str, double cx, double cy, double width, double beginHeight, double endHeight);

#endif
