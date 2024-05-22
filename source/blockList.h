#ifndef __BLOCK_LIST_H_INCLUDED__
#define __BLOCK_LIST_H_INCLUDED__

typedef struct {
	int column;
	int alignType; //0:head 1:middle 2:end
	int alignBlockID;
	double alignArgument;
} AlignmentInfo;

typedef struct {
	int ID;
	int type;
	void* dataptr;
	AlignmentInfo align;
} Block;

typedef void* (*FileReader)(FILE*);
typedef void  (*FileWriter)(void*, FILE*);
void RegisterReaderMethod(int type, FileReader func);
void RegisterWriterMethod(int type, FileWriter func);

void ChangePageOfBlockList(int p);
Block* BlockCreate(int type, void* dataptr);
void   BlockMove(int ID, int newColumn, int newAlignType, int newAlignID, double newAlignArgument);

void LoadBlockList(FILE* f);
void SaveBlockList(FILE* f);

typedef void  (BlockListTraverseFunc)(Block*);
typedef void* (BlockListAccumlateFunc)(void*, Block*);
void  TraverseBlockList(BlockListTraverseFunc func);
void* AccumlateBlockList(BlockListAccumlateFunc func, void* beginValue);

#endif
