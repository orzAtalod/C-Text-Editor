#include "blockList.h"
#include <stdlib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "extgraph.h"

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
		memset(blockDeleted[curPage],0,BLOCKLIST_FIRST_SHARE_SPACE*sizeof(int));
		memset(blockDeleteNum[curPage],0,BLOCKLIST_FIRST_SHARE_SPACE*sizeof(int)); 
		return;
	}
	if(blocklistSpace[curPage] == blocklistLength[curPage])
	{
		blocklist[curPage] = (Block*)realloc(blocklist[curPage], 2*blocklistSpace[curPage]*sizeof(Block));
		blockDeleted[curPage] = (int*)realloc(blockDeleted, 2*blocklistSpace[curPage]*sizeof(int));
		blockDeleteNum[curPage] = (int*)realloc(blockDeleteNum, 2*blocklistSpace[curPage]*sizeof(int));
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

static double* columnsW[255];  static int colNum[255];

void SetColumnInfo(int columnNum, double* columns)
{
	colNum[curPage] = columnNum;
	free(columnsW[curPage]);
	columnsW[curPage] = (double*)malloc((columnNum+5)*sizeof(double));
	memcpy(columnsW[curPage], columns, (colNum[curPage]+1)*sizeof(double));
}

int GetColumnNum(void)
{
	return colNum[curPage];
}

double GetColumnWidth(int cID)
{
	return columnsW[curPage][cID];
}

void readColumns(FILE* f)
{
	fread(colNum+curPage, sizeof(int), 1, f);
	free(columnsW[curPage]);
	columnsW[curPage] = (double*)malloc((colNum[curPage]+5)*sizeof(double));
	fread(columnsW[curPage]+1, sizeof(double), colNum[curPage], f);
	columnsW[curPage][0] = 0;
}

void writeColumns(FILE* f)
{
	fwrite(colNum+curPage, sizeof(int), 1, f);
	fwrite(columnsW[curPage]+1, sizeof(double), colNum[curPage], f);
}

static void clearCulumnInfo()
{
	if(columnsW[curPage]) free(columnsW[curPage]);
	columnsW[curPage] = (double*)malloc(2*sizeof(double));
	columnsW[curPage][0] = 0;
	columnsW[curPage][1] = 1;
	colNum[curPage] = 1;
}

void LoadBlockList(FILE* f)
{
	readColumns(f);

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
			blockDeleted[curPage][i] = 0;
			blockDeleteNum[curPage][i] = 0;
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
	if(!blocklistLength[curPage])
	{
		fwrite(blocklistLength+curPage,sizeof(int),1,f);
		return;
	} 
	assert(curPage);
	writeColumns(f);
	const int blocknum = blocklistLength[curPage];
	for(int i=1; i<=blocknum; ++i)
	{
		if(blockDeleted[curPage][i]) continue;
		const int abid = blocklist[curPage][i].align.alignBlockID;
		blocksBuffer[i-1-blockDeleteNum[curPage][i]].type  = blocklist[curPage][i].type;
		blocksBuffer[i-1-blockDeleteNum[curPage][i]].align = blocklist[curPage][i].align;
		blocksBuffer[i-1-blockDeleteNum[curPage][i]].align.alignBlockID -= blockDeleteNum[curPage][abid];
	}
	const int realBlkNum = blocknum - blockDeleteNum[curPage][blocknum];
	fwrite(&realBlkNum, sizeof(int), 1, f);
	fwrite(blocksBuffer, sizeof(blockExchangeStruct), realBlkNum, f);
	for(int i=1; i<=blocknum; ++i)
	{
		if(!blockDeleted[curPage][i])
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
	blockDeleted[curPage][blk->ID] = 1;
	const int blockNum = blocklistLength[curPage];
	for(int i=blk->ID; i<=blockNum; ++i)
	{
		++blockDeleteNum[curPage][i];
	}
}

void ClearBlockList()
{
	if(blocklist[curPage]) free(blocklist[curPage]);
	if(blockDeleted[curPage]) free(blockDeleted[curPage]);
	if(blockDeleteNum[curPage]) free(blockDeleteNum[curPage]);
	clearCulumnInfo();
	blocklist[curPage] = NULL;
	blockDeleted[curPage] = NULL;
	blockDeleteNum[curPage] = NULL;
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

Block* GetBlockFromIDInBlockList(int blockID)
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
	if(blockDeleted[curPage][blk->ID]) return 0;
	return heightFunc[blk->type](blk->dataptr, width);
}

static DrawFunc drawfunc[255];

void RegisterDrawFunc(int type, DrawFunc func)
{
	assert(type>=0 && type<=255);
	drawfunc[type] = func;
}

void DrawBlockInBlockList(Block* b, double cx, double cy, double width, double begH, double endH)
{
	if(blockDeleted[curPage][b->ID]) return;
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
	if(blockDeleted[curPage][b->ID]) return 0;
	return posFunci[b->type](b->dataptr, width, rx, ry);
}

double GetRelativeXFromPosition(Block* b, double width, int position)
{
	if(blockDeleted[curPage][b->ID]) return 0;
	return relFuncX[b->type](b->dataptr, width, position);
}

double GetRelativeYFromPosition(Block* b, double width, int position)
{
	if(blockDeleted[curPage][b->ID]) return 0;
	return relFuncY[b->type](b->dataptr, width, position);
}

GetEleHeightFunc gehf[255];

void RegisterGetEleHeightFunc(int type, GetEleHeightFunc func)
{
	gehf[type] = func;
}

double GetElementHeight(Block* b)
{
	return gehf[b->type](b->dataptr);
}

