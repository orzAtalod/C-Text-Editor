#include "explorerCore.h"
#include "extgraph.h"
#include <math.h>
/*
	DefineColor("Black", 0, 0, 0);
	DefineColor("Dark Gray", .35, .35, .35);
	DefineColor("Gray", .6, .6, .6);
	DefineColor("Light Gray", .75, .75, .75);
	DefineColor("White", 1, 1, 1);
	DefineColor("Brown", .35, .20, .05);
	DefineColor("Red", 1, 0, 0);
	DefineColor("Orange", 1, .40, .1);
	DefineColor("Yellow", 1, 1, 0);
	DefineColor("Green", 0, 1, 0);
	DefineColor("Blue", 0, 0, 1);
	DefineColor("Violet", .93, .5, .93);
	DefineColor("Magenta", 1, 0, 1);
	DefineColor("Cyan", 0, 1, 1);
*/
const char* colorSheet[13] = {"Red","Blue","Yellow","Light Gray","Green","Brown","Gray","Violet","Orange","Black","Cyan","Magenta","Dark Gray"};

static int tagNum;
static const char** tags;
static int tagVal[65535];
static int sumTagV;

static void stateTraverse(FileHeaderInfo* f, int p)
{
	for(int i=1; i<=f->tagNum; ++i)
	{
		tagVal[f->tags[i]] += f->editTime;
	}
}

void StatBegin()
{
	tagNum = GetTagNum();
	tags = GetTags();
	BrowseExplorer(stateTraverse, 1);
	for(int i=1; i<=tagNum; ++i)
	{
		sumTagV += tagVal[i];
	}
}

#define STATISTIC_R       1
#define STATISTIC_DENSITY 0.8
#define PI                3.1415926535897932384

void StatDraw(double cx,double cy,double dx,double dy)
{
	SaveGraphicsState();
	const double cenx = (cx+dx)/2;
	const double ceny = (cy+dy)/2;

	double curDegree = 0;
	for(int i=1; i<=tagNum; ++i)
	{
		MovePen(cenx, ceny); //Eliminating cumulative error
		SetPenColor(colorSheet[i-1]);
		StartFilledRegion(STATISTIC_DENSITY);
		DrawLine(STATISTIC_R*cos(curDegree), STATISTIC_R*sin(curDegree));
		DrawArc(STATISTIC_R, curDegree/2/PI*360, (1.*tagVal[i])/sumTagV*360);
		curDegree += 2*PI*(1.*tagVal[i])/sumTagV;
		DrawLine(-STATISTIC_R*cos(curDegree), -STATISTIC_R*sin(curDegree));
		EndFilledRegion();
	}
	
	const double tagx = cenx + (dx-cenx)/3*2;
	const double tagy = dy + (cy-dy)/3;
	for(int i=1; i<=tagNum; ++i)
	{
		MovePen(tagx, tagy-GetFontHeight()*(i-1));
		SetPenColor(colorSheet[i-1]);
		StartFilledRegion(STATISTIC_DENSITY);
		DrawLine(0.1,0);
		DrawLine(0,0.1);
		DrawLine(-0.1,0);
		DrawLine(0,-0.1);
		EndFilledRegion();
		SetPenColor("Black");
		MovePen(tagx+0.2, tagy-GetFontHeight()*(i-1));
		DrawTextString(tags[i]);
	}
	RestoreGraphicsState();
}
