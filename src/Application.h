#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "MessageQueue.h"
#include "TaskManager.h"
#include "SysMessager.h"
#include "Configure.h"
#include "Util.h"

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
 
    int  start();
    void stop();
    bool isInit() {return m_init;}

    static void writePidFile();
    static bool existProcess();
    static void delPidFile();

    /* TaskCallBack*/
    virtual void onTaskDone();
    MessageQueue* sysMessageQ() { return m_sysMessageQ; }
    MessageQueue* uiMessageQ()  { return m_uiMessageQ;  }
    Configure* m_configure;

private:
    int initialization();
    void slowJob(void);

    static inline string pidPath() {
        string path;
        Util::tempDir(path);
        path += "/alphadict.pid";
        return path;
    }

    bool m_init;
    MessageQueue* m_uiMessageQ;
    MessageQueue* m_sysMessageQ;

    SysMessager* m_sysMessager;
};

extern Application g_application;
#endif
