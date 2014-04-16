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

bool ThdCond::consume(void *v, int timeout)
{
    pthread_mutex_lock(&m_mutex);
    printf("consume 1.1\n");
	while (!canConsume(v)) {
        printf("consume to wait\n");
        if (timeout == 0) {
            pthread_cond_wait(&m_cv, &m_mutex);
        } else {
            struct timespec to;
            to.tv_sec = time(NULL) + timeout;
            to.tv_nsec = 0;            
            int err = pthread_cond_timedwait(&m_cv, &m_mutex, &to);
            if (err == ETIMEDOUT) {
                pthread_mutex_unlock(&m_mutex);
                return false;
		    }
        }

        printf("consume wakeup\n");
        if (m_unblock) {
            pthread_mutex_unlock(&m_mutex);
            return false;
        }
    }
    pthread_mutex_unlock(&m_mutex);

    onConsume(v);
    printf("consume exit\n");
    return true;
}

void ThdCond::produce(void *v, bool broadcast)
{
	onProduce(v);
    
    pthread_mutex_lock(&m_mutex);
    if (broadcast == false)
    	pthread_cond_signal(&m_cv);
    else
        pthread_cond_broadcast(&m_cv);
    printf("wakup a thread\n");
    pthread_mutex_unlock(&m_mutex);
}

bool ThdCond::waitEvent(int timeout)
{
    pthread_mutex_lock(&m_mutex);

    printf("waitEvent to wait\n");
    if (timeout == 0) {
        pthread_cond_wait(&m_cv, &m_mutex);
    } else {
        struct timespec to;
        to.tv_sec = time(NULL) + timeout;
        to.tv_nsec = 0;            
        int err = pthread_cond_timedwait(&m_cv, &m_mutex, &to);
        if (err == ETIMEDOUT) {
            pthread_mutex_unlock(&m_mutex);
            return false;
		}
    }
    printf("waitEvent wakeup %d\n", m_unblock);
    if (m_unblock) {
        printf("cond wait 2.3\n");
        pthread_mutex_unlock(&m_mutex);
        return false;
    }

    pthread_mutex_unlock(&m_mutex);
    return true;
}

void ThdCond::setEvent(bool broadcast)
{
    pthread_mutex_lock(&m_mutex);
 
    if (broadcast == false)
    	pthread_cond_signal(&m_cv);
    else
        pthread_cond_broadcast(&m_cv);

    pthread_mutex_unlock(&m_mutex);
}

void ThdCond::unblockAll()
{
    pthread_mutex_lock(&m_mutex);
    m_unblock = true;
    pthread_cond_broadcast(&m_cv);
    pthread_mutex_unlock(&m_mutex);
}
