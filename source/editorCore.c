#include "editCore.h"
#include "blockList.h"
#include "textStructure.h"

typedef struct blockChainBase {
	block* curr;
	struct blockChainBase* next;
} blockChain;


/////////////////////////////////////// ����������߼� //////////////////////////////////////
static int blockChainPage = 0;
static blockChain* corrBlockChain[65535];   static int blockNum;
static double columnWidth[255];             static int columnNum;
static double columnWidthPosition[255];
static double screenWidth;

static void setWidth(double w) { screenWidth = w; }

static void buildColumns()
{
	columnNum = GetColumnNum();
	for(int i=1; i<=columnNum; ++i)
	{
		columnWidth[i] = GetColumnWidth(i);
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

static void buildBC()  //ͬʱ���
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

///////////////////////////////////////// ��߶ȼ��� //////////////////////////////////////////////

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
	
	blockHeight[bc->curr->ID] = GetHeight(bc->curr, preWidth*columnWidth[bc->curr->align.culumn]);
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

//////////////////////////////////////// ��Ļλ��ά�� ////////////////////////////////////

static double screenHeight;
static double screenBeginHeight; // all negative
#define ROLLER_STEP 0.1

//used as callback
static void changeScreenHeight(double newScreenHeight) { screenHeight = -newScreenHeight; }

static double getRollerBegin() { return screenBeginHeight/fullHeight; }
static double getRollerEnd()   { return screenBeginHeight+screenHeight<=fullHeight ? 1 : (screenBeginHeight+screenHeight)/fullHeight; }

static double rollerRollUp()   { if(screenBeginHeight<0) screenBeginHeight+=ROLLER_STEP; }
static double rollerRollDown() { if(screenBeginHeight+screenHeight>fullHeight) screenBeginHeight-=ROLLER_STEP; }

static double rollerLeftDown(double mx, double my) { screenBeginHeight = my/screenHeight*fullHeight; }

//////////////////////////////////////// ���λ�ö���ʹ��� ////////////////////////////////////

typedef struct
{
	block* blockVal;
	int position;
} cursorMsg;

static int cursorType; //�������
static cursorMsg cursor1, cursor2; //˳����
#define CURSOR_DENSITY 0.2

static void printCursor(double wx, double wy, double width, double begH, double endH)
{
	if(cursorType == 1) //single cursor
	{
		const double cw = width*columnWidth[cursor1.blockVal->align.column];
		const double rx = GetRelativeXFromPosition(cursor1.blockVal, cw, cursor1.position);
		const double ry = GetRelativeYFromPosition(cursor1.blockVal, cw, cursor1.position);
		double lH = GetElementHeight(cursor1.blockVal);
		double Ax = width*columnWidthPosition[cursor1.blockVal->align.column] + rx;
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
		//1. ��������ָ���Ƿ���ͬһ������
		//2. ���ƿ��ϵĿ�ʼ��
		//3. �����м��
		//4. ���ƿ��µĽ�����
	}
}

cursorMsg getCursorPosition(double Ax, double Ay, double width)
{
	int curcol = 1;
	while(curcol<columnNum && columnWidthPosition[curcol+1]*width < Ax) ++curcol;
	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]->curr->align.culumn != curcol) continue;
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
			Ax-columnWidthPosition[curcol],
			Ay-blockBeginHeight[i]);
	
		return nc;
	}

	cursorMsg nc;
	nc.blockVal = 0;
	return nc;
}

//////////////////////////////////////////////// ������ ////////////////////////////////

static int mouseup = 1;

// TODO
// �������迼��һ�� LeftDown ���Ͻ� LeftUp

static void keyboardUp()

static void keyboardDown()

static void keyboardLeft()

static void keyboardRight()

static void mouseLeftDownOnMain(double rx, double ry)

static void mouseLeftUpOnMain()

//////////////////////////////////////////////// ���ƶ����� /////////////////////////////

// mouseRightDown
// mouseRightUp
// �������迼��һ�� RightDown ���Ͻ� RightUp

#define NEWLINE_MARGIN 0.1
#define INDENT 4
static int defaultPointSize = 5;
static char* defaultColor = "Black"; // �������ַ��������ھ�̬�Σ����ᱻ�޸�
static char* defaultFont = "Times";

static void createBlockAfterChain(blockChain* pre)
{
	assert(pre);
	StyleString* newss = malloc(sizeof(StyleString));
	newss->content = (StyleChar*)malloc(100*sizeof(StyleChar));
	newss->contentLen = 0;
	newss->contentSpace = 100;
	newss->indent = INDENT;
	newss->pointSize = defaultPointSize;
	newss->color = defaultColor;
	newss->font = defaultFont;

	Block* newBlock = BlockCreate();
	newBlock->type = 1;
	newBlock->dataptr = (void*)newss;

	AlignmentInfo ali;
	ali.alignBlockID = pre->curr->ID;
	ali.alignType = 2;
	ali.alignArgument = NEWLINE_MARGIN;
	ali.culumn = pre->curr->align.culumn;
	BlockMove(newBlock->ID, ali);

	corrBlockChain[newss->ID] = (blockChain*)malloc(sizeof(blockChain));
	corrBlockChain[newss->ID]->curr = newss;
	corrBlockChain[newss->ID]->next = pre->next;
	if(pre->next) pre->next->curr->align.alignBlockID = newBlock->ID;

	pre->next = newss;
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
	BlockMove(x->curr->ID, newAlign);
	if(newAlign->alignBlockID && newAlign->culumn==GetBlock(newAlign->alignBlockID)->align->column)
	{
		BlockChain* t  = getTail(x);
		BlockChain* pn = corrBlockChain[newAlign.alignBlockID]->next;
		corrBlockChain[newAlign.alignBlockID]->next = x;
		if(pn) pn->curr->align.alignBlockID = t->curr->ID;
		t->next = pn;
	}
}

static void deleteBlock(blockChain* x, AlignmentInfo newAlign)
{
	blockChain* pre = 0;
	if(x->curr->align.alignBlockID && corrBlockChain[x->curr->align.alignBlockID]->next==x)
	{
		pre = corrBlockChain[x->curr->align.alignBlockID];
		pre->next = x->next;
		if(x->next) x->next->curr->align.alignBlockID = pre->curr->ID;
	}

	for(int i=1; i<=blockNum; ++i)
	{
		if(corrBlockChain[i]->curr->align.alignBlockID == x->curr->ID)
		{
			corrBlockChain[i]->curr->align.alignBlockID = x->curr->align.alignBlockID
			corrBlockChain[i]->curr->align.alignArgument += x->curr->alignArgument;
			corrBlockChain[i]->curr->align.alignType = x->curr->align.alignType;
		}
	}

	x->align->alignBlockID = 0;
	x->align->alignArgument = 0;
	BlockDelete(x->curr);
}

static void mouseRightDownOnMain(double rx, double ry)

static void mouseRightUpOnMain()

//////////////////////////////////////////////// ���߼� ////////////////////////////////

static void leftclickOnRowRuler(double rx, double ry)
{
	const double curClickWidth = rx/
}

static void rightClickOnRowRuler(double rx, double ry)

//////////////////////////////////////////////// �༭ //////////////////////////////////

static int defaultStyle;

static void deleteTroughCursor()
{
	if(cursorType!=2) return;
}

static void bold()
{

}

static void italic()
{

}

// color change �� font change ��Ҫ lookup ���λ�� fileSystemCore �е��ȶ�ָ��
static void fontChange(const char* newFont)
{

}

static void colorChange(const char* newColor)
{

}

static void keyboardInput(char ch)
{

}

static void newLine() //Enter
{

}

static void backSpace()
{

}

static void keyboardDelete()
{

}

static void newParagraph() //Ctrl+Enter
{

}

//////////////////////////////////////////////// �˵������� ////////////////////////////
//�˵���Ӧ����
/*
static char *fileMenu[]  = {"File", "Save", "Open", "New", "Save As", "Close"};
static char *toolsMenu[] = {"Tools", "Stats", "Search", "Browse"};
static char *aboutMenu[] = {"About", "Settings", "Help"};
*/

static void save() //Ctrl+S

static void open()

static void new()

static void saveAs()

static void close()

static void tools()

static void stats()

static void search()

static void browse()

static void about()

static void settings()

static void help()

//////////////////////////////////////////////// ��ʼ���ӿ� ////////////////////////////

static void initCallbacks()
{

}

//////////////////////////////////////////////// GUI�ӿ� ///////////////////////////////

static void keyboardInputSpecial(char keyInputType) //1�� 2�� 3�� 4�� 5�س� 6�˸� 7Delete

static void mouseLeftDown(double rx,double ry)

static void mouseLeftUp()

static void mouseRightDown(double rx,double ry)

static void mouseRightUp()