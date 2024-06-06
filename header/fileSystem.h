#ifndef __FILE_SYSTEM_H_INCLUDED__
#define __FILE_SYSTEM_H_INCLUDED__
#include "fileSystemCore.h"

void InitFileSystem(ColorDefinitionFunction func);
void UpdateColorDefinitionMethod(ColorDefinitionFunction func);
void ChangePageAtFileSystem(int page);
void LoadFileAtPage(int page, const char* fileName);
void SaveFileAtPage(int page, const char* fileName);
void LoadFile(const char* fileName);
void SaveFile(const char* fileName);
void ClearAllItemsOnPage(int page);

void RegisterSavFilePath(const char* savFilePath);
void LoadSavFile();
void StoreSavFile();

#endif
