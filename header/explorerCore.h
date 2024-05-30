#ifndef __EXPLORER_CORE_H_INCLUDED__
#define __EXPLORER_CORE_H_INCLUDED__

typedef struct fhBase {
	int folderID;
	const char* folderName;
	struct fhBase* parentFolder;
} FolderHeaderInfo;

typedef struct {
	int tagNum;
	int editTime;
	int* tags;
	const char* fileName;
	FolderHeaderInfo* folder;
} FileHeaderInfo;

void ReadSavFile(FILE*);
void BuildFiles();
void WriteSavFile(FILE*);
void ClearBuilded();

typedef void (*FileBrowseFunc)(FileHeaderInfo*, int); //int 参数为 blocklist 可接受的 page 值 

#endif
