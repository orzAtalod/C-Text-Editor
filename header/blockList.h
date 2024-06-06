#ifndef __BLOCK_LIST_H_INCLUDED__
#define __BLOCK_LIST_H_INCLUDED__
#include <stdio.h>

typedef struct {
	int column;
	int alignType; //0:head 1:middle 2:end
	double alignArgument;
	int alignBlockID;
} AlignmentInfo;

typedef struct blockBase {
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
void   BlockDelete(Block* blk);
void   ClearBlockList();

void LoadBlockList(FILE* f);
void SaveBlockList(FILE* f);

typedef void  (BlockListTraverseFunc)(Block*);
typedef void* (BlockListAccumlateFunc)(void*, Block*);
void  TraverseBlockList(BlockListTraverseFunc func);
void* AccumlateBlockList(BlockListAccumlateFunc func, void* beginValue);
Block* GetBlockFromIDInBlockList(int blockID);

//位置相关
typedef int (*GetPositionFunc)(void*,double,double,double);
typedef double (*GetRelativeXYFunc)(void*,double,int);
void RegisterGetPositionFunc(int type, GetPositionFunc func);
void RegisterGetRelativeXYFunc(int type, GetRelativeXYFunc fx, GetRelativeXYFunc fy);
int GetPositionFromRelativeXY(Block* b, double width, double rx, double ry);
double GetRelativeXFromPosition(Block* b, double width, int position);
double GetRelativeYFromPosition(Block* b, double width, int position);

//绘制相关
typedef double (*GetHeightFunc)(void*,double);
void RegisterGetHeightFunc(int type, GetHeightFunc func);
double GetHeight(Block*, double);
typedef void (*DrawFunc)(void*,double,double,double,double,double); 
void RegisterDrawFunc(int type, DrawFunc func);
void DrawBlockInBlockList(Block*, double cx, double cy, double width, double begH, double endH);
typedef double (*GetEleHeightFunc)(void*);
void RegisterGetEleHeightFunc(int type, GetEleHeightFunc func);
double GetElementHeight(Block*);

//列相关
void SetColumnInfo(int columnNum, double* columns); 
int GetColumnNum(void);
double GetColumnWidth(int);

#endif
