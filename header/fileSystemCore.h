#ifndef __FILE_SYSTEM_CORE_H__
#define __FILE_SYSTEM_CORE_H__
#include <stdio.h>

/*
*
* Color Table���ݽṹ 
* ��Ϊ�������֣���һ�����ִ�char*ӳ�䵽double[3]�����ڶ�����ɫ���ڶ�������
*     ��intӳ�䵽char*�����ڶ���Color���ݣ��������char*���飩 
* ֧��ҳ��������������255ҳ���ݣ�ͨ��ChangePageOfColorTable�л�����ͬҳ��
*     �ݻ������ţ��൱�ڲ�ͬ�ļ�������id=0��Page��Ϊ�����ڴ��д�ȡ�κ���
*     �ݣ�Input�ຯ���Ծ���Ҫ�ƶ��ļ�ָ����λ��, Output�ຯ���豨��
* �����ʹ��<assert.h>������¼assert.hʹ������ 
* �����ļ���Ϊ�������ļ� 
* ֧�����¹��ܣ�
*
* 0. void ChangePageOfColorTable(int p)
* �޸ĵ�ǰҳ
*
* 1. void ReadColorTable(FILE* f)
* ����һ��Color Table���滻��ǰҳ��Color Table��������char*ӳ���double[3]
*     �ʹ�intӳ�䵽char*�������֣�ͬʱ���ڵ�һ�����ֵ�ÿһ�����func������ɫ
*     ע�� 
* ����f�Ĺ��λ������һ��������ʼ������Ȼ���뼴�ɣ���Ҫ����� 
*
* 2. void WriteColorTable(FILE* f) 
* ��Read������ϣ���д���������ֵ�Color Table��f������Ӧ���Ĺ��λ��
*
* 3. const char* LookupIDInColorTable(int id) 
* ������Ӧ����ɫid���ڱ�ĵڶ����ֲ�����ɫ����δ�ҵ�����0 
*
* 4. int LookupColorNameInColorTable(const char* colorName)
* ������Ӧ����ɫ�����ڱ�ĵڶ����ֲ�����ɫID��δ�ҵ�����0 
*
* 5. int RegisterColorTable(const char* colorName) 
* ��һ����ɫ����ӵ���ĵڶ����֣������ض�Ӧ��ID����Ҫע��ID��ӦΪ0��
*     ����������ɫ���ʹ����ڵڶ�����Ӧ���Բ��� 
* �����Դ�1��ʼ��μ�һ�ķ�ʽ����ID
*
* 6. void RegisterColorName(const char* colorName,double r,double g,double b)
* ��һ����ɫ����ӵ���ĵ�һ���֣��ɹ����� 0����ɫ�����ʹ��ڷ��� 1 
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
* document: 24/5/20 �־��� 
* program:
* last change: 
* details in github history
*
* Font Table���ݽṹ
* ��Color Table���ƣ�����ֻ�еڶ����֣���Ϊ��һ���ֵ�Font�Ѿ���ϵͳԤ�ȶ��塣 
* ��ϸ�ĵ��μ�Color Table����Ӧ���� 
* �����ļ���Ϊ�������ļ� 
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
* assert.h ʹ������
*
* 1. ���ĳ���������ʽ 

//#include <assert.h>
assert(ptr.next!=0);  //��ptr.next==0ʱ����

//������ʾ��assertion failed in 'ptr.next!=0' 

* 2. ��ĳ���������ʽ������������ʱ�������ϸ������Ϣ 

//#include <assert.h>
//const int ColorNameExsists = 0;
if() assert(ColorNameExsists);

*
*
*/
#endif
