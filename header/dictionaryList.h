#ifndef __LIST_STRUCTURE_H_INCLUDED__
#define __LIST_STRUCTURE_H_INCLUDED__

//һ���������ߵ����ݽṹ�ļ���ά��һ���ɵ��չ�����б� 
//���������ݽṹ����ʾ���������洢�κ����� 
//���ڲ��洢���ݣ����Բ��ṩ���������������������������������ʵ�ֱ�������� 
typedef struct dditemBase{
	int itemEmphasizeType;
	int itemID;
	struct dditemBase* prevItem;
	struct dditemBase* nextItem;
	struct ddfolderBase* folder;
	char* itemName; //�� malloc ����Ŀռ䣬�뱣ֻ֤��һ��ָ��ָ���ǿ��ڴ� 
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
	char* folderName; //�� malloc ����Ŀռ䣬�뱣ֻ֤��һ��ָ��ָ���ǿ��ڴ� 
} DictionaryFolder;

typedef struct{
	int pointSize;
	int indent;
	const char* entryFont;   //���¶����ⲿ������ַ�ָ�룬����֤��ָ���ڴ�ķ��䷽ʽ 
	const char* folderColors[15];
	const char* folderBoarderColors[15];
	const char* folderFillColors[15];
	const char* itemColors[15];
} DictionaryGraphicDatas;

/*
*
* �ڴ�����ͬʱ����ȱʡֵ 
* itemEmphasizeType Ϊ 0����ǿ���� 
* prevItem, nextItem �� folder ������Ϊ NULL
* Ϊ itemName ��ʼ����Ϊ "Noname"
* Ϊ itemID ����һ������������ʼֵ�ظ��ĳ�ʼֵ���� 1 ��ʼ������ӣ� 
* 
*/

DictionaryItem* CreateDictionaryItem();
void FreeDictionaryItem(DictionaryItem*);
DictionaryItem* CopyDictionaryItem(DictionaryItem* sorce);

/*
*
* �ڴ�����ͬʱ����ȱʡֵ 
* folderExpended Ϊ 0����չ���� 
* folderEmphasizeType Ϊ 0����ǿ���� 
* prevFolder, nextFolder, parent, items, subFolders ������Ϊ NULL
* Ϊ itemName ��ʼ����Ϊ "Noname"
* Ϊ folderID ����һ������������ʼֵ�ظ��ĳ�ʼֵ���� 1 ��ʼ������ӣ� 
* 
*/

DictionaryFolder* CreateDictionaryFolder();
void FreeDictionaryFolder(DictionaryFolder*);
DictionaryFolder* CopyDictionaryFolder(DictionaryFolder* sorce);

/*
*
* �ڴ�����ͬʱ������ȱʡֵ
* pointSize Ϊ -1�������ģ�
* indent Ϊ 4 
* entryFont Ϊ NULL�������ģ�
* folderColors Ϊ {"Black","Red","Green","Blue"}
* folderBoarderColors Ϊ {NULL, NULL, NULL, NULL} ��͸����
* folderFillColors Ϊ {NULL, NULL, NULL, NULL} ��͸���� 
* itemColors Ϊ {"Black","Red","Green","Blue"}
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
