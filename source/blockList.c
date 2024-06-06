#include "blockList.h"
#include <stdlib.h>
#include <assert.h>

#define BLOCKLIST_FIRST_SHARE_SPACE 100
static Block* blocklist[255];
static int* blockDeleted[255];
static int* blockDeleteNum[255];
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
		blockDeleted[curPage] = (int*)malloc(BLOCKLIST_FIRST_SHARE_SPACE*sizeof(int));
		blockDeleteNum[curPage] = (int*)malloc(BLOCKLIST_FIRST_SHARE_SPACE*sizeof(int));
		return;
	}
	if(blocklistSpace[curPage] == blocklistLength[curPage])
	{
		blocklist[curPage] = (Block*)realloc(blocklist[curPage], 2*blocklistSpace[curPage]*sizeof(Block));
		blockDeleted[curPage] = (int*)ralloc(blockDeleted, 2*blocklistSpace[curPage]*sizeof(int));
		blockDeleteNum[curPage] = (int*)ralloc(blockDeleteNum, 2*blocklistSpace[curPage]*sizeof(int));
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

static double columnsW[255];  static int colNum;

void SetColumnInfo(int columnNum, double* columns)
{
	colNum = columnNum;
	memcpy(columnsW, columns, (colNum+1)*sizeof(double));
}

int GetColumnNum(void)
{
	return colNum;
}

double GetColumnWidth(int cID)
{
	return columnsW[cID];
}

void LoadBlockList(FILE* f)
{
	fread(&colNum, sizeof(int), 1, f);
	fread(columnsW, sizeof(double), colNum+1, f);

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
			++blocklistLength[curPage];
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
	fwrite(&colNum, sizeof(int), 1, f);
	fwrite(columnsW, sizeof(double), colNum+1, f);
	const int blocknum = blocklistLength[curPage];
	for(int i=1; i<=blocknum; ++i)
	{
		if(blockDeleted[i]) continue;
		const int abid = blocklist[curPage][i].align.alignBlockID;
		blocksBuffer[i-1-blockDeleteNum[i]].type  = blocklist[curPage][i].type;
		blocksBuffer[i-1-blockDeleteNum[i]].align = blocklist[curPage][i].align;
		blocksBuffer[i-1-blockDeleteNum[i]].align.alignBlockID -= blockDeleteNum[abid];
	}
	const int realBlkNum = blocknum - blockDeleteNum[blocknum];
	fwrite(&realBlkNum, sizeof(int), 1, f);
	fwrite(blocksBuffer, sizeof(blockExchangeStruct), realBlkNum, f);
	for(int i=1; i<=blocknum; ++i)
	{
		if(!blockDeleted[i])
		{
			(writers[blocklist[curPage][i].type])(blocklist[curPage][i].dataptr, f);
		}
	}
}

void ChangePageOfBlockList(int p)
{
	curPage = p;
}

int GetPageOfBlockList()
{
	return curPage;
}

Block* BlockCreate(int type, void* dataptr)
{
	if(!curPage) return 0;
	ensureListSpace();
	const int curid = ++blocklistLength[curPage];
	blocklist[curPage][curid].ID      = curid;
	blocklist[curPage][curid].type    = type;
	blocklist[curPage][curid].dataptr = dataptr;
	blockDeleteNum[curPage][curid] = blockDeleteNum[curPage][curid-1];
	blockDeleted[curPage][curid] = 0;
	return blocklist[curPage] + curid;
}

void BlockDelete(Block* blk)
{
	blockDeleted[blk->ID] = 1;
	const int blockNum = blocklistLength[curPage];
	for(int i=blk->ID; i<=blockNum; ++i)
	{
		++blockDeleteNum[i];
	}
}

void ClearBlockList()
{
	if(blocklist[curPage]) free(blocklist[curPage]);
	if(blockDeleted[curPage]) free(blockDeleted[curPage]);
	if(blockDeleteNum[curPage]) free(blockDeleteNum[curPage]);
	blocklist[curPage] = 0;
	blocklistLength[curPage] = blocklistSpace[curPage] = 0;
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

Block* GetBlock(int blockID)
{
	return blocklist[curPage]+blockID;
}

static GetHeightFunc heightFunc[255];

void RegisterGetHeightFunc(int type, GetHeightFunc func)
{
	assert(type>=0 && type<=255);
	heightFunc[type] = func;
}

double GetHeight(Block* blk, double width)
{
	if(blockDeleted[blk->ID]) return 0;
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
	if(blockDeleted[b->ID]) return;
	drawfunc[b->type](b->dataptr, cx, cy, width, begH, endH);
	SetPenColor("Black");
	SetPenSize(3);
}

GetPositionFunc   posFunci[255];
GetRelativeXYFunc relFuncX[255];
GetRelativeXYFunc relFuncY[255];

void RegisterGetPositionFunc(int type, GetPositionFunc func)
{
	posFunci[type] = func;
}

void RegisterGetRelativeXYFunc(int type, GetRelativeXYFunc fx, GetRelativeXYFunc fy)
{
	relFuncX[type] = fx;
	relFuncY[type] = fy;
}

int GetPositionFromRelativeXY(Block* b, double width, double rx, double ry)
{
	if(blockDeleted[b->ID]) return 0;
	return posFunci[b->type](b->dataptr, width, rx, ry);
}

double GetRelativeXFromPosition(Block* b, double width, int position)
{
	if(blockDeleted[b->ID]) return 0;
	return relFuncX[b->type](b->dataptr, width, position);
}

double GetRelativeYFromPosition(Block* b, double width, int position)
{
	if(blockDeleted[b->ID]) return 0;
	return relFuncY[b->type](b->dataptr, width, position);
}