#include "myGUI.h"
#include "editorCore.h"
#include "explorerCore.h"
#include "windows.h"
//////////////////////////////////////////////// 菜单栏操作 ////////////////////////////
//菜单响应函数
/*
static char *fileMenu[]  = {"File", "Save", "Open", "New", "Save As", "Close"};
static char *toolsMenu[] = {"Tools", "Stats", "Search", "Browse"};
static char *aboutMenu[] = {"About", "Settings", "Help"};
*/

static void openFilePath;
static void emptyFunc(const char* str) {;}

static void save() //Ctrl+S
{
	SaveCurrentFile();
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
	ChangeDisplayMethodToMajorInput("输入需要打开的文件路径：",openFilePath);
}

static void openFilePath(const char* path)
{
	if(!path) return;
	FILE* f = fopen(path,"rb");
	if(!f)
	{
		ChangeDisplayMethodToMajorInput("文件不存在！", emptyFunc);
		return;
	}
	fclose(f);
	const int page = CreateEmptyFile();
	LoadFileAtPage(page, path);
	ReloadCurrentFile();
	setPath(GetCurrentFileHeaderInfo(), path);
}

char tmp[105];

static void newFile() //Ctrl+N
{
	CreateEmptyFile();
	sprintf(tmp,"note%04d.note",rand()%1000);
	setPath(GetCurrentFileHeaderInfo(), tmp);
}

static void saveAsFilePath(const char* path)
{
	if(!path) return;
	FileHeaderInfo* cf = GetCurrentFileHeaderInfo();
	if(!cf) ChangeDisplayMethodToMajorInput("当前无文件！", emptyFunc);
	else { setPath(GetCurrentFileHeaderInfo(), path); SaveCurrentFile(); }
}

static void saveAs()
{
	ChangeDisplayMethodToMajorInput("输入另存为路径（含文件名）", saveAsFilePath);
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

void Error()
{

}