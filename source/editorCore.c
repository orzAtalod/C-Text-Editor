#include "editorCore.h"
#include "blockList.h"
#include "textStructure.h"
#include "myGUI.h"
#include "fileSystemCore.h" 
#include "extgraph.h"
#include <string.h>
#include <stdlib.h>
#include "graphics.h"
#include <assert.h>

typedef struct blockChainBase {
	Block* curr;
	struct blockChainBase* next;
} blockChain;

static int blockChainPage = 0;
static blockChain* corrBlockChain[65535];   static int blockNum;
static double columnWidth[255];             static int columnNum;
static double columnWidthPosition[255];

/////////////////////////////////////// 基本操作 ///////////////////////////////////////////

blockChain* getPre(blockChain* x)
{
	if(!x->curr->align.alignBlockID) return NULL;
	if(corrBlockChain[x->curr->align.alignBlockID]->next != x) return NULL;
	return corrBlockChain[x->curr->align.alignBlockID];
}

/////////////////////////////////////// 建立与清空逻辑 //////////////////////////////////////

static double screenWidth;

static void setWidth(double w) { screenWidth = w; }

static void buildColumns()
{
	columnNum = GetColumnNum();
	for(int i=1; i<=columnNum; ++i)
	{
		columnWidth[i] = GetColumnWidth(i);
		columnWidthPosition[i] = columnWidthPosition[i-1] + columnWidth[i];
	}
	columnWidth[0] = 0;
}

static void buildBlock(Block* blk)
{
	if(corrBlockChain[blk->ID]) return;
	++blockNum;
	corrBlockChain[blk->ID] = (blockChain*)malloc(sizeof(blockChain));
	corrBlockChain[blk->ID]->curr = blk;
	if(blk->align.alignBlockID && GetBlockFromIDInBlockList(blk->align.alignBlockID)->align.column==blk->align.column)
	{
		buildBlock(GetBlockFromIDInBlockList(blk->align.alignBlockID));
		corrBlockChain[blk->align.alignBlockID]->next = corrBlockChain[blk->ID];
	}
}

static void buildBC()  //同时清空
{
//	TraverseColorDifinitions(DefineColor);
	buildColumns();

	corrBlockChain[0] = 0;
	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]) free(corrBlockChain[i]);
		corrBlockChain[i] = 0;
	}
	blockNum = 0;
	TraverseBlockList(buildBlock);
}

void ChangePageOfEditorCore(int p)
{
	ChangePageOfBlockList(p);
	ChangePageOfColorTable(p);
	ChangePageOfFontTable(p);
	blockChainPage = p;
	buildBC();
}

///////////////////////////////////////// 块高度计算 //////////////////////////////////////////////

static double blockHeight[65535];
static double blockBeginHeight[65535];
static int travelled[65535];
static double fullHeight;

static double calculateAlign(AlignmentInfo info)
{
	return blockBeginHeight[info.alignBlockID] + 
		info.alignType*blockHeight[info.alignBlockID]/2 + 
		info.alignArgument;
}

static void calculateBlockHeight(blockChain* bc)
{
	if(!bc) return;
	travelled[bc->curr->ID] = 1;
	const int preID = bc->curr->align.alignBlockID;
	if(!travelled[preID]) calculateBlockHeight(corrBlockChain[preID]);
	
	blockHeight[bc->curr->ID] = GetHeight(bc->curr, screenWidth*columnWidth[bc->curr->align.column]);
	blockBeginHeight[bc->curr->ID] = calculateAlign(bc->curr->align);
}

static double fullHeight;
static void getfullHeight()
{
	double fH = 0;
	for(int i=1; i<=blockNum; ++i)
	{
		if(blockBeginHeight[i]+blockHeight[i] < fullHeight)
		{
			fH = blockBeginHeight[i] + blockHeight[i];
		}
	}
	fullHeight = fH;
}

static void calculateHeight()
{
	memset(travelled, 0, (blockNum+5)*sizeof(int));
	travelled[0] = 1;
	for(int i=1; i<=blockNum; ++i)
	{
		calculateBlockHeight(corrBlockChain[i]);
	}
	getfullHeight();
}

//////////////////////////////////////// 屏幕位置维护 ////////////////////////////////////

static double screenHeight;
static double screenBeginHeight; // all negative
#define ROLLER_STEP 0.1
#define CULUMN_CURSOR_HEGIHT 0.1

//used as callback
static void changeScreenHeight(double newScreenHeight) { screenHeight = -(newScreenHeight-CULUMN_CURSOR_HEGIHT); }

static double getRollerBegin() { return screenBeginHeight>0 ? 0 : screenBeginHeight/fullHeight; }
static double getRollerEnd()   { return screenBeginHeight+screenHeight<=fullHeight ? 1 : (screenBeginHeight+screenHeight)/fullHeight; }

static void rollerRollUp()   { if(screenBeginHeight<0) screenBeginHeight+=ROLLER_STEP; }
static void rollerRollDown() { if(screenBeginHeight+screenHeight>fullHeight) screenBeginHeight-=ROLLER_STEP; }

static void rollerLeftDown(double mx, double my) { screenBeginHeight = my/screenHeight*fullHeight; }

//////////////////////////////////////// 光标操作 ///////////////////////////////////////////

typedef struct
{
	Block* blockVal;
	int position; //position是唯一指定块中光标位置的变量
} cursorMsg;

static int cursorType; //光标数量
static cursorMsg cursor1, cursor2; //顺序不限
#define CURSOR_DENSITY 0.2

static int priorierThan(cursorMsg cursor1, cursorMsg cursor2)
{
	if(cursor1.blockVal != cursor2.blockVal) return blockBeginHeight[cursor1.blockVal->ID] > blockBeginHeight[cursor2.blockVal->ID];
	return cursor1.position < cursor2.position;
}

static void checkPriorier() //保证cursor2更靠上
{
	if(priorierThan(cursor1, cursor2))
	{
		Block* ptr = cursor1.blockVal;
		cursor1.blockVal = cursor2.blockVal;
		cursor2.blockVal = ptr;
		int potr = cursor1.position;
		cursor1.position = cursor2.position;
		cursor2.position = potr;
	}	
}

static void printCursor(double wx, double wy, double width, double begH, double endH)
{
	if(!cursorType) return;
	const double cw = width*columnWidth[cursor1.blockVal->align.column];
	if(cursorType == 1) //single cursor
	{
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		double lH = GetElementHeight(cursor1.blockVal);
		double Ax = width*columnWidthPosition[cursor1.blockVal->align.column-1] + rx;
		double Ay = ry + blockBeginHeight[cursor1.blockVal->ID];
		if(Ay<=begH && Ay>=endH)
		{
			SetPenSize(3);
			MovePen(wx+Ax, wy+Ay-begH);
			DrawLine(0,lH);
			SetPenSize(1);
		}
	}
	else
	{
		checkPriorier();
		//TODO
		//1. 特判两个指针是否处在同一个块中
		//2. 绘制靠上的开始块
		//3. 绘制中间块
		//4. 绘制靠下的结束块
	}
}

cursorMsg getCursorPosition(double Ax, double Ay)
{
	int curcol = 1;
	while(curcol<columnNum && columnWidthPosition[curcol]*screenWidth < Ax) ++curcol;
	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]->curr->align.column != curcol) continue;
		if(blockBeginHeight[i]<Ax) continue;
		if(blockBeginHeight[i]+blockHeight[i]>=Ax && (!corrBlockChain[i]->next || blockBeginHeight[corrBlockChain[i]->next->curr->ID]>=Ax))
		{
			continue;
		}

		cursorMsg nc;
		nc.blockVal = corrBlockChain[i]->curr;
		nc.position = GetPositionFromRelativeXY(
			corrBlockChain[i]->curr,
			columnWidth[curcol]*screenWidth, 
			Ax-columnWidthPosition[curcol-1]*screenWidth,
			Ay-blockBeginHeight[i]);
	
		return nc;
	}

	cursorMsg nc;
	nc.blockVal = 0;
	return nc;
}

// TODO
// 单击仅需考虑一次 LeftDown 马上接 LeftUp

static void mergeCursorFrontward()
{
	if(priorierThan(cursor2, cursor1))
	{
		cursor1.blockVal = cursor2.blockVal;
		cursor1.position = cursor2.position;
	}
	cursor2.blockVal = 0;
	cursor2.position = 0;
	cursorType = 1;
}

static void mergeCursorBackward()
{
	if(!priorierThan(cursor2, cursor1))
	{
		cursor1.blockVal = cursor2.blockVal;
		cursor1.position = cursor2.position;
	}
	cursor2.blockVal = 0;
	cursor2.position = 0;
	cursorType = 1;
}

static void cursor1ToTail(blockChain* bc)
{
	if(!bc)
	{
		cursor1.position = 0;
	}
	else
	{
		cursor1.blockVal = bc->curr;
		cursor1.position = bc->curr->type==1 ? ((StyleString*)(bc->curr->dataptr))->contentLen : 1;
	}
}

static void cursor1ToHead(blockChain* bc)
{
	if(!bc)
	{
		cursor1.position = cursor1.blockVal->type==1 ? ((StyleString*)(cursor1.blockVal->dataptr))->contentLen : 1;
		
	}
	else
	{
		cursor1.blockVal = bc->curr;
		cursor1.position = 0;
	}
}

static void keyboardUp()
{
	if(cursorType == 0) return;
	if(cursorType == 2) mergeCursorFrontward();

	if(cursor1.blockVal->type==2) //imageStructure
	{
		cursor1ToTail(getPre(corrBlockChain[cursor1.blockVal->ID]));
	}
	else
	{
		const double cw = screenWidth*columnWidth[cursor1.blockVal->align.column];
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		if(ry>-1E-5 && ry<1E-5)
		{
			cursor1ToTail(getPre(corrBlockChain[cursor1.blockVal->ID]));
			return;
		}
		for(int px=0; px<=cursor1.position; ++px)
		{
			const double newrx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position-px);
			const double newry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position-px);
			if(newry<ry-1E-5 && newrx<rx+1E-5)
			{
				cursor1.position -= px;
				return;
			}
		}
		cursor1.position = 0;
	}
}

static void keyboardDown()
{
	if(cursorType == 0) return;
	if(cursorType == 2) mergeCursorBackward();

	if(cursor1.blockVal->type==2) //imageStructure
	{
		cursor1ToHead(corrBlockChain[cursor1.blockVal->ID]->next);
	}
	else
	{
		const double cw = screenWidth*columnWidth[cursor1.blockVal->align.column];
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		const int sLen = ((StyleString*)(cursor1.blockVal->dataptr))->contentLen;
		for(int px=0; px+cursor1.position<=sLen; ++px)
		{
			const double newrx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position-px);
			const double newry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position-px);
			if(newry>ry+1E-5 && newrx<rx+1E-5)
			{
				cursor1.position += px;
				return;
			}
		}
		cursor1ToHead(corrBlockChain[cursor1.blockVal->ID]->next);
	}
}

static void keyboardLeft()
{
	if(cursorType == 0) return;
	if(cursorType == 2) mergeCursorFrontward();

	if(cursor1.position)
	{
		--cursor1.position;
	}
	else
	{
		cursor1ToTail(getPre(corrBlockChain[cursor1.blockVal->ID]));
	}
}

static void keyboardRight()
{
	if(cursorType == 0) return;
	if(cursorType == 2) mergeCursorBackward();

	if(cursor1.position < (cursor1.blockVal->type==1 ? ((StyleString*)(cursor1.blockVal->dataptr))->contentLen : 1))
	{
		++cursor1.position;
	}
	else
	{
		cursor1ToHead(corrBlockChain[cursor1.blockVal->ID]->next);
	}
}

static int mouseUped = 1;
static double beginAx;
static double beginAy;
static double endAx;
static double endAy;

static void mouseLeftDownOnMain(double rx, double ry)
{
	const double Ay = ry + screenBeginHeight;
	if(mouseUped)
	{
		cursorType = 1;
		cursor1 = getCursorPosition(rx, Ay);
		beginAx = rx;
		beginAy = Ay;
		mouseUped = 0;
		if(!cursor1.blockVal)
		{
			cursorType = 0;
		}
	}
	else
	{
		if(!cursorType) return;
		if(!cursor2.blockVal) cursor2.blockVal = cursor1.blockVal;
		blockChain* pre = getPre(corrBlockChain[cursor2.blockVal->ID]);
		blockChain* nxt = corrBlockChain[cursor2.blockVal->ID]->next;
		if(pre && blockBeginHeight[pre->curr->ID]+blockHeight[pre->curr->ID] < Ay)
		{
			cursor2.blockVal = pre->curr;
		}
		if(nxt && blockBeginHeight[nxt->curr->ID] > Ay)
		{
			cursor2.blockVal = nxt->curr;
		}
		endAx = rx;
		endAy = Ay;
		cursorType = 2;
	}
}

static void mouseLeftUpOnMain()
{
	if(cursorType == 2)
	{
		const int curcol = cursor2.blockVal->align.column;
		cursor2.position = GetPositionFromRelativeXY(
			cursor2.blockVal, 
			columnWidth[curcol]*screenWidth, 
			endAx-columnWidthPosition[curcol-1]*screenWidth, 
			endAy-blockBeginHeight[cursor2.blockVal->ID]);
	}
	mouseUped = 1;
}

//////////////////////////////////////////////// 块移动处理 /////////////////////////////

// mouseRightDown
// mouseRightUp
// 单击仅需考虑一次 RightDown 马上接 RightUp

#define NEWLINE_MARGIN -0.3
#define INDENT 4
static int defaultPointSize = 15;
static const char* defaultColor = "Black"; // 这两个字符串保存在静态段，不会被修改
static const char* defaultFont = "Times";

StyleString* newStyleString()
{
	StyleString* newss = malloc(sizeof(StyleString));
	newss->content = (StyleChar*)malloc(100*sizeof(StyleChar));
	newss->contentLen = 0;
	newss->contentSpace = 100;
	newss->indent = INDENT;
	newss->pointSize = defaultPointSize;
	newss->color = defaultColor;
	newss->font = defaultFont;
	return newss;	
}

static blockChain* createBlock()
{
	++blockNum;

	Block* newBlock = BlockCreate(1, (void*)newStyleString());
	corrBlockChain[newBlock->ID] = (blockChain*)malloc(sizeof(blockChain));
	corrBlockChain[newBlock->ID]->curr = newBlock;
	corrBlockChain[newBlock->ID]->next = 0;
	return corrBlockChain[newBlock->ID];
}

static void createBlockAfterChain(blockChain* pre)
{
	assert(pre);
	blockChain* newBlockChain = createBlock();

	AlignmentInfo ali;
	ali.alignBlockID = pre->curr->ID;
	ali.alignType = 2;
	ali.alignArgument = NEWLINE_MARGIN;
	ali.column = pre->curr->align.column;
	BlockMove(newBlockChain->curr->ID, ali);

	if(pre->next) pre->next->curr->align.alignBlockID = newBlockChain->curr->ID;
	pre->next = newBlockChain;
}

static blockChain* getTail(blockChain* x)
{
	while(x->next) x = x->next;
	return x;
}

static void moveBlock(blockChain* x, AlignmentInfo newAlign)
{
	if(x->curr->align.alignBlockID && corrBlockChain[x->curr->align.alignBlockID]->next==x)
	{
		corrBlockChain[x->curr->align.alignBlockID]->next = NULL;
	}

	if(x->curr->align.column != newAlign.column)
	{
		for(blockChain* p=x->next; p; p=p->next)
		{
			p->curr->align.column = newAlign.column;
		}
	}
	
	if(newAlign.alignBlockID && newAlign.column==GetBlockFromIDInBlockList(newAlign.alignBlockID)->align.column)
	{
		newAlign.alignType = 2;
		newAlign.alignArgument = NEWLINE_MARGIN;
		blockChain* t  = getTail(x);
		blockChain* pn = corrBlockChain[newAlign.alignBlockID]->next;
		corrBlockChain[newAlign.alignBlockID]->next = x;
		if(pn) pn->curr->align.alignBlockID = t->curr->ID;
		t->next = pn;
	}

	BlockMove(x->curr->ID, newAlign);
}

static void deleteBlockChain(blockChain* x)
{
	blockChain* pre = getPre(x);
	if(pre)
	{
		pre->next = x->next;
		if(x->next) x->next->curr->align.alignBlockID = pre->curr->ID;
	}

	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]->curr->align.alignBlockID == x->curr->ID)
		{
			corrBlockChain[i]->curr->align.alignBlockID = x->curr->align.alignBlockID;
			corrBlockChain[i]->curr->align.alignArgument += x->curr->align.alignArgument;
			corrBlockChain[i]->curr->align.alignType = x->curr->align.alignType;
		}
	}

	x->curr->align.alignBlockID = 0;
	x->curr->align.alignArgument = 99999;
	BlockDelete(x->curr);
}

static void mouseRightDownOnMain(double rx, double ry)
{
	// TODO
}

static void mouseRightUpOnMain()
{
	// TODO
}

//////////////////////////////////////////////// 列逻辑 ////////////////////////////////

static void swapColumn(int col1, int col2)
{
	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]->curr->align.column == col1)
		{
			corrBlockChain[i]->curr->align.column = col2;
		}
		else if(corrBlockChain[i]->curr->align.column == col2)
		{
			corrBlockChain[i]->curr->align.column = col1;
		}
	}
}

static void createColumn(double colPos)
{
	int i = columnNum;
	while(columnWidthPosition[i] > colPos)
	{
		columnWidth[i+1] = columnWidth[i];
		columnWidthPosition[i+1] = columnWidthPosition[i];
		swapColumn(i, i+1);
		--i;
	}
	++columnNum; ++i;
	columnWidthPosition[columnNum] = 1;
	columnWidthPosition[i] = colPos;
	columnWidth[i] = colPos - columnWidthPosition[i-1];
	columnWidth[i+1] = columnWidthPosition[i+1] - colPos;
	SetColumnInfo(columnNum, columnWidth);
}

static void deleteColumn(int colID)
{
	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]->curr->align.column == colID)
		{
			blockChain* p = corrBlockChain[i];
			blockChain* pre = getPre(p);
			while(pre)
			{
				p = pre;
				pre = getPre(p);
			}
			AlignmentInfo newAlign = p->curr->align;
			newAlign.column = colID-1;
			moveBlock(p, newAlign);
		}
	}

	columnWidth[colID+1] += columnWidth[colID];
	for(int i=colID; i<columnNum; ++i)
	{
		columnWidthPosition[i] = columnWidthPosition[i+1];
		columnWidth[i] = columnWidth[i+1];
	}
	--columnNum;
	SetColumnInfo(columnNum, columnWidth);
}

#define COLUMN_CURSOR_CLICK_ERROR 0.01
static void leftclickOnRowRuler(double rx, double ry)
{
	const double curClickWidth = rx/screenWidth;
	for(int i=1; i<columnNum; ++i)
	{
		if(columnWidthPosition[i]-COLUMN_CURSOR_CLICK_ERROR<curClickWidth && columnWidthPosition[i]+COLUMN_CURSOR_CLICK_ERROR>curClickWidth)
		{
			swapColumn(i, i+1);
			return;
		}
	}
	createColumn(curClickWidth);
}

static void rightClickOnRowRuler(double rx, double ry)
{
	const double curClickWidth = rx/screenWidth;
	for(int i=2; i<=columnNum; ++i)
	{
		if(columnWidthPosition[i]-COLUMN_CURSOR_CLICK_ERROR<curClickWidth && columnWidthPosition[i]+COLUMN_CURSOR_CLICK_ERROR>curClickWidth)
		{
			deleteColumn(i);
			return;
		}
	}
}

//////////////////////////////////////////////// 编辑 //////////////////////////////////

static int defaultStyle;
typedef void (*blockOperatorFunc)(Block*);

static void mergeTextStr(blockChain* pre, blockChain* nxt)
{
	if(!pre || !nxt || pre->curr->type!=1 || nxt->curr->type!=1) return;
	StyleString* press = (StyleString*)pre->curr->dataptr;
	StyleString* nowss = (StyleString*)nxt->curr->dataptr;
	if(press->contentLen+nowss->contentLen >= press->contentSpace)
	{
		press->contentSpace = 2 * (press->contentLen+nowss->contentLen);
		press->content = (StyleChar*)realloc(press->content, 2*(press->contentLen+nowss->contentLen)*sizeof(StyleChar));
	}
	memcpy(press->content+press->contentLen, nowss->content, sizeof(StyleChar)*nowss->contentLen);
	deleteBlockChain(nxt);
}

static void deleteTroughCursor() //顺便还原成 cursorType=1 的状态
{
	if(cursorType!=2) return;
	checkPriorier();
	if(cursor1.blockVal == cursor2.blockVal)
	{
		if(cursor1.blockVal->type == 2)
		{
			if(cursor1.position!=cursor2.position)
			{
				FreeImageStructure(cursor1.blockVal->dataptr);
				cursor1.blockVal->type = 1;
				cursor1.blockVal->dataptr = (void*)newStyleString();
			} //else do nothing
		}
		else
		{
			const int delNum = cursor1.position - cursor2.position;
			StyleString* nowss = (StyleString*)cursor1.blockVal->dataptr;
			for(int i=cursor2.position; i+delNum<=nowss->contentLen; ++i)
			{
				nowss->content[i] = nowss->content[i+delNum];
			}
			nowss->contentLen -= delNum;
			cursor1.position -= delNum;
		}
	}
	else
	{
		blockChain* beg = corrBlockChain[cursor2.blockVal->ID];
		blockChain* now = corrBlockChain[cursor2.blockVal->ID];
		blockChain* nxt = now->next;
		int begDeleted = 0;
		if(cursor2.blockVal->type == 2)
		{
			if(cursor2.position == 0)
			{
				deleteBlockChain(now);
				begDeleted = 1;
			}
		}
		else
		{
			if(cursor2.position == 0)
			{
				deleteBlockChain(now);
				begDeleted = 1;
			}
			else
			{
				((StyleString*)(cursor2.blockVal->dataptr))->contentLen = cursor2.position;
			}
		}

		now = nxt;
		nxt = now->next;
		while(now->curr != cursor1.blockVal)
		{
			deleteBlockChain(now);
			now = nxt;
			nxt = now->next;
		}

		if(cursor1.blockVal->type == 2)
		{
			if(cursor1.position == 1)
			{
				if(begDeleted)
				{
					FreeImageStructure(cursor1.blockVal->dataptr);
					cursor1.blockVal->type = 1;
					cursor1.blockVal->dataptr = (void*)newStyleString();
					cursor1.position = 0;
				}
				else
				{
					deleteBlockChain(now);
					cursor1.blockVal = cursor2.blockVal;
					cursor1.position = cursor2.position;
				}
			}
			else
			{
				if(begDeleted)
				{
					StyleString* nowss = (StyleString*)cursor1.blockVal->dataptr;
					for(int i=cursor1.position; i<=nowss->contentLen; ++i)
					{
						nowss->content[i-cursor1.position] = nowss->content[i];
					}
					nowss->contentLen -= cursor1.position;
					cursor1.position = 0;
				}
				else
				{
					mergeTextStr(beg, now);
				}
			}
		}
	}
	cursorType = 1;
	cursor2.blockVal = 0;
	cursor2.position = 0;
}

static void applyBlocksTroughCursor(blockOperatorFunc bop) //还原成 cursorType=1 的状态
{
	if(cursorType!=2) return;
	checkPriorier();
	if(cursor1.blockVal == cursor2.blockVal)
	{
		bop(cursor1.blockVal);
	}
	else
	{
		bop(cursor2.blockVal);
		blockChain* now = corrBlockChain[cursor2.blockVal->ID]->next;
		while(now->curr != cursor1.blockVal)
		{
			bop(now->curr);
			now = now->next;
		}
		bop(cursor1.blockVal);
	}
	cursorType = 1;
	cursor2.blockVal = 0;
	cursor2.position = 0;
}

static void changeStyleTroughCursor(int newStyle)
{
	if(cursorType!=2) return;
	checkPriorier();
	if(cursor1.blockVal == cursor2.blockVal)
	{
		if(cursor1.blockVal->type == 1)
		{
			StyleString* nowss = (StyleString*)(cursor1.blockVal->dataptr);
			for(int i=cursor2.position; i<cursor1.position; ++i)
			{
				nowss->content[i].style ^= newStyle;
			}
		}
	}
	else
	{
		StyleString* nowss = (StyleString*)(cursor2.blockVal->dataptr);
		for(int i=cursor2.position; i<nowss->contentLen; ++i)
		{
			nowss->content[i].style ^= newStyle;
		}

		blockChain* now = corrBlockChain[cursor2.blockVal->ID]->next;
		while(now->curr != cursor1.blockVal)
		{
			nowss = (StyleString*)(now->curr->dataptr);
			for(int i=0; i<nowss->contentLen; ++i)
			{
				nowss->content[i].style ^= newStyle;
			}
			now = now->next;
		}

		nowss = cursor1.blockVal->dataptr;
		for(int i=0; i<cursor1.position; ++i)
		{
			nowss->content[i].style ^= newStyle;
		}
	}
	cursorType = 1;
	cursor2.blockVal = 0;
	cursor2.position = 0;
}

static void bold()
{
	if(cursorType==2)
	{
		changeStyleTroughCursor(1);
	}
	else
	{
		defaultStyle ^= 1;
	}
}

static void italic()
{
	if(cursorType==2)
	{
		changeStyleTroughCursor(2);
	}
	else
	{
		defaultStyle ^= 2;
	}
}

// color change 和 font change 需要 lookup 两次获得 fileSystemCore 中的稳定指针
static void fontChangeForSingleBlock(Block* blk)
{
	if(blk->type == 1)
	{
		((StyleString*)(blk->dataptr))->font = defaultFont;
	}
}

static void colorChangeForSingleBlock(Block* blk)
{
	if(blk->type == 1)
	{
		((StyleString*)(blk->dataptr))->color = defaultColor;
	}
}

static void pointsizeChangeForSingleBlock(Block* blk)
{
	if(blk->type == 1)
	{
		((StyleString*)(blk->dataptr))->pointSize = defaultPointSize;
	}
}

static void fontChange(const char* newFont)
{
	if(!newFont) return;
	int fontID = LookupFontNameInFontTable(newFont);
	if(!fontID) fontID = RegisterFontTable(newFont);
	const char* nowDefault = defaultFont;
	defaultFont = LookupIDInFontTable(fontID);
	if(cursorType == 2)
	{
		applyBlocksTroughCursor(fontChangeForSingleBlock);
		defaultFont = nowDefault;
	}
}

static void colorChange(const char* newColor)
{
	if(!newColor) return;
	int colorID = LookupColorNameInColorTable(newColor);
	if(!colorID) colorID = RegisterColorTable(newColor);
	const char const* nowDefault = defaultColor;
	defaultColor = LookupIDInColorTable(colorID);
	if(cursorType == 2)
	{
		applyBlocksTroughCursor(colorChangeForSingleBlock);
		defaultColor = nowDefault;
	}
}

static void pointSizeChange(const char* newPS)
{
	if(!newPS) return;
	int tmp = 0;
	sscanf(newPS,"%d",&tmp);
	if(tmp>0)
	{
		const int nowDefault = defaultPointSize;
		defaultPointSize = tmp;
		if(cursorType==2)
		{
			applyBlocksTroughCursor(pointsizeChangeForSingleBlock);
			defaultPointSize = nowDefault;
		}
	}
}

static void keyboardInput(char ch)
{
	if(cursorType == 2) deleteTroughCursor();
	if(!cursor1.blockVal || cursor1.blockVal->type!=1) return;
	StyleString* sstr = (StyleString*)cursor1.blockVal->dataptr;
	if(sstr->contentSpace == sstr->contentLen+1)
	{
		sstr->content = (StyleChar*)realloc(sstr->content, 2*sstr->contentSpace*sizeof(StyleChar));
		sstr->contentSpace *= 2;
	} 
	for(int i=sstr->contentLen; i>=cursor1.position; --i)
	{
		sstr->content[i+1] = sstr->content[i];
	}
	++sstr->contentLen;
	sstr->content[cursor1.position].content = ch;
	sstr->content[cursor1.position].style = defaultStyle;
	++cursor1.position;
}

static int getColumnFromX(double x)
{
	for(int i=1; i<=columnNum; ++i)
	{
		if(columnWidthPosition[i]>x-1E-5) return i;
	}
	return 1;
}

static void newLine() //Enter
{
	if(cursorType == 2) deleteTroughCursor();

	if(!cursorType || !cursor1.blockVal)
	{
		blockChain* nbc = createBlock();
		AlignmentInfo ali;
		ali.alignBlockID = 0;
		ali.alignArgument = beginAy;
		ali.alignType = 0;
		ali.column = getColumnFromX(beginAx);
		BlockMove(nbc->curr->ID, ali);
		cursor1.blockVal = nbc->curr;
		cursor1.position = 0;
		cursorType = 1;
		return;
	}

	createBlockAfterChain(corrBlockChain[cursor1.blockVal->ID]);
	Block* oldBlk = cursor1.blockVal;
	Block* newBlk = corrBlockChain[cursor1.blockVal->ID]->next->curr;
	if(cursor1.blockVal->type == 1)
	{
		StyleString* nowss = (StyleString*)oldBlk->dataptr;
		StyleString* toss  = (StyleString*)newBlk->dataptr;
		const int moveChrN = nowss->contentLen - cursor1.position;
		if(toss->contentSpace <= moveChrN)
		{
			toss->contentSpace = moveChrN*2;
			toss->content = realloc(toss->content, moveChrN*2*sizeof(StyleChar));
		}
		memcpy(toss->content, nowss->content+cursor1.position, moveChrN*sizeof(StyleChar));
		nowss->contentLen -= moveChrN;
		toss->contentLen  += moveChrN;
		cursor1.blockVal = newBlk;
		cursor1.position = moveChrN;
	}
	else
	{
		if(cursor1.position == 0) //left side of the image
		{
			void* f = newBlk->dataptr;
			newBlk->type = 2;
			oldBlk->type = 1;
			newBlk->dataptr = oldBlk->dataptr;
			oldBlk->dataptr = f;
		} //else do nothing
		cursor1.blockVal = newBlk;
		cursor1.position = 0;
	}
}

static void backSpace()
{
	if(cursorType == 2)
	{
		deleteTroughCursor();
		return;
	}
	if(!cursorType || !cursor1.blockVal) return;
	if(cursor1.position == 0)
	{
		if(cursor1.blockVal->type==1 && ((StyleString*)(cursor1.blockVal->dataptr))->contentLen==0)
		{
			deleteBlockChain(corrBlockChain[cursor1.blockVal->ID]);
		}
		else
		{
			mergeTextStr(getPre(corrBlockChain[cursor1.blockVal->ID]), corrBlockChain[cursor1.blockVal->ID]);
		}
	}
	else
	{
		if(cursor1.blockVal->type == 2)
		{
			cursor1.blockVal->type = 1;
			FreeImageStructure(cursor1.blockVal->dataptr);
			cursor1.blockVal->dataptr = (void*)newStyleString();
		}
		else
		{
			StyleString* nowss = (StyleString*)cursor1.blockVal->dataptr;
			for(int i=cursor1.position; i<=nowss->contentLen; ++i)
			{
				nowss->content[i-1] = nowss->content[i];
			}
			--nowss->contentLen;
			--cursor1.position;
		}
	}
}

static void keyboardDelete()
{
	if(cursorType == 2)
	{
		deleteTroughCursor();
		return;
	}
	if(!cursorType || !cursor1.blockVal) return;
	if(cursor1.blockVal->type == 2)
	{
		if(cursor1.position == 0)
		{
			cursor1.blockVal->type = 1;
			FreeImageStructure(cursor1.blockVal->dataptr);
			cursor1.blockVal->dataptr = (void*)newStyleString();
		}
	}
	else
	{
		if(cursor1.position == ((StyleString*)cursor1.blockVal->dataptr)->contentLen)
		{
			mergeTextStr(corrBlockChain[cursor1.blockVal->ID], corrBlockChain[cursor1.blockVal->ID]->next);			
		}
		else
		{
			StyleString* nowss = (StyleString*)cursor1.blockVal->dataptr;
			for(int i=cursor1.position+1; i<=nowss->contentLen; ++i)
			{
				nowss->content[i-1] = nowss->content[i];
			}
			--nowss->contentLen;
		}
	}
}

static void newParagraph() //Ctrl+Enter
{
	if(cursorType == 2) mergeCursorBackward();
	createBlockAfterChain(corrBlockChain[cursor1.blockVal->ID]);
	cursor1.blockVal = corrBlockChain[cursor1.blockVal->ID]->next->curr;
	cursor1.position = 0;
}

//////////////////////////////////////////////// GUI接口 ///////////////////////////////

ButtonEvent keyboardspc[8] = {NULL, keyboardUp, keyboardDown, keyboardLeft, keyboardRight, newLine, backSpace, keyboardDelete};
static void keyboardInputSpecial(char keyInputType) //1上 2下 3左 4右 5回车 6退格 7Delete
{
	keyboardspc[(int)keyInputType]();
}

int preClick = 0;
static void mouseLeftDown(double rx,double ry)
{
	if(!preClick)
	{
		if(-ry <= CULUMN_CURSOR_HEGIHT)
		{
			leftclickOnRowRuler(rx,ry);
			preClick = 1;
		}
		else
		{
			mouseLeftDownOnMain(rx,ry+CULUMN_CURSOR_HEGIHT);
			preClick = 2;
		}
	}
	else
	{
		if(preClick==1) leftclickOnRowRuler(rx,ry);
		else mouseLeftDownOnMain(rx,ry+CULUMN_CURSOR_HEGIHT);
	}
}

static void mouseLeftUp()
{
	if(preClick==2) mouseLeftUpOnMain();
	preClick = 0;
}

static void mouseRightDown(double rx,double ry)
{
	if(!preClick)
	{
		if(-ry <= CULUMN_CURSOR_HEGIHT)
		{
			rightClickOnRowRuler(rx,ry);
			preClick = 1;
		}
		else
		{
			mouseRightDownOnMain(rx,ry+CULUMN_CURSOR_HEGIHT);
			preClick = 2;
		}
	}
	else
	{
		if(preClick==1) rightClickOnRowRuler(rx,ry);
		else mouseRightDownOnMain(rx,ry+CULUMN_CURSOR_HEGIHT);
	}
}

static void mouseRightUp()
{
	if(preClick==2) mouseRightUpOnMain();
	preClick = 0;
}

static void drawCulumnCursor(double cx,double cy,double width)
{
	SetPenColor("Red");
	MovePen(cx,cy);
	DrawLine( width,0);
	DrawLine(0,-CULUMN_CURSOR_HEGIHT);
	DrawLine(-width,0);
	DrawLine(0, CULUMN_CURSOR_HEGIHT);
	for(int i=1; i<columnNum; ++i)
	{
		MovePen(columnWidthPosition[i]*width+cx-COLUMN_CURSOR_CLICK_ERROR, cy);
		StartFilledRegion(1);
		DrawLine( 2*COLUMN_CURSOR_CLICK_ERROR, 0);
		DrawLine(0,-CULUMN_CURSOR_HEGIHT);
		DrawLine(-2*COLUMN_CURSOR_CLICK_ERROR, 0);
		DrawLine(0, CULUMN_CURSOR_HEGIHT);
		EndFilledRegion();
	}
	SetPenColor("Black");
}

static void drawMainEdit(double cx,double cy,double dx,double dy)
{
	calculateHeight();
	drawCulumnCursor(cx,cy,dx-cx);
	cy -= CULUMN_CURSOR_HEGIHT;
	double AyBeg = screenBeginHeight;
	double AyEnd = screenBeginHeight - (cy-dy);
	printCursor(cx, cy, screenWidth, screenBeginHeight, screenBeginHeight+screenHeight);
	for(int i=1; i<=blockNum; ++i)
	{
		const int curcol = corrBlockChain[i]->curr->align.column;
		if(blockBeginHeight[i]>=AyBeg && blockBeginHeight[i]+blockHeight[i]<AyBeg)
		{
			if(blockBeginHeight[i]+blockHeight[i] > AyEnd)
			{
				SetPenSize(1);
				MovePen(cx+columnWidthPosition[curcol-1]*(dx-cx),cy);
				DrawLine(0,-AyBeg+blockBeginHeight[i]+blockHeight[i]);
				DrawLine(columnWidth[curcol]*(dx-cx),0);
				DrawLine(0, AyBeg-blockBeginHeight[i]-blockHeight[i]);
				DrawBlockInBlockList(corrBlockChain[i]->curr,
					cx+columnWidthPosition[curcol-1]*(dx-cx),
					cy,
					columnWidth[curcol]*(dx-cx),
					AyBeg-blockBeginHeight[i],
					blockHeight[i]);
			}
			else
			{
				SetPenSize(1);
				MovePen(cx+columnWidthPosition[curcol-1]*(dx-cx),cy);
				DrawLine(0,AyEnd-AyBeg);
				MovePen(cx+columnWidthPosition[curcol]*(dx-cx),cy);
				DrawLine(0,AyEnd-AyBeg);
				DrawBlockInBlockList(corrBlockChain[i]->curr,
					cx+columnWidthPosition[curcol-1]*(dx-cx),
					cy,
					columnWidth[curcol]*(dx-cx),
					AyBeg-blockBeginHeight[i],
					AyEnd-blockBeginHeight[i]);
			}
		}
		else if(blockBeginHeight[i]<AyBeg && blockBeginHeight[i]>=AyEnd)
		{
			if(blockBeginHeight[i]+blockHeight[i] > AyEnd)
			{
				SetPenSize(1);
				MovePen(cx+columnWidthPosition[curcol-1]*(dx-cx), cy+blockBeginHeight[i]-AyBeg);
				DrawLine( columnWidth[curcol]*(dx-cx), 0);
				DrawLine(0, blockHeight[i]);
				DrawLine(-columnWidth[curcol]*(dx-cx), 0);
				DrawLine(0,-blockHeight[i]);
				DrawBlockInBlockList(
					corrBlockChain[i]->curr,
					cx+columnWidthPosition[curcol-1]*(dx-cx),
					cy+blockBeginHeight[i]-AyBeg,
					columnWidth[curcol]*(dx-cx),
					0,
					blockHeight[i]);
			}
			else
			{
				SetPenSize(1);
				MovePen(cx+columnWidthPosition[curcol-1]*(dx-cx), cy+blockBeginHeight[i]-AyBeg);
				DrawLine(columnWidth[curcol]*(dx-cx), 0);
				DrawLine(0, AyEnd-blockBeginHeight[i]);
				MovePen(cx+columnWidthPosition[curcol-1]*(dx-cx), cy+blockBeginHeight[i]-AyBeg);
				DrawLine(0, AyEnd-blockBeginHeight[i]);
				DrawBlockInBlockList(
					corrBlockChain[i]->curr,
					cx+columnWidthPosition[curcol-1]*(dx-cx),
					cy+blockBeginHeight[i]-AyBeg,
					columnWidth[curcol]*(dx-cx),
					0,
					AyEnd-blockBeginHeight[i]);
			}
		}
	}
}

//////////////////////////////////////////////// 初始化接口 ////////////////////////////

void EditorCoreInitCallbacks()
{
	RegisterColorMethod(colorChange);
	RegisterBoldMethod(bold);
	RegisterItalicMethod(italic);
	RegisterPointSizeMethod(pointSizeChange);
	RegisterFontMethod(fontChange);

	RegisterEditorGraphicDraw(drawMainEdit);
	RegisterRollerUpperBoundDraw(getRollerBegin);
	RegisterRollerLowerBoundDraw(getRollerEnd);

	RegisterEditorMouseLeftDown(mouseLeftDown);
	RegisterEditorMouseLeftUp(mouseLeftUp);
	RegisterEditorMouseRightDown(mouseRightDown);
	RegisterEditorMouseRightUp(mouseRightUp);
	RegisterEditorMouseMiddleRollup(rollerRollUp);
	RegisterEditorMouseMiddleRolldown(rollerRollDown);
	RegisterEditorKeyboard(keyboardInput);
	RegisterEditorKeyboardSpecial(keyboardInputSpecial);
	RegisterRollerMouseLeftDown(rollerLeftDown);
	
	RegisterSetRollerHeightMethod(changeScreenHeight);
	RegisterSetEditorWidth(setWidth);
}
