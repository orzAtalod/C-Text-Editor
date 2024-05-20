#include "textStructure.h"
#include "fileSystemCore.h"
#include <stdlib.h>

static int  readBufferInt[255];
static char readBufferChar[65536];

StyleString* ReadStyleString(FILE* f)
{
	StyleString* res = (StyleString*)malloc(sizeof(StyleString));
	fread(readBufferInt,sizeof(int),5,f);
	res->pointSize = readBufferInt[0];
	res->indent    = readBufferInt[1];
	res->color     = LookupIDInColorTable(readBufferInt[2]);
	res->font      = LookupIDInFontTable(readBufferInt[3]);
	res->context   = (StyleChar*)malloc(readBufferInt[4]*sizeof(StyleChar));
	
	//TODO: 完成StyleChar的读入函数 
}
