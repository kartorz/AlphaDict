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

    /* Common Arguments:
     * @timeout == 0 meas waitting infintily. otherwise, waitting 'timeout' millisecods
     * @broadcast. send a broadcast or not.
     * @return for waitting.
     *    0: satisfy contidion.
     *   -1: timeout, no event.
     *   -2: force exit, don't wait.
     */

	int consume(void *v, int timeout=0/*ms*/);
    void produce(void *v, bool broadcast=false);
	int waitEvent(int timeout=0/*ms*/);
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
