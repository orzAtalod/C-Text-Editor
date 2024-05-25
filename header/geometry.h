#ifndef __GEOMETRY_H_INCLUDED__
#define __GEOMETRY_H_INCLUDED__

/*
*
* 维护块结构的几何定位，采用的是文件坐标系，以第零列第一行的块的左上角为(0,0) 
*
*/

#include "blocklist.h"

void GeometryBuildAll();
void GeometryRefreshAll();

double GeometryGetColumnWidth(int col);
void GeometrySetColumnWidth(int col, double width);

Block* GeometryGetNextBlock(Block* x);
Block* GeometryGetPriorBlock(Block* x);
AlignmentInfo GeometryAppendBlock(Block* x, Block* a);
int GeometryMoveBlock(Block* x, AlignmentInfo moveTo); //return 0 when succeed and 1 when failed.

void GeometrySetWindowsX(double windowWidth);
double GeometryCalculateOverallX();
double GeometryCalculateOverallY();
double GeometryCalculateXPosition(Block* x);
double GeometryCalculateYPosition(Block* x);

#endif
