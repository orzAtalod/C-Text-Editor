#ifndef __TEXT_STRUCTURE_INCLUDED__
#define __TEXT_STRUCTURE_INCLUDED__
#include <stdio.h>

/*
*
* 定义结构体StyleChar和StyleString，用于文件操作、编辑逻辑和显示逻辑。
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
* Read和Write函数直接与文件操作，直接在文件中进行读写，不应有任何多读和少读 
*
*/
StyleString* ReadStyleString(FILE* f);
void WriteStyleString(StyleString* str, FILE* f);

/*
*
* GetStyleStringHeight函数通过数据内容指针和宽度直接计算出高度
* 传入的一切单位都是英寸，高度为负数值
*
*/
double GetStyleStringHeight(StyleString* str, double width);

/*
*
* Draw函数通过libgraphics库，直接在(cx, cy)为**左上角**的区域中绘制相应内容
* 绘制内容宽度为 width，需要绘制高度介于 beginHeight 和 endHeight 之间的部分 
* 也就是说，绘制内容右下角为(dx+width, dy-beginHeight+endHeight) 
* 传入的一切单位都是英寸，高度为负数值
*
*/
void DrawStyleString(StyleString* str, double cx, double cy, double width, double beginHeight, double endHeight);

/*
*
* position是一个表示块内位置的整型变量，对于textString为光标在哪一个字符之前
* 返回的RelativeXY为相应字符左上角的位置
*
*/

int GetPositionFromRelativeXY_StyleString(StyleString* str, double width, double rx, double ry);
double GetRelativeXFromPosition_StyleString(StyleString* str, double width, int position);
double GetRelativeYFromPosition_StyleString(StyleString* str, double width, int position);

double GetStyleStringElementHeight(StyleString* str);

#endif
