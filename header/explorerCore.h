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

typedef void (*FileBrowseFunc)(FileHeaderInfo*, int); //int ����Ϊ blocklist �ɽ��ܵ� page ֵ 
typedef bool (*FileFilterFunc)(FileHeaderInfo*, int);

void              BrowseExplorer(FileBrowseFunc, int silentp); //silentp==1 ʱ�������ļ�ʱ�����ļ� 
DictionaryFolder* FilterExplorer(FileFilterFunc, int silentp); //FileFilterFunc ����1ʱɾ���ļ� 

#endif
