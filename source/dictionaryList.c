#include "dictionaryList.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graphics.h"
#include "extgraph.h"
#include <assert.h>

////////////////////////////////  基础工具  //////////////////////////// 

typedef struct {
	int indent;
	int type;
	void* source;
	const char* name;
	const char* penColor;
	const char* prefix;
} drawCommand;

static char outputBuffer[255];

static const char* generateLimitedOutput(drawCommand dc, double width)
{
	for(int i=0; i<dc.indent; ++i)
	{
		outputBuffer[i] = ' ';
	}
	strcpy(outputBuffer+dc.indent, dc.prefix);
	const int offset = dc.indent + strlen(dc.prefix);

	if(!dc.name[0])
	{
		if(TextStringWidth(outputBuffer) > width) outputBuffer[0] = '\0';
		return outputBuffer;
	}

	outputBuffer[offset] = outputBuffer[offset+1] = '.';
	outputBuffer[offset+2] = '\0';
	if(TextStringWidth(outputBuffer) > width)
	{
		outputBuffer[0] = '\0';
		return outputBuffer;
	}

	int i = 0;
	for(; dc.name[i]&&TextStringWidth(outputBuffer)<=width; ++i)
	{
		outputBuffer[offset+i] = dc.name[i];
		outputBuffer[offset+i+2] = '.';
		outputBuffer[offset+i+3] = '\0';
	}
	if(TextStringWidth(outputBuffer) > width)
	{
		outputBuffer[offset+i+1] = '\0';
		outputBuffer[offset+i-1] = '.';
	}
	else
	{
		outputBuffer[offset+i] = '\0';
	}
	return outputBuffer;
}

static drawCommand dicLines[65535];
static int curline;

static void fillDrawCommandsItem(DictionaryGraphicDatas* dl, DictionaryItem* x, int curindent)
{
	if(!x) return;
	++curline;
	dicLines[curline].type = 2;
	dicLines[curline].indent = curindent;
	dicLines[curline].name = x->itemName;
	dicLines[curline].penColor = (dl->itemColors)[x->itemEmphasizeType];
	dicLines[curline].prefix = "| ";
	dicLines[curline].source = (void*)x;
	fillDrawCommandsItem(dl, x->nextItem, curindent);
}

static void fillDrawCommandsFolder(DictionaryGraphicDatas* dl, DictionaryFolder* x, int curindent)
{
	if(!x) return;
	++curline;
	dicLines[curline].type = 1;
	dicLines[curline].indent = curindent;
	dicLines[curline].name = x->folderName;
	dicLines[curline].penColor = (dl->folderColors)[x->folderEmphasizeType];
	dicLines[curline].prefix = x->folderExpended ? "+ " : "- ";
	dicLines[curline].source = (void*)x;
	if(x->folderExpended)
	{
		fillDrawCommandsItem(dl, x->items, curindent+dl->indent);
		fillDrawCommandsFolder(dl, x->subFolders, curindent+dl->indent);
	}
	fillDrawCommandsFolder(dl, x->nextFolder, curindent);
}

static void fillDrawCommands(DictionaryGraphicDatas* dl, DictionaryFolder* x)
{
	assert(dl);
	curline = 0;
	fillDrawCommandsFolder(dl,x,0);
}

//////////////////////////////////  绘制与定位逻辑  ////////////////////////

/*
* pointSize 为 -1（不更改）
* indent 为 4 
* entryFont 为 NULL（不更改）
* folderColors 为 {"Black","Red","Green","Blue"}
* folderBoarderColors 为 {NULL, NULL, NULL, NULL} （透明）
* folderFillColors 为 {NULL, NULL, NULL, NULL} （透明） 
* itemColors 为 {"Black","Red","Green","Blue"}
*/
static DictionaryGraphicDatas defaultDicgd;
static const char* defaultColors[4] = {"Black","Red","Green","Blue"};

static void checkInitialized()
{
	if(defaultDicgd.pointSize != -1)
	{
		defaultDicgd.pointSize = -1;
		defaultDicgd.indent = 4;
		//由于是分配在 static 字段的变量，defaultDicgd 的所有值已被初始化为 0 
		for(int i=0; i<4; ++i)
		{
			defaultDicgd.folderColors[i] = defaultDicgd.itemColors[i] = defaultColors[i];
		}
	}
}

static int pointSizeBuffer;
static char* fontBuffer;
static char* penColor;

static void bufferChangeGraphicState(DictionaryGraphicDatas* dl)
{
	if(!dl) return;
	penColor = GetPenColor();
	if(dl->pointSize != -1)
	{	
		pointSizeBuffer = GetPointSize();
		SetPointSize(dl->pointSize);
	}
	if(dl->entryFont)
	{
		fontBuffer = GetFont();
		SetFont(dl->entryFont);
	}
}

static void retGraphicState(DictionaryGraphicDatas* dl)
{
	if(!dl) return;
	SetPenColor(penColor);
	if(dl->pointSize != -1)
	{	
		SetPointSize(pointSizeBuffer);
	}
	if(dl->entryFont)
	{
		SetFont(fontBuffer);
	}
}

double GetDictionaryListHeight(DictionaryGraphicDatas* dl, DictionaryFolder* fl, double width)
{
	checkInitialized();
	if(!dl) dl = &defaultDicgd;
	bufferChangeGraphicState(dl);
	
	fillDrawCommands(dl, fl);
	const double result = (curline+1)*GetFontHeight();

	retGraphicState(dl);
	return result;
}

static inline int max(int a,int b) { return a>b ? a : b; }

void DrawDictionaryList(DictionaryGraphicDatas* dl, DictionaryFolder* fl, double cx, double cy, double width, double begH, double endH)
{
	checkInitialized();
	if(!dl) dl = &defaultDicgd;
	bufferChangeGraphicState(dl);
	const double cyLimL = cy - begH + endH;
	cy -= GetFontAscent();
	
	const double fHeight = GetFontHeight();
	fillDrawCommands(dl, fl);
	for(int i=max(-begH/fHeight+1,1); i<=curline; ++i)
	{
		if(cy<cyLimL) break;
		const char* outputStr = generateLimitedOutput(dicLines[i], width);
		MovePen(cx, cy);
		SetPenColor(dicLines[i].penColor);
		DrawTextString(outputStr);
		cy -= fHeight;
	}
	
	retGraphicState(dl);
}

DictionaryCursor PositionizeDictionaryList(DictionaryGraphicDatas* dl, DictionaryFolder* fl, double width, double mX, double mY)
{
	checkInitialized();
	if(!dl) dl = &defaultDicgd;
	bufferChangeGraphicState(dl);

	fillDrawCommands(dl,fl);
	const int resLine = -mY/GetFontHeight();
	if(resLine<=0 || resLine>curline)
	{
		DictionaryCursor res;
		res.pointEntryType = 0;
		res.folderIn = 0;
		res.pointFolder = 0;
		res.pointItem = 0;
		return res;
	}
	else
	{
		DictionaryCursor res;
		res.pointEntryType = dicLines[resLine].type;
		res.folderIn = 0;
		res.pointFolder = 0;
		res.pointItem = 0;
		if(res.pointEntryType == 1)
		{
			res.pointFolder = (DictionaryFolder*)dicLines[resLine].source;
			res.folderIn = res.pointFolder;
		}
		else
		{
			assert(res.pointEntryType == 2);
			res.pointItem = (DictionaryItem*)dicLines[resLine].source;
			res.folderIn = res.pointItem->folder;
		}
		return res;
	}
}

//////////////////////////   创建、拷贝、销毁函数  //////////////////////// 
#define NEW(x)  ((x*)malloc(sizeof(x)))
#define NEWVARR(x,len) ((x*)malloc((len)*sizeof(x)))
#define CNEW(x)  ((x*)calloc(1,sizeof(x)))

DictionaryGraphicDatas* CopyDictionaryGraphicDatas(DictionaryGraphicDatas* sorce)
{
	DictionaryGraphicDatas* dest = NEW(DictionaryGraphicDatas);
	memcpy(dest, sorce, sizeof(DictionaryGraphicDatas));
	return dest;
}

DictionaryGraphicDatas* CreateDictionaryGraphicDatas()
{
	checkInitialized();
	return CopyDictionaryGraphicDatas(&defaultDicgd);
}

void FreeDictionaryGraphicDatas(DictionaryGraphicDatas* dg)
{
	free(dg);
}

#define NAME_SIZE 255

/*
*
* 在创建的同时设置缺省值 
* itemEmphasizeType 为 0（不强调） 
* prevItem, nextItem 与 folder 均设置为 NULL
* itemName 设置为 "Noname"
* 
*/

DictionaryItem* CreateDictionaryItem()
{
	static int IDs = 0;
	DictionaryItem* ni = CNEW(DictionaryItem);
	ni->itemID   = ++IDs;
	ni->itemName = NEWVARR(char, NAME_SIZE);
	strcpy(ni->itemName, "Noname"); 
	return ni;
}

void FreeDictionaryItem(DictionaryItem* di)
{
	if(di->folder && di->folder->items == di)
	{
		di->folder->items = 0;
		if(di->prevItem) di->folder->items = di->prevItem;
		if(di->nextItem) di->folder->items = di->nextItem;
	}
	if(di->prevItem) di->prevItem->nextItem = di->nextItem;
	if(di->nextItem) di->nextItem->prevItem = di->prevItem;
	free(di->itemName);
	free(di);
}

DictionaryItem* CopyDictionaryItem(DictionaryItem* sorce)
{
	DictionaryItem* dest = CreateDictionaryItem();
	dest->itemEmphasizeType = sorce->itemEmphasizeType;
	dest->itemID = sorce->itemID;
	dest->folder = sorce->folder;
	strcpy(dest->itemName, sorce->itemName);
	return dest;
}

DictionaryFolder* CreateDictionaryFolder()
{
	static int IDs = 0;
	DictionaryFolder* nf = CNEW(DictionaryFolder);
	nf->folderID   = ++IDs;
	nf->folderName = NEWVARR(char, NAME_SIZE);
	strcpy(nf->folderName, "Noname");
	 
	return nf;
}

void FreeDictionaryFolder(DictionaryFolder* df)
{
	if(df->parent && df->parent->subFolders==df)
	{
		df->parent->subFolders = 0;
		if(df->prevFolder) df->parent->subFolders = df->prevFolder;
		if(df->nextFolder) df->parent->subFolders = df->nextFolder;
	}
	if(df->subFolders)
	{
		while(df->subFolders) FreeDictionaryFolder(df->subFolders);
	}
	if(df->items)
	{
		while(df->items) FreeDictionaryItem(df->items);
	}
	if(df->prevFolder) df->prevFolder->nextFolder = df->nextFolder;
	if(df->nextFolder) df->nextFolder->prevFolder = df->prevFolder;
	free(df->folderName);
	free(df);
}

DictionaryFolder* CopyDictionaryFolder(DictionaryFolder* sorce)
{
	DictionaryFolder* dest    = CreateDictionaryFolder();
	dest->folderID            = sorce->folderID;
	dest->folderExpended      = sorce->folderExpended;
	dest->folderEmphasizeType = sorce->folderEmphasizeType;
	dest->parent              = sorce->parent;
	strcpy(dest->folderName, sorce->folderName);
	
	if(sorce->items)
	{
		dest->items = CopyDictionaryItem(sorce->items);	

		DictionaryItem* dt = dest->items;
		for(DictionaryItem* it = sorce->items->nextItem; it; it=it->nextItem)
		{
			dt->nextItem = CopyDictionaryItem(it);
			dt->nextItem->prevItem = dt;
			dt = dt->nextItem;
		}
	
		dt = dest->items;
		for(DictionaryItem* it = sorce->items->prevItem; it; it=it->prevItem)
		{
			dt->prevItem = CopyDictionaryItem(it);
			dt->prevItem->nextItem = dt;
			dt = dt->prevItem;
		}
	}
	
	if(sorce->subFolders)
	{
		dest->subFolders = CopyDictionaryFolder(sorce->subFolders);
		
		DictionaryFolder* df = dest->subFolders;
		for(DictionaryFolder* it = sorce->subFolders->nextFolder; it; it=it->nextFolder)
		{
			df->nextFolder = CopyDictionaryFolder(it);
			df->nextFolder->prevFolder = df;
			df = df->nextFolder;
		}
		
		df = dest->subFolders;
		for(DictionaryFolder* it = sorce->subFolders->prevFolder; it; it=it->prevFolder)
		{
			df->prevFolder = CopyDictionaryFolder(it);
			df->prevFolder->nextFolder = df;
			df = df->nextFolder;
		}
	}
	
	return dest;
}
