#ifndef __FILE_SYSTEM_CORE_H__
#define __FILE_SYSTEM_CORE_H__
#include <stdio.h>

/*
*
* Color Table数据结构 
* 分为两个部分，第一个部分从char*映射到double[3]，用于定义颜色；第二个部分
*     从int映射到char*，用于读入Color数据（建议采用char*数组） 
* 支持页操作，保存至多255页数据，通过ChangePageOfColorTable切换，不同页数
*     据互不干扰，相当于不同文件，其中id=0的Page意为不在内存中存取任何数
*     据（Input类函数仍旧需要移动文件指针光标位置, Output类函数需报错）
* 报错可使用<assert.h>，见附录assert.h使用例程 
* 所有文件均为二进制文件 
* 支持以下功能：
*
* 0. void ChangePageOfColorTable(int p)
* 修改当前页
*
* 1. void ReadColorTable(FILE* f)
* 读入一个Color Table，替换当前页的Color Table，需读入从char*映射成double[3]
*     和从int映射到char*两个部分，同时对于第一个部分的每一项调用func进行颜色
*     注册 
* 更改f的光标位置至下一块数据起始处（自然读入即可，不要多读） 
*
* 2. void WriteColorTable(FILE* f) 
* 与Read函数配合，需写出两个部分的Color Table到f，并相应更改光标位置
*
* 3. const char* LookupIDInColorTable(int id) 
* 给定对应的颜色id，在表的第二部分查找颜色名，未找到返回0 
*
* 4. int LookupColorNameInColorTable(const char* colorName)
* 给定对应的颜色名，在表的第二部分查找颜色ID，未找到返回0 
*
* 5. int RegisterColorTable(const char* colorName) 
* 将一个颜色名添加到表的第二部分，并返回对应的ID，需要注意ID不应为0，
*     并且若该颜色本就存在于第二部分应忽略操作 
* 建议以从1开始逐次加一的方式分配ID
*
* 6. void RegisterColorName(const char* colorName,double r,double g,double b)
* 将一个颜色名添加到表的第一部分，成功返回 0，颜色名本就存在返回 1 
*
*
*/
typedef void (*ColorDefinitionTraverseFunction)(const char*, double, double, double);
void ChangePageOfColorTable(int p);
void ReadColorTable(FILE* f);
void WriteColorTable(FILE* f);
const char* LookupIDInColorTable(int id);
int LookupColorNameInColorTable(const char* colorName);
int RegisterColorTable(const char* colorName);
int RegisterColorName(const char* colorName,double r,double g,double b);
void TraverseColorDifinitions(ColorDefinitionTraverseFunction);
void ClearColorTable();

/*
*
* document: 24/5/20 林景行 
* program:
* last change: 
* details in github history
*
* Font Table数据结构
* 与Color Table相似，不过只有第二部分，因为第一部分的Font已经由系统预先定义。 
* 详细文档参见Color Table的相应函数 
* 所有文件均为二进制文件 
*
* 
*/
void ChangePageOfFontTable(int p);
void ReadFontTable(FILE* f);
void WriteFontTable(FILE* f);
const char* LookupIDInFontTable(int id);
int LookupFontNameInFontTable(const char* fontName);
int RegisterFontTable(const char* fontName);
void ClearFontTable();

/*
*
* assert.h 使用例程
*
* 1. 检查某个条件表达式 

//#include <assert.h>
assert(ptr.next!=0);  //在ptr.next==0时报错

//报错显示：assertion failed in 'ptr.next!=0' 

* 2. 在某个条件表达式本身并不清晰的时候输出详细报错信息 

//#include <assert.h>
//const int ColorNameExsists = 0;
if() assert(ColorNameExsists);

*
*
*/
#endif
