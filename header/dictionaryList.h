#ifndef __LIST_STRUCTURE_H_INCLUDED__
#define __LIST_STRUCTURE_H_INCLUDED__

//一个用作工具的数据结构文件，维护一个可点击展开的列表 
//仅定义数据结构和显示方法，不存储任何数据 
//由于不存储数据，所以不提供遍历方法与操作方法，由下游任务自行实现遍历与操作 
typedef struct dditemBase{
	int itemEmphasizeType;
	int itemID;
	struct dditemBase* prevItem;
	struct dditemBase* nextItem;
	struct ddfolderBase* folder;
	char* itemName; //由 malloc 申请的空间，请保证只有一个指针指向那块内存 
} DictionaryItem;

typedef struct ddfolderBase{
	int folderExpended;
	int folderEmphasizeType; 
	int folderID;
	struct ddfolderBase* parent;
	struct ddfolderBase* prevFolder;
	struct ddfolderBase* nextFolder;
	struct dditemBase*   items;
	struct ddfolderBase* subFolders;
	char* folderName; //由 malloc 申请的空间，请保证只有一个指针指向那块内存 
} DictionaryFolder;

typedef struct{
	int pointSize;
	int indent;
	const char* entryFont;   //以下都是外部输入的字符指针，不保证所指向内存的分配方式 
	const char* folderColors[15];
	const char* folderBoarderColors[15];
	const char* folderFillColors[15];
	const char* itemColors[15];
} DictionaryGraphicDatas;

/*
*
* 在创建的同时设置缺省值 
* itemEmphasizeType 为 0（不强调） 
* prevItem, nextItem 与 folder 均设置为 NULL
* 为 itemName 初始设置为 "Noname"
* 为 itemID 设置一个不与其他初始值重复的初始值（从 1 开始逐个增加） 
* 
*/

DictionaryItem* CreateDictionaryItem();
void FreeDictionaryItem(DictionaryItem*);
DictionaryItem* CopyDictionaryItem(DictionaryItem* sorce);

/*
*
* 在创建的同时设置缺省值 
* folderExpended 为 0（不展开） 
* folderEmphasizeType 为 0（不强调） 
* prevFolder, nextFolder, parent, items, subFolders 均设置为 NULL
* 为 itemName 初始设置为 "Noname"
* 为 folderID 设置一个不与其他初始值重复的初始值（从 1 开始逐个增加） 
* 
*/

DictionaryFolder* CreateDictionaryFolder();
void FreeDictionaryFolder(DictionaryFolder*);
DictionaryFolder* CopyDictionaryFolder(DictionaryFolder* sorce);

/*
*
* 在创建的同时会设置缺省值
* pointSize 为 -1（不更改）
* indent 为 4 
* entryFont 为 NULL（不更改）
* folderColors 为 {"Black","Red","Green","Blue"}
* folderBoarderColors 为 {NULL, NULL, NULL, NULL} （透明）
* folderFillColors 为 {NULL, NULL, NULL, NULL} （透明） 
* itemColors 为 {"Black","Red","Green","Blue"}
*
*/

DictionaryGraphicDatas* CreateDictionaryGraphicDatas();
void FreeDictionaryGraphicDatas(DictionaryGraphicDatas*);
DictionaryGraphicDatas* CopyDictionaryGraphicDatas(DictionaryGraphicDatas* sorce);

void DrawDictionaryList(DictionaryGraphicDatas* dl, DictionaryFolder* fl, double cx, double cy, double width, double begH, double endH);
double GetDictionaryListHeight(DictionaryGraphicDatas* dl, DictionaryFolder* fl, double width);

typedef struct {
	int pointEntryType; //0:space 1:folder 2:item
	DictionaryFolder* folderIn; //NULL if on the outside
	DictionaryFolder* pointFolder; //NULL if the entry pointed is item or space
	DictionaryItem*   pointItem; //NULL if the entry pointed is item or space
} DictionaryCursor;

DictionaryCursor PositionizeDictionaryList(DictionaryGraphicDatas* dl, DictionaryFolder* fl, double width, double mX, double mY);

#endif 
