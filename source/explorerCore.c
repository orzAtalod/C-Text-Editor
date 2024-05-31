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
		folders[i] = NEW(FolderHeaderInfo);
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
	readFiles(f);
	readFolders(f);
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
	
	if(currentFile)
	{
		fileOnEditNum = 1;
		fileOnBlockListPage[currentFile] = 1;
		UpdateColorDefinitionMethod(GetEditCoreColorDefMethod());
		LoadFileAtPage(1,files[currentFile]->filePath);
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

void ClearBuilded()
{
	FreeDictionaryFolder(fullFolder);
} 

/////////////////////////////////////// 遍历访问函数  ///////////////////////////////////// 
void silentColordef(const char* str, double r, double g, double b)
{
	return;
} 

void BrowseExplorer(FileBrowseFunc func, int silentp)
{
	const int currentBlockListPage = GetPageOfBlockList();
	for(int i=1; i<=fileNum; ++i)
	{
		int curPage = fileOnBlockListPage[i];
		if(!curPage && !silentp)
		{
			curPage = fileOnEditNum+1;
			UpdateColorDefinitionMethod(silentColordef);
			LoadFileAtPage(curPage,files[currentFile]->filePath);
		}
		func(files[i], curPage);
	}
	ChangePageOfBlockList(currentBlockListPage);
}

// TODO
DictionaryFolder* FilterExplorer(FileFilterFunc func, int silentp)
{
	const DictionaryFolder* newFolder = CopyDictionaryFolder(fullFolder);	
}
