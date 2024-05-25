#ifndef __EDITOR_CORE_H_INCLUDED__
#define __EDITOR_CORE_H_INCLUDED__

#include "blockList.h"

/*
*
* 维护光标和完成文本编辑操作 
*
*/

#define KEYBOARD_FOCUS_DOWN  1
#define KEYBOARD_FOCUS_UP    2
#define KEYBOARD_FOCUS_LEFT  3
#define KEYBOARD_FOCUS_RIGHT 4
#define KEYBOARD_FOCUS_TAB   5
void MouseDown(Block* blk, double x, double y);
void MouseUp();
void Bold();
void Italic();
void FontChange(const char* newFont);
void ColorChange(const char* newColor);
void KeyboardFocus(int keyInputType);
void KeyboardInput(char ch);
void NewParagraph();

#endif
