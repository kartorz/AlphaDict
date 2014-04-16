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
#ifndef _THDCOND_H_
#define _THDCOND_H_

#include <MutexLock.h>

class ThdCond
{
public:
	ThdCond();
	virtual ~ThdCond();
    
    /* timeout == 0 meas waitting infintily or waitting 'timeout' secods */
	bool consume(void *v, int timeout=0);
    void produce(void *v, bool broadcast=false);

	bool waitEvent(int timeout=0);
    void setEvent(bool broadcast=false);

    void unblockAll(); /* unblock all blocked threads. */

protected:
	virtual void onConsume(void *v) {}
	virtual void onProduce(void *v) {}
	virtual bool canConsume(void *v) {return false;}

private:
	pthread_cond_t m_cv;
    pthread_mutex_t m_mutex;
    bool m_unblock;
};

#endif
