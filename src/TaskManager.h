/**
 * Define Task and TaskManager.
 * The macro DEFAULT_THREAD_NUMBER defines the default number of total threads.
 *
 *
 * changes:
 *     Mar 3,2014  Created. [liqiong lee]
 *
 */
#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_
# ifdef WIN32
#include <windows.h>
# endif
#include "type.h"
#include "MutexLock.h"
#include "ThdCond.h"

#include <list>
#include <vector>
#include <string>

#define DEFAULT_THREAD_NUMBER  1

class TaskCallBack
{
public:
    virtual void onTaskInit() {};
    virtual void onTaskUnInit() {};
    virtual void onTaskDone() {};
    virtual void onTaskAbort() {};
};

class Task 
{
public:
	Task(int inv=60, bool rept=true, TaskCallBack* callback=NULL, std::string identifier="");
	virtual ~Task();
	virtual void doWork() = 0;
    virtual void abort() { m_abort = true; m_repeat = false;}

	int getInterval() { return m_interval; }
	unsigned int getStartRunningTime() { return m_starRunningTime; }
	void setStartRunningTime(unsigned int time) { m_starRunningTime = time; }
	bool isRepeat() { return m_repeat; }
    void setRepeat(bool rept) {m_repeat = rept;}
    std::string& getIdentify() {return m_identify;}
    bool isAbort() { return m_abort; }
	TaskCallBack* m_callback;

protected:
	int m_interval;
	unsigned int m_starRunningTime;
	bool m_repeat;
    bool m_abort;
    std::string m_identify;
};

class TaskManager {
#ifdef WIN32
friend unsigned WINAPI schedule(void* owner);
friend unsigned WINAPI execute(LPVOID owner);
#else
friend void* schedule(void *owner);
friend void* execute(void *owner);
#endif

public:
	TaskManager();
	~TaskManager();
	void start(int thread_number);
	void stop();
	bool isRunning() { return m_bRunning; }

	void addTask(Task *tsk, int delay=0);
	void deleteTask(Task *tsk) {} /* Not used */
    Task* getTask(std::string identify);

	void dump();
	int  getTaskNumber();

    static TaskManager* getInstance();
private:
	bool IsExistTask(Task *tsk);
	void waitForThrdExit();

	std::vector<pthread_t> m_threadid;
#ifdef WIN32
        std::vector<HANDLE> m_thrdhandle;
#endif

	std::list<Task*>  m_taskQueue;
	Task* m_curTask;
	MutexCriticalSection  m_taskQueueLock;
	MutexCriticalSection  m_curTaskLock;
	ThdCond m_queueCond;
	ThdCond m_taskCond;
	bool m_bRunning;
};

#endif
