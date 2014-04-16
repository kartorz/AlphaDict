#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "TaskManager.h"

class InitTask: public Task
{
public:
      InitTask(TaskCallBack* callback):Task(0, false, callback){}
      virtual void doWork();
      virtual ~InitTask(){}
};

class Application: public TaskCallBack
{
public:
    static Application&  getRefrence();
    Application();
	virtual ~Application();

    void initialization();
	bool isInit() {return m_init;}

	/* TaskCallBack*/
	virtual void onTaskDone();
private:
	bool m_init;
};

#endif
