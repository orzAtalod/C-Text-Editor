#ifndef __EXPLORER_CORE_H_INCLUDED__
#define __EXPLORER_CORE_H_INCLUDED__
#include <stdio.h>
#include "dictionaryList.h"

typedef struct fhBase {
	int folderID;
	char* folderName;
	struct fhBase* parentFolder;
} FolderHeaderInfo;

typedef struct {
	int fileID;
	int tagNum;
	int editTime;
	int* tags;
	char* fileName;
	char* filePath;
	FolderHeaderInfo* folder;
} FileHeaderInfo;

void ReadSavFile(FILE*);
void BuildFiles();
void WriteSavFile(FILE*);
void ClearBuilded();
void BuildFiles();

typedef void (*FileBrowseFunc)(FileHeaderInfo*, int); //int 参数为 blocklist 可接受的 page
typedef int  (*FileFilterFunc)(FileHeaderInfo*, int);

void              BrowseExplorer(FileBrowseFunc func, int silentp); //silentp==1 时，遍历文件时不打开文件 
DictionaryFolder* FilterExplorer(FileFilterFunc func, int silentp); //FileFilterFunc 返回1时删除文件 

void SaveCurrentFile();
int CreateEmptyFile(); //新建一页，返回新建的那一页
void ReloadCurrentFile();
void CloseCurrentFile();
FileHeaderInfo* GetCurrentFileHeaderInfo();

void ExploreCoreInitCallBacks(); 

#endif
