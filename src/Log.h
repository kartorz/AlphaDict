#ifndef _LOG_H_
#define _LOG_H_

#include "alphadict.h"

typedef enum {
  LOG_DEBUG,
  LOG_ERROR,
  LOG_INFO,
  LOG_NONE,
}LogLevel;

class Log {
public:
	static Log& getInstance();
	Log();
	void d(char *msg, ...) const { operator()(LOG_DEBUG, msg);  }
	void i(char *msg, ...) const { operator()(LOG_INFO,  msg);  }
	void e(char *msg, ...) const { operator()(LOG_ERROR, msg);  }
	void operator()(LogLevel l, const char *msg, ...) const;

	void setLevel(LogLevel l) { m_level = l; }
	LogLevel getLevel(LogLevel l) const { return m_level; }

private:
	LogLevel m_level;
	FILE *m_logFile;
};

extern Log g_log;
#endif
