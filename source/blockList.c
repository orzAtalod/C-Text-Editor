#include "blockList.h"
#include <stdlib.h>
#include <assert.h>

#define BLOCKLIST_FIRST_SHARE_SPACE 100
static Block* blocklist[255];
static int blocklistLength[255];
static int blocklistSpace[255];
static int curPage;

static void ensureListSpace()
{
	if(!curPage) return;
	if(blocklistSpace[curPage] == 0)
	{
		blocklistSpace[curPage] = BLOCKLIST_FIRST_SHARE_SPACE;
		blocklist[curPage] = (Block*)malloc(BLOCKLIST_FIRST_SHARE_SPACE*sizeof(Block));
		return;
	}
	if(blocklistSpace[curPage] == blocklistLength[curPage])
	{
		blocklist[curPage] = (Block*)realloc(blocklist[curPage], 2*blocklistSpace[curPage]*sizeof(Block));
		blocklistSpace[curPage] *= 2;
		return;
	}
}

static FileReader readers[255];
static FileWriter writers[255];

void RegisterReaderMethod(int type, FileReader func)
{
	readers[type] = func;
}

void RegisterWriterMethod(int type, FileWriter func)
{
	writers[type] = func;
}

typedef struct {
	int type;
	AlignmentInfo align;
} blockExchangeStruct;

blockExchangeStruct blocksBuffer[65535];

void LoadBlockList(FILE* f)
{
	int blocknum;
	fread(&blocknum, sizeof(int), 1, f);
	fread(blocksBuffer, sizeof(blockExchangeStruct), blocknum, f);
	
	if(curPage)
	{
		for(int i=1; i<=blocknum; ++i)
		{
			ensureListSpace();
			blocklist[curPage][i].ID    = i;
			blocklist[curPage][i].type  = blocksBuffer[i-1].type;
			blocklist[curPage][i].align = blocksBuffer[i-1].align;
		}
	}
	
	for(int i=1; i<=blocknum; ++i)
	{
		void* tmpptr = (readers[blocklist[curPage][i].type])(f);
		if(curPage) blocklist[curPage][i].dataptr = tmpptr;
		else free(tmpptr);
	}
}

void SaveBlockList(FILE* f)
{
	assert(curPage);
	const int blocknum = blocklistLength[curPage];
	for(int i=1; i<=blocknum; ++i)
	{
		blocksBuffer[i-1].type  = blocklist[curPage][i].type;
		blocksBuffer[i-1].align = blocklist[curPage][i].align;
	}
	fwrite(&blocknum, sizeof(int), 1, f);
	fwrite(blocksBuffer, sizeof(blockExchangeStruct), blocknum, f);
	for(int i=1; i<=blocknum; ++i)
	{
		(writers[blocklist[curPage][i].type])(blocklist[curPage][i].dataptr, f);
	}
}

void ChangePageOfBlockList(int p)
{
	curPage = p;
}

Block* BlockCreate(int type, void* dataptr)
{
	if(!curPage) return 0;
	ensureListSpace();
	const int curid = ++blocklistLength[curPage];
	blocklist[curPage][curid].ID      = curid;
	blocklist[curPage][curid].type    = type;
	blocklist[curPage][curid].dataptr = dataptr;
	return blocklist[curPage] + curid;
}

void BlockMove(int ID, AlignmentInfo align)
{
	if(!curPage) return;
	assert(align.alignBlockID < ID);
	blocklist[curPage][ID].align = align;
}

void TraverseBlockList(BlockListTraverseFunc func)
{
	assert(curPage);
	const int blocknum = blocklistLength[curPage];
	for(int i=1; i<=blocknum; ++i)
	{
		func(blocklist[curPage]+i);
	}
}

void* AccumlateBlockList(BlockListAccumlateFunc func, void* beginValue)
{
	assert(curPage);
	const int blocknum = blocklistLength[curPage];
	for(int i=1; i<=blocknum; ++i)
	{
		beginValue = func(beginValue, blocklist[curPage]+i);
	}
	return beginValue;
}

static GetHeightFunc heightFunc[255];

void RegisterGetHeightFunc(int type, GetHeightFunc func)
{
	assert(type>=0 && type<=255);
	heightFunc[type] = func;
}

double GetHeight(Block* blk, double width)
{
	return heightFunc[blk->type](blk->dataptr, width);
}

static DrawFunc drawfunc[255];

void RegisterDrawFunc(int type, DrawFunc func)
{
	assert(type>=0 && type<=255);
	drawfunc[type] = func;
}

void DrawBlock(Block* b, double cx, double cy, double width, double begH, double endH)
{
	drawfunc[b->type](b->dataptr, cx, cy, width, begH, endH);
}
