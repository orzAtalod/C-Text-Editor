#include "explorerCore.h"
#include "dictionaryList.h"
#include "blockList.h"
#include "editorCore.h"
#include "fileSystem.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "myGUI.h"
#include "controller.h"
#define NEW(x)  ((x*)malloc(sizeof(x)))
#define NEWVARR(x,len) ((x*)calloc((len),(sizeof(x)))) 

static char* tags[65535];                 static int tagStoreNum;
static FolderHeaderInfo* folders[65535];  static int folderNum;
static FileHeaderInfo*   files[65535];    static int fileNum;
static int fileOnBlockListPage[65535];    static int fileOnEditNum;
static char userName[255];                static int userNameLen;
static int currentFile;

static DictionaryFolder* corresDFolder[65535];
static DictionaryItem*   corresDFile[65535];
static DictionaryFolder* fullFolder;
static int curTime;

///////////////////////////////////////////////// 基础函数 //////////////////////////////////////
void RefreshEditTime()
{
	const int x = GetClock();
	if(currentFile)
	{
		files[currentFile]->editTime += x-curTime;
	}
	curTime = x;
}

static void changeCurrentFileByID(int newFile)
{
	RefreshEditTime();
	if(currentFile)
	{
		corresDFile[currentFile]->itemEmphasizeType = 1;
	}
	currentFile = newFile;
	corresDFile[currentFile]->itemEmphasizeType = 2;
	ChangePageOfEditorCore(fileOnBlockListPage[newFile]);
}

static int silentOpenFile(int fileID)
{
	if(!fileID) return 0;
	if(fileOnBlockListPage[fileID])
	{
		return fileOnBlockListPage[fileID];
	}
	
	StoreCurPage();
	++fileOnEditNum;
	fileOnBlockListPage[fileID] = fileOnEditNum;
	LoadFileAtPage(fileOnEditNum, files[fileID]->filePath);
	RecoverCurPage();
	
	return fileOnEditNum;
}

static int openFile(int fileID) //返回打开页码
{
	const int res = silentOpenFile(fileID);
	changeCurrentFileByID(fileID);
	return res;
}

static void closeFile(int fileID)
{
	assert(currentFile != fileID);
	corresDFile[fileID]->itemEmphasizeType = 0;
	if(fileOnBlockListPage[fileID] == fileOnEditNum) --fileOnEditNum;
	fileOnBlockListPage[fileID] = 0;
}

void ReloadCurrentFile()
{
	if(currentFile) ChangePageOfEditorCore(fileOnBlockListPage[currentFile]);
}

FileHeaderInfo* GetCurrentFileHeaderInfo()
{
	if(currentFile) return files[currentFile];
	else return 0;
}

int CreateEmptyFile()  //新建一页，返回新建的那一页
{
	files[++fileNum] = (FileHeaderInfo*)malloc(sizeof(FileHeaderInfo));
	files[fileNum]->editTime = 0;
	files[fileNum]->tagNum = 0;
	files[fileNum]->tags = NEWVARR(int,5);
	files[fileNum]->fileName = NEWVARR(char,20);
	strcpy(files[fileNum]->fileName, "Noname");
	files[fileNum]->filePath = NEWVARR(char,20);
	files[fileNum]->folder = 0;
	files[fileNum]->fileID = fileNum;

	DictionaryItem* newFileDic = CreateDictionaryItem();
	newFileDic->itemID = fileNum;
	newFileDic->folder = fullFolder;
	newFileDic->nextItem = fullFolder->items;
	if(fullFolder->items) fullFolder->items->prevItem = newFileDic;
	fullFolder->items = newFileDic;
	corresDFile[fileNum] = newFileDic;

	++fileOnEditNum;
	fileOnBlockListPage[fileNum] = fileOnEditNum;
	ClearAllItemsOnPage(fileOnEditNum);
	changeCurrentFileByID(fileNum);

	ChangePageOfFontTable(fileOnEditNum);
	ChangePageOfColorTable(fileOnEditNum);
	RegisterFontTable("default");
	RegisterFontTable("Times");
	RegisterColorTable("Black");
	return fileOnEditNum;
}

static void createEmptyFolder()
{
	folders[++folderNum] = (FolderHeaderInfo*)malloc(sizeof(FolderHeaderInfo));
	folders[folderNum]->folderID = folderNum;
	folders[folderNum]->folderName = (char*)malloc(20*sizeof(char));
	strcpy(folders[folderNum]->folderName, "Noname");
	folders[folderNum]->parentFolder = 0;

	DictionaryFolder* newFolder = CreateDictionaryFolder();
	newFolder->folderID = folderNum;
	newFolder->parent = fullFolder;
	newFolder->nextFolder = fullFolder->subFolders;
	if(fullFolder->subFolders) fullFolder->subFolders->prevFolder = newFolder;
	fullFolder->subFolders = newFolder;
	corresDFolder[folderNum] = newFolder;
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
	fread(folderBuffer+1,sizeof(folderExchangeInfo),folderNum,f);
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
	fread(fileBuffer+1, sizeof(fileExchangeInfo), fileNum, f);
	for(int i=1; i<=fileNum; ++i)
	{
		files[i] = NEW(FileHeaderInfo);
		files[i]->editTime = fileBuffer[i].editTime;
		files[i]->tagNum = fileBuffer[i].tagNum;
		files[i]->folder = folders[fileBuffer[i].folderID];
		files[i]->fileName = NEWVARR(char, fileBuffer[i].fileNameLen+5);
		files[i]->tags = NEWVARR(int, fileBuffer[i].tagNum+5);
		files[i]->filePath = NEWVARR(char, fileBuffer[i].filePathLen+5);
		files[i]->fileID = i;
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
	fullFolder->parent = 0; 

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

	for(int i=1; i<=fileNum; ++i)
	{
		DictionaryItem* now = CreateDictionaryItem();
		strcpy(now->itemName, files[i]->fileName);
		now->itemID = i;
		now->folder = files[i]->folder ? corresDFolder[files[i]->folder->folderID] : fullFolder;
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
		corresDFile[i] = now;
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
	
	if(fullFolder->subFolders)
	{
		while(fullFolder->subFolders->prevFolder)
		{
			fullFolder->subFolders = fullFolder->subFolders->prevFolder;
		}
	}
	if(fullFolder->items)
	{
		while(fullFolder->items->prevItem)
		{
			fullFolder->items = fullFolder->items->prevItem;
		}
	}

	if(currentFile)
	{
		openFile(currentFile);
		changeCurrentFileByID(currentFile);
	}
	else
	{
		CreateEmptyFile();
	}
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
	fwrite(folderBuffer+1,sizeof(folderExchangeInfo),folderNum,f);
	
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
		fileBuffer[i].folderID = files[i]->folder ? files[i]->folder->folderID : 0;
		fileBuffer[i].tagNum = files[i]->tagNum;
	}

	fwrite(&fileNum, sizeof(int), 1, f);
	fwrite(fileBuffer+1, sizeof(fileExchangeInfo), fileNum, f);
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
	userNameLen = strlen(fullFolder->folderName);
	strcpy(userName, fullFolder->folderName);
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
			curPage = silentOpenFile(i);
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
			if(!func(files[it->itemID], curPage))
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
	DictionaryFolder* newFolder = CopyDictionaryFolder(fullFolder);
	filterFolder(func, newFolder, silentp);
	return newFolder;
}

////////////////////////////////////////////////// tags ///////////////////////////////////////////
int GetTagNum() { return tagStoreNum; }
char** GetTags() { return tags; }

void AddtagToCurFile(const char* tagName)
{
	int tagID = 0;
	for(int i=1; i<=tagStoreNum; ++i)
	{
		if(!strcmp(tags[i], tagName))
		{
			tagID = i;
			break;
		}
	}
	if(!tagID)
	{
		++tagStoreNum;
		const int tagNameLen = strlen(tagName);
		tags[tagStoreNum] = (char*)malloc((tagNameLen+5)*sizeof(char));
		strcpy(tags[tagStoreNum], tagName);
		tagID = tagStoreNum;
	}
	if(currentFile)
	{
		int  tagsLen = files[currentFile]->tagNum;
		int* tagsArr = files[currentFile]->tags;
		for(int i=1; i<=tagsLen; ++i)
		{
			if(tagsArr[i] == tagID)
			{
				for(int j=i+1; j<=tagsLen; ++j)
				{
					tagsArr[j-1] = tagsArr[j];
				}
				--files[currentFile]->tagNum;
				return;
			}
		}
		files[currentFile]->tags = realloc(files[currentFile]->tags, (tagsLen+6)*sizeof(int));
		files[currentFile]->tags[tagsLen+1] = tagID;
		++files[currentFile]->tagNum;
	}
}

char tagBuffer[255];
const char* ShowTags()
{
	tagBuffer[0] = '\0'; 
	int* tagsArr = files[currentFile]->tags;
	int  tagsLen = files[currentFile]->tagNum;
	int  index = 0;
	for(int i=1; i<=tagsLen; ++i)
	{
		tagBuffer[index] = '#';
		strcpy(tagBuffer+index+1, tags[tagsArr[i]]);
		index += strlen(tags[tagsArr[i]])+2;
		tagBuffer[index-1] = ' ';
		tagBuffer[index] = '\0';
	}
	return tagBuffer;
}

////////////////////////////////////////////////// 显示与交互  //////////////////////////////////// 
static DictionaryGraphicDatas* showDGD;
static double lastWidth;

static void initializeDGD()
{
	showDGD = CreateDictionaryGraphicDatas();
}

void DrawExplorer(double cx, double cy, double dx, double dy)
{
	if(!showDGD) initializeDGD();
	lastWidth = dx-cx;
	DrawDictionaryList(showDGD, fullFolder, cx, cy, dx-cx, 0, dy-cy);
}

static int lastClicked;
static int currentClicked;
static DictionaryCursor lastClickedPosition;
static DictionaryCursor currentClickedPosition;
static int lastOriEmph;

void ExplorerLeftMouseDown(double mx, double my)
{
	if(!lastClicked)
	{
		lastClicked = 1;
		lastClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth, mx, my);
		if(lastClickedPosition.pointEntryType)
		{
			if(lastClickedPosition.pointEntryType==1)
			{
				lastOriEmph = (lastClickedPosition.pointFolder)->folderEmphasizeType; 
				(lastClickedPosition.pointFolder)->folderEmphasizeType = 3;
			}
			else
			{
				lastOriEmph = (lastClickedPosition.pointItem)->itemEmphasizeType;
				(lastClickedPosition.pointItem)->itemEmphasizeType = 3;
			}
		}
		return;
	}
	else
	{
		currentClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth,mx, my);
		currentClicked = 1;
	}
}

static int folderSamePosition()
{
	return lastClickedPosition.pointFolder==currentClickedPosition.pointFolder && lastClickedPosition.pointItem==currentClickedPosition.pointItem;
}

void ExplorerLeftMouseUp()
{
	if(!lastClicked) return;
	if((lastClicked&&!currentClicked) || folderSamePosition()) //判定为单击
	{
		lastClicked = currentClicked = 0;
		if(!lastClickedPosition.pointEntryType) return;
		if(lastClickedPosition.pointEntryType == 1)
		{
			(lastClickedPosition.pointFolder)->folderEmphasizeType = lastOriEmph;
			(lastClickedPosition.pointFolder)->folderExpended = !(lastClickedPosition.pointFolder)->folderExpended;
		}
		else
		{
			lastClickedPosition.pointItem->itemEmphasizeType = lastOriEmph;
			openFile((lastClickedPosition.pointItem)->itemID);
			changeCurrentFileByID((lastClickedPosition.pointItem)->itemID);
		}
	}
	else
	{
		lastClicked = currentClicked = 0;
		if(!lastClickedPosition.pointEntryType) return;
		if(!currentClickedPosition.folderIn) currentClickedPosition.folderIn = fullFolder;
		if(lastClickedPosition.pointEntryType == 1)
		{
			(lastClickedPosition.pointFolder)->folderEmphasizeType = lastOriEmph;
			DictionaryFolder* a = lastClickedPosition.pointFolder;
			if(a == currentClickedPosition.folderIn) return;
			if(a->parent->subFolders == a)
			{
				a->parent->subFolders = 0;
				if(a->prevFolder) a->parent->subFolders = a->prevFolder;
				if(a->nextFolder) a->parent->subFolders = a->nextFolder;
			}
			if(a->prevFolder) a->prevFolder->nextFolder = a->nextFolder;
			if(a->nextFolder) a->nextFolder->prevFolder = a->prevFolder;
			a->nextFolder = a->prevFolder = 0;

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
			lastClickedPosition.pointItem->itemEmphasizeType = lastOriEmph;			
			DictionaryItem* a = lastClickedPosition.pointItem;
			if(a->folder->items == a)
			{
				a->folder->items = 0;
				if(a->prevItem) a->folder->items = a->prevItem;
				if(a->nextItem) a->folder->items = a->nextItem;
			}
			if(a->prevItem) a->prevItem->nextItem = a->nextItem;
			if(a->nextItem) a->nextItem->prevItem = a->prevItem;
			a->nextItem = a->prevItem = 0;

			DictionaryFolder* b = currentClickedPosition.folderIn;
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

			files[a->itemID]->folder = folders[b->folderID];
		}
	}
}

void ExplorerRightMouseDown(double mx, double my)
{
	if(!lastClicked)
	{
		lastClicked = 1;
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
	else
	{
		currentClickedPosition = PositionizeDictionaryList(showDGD, fullFolder, lastWidth,mx, my);
		currentClicked = 1;
	}
}

static void setName(char* str, const char* renameV, int renameN)
{
	str = realloc(str, (renameN+5)*sizeof(char));
	strcpy(str, renameV);
}

static int renameType;
static int renameID;
static void renameCallback(const char* renameValue)
{
	if(!renameValue) return;
	const int renameLen = strlen(renameValue);
	if(renameType == 1)
	{
		if(renameID)
		{
			setName(folders[renameID]->folderName, renameValue, renameLen);
			setName(corresDFolder[renameID]->folderName, renameValue, renameLen);
		}
		else
		{
			setName(fullFolder->folderName, renameValue, renameLen);
		}
	}
	else
	{
		setName(files[renameID]->fileName, renameValue, renameLen);
		setName(corresDFile[renameID]->itemName, renameValue, renameLen);
	}
}

void ExplorerRightMouseUp()
{
	if(!lastClicked) return;
	if((lastClicked&&!currentClicked) || folderSamePosition()) //判定为单击
	{
		lastClicked = currentClicked = 0;
		if(!lastClickedPosition.pointEntryType)
		{
			createEmptyFolder();
		}
		else
		{
			renameType = lastClickedPosition.pointEntryType;
			renameID = lastClickedPosition.pointEntryType == 1          ? 
							(lastClickedPosition.pointFolder)->folderID :
							(lastClickedPosition.pointItem)->itemID; 
			ChangeDisplayMethodToMajorInput("重命名为：",renameCallback);
		}
	}
	else //右键拖拽，为删除
	{
		//TODO
	}
	lastClicked = currentClicked = 0;
}

//或许可以增加一个右键双击更改路径的功能
////////////////////////////////////////////// 其他由 explorer 负责的杂类函数 //////////////////////////

void SaveCurrentFile()
{
	RefreshEditTime();
	if(currentFile) SaveFileAtPage(fileOnBlockListPage[currentFile], files[currentFile]->filePath);
}

void CloseCurrentFile()
{
	const int nowCurr = currentFile;
	if(currentFile)
	{
		for(int i=1; i<=fileNum; ++i)
		{
			if(i!=currentFile && fileOnBlockListPage[i])
			{
				changeCurrentFileByID(i);
				break;
			}
		}
		if(currentFile == nowCurr)
		{
			ExitProgram();
		}
		closeFile(nowCurr);
	}
}

void ExploreCoreInitCallBacks()
{
	RegisterExplorerMouseLeftDown(ExplorerLeftMouseDown);
	RegisterExplorerMouseLeftUp(ExplorerLeftMouseUp);
	RegisterExplorerMouseRightDown(ExplorerRightMouseDown);
	RegisterExplorerMouseRightUp(ExplorerRightMouseUp);
	RegisterExplorerDraw(DrawExplorer);
} 
