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
# ifdef WIN32
#include <windows.h>
#define  cond_handle   HANDLE
#define  mutex_handle  HANDLE
#else
#include <pthread.h>
#define  cond_handle   pthread_cond_t    
#define  mutex_handle  pthread_mutex_t
#endif

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

    int consume(void *v, int timeout=-1/*ms*/);
    void produce(void *v, bool broadcast=false);
    int waitEvent(int timeout=-1/*ms*/);
    void setEvent(bool broadcast=false);
    void unblockAll();

protected:
	virtual void onConsume(void *v) {}
	virtual void onProduce(void *v) {}
	virtual bool canConsume(void *v) {return false;}

private:
    cond_handle  m_cv;
    mutex_handle m_mutex;
    bool m_unblock;
};

#endif
