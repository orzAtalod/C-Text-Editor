#include "blocklistInitor.h"
#include "blockList.h"
#include "textStructure.h"
#include "imageStructure.h"

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

static void* imageInfoReader(FILE* f)
{
	return (void*)ReadImage(f);
}

static void imageInfoWriter(void* str, FILE* f)
{
	WriteImage((ImageInfo*)str, f);
}

static void imageInfoDrawer(void* str, double cx, double cy, double w, double begH, double endH)
{
	FakeDraw((ImageInfo*)str, cx, cy, w, begH, endH);
}

static double imageInfoHeighter(void* str, double w)
{
	return GetImageHeight((ImageInfo*)str, w);
}

static int imageInfoGetPos(void* str, double w, double x, double y)
{
	return GetPositionFromRelativeXY_ImageInfo((ImageInfo*)str, w, x, y);
}

static double imageInfoGetRX(void* str, double w, int pos)
{
	return GetRelativeXFromPosition_ImageInfo((ImageInfo*)str, w, pos);
}

static double imageInfoGetRY(void* str, double w, int pos)
{
	return GetRelativeYFromPosition_ImageInfo((ImageInfo*)str, w, pos);
}

static double imageInfoGetEH(void* str)
{
	return GetImageInfoElementHeight((ImageInfo*)str);
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

	RegisterReaderMethod(2,      imageInfoReader);
	RegisterWriterMethod(2,      imageInfoWriter);
	RegisterDrawFunc(2,          imageInfoDrawer);
	RegisterGetHeightFunc(2,     imageInfoHeighter);
	RegisterGetPositionFunc(2,   imageInfoGetPos);
	RegisterGetRelativeXYFunc(2, imageInfoGetRX, imageInfoGetRY);
	RegisterGetEleHeightFunc(2,  imageInfoGetEH);
}
