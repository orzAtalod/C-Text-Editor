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

typedef void (*FileBrowseFunc)(FileHeaderInfo*, int); //int ����Ϊ blocklist �ɽ��ܵ� page
typedef int  (*FileFilterFunc)(FileHeaderInfo*, int);

void              BrowseExplorer(FileBrowseFunc func, int silentp); //silentp==1 ʱ�������ļ�ʱ�����ļ� 
DictionaryFolder* FilterExplorer(FileFilterFunc func, int silentp); //FileFilterFunc ����1ʱɾ���ļ� 

void SaveCurrentFile();
int CreateEmptyFile(); //�½�һҳ�������½�����һҳ
void ReloadCurrentFile();
void CloseCurrentFile();
FileHeaderInfo* GetCurrentFileHeaderInfo();

void ExploreCoreInitCallBacks(); 

#endif
