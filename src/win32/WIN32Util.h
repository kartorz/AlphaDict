#ifndef _WIN32UTIL_H_
#define _WIN32UTIL_H_
#include <string>

using namespace std;

class WIN32Util {
public:
    static string getSpecialFolder(int csidl);
    static void   getProfilePath(string &path);
    static void   execDir(string& strpath);
    static string WIN32Util::toUnixPathStyle(string &winPath);
};

#endif