#ifndef __SEARCH_H_INCLUDED__
#define __SEARCH_H_INCLUDED__

//callbacks will be initialized at controller.c
void SearchBuild(); //must be called after explorer has been built (that is, after ReadSavFile() has been called)
void SearchString(const char* searchFunc);
void SearchDraw(double, double, double, double);

#endif