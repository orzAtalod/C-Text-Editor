#include "fileSystem.h"
#define FILE_SYSTEM_TEST

#ifdef FILE_SYSTEM_TEST
void definColorFunc(const char* str, double r, double g, double b)
{
	printf("color %s is defined as %lf,%lf,%lf\n",str,r,g,b);
}

void Main()
{
	printf("tester begin\n");
	definColorFunc("red",1,0,0);
	InitFileSystem(definColorFunc);
	printf("commands:"); 
}
#endif
