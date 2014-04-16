/**
 * A wrapper of spinlock.
 *
 * changes:
 *     Feb 20,2014  Created. [liqiong lee]
 */
#include <SpinLock.h>

SpinLock::SpinLock(SpinCriticalSection &scs)
:m_criticalSection(scs)
{
	m_criticalSection.lock();
}

SpinLock::~SpinLock()
{
	m_criticalSection.unlock();
}
