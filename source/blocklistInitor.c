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

void BlockListInitMethods()
{
	RegisterReaderMethod(1,styleStringReader);
	RegisterWriterMethod(1,styleStringWriter);
	RegisterDrawMethod(1,DrawStyleString);
	RegisterGetPositionFunc(1, GetPositionFromRelativeXY_StyleString);
	RegisterGetRelativeXYFunc(1, GetRelativeXFromPosition_StyleString, GetRelativeYFromPosition_StyleString);
}
