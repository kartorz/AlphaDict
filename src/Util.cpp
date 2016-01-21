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
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::filesystem;

#include "alphadict.h"
#include "string.h"
#include "Util.h"
#include "Configure.h"
#ifdef WIN32
#include "win32/WIN32Util.h"
#endif


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

unsigned long long Util::getAbsTimeSeconds()
{
#if defined(WIN32)
	LARGE_INTEGER  large_interger;
	QueryPerformanceFrequency(&large_interger);
	double dff = large_interger.QuadPart;
	QueryPerformanceCounter(&large_interger);
	return (unsigned long long)(large_interger.QuadPart * 1000 / dff);
#else
	struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec;
#endif
}

string Util::getDate()
{
    time_t tm = time(NULL);
    struct tm* dt = localtime(&tm);
    string year = intToString(dt->tm_year + 1900);
    string mon = intToString(dt->tm_mon + 1); // in the range 0 to 11.
    string day = intToString(dt->tm_mday);    // in the range 1 to 31.

    string dateformat = "YYYY-MM-DD";
    string format = "YYYY";
    boost::algorithm::replace_first(dateformat, format, year);
    
	format = "MM";
    boost::algorithm::replace_first(dateformat, format, mon);

	format = "DD";
    boost::algorithm::replace_first(dateformat, format, day);

    return dateformat;
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

string Util::replaceSuffix(const string& path, const string suffix)
{
    boost::filesystem::path p(path);
    return p.replace_extension(suffix).string();
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
        copy_file(from, to, copy_option::overwrite_if_exists);
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

void Util::usrProfileDir(string& path)
{
    string userHome;
#ifdef _LINUX
    if (getenv("HOME"))
        userHome = getenv("HOME");
    else
        userHome = "/root";
    path = userHome + "/." + APP_NAME;
#else
    WIN32Util::getProfilePath(userHome);
    path = userHome + "/" + APP_NAME;
#endif
}

void Util::execDir(string& strpath)
{
#ifdef WIN32
    WIN32Util::execDir(strpath);
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
    usrProfileDir(path);
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


bool Util::isValidInput(string& str)
{
    #define MAX_PHRASE_LEN  30
    if (str.length() >  MAX_PHRASE_LEN || str.empty())
        return false;

    // Check English
    if (isValidEnglishChar(str[0]))
    {
        // Don't check last char, eg: boy, boy;
        for (int i=0; i<str.length() - 1; i++) {
            char c = str[i];
            if (!(isValidEnglishChar(c) || c == ' '))
                return false;
        }
    }

    return true;
}

bool Util::isValidEnglishChar(char c)
{
    if (c >= 'A' && c <= 'Z')
        return true;

    if (c >= 'a' && c <= 'z')
        return true;

    return false;
}


int Util::stringToInt(string strInt)
{
    //strtol
    //atoi
    return boost::lexical_cast<int>(strInt);
}

string Util::intToString(int i)
{
    //snprintf
    //std::strstream
    string ret;
    std::stringstream ss;
    ss << i;
    ss >> ret;
   return ret;
}

string Util::stringCaseChange(const string& str, int start, int len)
{
    string ret = str;
    if (str.length() >= start + len) {
        for (int i = 0; i < len; i++) {
            char chr = str.at(start + i);
            if (islower(chr))
                chr = toupper(chr);
            else
                chr = tolower(chr);
            ret = ret.replace(start + i, 1, 1, chr);
        }
    }

    return ret;
}

string Util::stringCaseToLower(const string& str)
{
    return boost::algorithm::to_lower_copy(str);
#if 0
    string ret;
    for (int i = 0; i < str.length(); i++) {
        char chr = str.at(i);
        ret += tolower(chr);
    }
    return ret;
#endif
}

string Util::stringCaseToUpper(const string& str)
{
    return boost::algorithm::to_upper_copy(str);
#if 0
    string ret;
    for (int i = 0; i < str.length(); i++) {
        char chr = str.at(i);
        ret += toupper(chr);
    }
    return ret;
#endif
}

/*string Util::replaceString(string& ori, string old, string new, int count)
{

}*/

namespace util {

size_t ReadFile::operator()(FILE *f, void *ptr, size_t length)
{
    size_t rdbytes = 0;
    size_t rsize;
    if (f == NULL)
        return 0;
    do {
        int bytes = length - rdbytes;
	rsize = fread(ptr, 1, bytes, f);
        rdbytes += rsize;
    }while(rsize > 0 && rdbytes < length);
    return rdbytes;
}

void* ReadFile::operator()(FILE *f, size_t length)
{
    if (f == NULL)
        return NULL;

    if (ptr != NULL)
        free(ptr);
    if (length == -1) {
        fseek(f, 0, SEEK_END);
        length = ftello(f);
        fseek(f, 0, SEEK_SET); 
    }
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

XMLElement* XMLUtil::Child(XMLElement *parent, int n /*0..n*/)
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

int XMLUtil::ChildrenSize(XMLElement *parent)
{
    XMLElement* e = parent->FirstChildElement();
    int size = 0;
    while (e) {
        size++;
        e = e->NextSiblingElement();
    }
    return size;
}

string XMLUtil::Attribute(const XMLElement *e, const char* attr, const char* defval)
{
    string val;
    const char *attrval = e->Attribute(attr);
    if (attrval)
        val  = string(attrval);
    else
        val = string(defval);
    return val;
}

}
