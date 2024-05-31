#ifndef __BLOCK_LIST_H_INCLUDED__
#define __BLOCK_LIST_H_INCLUDED__
#include <stdio.h>

typedef double (*GetHeightFunc)(void*,double);
typedef void (*DrawFunc)(void*,double,double,double,double,double); 

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
//这两个函数在FileSystem初始化的时候完成注册 

void ChangePageOfBlockList(int p);
int GetPageOfBlockList();
Block* BlockCreate(int type, void* dataptr);
void   BlockMove(int ID, AlignmentInfo align);

void LoadBlockList(FILE* f);
void SaveBlockList(FILE* f);

typedef void  (BlockListTraverseFunc)(Block*);
typedef void* (BlockListAccumlateFunc)(void*, Block*);
void  TraverseBlockList(BlockListTraverseFunc func);
void* AccumlateBlockList(BlockListAccumlateFunc func, void* beginValue);

void RegisterGetHeightFunc(int type, GetHeightFunc func);
double GetHeight(Block*, double);
void RegisterDrawFunc(int type, DrawFunc func);
void DrawBlock(Block*, double cx, double cy, double width, double begH, double endH);
//这个函数在Editor初始化的时候完成注册 

#endif
