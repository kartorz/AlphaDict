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
    static unsigned long long getAbsTimeSeconds();
    static string getDate();
    static bool isDirExist(const string& dir);
    static bool isFileExist(const string& filename);
    static bool copyFile(const string& from, const string& to);
    static void copyDir(const string& from, const string& to);
    static bool createDir(const string& path);
    static string replaceSuffix(const string& path, const string suffix);

    static void currentDir(string& path);
    static void usrProfileDir(string& path);
    static void execDir(string& path);
    static void tempDir(string& path);
    static void removeFileName(string& path);
    static void sleep(int ms/*milliseconds*/);

    static bool isValidInput(string& str);
    static bool isValidEnglishChar(char c);

    static int stringToInt(string strInt);
    static string intToString(int i);

    // [str]: should be Latin alphabetic string.
    static string stringCaseChange(const string& str, int start, int len);
    static string stringCaseToLower(const string& str);
    static string stringCaseToUpper(const string& str);
    static int    stringCommonLen(const string& str1, const string& str2, int start = 0);
    // Replace /old/ with /new/ of the /ori/ up to /count/ /old/ occurs. 
    //static string replaceString(string& ori, string old, string new, int count);
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
    static XMLElement* Child(XMLElement *parent, int n);
    static int ChildrenSize(XMLElement *parent);
    static string Attribute(const XMLElement *e, const char* attr, const char* defval);
};

}
#endif
