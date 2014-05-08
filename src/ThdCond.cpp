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
#include "ThdCond.h"

#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>

#include "Util.h"

ThdCond::ThdCond()
 :m_unblock(false)
{
	int ret = pthread_cond_init(&m_cv, NULL);
	assert(ret == 0);
	ret = pthread_mutex_init(&m_mutex, NULL);
    assert(ret == 0);
}

ThdCond::~ThdCond()
{
    /* Use have to wakeup all blocked thread */

	pthread_cond_destroy(&m_cv);
    pthread_mutex_destroy(&m_mutex);
}

int ThdCond::consume(void *v, int timeout)
{
    pthread_mutex_lock(&m_mutex);
    //printf("consume enter \n");
	while (!canConsume(v)) {
        //printf("can't consume consume to wait\n");
        if (timeout == 0) {
            pthread_cond_wait(&m_cv, &m_mutex);
        } else {
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
        }

        //printf("consume: wakeup\n");
        if (m_unblock) {
            pthread_mutex_unlock(&m_mutex);
            return -2;
        }
    }
    pthread_mutex_unlock(&m_mutex);

    onConsume(v);
    //printf("consume exit\n");
    return 0;
}

void ThdCond::produce(void *v, bool broadcast)
{
	onProduce(v);
    
    pthread_mutex_lock(&m_mutex);
    if (broadcast == false)
    	pthread_cond_signal(&m_cv);
    else
        pthread_cond_broadcast(&m_cv);
    pthread_mutex_unlock(&m_mutex);
    //printf("produce %u: %d\n", Util::getTimeMS(), broadcast);
}

int ThdCond::waitEvent(int timeout/*ms*/)
{
    pthread_mutex_lock(&m_mutex);

    if (timeout == 0) {
        pthread_cond_wait(&m_cv, &m_mutex);
    } else {
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
    }

    if (m_unblock) {
        pthread_mutex_unlock(&m_mutex);
        return -2;
    }
    //printf("%u:waitEvent ending\n", Util::getTimeMS());
    pthread_mutex_unlock(&m_mutex);
    return 0;
}

void ThdCond::setEvent(bool broadcast)
{
    pthread_mutex_lock(&m_mutex);
 
    if (broadcast == false)
    	pthread_cond_signal(&m_cv);
    else
        pthread_cond_broadcast(&m_cv);
    //printf("%u: setEvent %d\n", Util::getTimeMS(), broadcast);
    pthread_mutex_unlock(&m_mutex);
}

void ThdCond::unblockAll()
{
    pthread_mutex_lock(&m_mutex);
    m_unblock = true;
    pthread_cond_broadcast(&m_cv);
    pthread_mutex_unlock(&m_mutex);
}
