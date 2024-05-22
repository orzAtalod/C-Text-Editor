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
	res->contextLen = res->contextSpace = bufferInt[4];
	res->context   = (StyleChar*)malloc(res->contextSpace*sizeof(StyleChar));
	fread(res->context, sizeof(StyleChar), res->contextLen, f);
}


void WriteStyleString(StyleString* str, FILE* f)
{
	bufferInt[0] = str->pointSize;
	bufferInt[1] = str->indent;
	bufferInt[2] = LookupColorNameInColorTable(str->color);
	bufferInt[3] = LookupFontNameInFontTable(str->font);
	bufferInt[4] = str->contextLen;
	fwrite(bufferInt, sizeof(int), 5, f);
	fwrite(str->context, sizeof(StyleChar), str->contextLen, f);
}
