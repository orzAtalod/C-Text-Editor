#ifndef __TEXT_STRUCTURE_INCLUDED__
#define __TEXT_STRUCTURE_INCLUDED__

typedef struct{
	char context;
	char style; //0 for nothing, 1 for Bold, 2 for Italic, 3 for Bold+Italic
} StyleChar;

typedef struct{
	int pointSize;
	int indent;
	const char* color;
	const char* font;
	StyleChar* context;
} StyleString;

StyleString* ReadStyleString(FILE* f);
StyleString* WriteStyleString(FILE* f);

#endif
