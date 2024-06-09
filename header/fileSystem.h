#ifndef __FILE_SYSTEM_H_INCLUDED__
#define __FILE_SYSTEM_H_INCLUDED__
#include "fileSystemCore.h"

void InitFileSystem();
void ChangePageAtFileSystem(int page);
int  LoadFileAtPage(int page, const char* fileName); //return 1 when failed
void SaveFileAtPage(int page, const char* fileName);
void LoadFile(const char* fileName);
void SaveFile(const char* fileName);
void ClearAllItemsOnPage(int page);

void RegisterSavFilePath(const char* savFilePath);
void LoadSavFile();
void StoreSavFile();

void RecoverCurPage();
void StoreCurPage();

#endif
