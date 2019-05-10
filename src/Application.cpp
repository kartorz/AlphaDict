# ifdef WIN32
#include <windows.h>
#include "win32/posixtoiso.h"
# endif

# ifdef _LINUX
#include <unistd.h>
#endif

#include <stdlib.h>

#include "Application.h"
#include "Log.h"
#include "DictManager.h"
#include "TaskManager.h"
#include "SysMessager.h"
#include "alphadict.h"


Application g_application;

void InitTask::doWork()
{
    //equal to onTaskDone
}

// loop time is 5s.
void SlowJob::doWork()
{
    //printf("slowjob dowork\n");
    //g_sysLog.d("slowjob dowork\n");
    m_owner->slowJob();
}

Application::Application():m_init(false)
{
    m_sysMessageQ = new MessageQueue("ui");   /* UI system should listen this */
    m_uiMessageQ  = new MessageQueue("sys");  /* SysMessager should listen this */
    m_sysMessager = new SysMessager(m_sysMessageQ);
    m_configure   = new Configure();
}

Application::~Application()
{
    delete m_sysMessager;

    // Any thread invoking TaskManager should already stop. (SysMessager)
    // Have to do 'Application::stop()' before, or 'WaitForSingleObject' will be blocked.
    delete TaskManager::getInstance();

    delete m_configure;

    /* Some tasks may push message to message queue, before delete MessageQ, 
       all the tasks should be stopped. */
    delete m_uiMessageQ;
    delete m_sysMessageQ;
    //printf("~Application");
}

int Application::start()
{
    int ret = initialization();
    TaskManager::getInstance()->addTask(new SlowJob(this), 0);
    return ret;
}

int Application::initialization()
{
    int ret = 0;
    TaskManager::getInstance()->start(MAX_WORK_THREAD);
    g_sysLog.setLevel(LOG_DEBUG);
    g_sysLog(LOG_INFO, "Application initialization\n");

    m_sysMessager->start();
    if ((ret = m_configure->initialization()) != 0)
        return ret;

    DictManager::getReference().initialization();
    m_init = true;
    //TaskManager::getInstance()->addTask(new InitTask(this), 0);
    return ret;
}

bool Application::existProcess()
{
    string path = pidPath();
    FILE* fpid = fopen(path.c_str(), "r");
    if (fpid != NULL) {
        char buf[8] = {'\0'};
        fread(buf, 1, 7, fpid);
        int pid = Util::stringToInt(buf);
        //printf("existProcess, pidbuf:%s, pid:%d\n", buf, pid);
        if (pid > 100) {
            fclose(fpid);
            return true;
        }
    }
    return false;
}

void Application::writePidFile()
{
    string path = pidPath();
    FILE* fpid = fopen(path.c_str(), "w");
    if (fpid != NULL) {
        string strpid = Util::intToString(Util::curpid());
        //printf("writePidFile, strpid:%s\n", strpid.c_str());
        fwrite(strpid.c_str(), 1, strpid.size(), fpid);
        fclose(fpid);
    }
}

// Can't put stop or ~Application(), when exit by 'checkExistAppProc'
// will enter this functions;
void Application::delPidFile()
{
    //printf("delPidFile");
    string path = pidPath();
    FILE* fpid = fopen(path.c_str(), "w");
    if (fpid != NULL) {
        fclose(fpid);
        unlink(path.c_str());
    }
}

void Application::stop()
{
    /* Some tasks may send message, invoke g_application, So,
       The first thing to do is stopping all the tasks */
    TaskManager::getInstance()->stop();
    m_sysMessager->stop();
}

void Application::onTaskDone()
{
#if 0
    //DictManager::getReference().initialization();
    m_init = true;
    TaskManager::getInstance()->addTask(new SlowJob(this), 0);
    printf("Application onTaskDone\n");
#endif
}

void Application::slowJob(void)
{
    m_configure->writeXml();
}
