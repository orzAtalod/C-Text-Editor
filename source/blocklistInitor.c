#include "blocklistInitor.h"
#include "blockList.h"
#include "textStructure.h"

static void* styleStringReader(FILE* f)
{
	return (void*)ReadStyleString(f);
}

static void styleStringWriter(void* str, FILE* f)
{
	WriteStyleString((StyleString*)str, f);
}

static void styleStringDrawer(void* str, double cx, double cy, double w, double begH, double endH)
{
	DrawStyleString((StyleString*)str, cx, cy, w, begH, endH);
}

static double styleStringHeighter(void* str, double w)
{
	return GetStyleStringHeight((StyleString*)str, w);
}

static int styleStringGetPos(void* str, double w, double x, double y)
{
	return GetPositionFromRelativeXY_StyleString((StyleString*)str, w, x, y);
}

static double styleStringGetRX(void* str, double w, int pos)
{
	return GetRelativeXFromPosition_StyleString((StyleString*)str, w, pos);
}

static double styleStringGetRY(void* str, double w, int pos)
{
	return GetRelativeYFromPosition_StyleString((StyleString*)str, w, pos);
}

static double styleStringGetEH(void* str)
{
	return GetStyleStringElementHeight((StyleString*)str);
} 

void BlockListInitMethods()
{
	RegisterReaderMethod(1,      styleStringReader);
	RegisterWriterMethod(1,      styleStringWriter);
	RegisterDrawFunc(1,          styleStringDrawer);
	RegisterGetHeightFunc(1,     styleStringHeighter);
	RegisterGetPositionFunc(1,   styleStringGetPos);
	RegisterGetRelativeXYFunc(1, styleStringGetRX, styleStringGetRY);
	RegisterGetEleHeightFunc(1,  styleStringGetEH);
}
