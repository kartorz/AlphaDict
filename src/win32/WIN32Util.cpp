#include <Windows.h>
#include <direct.h>
#include <Shlobj.h>

#include "WIN32Util.h"
#include "Util.h"

string WIN32Util::getSpecialFolder(int csidl)
{
  string strProfilePath = "";
  CHAR szPath[MAX_PATH];

  if(SUCCEEDED(SHGetFolderPathA(NULL,csidl,NULL,0,szPath)))
  {
    strProfilePath = szPath;
  }
  strProfilePath = toUnixPathStyle(strProfilePath);
  return strProfilePath;
}

void WIN32Util::getProfilePath(string &path)
{
    path = getSpecialFolder(CSIDL_APPDATA|CSIDL_FLAG_CREATE);
    if (path.length() == 0)
        execDir(path);
}

void WIN32Util::execDir(string& strpath)
{
    char szAppPath[MAX_PATH] = "";
    ::GetModuleFileNameA(0, szAppPath, sizeof(szAppPath) - 1);
    //strncpy(szDest,szAppPath,sizeof(szAppPath));
    strpath.append(szAppPath);
    Util::removeFileName(strpath);
    strpath = toUnixPathStyle(strpath);
}

string WIN32Util::toUnixPathStyle(string &winPath)
{
    string unixPath = winPath;
    int pos = unixPath.find_first_of('\\');
    while (pos != string::npos) {
        unixPath.replace(pos, 1, "/");
        pos = unixPath.find_first_of('\\', ++pos);
    }
    return unixPath;
}
