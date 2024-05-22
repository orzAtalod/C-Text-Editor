#ifndef __FILE_SYSTEM_H_INCLUDED__
#define __FILE_SYSTEM_H_INCLUDED__

void InitFileSystem(ColorDefinitionFunction func);
void UpdateColorDefinitionMethod(ColorDifinitionFunction func);
void ChangePageAtFileSystem(int page);
void LoadFileAtPage(int page, const char* fileName);
void SaveFileAtPage(int page, const char* fileName);
void LoadFile(const char* fileName);
void SaveFile(const char* fileName);

#endif
