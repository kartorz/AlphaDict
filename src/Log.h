#ifndef _LOG_H_
#define _LOG_H_

#include "alphadict.h"
#include "SpinLock.h"

typedef enum {
  LOG_DEBUG,
  LOG_ERROR,
  LOG_WARRNING,
  LOG_INFO,
  LOG_NONE,
}LogLevel;

class Log {
public:
	Log();
	~Log();
	void d(const char *msg, ...);
	void i(const char *msg, ...);
    void w(const char *msg, ...);
	void e(const char *msg, ...);
	void operator()(LogLevel l, const char *msg, ...);

	void setLevel(LogLevel l) { m_level = l; }
	LogLevel getLevel(LogLevel l) const { return m_level; }

private:
    LogLevel m_level;
    FILE *m_logFile;
    SpinCriticalSection m_crs;
};

extern  Log g_log;

#endif
