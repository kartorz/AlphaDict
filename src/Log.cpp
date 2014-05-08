#define _LOG_CPP_

#include "alphadict.h"
#include "Log.h"

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>

Log::Log()
: m_level(LOG_DEBUG)
{
	std::string path = "/tmp/alphadict.log";
    //std::string path = "stdout";
	m_logFile = fopen(path.c_str(), "w+");
}

Log::~Log()
{
	fclose(m_logFile);
}

#define VFPRINT_MSG do {           \
    SpinLock lock(m_crs);          \
	va_list args;                  \
	va_start(args, msg);           \
	vfprintf(m_logFile, msg, args);\
	va_end(args);                  \
	fflush(m_logFile);             \
}while(0)

void Log::d(const char *msg, ...)
{
    if (LOG_DEBUG >= m_level) {
        VFPRINT_MSG;
    }
}

void Log::i(const char *msg, ...)
{
    if (LOG_INFO >= m_level){
        VFPRINT_MSG;
    }
}

void Log::w(const char *msg, ...)
{
  if (LOG_WARRNING >= m_level){
        VFPRINT_MSG;
    }
}

void Log::e(const char *msg, ...)
{
    if (LOG_ERROR >= m_level){
        VFPRINT_MSG;
    }
}

void Log::operator()(LogLevel l, const char *msg, ...)
{
	if(l >= m_level) {
        VFPRINT_MSG;
    }
}
