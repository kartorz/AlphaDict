# ifdef WIN32
#include <Windows.h>
#include <direct.h>
# endif
#include <time.h>
#include <stdarg.h>
#include <wchar.h>
#include <sys/stat.h>
# ifdef _LINUX
#include <dirent.h>
# endif
#include <iostream>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "alphadict.h"
#include "string.h"
#include "Util.h"

unsigned int Util::getTimeMS()
{
#if defined(WIN32)
  return timeGetTime();
#else
	static unsigned long long start_mstime = 0;
	unsigned long long now_mstime;
	struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
	now_mstime = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
	if (start_mstime == 0) {
		start_mstime = now_mstime;
	}
    /*printf("{getTimeMS} (%lu, %lu) --> (%lu, %lu), %u\n", 
       ts.tv_sec, ts.tv_nsec, now_mstime, start_mstime, now_mstime - start_mstime);*/
	return(now_mstime - start_mstime);
#endif
}

bool Util::isDirExist(const string& dir)
{
    struct stat attr;
    int ret = stat(dir.c_str(), &attr);
    if (ret != 0)
        return false;

    if (S_ISDIR(attr.st_mode))
        return true;
    return false;
}

bool Util::isFileExist(const string& filename)
{
    struct stat attr;
    int ret = stat(filename.c_str(), &attr);
    if (ret != 0)
        return false;
    if (S_IFREG & attr.st_mode)
        return true;
    return false;
}

bool Util::createDir(const string& path)
{
#ifdef WIN32
    int ret = _mkdir(path.c_str());
#else
    int ret = mkdir(path.c_str(), S_IRWXU);
#endif
    return ret == 0;
}

bool Util::copyFile(const string& from, const string& to)
{
    try {
        copy_file(from, to);
    } catch (const filesystem_error& ex) {
         printf("%s", ex.what());   
         return false;
    }
    return true;
}

void Util::copyDir(const string& from, const string& to)
{
#ifdef _LINUX
    struct dirent *ep;
    DIR *dp = opendir(from.c_str());
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            if (ep->d_name[0] == '.')
                continue;
            string f = string(ep->d_name);
            //printf("dirctory:(%s)\n", f.c_str());
            string s = from + "/" + f;
            string d = to + "/" + f;
            try {
                copy_file(s, d);
            } catch (const filesystem_error& ex) {
                printf("%s", ex.what());
            }
        }
        closedir(dp);
    }
#else
    path p(from);
    for (directory_iterator iter(p); iter != directory_iterator(); iter++) {
        //cout << iter->path().string() << "\n";
        //string d = to + "/" + iter->path().filename().string();
        path d(to + "/" + iter->path().filename().string());
        copy_file(p, d);
    }
#endif
}

void Util::currentDir(string& path)
{
    char buf[512];
    if (getcwd(buf, 512) == NULL)
        return;
    path = string(buf, 512);
}

void Util::usrHomeDir(string& path)
{
#ifdef _LINUX
    string userHome;
    if (getenv("HOME"))
        userHome = getenv("HOME");
    else
        userHome = "/root";
    path = userHome + "/." + APP_NAME;
#else
    execDir(path);
#endif
}

void Util::execDir(string& strpath)
{
#ifdef WIN32
    char szAppPath[MAX_PATH] = "";
    ::GetModuleFileNameA(0, szAppPath, sizeof(szAppPath) - 1);
    //strncpy(szDest,szAppPath,sizeof(szAppPath));
    strpath.append(szAppPath);
    removeFileName(strpath);
    //boost::filesystem::path p(p1);
    //p.remove_filename();
    //strpath = p.string();
#elif defined(_LINUX)
    strpath = "/usr/local/share/alphadict";
    char buf[512];
    int length = readlink("/proc/self/exe", buf, 512);
    if(length != -1) {
        //dirname(buf);
        string temp(buf, length);
        boost::filesystem::path execPath(temp);
        //boost::filesystem::path dir(path);
        strpath = execPath.remove_filename().string();
    }
#endif
}
 
void Util::tempDir(string& path)
{
#ifdef _LINUX
    path = "/tmp";
#else
    execDir(path);
#endif
}

void Util::removeFileName(string& path)
{
    int pos;
    if ((pos = path.find_last_of('/')) == string::npos) {
        pos = path.find_last_of('\\');
    }

    if (pos != string::npos) {
        path.erase(pos);
    }
}

void Util::sleep(int ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    if (ms/1000 > 0)
        sleep(ms/1000);
    //EINVAL usec is not smaller than 1000000.
    if (ms%1000 > 0)
        usleep((ms%1000)*1000);
#endif
}

namespace util {

size_t ReadFile::operator()(FILE *f, void *ptr, size_t length)
{
    size_t rdbytes = 0;
    size_t rsize;
    do {
        int bytes = length - rdbytes;
	rsize = fread(ptr, 1, bytes, f);
        rdbytes += rsize;
    }while(rsize > 0 && rdbytes < length);
    return rdbytes;
}

void* ReadFile::operator()(FILE *f, size_t length)
{
    if (ptr != NULL)
        free(ptr);
    ptr = malloc(length);
    size_t rdbytes = 0;
    size_t rsize;
    do {
        int bytes = length - rdbytes;
	rsize = fread(ptr, 1, bytes, f);
        rdbytes += rsize;
    }while(rsize > 0 && rdbytes < length);
    return ptr;	
}

XMLElement* XMLUtil::child(XMLElement *parent, int n /*0..n*/)
{
    XMLElement* e = parent->FirstChildElement();
    if (!e)
        return NULL;

    for (int i=0; i<n; i++) {
        if (e)
            e = e->NextSiblingElement();
        else
            return NULL;
    }
    return e;
}

int XMLUtil::childrenSize(XMLElement *parent)
{
    XMLElement* e = parent->FirstChildElement();
    int size = 0;
    while (e) {
        size++;
        e = e->NextSiblingElement();
    }
    return size;
}

}
