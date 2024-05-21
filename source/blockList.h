#ifndef __BLOCK_LIST_H_INCLUDED__
#define __BLOCK_LIST_H_INCLUDED__

typedef struct {
	int ID;
	int type;
	int columID;
	int alignBlockID;
	double alignDistance;
	void* dataptr;
} Block;

typedef void* (*FileReader)(FILE*);
void RegisterReadMethod(int type, FileReader func);
void RegisterWriteMethod(int type, FileReader func);

Block* BlockCreate(int type, void* dataptr);
void   BlockMove(int ID, int newAlignID, double newAlignDistance);

void ChangePageOfBlockList(int p);
void LoadBlockList(FILE* f);
void SaveBlockList(FILE* f);

typedef void  (BlockListTraverseFunc)(Block*);
typedef void* (BlockListAccumlateFunc)(void*, Block*);
void  TraverseBlockList(BlockListTraverseFunc func);
void* AccumlateBlockList(BlockListAccumlateFunc func, void* beginValue);

#endif
