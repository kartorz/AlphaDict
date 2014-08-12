#ifndef _UTIL_H_
#define _UTIL_H_
#include <string>

#include "alphadict.h"
#include "iDict.h"
#include "tinyxml2/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

class Util
{
public:
    static unsigned int getTimeMS();

    static bool isDirExist(const string& dir);
    static bool isFileExist(const string& filename);
    static bool copyFile(const string& from, const string& to);
    static void copyDir(const string& from, const string& to);
    static bool createDir(const string& path);

    static void currentDir(string& path);
    static void usrProfileDir(string& path);
    static void execDir(string& path);
    static void tempDir(string& path);
    static void removeFileName(string& path);
    static void sleep(int ms/*milliseconds*/);
};

namespace util {

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

class XMLUtil {
public:
    static XMLElement* child(XMLElement *parent, int n);
    static int childrenSize(XMLElement *parent);
};

}
#endif
