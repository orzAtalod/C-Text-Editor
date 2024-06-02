#ifndef __EXPLORER_CORE_H_INCLUDED__
#define __EXPLORER_CORE_H_INCLUDED__

typedef struct fhBase {
	int folderID;
	char* folderName;
	struct fhBase* parentFolder;
} FolderHeaderInfo;

typedef struct {
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

typedef void (*FileBrowseFunc)(FileHeaderInfo*, int); //int 参数为 blocklist 可接受的 page 值 
typedef bool (*FileFilterFunc)(FileHeaderInfo*, int);

void              BrowseExplorer(FileBrowseFunc, int silentp); //silentp==1 时，遍历文件时不打开文件 
DictionaryFolder* FilterExplorer(FileFilterFunc, int silentp); //FileFilterFunc 返回1时删除文件 

#endif
