#include "blockList.h"

#define BLOCKLIST_FIRST_SHARE_SPACE 100
static block* blocklist[255];
static int blocklistLength[255];
static int blocklistSpace[255];
static int curPage;

static void ensureListSpace()
{
	if(!curPage) return;
	if(blocklistSpace[curPage] == 0)
	{
		blocklistSpace[curPage] = BLOCKLIST_FIRST_SHARE_SPACE;
		blocklist[curPage] = (block*)malloc(BLOCKLIST_FIRST_SHARE_SPACE*sizeof(block));
		return;
	}
	if(blocklistSpace[curPage] == blocklistLength[curPage])
	{
		blocklist[curPage] = (block*)realloc(blocklist[curPage], 2*blocklistSpace[curPage]*sizeof(block));
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
	int alignType;
	int alignBlockID;
	int culumn
	double alignArgument;
} blockExchangeStruct;

blockExcnageStruct blocksBuffer[65535];

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
			blocklist[curPage][i].ID                  = i;
			blocklist[curPage][i].type                = blocksBuffer[i-1].type;
			blocklist[curPage][i].align.column        = blocksBuffer[i-1].column;
			blocklist[curPage][i].align.alignType     = blocksBuffer[i-1].alignType;
			blocklist[curPage][i].align.alignBlockID  = blocksBuffer[i-1].alignBlockID;
			blocklist[curPage][i].align.alignArgument = blocksBuffer[i-1].alignArgument;
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
		blocksBuffer[i-1].type          = blocklist[curPage][i].type;
		blocksBuffer[i-1].column        = blocklist[curPage][i].align.column;
		blocksBuffer[i-1].alignType     = blocklist[curPage][i].align.alignType;
		blocksBuffer[i-1].alignBlockID  = blocklist[curPage][i].align.alignBlockID;
		blocksBuffer[i-1].alignArgument = blocklist[curPage][i].align.alignArgument;
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

void BlockMove(int ID, int newColumn, int newAlignType, int newAlignID, double newAlignArgument)
{
	if(!curPage) return;
	assert(newAlignID < ID);
	blocklist[curPage][ID].align.column        = newColumn;
	blocklist[curPage][ID].align.alignType     = newAlignType;
	blocklist[curPage][ID].align.alignBlockID  = newAlignID;
	blocklist[curPage][ID].align.alignArgument = newAlignArgument;
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
