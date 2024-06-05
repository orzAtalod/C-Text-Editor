#include "textStructure.h"
#include "fileSystemCore.h"
#include <stdlib.h>

static int bufferInt[255];

StyleString* ReadStyleString(FILE* f)
{
	StyleString* res = (StyleString*)malloc(sizeof(StyleString));
	fread(bufferInt, sizeof(int), 5, f);
	res->pointSize = bufferInt[0];
	res->indent    = bufferInt[1];
	res->color     = LookupIDInColorTable(bufferInt[2]);
	res->font      = LookupIDInFontTable(bufferInt[3]);
	res->contentLen = res->contentSpace = bufferInt[4];
	res->content   = (StyleChar*)malloc(res->contentSpace*sizeof(StyleChar));
	fread(res->content, sizeof(StyleChar), res->contentLen, f);
}


void WriteStyleString(StyleString* str, FILE* f)
{
	bufferInt[0] = str->pointSize;
	bufferInt[1] = str->indent;
	bufferInt[2] = LookupColorNameInColorTable(str->color);
	bufferInt[3] = LookupFontNameInFontTable(str->font);
	bufferInt[4] = str->contentLen;
	fwrite(bufferInt, sizeof(int), 5, f);
	fwrite(str->content, sizeof(StyleChar), str->contentLen, f);
}

static char* tmp[255];
#define TEXT_STRING_MARGIN 0.01
double GetStyleStringHeight(StyleString* str, double width)
{
	SetPointSize(str->pointSize);
	SetFont(str->font);
	const double lineHeight = GetFontHeight();

	int curIndex = 0;
	double curHeight = lineHeight;
	for(int i=0; i<=str->indent; ++i)
	{
		tmp[curIndex++] = ' ';
	}
	for(int i=0; i<str->contentLen; ++i)
	{
		tmp[curIndex++] = str->contentLen[i].content;
		tmp[curIndex] = '\0';
		if(TextStringWidth(tmp) > width-2*TEXT_STRING_MARGIN)
		{
			tmp[0] = str->contentLen[i].content;
			tmp[1] = '\0';
			curIndex = 1;
			curHeight += lineHeight;
		} 
	}

	return -curHeight;
}

static void drawStrings(StyleString* str, int start, int end)
{
	if(start==-1)
	{
		for(int i=1; i<=str->indent; ++i)
		{
			DrawTextString(" ");
		}
		++start;
	}
	for(int i=start; i<=end; ++i)
	{
		char ttmp[2];
		ttmp[0] = str->content[i].content;
		ttmp[1] = '\0';
		SetStyle(str->content[i].style);
		DrawTextString(ttmp);
	}
}

void DrawStyleString(StyleString* str, double cx, double cy, double width, double begH, double endH)
{
	SetPointSize(str->pointSize);
	SetFont(str->font);
	SetPenColor(str->color);
	const double lineHeight = GetFontHeight();

	int curIndex = 0;
	int preIndex = -1;
	double nowH = 0;
	for(int i=0; i<=str->indent; ++i)
	{
		tmp[curIndex++] = ' ';
	}
	for(int i=0; i<str->contentLen; ++i)
	{
		tmp[curIndex++] = str->contentLen[i].content;
		tmp[curIndex] = '\0';
		if(TextStringWidth(tmp) > width-2*TEXT_STRING_MARGIN)
		{
			if(nowH>=begH-1E-5 && nowH<=endH+1E-5)
			{
				MovePen(cx+TEXT_STRING_MARGIN,cy-nowH+begH-GetFontAscent())
				drawStrings(str,preIndex,i-1);
			}
			preIndex = i;
			tmp[0] = str->contentLen[i].content;
			tmp[1] = '\0';
			curIndex = 1;
			nowH += lineHeight;
		}
	}
}

int GetPositionFromRelativeXY_StyleString(StyleString* str, double width, double rx, double ry)
{
	SetPointSize(str->pointSize);
	SetFont(str->font);
	if(ry > 0) return 0;
	if(ry < GetStyleStringHeight(str,width)) return str->contentLen;
	int lineN = ry/GetFontHeight();

	int curIndex = 0;
	for(int i=0; i<=str->indent; ++i)
	{
		tmp[curIndex++] = ' ';
	}
	for(int i=0; i<str->contentLen; ++i)
	{
		tmp[curIndex++] = str->contentLen[i].content;
		tmp[curIndex] = '\0';
		if(!lineN)
		{
			if(TextStringWidth(tmp)+TEXT_STRING_MARGIN > rx) return i;
		}
		if(TextStringWidth(tmp) > width-2*TEXT_STRING_MARGIN)
		{
			if(!lineN) return i+1;
			tmp[0] = str->contentLen[i].content;
			tmp[1] = '\0';
			curIndex = 1;
			--lineN;
		}
	}
	return str->contentLen;
}

double GetRelativeXFromPosition_StyleString(StyleString* str, double width, int position)
{
	SetPointSize(str->pointSize);
	SetFont(str->font);
	const double lineHeight = GetFontHeight();

	int curIndex = 0;
	double curHeight = lineHeight;
	for(int i=0; i<=str->indent; ++i)
	{
		tmp[curIndex++] = ' ';
	}
	for(int i=0; i<position; ++i)
	{
		tmp[curIndex++] = str->contentLen[i].content;
		tmp[curIndex] = '\0';
		if(TextStringWidth(tmp) > width-2*TEXT_STRING_MARGIN)
		{
			tmp[0] = str->contentLen[i].content;
			tmp[1] = '\0';
			curIndex = 1;
			curHeight += lineHeight;
		} 
	}

	return TEXT_STRING_MARGIN + TextStringWidth(tmp);
}

double GetRelativeYFromPosition_StyleString(StyleString* str, double width, int position)
{
	SetPointSize(str->pointSize);
	SetFont(str->font);
	const double lineHeight = GetFontHeight();

	int curIndex = 0;
	double curHeight = lineHeight;
	for(int i=0; i<=str->indent; ++i)
	{
		tmp[curIndex++] = ' ';
	}
	for(int i=0; i<position; ++i)
	{
		tmp[curIndex++] = str->contentLen[i].content;
		tmp[curIndex] = '\0';
		if(TextStringWidth(tmp) > width-2*TEXT_STRING_MARGIN)
		{
			tmp[0] = str->contentLen[i].content;
			tmp[1] = '\0';
			curIndex = 1;
			curHeight += lineHeight;
		} 
	}

	return -curHeight;
}