#include "alphadict.h"
#include "string.h"
#include "Util.h"

#include <time.h>
#include <stdarg.h>
#include <wchar.h>
#include <sys/stat.h>
#include <dirent.h>

#include <iostream>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

unsigned int Util::getTimeMS()
{
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
}

/* This piece of code comes from an example of mbrtowc function of GNU libc. */
wchar_t Util::mbrtowc_r(char** mb)
{
	wchar_t wctmp[1];
	size_t len = strlen(*mb);
	size_t nbytes = mbrtowc(wctmp, *mb, len, NULL);
	if (nbytes > 0) {
		if (nbytes > (size_t)-2)
			return 0;
		*mb += nbytes;
		return wctmp[0];
	}
}

int Util::wcrtomb_r(char* s, wchar_t *wc)
{
    int nbytes = wcrtomb(s, *wc, NULL);
    if (nbytes == (size_t) -1)
        /* Error in the conversion. */
        return -1;
    return nbytes;
}

/* This piece of code comes from an example of mbrtowc function of GNU libc. */
// Caller should release pointer 'wchar_t*'.
wchar_t* Util::mbstowcs(const char *mb)
{
	size_t len = strlen(mb);
	wchar_t *result = (wchar_t *)malloc((len+1)*sizeof(wchar_t));
	wchar_t *wcp = result;
	wchar_t tmp[1];
	mbstate_t state;
	size_t nbytes;
	memset(&state, '\0', sizeof(state));
	memset(result, '\0', (len+1)*sizeof(wchar_t));
	while ((nbytes = mbrtowc(tmp, mb, len, &state)) > 0)
	{
		if (nbytes >= (size_t) -2) {
		    g_log.e("mbstowcs: invalid input string\n");
            free(result);
			return NULL;
		}
		*wcp++ = tmp[0];
		len -= nbytes;
		mb += nbytes;
	}
	return result;
}

// Caller should release pointer 'wchar_t*'
wchar_t* Util::mbsrtowcs_r(const char *mb)
{
    size_t len = strlen(mb);
    len = (len+1)*sizeof(wchar_t);
    wchar_t *result = (wchar_t *)malloc(len);
    memset(result, L'\0', len);
    size_t ret = mbsrtowcs(result, &mb, len, NULL);
    if (ret == (size_t)-1) {
        free(result);
        return NULL;
    }

    return result;
}

// Caller should release pointer char*
char* Util::wcsrtombs_r(const wchar_t *wc)
{
    size_t len = wcslen(wc);
    len = len*sizeof(wchar_t) + 1;
    char *result = (char *)malloc(len);
    memset(result, '\0', len);
    size_t ret = wcsrtombs(result, &wc, len, NULL);
    if (ret == (size_t)-1) {
        if (result[0] == '\0') {
            free(result);
            g_log.e("{wcsrtombs_r}: invalid wc string\n");
            return NULL;
        } else {
            g_log.w("{wcsrtombs_r}: (%s), encounter a invalid wide character(0x%x)\n", result, *wc);
        }
    }
    return result;
}

bool Util::isDirExist(const string& dir)
{
    struct stat attr;
    int ret = stat(dir.c_str(), &attr);
    if (ret != 0)
        return false;
    if (S_ISDIR(attr.st_mode) != 0)
        return true;
    return false;
}

bool Util::isFileExist(const string& filename)
{
    struct stat attr;
    int ret = stat(filename.c_str(), &attr);
    if (ret != 0)
        return false;
    if (S_ISREG(attr.st_mode) != 0)
        return true;
    return false;
}

bool Util::createDir(const string& path)
{
    int ret = mkdir(path.c_str(), S_IRWXU);
    return ret == 0;
}

bool Util::copyFile(const string& from, const string& to)
{
    try {
        copy_file(from, to);
    } catch (const filesystem_error& ex) {
         //printf("%s", ex.what());
         g_log.e("%s\n", ex.what());   
         return false;
    }
    return true;
}

void Util::copyDir(const string& from, const string& to)
{
    struct dirent *ep;
    DIR *dp = opendir(from.c_str());
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            if (ep->d_name[0] == '.')
                continue;
            string f = string(ep->d_name);
            printf("dirctory:(%s)\n", f.c_str());
            string s = from + "/" + f;
            string d = to + "/" + f;
            try {
                copy_file(s, d);
            } catch (const filesystem_error& ex) {
                //printf("%s", ex.what());
                g_log.e("%s\n", ex.what());
            }
        }
        closedir(dp);
    }
}

bool Util::currentDir(string& path)
{
    char buf[512];
    if (getcwd(buf, 512) == NULL)
        return false;
    path = string(buf, 512);
    return true;
}

bool Util::execDir(string& path)
{
#ifdef _WIN32
    //GetModuleFileName 
#elif defined(_LINUX)
    path = "/usr/local/share/alphadict";
#if 0
    char buf[512];
    int length = readlink("/proc/self/exe", buf, 512);
    if(length != -1) {
        //dirname(buf);
        string temp(buf, length);
        boost::filesystem::path execPath(temp);
        //boost::filesystem::path dir(path);
        path = execPath.remove_filename().string();
        return true;
    }
    return false;
#endif
#endif
}

size_t ReadFile::operator()(FILE *f, void *ptr, size_t length)
{
	int rdbytes = 0;
    int rsize;
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
	int rdbytes = 0;
    int rsize;
	do {
		int bytes = length - rdbytes;
		rsize = fread(ptr, 1, bytes, f);
        rdbytes += rsize;
	}while(rsize > 0 && rdbytes < length);
	return ptr;	
}

