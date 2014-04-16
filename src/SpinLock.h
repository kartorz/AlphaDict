/**
 * A wrapper of spinlock.
 *
 * changes:
 *     Feb 20,2014  Created.  [liqiong lee]
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <pthread.h>

class SpinCriticalSection {
public:
	SpinCriticalSection()
	{
		pthread_spin_init(&m_spinlock, PTHREAD_PROCESS_PRIVATE);
	}

	~SpinCriticalSection()
	{
		pthread_spin_destroy(&m_spinlock);
	}

	void lock()
	{
		pthread_spin_lock(&m_spinlock);
	}

	void trylock()
	{
		pthread_spin_trylock(&m_spinlock);
	}
	void unlock()
	{
		pthread_spin_unlock(&m_spinlock);
	}

private:
        pthread_spinlock_t m_spinlock;
};

class SpinLock {
public:
	SpinLock(SpinCriticalSection &scs);
	~SpinLock();
private:
	SpinCriticalSection& m_criticalSection;

	SpinLock(); /* Not allowed default contruct function */
};

#endif
