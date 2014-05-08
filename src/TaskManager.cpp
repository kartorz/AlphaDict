/**
 * The Task Manager schedules the backgroud jobs according to the interval time.
 * It's a multi-thread system, Every thread gets and executes job.
 * A Task Manager can efficiently schedule about 20 jobs. More jobs need one 
 * more Task Manager.
 * The macro DEFAULT_THREAD_NUMBER defines the default number of total threads.
 *
 *
 * @change
 *     Mar 3,2013  Created.  [LiQiong Lee]
 */

#include "TaskManager.h"
#include "MutexLock.h"
#include "alphadict.h"
#include "Util.h"

#include <assert.h>
#include <unistd.h>

void* schedule(void *owner);
void* execute(void *owner);

Task::Task(int inv, bool rept, TaskCallBack* callback, std::string identifier)
:m_interval(inv), m_repeat(rept), m_callback(callback), m_identify(identifier), m_abort(false)
{
    if (m_callback == NULL) {
        m_callback = new TaskCallBack(); // A dummy callback, So we don't need check this pointer if is NULL.
	}

    m_callback->onTaskInit();
}

Task::~Task()
{
    m_callback->onTaskUnInit();
}

TaskManager* TaskManager::getInstance()
{
    static TaskManager* tmgr = new TaskManager();
    return tmgr;
}

TaskManager::TaskManager():m_curTask(NULL)
{
}

TaskManager::~TaskManager()
{
    g_log(LOG_INFO, "TaskManager::~TaskManager\n");
	stop();
	join();
	std::list<Task*>::const_iterator iter = m_taskQueue.begin();
	for ( ; iter != m_taskQueue.end(); ++iter) {
		delete *iter;
	}
}

void TaskManager::start(int thread_number)
{
	m_bRunning = true; /* Don't put at the end */
	pthread_t tid;
	pthread_create(&tid, NULL, &schedule, this);
	m_threadid.push_back(tid);
	LOOP(thread_number) {
		pthread_t tid;
		pthread_create(&tid, NULL, &execute, this);
		m_threadid.push_back(tid);
	}
	g_log.d("TaskManager::start(%d)\n", thread_number);
}

void TaskManager::stop()
{
	m_bRunning = false;
    m_queueCond.unblockAll();
    m_taskCond.unblockAll();
}

void TaskManager::addTask(Task *tsk, int delay)
{
	unsigned int now = Util::getTimeMS();
	MutexLock lock(m_taskQueueLock);
    //printf("{addTask} %d \n", tsk->getInterval());
	/* Can't add a task repeatly. */
    //printf("{addTask}: %u,%u\n", now, now+delay);
	if (!IsExistTask(tsk)) {
		tsk->setStartRunningTime(now+delay);
	
		/* The task was queued in ascending order by its waittime property.*/
		std::list<Task*>::iterator iter = m_taskQueue.begin();
		for ( ; iter != m_taskQueue.end(); ++iter) {
			if ((*iter)->getStartRunningTime() >= tsk->getStartRunningTime()) {
				m_taskQueue.insert(iter, tsk);
                m_queueCond.setEvent();
				return;
			}
		}
		m_taskQueue.push_back(tsk);
		m_queueCond.setEvent();
	}
}

void TaskManager::join()
{
	LOOP (m_threadid.size()) {
		pthread_join(m_threadid[i], NULL);
	}

	g_log.d("TaskManager::join, threads exit\n");
}

bool TaskManager::IsExistTask(Task *tsk)
{
	std::list<Task*>::iterator iter = m_taskQueue.begin();
	for (; iter != m_taskQueue.end(); ++iter) {
		if (*iter == tsk)
			return true;
	}
	return false;
}

Task* TaskManager::getTask(std::string identify)
{
    MutexLock lock(m_taskQueueLock);
	std::list<Task*>::iterator iter = m_taskQueue.begin();
	for (; iter != m_taskQueue.end(); ++iter) {
        if ((*iter)->getIdentify() == identify) {
            return *iter;
        }
	}
    return NULL;
}

int TaskManager::getTaskNumber()
{
    MutexLock lock(m_taskQueueLock);
    return m_taskQueue.size();
}

void TaskManager::dump()
{
	unsigned int now = Util::getTimeMS();
	std::list<Task*>::iterator iter = m_taskQueue.begin();
	printf("dump taskmanager\n");
	for (; iter != m_taskQueue.end(); ++iter) {
		printf("  startrunningtime:(%d), interval:(%d) now:(%d)\n", 
		       (*iter)->getStartRunningTime(),
		       (*iter)->getInterval(),
		       now);
	}
	printf("dump taskmanager end\n");
}

void* schedule(void *owner)
{
	TaskManager *tmgr = (TaskManager *)owner;
	assert(tmgr);
    
	while (tmgr->isRunning()) {
        //printf("{schedule}: get task from queue\n");
        bool canScheldule = true;
        {
			MutexLock lock (tmgr->m_curTaskLock);
			if (tmgr->m_curTask != NULL) {         
			    tmgr->m_taskCond.setEvent();
                canScheldule = false;
                //printf("curTask != NULL\n");
            }
            /* Do't do sleep in this block, It will delay unlock "m_curTaskLock", 
               so execute thread can't pickup this tadk */
		}

        if (!canScheldule) {
            //printf("{schedule} m_curTask != NULL\n");
            usleep(20*1000); /* 20ms*/
            pthread_yield();
            continue;
        }

        if (tmgr->getTaskNumber() == 0) {
            if (tmgr->m_queueCond.waitEvent() == -2)
                goto EXIT;
        }

        /* The queue is not empty, schedule a job */
        Task *tsk = NULL;
        do {
            unsigned int now = Util::getTimeMS();
            unsigned int start = 0;
            // Lock this block for pickup a job
            {
		        MutexLock lock(tmgr->m_taskQueueLock);
                
		        tsk = tmgr->m_taskQueue.front();
                start = tsk->getStartRunningTime();
                if (now >= start) {            
                    //printf("{schedule} got a job right now (%ld, %ld)\n", now, start);
		            tmgr->m_taskQueue.pop_front(); /* pick up this task, remove it from queue. */
                    break;
                }
            }
            /* Waitting. There will are two situations:
             *    - Add a new job. 
             *    - Waitting the front job util timeout.
             */
            int timeout = start - now;
            //printf("schedul timeout:%d, start:%u, now:%u\n", timeout, start, now);
            int wait_status = tmgr->m_queueCond.waitEvent(timeout);
            if (wait_status == -2) {
                goto EXIT;
            }
            //printf("{schedule}  schedule again\n");
            // schedule again.
         }while(true);

         /* Got a job. */
		 {
			 MutexLock lock (tmgr->m_curTaskLock);
			 tmgr->m_curTask = tsk;
			 tmgr->m_taskCond.setEvent(); /* wake up a exection thread.*/
         }
         //printf("{schedule} wakeup exection thread\n");
         pthread_yield();
		/* Dump */
		#if 0
		{
			MutexLock lock(tmgr->m_taskQueueLock);
			tmgr->dump();
		}
		#endif
	}

EXIT:
    g_log.d("{schedule} thread exit\n");
	return NULL;
}

void* execute(void *owner)
{
	TaskManager *tmgr = (TaskManager *)owner;
	assert(tmgr);
	while (tmgr->isRunning()) {
        Task* work = NULL;
		/* Lock this block */
		{
			MutexLock lock (tmgr->m_curTaskLock);
			if (tmgr->m_curTask != NULL) {
                //printf("{execute}: get task\n");
				work = tmgr->m_curTask;
				tmgr->m_curTask = NULL;
			}
		}

		if (work == NULL) {
            //printf("{execute} no work, waitting... \n");
            if(tmgr->m_taskCond.waitEvent() == -2)
                break;
            //printf("{execute} wakeup\n");
        } else {
            //printf("{execute}: %u: get a work to do \n", Util::getTimeMS());
            if (!work->isAbort()) {
			    work->doWork();
				work->m_callback->onTaskDone();
                if (work->isRepeat()) {
				    tmgr->addTask(work, work->getInterval());
			    } else {
                    delete work;
                }
                //printf("{execute} doWork\n");
            } else {
			    work->m_callback->onTaskAbort();
                g_log.d("{execute} onTaskAbort\n");
                delete work;
            }
		}
	}
    g_log.d("{execute} thread exit\n");
	return NULL;
}

