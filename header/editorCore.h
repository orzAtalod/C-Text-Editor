#ifndef __EDITOR_CORE_H_INCLUDED__
#define __EDITOR_CORE_H_INCLUDED__

#include "blockList.h"

/*
*
* 维护光标和完成文本编辑操作 
*
*/

void ChangePageOfEditor(int p);
void MouseDown(Block* blk, double x, double y);
void MouseUp();
void Bold();
void Italic();
void FontChange(const char* newFont);
void ColorChange(const char* newColor);
void KeyboardInput(char ch);
void KeyboardInputSpecial(char keyInputType); //1上 2下 3左 4右 5回车 6退格 7Delete
void NewParagraph();

#endif
