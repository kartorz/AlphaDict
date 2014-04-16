#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>

class Thread
{
friend void* thread_loop(void *v);

public:
    Thread(int sleep=20, bool once=false);
    virtual ~Thread();

    void start();
    void abort();
    bool isRunning() {return m_stop ? false : true; }
    pthread_t threadId() {return m_threadId;}
    void join();
    bool IsCurrentThread();

    static pthread_t getCurrentThreadId();
    static bool IsCurrentThread(const pthread_t tid);

protected:
    virtual void doWork() = 0;
    virtual void onExit() {}
    virtual void onStartup() {}

    pthread_t m_threadId;
    bool m_reqAbort;
    int m_sleepTimeMs;
    bool m_stop;
};

#endif
