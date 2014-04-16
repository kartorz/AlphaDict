#ifndef _UTIL_H_
#define _UTIL_H_
#include "alphadict.h"
#include "iDict.h"

class Util
{
public:
	static unsigned int getTimeMS();
	static wchar_t* mbstowcs(const char *mb);
	static wchar_t  mbrtowc_r(char** mb);
};

class ReadFile {
public:
	ReadFile()
	:ptr(NULL)
	{ }

	~ReadFile()	
	{ 
		if (ptr != NULL)
			free(ptr);
	}
	size_t operator()(FILE *f, void *ptr, size_t length);
	void*  operator()(FILE *f, size_t length);

	void *ptr;
};

class Malloc {
public:
	Malloc()
	:ptr(NULL) { }
	void* operator()(size_t size) {
	    ptr = malloc(size);
		return ptr;
	}
	~Malloc()
    {
		if (ptr != NULL)
			free(ptr);
	}
	void *ptr;
};
#endif
