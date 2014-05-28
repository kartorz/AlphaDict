#ifndef _THREAD_H_
#define _THREAD_H_
#ifdef _LINUX
#include <pthread.h>
#endif
#include "type.h"

class Thread
{
#ifdef WIN32
friend unsigned WINAPI thread_loop(void *v);
#else
friend void* thread_loop(void *v);
#endif

public:
    Thread(int sleep=20, bool once=false);
    virtual ~Thread();

    void start();
    void abort();
    bool isRunning() {return m_stop ? false : true; }
    pthread_t threadId() {return m_threadId;}
    void waitThreadExit();
    bool isCurrentThread();

    static pthread_t currentThreadId();

protected:
    virtual void doWork() = 0;
    virtual void onExit() { }
    virtual void onStartup() {}

    pthread_t m_threadId;
    bool m_reqAbort;
    int m_sleepTimeMs;
    bool m_stop;
#ifdef WIN32
    HANDLE  m_thrdHandle;
#endif
};

#endif
