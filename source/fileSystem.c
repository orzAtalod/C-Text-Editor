#include "fileSystem.h"
#include <stdio.h>
#include "blockList.h"
#include "textStructure.h"
#include "fileSystemCore.h"
#include "explorerCore.h"
#include <string.h>

static int curPage;

void ChangePageAtFileSystem(int page)
{
	curPage = page;
}

void ClearAllItemsOnPage(int page)
{
	ChangePageOfColorTable(page);
	ClearColorTable();
	ChangePageOfFontTable(page);
	ClearFontTable();
	ChangePageOfBlockList(page);
	ClearBlockList();
}

static int curPage;
void StoreCurPage()
{
	curPage = GetPageOfBlockList();
}

void RecoverCurPage()
{
	ChangePageOfColorTable(curPage);
	ChangePageOfFontTable(curPage);
	ChangePageOfBlockList(curPage);
}

int LoadFileAtPage(int page, const char* fileName)
{
	ClearAllItemsOnPage(page);
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
	ChangePageOfFontTable(page);
	ChangePageOfBlockList(page);
	WriteColorTable(f);
	WriteFontTable(f);
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

char savFilePathstore[255];
void RegisterSavFilePath(const char* savFilePath)
{
	strcpy(savFilePathstore, savFilePath);
}

void LoadSavFile()
{
	FILE* f = fopen(savFilePathstore, "rb");
	if(f)
	{
		ReadSavFile(f);
		fclose(f);
	}
}

void StoreSavFile()
{
	FILE* f = fopen(savFilePathstore, "wb");
	if(f)
	{
		WriteSavFile(f);
		fclose(f);
	}
}
