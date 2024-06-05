#include "explorerCore.h"
#include "dictionaryList.h"
#include "blockList.h"
#define NEW(x)  ((x*)malloc(sizeof(x)))
#define NEWVARR(x,len) ((x*)malloc((len)*sizeof(x)))

static char* tags[65535];                 static int tagStoreNum;
static FolderHeaderInfo* folders[65535];  static int folderNum;
static FileHeaderInfo*   files[65535];    static int fileNum;
static int fileOnBlockListPage[65535];    static int fileOnEditNum;
static char userName[255];                static int userNameLen;
static int currentFile;
static int editPage[65535];

static DictionaryFolder* corresDFolder[65535];
static DictionaryFolder* corresDFile[65535];
static DictionaryFolder* fullFolder;

///////////////////////////////////////////////// 基础函数 //////////////////////////////////////
static void changeCurrentFileByID(int newFile)
{
	if(currentFile)
	{
		corresDFile[currentFile]->itemEmphasizeType = 1;
	}
	currentFile = newFile;
	corresDFile[currentFile]->itemEmphasizeType = 2;
	ChangePageOfEditCore(fileOnBlockListPage[newFile]);
}

static int openFile(int fileID) //返回打开页码
{
	if(fileOnBlockListPage[fileID])
	{
		return fileOnBlockListPage[fileID];
	}

	const int curBLPage = GetPageOfBlockList();
	++fileOnEditNum;
	fileOnBlockListPage[fileID] = fileOnEditNum;
	LoadFileAtPage(fileOnEditNum, files[currentFile]->filePath);
	corresDFile[fileID]->itemEmphasizeType = 1;
	changeCurrentFileByID(fileID);
	
	return fileOnEditNum;
}

static void closeFile(int fileID)
{
	assert(currentFile != fileID);
	corresDFile[fileID]->itemEmphasizeType = 0;
	if(fileOnBlockListPage[fileID] == fileOnEditNum) --fileOnEditNum;
	fileOnBlockListPage[fileID] = 0;
}

///////////////////////////////////////////////// 读取、写入 /////////////////////////////////////// 
static void readTags(FILE* f)
{
	fread(&tagStoreNum,sizeof(int),1,f);
	for(int i=1; i<=tagStoreNum; ++i)
	{
		int strLen;
		fread(&strLen,sizeof(int),1,f);
		tags[i] = NEWVARR(char,strLen+5);
		fread(tags+i,sizeof(char),strLen,f);
	}
}

typedef struct {
	int folderNameLen;
	int parentFolderID;
} folderExchangeInfo;

folderExchangeInfo folderBuffer[65535];

static void readFolders(FILE* f)
{
	fread(&folderNum,sizeof(int),1,f);
	fread(folderBuffer,sizeof(folderExchangeInfo),folderNum,f);
	for(int i=1; i<=folderNum; ++i)
	{
		folders[i] = NEW(FolderHeaderInfo); //防止有parentFolder的id比自己大的情况
	}
	for(int i=1; i<=folderNum; ++i)
	{
		folders[i]->folderID = i;
		folders[i]->folderName = NEWVARR(char, folderBuffer[i].folderNameLen+5);
		folders[i]->parentFolder = folders[folderBuffer[i].parentFolderID];
		fread(folders[i]->folderName, sizeof(char), folderBuffer[i].folderNameLen, f);
	}
}

typedef struct {
	int tagNum;
	int editTime;
	int fileNameLen;
	int folderID;
	int filePathLen;
} fileExchangeInfo;

fileExchangeInfo fileBuffer[65535];

void readFiles(FILE* f)
{
	fread(&fileNum, sizeof(int), 1, f);
	fread(fileBuffer, sizeof(fileExchangeInfo), fileNum, f);
	for(int i=1; i<=fileNum; ++i)
	{
		files[i] = NEW(FileHeaderInfo);
		files[i]->editTime = fileBuffer[i].editTime;
		files[i]->tagNum = fileBuffer[i].tagNum;
		files[i]->folder = folders[fileBuffer[i].folderID];
		files[i]->fileName = NEWVARR(char, fileBuffer[i].fileNameLen+5);
		files[i]->tags = NEWVARR(int, fileBuffer[i].tagNum+5);
		filePath[i] = NEWVARR(char, fileBuffer[i].filePathLen+5);
		fread(files[i]->fileName, sizeof(char), fileBuffer[i].fileNameLen, f);
		fread(files[i]->tags,     sizeof(int),  fileBuffer[i].tagNum,      f);
		fread(files[i]->filePath, sizeof(char), fileBuffer[i].filePathLen, f);
	}
}

void ReadSavFile(FILE* f)
{
	readTags(f);
	readFolders(f);
	readFiles(f);
	fread(&userNameLen, sizeof(int), 1, f);
	fread(userName, sizeof(char), userNameLen, f);
	fread(&currentFile, sizeof(int), 1, f);
	memset(fileOnBlockListPage, 0, sizeof(fileOnBlockListPage));
	fileOnEditNum = 0; 
}

/*
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

typedef struct dditemBase{
	int itemEphasizeType;
	int itemID;
	struct dditemBase* prevItem;
	struct dditemBase* nextItem;
	struct ddfolderBase* folder;
	char* itemName; //由 malloc 申请的空间，请保证只有一个指针指向那块内存 
} DictionaryItem;
*/

void BuildFiles()
{
	fullFolder = CreateDictionaryFolder();
	strcpy(fullFolder->folderName, userName);
	fullFolder->folderExpended = 1;
	fullFolder->folderID = 0;

	for(int i=1; i<=folderNum; ++i)
	{
		DictionaryFolder* now = CreateDictionaryFolder();
		strcpy(now->folderName, folders[i]->folderName);
		now->folderID = i;
		now->parent = folders[i]->parentFolder ? 
			corresDFolder[(folders[i]->parentFolder)->folderID] :
			fullFolder;
		if(!now->parent->subFolders)
		{
			now->parent->subFolders = now;
		}
		else
		{
			now->parent->subFolders->nextFolder = now;
			now->prevFolder = now->parent->subFolders;
			now->parent->subFolders = now;
		}
		corresDFolder[i] = now;
	}

	for(int i=1; i<=itemNum; ++i)
	{
		DictionaryItem* now = CreateDictionaryItem();
		strcpy(now->itemName, files[i]->fileName);
		now->itemID = i;
		now->folder = files[i]->folder ? files[i]->folder : fullFolder;
		if(!now->folder->items)
		{
			now->folder->items = now;
		}
		else
		{
			now->folder->items->nextItem = now;
			now->prevItem = now->folder->items;
			now->folder->items = now;
		}
		corresDItem[i] = now;
	}
	
	for(int i=1; i<=folderNum; ++i)
	{
		if(corresDFolder[i]->subFolders)
		{
			while(corresDFolder[i]->subFolders->prevFolder)
			{
				corresDFolder[i]->subFolders = corresDFolder[i]->subFolders->prevFolder;
			}
		}
		if(corresDFolder[i]->items)
		{
			while(corresDFolder[i]->items->prevItem)
			{
				corresDFolder[i]->items = corresDFolder[i]->items->prevItem;
			}
		}
	}

	openFile(currentFile);
	changeCurrentFileByID(currentFile);
}

static void writeTags(FILE* f)
{	
	fwrite(&tagStoreNum,sizeof(int),1,f);
	for(int i=1; i<=tagStoreNum; ++i)
	{
		int strLen = strlen(tags[i]);
		fwrite(&strLen,sizeof(int),1,f);
		fwrite(tags+i,sizeof(char),strLen,f);
	}
}

static void writeFolders(FILE* f)
{
	for(int i=1; i<=folderNum; ++i)
	{
		folderBuffer[i].folderNameLen = strlen(folders[i]->folderName);
		folderBuffer[i].parentFolderID = folders[i]->parentFolder->folderID;
	}
	
	fwrite(&folderNum,sizeof(int),1,f);
	fwrite(folderBuffer,sizeof(folderExchangeInfo),folderNum,f);
	
	for(int i=1; i<=folderNum; ++i)
	{
		fwrite(folders[i]->folderName, sizeof(char), strlen(folders[i]->folderName), f);
	}
}

void writeFiles(FILE* f)
{
	for(int i=1; i<=fileNum; ++i)
	{
		fileBuffer[i].editTime = files[i]->editTime;
		fileBuffer[i].fileNameLen = strlen(files[i]->fileName);
		fileBuffer[i].filePathLen = strlen(files[i]->filePath);
		fileBuffer[i].folderID = files[i]->folder->folderID;
		fileBuffer[i].tagNum = files[i]->tagNum;
	}

	fwrite(&fileNum, sizeof(int), 1, f);
	fwrite(fileBuffer, sizeof(fileExchangeInfo), fileNum, f);
	for(int i=1; i<=fileNum; ++i)
	{
		fwrite(files[i]->fileName, sizeof(char), fileBuffer[i].fileNameLen, f);
		fwrite(files[i]->tags,     sizeof(int),  fileBuffer[i].tagNum,      f);
		fwrite(files[i]->filePath, sizeof(char), fileBuffer[i].filePathLen, f);
	}
}

void WriteSavFile(FILE* f)
{
	writeTags(f);
	writeFolders(f);
	writeFiles(f);
	fwrite(&userNameLen, sizeof(int), 1, f);
	fwrite(userName, sizeof(char), userNameLen, f);
	fwrite(&currentFile, sizeof(int), 1, f);
}

void ClearBuilded()
{
	FreeDictionaryFolder(fullFolder);
} 

/////////////////////////////////////// 遍历访问函数  ///////////////////////////////////// 
void BrowseExplorer(FileBrowseFunc func, int silentp)
{
	for(int i=1; i<=fileNum; ++i)
	{
		int curPage = fileOnBlockListPage[i];
		int flag = 0;
		if(!curPage && !silentp)
		{
			curPage = openFile(i);
			flag = 1;
		}
		func(files[i], curPage);
		if(flag) closeFile(i);
	}
}

static void filterFolder(FileFilterFunc func, DictionaryFolder* df, int silentp)
{
	if(df->items)
	{
		DictionaryItem* it = df->items;
		while(it)
		{
			int curPage = fileOnBlockListPage[it->itemID];
			int flag = 0;
			if(!curPage && !silentp)
			{
				curPage = openFile(it->itemID);
				flag = 1;
			}
			if(func(files[it->itemID], curPage))
			{
				if(flag) closeFile(it->itemID);
				DictionaryItem* tmp = it;
				it = it->nextItem;
				FreeDictionaryItem(tmp);
			}
			else
			{
				if(flag) closeFile(it->itemID);
				it = it->nextItem;
			}
		}
	}
	
	if(df->subFolders)
	{
		DictionaryFolder* it = df->subFolders;
		while(it)
		{
			filterFolder(func, it, silentp);
			if(!it->subFolders && !it->items)
			{
				DictionaryFolder* tmp = it;
				it = it->nextFolder;
				FreeDictionaryFolder(tmp);
			}
		}
	}
}

DictionaryFolder* FilterExplorer(FileFilterFunc func, int silentp)
{
	const DictionaryFolder* newFolder = CopyDictionaryFolder(fullFolder);
	filterFolder(func, newFolder, silentp);
	return newFolder;
}

////////////////////////////////////////////////// 显示与交互  //////////////////////////////////// 
static DictionaryGraphicDatas* showDGD;
static double lastWidth;

static void initializeDGD()
{
	showDGD = CreateDictionaryGraphicDatas();
}

void DrawExplorer(double cx, double cy, double width, double height)
{
	if(!showDGD) initializeDGD();
	lastWidth = width;
	DrawDictionaryList(showDGD, fullFolder, cx, cy, width, 0, height);
}

static DictionaryCursor lastClickedPosition;
static DictionaryCursor currentClickedPosition;

void ExplorerLeftMouseDown(double mx, double my)
{
	if(!lastClickedPosition)
	{
		lastClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth, mx, my);
		if(lastClickedPosition.pointEntryType)
		{
			if(lastClickedPosition.pointEntryType==1)
			{
				(lastClickedPosition.pointFolder)->folderEmphasizeType = 3;
			}
			else
			{
				(lastClickedPosition.pointItem)->itemEmphasizeType = 3;
			}
		}
		return;
	}
	else currentClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth,mx, my);
}

void ExplorerLeftMouseUp()
{
	if(lastClickedPosition == currentClickedPosition) //判定为单击
	{
		if(!lastClickedPosition.pointEntryType) return;
		if(lastClickedPosition.pointEntryType == 1)
		{
			(lastClickedPosition.pointFolder)->folderExpended = !(lastClickedPosition.pointFolder)->folderExpended;
			(lastClickedPosition.pointFolder)->folderEmphasizeType = 0;
		}
		else
		{
			openFile((lastClickedPosition.pointItem)->itemID);
			changeCurrentFileByID((lastClickedPosition.pointItem)->itemID);
		}
	}
	else
	{
		if(!lastClickedPosition.pointEntryType) return;
		if(!currentClickedPosition.folderIn) currentClickedPosition.folderIn = fullFolder;
		if(lastClickedPosition.pointEntryType == 1)
		{
			DictionaryFolder* a = lastClickedPosition.pointFolder;
			if(a->parent->subFolders == a)
			{
				a->parent->subFolders = 0;
				if(a->prevFolder) a->parent->subFolders = a->prevFolder;
				if(a->nextFolder) a->parent->subFolders = a->nextFolder;
			}
			if(a->prevFolder) a->prevFolder->nextFolder = a->nextFolder;
			if(a->nextFolder) a->nextFolder->prevFolder = a->prevFolder;

			DictionaryFolder* b = currentClickedPosition.folderIn;
			a->parent = b;
			if(!b->subFolders)
			{
				b->subFolders = a;
			}
			else
			{
				a->prevFolder = 0;
				a->nextFolder = b->subFolders;
				b->subFolders->prevFolder = a;
				b->subFolders = a;
			}

			folders[a->folderID]->parentFolder = folders[b->folderID];
		}
		else
		{
			DictionaryItem* a = lastClickedPosition.pointItem;
			if(a->folder->items == a)
			{
				a->folder->items = 0;
				if(a->prevItem) a->folder->items = a->prevItem;
				if(a->nextItem) a->folder->items = a->nextItem;
			}
			if(a->prevItem) a->prevItem->nextItem = a->nextItem;
			if(a->nextItem) a->nextItem->prevItem = a->prevItem;

			DictionaryItem* b = currentClickedPosition.folderIn;
			a->folder = b;
			if(!b->items)
			{
				b->items = a;
			}
			else
			{
				a->prevItem = 0;
				a->nextItem = b->items;
				b->items->prevItem = a;
				b->items = a;
			}

			files[a->fileID]->folder = folders[b->folderID];
		}
	}
}

void ExplorerRightMouseDown(double mx, double my)
{
	if(!lastClickedPosition)
	{
		lastClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth, mx, my);
		if(lastClickedPosition.pointEntryType)
		{
			if(lastClickedPosition.pointEntryType==1)
			{
				(lastClickedPosition.pointFolder)->folderEmphasizeType = 3;
			}
			else
			{
				(lastClickedPosition.pointItem)->itemEmphasizeType = 3;
			}
		}
		return;
	}
	else currentClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth,mx, my);
}

static int renameType;
static int renameID;
static void renameCallback(const char* renameValue)
{
	if(!renameValue) return;
	if(renameType == 1)
	{
		strcpy(folders[renameID]->folderName, renameValue);
		folders[renameID].folderNameLen = strlen(renameValue);
		strcpy(corresDFolder[renameID]->folderName, renameValue);
	}
	else
	{
		strcpy(files[renameID]->fileName, renameValue);
		files[renameID].fileNameLen = strlen(renameValue);
		strcpy(corresDFile[renameID]->fileName, renameValue);
	}
}

void ExplorerRightMouseUp()
{
	if(lastClickedPosition == currentClickedPosition) //判定为单击
	{
		if(!lastClickedPosition.pointEntryType) return;
		renameType = lastClickedPosition.pointEntryType;
		renameID = lastClickedPosition.pointEntryType == 1          ? 
						(lastClickedPosition.pointFolder)->folderID :
						(lastClickedPosition.pointItem)->itemID；
		ChangeDisplayMethodToMajorInput(renameCallback);
	}
	else //右键拖拽，为删除
	{
		//TODO
	}
}

//或许可以增加一个右键双击更改路径的功能
////////////////////////////////////////////// 其他由 explorer 负责的杂类函数 //////////////////////////

void SaveCurrentFile()
{
	SaveFileAtPage(fileOnBlockListPage[currentFile], files[currentFile]->filePath);
}