#include "fileSystem.h"
#include <stdio.h>
#include "blockList.h"
#include "textStructure.h"
#include "fileSystemCore.h"

static void* styleStringReader(FILE* f)
{
	return (void*)ReadStyleString(f);
}

static void styleStringWriter(void* str, FILE* f)
{
	WriteStyleString((StyleString*)str, f);
}

static int curPage;

void ChangePageAtFileSystem(int page)
{
	curPage = page;
}

void InitFileSystem()
{
	RegisterReaderMethod(1,styleStringReader);
	RegisterWriterMethod(1,styleStringWriter);
}

int LoadFileAtPage(int page, const char* fileName)
{
	ChangePageOfColorTable(page);
	ClearColorTable();
	ChangePageOfFontTable(page);
	ClearFontTable();
	ChangePageOfBlockList(page);
	ClearBlockList();
	FILE* f = fopen(fileName,"rb");
	if(!f) return 1;
	ReadColorTable(f);
	ReadFontTable(f);
	LoadBlockList(f);
	fclose(f);
	return 0;
}

void SaveFileAtPage(int page, const char* fileName)
{
	FILE* f = fopen(fileName,"wb");
	ChangePageOfColorTable(page);
	WriteColorTable(f);
	ChangePageOfFontTable(page);
	WriteFontTable(f);
	ChangePageOfBlockList(page);
	SaveBlockList(f);
	fclose(f);
}

void LoadFile(const char* fileName)
{
	LoadFileAtPage(curPage, fileName);
}

void SaveFile(const char* fileName)
{
	SaveFileAtPage(curPage, fileName);
}
