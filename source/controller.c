#include "myGUI.h"
#include "editorCore.h"
#include "explorerCore.h"
#include "windows.h"
#include "fileSystem.h" 
#include "search.h" 
//////////////////////////////////////////////// �˵������� ////////////////////////////
//�˵���Ӧ����
/*
static char *fileMenu[]  = {"File", "Save", "Open", "New", "Save As", "Close"};
static char *toolsMenu[] = {"Tools", "Stats", "Search", "Browse"};
static char *aboutMenu[] = {"About", "Settings", "Help"};
*/

static void openFilePath(const char*);
static void emptyFunc(const char* str) {;}

static void save() //Ctrl+S
{
	SaveCurrentFile();
	StoreSavFile();
}

static void setPath(FileHeaderInfo* fh, const char* path)
{
	if(!fh) return;
	int pathlen = strlen(path);
	free(fh->filePath);
	fh->filePath = (char*)malloc((pathlen+5)*sizeof(char));
	strcpy(fh->filePath, path);
}

static void open() //Ctrl+O
{
	ChangeDisplayMethodToMajorInput("������Ҫ�򿪵��ļ�·����",openFilePath);
}

static void openFilePath(const char* path)
{
	if(!path) return;
	FILE* f = fopen(path,"rb");
	if(!f)
	{
		ChangeDisplayMethodToMajorInput("�ļ������ڣ�", emptyFunc);
		return;
	}
	fclose(f);
	const int page = CreateEmptyFile();
	LoadFileAtPage(page, path);
	ReloadCurrentFile();
	setPath(GetCurrentFileHeaderInfo(), path);
	StoreSavFile();
}

char tmp[105];

static void newFile() //Ctrl+N
{
	CreateEmptyFile();
	sprintf(tmp,"note%04d.note",rand()%1000);
	setPath(GetCurrentFileHeaderInfo(), tmp);
	StoreSavFile();
}

static void saveAsFilePath(const char* path)
{
	if(!path) return;
	FileHeaderInfo* cf = GetCurrentFileHeaderInfo();
	if(!cf) ChangeDisplayMethodToMajorInput("��ǰ���ļ���", emptyFunc);
	else { setPath(GetCurrentFileHeaderInfo(), path); SaveCurrentFile(); StoreSavFile(); }
}

static void saveAs()
{
	ChangeDisplayMethodToMajorInput("�������Ϊ·�������ļ�����", saveAsFilePath);
}

static void close()
{
	SaveCurrentFile();
	CloseCurrentFile();
}

static void stats()
{

}

static void search()
{
	ChangeDisplayMethodToSearch(SearchString);
}

static void browse()
{

}

static void settings()
{

}

static void help()
{
	system("help.txt");
}

void ControllerInitCallbacks()
{
	RegisterSaveMethod(save);
	RegisterOpenMethod(open);
	RegisterCreateMethod(newFile);
	RegisterSaveasMethod(saveAs);
	RegisterCloseMethod(close);

	RegisterStatMethod(stats);
	RegisterSearchMethod(search);
	RegisterExploreMethod(browse);

	RegisterSettingMethod(settings);
	RegisterHelpMethod(help);

	RegisterSearchDisplayMethod(SearchDraw);
}
