#include "alphadict.h"
#include "Log.h"

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <string>

Log g_log;

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

void Log::operator()(LogLevel l, const char *msg, ...) const
{
	if(l < m_level) return;

	va_list args;
	va_start(args, msg);
	vfprintf(m_logFile, msg, args);
	va_end(args);
	fflush(m_logFile);
}
