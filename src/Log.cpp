# ifdef WIN32
#include <Windows.h>
# endif

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include "alphadict.h"
#include "Log.h"
#include "Util.h"

Log g_sysLog;

Log::Log()
: m_level(LOG_ERROR)
{
    std::string path;
    Util::tempDir(path);
    path += "/alphadict.log";
    //std::string path = "stdout";
    m_logFile = fopen(path.c_str(), "w+");
    if (m_logFile == NULL) {
        printf("can't open %s for writing \n", path.c_str());
        m_logFile = stdout/*STDERR*/;
    }
}

Log::~Log()
{
    i("{Log}: close log file\n");
    if (m_logFile != stdout)
        fclose(m_logFile);
}

#define VFPRINT_MSG(l) do {        \
    SpinLock lock(m_crs);          \
    fprintf(m_logFile, "[%u] %d:  ",Util::getTimeMS(), l);   \
    va_list args;                  \
    va_start(args, msg);           \
    vfprintf(m_logFile, msg, args);\
    va_end(args);                  \
    fflush(m_logFile);         \
}while(0)

void Log::d(const char *msg, ...)
{
    if (LOG_DEBUG >= m_level) {
        VFPRINT_MSG(LOG_DEBUG);
    }
}

void Log::i(const char *msg, ...)
{
    if (LOG_INFO >= m_level){
        VFPRINT_MSG(LOG_INFO);
    }
}

void Log::w(const char *msg, ...)
{
  if (LOG_WARRNING >= m_level){
        VFPRINT_MSG(LOG_WARRNING);
    }
}

void Log::e(const char *msg, ...)
{
    if (LOG_ERROR >= m_level){
        VFPRINT_MSG(LOG_ERROR);
    }
}

void Log::operator()(LogLevel l, const char *msg, ...)
{
    if(l >= m_level) {
        VFPRINT_MSG(l);
    }
}
