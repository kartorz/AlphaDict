# ifdef WIN32
#include <Windows.h>
#include <Process.h>
# endif
#include "alphadict.h"
#include "Util.h"
#include "Log.h"
#include "Thread.h"

#ifdef WIN32
unsigned WINAPI thread_loop(void *v)
#else
void* thread_loop(void *v)
#endif
{
    Thread *thrd = (Thread *)v;
    thrd->onStartup();

    do {
        thrd->doWork();
        if (thrd->m_sleepTimeMs > 0) {
            Util::sleep(thrd->m_sleepTimeMs);
        }
    }while(!thrd->m_reqAbort);

    thrd->onExit();
    thrd->m_stop = true;
    return 0;
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
    if (!m_stop)
        stop();
}

void Thread::start()
{
    bool ret = false;
#ifdef _LINUX
    if (pthread_create(&m_threadId, NULL, &thread_loop, this) == 0)
        ret = true; 
#elif defined(WIN32)
    m_thrdHandle = (HANDLE)_beginthreadex(NULL, 0, &thread_loop, this, 0, &m_threadId);
    if (m_thrdHandle)
        ret = true;
#endif

    if (ret == false) {
        m_stop = true;
        g_sysLog.e("{Thread} create thread failure ret(%d)\n", ret);
    }
}

void Thread::stop()
{
    abort();
    waitThreadExit();
}

void Thread::abort()
{
    m_reqAbort = true;
}

bool Thread::isCurrentThread()
{
#ifdef _LINUX
    return pthread_equal(currentThreadId(), m_threadId);
#else
    return m_threadId == currentThreadId();
#endif
}

void Thread::waitThreadExit()
{
    if (!isCurrentThread()) {
#ifdef _LINUX
        pthread_join(m_threadId, NULL);
#elif defined(WIN32)
        WaitForSingleObject(m_thrdHandle, INFINITE);
        CloseHandle(m_thrdHandle);
#endif
    }
}

pthread_t Thread::currentThreadId()
{
#ifdef _LINUX
    return pthread_self();
#elif defined(WIN32)
    return GetCurrentThreadId();
#endif
}

