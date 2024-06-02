#ifndef __EDITOR_CORE_H_INCLUDED__
#define __EDITOR_CORE_H_INCLUDED__

#include "blockList.h"

/*
*
* ά����������ı��༭���� 
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
void KeyboardInputSpecial(char keyInputType); //1�� 2�� 3�� 4�� 5�س� 6�˸� 7Delete
void NewParagraph();

#endif
