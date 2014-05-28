 /**
 * A wrapper of pthread_cond
 *
 * authors:
 *     liqiong lee <LiQiong.kartorz.lee@gmail.com>
 *
 * changes:
 *     Feb 19,2014  Created. [liqiong lee]
 *
 */
#include <assert.h>
#ifdef _LINUX
#include <sys/time.h>
#endif

#include <errno.h>
#include <stdio.h>

#include "ThdCond.h"
#include "Util.h"

#ifdef WIN32
#define SET_EVENT_BROADCAST(n) \
do {\
for (int i=0; i< n; i++) \
    SetEvent(m_cv); \
}while(0)

#endif

ThdCond::ThdCond()
 :m_unblock(false)
{
#ifdef WIN32
    m_cv = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_cv != NULL);
    //m_mutex =  CreateMutex(NULL, FALSE, NULL);
    //assert(m_mutex != NULL);
#else
    int ret = pthread_cond_init(&m_cv, NULL);
    assert(ret == 0);
    ret = pthread_mutex_init(&m_mutex, NULL);
    assert(ret == 0);
#endif
}

ThdCond::~ThdCond()
{
    /* Use have to wakeup all blocked thread */
#ifdef WIN32
    CloseHandle(m_cv);
    //CloseHandle(m_mutex);
#else
    pthread_cond_destroy(&m_cv);
    pthread_mutex_destroy(&m_mutex);
#endif
}

int ThdCond::consume(void *v, int timeout)
{
#ifdef _LINUX
    pthread_mutex_lock(&m_mutex);
#endif
    //printf("consume enter \n");
    while (!canConsume(v)) {
        //printf("can't consume consume to wait\n");
        if (timeout == -1) {
        #ifdef WIN32
            WaitForSingleObject(m_cv, INFINITE);
        #else
            pthread_cond_wait(&m_cv, &m_mutex);
        #endif
        } else {
        #ifdef WIN32
            DWORD result = WaitForSingleObject(m_cv, timeout);
            if (result == WAIT_TIMEOUT) {
                return -1;
	    }
        #else
            struct timespec to;
            int sec = timeout/1000;
            int nsec = (timeout%1000)*1000;
            to.tv_sec = time(NULL) + sec;
            to.tv_nsec = nsec;
            int err = pthread_cond_timedwait(&m_cv, &m_mutex, &to);
            if (err == ETIMEDOUT) {
                pthread_mutex_unlock(&m_mutex);
                return -1;
            }
        #endif
        }

        //printf("consume: wakeup\n");
        if (m_unblock) {
        #ifdef _LINUX
            pthread_mutex_unlock(&m_mutex);
        #endif
            return -2;
        }
    }
#ifdef _LINUX
    pthread_mutex_unlock(&m_mutex);
#endif
    onConsume(v);
    //printf("consume exit\n");
    return 0;
}

void ThdCond::produce(void *v, bool broadcast,  int blockthrds)
{
    onProduce(v);
#ifdef _LINUX
    pthread_mutex_lock(&m_mutex);
#endif

    if (broadcast == false) {
    #ifdef WIN32
        SetEvent(m_cv);
    #else
        pthread_cond_signal(&m_cv);
    #endif
    } else {
    #ifdef WIN32
        SET_EVENT_BROADCAST(blockthrds);
    #else
        pthread_cond_broadcast(&m_cv);
    #endif
    }
#ifdef _LINUX
    pthread_mutex_unlock(&m_mutex);
#endif
    //printf("produce %u: %d\n", Util::getTimeMS(), broadcast);
}

int ThdCond::waitEvent(int timeout/*ms*/)
{
#ifdef _LINUX
    pthread_mutex_lock(&m_mutex);
#endif
    if (timeout == -1) {
    #ifdef WIN32
        WaitForSingleObject(m_cv, INFINITE);
    #else
        pthread_cond_wait(&m_cv, &m_mutex);
    #endif
    } else {
    #ifdef WIN32
            DWORD result = WaitForSingleObject(m_cv, timeout);
            if (result == WAIT_TIMEOUT) {
                return -1;
	    }
    #else
        struct timespec to;
        clock_gettime(CLOCK_REALTIME, &to); /* sync with Util::GetTimeMS */
        //printf("waitEvent timeout1 (%lu, %lu)\n", to.tv_sec, to.tv_nsec);
        unsigned long long now_ms = (to.tv_sec * 1000) + (to.tv_nsec / 1000000);
        unsigned long long to_ms = now_ms + timeout;
        //int sec =  to_ms/1000;
        //int nsec = (timeout%1000)*1000000;
        to.tv_sec = to_ms/1000;
        to.tv_nsec = (to_ms%1000)*1000000;
        //printf("waitEvent timeout2 (%lu, %lu)\n",to.tv_sec, to.tv_nsec);
        int err = pthread_cond_timedwait(&m_cv, &m_mutex, &to);
        if (err == ETIMEDOUT) {
            pthread_mutex_unlock(&m_mutex);
            //printf("%u: waitEvent timeout ending\n", Util::getTimeMS());
            return -1;
	}
    #endif
    }

    if (m_unblock) {
    #ifdef _LINUX
        pthread_mutex_unlock(&m_mutex);
    #endif
        return -2;
    }
    //printf("%u:waitEvent ending\n", Util::getTimeMS());
#ifdef _LINUX
    pthread_mutex_unlock(&m_mutex);
#endif
    return 0;
}

void ThdCond::setEvent(bool broadcast, int blockthrds)
{
#ifdef _LINUX
    pthread_mutex_lock(&m_mutex);
#endif

    if (broadcast == false) {
    #ifdef WIN32
        SetEvent(m_cv);
    #else
        pthread_cond_signal(&m_cv);
    #endif
    } else {
    #ifdef WIN32
        SET_EVENT_BROADCAST(blockthrds);
    #else
        pthread_cond_broadcast(&m_cv);
    #endif
    }
    //printf("%u: setEvent %d\n", Util::getTimeMS(), broadcast);
#ifdef _LINUX
    pthread_mutex_unlock(&m_mutex);
#endif
}

void ThdCond::unblockAll(int blockthrds)
{
    //pthread_mutex_lock(&m_mutex);
    m_unblock = true;
#ifdef WIN32
    SET_EVENT_BROADCAST(blockthrds);
#else
    pthread_cond_broadcast(&m_cv);
#endif
    //pthread_mutex_unlock(&m_mutex);
}
