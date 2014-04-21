/**
 * A wrapper of mutex.
 *
 * changes:
 *     Feb 19,2014  Created. [LiQiong lee]
 *
 */
#include "MutexLock.h"

MutexCriticalSection::MutexCriticalSection(bool re)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	if (re)
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	else
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

	pthread_mutex_init(&m_mutex, &attr);

	pthread_mutexattr_destroy(&attr);
}


MutexLock::MutexLock(MutexCriticalSection &mcs)
:m_criticalSection(mcs)
{
	m_criticalSection.lock();
}

MutexLock::~MutexLock()
{
	m_criticalSection.unlock();
}

