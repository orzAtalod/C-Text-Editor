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

static ColorDefinitionFunction colorFunc;

void InitFileSystem(ColorDefinitionFunction func)
{
	colorFunc = func;
	RegisterReaderMethod(1,styleStringReader);
	RegisterWriterMethod(1,styleStringWriter);
}

void UpdateColorDefinitionMethod(ColorDefinitionFunction func)
{
	colorFunc = func;
}

void LoadFileAtPage(int page, const char* fileName)
{
	FILE* f = fopen(fileName,"rb");
	ChangePageOfColorTable(page);
	ClearColorTable(f);
	ReadColorTable(f, colorFunc);
	ChangePageOfFontTable(page);
	ClearFontTable(f);
	ReadFontTable(f);
	ChangePageOfBlockList(page);
	ClearBlockList();
	LoadBlockList(f);
	fclose(f);
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
