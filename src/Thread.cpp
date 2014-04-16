#include "alphadict.h"
#include "Thread.h"

#include <unistd.h>

void* thread_loop(void *v)
{
    Thread *thrd = (Thread *)v;
    thrd->onStartup();

    do {
        thrd->doWork();
        if (thrd->m_sleepTimeMs > 0)
            usleep(thrd->m_sleepTimeMs);
    }while(!thrd->m_reqAbort);

    thrd->onExit();
    thrd->m_stop = true;
}

Thread::Thread(int sleep, bool once)
:m_threadId(-1), 
 m_stop(false)
{
    m_sleepTimeMs = sleep;
    m_reqAbort = once;
}

Thread::~Thread()
{
    abort();
    pthread_join(m_threadId, NULL);
}

void Thread::start()
{
    int ret = pthread_create(&m_threadId, NULL, &thread_loop, this);
    if (ret != 0) {
        m_stop = true;
        g_log.e("{Thread} create thread failure ret(%d)\n", ret);
    }    
}

void Thread::abort()
{
    m_reqAbort = true;
}

bool Thread::IsCurrentThread()
{
    return pthread_equal(pthread_self(), m_threadId);
}

void Thread::join()
{
    if (!IsCurrentThread())
        pthread_join(m_threadId, NULL);
}

pthread_t Thread::getCurrentThreadId()
{
    return pthread_self();
}

bool IsCurrentThread(const pthread_t tid)
{
    return pthread_equal(pthread_self(), tid);
}
