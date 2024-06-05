#include "editCore.h"
#include "blockList.h"
#include "textStructure.h"
#include "myGUI.h"

typedef struct blockChainBase {
	block* curr;
	struct blockChainBase* next;
} blockChain;

static int blockChainPage = 0;
static blockChain* corrBlockChain[65535];   static int blockNum;
static double columnWidth[255];             static int columnNum;
static double columnWidthPosition[255];

/////////////////////////////////////// 基本操作 ///////////////////////////////////////////

corrBlockChain* getPre(corrBlockChain* x)
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
}

static void buildBlock(Block* blk)
{
	if(corrBlockChain[blk->ID]) return;
	++blockNum;
	corrBlockChain[blk->ID] = (blockChain*)malloc(sizeof(blockChain));
	corrBlockChain[blk->ID]->curr = blk;
	if(blk->align.alignBlockID && GetBlock(blk->align.alignBlockID)->align.column==blk->align.column)
	{
		buildBlock(GetBlock(blk->align.alignBlockID));
		corrBlockChain[blk->align->alignBlockID]->next = corrBlockChain[blk->ID];
	}
}

static void buildBC()  //同时清空
{
	TraverseColorDifinitions(DefineColor);
	buildColumns();

	for(int i=1; i<=blockNum; ++i)
	{
		free(corrBlockChain[i]);
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
	if(p!=blockChainPage)
	{
		blockChainPage = p;
		buildBC();
	}
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
	travelled[bc->curr->ID] = 1;
	const int preID = bc->curr->align.alignBlockID;
	if(!travelled[preID]) calculateBlockHeight(corrBlockChain[preID]);
	
	blockHeight[bc->curr->ID] = GetHeight(bc->curr, preWidth*columnWidth[bc->curr->align.column]);
	blockBeginHeight[bc->curr->ID] = calculateAlign(bc->curr->align);
}

static double fullHeight;
static double getfullHeight()
{
	double screenHeight = 0;
	for(int i=1; i<=blockNum; ++i)
	{
		if(blockBeginHeight[i]+blockHeight[i] < screenHeight)
		{
			screenHeight = blockBeginHeight[i] + blockHeight[i];
		}
	}
	fullHeight = screenHeight;
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

//used as callback
static void changeScreenHeight(double newScreenHeight) { screenHeight = -newScreenHeight; }

static double getRollerBegin() { return screenBeginHeight>0 ? 0 : screenBeginHeight/fullHeight; }
static double getRollerEnd()   { return screenBeginHeight+screenHeight<=fullHeight ? 1 : (screenBeginHeight+screenHeight)/fullHeight; }

static double rollerRollUp()   { if(screenBeginHeight<0) screenBeginHeight+=ROLLER_STEP; }
static double rollerRollDown() { if(screenBeginHeight+screenHeight>fullHeight) screenBeginHeight-=ROLLER_STEP; }

static double rollerLeftDown(double mx, double my) { screenBeginHeight = my/screenHeight*fullHeight; }

//////////////////////////////////////// 光标操作 ///////////////////////////////////////////

typedef struct
{
	block* blockVal;
	int position; //position是唯一指定块中光标位置的变量
} cursorMsg;

static int cursorType; //光标数量
static cursorMsg cursor1, cursor2; //顺序不限
#define CURSOR_DENSITY 0.2

static void printCursor(double wx, double wy, double width, double begH, double endH)
{
	if(cursorType == 1) //single cursor
	{
		const double cw = width*columnWidth[cursor1.blockVal->align.column];
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		double lH = GetElementHeight(cursor1.blockVal);
		double Ax = width*columnWidthPosition[cursor1.blockVal->align.column-1] + rx;
		double Ay = ry + blockBeginHeight[cursor1.blockVal->ID];
		if(Ay<=begH && Ay>=endH)
		{
			movePen(wx+Ax, wy+Ay-begH);
			DrawLine(0,lH);
		}
	}
	else
	{
		StartFilledRegion(CURSOR_DENSITY);
		//TODO
		//1. 特判两个指针是否处在同一个块中
		//2. 绘制靠上的开始块
		//3. 绘制中间块
		//4. 绘制靠下的结束块
	}
}

cursorMsg getCursorPosition(double Ax, double Ay, double width)
{
	int curcol = 1;
	while(curcol<columnNum && columnWidthPosition[curcol]*width < Ax) ++curcol;
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
			columnWidth[curcol]*width, 
			Ax-columnWidthPosition[curcol-1]*width,
			Ay-blockBeginHeight[i]);
	
		return nc;
	}

	cursorMsg nc;
	nc.blockVal = 0;
	return nc;
}

static int mouseup = 1;

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
		const double cw = width*columnWidth[cursor1.blockVal->align.column];
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double cw = width*columnWidth[cursor1.blockVal->align.column];
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
		const double cw = width*columnWidth[cursor1.blockVal->align.column];
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		const int sLen = (StyleString*)(cursor1.blockVal->dataptr)->contentLen;
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
		cursor1ToHead(corrBlockChain[cursor1.blockVal->ID].next);
	}
}

static void mouseUped = 1;
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
		blockChain* nxt = corrBlockChain[cursor2.blockVal->ID].next;
		if(pre && blockBeginHeight[pre->curr->ID]+blockHeight[pre->curr->ID] < Ay)
		{
			cursor2.blockVal = pre;
		}
		if(nxt && blockBeginHeight[nxt->curr->ID] > Ay)
		{
			cursor2.blockVal = nxt;
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
			endAy-blockBeginHeight);
	}
	mouseUped = 1;
}

//////////////////////////////////////////////// 块移动处理 /////////////////////////////

// mouseRightDown
// mouseRightUp
// 单击仅需考虑一次 RightDown 马上接 RightUp

#define NEWLINE_MARGIN 0.1
#define INDENT 4
static int defaultPointSize = 5;
static char* defaultColor = "Black"; // 这两个字符串保存在静态段，不会被修改
static char* defaultFont = "Times";

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

	Block* newBlock = BlockCreate();
	newBlock->type = 1;
	newBlock->dataptr = (void*)newStyleString();

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

	if(x->curr->align.column != newAlign->column)
	{
		for(blockChain* p=x->next; p; p=p->next)
		{
			p->curr->align->column = newAlign->column;
		}
	}
	
	if(newAlign->alignBlockID && newAlign->column==GetBlock(newAlign->alignBlockID)->align->column)
	{
		newAlign.alignType = 2;
		newAlign.alignArgument = NEWLINE_MARGIN;
		BlockChain* t  = getTail(x);
		BlockChain* pn = corrBlockChain[newAlign.alignBlockID]->next;
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
			corrBlockChain[i]->curr->align.alignArgument += x->curr->alignArgument;
			corrBlockChain[i]->curr->align.alignType = x->curr->align.alignType;
		}
	}

	x->curr->align.alignBlockID = 0;
	x->curr->align.alignArgument = 0;
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
	++columnNum; ++i
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
			corrBlockChain* p = corrBlockChain[i];
			corrBlockChain* pre = getPre(p);
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

	columnWidth[i+1] += columnWidth[i];
	for(int i=colID; i<columnNum; ++i)
	{
		columnWidthPosition[i] = columnWidthPosition[i+1];
		columnWidth[i] = columnWidth[i+1];
	}
	--columnNum;
	SetColumnInfo(columnNum, columnWidth);
}

#define clickError 0.01
static void leftclickOnRowRuler(double rx, double ry)
{
	const double curClickWidth = rx/screenWidth;
	for(int i=1; i<columnNum; ++i)
	{
		if(columnWidthPosition[i]-clickError<rx && columnWidthPosition[i]+clickError>rx)
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
		if(columnWidthPosition[i]-clickError<rx && columnWidthPosition[i]+clickError>rx)
		{
			deleteColumn(i);
			return;
		}
	}
}

//////////////////////////////////////////////// 编辑 //////////////////////////////////

static int defaultStyle;
typedef void (*blockOperatorFunc)(Block*)

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

static void deleteTroughCursor() //顺便还原成 cursorType=1 的状态
{
	if(cursorType!=2) return;
	checkPriorier();
	if(cursor1.blockVal == cursor2.blockVal)
	{
		//TODO
	}
}

static void applyBlocksTroughCursor(blockOperatorFunc b) //还原成 cursorType=1 的状态
{
	if(cursorType!=2) return;
}

static void changeStyleTroughCursor(int newStyle)
{
	if(cursorType!=2) return;
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

static void fontChange(const char* newFont)
{
	const int fontID = LookupFontNameInFontTable(newFont);
	if(!fontID) fontID = RegisterFontTable(newFont);
	const char* nowDefault = defaultFont;
	defaultFont = LookupIDInFontTable(fontID);
	if(cursorType == 2)
	{
		applyBlocksTroughCursor(fontChangeForSingleBlock)
		defaultFont = nowDefault;
	}
}

static void colorChange(const char* newColor)
{
	const int colorID = LookupColorNameInColorTable(newColor);
	if(!colorID) colorID = RegisterColorTable(newColor);
	const char const* nowDefault = defaultColor;
	defaultColor = LookupIDInColorTable(colorID);
	if(cursorType == 2)
	{
		applyBlocksTroughCursor(colorChangeForSingleBlock)
		defaultColor = nowDefault;
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
	sstr->content[cursor1.position] = ch;
	++cursor1.position;
}

static int getColumnFromX(double x)
{
	for(int i=1; i<=columnNum; ++i)
	{
		if(columnWidthPosition[i]>x-1E-5) return i;
	}
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
		ali.column = getColumnFromX(Ax);
		BlockMove(nbc->curr->ID, ali);
	}

	createBlockAfterChain(corrBlockChain[cursor1.blockVal->ID]);
	const Block* oldBlk = cursor1.blockVal;
	const Block* newBlk = corrBlockChain[cursor1.blockVal->ID]->next->curr;
	if(cursor1.blockVal->type == 1)
	{
		const StyleString* nowss = (StyleString*)oldBlk->dataptr;
		const StyleString* toss  = (StyleString*)newBlk->dataptr;
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
		if(cursor1.blockVal->type==1 && (StyleString*)(cursor1.blockVal->dataptr)->contentLen==0)
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
		if(cursor1.position == ((StyleString*)c1.blockVal->dataptr)->contentLen)
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
	keyboardspc[keyInputType]();
}

#define CULUMN_CURSOR_HEGIHT 0.1

static void mouseLeftDown(double rx,double ry)
{

}

static void mouseLeftUp()

static void mouseRightDown(double rx,double ry)

static void mouseRightUp()

static void drawMainEdit()

//////////////////////////////////////////////// 初始化接口 ////////////////////////////

void EditorCoreInitCallbacks()
{
	
}
