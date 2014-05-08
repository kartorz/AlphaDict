#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "TaskManager.h"
#include "SysMessager.h"
#include "Configure.h"

using namespace std;

class Application;

class InitTask: public Task
{
public:
      InitTask(TaskCallBack* callback):Task(0, false, callback){}
      virtual void doWork();
      virtual ~InitTask(){}
};

class SlowJob: public Task
{
public:
      SlowJob(Application *owner):m_owner(owner),Task(1000*5, true, NULL) {}
      virtual void doWork();
      virtual ~SlowJob(){}
private:
    Application* m_owner;
};

class Configure;

class Application: public TaskCallBack
{
friend class SlowJob;
public:
    Application();
	virtual ~Application();

    void initialization();
	bool isInit() {return m_init;}

	/* TaskCallBack*/
	virtual void onTaskDone();

    Configure* m_configure;

private:
    void slowJob(void);

    SysMessager* m_sysMessager;
	bool m_init;
};

#undef EXTERN
#ifdef _APPLICATION_CPP_
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN Application g_application;
#endif
