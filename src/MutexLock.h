/**
 * A wrapper of mutex.
 *
 * changes:
 *     Feb 19,2014  Created. [liqiong lee]
 *
 */
#ifndef _MUTEXLOCK_H_
#define _MUTEXLOCK_H_

#include <pthread.h>

class MutexCriticalSection {
public:
	MutexCriticalSection(bool re=false);

	~MutexCriticalSection()
	{
		pthread_mutex_destroy(&m_mutex);
	}

	void lock()
	{
		pthread_mutex_lock(&m_mutex);
	}

	void trylock()
	{
		pthread_mutex_trylock(&m_mutex);
	}

	void unlock()
	{
		pthread_mutex_unlock(&m_mutex);
	}
	
	pthread_mutex_t& acquire() {return m_mutex;}
private:
    pthread_mutex_t m_mutex;
};

class MutexLock {
public:
	MutexLock(MutexCriticalSection &mcs);
	~MutexLock();
private:
	MutexCriticalSection& m_criticalSection;
};

#endif
